#include "ArduinoOpticalTransmitter.h"

void ArduinoOpticalTransmitter::begin(unsigned long baudRate)
{
	Serial.begin(baudRate); // set baudrate
	_sequenceNumber = 0; // reset packet sequence number
}

char ArduinoOpticalTransmitter::updateChecksum(const char byte, char crc)
{
  // calculate packet checksum
  crc = crc ^ byte;
  return crc;
}

void ArduinoOpticalTransmitter::sendPacket(const char* bytes, const unsigned long length, const unsigned long byteDelay)
{
	char crc = 0; // reset checksum
	Serial.print('s'); // send start of frame
	delay(byteDelay);
	
	if((_sequenceNumber == 's') || (_sequenceNumber == 'd')) // perform byte stuffing
	{
		Serial.print('d'); // send delimiter
		delay(byteDelay);
	}
	crc = updateChecksum(_sequenceNumber, crc);
	Serial.print(_sequenceNumber); // send packet sequence number
	delay(byteDelay);
	
	for(unsigned long i = 0; i < length; i++)
	{
		if((bytes[i] == 's') || (bytes[i] == 'd'))  // perform byte stuffing
		{
			Serial.print('d'); // send delimiter
			delay(byteDelay);
		}
		crc = updateChecksum(bytes[i], crc);
		Serial.print(bytes[i]);  // send next payload byte
		delay(byteDelay);
	}
	
	if((crc == 's') || (crc == 'd'))  // perform byte stuffing
	{
		Serial.print('d'); // send delimiter
		delay(byteDelay);
	}
	Serial.print(crc);  // send packet CRC
	delay(byteDelay);
	
	// update packet sequence number
	if(_sequenceNumber == 127) _sequenceNumber = 0;
	else _sequenceNumber++;
}