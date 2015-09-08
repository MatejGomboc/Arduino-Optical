/*
  ArduinoOpticalReceiver.h - Library for receiving data packets via optical serial link.
  Created by Matej Gomboc, September 8, 2015.
  Released into the public domain.
*/

#ifndef ArduinoOpticalReceiver_h
#define ArduinoOpticalReceiver_h

#include "Arduino.h"

// all possible states of receive state machine
typedef enum {waitStart, waitSeq, receiveData, waitCRC} State;

class ArduinoOpticalReceiver
{
	public:
		void begin(unsigned long baudRate); // begin optical communication
		bool receivePacket(char* bytes, const unsigned long length); // receive one packet
		// returns true if entire packet received else return false
		// status holds current receiver status: 1-normal, 2-seqNumErr, 3-CRCerr, 0-invalidFunctionParameters
		char sequenceNumberExpected; // current expected packet sequence number
		char sequenceNumberReceived; // current received sequence number
		bool seqNumError; // if invalid sequence number
		bool CRCerror; // if CRC error
		long numOfReceivedPackets; // received packets counter
		long numOfCurruptedPackets; // currupted packets counter
		float errorRate; // error rate
	private:
		char _receivedCRC; // received packet CRC
		char _calculatedCRC; // calculated  packet CRC
		unsigned long _byteCount; // payload byte counter
		bool _delimiterReceived; // if previous byte was delimiter
		State _state; // current State of receive state machine
		char updateChecksum(const char byte, char crc); // calculate packet checksumm
};

#endif