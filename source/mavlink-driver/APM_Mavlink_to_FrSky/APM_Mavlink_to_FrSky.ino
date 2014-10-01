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

// Version 1.1.114

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

#define HEARTBEATLED 13
#define HEARTBEATFREQ 100

// Do not enable both at the same time
#define DEBUG
//#define DEBUGFRSKY

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

int      counter = 0;
unsigned long   hbMillis = 0;
unsigned long   rateRequestTimer = 0;
byte   hbState;

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
   digitalWrite(HEARTBEATLED, HIGH);
   hbState = HIGH;
#ifdef MAVLINKTELEMETRY
   dataProvider = new Mavlink(&Serial);
#else
   dataProvider = new SimpleTelemetry();
#endif 
   frSky = new FrSky();
#ifdef DEBUG
   debugSerial->println("Waiting for APM to boot...");
#endif
   // Blink fast a couple of times to wait for the APM to boot
   for (int i = 0; i < 200; i++)
   {
      if (i % 2)
      {
#ifdef DEBUG
   debugSerial->print(".");
#endif
         digitalWrite(HEARTBEATLED, HIGH);
         hbState = HIGH;
      }
      else
      {
         digitalWrite(HEARTBEATLED, LOW);
         hbState = LOW;
      }
      delay(50);
   }
#ifdef DEBUG
   debugSerial->println("");
   debugSerial->println("Starting Timer...");
#endif
   FlexiTimer2::set(200, 1.0/1000, sendFrSkyData); // call every 200 1ms "ticks"
   FlexiTimer2::start();
#ifdef DEBUG
   debugSerial->println("Initialization done.");
   debugSerial->print("Free ram after setup: ");
   debugSerial->print(freeRam());
   debugSerial->println(" bytes");
#endif

}

void loop() {

#ifdef MAVLINKTELEMETRY
   if( dataProvider->enable_mav_request || ((millis() - dataProvider->lastMAVBeat) > 3000) )
   {
         Serial.flush();
		 queue.flush();
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
         delay(1000);
         dataProvider->reset();
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
   updateHeartbeat();
}

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

void sendFrSkyData()
{
   counter++;
   if ((counter % 25) == 0)          // Send 5000 ms frame
   {
      counter = 0;
      frSky->sendFrSky05Hz(frSkySerial, dataProvider);
   } 
   if ((counter % 5) == 0) {    // Send 1000 ms frame
      frSky->sendFrSky1Hz(frSkySerial, dataProvider);
#ifdef DEBUG
      frSky->printValues(debugSerial, dataProvider);
#endif
      if ( dataProvider->msg_timer > 0 ) {
         dataProvider->msg_timer -= 1;  // counter -1 sec
      }
   } else {
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

