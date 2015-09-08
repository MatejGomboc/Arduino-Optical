/*
  ArduinoOptical.h - Library for sending data packets via optical serial link.
  Created by Matej Gomboc, September 5, 2015.
  Released into the public domain.
*/

#ifndef ArduinoOpticalTransmitter_h
#define ArduinoOpticalTransmitter_h

#include "Arduino.h"

class ArduinoOpticalTransmitter
{
  public:
    void begin(unsigned long baudRate); // begin optical communication
	void sendPacket(const char* bytes, const unsigned long length, const unsigned long byteDelay); // transmitt one packet
  private:
	char _sequenceNumber; // current packet sequence number
	char updateChecksum(const char byte, char crc); // calculate packet checksumm
};

#endif