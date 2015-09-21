#include "ArduinoOpticalTransmitter.h"

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

packet_t packet; // packet to be transmitted
ArduinoOpticalTransmitter arduinoOpticalTransmitter;

void setup()
{
  arduinoOpticalTransmitter.begin(1000000); // init optical transmission, 1 Mbps
  
  packet.value.c = 42; // prepare new packet to be sent
  packet.value.i = 2;
  packet.value.l = 3L;
  packet.value.f = 4.0f;
  packet.value.d = 5.0;

  pinMode(13, OUTPUT); // enable LED on pin 13, LED is blinking if packets being transmitted
}

void loop()
{
  delay(5);
  digitalWrite(13, LOW); // turn LED off
  
  packet.value.c ++; // prepare new packet to be sent
  packet.value.i ++;
  packet.value.l ++;
  packet.value.f += 1.0f;
  packet.value.d += 1.0;

  arduinoOpticalTransmitter.sendPacket(&packet.bytes[0], sizeof(packet_t), 5); // send packet, inter-byte delay 5ms
  
  delay(5); // inter-packet delay 10ms
  digitalWrite(13, HIGH); // turn LED on
}
