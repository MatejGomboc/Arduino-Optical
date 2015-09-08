#include "ArduinoOpticalTransmitter.h"

typedef struct // define packet structure
{
  char c;
  int i;
  long l;
  float f;
  double d;
} packet_t;

packet_t packet; // packet to be transmitted
ArduinoOpticalTransmitter arduinoOpticalTransmitter;

void setup()
{
  arduinoOpticalTransmitter.begin(1000000); // init optical transmission, 1 Mbps
  
  packet.c = 42; // prepare new packet to be sent
  packet.i = 2;
  packet.l = 3L;
  packet.f = 4.0f;
  packet.d = 5.0;

  pinMode(13, OUTPUT); // enable LED on pin 13, LED is blinking if packets being transmitted
}

void loop()
{
  delay(100);
  digitalWrite(13, LOW); // turn LED off
  
  packet.c ++; // prepare new packet to be sent
  packet.i ++;
  packet.l ++;
  packet.f += 1.0f;
  packet.d += 1.0;

  char transmittBuffer[sizeof(packet_t)] = {0}; // create empty transmitt buffer
  memcpy(&transmittBuffer[0], &packet, sizeof(packet_t)); // serialize packet
  arduinoOpticalTransmitter.sendPacket(&transmittBuffer[0], sizeof(packet_t), 100); // send packet, inter-byte delay 100ms
  
  delay(100); // inter-packet delay 200ms
  digitalWrite(13, HIGH); // turn LED on
}
