/*
   @author    Nils Hцgberg
   @contact    nils.hogberg@gmail.com
    @coauthor(s):
     Victor Brutskiy, 4refr0nt@gmail.com, er9x adaptation

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Version 1.1.117 03.10.2014

#undef PROGMEM 
#define PROGMEM __attribute__(( section(".progmem.data") )) 

#undef PSTR 
#define PSTR(s) (__extension__({static prog_char __c[] PROGMEM = (s); &__c[0];})) 

#include <SoftwareSerial.h>
#include <FlexiTimer2.h>
#include <FastSerial.h>
#include "SimpleTelemetry.h"
#include "Mavlink.h"
#include "FrSky.h"
#include "SimpleFIFO.h"
#include <GCS_MAVLink.h>

//#define HEARTBEATLED 13
//#define HEARTBEATFREQ 100

// Do not enable both at the same time
//#define DEBUG
//#define DEBUGFRSKY

// Lighting
// Output pins   ( Note: 5,6,11,12 already used by other needs )
#define FRONT      7
#define REAR       8
#define LEFT       9
#define RIGHT     10
#define WHITE      2
#define RED        3
#define BLUE       4
#define GREEN     13
//static long p_preMillis;
//static long p_curMillis;
static int  pattern_index = 0;
static int  last_mode = 0;

char LEFT_STAB[]   PROGMEM = "1111111110";   // pattern for LEFT  light, mode - STAB
char RIGHT_STAB[]  PROGMEM = "1111111110";   // pattern for RIGHT light, mode - STAB
char FRONT_STAB[]  PROGMEM = "1111111110";   // pattern for FRONT light, mode - STAB
char REAR_STAB[]   PROGMEM = "1111111110";   // pattern for REAR  light, mode - STAB

char LEFT_AHOLD[]  PROGMEM = "111000";  // medium blink
char RIGHT_AHOLD[] PROGMEM = "111000"; 
char FRONT_AHOLD[] PROGMEM = "111000"; 
char REAR_AHOLD[]  PROGMEM = "111000"; 

char LEFT_RTL[]    PROGMEM = "10";  // fast blink
char RIGHT_RTL[]   PROGMEM = "10"; 
char FRONT_RTL[]   PROGMEM = "10"; 
char REAR_RTL[]    PROGMEM = "10"; 

char LEFT_OTHER[]  PROGMEM = "1";  // always ON
char RIGHT_OTHER[] PROGMEM = "1"; 
char FRONT_OTHER[] PROGMEM = "1"; 
char REAR_OTHER[]  PROGMEM = "1"; 

char *left[]  PROGMEM = {LEFT_STAB,  LEFT_AHOLD,  LEFT_RTL,  LEFT_OTHER };
char *right[] PROGMEM = {RIGHT_STAB, RIGHT_AHOLD, RIGHT_RTL, RIGHT_OTHER};
char *front[] PROGMEM = {FRONT_STAB, FRONT_AHOLD, FRONT_RTL, FRONT_OTHER};
char *rear[]  PROGMEM = {REAR_STAB,  REAR_AHOLD,  REAR_RTL,  REAR_OTHER};

// Comment this to run simple telemetry protocol
#define MAVLINKTELEMETRY

#ifdef MAVLINKTELEMETRY
Mavlink *dataProvider;
#else
SimpleTelemetry *dataProvider;
#endif

FastSerialPort0(Serial);
FrSky *frSky;
SoftwareSerial *frSkySerial;

#ifdef DEBUG
SoftwareSerial *debugSerial;
#elif defined DEBUGFRSKY
SoftwareSerial *frskyDebugSerial;
#endif

#define Q_BUFF 128
SimpleFIFO<char, Q_BUFF> queue;

int counter = 0;
unsigned long   rateRequestTimer = 0;
//unsigned long   hbMillis = 0;
//byte   hbState;

void setup() {

// Debug serial port pin 11 rx, 12 tx
#ifdef DEBUG
   debugSerial = new SoftwareSerial(11, 12);
   debugSerial->begin(38400);
   debugSerial->flush();
#elif defined DEBUGFRSKY
   frskyDebugSerial = new SoftwareSerial(11, 12);
   frskyDebugSerial->begin(38400);
#endif
   
   // FrSky data port pin 6 rx, 5 tx
   frSkySerial = new SoftwareSerial(6, 5, true);
   frSkySerial->begin(9600);
   // Incoming data from APM
   Serial.begin(57600);
   Serial.flush();
   
#ifdef DEBUG
   debugSerial->println("");
   debugSerial->println("Mavlink to FrSky converter start");
   debugSerial->println("Initializing...");
   debugSerial->print("Free ram: ");
   debugSerial->print(freeRam());
   debugSerial->println(" bytes");
#endif
#ifdef MAVLINKTELEMETRY
   dataProvider = new Mavlink(&Serial);
#else
   dataProvider = new SimpleTelemetry();
#endif 
   frSky = new FrSky();
#ifdef DEBUG
   debugSerial->println("Waiting for APM to boot...");
#endif
  // Initializing output pins
  pinMode(LEFT, OUTPUT);
  pinMode(RIGHT,OUTPUT);
  pinMode(FRONT,OUTPUT);
  pinMode(REAR, OUTPUT);
  pinMode(RED,  OUTPUT);
  pinMode(GREEN,OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(WHITE,OUTPUT);

   // Blink fast a couple of times to wait for the APM to boot
   for (int i = 0; i < 200; i++)
   {
      if (i % 2)
      {
         AllOn();
      }
      else
      {
         AllOff();
      }
      delay(50);
   }
#ifdef DEBUG
   debugSerial->println("");
   debugSerial->println("Starting Timer...");
#endif
   FlexiTimer2::set( 100, 1.0/1000, sendFrSkyData ); // call every 100 1ms "ticks"
   FlexiTimer2::start();
#ifdef DEBUG
   debugSerial->println("Initialization done.");
   debugSerial->print("Free ram after setup: ");
   debugSerial->print(freeRam());
   debugSerial->println(" bytes");
#endif

}

void loop() {

//    if(millis() - p_preMillis > LOOPTIME) {
      // save the last time you blinked the LED 
//      p_preMillis = millis();   
      // Update base lights
//    }


#ifdef MAVLINKTELEMETRY
   if( dataProvider->enable_mav_request || ((millis() - dataProvider->lastMAVBeat) > 5000) )
   {
     if(millis() - rateRequestTimer > 2000) {
         Serial.flush();
         dataProvider->reset();
         for(int n = 0; n < 3; n++)
         {
#ifdef DEBUG
            debugSerial->print("Making rate request. ");
            debugSerial->println(millis() - dataProvider->lastMAVBeat);
#endif
            dataProvider->makeRateRequest();
            delay(50);
         }
         dataProvider->enable_mav_request = 0;
         rateRequestTimer = millis();
     }
   }
#endif

   while (Serial.available() > 0)
   {
      char c = Serial.read();
      if (queue.count() > Q_BUFF)
      {
#ifdef DEBUG
         debugSerial->println("QUEUE IS FULL!");
		 queue.flush();
#endif
      }
      queue.enqueue(c);
   }

   processData();
   //updateHeartbeat();
}
/*
void updateHeartbeat()
{
   if(millis() - hbMillis > HEARTBEATFREQ) {
      hbMillis = millis();
      if (hbState == LOW)
      {
         hbState = HIGH;
      }
      else
      {
         hbState = LOW;
      }
      digitalWrite(HEARTBEATLED, hbState); 
   }
}
*/
void sendFrSkyData()
{
   updateLights(); // lights every 100ms
   counter++;
   if ((counter % 50) == 0)          // Send 5000 ms frame
   {
      counter = 0;
      frSky->sendFrSky05Hz(frSkySerial, dataProvider);
   } 
   if ((counter % 10) == 0) {    // Send 1000 ms frame
      frSky->sendFrSky1Hz(frSkySerial, dataProvider);
#ifdef DEBUG
      frSky->printValues(debugSerial, dataProvider);
#endif
      if ( dataProvider->msg_timer > 0 ) {
         dataProvider->msg_timer -= 1;  // counter -1 sec
      }
   }
   if ( (counter % 2) == 0 ) {
   // Send 200 ms frame
      frSky->sendFrSky5Hz(frSkySerial, dataProvider);
   }
}

void processData()
{  
   while (queue.count() > 0)
   { 
      int msg = dataProvider->parseMessage(queue.dequeue());
#ifdef DEBUG
      if (msg >= 0) {
	     dataProvider->printMessage(debugSerial, dataProvider, msg);
	  }
#endif
   }
}

int freeRam () {
   extern int __heap_start, *__brkval; 
   int v; 
   return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

// Switch all outputs ON
void AllOn() {
#ifdef DEBUG
   debugSerial->print(".");
#endif
    digitalWrite(LEFT,  1);
    digitalWrite(RIGHT, 1);
    digitalWrite(FRONT, 1);
    digitalWrite(REAR,  1);

    digitalWrite(RED,   1);
    digitalWrite(WHITE, 1);
    digitalWrite(BLUE,  1);
    digitalWrite(GREEN, 1);
}

// Switch all outputs OFF
void AllOff() {
#ifdef DEBUG
   debugSerial->print(".");
#endif
    digitalWrite(LEFT,  0);
    digitalWrite(RIGHT, 0);
    digitalWrite(FRONT, 0);
    digitalWrite(REAR,  0);

    digitalWrite(RED,   0);
    digitalWrite(WHITE, 0);
    digitalWrite(BLUE,  0);
    digitalWrite(GREEN, 0);
}
// Updating base leds state
void updateLights() {
   int pin, index;
   if ( dataProvider->motor_armed ) {
      digitalWrite(WHITE, 1);
   } else {
      digitalWrite(WHITE, 0);
   }
   if ( (dataProvider->gpsStatus >= 3) && ( dataProvider->gpsHdop <= 200 ) ) {
      digitalWrite(BLUE, 1);
   } else {
      digitalWrite(BLUE, 0);
   }
   if ( dataProvider->status_msg > 1) {
      digitalWrite(RED, 1);
   } else {
      digitalWrite(RED, 0);
   }
   if (last_mode != dataProvider->apmMode) {
      pattern_index = 0;
	  last_mode = dataProvider->apmMode;
   }
   switch(last_mode) {
   case 0: // STAB
       index = 0;
       break;
   case 2: // AltHold
       index = 1;
       break;
   case 6: // RTL
       index = 2;
       break;
   default:
       index = 3;
       break;
   }
   pin = check_pattern((char*)pgm_read_word(&(left[index])));
   digitalWrite(LEFT, pin);
#ifdef DEBUG
   debugSerial->print("LEFT LIGHT ");
   debugSerial->print(pin);
   debugSerial->print(" ");
   debugSerial->println(pattern_index);
#endif
   pin = check_pattern((char*)pgm_read_word(&(right[index])));
   digitalWrite(RIGHT, pin);
   pin = check_pattern((char*)pgm_read_word(&(front[index])));
   digitalWrite(FRONT, pin);
   digitalWrite(GREEN, pin); // green led some as front light
   pin = check_pattern((char*)pgm_read_word(&(rear[index])));
   digitalWrite(REAR, pin);
   pattern_index ++;
   if (eol((char*)pgm_read_word(&(left[index])))) pattern_index = 0;
}

int check_pattern(char *string)
{
    int value = 0;
    if ( pgm_read_byte( string + pattern_index ) == '1' )	{
	   value = 1;
	}
	return value;
}

bool eol(char *string)
{
    bool value = false;
    if ( pgm_read_byte( string + pattern_index ) == '\0' )	{
	   value = true;
	}
	return value;
}
