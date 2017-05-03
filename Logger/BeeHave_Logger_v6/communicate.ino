/*
 *  als er een verbinding is met bluetooth (Serial) en
 *  er komen tekens binnen, analyse en reageer
 */

#define _C_MAXSIZE    50

char        command[_C_MAXSIZE + 1];
uint8_t     cSize;

uint8_t communicate() {
    //String  command;
    char    c;
    bool    getCommand;

    //dPrintln("Start communicate()");
    memset(command,0,sizeof(command));
    getCommand = false;
    keepCommAlive = millis() + 7000;    // wordt later langer (70 seconden)

    while (keepCommAlive > millis()) {
        if (commPort.available() > 0) {
            // wait for "<" lead-in
            c = commPort.read();
            if (c == '<') {     // start reading command
                memset(command,0,sizeof(command));
                getCommand = true;
            }
            if (getCommand && (c >= 32 && c <= 126)) {
                command[cSize++] = c;
                if (c == '>') {     // command complete
                    dPrintln(command);
                    state = processCommand(command);
                    cSize = 0;
                    memset(command,0,sizeof(command));
                }
                if (cSize > _C_MAXSIZE) {
                    commPort.println("<ERROR>");
                    cSize = 0;
                    memset(command,0,sizeof(command));
                    getCommand = false;
                }
                keepCommAlive = millis() + _TIMEOUT;
            }
        }   // if (commPort.available()) 

        yield();
        
    }   // while (keepCommAlive > millis()) 

    //dPrintln("Done communicating!");
    if (state == SLEEP_STATE) {
        return SLEEP_STATE;
    }
    return UNKNOWN_STATE;
    
}  //  communicate()
 

uint16_t getParm() {
    uint32_t  parm;
    String    endString;
    char      c;
    bool      getEnd;

    parm = 0;
    getEnd = false;
    keepCommAlive = millis() + _TIMEOUT;

    //parm = Serial.parseInt();
    parm = commPort.parseInt();
    
    while (keepCommAlive > millis()) {
        if (commPort.available()) {
            // wait for "<" lead-in
            c = commPort.read();
            if (c == '<') {     // start reading command
                endString = "";
                getEnd = true;
            }
            if (getEnd) {
                endString += c;
                if (c == '>') {     // command complete
                    if (endString.equals("<END>")) {
                        return parm;
                    }
                }
                if (endString.length() > 100) {
                    commPort.println("<ERROR>");
                    parm = -1;
                    return parm;    
                }
                keepCommAlive = millis() + _TIMEOUT;
            } 
        }   // commPort.available()

        yield();
            
    }   // while (keepCommAlive > millis())
    
 }  //  getParm()
 

 uint8_t processCommand(String command) {

    //uint32_t    sleepTime;
    
    dPrintln("<----[" + command + "]---->");
    if (command.equals("<HELP>")) {
        getHelp();
    } else if (command.equals("<INFO>")) {
        getKastInfo();
    } else if (command.equals("<LEESLOG>")) {
        getLogEntries();
    } else if (command.equals("<EMPTYLOG>")) {
        doEmptyLog();
    } else if (command.equals("<SETKASTID>")) {
        setKastId();
    } else if (command.equals("<SETINTERVAL>")) {
        setInterval();
    } else if (command.equals("<SETJAAR>")) {
        setJaar();
    } else if (command.equals("<SETMAAND>")) {
        setMaand();
    } else if (command.equals("<SETDAG>")) {
        setDag();
    } else if (command.equals("<SETUUR>")) {
        setUur();
    } else if (command.equals("<SETMINUUT>")) {
        setMinuut();
    } else if (command.equals("<WRITE2LOG>")) {
        write2Log();
    } else if (command.equals("<LEESDIR>")) {
        listFiles();
    } else if (command.equals("<PING>")) {
        actKast = readKastFile(actKast);
        commPort.println("<DATA>");
        dPrintln("<DATA>");
        commPort.println(String(actKast.kastID));
        dPrintln(String(actKast.kastID));
        commPort.println("<END>");
        dPrintln("<END>");
        commPort.println("<OK>");
        dPrintln("<OK>");
    } else if (command.equals("<GO2SLEEP>")) {
        state = SLEEP_STATE;
        commPort.println("<GO2SLEEP>");
        commPort.println("<OK>");
        goToSleep('M');
        return SLEEP_STATE;

    } else {
        commPort.print("<ERROR ");
        commPort.print(command);
        commPort.println(">");
    }
    dFlush();
    
 }  //      processCommand()

