//#include <SoftwareSerial.h>

typedef struct
{
  char c;
  int i;
  long l;
  float f;
  double d;
} packet_t;

//SoftwareSerial swSerial(2, 3); // RX, TX // debug interface
packet_t packet; // packet to be transmitted
char sequenceNumber = 0; // current packet sequence number

char calculateChecksum(const char* bytes)
{
  char checksum = bytes[0];
  
  for(long indxByte = 1; indxByte < sizeof(packet_t) + 4; indxByte++)
  {
    checksum ^= bytes[indxByte];
  }

  return checksum;
}

void setup()
{
  Serial.begin(1000000); // init optical transmission, 1 Mbps
  //swSerial.begin(9600); // init debug interface, 9600 bps

  packet.c = 42; // prepare new packet to be sent
  packet.i = 2;
  packet.l = 3L;
  packet.f = 4.0f;
  packet.d = 5.0;

  pinMode(13, OUTPUT); // enable LED on pin 13, LED is blinking if packets being transmitted
}

void loop()
{
  delay(1000);
  digitalWrite(13, LOW); // turn LED off

  packet.c ++; // prepare new packet to be sent
  packet.i ++;
  packet.l ++;
  packet.f += 1.0f;
  packet.d += 1.0;

  char transmittBuffer[sizeof(packet_t) + 4] = {0}; // create empty transmitt buffer

  // fill transmitt buffer with data
  transmittBuffer[0] = 'h'; // add Start of Header delimiter
  transmittBuffer[1] = sequenceNumber; // add current packet sequence number
  transmittBuffer[2] = 's'; // add Start of Text delimiter
  memcpy(&transmittBuffer[3], &packet, sizeof(packet_t)); // serialize packet
  transmittBuffer[sizeof(packet_t) - 1 + 4] = 'e'; // add End of Text delimiter
  
  char checksum = calculateChecksum(transmittBuffer); // calculate packet checksumm

  for(long i = 0; i < sizeof(packet_t) + 4; i++) // send data from transmitt buffer
  {
    // perform byte stuffing
    if(transmittBuffer[i] == 'h' || transmittBuffer[i] == 's' || transmittBuffer[i] == 'e' || transmittBuffer[i] == 'd') 
    {
      Serial.print('d');
      delay(100);
    }
    Serial.print(transmittBuffer[i]); // send next byte
    delay(100);
  }

  // send checksum
  if(checksum == 'h' || checksum == 's' || checksum == 'e' || checksum == 'd') // perform byte stuffing
  {
    Serial.print('d');
    delay(100);
  }
  Serial.print(checksum); // send checksumm
  delay(100);

  if (sequenceNumber < 127) sequenceNumber++; // increase packet sequence number
  else sequenceNumber = 0; // reset packet sequence number
  
  //delay(1000);
  digitalWrite(13, HIGH); // turn LED on
}
