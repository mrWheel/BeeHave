/*
 * Lezen en schrijven naar de RTC module
*/

bool writeRTC(structKast newKast) {

    RTC.adjust(DateTime(newKast.year, newKast.month, newKast.day, newKast.hour, newKast.minute, newKast.second));

}  //  writeRTC()
 

structKast readRTC(structKast oldKast) {

    structKast newKast;
    uint8_t L = 0;
    DateTime now;
    
    newKast = oldKast;
    do {
        if (L > 0) delay(200);
        L++;
        now = RTC.now();
    } while ((   (now.hour() < 0 || now.hour() > 24) 
              || (now.minute() < 0 || now.minute() > 60)
              || (now.month() == 0) || now.day() == 0) 
              && (L < 4));

    if (L < 4) {
        newKast.second  = now.second();
        newKast.minute  = now.minute();
        newKast.hour    = now.hour();
        newKast.weekday = 0;        // who cares..
        newKast.day     = now.day();
        newKast.month   = now.month();
        newKast.year    = now.year();
    } else { // if (RTC.lostPower()) {
        dPrintln("RTC lost power, set to compile time!");
        // following line sets the RTC to the date & time this sketch was compiled
        RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    return newKast;
    
 }  //  readRTC()


/*----------------------------------------------------------------------*
 * Returns the temperature in Celsius                                   *
 *----------------------------------------------------------------------*/
float getDS3231Temp() {

    byte T0, T1;
    int16_t temp;

    read3231(0x12, &T0, 1);  // #define TEMP_LSB    0x12
    read3231(0x11, &T1, 1);  // #define TEMP_MSB    0x11
    temp = T1 << 8;
    temp |= T0;

    return (temp / 256.0);
    
}   // getDS3231Temp()


byte read3231(byte addr, byte *values, byte nBytes) {
    Wire.beginTransmission(_I2C_RTC);
    Wire.write(addr);
    if ( byte e = Wire.endTransmission() ) return e;
    Wire.requestFrom( (uint8_t)_I2C_RTC, nBytes );
    for (byte i=0; i<nBytes; i++) values[i] = Wire.read();
    return 0;
    
}   // read3231()
