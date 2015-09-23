#include <EasyTransferToslink.h>
#include <SoftwareSerial.h>

//create object
EasyTransferToslink ET;
SoftwareSerial swSerial(2, 3); // RX, TX // debug interface

uint8_t testData[16] = {0};
bool receiving = true;
uint32_t numOfTrys = 0;
uint32_t timeout = 0;
uint32_t time_delay1 = 200; // time master waits for new packet
uint32_t time_delay2 = 1000; // receiver timeout

void setup()
{
  Serial.begin(1000000);
  ET.begin(details(testData), &Serial);
  swSerial.begin(115200); // init debug interface

  testData[0] = 0; //sequence number
  testData[1] = 0; //number of measurements

  swSerial.println("initialized");
}

void loop()
{
  if (receiving)
  {
    //swSerial.println("state: slave");

    if (millis() > timeout) // if timeout occured
    {
      swSerial.println("receiver timed out");
      //signal from master timed out
      testData[0] = 0; //reset sequence number

      tx_measurements();
      timeout = millis() + time_delay1; //delay till sending new packet;
      //return to master state
      receiving = false;
      return;
    }

    // check and see if a testData packet has come in
    while (false == ET.receiveData(swSerial)) // swSerial needed when debuging
    {
      //swSerial.println();
      if (ET.receiveFailed) // if reception error try again
      {
        //        swSerial.println("receive failed");
        //        swSerial.println();
        return;
      }
    }

    // if packet received successfuly
    swSerial.println("testData received");

    swSerial.print("rx");
    swSerial.print(" s ");
    swSerial.print(testData[0]);
    swSerial.print(" n ");
    swSerial.print(testData[1]);

    for (int i = 0; i < testData[1]; i++)
    {
      swSerial.print(" ");
      swSerial.print(testData[i + 2]);
    }

    swSerial.println();

    tx_measurements();
    timeout = millis() + time_delay1; //delay till sending new packet;
    //return to master state
    receiving = false;

    return;
  }
  else
  {
    swSerial.println("state: master");
    swSerial.println("waiting for packet to arrive");

    if (millis() > timeout) // if no packets received, timeout occured, send new packet
    {
      swSerial.println("no packet received timeout occured, sending new packet");
      testData[0]++; //increment sequence number
      testData[1] = 0; //number of measurements
      tx_measurements();
      timeout = millis() + time_delay1; //delay till sending new packet;
    }

    if (Serial.available()) // if new packet arrived
    {
      swSerial.println("packet arrived, returning to slave state");
      //return to slave state
      receiving = true;
      timeout = millis() + time_delay2; //receiver timeout value
      swSerial.println("state: slave");
      return;
    }
  }
}

void tx_measurements()
{
  if (testData[1] == 14) testData[1] = 0;
  testData[1] = testData[1] + 1; //number of measurements

  uint16_t voltage = analogRead(A1) / 2;
  if (voltage > 255) voltage = 255;

  testData[testData[1] + 1] = (uint8_t)(voltage); //drop two low bytes

  ET.sendData(swSerial); //swSerial needed when debuging library

  swSerial.print("tx");
  swSerial.print(" s ");
  swSerial.print(testData[0]);
  swSerial.print(" n ");
  swSerial.print(testData[1]);

  for (int i = 0; i < testData[1]; i++)
  {
    swSerial.print(" ");
    swSerial.print(testData[i + 2]);
  }
  swSerial.println();
}

