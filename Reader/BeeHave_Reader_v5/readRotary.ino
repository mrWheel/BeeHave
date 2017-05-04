
#include "BeeHave_Reader.h"

int32_t   oldPosition = 0;
int32_t   rotaryDelay = 0;

/* Read input pins and process for events. Call this either from a
 * loop or an interrupt (eg pin change or timer).
 *
 * Returns 0 on no event, otherwise 0x80 or 0x40 depending on the direction.
 */
char readEncoder() {
    
    int8_t newPosition = myEnc.read();

    int  swState = readSwitch(ROTARY_SWITCH);

    if (swState == _SHORTPRESS)     { rotaryDelay = millis() + 150; return 'S'; }
    else if (swState == _LONGPRESS) { rotaryDelay = millis() + 150; return 'L'; }

    if (newPosition > oldPosition && millis() > rotaryDelay) {
        rotaryDelay = millis() + 200;
        oldPosition = newPosition;
        return 'U';
    } else if (newPosition < oldPosition && millis() > rotaryDelay) {
        rotaryDelay = millis() + 200;
        oldPosition = newPosition;
        return 'D';
    }
    oldPosition = newPosition;
    return '-';
    
}   // readEncoder()
/*
 * readSwitch2()
 */
long buttonTimer         = 0;

boolean buttonActive     = false;
boolean longPressActive  = false;
boolean shortPressActive = false;

 
uint8_t readSwitch(int switchPin) {

    if (digitalRead(switchPin) == LOW) {
        //Serial.print("Switch is LOW ");
        if (buttonActive == false) {
            buttonActive = true;
            buttonTimer = millis();
        }
        while (digitalRead(switchPin) == LOW) {
            if ((millis() - buttonTimer > _LONGPRESSTIME)  
                && (longPressActive == false)) {
                longPressActive = true;
                shortPressActive = false;
                while (digitalRead(switchPin) == LOW) {yield();}
            }
            else if ((millis() - buttonTimer > _SHORTPRESSTIME) 
                && (shortPressActive == false)) {
                shortPressActive = true;
            }
            yield();
        }   // while switch = LOW
    } else {
        if (buttonActive == true) {
            shortPressActive    = false;
            longPressActive     = false;
            buttonActive        = false;
        } 
    }

    if (longPressActive)    {longPressActive = false; Serial.println("LONG");  return _LONGPRESS;  }
    if (shortPressActive)   {shortPressActive = false; Serial.println("SHORT"); return _SHORTPRESS; }
    return _NOKEYPRESSED;

}   // readSwitch()

