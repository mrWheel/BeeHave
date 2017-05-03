


void sendATTinyReadVcc() {

    uint8_t L = 0;

    do {
        L++;
        Wire.beginTransmission(_ATTINY_ADDR);
        Wire.write(0x0E);             // Send command
        Wire.write(decToBcd(12));     // .. "readVcc()"
        result = Wire.endTransmission();

        if (result == 0) {
            readATTiny();
            return;
        }
    } while ((result != 0) && (L < 4));
    
    if (L >= 4) {
        dPrint("sendATTinyReadVcc(): error ");
        dPrintln(String(result, DEC));
        dPrintln("<ERROR>");
        dFlush();
        wakeUpType = 0;
        memset(float2Char,0,sizeof(float2Char));
    }

}   // sendATTinyReadVcc()


void readATTiny() {

int L;

    wakeUpType  = 0;
    L           = 0;
    
    do {
        L++;
        memset(float2Char,0,sizeof(float2Char));

        Wire.beginTransmission(_ATTINY_ADDR);
        Wire.write(0); // Set start at reg[0] (= wakeUpType)
        result = Wire.endTransmission();
        if (result == 0) {
            Wire.requestFrom(_ATTINY_ADDR,1);           // Request reg[0] from slave
            wakeUpType = bcdToDec(Wire.read());         // get wakeUpType from ATtiny84
            //dPrintf("[%d] WakeUp type is [%d]\n", L, wakeUpType);
    
            for (int b=0; b<4; b++) {                   // Request 4 byte from slave
                Wire.requestFrom(_ATTINY_ADDR,1);       // reg[1,2,3,4] = Vcc
                float2Char[b] = bcdToDec(Wire.read());
            }
        
            actKast.Vcc = (float)((String(float2Char).toInt()) / 1000.0);
            //dtostrf(actKast.Vcc, 5, 3, float2Char);
            //if (actKast.Vcc != 0.0) dPrintf("[%d] Battery voltage is [%s]V\n", L, float2Char);

        } else {
            wakeUpType = 0;
            memset(float2Char,0,sizeof(float2Char));
            delay(100);
        }
        
    } while ((wakeUpType == 0 || actKast.Vcc < 2.0) && (L < 4));

    if (wakeUpType == 0) {
        dPrint("readATTiny(): error ");
        dPrintln(String(result, DEC));
        dPrintln("<ERROR>");
    } else {
        dPrintf("[%d] WakeUp type is [%d]\n", L, wakeUpType);
        dtostrf(actKast.Vcc, 5, 3, float2Char);
        if (actKast.Vcc != 0.0) dPrintf("[%d] Battery voltage is [%s]V\n", L, float2Char);
    }
    dFlush();
    
}   // readATTiny()

