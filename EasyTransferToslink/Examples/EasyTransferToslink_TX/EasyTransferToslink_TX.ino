#include <EasyTransferToslink.h>
#include <SoftwareSerial.h>

//create object
EasyTransferToslink ET;
SoftwareSerial swSerial(2, 3); // RX, TX // debug interface

uint8_t testData[10] = {0};

void setup()
{
  Serial.begin(1000000);
  ET.begin(details(testData), &Serial);
  swSerial.begin(9600); // init debug interface, 9600 bps
}

char c = 'a';

void loop()
{
  for (int i = 0; i < sizeof(testData); i++)
  {
    testData[i] = c;
    c++;
  }

  //send test packet
  ET.sendData(swSerial); //swSerial needed when debuging

  swSerial.println("data sent");
  swSerial.println();
  
  delay(500);
}
