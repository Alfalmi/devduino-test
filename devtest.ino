/*
This sketch is for a devDuino SN v2.2 with the support OTA http://www.seeedstudio.com/wiki/DevDuino_Sensor_Node_V2.2_(ATmega_328)
and MySensors 1.5
 
For testing all the equipment, press the button D4 when a voltage is applied.
 
This sketch is a modification of code written
Version 1.3 - Thomas Bowman Mørch
for sensor Sensebender Micro
http://www.mysensors.org/hardware/micro
 
 modified
 18 December 2015
 by greengo
 */
 
#include <MySensor.h>
#include <SPI.h>
#include "utility/SPIFlash.h"
#include <EEPROM.h>  
#include <sha204_lib_return_codes.h>
#include <sha204_library.h>
 
// Define a static node address, remove if you want auto address assignment
//#define NODE_ADDRESS   2
 
#define RELEASE "0.01"
 
#define TEST_PIN          4 // Button D4
#define TEMP_SENSE_PIN   A3 // Input pin for the Temp sensor MCP9700
#define LED_PIN           9  // LED 
#define ATSHA204_PIN     A2 // ATSHA204A 
 
const int sha204Pin = ATSHA204_PIN;
atsha204Class sha204(sha204Pin);
 
SPIFlash flash(8, 0x1F65);
 
MyTransportNRF24 transport(7, 6);
MySensor gw(transport); 
 
// the setup function runs once when you press reset or power the board
void setup() {
 
  // initialize digital pin 9 as an output.
  pinMode(LED_PIN, OUTPUT); 
 
  Serial.begin(9600);
  Serial.print(F("devDuino SNv2.2"));
  Serial.println(RELEASE);
  Serial.flush(); 
 
 // First check if we should boot into test mode
 
  pinMode(TEST_PIN,INPUT);
  digitalWrite(TEST_PIN, HIGH); // Enable pullup
  if (!digitalRead(TEST_PIN)) testMode();
 
  // Make sure that ATSHA204 is not floating
  pinMode(ATSHA204_PIN, INPUT);
  digitalWrite(ATSHA204_PIN, HIGH);
 
  digitalWrite(LED_PIN, HIGH); 
 
#ifdef NODE_ADDRESS
  gw.begin(NULL, NODE_ADDRESS, false);
#else
  gw.begin(NULL,AUTO,false);
#endif  
 
  digitalWrite(LED_PIN, LOW);
 
  Serial.flush();
  Serial.println(F(" - Online!"));
 
  gw.sendSketchInfo("devDuino SNv2.2", RELEASE);   
 
}
 
// the loop function runs over and over again forever
void loop() {
 
  gw.process();
 
}
 
/****************************************************
 *
 * Verify all peripherals, and signal via the LED if any problems.
 *
 ****************************************************/
void testMode()
{
  uint8_t rx_buffer[SHA204_RSP_SIZE_MAX];
  uint8_t ret_code;
  byte tests = 0;
 
  digitalWrite(LED_PIN, HIGH); // Turn on LED.
  Serial.println(F(" - TestMode"));
  Serial.println(F("Testing peripherals!"));
  Serial.flush();
  Serial.print(F("-> MCP9700 : ")); 
  Serial.flush();
 
  if (analogRead (TEMP_SENSE_PIN),HIGH ) 
  {
    Serial.println(F("ok!"));
    tests ++;
  }
  else
  {
    Serial.println(F("failed!"));
  }
  Serial.flush();
 
  Serial.print(F("-> Flash : "));
  Serial.flush();
  if (flash.initialize())
  {
    Serial.println(F("ok!"));
    tests ++;
  }
  else
  {
    Serial.println(F("failed!"));
  }
  Serial.flush();
 
 
  Serial.print(F("-> SHA204 : "));
  ret_code = sha204.sha204c_wakeup(rx_buffer);
  Serial.flush();
  if (ret_code != SHA204_SUCCESS)
  {
    Serial.print(F("Failed to wake device. Response: ")); Serial.println(ret_code, HEX);
  }
  Serial.flush();
  if (ret_code == SHA204_SUCCESS)
  {
    ret_code = sha204.getSerialNumber(rx_buffer);
    if (ret_code != SHA204_SUCCESS)
    {
      Serial.print(F("Failed to obtain device serial number. Response: ")); Serial.println(ret_code, HEX);
    }
    else
    {
      Serial.print(F("Ok (serial : "));
      for (int i=0; i<9; i++)
      {
        if (rx_buffer[i] < 0x10)
        {
          Serial.print('0'); // Because Serial.print does not 0-pad HEX
        }
        Serial.print(rx_buffer[i], HEX);
      }
      Serial.println(")");
      tests ++;
    }
 
  }
  Serial.flush();
 
  Serial.println(F("Test finished"));
 
  if (tests == 3) 
  {
    Serial.println(F("Selftest ok!"));
    while (1) // Blink OK pattern!
    {
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
      delay(200);
    }
  }
  else 
  {
    Serial.println(F("----> Selftest failed!"));
    while (1) // Blink FAILED pattern! Rappidly blinking..
    {
    }
  }  
}
