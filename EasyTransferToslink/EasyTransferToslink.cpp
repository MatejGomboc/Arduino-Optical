#include "EasyTransferToslink.h"

//captures address and size of struct
void EasyTransferToslink::begin(uint8_t* ptr, uint8_t length, Stream* theStream)
{
  _address = ptr;
  _size = length;
  _stream = theStream;

  //dynamic creation of rx parsing buffer in RAM
  _rx_buffer = (uint8_t*) malloc(_size);

  _lowerNibbleReceived = false;
  _receivedLowerNibble = 0;
  _receivedUpperNibble = 0;

  _state = waitStart; // first state of receiver state machine
  _delimiterReceived = false;
  _timeoutCounter = 0;
  receiveFailed = false;
}

EasyTransferToslink::EasyTransferToslink()
{
  _rx_buffer = NULL;
}

EasyTransferToslink::~EasyTransferToslink()
{
  if (_rx_buffer != NULL) free(_rx_buffer); // release rx parsing buffer
  _rx_buffer = NULL;
}

void EasyTransferToslink::getUpperAndLowerNibble(uint8_t inputByte, uint8_t* lowerNibble, uint8_t* upperNibble)
{
  *lowerNibble = inputByte & 0x0f;
  *upperNibble = (inputByte >> 4) & 0x0f;
}

uint8_t EasyTransferToslink::encodeNibble(uint8_t inputNibble)
{
  uint8_t encodedNibble = 0; // manchester encoded txbyte
  for (int j = 0 ; j < 4; j++)
  {
    encodedNibble >>= 2;
    if (inputNibble & 0b00000001 == 0b00000001)
      encodedNibble |= 0b01000000; // 1->0
    else
      encodedNibble |= 0b10000000; // 0->1
    inputNibble >>= 1;
  }
  return encodedNibble;
}

void EasyTransferToslink::updateEncodedString(uint8_t inputByte, uint8_t* encodedString, uint8_t indx)
{
  // add one Mancester encoded byte to transmitt buffer
  uint8_t lowerNibble = 0, upperNibble = 0;
  getUpperAndLowerNibble(inputByte, &lowerNibble, &upperNibble);

  *(encodedString + indx) = encodeNibble(lowerNibble);
  *(encodedString + indx + 1) = encodeNibble(upperNibble);
}

// sends out struct in binary, with start-of-frame, length info and checksum
// packet structure:
// preamble[4] + 0x02[1] + _size[1] + data[_size] + CRC[1]
void EasyTransferToslink::sendData(SoftwareSerial swSerial) // swSerial needed when debuging
{
  uint8_t* encodedString = (uint8_t*) malloc(encodedStringSize(_size));
  uint8_t indx = 0;
  uint8_t CRC = _size;
  
  // swSerial.println("reserved encoded string buffer");

  for (int i = 0; i < 4; i++) // add preamble
  {
    encodedString[indx] = 0xf0;
    indx++;
  }
  
  // swSerial.println("preamble added");

  updateEncodedString(0x02, &encodedString[0], indx);
  indx += 2;
  // swSerial.println("start byte added");
  
  if(_size == 0x02 || _size == 0x10) // byte stuffing
  {
	updateEncodedString(0x10, &encodedString[0], indx);
	indx += 2;
  }
  updateEncodedString(_size, &encodedString[0], indx);
  indx += 2;
  
  // swSerial.println("packet size added");

  for (int i = 0; i < _size; i++) {
    CRC ^= *(_address + i);
	if(*(_address + i) == 0x02 || *(_address + i) == 0x10) // byte stuffing
	{
		updateEncodedString(0x10, &encodedString[0], indx);
		indx += 2;
	}
    updateEncodedString(*(_address + i), &encodedString[0], indx);
    indx += 2;
	
	// swSerial.print("data byte"); // swSerial.print(i); // swSerial.print(" added: "); // swSerial.println(*(_address + i));
  }

  if(CRC == 0x02 || CRC == 0x10) // byte stuffing
  {
	updateEncodedString(0x10, &encodedString[0], indx);
	indx += 2;
  }
  updateEncodedString(CRC, &encodedString[0], indx);
  
  // swSerial.print("CRC added: "); // swSerial.println(CRC);

  for (int i = 0; i < encodedStringSize(_size); i++) // send encoded string
  {
    _stream->write(encodedString[i]);
  }
  
  // swSerial.println("encoded string sent");

  free(encodedString); // release encoded string transmitt buffer
  
  // swSerial.println("transmitt buffer released");
  // swSerial.println();
}

uint8_t EasyTransferToslink::decodeByte(uint8_t encoded)
{
  uint8_t i, dec, enc, pattern;
  enc = encoded;
  if (enc == 0xf0) // start/end condition encountered
    return 0xf0;
  dec = 0;
  for (i = 0; i < 4; i++)
  {
    dec >>= 1;
    pattern = enc & 0b00000011;
    if (pattern == 0b00000001) // 1
      dec |= (1 << 3);
    else if (pattern == 0b00000010)
      dec &= ~(1 << 3); // 0
    else
      return 0xff; // illegal code
    enc >>= 2;
  }
  return dec;
}

uint8_t EasyTransferToslink::reconstructByte(uint8_t lowerNibble, uint8_t upperNibble)
{
  uint8_t outputByte = lowerNibble & 0x0f;
  outputByte |= (upperNibble << 4) & 0xf0;
  return outputByte;
}

bool EasyTransferToslink::receiveData(SoftwareSerial swSerial) // swSerial needed when debuging
{
	if (_timeoutCounter == 0) receiveFailed = false; // if start of reception

	// swSerial.print("timeout counter: "); // swSerial.println(_timeoutCounter);
	if (_timeoutCounter >= encodedStringSize(_size)) // if timeout error
	{
		// swSerial.println("timeout reached");
		_state = waitStart;
		receiveFailed = true;
		_timeoutCounter = 0;
		return false;
	}

	// swSerial.print("state: "); // swSerial.println(_state);
	// swSerial.print("available bytes: "); // swSerial.println(Serial.available());

	if (Serial.available() < 1) // if empty receive buffer
	{
		// swSerial.println("empty receive buffer");
		_state = waitStart;
		receiveFailed = true;
		_timeoutCounter = 0;
		return false;
	}

	uint8_t receivedByte = 0;

    if (!_lowerNibbleReceived)
    {
      _receivedLowerNibble = decodeByte(_stream->read()); // receive and decode lower nibble
      if (_receivedLowerNibble == 0xf0) // if preamble byte received
      {
		// swSerial.print("preamble byte received: "); // swSerial.println(_receivedLowerNibble, HEX);
        _lowerNibbleReceived = false;
		if (_state != waitStart) // error if not in beginning state
		{
			_state = waitStart;
			receiveFailed = true;
			_timeoutCounter = 0;
			return false;
		}
		_timeoutCounter++;
        return false;
      }
	  if (_receivedLowerNibble == 0xff) // if invalid byte received
	  {
		// swSerial.print("invalid nibble received: "); // swSerial.println(_receivedLowerNibble, HEX);
        _lowerNibbleReceived = false;
		if (_state != waitStart) // error if not in beginning state
		{
			_state = waitStart;
			receiveFailed = true;
			_timeoutCounter = 0;
			return false;
		}
		_timeoutCounter++;
        return false;
	  }
      _lowerNibbleReceived = true;
	  // swSerial.println("lower nibble received");
	  return false;
    }

    _receivedUpperNibble = decodeByte(_stream->read()); // receive and decode upper nibble
    if (_receivedUpperNibble == 0xf0) // if preamble byte received
    {
		// swSerial.print("preamble byte received: "); // swSerial.println(_receivedUpperNibble, HEX);
		_lowerNibbleReceived = false; // receive again lower nibble
		if (_state != waitStart) // error if not in beginning state
		{
			_state = waitStart;
			receiveFailed = true;
			_timeoutCounter = 0;
			return false;
		}
		_timeoutCounter++;
		return false;
    }
	if (_receivedUpperNibble == 0xff) // if invalid byte received
	{
		// swSerial.print("invalid nibble received: "); // swSerial.println(_receivedUpperNibble, HEX);
		_lowerNibbleReceived = false; // receive again lower nibble
		if (_state != waitStart) // error if not in beginning state
		{
			_state = waitStart;
			receiveFailed = true;
			_timeoutCounter = 0;
			return false;
		}
		_timeoutCounter++;
		return false;
	}
	// swSerial.println("upper nibble received");
    _lowerNibbleReceived = false;
    receivedByte = reconstructByte(_receivedLowerNibble, _receivedUpperNibble); // reconstruct original byte
	
	if (_state == waitStart && _delimiterReceived == false && receivedByte == 0x02) // if start byte received	
	{
		// swSerial.println("start byte received");
		_state = receiveLength; // proceede with reception of packet length
		_timeoutCounter = 0;
		return false;
	}
	
	// perform byte unstuffing
	if (receivedByte == 0x10 && _delimiterReceived == false) // if first delimiter received
	{
		// swSerial.println("delimiter received");
		_delimiterReceived = true;
		// discard first delimiter byte
		return false;
	}
	if (_delimiterReceived == true)
	{
		// swSerial.println("second delimiter or other character received");
		// if second delimiter received
		// keep second delimiter
		_delimiterReceived = false;
	}
	
	switch(_state) // data receiving state machine
	{
		// swSerial.println("entering receive state machine");
		case receiveLength:
			// swSerial.println("receiving packet length");
			_rx_len = receivedByte; // packet length received
			// swSerial.print("length: "); // swSerial.println(_rx_len);
			if (_rx_len == _size) // if received length OK
			{
				// swSerial.println("packet length OK");
				_state = receivingData;
				_rx_array_inx = 0;
				_calc_CRC = _rx_len;
				_timeoutCounter = 0;
			}
			else // if packet length not matching, error
			{
				// swSerial.println("packet length error");
				_rx_len = 0;
				_state = waitStart;
				receiveFailed = true;
				_timeoutCounter = 0;
				// swSerial.println("receive failed");
			}
		    return false;
		case receivingData: // receiving payload
			// swSerial.println("receiving payload");
			if (_rx_array_inx < _rx_len)
			{
				// swSerial.print("payload byte"); // swSerial.print(_rx_array_inx); // swSerial.print(" received: ");
				_rx_buffer[_rx_array_inx] = receivedByte;
				// swSerial.println(_rx_buffer[_rx_array_inx]);
				_calc_CRC ^= _rx_buffer[_rx_array_inx];
				_rx_array_inx++;
				_timeoutCounter = 0;
			}
			if (_rx_array_inx == _rx_len) // if entire payload received
			{
				// swSerial.println("entire payload received");
				_rx_array_inx = 0;
				_state = receiveCRC;
				_timeoutCounter = 0;
			}
			return false;
		case receiveCRC: // receive packet CRC
			// swSerial.println("receiving CRC");
			// swSerial.print("CRC: "); // swSerial.println(receivedByte);
			if(_calc_CRC == receivedByte) // if CRC OK
			{
				// swSerial.println("CRC OK");
				memcpy(_address, _rx_buffer, _size);
				_rx_len = 0;
				_rx_array_inx = 0;
				_state = waitStart;
				receiveFailed = false;
				_timeoutCounter = 0;
				return true;
			}
			else // if CRC error
			{
				// swSerial.println("CRC error");
				_rx_len = 0;
				_rx_array_inx = 0;
				_state = waitStart;
				receiveFailed = true;
				_timeoutCounter = 0;
				// swSerial.println("receive failed");
				return false;
			}
		default:
			// swSerial.println("out of state machine");
			_timeoutCounter++;
			return false;
	}
	
	// swSerial.println("out of state machine");
	_timeoutCounter++;
	return false;
}