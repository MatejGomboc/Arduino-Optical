#ifndef EasyTransferToslink_h
#define EasyTransferToslink_h

//make it a little prettier on the front end. 
#define details(name) (byte*)&name,sizeof(name)

//not neccessary, but just in case. 
#if ARDUINO > 22
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "Stream.h"
#include <SoftwareSerial.h>

//size of encoded string transmitt buffer
#define encodedStringSize(size) ((size + 2)*2 + 1) * 2 + 4

// packet structure:
// preamble[4] + 0x02[1] + _size[1] + data[_size] + CRC[1]

class EasyTransferToslink
{
public:
	EasyTransferToslink(); // default constructor
	~EasyTransferToslink(); // destructor
	
	void begin(uint8_t * ptr, uint8_t length, Stream *theStream); // establish optical connection
	void sendData(SoftwareSerial swSerial); // send one Manchester encoded frame
	bool receiveData(SoftwareSerial swSerial); // receive one Manchester encoded frame, if all data received returns true else returns false
	bool receiveFailed; // reception failed indicator
private:
	// all possible states of receive state machine
	typedef enum {waitStart, receiveLength, receivingData, receiveCRC} State;

	Stream* _stream;		//stream selected for optical communication
	uint8_t* _address;      //address of struct
	uint8_t _size;          //size of struct
	uint8_t* _rx_buffer;    //address for temporary storage and parsing buffer
	uint8_t _rx_array_inx;  //index for RX parsing buffer
	uint8_t _rx_len;		//RX packet length according to the packet
	uint8_t _calc_CRC;	    //calculated Chacksum

	//break one byte into two nibbles
	void getUpperAndLowerNibble(uint8_t inputByte, uint8_t* lowerNibble, uint8_t* upperNibble);
	uint8_t encodeNibble(uint8_t inputNibble); // Manchester encode one nibble
	// add one byte to encoded string transmitt buffer
	void updateEncodedString(uint8_t inputByte, uint8_t* encodedString, uint8_t indx);
	
	uint8_t decodeByte(uint8_t encoded); // decode one Manchester encoded byte
	uint8_t reconstructByte(uint8_t lowerNibble, uint8_t upperNibble);
	
	bool _lowerNibbleReceived;
	uint8_t _receivedLowerNibble;
	uint8_t _receivedUpperNibble;
	
	State _state; // current state of receiver state machine
	bool _delimiterReceived; // indicate if byte stuffing delimiter received
	
	uint8_t _timeoutCounter; // counter to indicate reception timeout
};

#endif