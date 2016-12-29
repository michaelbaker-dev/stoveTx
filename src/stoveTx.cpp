// Feather9x_TX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (transmitter)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_RX

#include <SPI.h>
#include <RH_RF95.h>
#include <Adafruit_MAX31856.h>
#include "Wire.h"
#include "Adafruit_BMP085.h"
#include <Adafruit_SleepyDog.h>

/* for feather32u4 */
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

/* for feather m0
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3
*/

/* for shield
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 7
*/


/* for ESP w/featherwing
#define RFM95_CS  2    // "E"
#define RFM95_RST 16   // "D"
#define RFM95_INT 15   // "B"
*/

/* Feather 32u4 w/wing
#define RFM95_RST     11   // "A"
#define RFM95_CS      10   // "B"
#define RFM95_INT     2    // "SDA" (only SDA/SCL/RX/TX have IRQ!)
*/

/* Feather m0 w/wing
#define RFM95_RST     11   // "A"
#define RFM95_CS      10   // "B"
#define RFM95_INT     6    // "D"
*/

/* Teensy 3.x w/wing
#define RFM95_RST     9   // "A"
#define RFM95_CS      10   // "B"
#define RFM95_INT     4    // "C"
*/

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

//used for getting battery level
#define VBATPIN A9

//LED Light
#define LED 13

// Temp Probe - Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31856 max = Adafruit_MAX31856(10, 11, 12, 13);
// use hardware SPI, just pass in the CS pin
//Adafruit_MAX31856 max = Adafruit_MAX31856(10);

//temp, humidity,pressure sensor
Adafruit_BMP085 bmp;

//Vars for displaying floats
float tc_val = 0;
char tc_str[5];

//Vars for displaying battery level
float batt_val = 0;
char batt_str[5];

//var for outside temp
char outTemp_str [5];

void setup()
{
 //while (!Serial);
 //Serial.begin(9600);
 //delay(1000);

 pinMode (LED, OUTPUT);
 digitalWrite(LED, LOW);

 //startup weather temp sensor
 bmp.begin();
 //startup thermopcoupler sensor
 max.begin();
 max.setThermocoupleType(MAX31856_TCTYPE_K);

 //check for thermocoupler type
 Serial.print("Thermocouple type: ");
 switch ( max.getThermocoupleType() ) {
   case MAX31856_TCTYPE_B: Serial.println("B Type"); break;
   case MAX31856_TCTYPE_E: Serial.println("E Type"); break;
   case MAX31856_TCTYPE_J: Serial.println("J Type"); break;
   case MAX31856_TCTYPE_K: Serial.println("K Type"); break;
   case MAX31856_TCTYPE_N: Serial.println("N Type"); break;
   case MAX31856_TCTYPE_R: Serial.println("R Type"); break;
   case MAX31856_TCTYPE_S: Serial.println("S Type"); break;
   case MAX31856_TCTYPE_T: Serial.println("T Type"); break;
   case MAX31856_VMODE_G8: Serial.println("Voltage x8 Gain mode"); break;
   case MAX31856_VMODE_G32: Serial.println("Voltage x8 Gain mode"); break;
   default: Serial.println("Unknown"); break;
 }

//set pins for RFM95
 pinMode(RFM95_RST, OUTPUT);
 digitalWrite(RFM95_RST, HIGH);

 // manual reset
 digitalWrite(RFM95_RST, LOW);
 delay(10);
 digitalWrite(RFM95_RST, HIGH);
 delay(10);

 while (!rf95.init()) {
   Serial.println("LoRa radio init failed");
   while (1);
 }
 Serial.println("LoRa radio init OK!");

 // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
 if (!rf95.setFrequency(RF95_FREQ)) {
   Serial.println("setFrequency failed");
   while (1);
 }
 Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

 // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

 // The default transmitter power is 13dBm, using PA_BOOST.
 // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
 // you can set transmitter powers from 5 to 23 dBm:
 rf95.setTxPower(23, false);
}

int16_t packetnum = 0;  // packet counter, we increment per xmission
char txBuffer[100];
int16_t stoveTemp = 0;

void gotoSleep()
{
 Serial.println("Radio Sleep");
 rf95.sleep();
 delay (10);

 // To enter low power sleep mode call Watchdog.sleep() like below
 // and the watchdog will allow low power sleep for as long as possible.
 // The actual amount of time spent in sleep will be returned (in
 // milliseconds).
 //int sleepMS = Watchdog.sleep();

 // Alternatively you can provide a millisecond value to specify
 // how long you'd like the chip to sleep, but the hardware only
 // supports a limited range of values so the actual sleep time might
 // be smaller.  The time spent in sleep will be returned (in
 // milliseconds).
 Serial.println("Going to sleep now....");
 delay (10);
 unsigned long sleepMS, totalMS = 0;
 int loops = 8;
 for ( int a = 0; a < loops; a++)
 {
   sleepMS = Watchdog.sleep(8000);  // Sleep for up to 8 second.
   delay (20);
   Serial.print("Slept for = ");
   delay (20);
   Serial.println(sleepMS);
   delay (20);
   Serial.print("Sleep # = ");
   delay (20);
   Serial.println(a);
   delay (20);
   totalMS = totalMS + sleepMS;
 }

 Serial.print("I'm awake now!  I slept for ");
 totalMS = (totalMS+500)/1000;
 Serial.print(totalMS);
 Serial.println(" Seconds.");
 Serial.println();
}

void loop()
{

//BMP180 Sensor readings
float outTemp_val = (1.8 * bmp.readTemperature()) + 32;
//new Calculate
float currentTemperatureC = bmp.readTemperature();
float currentTemperatureF = (9.0/5.0)*currentTemperatureC+32.0;

//dtostrf(outTemp_val,1,0, outTemp_str);
dtostrf(currentTemperatureF,1,0, outTemp_str);
/*
 Serial.print("Temperature = ");
 Serial.print(bmp.readTemperature());
 Serial.println(" *C");

 Serial.print("Pressure = ");
 Serial.print(bmp.readPressure());
 Serial.println(" Pa");

   // Calculate altitude assuming 'standard' barometric
   // pressure of 1013.25 millibar = 101325 Pascal
   Serial.print("Altitude = ");
   Serial.print(bmp.readAltitude());
   Serial.println(" meters");

 Serial.println();
 delay(500);
*/

//***************  THERMOCOUPLER
 tc_val = (1.8 * max.readThermocoupleTemperature() ) + 32;
 dtostrf(tc_val,1,0, tc_str);

 //Serial.print("Cold Junction Temp: "); Serial.println(max.readCJTemperature());
 //Serial.print("Thermocouple Temp: "); Serial.println(max.readThermocoupleTemperature());
 // Check and print any faults
 uint8_t fault = max.readFault();
 if (fault) {
   if (fault & MAX31856_FAULT_CJRANGE) Serial.println("Cold Junction Range Fault");
   if (fault & MAX31856_FAULT_TCRANGE) Serial.println("Thermocouple Range Fault");
   if (fault & MAX31856_FAULT_CJHIGH)  Serial.println("Cold Junction High Fault");
   if (fault & MAX31856_FAULT_CJLOW)   Serial.println("Cold Junction Low Fault");
   if (fault & MAX31856_FAULT_TCHIGH)  Serial.println("Thermocouple High Fault");
   if (fault & MAX31856_FAULT_TCLOW)   Serial.println("Thermocouple Low Fault");
   if (fault & MAX31856_FAULT_OVUV)    Serial.println("Over/Under Voltage Fault");
   if (fault & MAX31856_FAULT_OPEN)    Serial.println("Thermocouple Open Fault");
 }
 delay(10);

// READ BATTERY LEVEL
 float measuredvbat = analogRead(VBATPIN);
 measuredvbat *= 2;    // we divided by 2, so multiply back
 measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
 measuredvbat /= 1024; // convert to voltage
 //Serial.print("VBat: " ); Serial.println(measuredvbat);
 dtostrf(measuredvbat,1,2, batt_str);
 int bPercent = 0;

 if (measuredvbat > 3.05) bPercent = 0;
 if (measuredvbat > 3.1) bPercent = 10;
 if (measuredvbat > 3.4) bPercent = 20;
 if (measuredvbat > 3.5) bPercent = 40;
 if (measuredvbat > 3.7) bPercent = 60;
 if (measuredvbat > 3.8) bPercent = 80;
 if (measuredvbat > 4.1) bPercent = 100;

 //Serial.println("Sending to rf95_server");
 // Send a message to rf95_server

//sending data in this order..
 sprintf (txBuffer,"%s,%s,%s,%i\0", tc_str,outTemp_str,batt_str,bPercent );
 Serial.println(txBuffer);
 digitalWrite(LED, HIGH);
 rf95.send((uint8_t *)txBuffer, sizeof(txBuffer));

 //wait for write to compete
 Serial.println("Waiting for packet to complete..."); delay(10);
 rf95.waitPacketSent();
 // Now wait for a reply
 uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
 uint8_t len = sizeof(buf);

 Serial.println("Waiting for reply..."); delay(10);
 if (rf95.waitAvailableTimeout(3000))
 {
   // Should be a reply message for us now
   if (rf95.recv(buf, &len))
  {
     Serial.print("Got reply: ");
     Serial.print((char*)buf);
     Serial.print("  -- RSSI: ");
     Serial.println(rf95.lastRssi(), DEC);
   }
   else
   {
     Serial.println("Receive failed");
   }
 }
 else
 {
   Serial.println("No reply, is there a listener around?");
 }
 digitalWrite (LED, LOW);
 gotoSleep();
 delay(100);
}
