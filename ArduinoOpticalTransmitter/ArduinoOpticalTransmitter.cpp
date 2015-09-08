#include "ArduinoOpticalTransmitter.h"

void ArduinoOpticalTransmitter::begin(unsigned long baudRate)
{
	Serial.begin(baudRate);
	_sequenceNumber = 0;
}

char ArduinoOpticalTransmitter::updateChecksum(const char byte, char crc)
{
  crc = crc ^ byte;
  return crc;
}

void ArduinoOpticalTransmitter::sendPacket(const char* bytes, const unsigned long length, const unsigned long byteDelay)
{
	char crc = 0;
	Serial.print('s');
	delay(byteDelay);
	
	if((_sequenceNumber == 's') || (_sequenceNumber == 'd'))
	{
		Serial.print('d');
		delay(byteDelay);
	}
	crc = updateChecksum(_sequenceNumber, crc);
	Serial.print(_sequenceNumber);
	delay(byteDelay);
	
	for(unsigned long i = 0; i < length; i++)
	{
		if((bytes[i] == 's') || (bytes[i] == 'd'))
		{
			Serial.print('d');
			delay(byteDelay);
		}
		crc = updateChecksum(bytes[i], crc);
		Serial.print(bytes[i]);
		delay(byteDelay);
	}
	
	if((crc == 's') || (crc == 'd'))
	{
		Serial.print('d');
		delay(byteDelay);
	}
	Serial.print(crc);
	delay(byteDelay);
	
	if(_sequenceNumber == 127) _sequenceNumber = 0;
	else _sequenceNumber++;
}