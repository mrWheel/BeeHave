
int32_t enterNumber(String prompt, int32_t var, int16_t varMin, int16_t varMax, int16_t step=1) {

bool    stillEntering = true;
bool    prevDoSet, doSet = false;
int32_t oldNumber, newNumber;
int8_t  swState, rState;
char    cNum[7];

    Serial.print(F("In enterNumber() ["));
    Serial.print(prompt); Serial.print(F("]: [")); Serial.print(var);
    Serial.print(F("], min[")); Serial.print(varMin);
    Serial.print(F("], max[")); Serial.print(varMax);
    Serial.println(F("]"));
    newNumber = var;
    oldNumber = var;
    dspl_Clear();
    dspl_Print(0,0, ">");
    dspl_Print(0,0,prompt.substring(0,9));                      // and erase the previously displayed cursor
    dspl_Print(10,0, "[    ]");
    dspl_Print(11,0,String(newNumber));
    //dspl_Print(0,                _TOTAL_ROWS, " Cancel");
    //dspl_Print((_TOTAL_COLS - 9),_TOTAL_ROWS, " Set");

    do {
        switch(readEncoder()) {  // analyze encoder response. Default is 0.
            case 'U':  // ENCODER ROTATED UP. EQUIVALENT OF 'UP' BUTTON PUSHED
                        newNumber += step;
                        if (newNumber > varMax)   newNumber = varMax;
                        pingTime = millis() + _PINGTIME;
                        break;
                        
            case 'D':  // ENCODER ROTATED UP. EQUIVALENT OF 'UP' BUTTON PUSHED
                        newNumber -= step;
                        if (newNumber < varMin)   newNumber = varMin;
                        pingTime = millis() + _PINGTIME;
                        break;
                        
            case 'S':  // ENCODER ROTATED UP. EQUIVALENT OF 'UP' BUTTON PUSHED
                        //swState = _NOKEYPRESSED;
                        doSet = confirm("Cancel", "Set");
                        menuTimeOut = millis() + _MENUTIMEOUT;  // reset timeout timer
                        pingTime = millis() + _PINGTIME;
                        stillEntering = false;
                        break;
                        
            case 'L':  // ENCODER ROTATED UP. EQUIVALENT OF 'UP' BUTTON PUSHED
                        stillEntering  = false;
                        doSet          = false;
                        Serial.println(F("Cancel pressed"));
                        dspl_Clear();  // clear the screen so we can paint the menu.
                        menuId = 0;
                        redrawMenu = _MOVE_LIST;
                        pingTime = millis() + _PINGTIME;
                        return -1;
                        break;
                        
        }   // switch(nKey)

        //dspl_Print(10,0, "[    ]");
        sprintf(cNum, "[%4d]", newNumber);
        /**
        dspl_Print(11,0, String(newNumber));
        if (newNumber < 1000)     dspl_Print(12,0," ");
        if (newNumber < 100)      dspl_Print(13,0," ");
        if (newNumber < 10)       dspl_Print(14,0," ");
        **/
        dspl_Print(10,0, cNum);
        if (oldNumber != newNumber) { 
            Serial.println(newNumber); 
            oldNumber = newNumber;
        }

        yield();
        
    } while (stillEntering);

    if (doSet) {
        //var = newNumber;
        return newNumber;
    }
    return var;
    
}   // enterNumber()

