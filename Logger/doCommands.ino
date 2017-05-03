/*
 * voer binnengekregen commando's uit
 */

void getHelp() {

    commPort.println("<HELP>");
    commPort.println("<INFO>");
    commPort.println("<LEESLOG>");
    commPort.println("<EMPTYLOG>");
    commPort.println("<SETKASTID>{val}<END>");
    commPort.println("<SETINTERVAL>{val}<END>");
    commPort.println("<SETJAAR>{val}<END>");
    commPort.println("<SETMAAND>{val}<END>");
    commPort.println("<SETDAG>{val}<END>");
    commPort.println("<SETUUR>{val}<END>");
    commPort.println("<SETMINUUT>{val}<END>");
    commPort.println("<GO2SLEEP>");
    commPort.println("<LEESDIR>");
        
    commPort.println("<OK>");
    commPort.flush();

}   //  getHelp()


void getKastInfo() {

char    KIbuffer[100];
char    str_tempIn[10];
char    str_tempOut[10];
char    str_Vcc[7];
//uint8_t kSize;

    memset(KIbuffer,0,sizeof(KIbuffer));

    actKast         = readKastFile(actKast);
    actKast         = readRTC(actKast);
  //actKast.tempIn   = readDHTsensor();
    actKast.tempIn   = readDS18B20sensor();
    actKast.tempOut  = getDS3231Temp();
    dtostrf(actKast.Vcc, 5, 3, str_Vcc);
    dtostrf(actKast.tempIn, 4, 2, str_tempIn);
    dtostrf(actKast.tempOut, 4, 2, str_tempOut);
    commPort.println("<DATA>");
    dPrintln("<DATA>");
    //                 ID  I   Sec   Min   Uur  wd
    sprintf(KIbuffer, "%d\n%d\n%02d\n%02d\n%02d\n%d"
                                        , actKast.kastID
                                        , actKast.interval
                                        , actKast.second
                                        , actKast.minute
                                        , actKast.hour
                                        , actKast.weekday
             );
    commPort.println(KIbuffer);
    commPort.flush();
    dPrintln(KIbuffer);
    //                 day   Mnd  Year Vcc tempIn tempOut
    sprintf(KIbuffer, "%02d\n%02d\n%04d\n%s\n%s\n%s"
                                        , actKast.day
                                        , actKast.month
                                        , actKast.year
                                        , str_Vcc
                                        , str_tempIn
                                        , str_tempOut
             );
    commPort.println(KIbuffer);
    commPort.flush();
    dPrintln(KIbuffer);
    commPort.println("<END>");
    dPrintln("<END>");

    commPort.println("<OK>");
    dPrintln("<OK>");
    dFlush();
    delay(500);

}  // getKastInfo()

void getLogEntries() {

    readLogFile();
    commPort.println("<OK>");
    commPort.flush();
    dPrintln("<OK>");

}  // getLogEntries()

void doEmptyLog() {

    //SPIFFS.begin();
    if (!SPIFFS.exists("/Data/LogData.cvs")) {
        commPort.println("<ERROR>");
        return;
    }

    // delete the file:
    commPort.println("Removing DataLog.csv...");
    SPIFFS.remove("/Data/LogData.sav");
    commPort.println("Rename DataLog.csv...DataLog.sav");
    SPIFFS.rename("/Data/LogData.cvs", "/Data/LogData.sav");

    if (SPIFFS.exists("/Data/LogData.cvs")) {
        commPort.println("<ERROR>");
    } else {
        commPort.println("<OK>");
    }
    commPort.flush();
    //SPIFFS.end();
    
}   // doEmptyLog()


void setKastId() {
    int16_t parm;

    parm = getParm();
    //commPort.print("Received parm ["); commPort.print(String(parm)); commPort.print("] ");
    actKast.kastID = parm;
    writeKastFile(actKast);
    commPort.println("<OK>");
    sprintf(debugLine, "KastID set to [%d]", actKast.kastID);
    debug2LogFile('S', debugLine);

}   // setKastId()

void setInterval() {
    
    int16_t parm;

    parm = getParm();
    //commPort.print("Received parm ["); commPort.print(String(parm)); commPort.print("] ");
    if (parm == 5)  parm = 6;
    actKast.interval = parm;
    writeKastFile(actKast);
    commPort.println("<OK>");
    sprintf(debugLine, "Interval set to [%d] hour", actKast.interval);
    debug2LogFile('S', debugLine);

}   //  setInterval()

void setJaar() {
    
    int16_t parm;

    parm = getParm();
    //commPort.print("Received parm ["); commPort.print(String(parm)); commPort.print("] ");
    if (parm >= 2000 && parm <= 2099) {
        actKast.year = parm;
        writeRTC(actKast);
        commPort.println("<OK>");
        sprintf(debugLine, "TimeSet: Jaar [%d]", actKast.year);
        debug2LogFile('S', debugLine);
} else {
        commPort.println("<ERROR>");
    }

}   //  setJaar()

void setMaand() {
    
    int16_t parm;

    parm = getParm();
    //commPort.println("Received parm ["); commPort.print(String(parm)); commPort.print("] ");
    if (parm >= 1 && parm <= 12) {
        actKast.month = parm;
        writeRTC(actKast);
        commPort.println("<OK>");
        sprintf(debugLine, "TimeSet: maand [%d]", actKast.month);
        debug2LogFile('S', debugLine);
    } else {
        commPort.println("<ERROR>");
    }

}   //  setMaand()

void setDag() {

    int16_t parm;

    parm = getParm();
    //commPort.print("Received parm ["); commPort.print(String(parm)); commPort.print("] ");
    if (parm >= 1 && parm <= 31) {
        actKast.day = parm;
        writeRTC(actKast);
        commPort.println("<OK>");
        sprintf(debugLine, "TimeSet: dag [%d]", actKast.day);
        debug2LogFile('S', debugLine);
    } else {
        commPort.println("<ERROR>");
    }

}   //  setDag()

void setUur() {
    
    int16_t parm;

    parm = getParm();
    //commPort.print("Received parm ["); commPort.print(String(parm)); commPort.print("] ");
    if (parm >= 0 && parm <= 24) {
        actKast.hour = parm;
        writeRTC(actKast);
        commPort.println("<OK>");
        sprintf(debugLine, "TimeSet: uur [%d]", actKast.hour);
        debug2LogFile('S', debugLine);
    } else {
        commPort.println("<ERROR>");
    }

}   //  setUur()

void setMinuut() {
    
    int16_t parm;

    parm = getParm();
    //commPort.print("Received parm ["); commPort.print(String(parm)); commPort.print("] ");
    if (parm >= 0 && parm <= 59) {
        actKast.minute = parm;
        actKast.second = 1;
        writeRTC(actKast);
        commPort.println("<OK>");
        sprintf(debugLine, "TimeSet: minuten [%d]", actKast.minute);
        debug2LogFile('S', debugLine);
    } else {
        commPort.println("<ERROR>");
    }

}   //  setMinuut()
 

void listFiles() {

String fileName;

String oldName = "x";
File f ;

    //SPIFFS.begin();

    dPrintln("[1] /");
    Dir dir = SPIFFS.openDir("/");
    delay(100);
    while (dir.next()) {
        fileName = dir.fileName();
        if (oldName == fileName) {
            break;
        }
        oldName = fileName;
        f = dir.openFile("r");
        sprintf(debugLine, "%06d: %s", f.size(), fileName.c_str());
        f.close();
        debug2LogFile('D', debugLine);
        dPrintln(debugLine);
        delay(50);
    } 
    
    commPort.println("<OK>");

}   //  listFiles()
 

void write2Log() {
char  tempIn_str[8];
char  tempOut_str[8];
char  Vcc_str[8];
    
    sendATTinyReadVcc();
  //actKast.tempIn  = readDHTsensor();
    actKast.tempIn  = readDS18B20sensor();
    actKast.tempOut = getDS3231Temp();
    writeLogFile(actKast, 'M');
    dtostrf(actKast.Vcc, 5, 3, Vcc_str);
    dtostrf(actKast.tempIn, 4, 2, tempIn_str);
    dtostrf(actKast.tempOut, 4, 2, tempOut_str);
    commPort.println("<DATA>");
    commPort.println(Vcc_str);
    commPort.println(tempIn_str);
    commPort.println(tempOut_str);
    commPort.println("<END>");
    commPort.println("<OK>");
    commPort.flush();

}   //  write2Log()

