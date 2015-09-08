#include "ArduinoOpticalReceiver.h"

void ArduinoOpticalReceiver::begin(unsigned long baudRate)
{
	Serial.begin(baudRate); // open serial port
	sequenceNumberReceived = 0; // reset received seq num
	sequenceNumberExpected = 0; // reset calculated seq num
	_receivedCRC = 0; // reset received crc
	_calculatedCRC = 0; // reset calculated crc
	_byteCount = 0; // reset payload byte counter
	_delimiterReceived = false; // reset delimiter indicator
	seqNumError = false; // reset sequence number error indicator
	CRCerror = false; // reset CRC error indicator
	_state = waitStart; // first state
	
	numOfReceivedPackets = 0; // reset received packets counter
	numOfCurruptedPackets = 0; // reset currupted packets counter
	errorRate = 0.0f; // reset error rate
}

char ArduinoOpticalReceiver::updateChecksum(const char byte, char crc)
{
	crc = crc ^ byte;  
	return crc;
}

bool ArduinoOpticalReceiver::receivePacket(char* bytes, const unsigned long length)
{
	if (length == 0 || bytes == NULL) return true; // if invalid function parameters
	
	char receivedByte = 0; // reset received byte buffer
	
	if (Serial.available() > 0) // if data available
	{
		receivedByte = Serial.read(); // read one received byte
		
		// perform byte unstuffing
		if (receivedByte == 'd' && _delimiterReceived == false) // if first delimiter received
		{
			_delimiterReceived = true;
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
				// if start of frame received go to next state
				if (receivedByte == 's') _state = waitSeq;
				_calculatedCRC = 0; // reset CRC
				seqNumError = false; // reset sequence number error indicator
	            CRCerror = false; // reset CRC error indicator
				return false;
			case waitSeq: // receive sequence number
				sequenceNumberReceived = receivedByte;
				_calculatedCRC = 0;  // reset CRC
				_calculatedCRC = updateChecksum(receivedByte, _calculatedCRC);
				// if sequence number received go to next state
				_state = receiveData;
				_byteCount = 0;
				if (sequenceNumberReceived != sequenceNumberExpected) seqNumError = true; // if invalid sequence number
				else seqNumError = false;
				return false;
			case receiveData: // receive payload
				bytes[_byteCount] = receivedByte;
				_calculatedCRC = updateChecksum(receivedByte, _calculatedCRC);
				_byteCount++; // increment payload byte counter
				// if all payload received go to next state
				if (_byteCount == length) _state = waitCRC;
				CRCerror = false; // reset CRC error indicator
				return false;
			case waitCRC: // receive CRC
				_receivedCRC = receivedByte;
				_calculatedCRC = updateChecksum(receivedByte, _calculatedCRC);
				// if CRC received go to frst state
				_state = waitStart;
				if(_receivedCRC != _calculatedCRC) CRCerror = true; // if CRC error
				else CRCerror = false;
				sequenceNumberExpected = sequenceNumberReceived; // update packet sequence number
				if(sequenceNumberExpected == 127) sequenceNumberExpected = 0;
				else sequenceNumberExpected++;
				numOfReceivedPackets ++; // increase received packets counter
				numOfCurruptedPackets ++; // increase currupted packets counter
				if (numOfReceivedPackets > 0) errorRate = (float)numOfCurruptedPackets * 100.0f / (float)numOfReceivedPackets; // calculate packet error rate
				if (numOfReceivedPackets == 1000) // after 1000 received packets reset both packet counters
				{
				  numOfReceivedPackets = 0;
				  numOfCurruptedPackets = 0;
				}
				return true; // entire packet received
		}
	}
	else return false;
}