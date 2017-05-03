#define _CONFIGVERSION   1
#define _EEPROM_ADDR     0
#define _INTERNALREF     1125300L    // default

static void loadConfig() {
    eeprom_read_block(&EEPROM, _EEPROM_ADDR, sizeof EEPROM);
    if (EEPROM.configVersion   != _CONFIGVERSION) {
        EEPROM.internalRef      = _INTERNALREF;
        EEPROM.oscillatorCal    = 1;
    }
}   // loadConfig()

