/*
    Program  : BeeHave_Reader_v4
    
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
#include <SPI.h>
#include <SdFat.h>

#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>
//#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <SSD1306.h>

#include "BeeHave_Reader.h"
#include "menuBackend.h"

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

enum { BAR_INIT, BAR_TICK };

WiFiServer server(port);
WiFiClient serverClient;
Encoder myEnc(ROTARY_PIN1, ROTARY_PIN2);
//LiquidCrystal_I2C lcd(0x27, 16, 2);
// Initialize the OLED display using brzo_i2c
// D3 -> SDA
// D2 -> SCL
// SSD1306Brzo display(0x3c, D3, D5);
// or
// SH1106Brzo  display(0x3c, D3, D5);

// Initialize the OLED display using Wire library
SSD1306  display(0x3c, _SDA, _SCL);

/* SD card attached to SPI bus as follows:
 * MOSI - GPIO13, D7, pin  7
 * MISO - GPIO12, D6, pin  6
 * CLK  - GPIO14, D5, pin  5
 * CS   - GPIO15, D8, pin 16
*/
SdFat SD;
File logFile;

// Global variables
structKast   actKast;

uint8_t     menuId = 0;
uint32_t    pingTime;
uint8_t     redrawMenu;  // triggers whether menu is redrawMenun after cursor move.
String      compiled = "";
bool        clientConnected;
uint8_t     barPos = 0;
uint8_t     barDirection = 1;
String      lineIn;

void compileInfo() {
    String progName    = String(__FILE__);

    Serial.flush();
    Serial.print("\nProgram [");
    Serial.print(progName.substring(progName.lastIndexOf('/')+1));
    Serial.print("] ");
    Serial.print("Version [");
    Serial.print(__DATE__);
    Serial.print(" ");
    Serial.print(__TIME__);
    Serial.println("]");
    Serial.flush();
    
}   // compileInfo()


void renameLogFile(String fileName) {
    char rDir[50];
    char fOld[100];
    char fNew[100];
    File tmpFile;

    //Serial.printf("rename logFile [%s]\nfirst change to [/BeeHave]\n", fileName.c_str());

    sprintf(rDir, "/BeeHave/Kast_%03d", actKast.kastID);
    SD.mkdir(rDir);
    SD.chdir(rDir);
    for(uint8_t R=98; R>0; R--) {
        sprintf(fOld, "Kast_%03d-%02d.csv", actKast.kastID, R);
        if (SD.exists(fOld)) {
            tmpFile = SD.open(fOld, FILE_WRITE);
            sprintf(fNew, "Kast_%03d-%02d.csv", actKast.kastID, (R + 1));
            //Serial.printf("Rename [%s] to [%s]\n", fOld, fNew);
            tmpFile.rename(SD.vwd(), fNew);
            tmpFile.close();
        }
    }
    SD.chdir(rDir);
    sprintf(fOld, "Kast_%03d.csv", actKast.kastID);
    if (SD.exists(fOld)) {
        tmpFile = SD.open(fOld, FILE_WRITE);
        sprintf(fNew, "Kast_%03d-%02d.csv", actKast.kastID, 1);
        //Serial.printf("Rename [%s] to [%s]\n", fOld, fNew);
        tmpFile.rename(SD.vwd(), fNew);
        tmpFile.close();
    }

}   // renameLogFile()

File openLogFile(String fileName) {
    //Serial.printf("Open logFile [%s]\n", fileName.c_str());
    renameLogFile(fileName);
    logFile = SD.open(fileName, FILE_WRITE);
    return logFile;
    
}   // openLogFile()

bool writeLogFile(String logLine) {

    // if the file is available, write to it:
    if (logFile) {
        logFile.println(logLine);
    }
    
}   // saveLogFile()

bool closeLogFile() {
    logFile.close();
    // --- now, empty the internal log at the Logger
    serverClient.println("<EMPTYLOG>");
    pingTime = millis() + _PINGTIME;
    lineIn = "";
    bool endResponse = false;
    do {
        lineIn = readFromKast();
        if (lineIn.equals("<OK>") || lineIn.equals("<ERROR>") || lineIn.equals("<TIMEOUT>")) {
            endResponse = true;
        }
        yield();
    } while (!endResponse);

}   // closeLogFile()

 
void setup() {
    Serial.begin(19200);
    delay(100);
    Serial.println();
    Serial.flush();
    compileInfo();
    Serial.println("\n==> And then it begins...\n");

    dspl_Init();
    dspl_Clear();  // clear the screen so we can paint the menu.
    dspl_Print_Msg(0, "BeeHave Master", 0);
    dspl_Print_Msg(2, "By: Willem", 0);
    dspl_Print_Msg(3, "    Aandewiel", 0);
    
    //--------------------------- setup WiFi AccessPoint ------------------------
    //Serial.setDebugOutput(true);
    Serial.print("StartUp Access Point ");
    while (!startAP()) {
        Serial.print(".");
        delay(5000);
    }

    Serial.printf("\nSoft-AP SSID [%s]\n", ssid);
    dspl_Print_Msg(2, "Status:              ", 0);
    dspl_Print_Msg(3, "                     ", 0);
    dspl_Print(0,3,"SSID: ");
    dspl_Print(6,3, ssid);
    delay(500);
    //---------- start server ----------
    server.begin();
    Serial.printf("TCP server started, IP [%s] port [%d] \n", WiFi.softAPIP().toString().c_str(), port);
    server.setNoDelay(true);
    clientConnected = false;
    // set the data rate for the SoftwareSerial port
    //serverClient.begin(19200);

    pinMode(ROTARY_SWITCH, INPUT_PULLUP);

    Serial.print("Initializing SD card...");
    // see if the card is present and can be initialized:
    pinMode(_CS, OUTPUT);

    while (!SD.begin(_CS, SPI_SPEED)) {
        Serial.println("Card failed, or not present");
        dspl_Print(0,3, "SD Card fout!          ");
        delay(500);
    }
    Serial.println(".. initialized.");
    dspl_Print(0,3, "SD Card gevonden      ");    
    SD.mkdir("/BeeHave");
    delay(500);

    dspl_Print(0,2, "Status: Wacht op      ");
    dspl_Print(0,3, "verbinding ...        ");

    pingTime = millis(); // + _PINGTIME;

}   // setup()(

 
void loop() {

    if (WiFi.softAPgetStationNum() == 0) {
        clientConnected = false;
        //progressBar(1, BAR_INIT);
        if (millis() > pingTime) {
            pingTime = millis() + 300;
          //Serial.printf("Stations connected = %d .. ", WiFi.softAPgetStationNum());
          //Serial.println("wacht op verbinding ...");
            progressBar(1, BAR_TICK);
        }
     } else if (!clientConnected) {
                if (server.hasClient()) {
                    Serial.println("server has client ...");
                    serverClient = server.available();
                    clientConnected = true;
                    Serial.println("pingTime = 0");
                    pingTime = 0;
                }
    } else {    // normale verbinding met state COMM_STATE
        if (serverClient.available() > 5) {
            lineIn = readFromKast();
        }
        mainMenu(menuId);
    }
    
}       // loop()

