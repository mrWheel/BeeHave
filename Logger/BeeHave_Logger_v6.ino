/*
    Program  : BeeHave_Logger_v6
    
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

#include <ESP.h>
#include <ESP8266WiFi.h>
#include <Wire.h> //I2C header file
#include <RTClib.h>
//#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <FS.h>

#define _HAS_WIFI           true
#define _DEBUG              true

#if _DEBUG == false 
    #define dBegin(...)   
    #define dPrint(...)   
    #define dPrintln(...) 
    #define SdFlush(...)    
#else
    #define dBegin(...)     {Serial.begin(__VA_ARGS__); }
    #define dPrint(...)     {Serial.print(__VA_ARGS__); }
    #define dPrintln(...)   {Serial.println(__VA_ARGS__); }
    #define dPrintf(...)    {Serial.printf(__VA_ARGS__); }
    #define dFlush(...)     {Serial.flush(__VA_ARGS__); }
#endif

#if _HAS_WIFI == true
    #define commPort        client
#else
    #define commPort        Serial
#endif

// Defines
#define _SDA                4
#define _SCL                5
#define _ATTINY_ADDR        0x26         // 7 bit I2C address for ATTiny84 wakeUp master
#define _TIMEOUT            60000        // 60 seconden?
#define _SLOW_PRINT_DELAY   1
#define _SLOW_PRINTLN_DELAY 5
#define _MAX_SERIAL_BUF     15
#define _I2C_RTC            0x68        // 7 bit address (without last bit - look at the datasheet)
#define ATTINY_KEEP_AWAKE   13          // GPIO13
#define POWER_DEVICES       15          // GPIO15
#define POWER_ON            HIGH        // Pin HIGH is POWER ON
#define POWER_OFF           LOW         // Pin LOW is POWER OFF
#define DS18B20_PIN         0           // GPIO00
/***
//#define DHTTYPE             DHT11       // DHT 11
//#define DHTTYPE             DHT21       // DHT 21 (AM2301)
#define DHTTYPE             DHT22       // DHT 22  (AM2302), AM2321
// DHT Sensor
#define DHT_PIN             0
***/
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(DS18B20_PIN);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature DS18B20(&oneWire);


// Errors
#define ERROR_Rtc_SET       1           // Unable to set Rtc time and date
#define ERROR_Rtc_GET       2           // Unable to get Rtc time and date

enum { UNKNOWN_STATE, COMM_STATE, LOG_STATE, SLEEP_STATE };

struct structKast {
    uint8_t     kastID;
    uint16_t    interval;
    byte        second;
    byte        minute;
    byte        hour;
    byte        weekday;
    byte        day;
    byte        month;
    uint16_t    year;
    float       tempIn;
    float       tempOut;
    float       Vcc;
};   // structKast

// Global variables
structKast   actKast;

char* statusWL[]={
    "WL_IDLE_STATUS",       //  0
    "WL_NO_SSID_AVAIL",     //  1
    "WL_SCAN_COMPLETED",    //  2
    "WL_CONNECTED",         //  3
    "WL_CONNECT_FAILED",    //  4
    "WL_CONNECTION_LOST",   //  5
    "WL_DISCONNECTED",      //  6
    "\0\0\0\0"
};


byte        result;
byte        second_old; // The code ask the Rtc for data only when the previous value has changed
byte        minute_old; // The code ask the Rtc for data only when the previous value has changed
byte        hour_old; // The code ask the Rtc for data only when the previous value has changed
char*       weekdayname[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
uint32_t    keepCommAlive;
uint32_t    sendTime;
uint8_t     state;
uint32_t    sleepTime;
float       tempInstrt = 0.0;
char        debugLine[100];
uint8_t     serialBufCount;
bool        commState;
uint8_t     wakeUpType;
char        float2Char[10];

/***
// Initialize DHT sensor.
DHT dht(DHT_PIN, DHTTYPE);
***/
// DS3231 clock module
RTC_DS3231  RTC;
// WiFi stuff ------------
// Use WiFiClient class to create TCP connections
WiFiClient client;

const uint16_t port = 8266;
const char * host = "192.168.4.1"; // ip or dns

// Temporary variables
static char celsiusTemp[8];
static char fahrenheitTemp[8];
static char humidityTemp[8];

// Function prototypes
byte bcdToDec(byte);
byte decToBcd(byte);


void compileInfo() {
    String progName    = String(__FILE__);

    dFlush();
    dPrint("\nProgram [");
    dPrint(progName.substring(progName.lastIndexOf('/')+1));
    dPrint("] compiled at [");
    dPrint(__DATE__);
    dPrint(" ");
    dPrint(__TIME__);
    dPrintln("]");
    dFlush();
    
}   // compileInfo()


void print2Digits(int number) {
    if (number >= 0 && number < 10) {
        commPort.print("0");
    }
    commPort.print(String(number));
    
}   // print2Digits()


// Converts a BCD (binary coded decimal) to decimal
byte bcdToDec(byte value) {
    return ((value / 16) * 10 + value % 16);
}   // bcdToDec()


// Converts a decimal to BCD (binary coded decimal)
byte decToBcd(byte value) {
    return (value / 10 * 16 + value % 10);
}   // decToBcd()


//------------------------------------------------------
// read the logFile
//------------------------------------------------------
void readLogFile()
{
    char        charF;
    char        SDbuffer[100];
    charF       = '\0';
    uint8_t     bSize;

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile = SPIFFS.open("/Data/LogData.cvs", "r");

    commPort.println("<DATA>");
    dPrintln("<DATA>");

    bSize = 0;
    memset(SDbuffer,0,sizeof(SDbuffer));

    while (dataFile.available()) {
        charF = (char)dataFile.read();
        if (charF == '\n' || charF == '\r' || charF == '\0' || bSize > 98) {
            if (bSize > 0) {
                //SDbuffer[bSize++] = '\0';
                commPort.println(SDbuffer);
                dPrint("send> [");
                dPrint(SDbuffer);
                dPrintln("]");
                //delay(100);
            }
            bSize = 0;
            memset(SDbuffer,0,sizeof(SDbuffer));
        }
        else {
            if (charF >= 32 && charF <= 126) {
                SDbuffer[bSize++] = (char)charF;
            }
        }
    }

    dPrintln("<END>");
    commPort.println("\n<END>");
    //SPIFFS.end();

}   // readLogFile()


bool writeLogFile(structKast logKast, char lType) {

char tempIn_str[8];
char tempOut_str[8];
char Vcc_str[7];
char logLine[80];
bool newLogFile;

    if (logKast.year == 0 && logKast.day == 0) return false;
    if (logKast.tempIn < -90.0) {
        debug2LogFile('D', "Foute Temperatuur waarde");
        return false;
    }
    
    //SPIFFS.begin();
    if (SPIFFS.exists("/Data/LogData.cvs")) {
        newLogFile = false;
    } else {
        newLogFile = true;
    }

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile = SPIFFS.open("/Data/LogData.cvs", "a");

    if (newLogFile) {
        dataFile.println("\"Type\"; \"DateTime\";\"Batt.\";\"TempIn\";\"TempOut\"\n");
    }

    dtostrf(logKast.tempIn,  4, 2, tempIn_str);
    dtostrf(logKast.tempOut, 4, 2, tempOut_str);
    dtostrf(logKast.Vcc, 5, 3, Vcc_str);
    sprintf(logLine, "%c; %04d-%02d-%02d %02d:%02d; %s; %s; %s;"
                                , lType
                                , logKast.year
                                , logKast.month
                                , logKast.day
                                , logKast.hour
                                , logKast.minute
                                , Vcc_str
                                , tempIn_str
                                , tempOut_str
            );
    commPort.println(String(logLine));
    // if the file is available, write to it:
    if (dataFile) {
        dataFile.println(logLine);
        dataFile.close();
        return true;
    }
    // if the file isn't open, pop up an error:
    else {
        commPort.println("ERROR OPENING DATALOG.CSV");
        return false;
    }

}   // writeLogFile()


bool debug2LogFile(char lType, String dLine) {

char logLine[80];
bool newLogFile;

    if (actKast.year == 0 && actKast.day == 0) return false;

    if (SPIFFS.exists("/Data/LogData.cvs")) {
        newLogFile = false;
    } else {
        newLogFile = true;
    }
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile = SPIFFS.open("/Data/LogData.cvs", "a");

    if (newLogFile) {
        dataFile.println("\"Type\"; \"DateTime\";\"Batt.\";\"TempIn\";\"TempOut\"\n");
    }

    dLine.substring(0,38);
    // if the file is available, write to it:
    if (dataFile) {
        //-----------------1    5 6  8 9 11    16   19  59
        sprintf(logLine, "%c; %04d-%02d-%02d %02d:%02d; \"%s\"; "
                                , lType
                                , actKast.year
                                , actKast.month
                                , actKast.day
                                , actKast.hour
                                , actKast.minute
                                , dLine.c_str()
               );
        dataFile.println(logLine);
        dataFile.close();
        commPort.println(logLine);
    } else {  // if the file isn't open, pop up an error:
        commPort.println("ERROR OPENING DATALOG.CSV");
        //SPIFFS.end();
        return false;
    }
    //SPIFFS.end();

}   // debug2LogFile()

//------------------------------------------------------
// read the kastFile
//------------------------------------------------------
structKast readKastFile(structKast actKast) {

    structKast  newKast;
    char        charF;

    newKast = actKast;

    charF   = '\0';

    //SPIFFS.begin();
    
    if (!SPIFFS.exists("/Data/KastInfo.txt")) {
        newKast.kastID      = 0;
        newKast.interval    = 0;
        File dataFile = SPIFFS.open("/Data/KastInfo.txt", "w");
        // if the file is available, write to it:
        if (dataFile) {
            dataFile.println(newKast.kastID, DEC);
            dataFile.println(newKast.interval, DEC);
            dataFile.close();
        }
        // if the file isn't open, pop up an error:
        else {
            commPort.println("writeKastFile(): error opening KastCred.txt");
            commPort.println("<ERROR>");
            //SPIFFS.end();
            return newKast;
        }
    }
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile = SPIFFS.open("/Data/KastInfo.txt", "r");

    // first read kastID
    newKast.kastID  = 0;
    while (dataFile.available()) {
        charF = dataFile.read();
        if (('0' <= charF) && (charF <= '9')) {
            newKast.kastID = (newKast.kastID * 10) + ((int)charF - 48);
        }
        if (charF == '\n' || charF == '\r') break;
    }

    // second read interval
    newKast.interval  = 0;
    while (dataFile.available()) {
        charF = dataFile.read();
        if (('0' <= charF) && (charF <= '9')) {
            newKast.interval = (newKast.interval * 10) + ((int)charF - 48);
        }
        if ((newKast.interval > 0) && (charF == '\n' || charF == '\r')) break;
    }

    //SPIFFS.end();
    sendATTinyReadVcc();
    
    return newKast;
    
}   // readKastFile()


bool writeKastFile(structKast actKast) {
    
    if (actKast.year == 0 && actKast.day == 0) return false;

    //SPIFFS.begin();
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile = SPIFFS.open("/Data/KastInfo.txt", "w");
    //dataFile.seek(0);
    //dPrint("writeKastFile() => kastID [");
    //dPrint(kastID);   dPrintln("]");

    // if the file is available, write to it:
    if (dataFile) {
        dataFile.println(actKast.kastID, DEC);
        dataFile.println(actKast.interval, DEC);
        dataFile.close();
    }
    // if the file isn't open, pop up an error:
    else {
        commPort.println("writeKastFile(): error opening KastCred.txt");
        commPort.println("<ERROR>");
        //SPIFFS.end();
        return false;
    }

    //SPIFFS.end();

}   // writeKastFile()


float readDS18B20sensor() {
    
    float sensorTemp;
    char  fTmp[10];
    
    DS18B20.begin();
    DS18B20.requestTemperatures();
    sensorTemp = DS18B20.getTempCByIndex(0);
    dtostrf(sensorTemp, 6, 2, fTmp);  
    dPrintf("[-] TempIn [%s]\n", fTmp);          
    dFlush();
    
    return sensorTemp;
    
}   // readDS18B20sensor()

/***
float readDHTsensor() {

float h, t, hic;
char  fTmp[10];
int   L;

    pinMode(DHT_PIN, INPUT);
    delay(200);
    hic = -99.9;
    L   = 0;
    do {
        L++;    
        // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
        h = dht.readHumidity();
        // Read temperature as Celsius (the default)
        t = dht.readTemperature();
    
        if (isnan(h) || isnan(t)) {
            hic = -99.9;
            delay(200);
        } else {
            // Computes temperature values in Celsius + Fahrenheit and Humidity
            hic = dht.computeHeatIndex(t, h, false);       
        }
    } while ((hic <= -99.0) && (L < 6));
    
    if (hic == -99.9) {
        dPrintln("<DHT ERROR>");
        dPrint(" HumIn ["); dPrint(h); dPrintln("]");
        dPrint("TempIn ["); dPrint(t); dPrintln("]");
    } else {
        dtostrf(hic, 6, 2, fTmp);  
        dPrintf("[%d] TempIn [%s]\n", L, fTmp);          
    }
    
    dFlush();
    return hic;
    
}   // readDHTsensor()
***/

void goToSleep(char cType) {

    actKast = readRTC(actKast);
    digitalWrite(ATTINY_KEEP_AWAKE, HIGH);
    sleepTime = (65 - actKast.minute);
    //--- stressTest --------
    if (sleepTime > 35) sleepTime -= 30;
    //--- stressTest --------
    if (cType != 'M')   cType = 'A';
    sprintf(debugLine, "Start deepSleep @%02d:%02d, voor %d minuten", actKast.hour, actKast.minute, sleepTime);   
    dPrintln(debugLine);   
    //debug2LogFile(cType, debugLine);   
    delay(10);
    dFlush();
    digitalWrite(POWER_DEVICES, POWER_OFF);    
    WiFi.disconnect();          
    ESP.deepSleep((sleepTime * 60 * 1000000), WAKE_RF_DEFAULT); // 
    ESP.reset();
    
}   // goToSleep()



//===================================================================================
void setup() {
    dBegin(19200,SERIAL_8N1,SERIAL_TX_ONLY);
    WiFi.mode(WIFI_OFF);
    pinMode(POWER_DEVICES, OUTPUT);
    digitalWrite(POWER_DEVICES, POWER_ON);
    delay(100);
    dPrintln("\n+++++++    ");
    dFlush();

    SPIFFS.begin();
//------------------ test wakeUp by time or by reset button --------------
    
    Wire.begin(_SDA, _SCL); // Initiate the Wire library and join the I2C bus as a master
    if (! RTC.begin()) {
        Serial.println("Geen DS3231 klok module gevonden..");
    }
    RTC.writeSqwPinMode(DS3231_OFF);
    actKast = readRTC(actKast);
    actKast.tempOut = getDS3231Temp();
    //dtostrf(actKast.tempOut, 5, 2, debugLine);
    //Serial.printf("DS3231 temp [%s]\n", debugLine);
     
    readATTiny();    

    if (wakeUpType == 20)   commState = true;
    else                    commState = false;
    
    pinMode(ATTINY_KEEP_AWAKE, OUTPUT); // wakeUp ATTiny84 for I2C communication
    digitalWrite(ATTINY_KEEP_AWAKE, HIGH);
    
    //pinMode(POWER_DEVICES, OUTPUT);         // Set Power switch devices

    compileInfo();

//  Serial.setDebugOutput(true);
    if (commState) {
        digitalWrite(ATTINY_KEEP_AWAKE, LOW);
        WiFi.mode(WIFI_STA);
        delay(50);
        WiFi.begin("BeeHave");
        delay(50);
        WiFi.mode(WIFI_STA);
        delay(100);
        dPrintln("\nStartup BeeHave Kast");
    
        // We start by connecting to a WiFi network ------------------
        dPrint("\nWait for WiFi accessPoint (SIDD 'BeeHave') ... ");

        state = 0;
        while((WiFi.status() != WL_CONNECTED) && (state < 50)) {
            dPrint(".");
            state++;
            digitalWrite(POWER_DEVICES, POWER_ON);
            delay(200);
            digitalWrite(POWER_DEVICES, POWER_OFF);
            delay(300);
        }

        dPrintln(" ");
        if (WiFi.status() == WL_CONNECTED) {
            dPrintf("\nConnection status: %s\n", statusWL[WiFi.status()]);
            dPrint("WiFi connected with IP address: ");
            dPrintln(WiFi.localIP());
            digitalWrite(POWER_DEVICES, POWER_ON);
        } else {
            dPrintln("\nNo BeeHave WiFi found!");
            digitalWrite(POWER_DEVICES, POWER_OFF);
        }

        commPort.println("<OK>");
        commPort.flush();
    }   // commState
    
    dFlush();
    
    digitalWrite(POWER_DEVICES, POWER_ON);
    delay(500);
    sendTime = 0;
//================ WiFi connected =======================
    //dht.begin();
    //actKast.tempIn = readDHTsensor();
    actKast.tempIn = readDS18B20sensor();
    dtostrf(actKast.tempIn, 4, 2, celsiusTemp);
    dPrint("temperatuur (In) ["); dPrint(celsiusTemp); dPrintln("]");
    actKast.tempOut = getDS3231Temp();
    dtostrf(actKast.tempOut, 5, 2, debugLine);
    dPrint("temperatuur (Out) ["); dPrint(celsiusTemp); dPrintln("]");
    //       
    actKast     = readRTC(actKast);

    actKast = readKastFile(actKast); // to get kastID
    dPrint("KastID ["); dPrint(actKast.kastID); dPrintln("]");

    if (commState) {
        debug2LogFile('D', "wakeUp by Button");
        sprintf(debugLine, "wakeUp by Button @%02d:%02d", actKast.hour, actKast.minute);
        dPrintln(debugLine);
        state = COMM_STATE;
    } else {
        //debug2LogFile('D', "wakeUp by Timer");
        sprintf(debugLine, "wakeUp by Timer @%02d:%02d", actKast.hour, actKast.minute);
        dPrintln(debugLine);
        state = LOG_STATE;
    }
    
    dPrintln("End of startUp() .. ");

}   // setup()


void loop() {

    if (commState && (state == COMM_STATE)) {
        if (client.connect(host, port)) {
            state = COMM_STATE;
            keepCommAlive = millis() + 6000;
        } else {
            dPrintln("\nWiFi server connection lost ..");
            state == UNKNOWN_STATE;
            digitalWrite(POWER_DEVICES, POWER_OFF);
            digitalWrite(ATTINY_KEEP_AWAKE, HIGH);
        }
    }   // commState
    
    switch(state) {
        case UNKNOWN_STATE:
                        digitalWrite(ATTINY_KEEP_AWAKE, HIGH);
                        dPrintln("Unknown State: set to Logging ...");
                        state = LOG_STATE;
                        break;
        
        case COMM_STATE:
                        dPrintln("Do some Communicating ..");
                        if (millis() > keepCommAlive) {
                                 state = SLEEP_STATE;
                        } else { state = communicate();
                        }
                        break;                  

        case LOG_STATE:
                        digitalWrite(ATTINY_KEEP_AWAKE, HIGH);
                        delay(200);
                        dPrintln("Do some logging ...");
                        sendATTinyReadVcc();
                      //actKast.tempIn  = readDHTsensor();
                        actKast.tempIn  = readDS18B20sensor();
                        actKast.tempOut = getDS3231Temp();
                        dtostrf(actKast.tempOut, 6, 2, float2Char);  
                        dPrintf("[-] TempOut [%s]\n", float2Char);          
                        if (actKast.tempOut == 0.0) actKast.tempOut = actKast.tempIn;
                        if ((actKast.hour % actKast.interval) == 0) {
                            writeLogFile(actKast, 'T');
                        //} else {
                        //    sprintf(debugLine, "Nog even door slapen");
                        //    debug2LogFile('D', debugLine);
                        }
                        state = SLEEP_STATE;
                        break;
                        
        case SLEEP_STATE:
                        dPrintln("go to sleep!");
                        commPort.println("<GO2SLEEP>");
                        commPort.println("<GO2SLEEP>");
                        commPort.println("<GO2SLEEP>");
                        commPort.flush();
                        goToSleep('-');
                        state = COMM_STATE;
                        break;
                  
    }   // switch(state)

    delay(100);

}   // loop()

