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

void loop()
{
  char c = 'a';
  
  for (int i = 0; i < sizeof(testData); i++)
  {
    testData[i] = c;
    c++;
  }

  //send test packet
  ET.sendData(swSerial); //swSerial needed when debuging

  swSerial.println("data sent");
  swSerial.println();

  delay(1000);
  
  //check and see if a data packet has come in.
  while (false == ET.receiveData(swSerial)) //swSerial needed when debuging
  {
    swSerial.println();
    if (ET.receiveFailed) // if reception error
    {
      swSerial.println("receive failed");
      swSerial.println();
      return;
    }
  }

  for (int i = 0; i < sizeof(testData); i++)
  {
    swSerial.println();
    swSerial.print("received: "); swSerial.println((char)testData[i]);
  }
}
