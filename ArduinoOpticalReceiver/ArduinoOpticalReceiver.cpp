#include "ArduinoOpticalReceiver.h"

void ArduinoOpticalReceiver::begin(unsigned long baudRate)
{
	Serial.begin(baudRate); // open serial port
	_sequenceNumberReceived = 0; // reset received seq num
	_sequenceNumberExpected = 0; // reset calculated seq num
	_receivedCRC = 0; // reset received crc
	_calculatedCRC = 0; // reset calculated crc
	_byteCount = 0; // reset payload byte counter
	_delimiterReceived = false; // reset delimiter indicator
	_state = waitStart; // first state
}

char ArduinoOpticalReceiver::updateChecksum(const char byte, char crc)
{
	crc = crc ^ byte;  
	return crc;
}

bool ArduinoOpticalReceiver::receivePacket(char* bytes, const unsigned long length, char* status)
{
	if (length == 0 || bytes == NULL)
	{
		// if invalid function parameters
		*status = 0;
		return true;
	}

	char receivedByte = 0; // received byte buffer
	
	if (Serial.available() > 0) // if data available
	{
		receivedByte = Serial.read(); // read one received byte
		
		// perform byte unstuffing
		if (receivedByte == 'd' && _delimiterReceived == false) // if first delimiter received
		{
			_delimiterReceived = true;
			*status = 1;
			// discard first delimiter byte
			return false;
		}
		if (_delimiterReceived == true)
		{
			// if second delimiter received
			// keep second delimiter
			_delimiterReceived = false;
		}
		
		switch(_state)
		{
			case waitStart: // wait for start of frame
				// if start of frame received go to next State
				if (receivedByte == 's') _state = waitSeq;
				*status = 1;
				return false;
			case waitSeq: // receive sequence number
				_sequenceNumberReceived = receivedByte;
				_calculatedCRC = 0;
				_calculatedCRC = updateChecksum(receivedByte, _calculatedCRC);
				_state = receiveData;
				_byteCount = 0;
				if(_sequenceNumberReceived != _sequenceNumberExpected)
				{
					// if invalid sequence number
					*status = 2;
					_sequenceNumberExpected = _sequenceNumberReceived;
				}
				else *status = 1;
				return false;
			case receiveData: // receive payload
				bytes[_byteCount] = receivedByte;
				_calculatedCRC = updateChecksum(receivedByte, _calculatedCRC);
				_byteCount++; // increment payload byte counter
				if (_byteCount == length) _state = waitCRC;
				*status = 1;
				return false;
			case waitCRC: // receive CRC
				_receivedCRC = 0;
				_receivedCRC = receivedByte;
				_calculatedCRC = updateChecksum(receivedByte, _calculatedCRC);
				_state = waitStart;
				if(_receivedCRC != _calculatedCRC) *status = 3;
				else *status = 1;
				if(_sequenceNumberExpected == 127) _sequenceNumberExpected = 0;
				else _sequenceNumberExpected++;
				return true;
		}
	}
	else
	{
		*status = 1;
		return false;
	}
}