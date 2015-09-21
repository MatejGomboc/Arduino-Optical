#include <SoftwareSerial.h>
#include "ArduinoOpticalReceiver.h"

typedef union // define packet
{
  struct __attribute__((__packed__))
  {
    char c;
    int i;
    long l;
    float f;
    double d;
  } value;
  char* bytes;
} packet_t;

packet_t packet; // received packet
ArduinoOpticalReceiver arduinoOpticalReceiver;
SoftwareSerial swSerial(2, 3); // RX, TX // debug interface

void setup()
{
  arduinoOpticalReceiver.begin(1000000); // init optical reception, 1 Mbps
  swSerial.begin(9600); // init debug interface, 9600 bps
}

void loop()
{
  packet.value.c = 0; // reset packet
  packet.value.i = 0;
  packet.value.l = 0L;
  packet.value.f = 0.0f;
  packet.value.d = 0.0;
  
  while(false == arduinoOpticalReceiver.receivePacket(&packet.bytes[0], sizeof(packet_t))); // receive one packet

  if (arduinoOpticalReceiver.seqNumError)
  {
    swSerial.println("Invalid packet sequence number.");
    swSerial.print("expected seqNum: "); swSerial.println(arduinoOpticalReceiver.sequenceNumberExpected);
    swSerial.print("received seqNum: "); swSerial.println(arduinoOpticalReceiver.sequenceNumberReceived);
    swSerial.println();
  }

  if (arduinoOpticalReceiver.CRCerror)
  {
    swSerial.println("Invalid packet CRC. Packet currupted.");
    swSerial.println();
  }

  swSerial.print("err rate: "); swSerial.print(arduinoOpticalReceiver.errorRate); swSerial.println("%");
  swSerial.println();

  // packet printout
  swSerial.print("c: "); swSerial.println(packet.value.c); // debug output
  swSerial.print("i: "); swSerial.println(packet.value.i); // debug output
  swSerial.print("l: "); swSerial.println(packet.value.l); // debug output
  swSerial.print("f: "); swSerial.println(packet.value.f); // debug output
  swSerial.print("d: "); swSerial.println(packet.value.d); // debug output
  swSerial.println(); // debug output
}
