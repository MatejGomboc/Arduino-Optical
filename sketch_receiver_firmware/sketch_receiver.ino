#include <SoftwareSerial.h>

typedef struct
{
  char c;
  int i;
  long l;
  float f;
  double d;
} packet_t;

SoftwareSerial swSerial(2, 3); // RX, TX // debug interface

long numOfReceivedPackets = 0;
long numOfCurruptedPackets = 0;
float errorRate = 0.0f;
bool previousByteWasDLE = false;
char previousPacketSequenceNumber = 0;

char calculateChecksum(const char* bytes)
{
  char checksum = bytes[0];

  for (long indxByte = 1; indxByte < sizeof(packet_t) + 4; indxByte++)
  {
    checksum ^= bytes[indxByte];
  }

  return checksum;
}

void setup()
{
  Serial.begin(1000000); // init optical transmission, 1 Mbps
  swSerial.begin(9600); // init debug interface, 9600 bps
}

void loop()
{
  char receiveBuffer[sizeof(packet_t) + 4] = {0}; // create empty receive buffer
  char receivedByte = 0; // reset received byte
  long receivedByteCount = 0; // reset received bytes counter
  char checksum = 0; // reset received checksum buffer

  while (receivedByteCount < sizeof(packet_t) + 4) // check if all bytes received
  {
    if (Serial.available() > 0) // if new byte available
    {
      receivedByte = Serial.read(); // read byte

      if (receivedByte == 'd' && previousByteWasDLE == false) // perform byte unstuffing
      {
        //swSerial.println("DLE discarded"); // debug output
        previousByteWasDLE = true; // indicate that DLE was received
        continue; // receive next byte
      }
      else if (receivedByte == 'd' && previousByteWasDLE == true) // indicate if second DLE received
      {
        //swSerial.println("DLE"); // debug output
      }
      previousByteWasDLE = false; // reset DLE received indicator

      receiveBuffer[receivedByteCount] = receivedByte; // save received byte to receive buffer
      //swSerial.print("b"); swSerial.print(receivedByteCount); swSerial.print(": "); swSerial.println(receivedByte); // debug output
      receivedByteCount ++; // increase byte counter
    }
  }

  while (true) // wait for checksum byte
  {
    if (Serial.available() > 0) // if new byte available
    {
      receivedByte = Serial.read(); // read byte

      if (receivedByte == 'd' && previousByteWasDLE == false) // perform byte unstuffing
      {
        //swSerial.println("DLE discarded"); // debug output
        previousByteWasDLE = true; // indicate that DLE was received
        continue; // receive next byte
      }
      else if (receivedByte == 'd' && previousByteWasDLE == true) // indicate if second DLE received
      {
        //swSerial.println("DLE"); // debug output
      }
      previousByteWasDLE = false; // reset DLE received indicator

      checksum = receivedByte; // store checksum
      //swSerial.print("checksum: "); swSerial.println(checksum); // debug output

      break;
    }
  }

  if (receiveBuffer[0] != 'h') // if first byte is not SOH
  {
    swSerial.println("SOH err");

    numOfReceivedPackets ++; // increase received packets counter
    numOfCurruptedPackets ++; // increase currupted packets counter

    if (numOfReceivedPackets > 0) errorRate = (float)numOfCurruptedPackets * 100.0f / (float)numOfReceivedPackets; // calculate packet error rate
    swSerial.print("err rate: "); swSerial.print(errorRate); swSerial.println("%"); // debug output

    if (numOfReceivedPackets == 1000) // after 1000 received packets reset both packet counters
    {
      numOfReceivedPackets = 0;
      numOfCurruptedPackets = 0;
    }

    swSerial.println(); // debug output
    return;
  }

  if (receiveBuffer[2] != 's') // if STX missing
  {
    swSerial.println("STX err");

    numOfReceivedPackets ++; // increase received packets counter
    numOfCurruptedPackets ++; // increase currupted packets counter

    if (numOfReceivedPackets > 0) errorRate = (float)numOfCurruptedPackets * 100.0f / (float)numOfReceivedPackets; // calculate packet error rate
    swSerial.print("err rate: "); swSerial.print(errorRate); swSerial.println("%"); // debug output

    if (numOfReceivedPackets == 1000) // after 1000 received packets reset both packet counters
    {
      numOfReceivedPackets = 0;
      numOfCurruptedPackets = 0;
    }

    swSerial.println(); // debug output
    return;
  }

  if (receiveBuffer[sizeof(packet_t) + 3] != 'e') // if ETX missing
  {
    swSerial.println("ETX err");

    numOfReceivedPackets ++; // increase received packets counter
    numOfCurruptedPackets ++; // increase currupted packets counter

    if (numOfReceivedPackets > 0) errorRate = (float)numOfCurruptedPackets * 100.0f / (float)numOfReceivedPackets; // calculate packet error rate
    swSerial.print("err rate: "); swSerial.print(errorRate); swSerial.println("%"); // debug output

    if (numOfReceivedPackets == 1000) // after 1000 received packets reset both packet counters
    {
      numOfReceivedPackets = 0;
      numOfCurruptedPackets = 0;
    }

    swSerial.println(); // debug output
    return;
  }

  swSerial.print("seq num: "); swSerial.println((unsigned)receiveBuffer[1]); // debug output

  if (((previousPacketSequenceNumber < 127) && (receiveBuffer[1] != (previousPacketSequenceNumber + 1))) || ((previousPacketSequenceNumber == 127) && (receiveBuffer[1] != 0)))
  {
    // packet sequence number error, some packets were lost OR current packet currupted
    
    swSerial.println("seq num err"); // debug output
    previousPacketSequenceNumber = receiveBuffer[1]; // save current sequence number

    numOfReceivedPackets ++; // increase received packet number
    numOfCurruptedPackets ++; // increase currupted packet number

    if (numOfReceivedPackets > 0) errorRate = (float)numOfCurruptedPackets * 100.0f / (float)numOfReceivedPackets; // calculate packet error rate
    swSerial.print("err rate: "); swSerial.print(errorRate); swSerial.println("%"); // debug output

    if (numOfReceivedPackets == 1000) // after 1000 received packets reset both packet counters
    {
      numOfReceivedPackets = 0;
      numOfCurruptedPackets = 0;
    }

    swSerial.println(); // debug output
    return;
  }

  previousPacketSequenceNumber = receiveBuffer[1]; // save current sequence number

  if (checksum != calculateChecksum(receiveBuffer)) // calculate checksumm, compare received and calculated checksumm
  {
    swSerial.println("checksum err"); // debug output

    numOfReceivedPackets ++; // increase received packet number
    numOfCurruptedPackets ++; // increase currupted packets counter

    if (numOfReceivedPackets > 0) errorRate = (float)numOfCurruptedPackets * 100.0f / (float)numOfReceivedPackets; // calculate packet error rate
    swSerial.print("err rate: "); swSerial.print(errorRate); swSerial.println("%"); // debug output

    if (numOfReceivedPackets == 1000) // after 1000 received packets reset both packet counters
    {
      numOfReceivedPackets = 0;
      numOfCurruptedPackets = 0;
    }

    swSerial.println(); // debug output
    return;
  }

  packet_t packet; // current received packet
  memcpy(&packet, &receiveBuffer[3], sizeof(packet_t)); // deserialize received bytes, reconstruct packet

  numOfReceivedPackets ++; // increase received packets counter

  if (numOfReceivedPackets > 0) errorRate = (float)numOfCurruptedPackets * 100.0f / (float)numOfReceivedPackets; // calculate packet error rate
  swSerial.print("err rate: "); swSerial.print(errorRate); swSerial.println("%"); // debug output

  if (numOfReceivedPackets == 1000) // after 1000 received packets reset both packet counters
  {
    numOfReceivedPackets = 0;
    numOfCurruptedPackets = 0;
  }

  swSerial.print("c: "); swSerial.println(packet.c); // debug output
  swSerial.print("i: "); swSerial.println(packet.i); // debug output
  swSerial.print("l: "); swSerial.println(packet.l); // debug output
  swSerial.print("f: "); swSerial.println(packet.f); // debug output
  swSerial.print("d: "); swSerial.println(packet.d); // debug output
  swSerial.println(); // debug output

  return;
}
