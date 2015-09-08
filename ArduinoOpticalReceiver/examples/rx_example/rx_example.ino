#include "ArduinoOpticalReceiver.h"
#include <SoftwareSerial.h>

typedef struct // define packet structure
{
  char c;
  int i;
  long l;
  float f;
  double d;
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
  packet.c = 0; // reset packet
  packet.i = 0;
  packet.l = 0L;
  packet.f = 0.0f;
  packet.d = 0.0;
  
  char receiveBuffer[sizeof(packet_t)] = {0}; // create empty receive buffer
  char receiverStatus = 1; // reset status
  
  while(false == arduinoOpticalReceiver.receivePacket(&receiveBuffer[0], sizeof(packet_t), &receiverStatus)) // receive one packet
  {
    switch(receiverStatus)
    {
      case 0:
        swSerial.println("Invalid packet length OR receive buffer error.");
        break;
      case 1:
        // swSerial.println("Ok");
        break;
      case 2:
        swSerial.println("Invalid packet sequence number.");
        swSerial.println(arduinoOpticalReceiver._sequenceNumberReceived);
        swSerial.println(arduinoOpticalReceiver._sequenceNumberExpected);
        break;
      case 3:
        swSerial.println("Invalid packet CRC. Packet currupted.");
        break;
    }
  }
  
  memcpy(&packet, &receiveBuffer[0], sizeof(packet_t)); // deserialize received bytes, reconstruct packet

  // packet printout
  swSerial.print("c: "); swSerial.println(packet.c); // debug output
  swSerial.print("i: "); swSerial.println(packet.i); // debug output
  swSerial.print("l: "); swSerial.println(packet.l); // debug output
  swSerial.print("f: "); swSerial.println(packet.f); // debug output
  swSerial.print("d: "); swSerial.println(packet.d); // debug output
  swSerial.println(); // debug output
}
