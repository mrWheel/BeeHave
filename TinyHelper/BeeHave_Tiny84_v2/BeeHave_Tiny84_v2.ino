/*
    Program  : BeeHave_Tiny84_v2
    
    Copyright (C) 2017 Willem Aandewiel
    
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

#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include <TinyWireS.h>
#include "PinChangeInterrupt.h"


#define _TO_ESP_RST         2       // DIL-11, PA2, D2
#define SCL                 4       // DIL-9, PA4, D4
#define SDA                 6       // DIL-7, PA6, D6
#define _PCINT0_DIL13       13      // DIL-13 (PA0, D0)
#define _BUTTON             0       // PCINT0 = PA0, D0
#define _PCINT1_DIL12       12      // DIL-12 (PA1, D1)
#define _ESP_GPIO16         1       // PCINT1 = PA1, D1
#define _PCINT9_DIL3        9       // DIL-3 (PB1, D9) 
#define _ESP_GPIO13_DIL3    9       // PB1, D9, DIL-3 

#define _LED                10      // DIL-2 (PB0, D10)

#define I2C_SLAVE_ADDRESS   0x26    // the 7-bit address 
                                    // Get this from https://github.com/rambo/TinyWire
          

typedef struct {
    uint8_t     configVersion;
    uint32_t    internalRef;
    uint8_t     oscillatorCal;
}   configTiny;

volatile uint8_t        Type;
extern volatile uint8_t i2c_regs[16];
uint32_t                I2Ctimer, I2CtimerWait, ledBlink;
static  configTiny      EEPROM;
int32_t                 intVcc;
char                    charVcc[10];
bool                    doReadVcc;

// Converts a BCD (binary coded decimal) to decimal
byte bcdToDec(byte value) {
    return ((value / 16) * 10 + value % 16);
}   // bcdToDec()


// Converts a decimal to BCD (binary coded decimal)
byte decToBcd(byte value) {
    return (value / 10 * 16 + value % 10);
}   // decToBcd()


void sleepNow(){         
 
    i2c_regs[0] = decToBcd(0); // type of Event
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // sleep mode is set here
    sleep_enable();                         // enables the sleep bit in the mcucr register so sleep is possible
  
    attachPCINT(_BUTTON, buttonEvent, RISING);
    attachPCINT(_ESP_GPIO16, timerEvent, FALLING);

    sleep_mode();                           // here the device is actually put to sleep!!

}   // sleepNow()


void buttonEvent() {
    detachPCINT(_BUTTON);
    detachPCINT(_ESP_GPIO16);

    i2c_regs[0] = decToBcd(20); // type of Event
    wakeUpESP();

}   // buttonEvent()


void timerEvent() {
    detachPCINT(_ESP_GPIO16);
    detachPCINT(_BUTTON);

    i2c_regs[0] = decToBcd(10); // type of Event
    wakeUpESP();
        
}   // timerEvent()


void wakeUpESP() {
    pinMode(_TO_ESP_RST, OUTPUT);           // sets the digital pin as output
    digitalWrite(_LED, HIGH);

    doReadVcc = true;   // leesBatterij() in main();

    digitalWrite(_TO_ESP_RST,  HIGH);       // set ESP reset to HIGH
    delay(150);                             // wait 150 mSec
    digitalWrite(_TO_ESP_RST,  LOW);        // set ESP reset to LOW
    delay(50);                              // wait 50 mSec --> this should restart the ESP
    digitalWrite(_TO_ESP_RST,  HIGH);       // set ESP reset to HIGH again
    
}   // wakeUpESP();


void leesBatterij() {
    sprintf(charVcc, "%d", readVcc());
    for(int p=0; p<5, charVcc[p]; p++) {
        i2c_regs[p+1] = decToBcd(charVcc[p]);
    }
    
}   // leesBatterij()


void setup(){
    pinMode(_TO_ESP_RST, OUTPUT);           // sets the digital pin as output
    digitalWrite(_TO_ESP_RST, HIGH);           // sets the digital pin as output
    pinMode(_LED, OUTPUT);                  // sets the digital pin as output
    digitalWrite(_LED, LOW);
    
    doReadVcc   = false;
    regInit();
    loadConfig();
    leesBatterij();
    
    pinMode(_BUTTON, INPUT_PULLUP);
    pinMode(_ESP_GPIO16, INPUT_PULLUP);

    // attach the new PinChangeInterrupts and enable event functions below
    attachPCINT(_BUTTON, buttonEvent, RISING);
    attachPCINT(_ESP_GPIO16, timerEvent, FALLING);
    
}   // setup()


void loop(){
    attachPCINT(_BUTTON, buttonEvent, RISING);
    attachPCINT(_ESP_GPIO16, timerEvent, FALLING);

    if (digitalRead(_ESP_GPIO13_DIL3) == HIGH) {
        digitalWrite(_LED, LOW);
        sleepNow();                             // sleep function called here
   
        digitalWrite(_LED, HIGH);
    }
        /**
        * I2C setup
        * Reminder: taking care of pull-ups is the masters job
        */
    
    TinyWireS.begin(I2C_SLAVE_ADDRESS);
    TinyWireS.onReceive(receiveEvent);
    TinyWireS.onRequest(requestEvent);
    
    //if (decToBcd(i2c_regs[0]) == 10) {
        I2CtimerWait    =  5000; // vijf seconden
    //} else {
    //    I2CtimerWait    = 10000; // tien seconden
    //}
    I2Ctimer    = millis() + I2CtimerWait;
    ledBlink    = millis() + 500;
    
    while (I2Ctimer > millis()) {
        // handle I2C requests....
        if (doReadVcc) {
            leesBatterij();
            doReadVcc = false;
        }
        if (millis() > ledBlink) {
            if (digitalRead(_LED)) {
                digitalWrite(_LED, LOW);
                ledBlink = millis() + 1300;
            } else {
                digitalWrite(_LED, HIGH);
                ledBlink = millis() + 100;
            }
        }
    }
    TinyWireS_stop_check();

}   // loop()
