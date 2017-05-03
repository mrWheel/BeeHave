
// Put the menu items here. Remember, the first item will have a 'position' of 0.
  String hoofdItems[]={
    "Kast Info", 
    "Lees Logfile", 
    "Schrijf Log",
    "Kast Setup", 
    "Log leegmaken",
    "Go To Sleep",
    "Lees Map",
    ""
  };
  String sub1Items[]={
    "Terug", 
    "Set KastID", 
    "Set Interval", 
    "Set Klok",
    ""
  };
  String sub2Items[]={
    "Terug", 
    "Set Jaar", 
    "Set Maand", 
    "Set Dag",
    "Set Uur",
    "Set Minuut",
    ""  };
    
  String sub3Info[]={
    "--- INFO ---",
    "KastID         ", 
    "Interval       ", 
    "Minuut         ",
    "Uur            ",
    "Dag            ",
    "Maand          ",
    "Jaar           ",
    "Batterij       ",
    "Temp. (In)     ",
    "Temp. (Out)    ",
    ""  };


  // redrawMenu = 0  - don't redrawMenu
  // redrawMenu = 1 - redrawMenu cursor
  // redrawMenu = 2 - redrawMenu list
//  byte      redrawMenu = _MOVE_LIST;  // triggers whether menu is redrawMenun after cursor move.
  boolean   stillSelecting;     // set because user is still selecting.
  byte      i;                  // temp variable for loops.
  int32_t   tempNr;
  String    toLCD;
  bool      endResponse;
  bool      doData;
  bool      endData;
  uint8_t   numData;

/*
MENU ROUTINE
*/

void mainMenu(int menuId) {

    Serial.print(F("==> mainMenu(")); Serial.print(menuId); Serial.println(F(");"));
    switch(menuId) {
        case 0:     redrawMenu = _MOVE_LIST;
                    doMenu(hoofdItems);
                    break;
        case 1:     redrawMenu = _MOVE_LIST;
                    doMenu(sub1Items);
                    break;
        case 2:     redrawMenu = _MOVE_LIST;
                    doMenu(sub2Items);
                    break;
        case 3:     redrawMenu = _MOVE_LIST;
                    doMenu(sub3Info);
                    break;
        default:    redrawMenu = _MOVE_LIST;
                    doMenu(hoofdItems);
                    break;
    }   // switch(menuId)

    Serial.printf("Exit mainMenu(%d) clientConnected[", menuId); 
    if (clientConnected)    Serial.println("true] ..");
    else                    Serial.println("false] ..");
    
}   // mainMenu()


void doMenu(String menuItems[]) {

  byte      topItemDisplayed = 0;  // stores menu item displayed at top of LCD screen
  byte      cursorPosition = 0;  // where cursor is on screen, from 0 --> _TOTAL_ROWS . 
  byte      totalMenuItems = 0;  //a while loop below will set this to the # of menu items.
  uint8_t   menuNew   = 0;
  char      mKey;
  uint8_t   swState, rState;


    while (menuItems[totalMenuItems] != "") {
        Serial.print("["); Serial.print(totalMenuItems); Serial.print("] "); 
        Serial.println(menuItems[totalMenuItems]);
        totalMenuItems++;  // count how many items are in list.
        yield();
    }
    totalMenuItems--;  //subtract 1 so we know total items in array.

    dspl_Clear();  // clear the screen so we can paint the menu.

    stillSelecting = true;  // set because user is still selecting.

    menuTimeOut = millis() + _MENUTIMEOUT; // set initial timeout limit. 

    do {   // loop while waiting for user to select.
        if (pingTime < millis()) {
            Serial.println(F("==> <PING>"));
            serverClient.println(F("<PING>"));
            pingTime = millis() + _PINGTIME;
            lineIn      = "";
            doData      = false;
            endResponse = false;
            do {
                lineIn = readFromKast();
                Serial.println(lineIn);
                if (lineIn.equals("<OK>")) {
                    endResponse = true;
                }
                if (lineIn.equals("<PONG>")) {
                    Serial.print(F("==> "));
                    Serial.println(lineIn);
                    endResponse = true;
                    actKast.kastID = 0;
                }
                if (lineIn.equals("<ERROR>") || lineIn.equals("<TIMEOUT>")) {
                    Serial.print(F("==> "));
                    Serial.println(lineIn);
                    endResponse = true;
                    actKast.kastID = 0;
                }
                if (lineIn.equals("<END>")) {  
                    doData   = false;
                }
                if (doData) {
                    actKast.kastID = lineIn.toInt();
                }
                if (lineIn.equals("<DATA>")) {  
                    doData   = true;
                }
                
                yield();       

            } while (!endResponse);
            redrawMenu = _MOVE_LIST;
        }

        
        switch(readEncoder()) {  // analyze encoder response. Default is 0.

        case 'U':  // ENCODER ROTATED UP. EQUIVALENT OF 'UP' BUTTON PUSHED

                menuTimeOut = millis()+_MENUTIMEOUT;  // reset timeout timer
                //  if cursor is at top and menu is NOT at top
                //  move menu up one.
                if (cursorPosition == 0 && topItemDisplayed > 0) {   // Cursor is at top of LCD, and there 
                                                                     // are higher menu items still to be displayed.
                    topItemDisplayed--;  // move top menu item displayed up one. 
                    redrawMenu = _MOVE_LIST;  // redrawMenu the entire menu
                } 
                if (cursorPosition == 0 && topItemDisplayed == 0) {  
                    dspl_Print(11,0,"[   ]");
                    dspl_Print(12,0,String(actKast.kastID));
                }

                // if cursor not at top, move it up one.
                if (cursorPosition>0) {
                    cursorPosition--;  // move cursor up one.
                    redrawMenu = _MOVE_CURSOR;  // redrawMenu just cursor.
                }
                Serial.print(">["); Serial.print((topItemDisplayed + cursorPosition)); Serial.print("] ");
                Serial.println(menuItems[(topItemDisplayed + cursorPosition)]);
                break;

        case 'D':    // ENCODER ROTATED UP. EQUIVALENT OF 'DOWN' BUTTON PUSHED

                menuTimeOut = millis()+_MENUTIMEOUT;  // reset timeout timer
                // this sees if there are menu items below the bottom of the LCD screen & sees if cursor 
                // is at bottom of LCD 
                if ((topItemDisplayed + (_TOTAL_ROWS -1)) < totalMenuItems && cursorPosition == (_TOTAL_ROWS -1)) {
                    topItemDisplayed++;  // move menu down one
                    redrawMenu = _MOVE_LIST;  // redrawMenu entire menu
                }
                if (cursorPosition<(_TOTAL_ROWS -1)) { // cursor is not at bottom of LCD, so move it down one.
                    cursorPosition++;  // move cursor down one
                    redrawMenu = _MOVE_CURSOR;  // redrawMenu just cursor.
                }
                Serial.print(">["); Serial.print((topItemDisplayed + cursorPosition)); Serial.print("] ");
                Serial.println(menuItems[(topItemDisplayed + cursorPosition)]);
                break;

        case 'S':  // ENCODER BUTTON PUSHED FOR SHORT PERIOD & RELEASED.
                //swState = _NOKEYPRESSED;

                menuTimeOut = millis()+_MENUTIMEOUT;  // reset timeout timer
                //pingTime = 0;
                yield();

                switch(menuId) {
                    case 0:     menuNew = processHoofdMenu(menuItems, (topItemDisplayed + cursorPosition));
                                break;
                    case 1:     menuNew = processSub1Menu(menuItems, (topItemDisplayed + cursorPosition));
                                break;
                    case 2:     menuNew = processSub2Menu(menuItems, (topItemDisplayed + cursorPosition));
                                break;
                    case 3:     menuNew = processKastInfo(menuItems, (topItemDisplayed + cursorPosition));
                                break;
                    default:    menuNew = processHoofdMenu(menuItems, 0);
                                break;
                }   // switch(menuId)
                if (menuNew != menuId) {
                    Serial.println(F("==> new menu"));
                    stillSelecting = false;
                    menuId = menuNew;
                }
                break;
        
        case 'L':  // encoder button was pushed for long time. This corresponds to "Back" or "Cancel" being pushed.
                //swState = _NOKEYPRESSED;
                yield();
                stillSelecting = false;
                Serial.println(F("==> Cancel pressed"));
                dspl_Clear();  // clear the screen so we can paint the menu.
                menuId = 0;
                return;
                break;
        
        }       // switch(getKey())
        
        switch(redrawMenu) {  //  checks if menu should be redrawMenun at all.
        case _MOVE_LIST:  // the entire menu needs to be redrawMenun
                redrawMenu=_MOVE_CURSOR;  // redrawMenu cursor after clearing LCD and printing menu.
                dspl_Clear(); // clear screen so it can be repainted.
                if (totalMenuItems>((_TOTAL_ROWS -1))) {  // if there are more menu items than LCD rows, then cycle through menu items.
                    for (i = 0; i < (_TOTAL_ROWS ); i++){
                        dspl_Print(1,i, menuItems[topItemDisplayed + i]);
                        if ((topItemDisplayed + i) == 0) {   //  first menu item 
                            dspl_Print(11,0,"[   ]");
                            dspl_Print(12,0,String(actKast.kastID));
                        }
                        yield();
                    }
                }
                else {  // if menu has less items than LCD rows, display all available menu items.
                    for (i = 0; i < totalMenuItems+1; i++){
                        dspl_Print(1, i, menuItems[topItemDisplayed + i]);
                    }
                }
                //break;  // <<====== don't break, direct door naar _MOVE_CURSOR

        case _MOVE_CURSOR:  // Only the cursor needs to be moved.
                redrawMenu = false;  // reset flag.
                if (cursorPosition > totalMenuItems) { // keeps cursor from moving beyond menu items.
                    cursorPosition = totalMenuItems;
                }
                for (i = 0; i < (_TOTAL_ROWS); i++) {  // loop through all of the lines on the LCD
                    dspl_Print(0,i, " ");                      // and erase the previously displayed cursor
                }
                dspl_Print(0,cursorPosition, ">");
                break;  // _MOVE_CURSOR break.

        }       // switch(redrawMenu)
        
        if (menuTimeOut < millis()) {  // user hasn't done anything in awhile
            stillSelecting = false;  // tell loop to bail out.
            menuId = 0;
            stillSelecting = false;
 
        }

        yield();
        if (WiFi.softAPgetStationNum() == 0) clientConnected = false;
        
    } while (stillSelecting && clientConnected && pingTime > millis());  //

    Serial.println("Exiting doMenu()");

}   // doMenu()


uint8_t processHoofdMenu(String menuItems[], uint8_t Selection) {

    uint16_t I;
    String   nLine;
    char     cTmp[20];
    bool     prevDoExec, doExec = false;
    bool     stillEntering = true;

    switch(Selection) { 
                case 0:     // menu item 1 selected <kastInfo>
                            Serial.println(menuItems[Selection]);
                            Serial.println(F("==> send <INFO> .."));
                            serverClient.println(F("<INFO>"));
                            pingTime = millis() + _PINGTIME;
                            dspl_Print_Msg((_TOTAL_ROWS - 1), "--- Kast Info ---   ", 0);
                            lineIn      = "";
                            doData      = false;
                            endResponse = false;
                            I = 0;
                            do {
                                I++;
                                lineIn = readFromKast();
                                Serial.print(F("==> ["));
                                Serial.print(lineIn);
                                Serial.println(F("]"));
                                dspl_Print_Msg(_TOTAL_ROWS, "                    ", 0);
                                if (lineIn.equals("<OK>")) {
                                    endResponse = true;
                                }
                                if (lineIn.equals("<ERROR>") || lineIn.equals("<TIMEOUT>")) {
                                    dspl_Print_Msg(_TOTAL_ROWS, lineIn, 200);
                                    menuId = 0;
                                    endResponse = true;
                                    return 0;
                                }
                                if (lineIn.equals("<END>")) {  
                                    doData   = false;
                                    numData     = 0;
                                }
                                if (doData) {
                                    nLine = "";
                                    numData++;
                                    switch(numData) {
                                        case 1: actKast.kastID = lineIn.toInt();
                                                sprintf(cTmp, "KastID [%3d]", actKast.kastID);
                                                sub3Info[1] = cTmp;
                                                nLine = "KastID ";
                                                break;
                                        case 2: actKast.interval = lineIn.toInt();
                                                sprintf(cTmp, "Interval [%1d]", actKast.interval);
                                                sub3Info[2] = cTmp;
                                                nLine = "Interval ";
                                                break;
                                        case 3: actKast.second = lineIn.toInt();
                                                //sprintf(cTmp, "Seconden [%2d]", actKast.second);
                                                //sub3Info[3] = cTmp;
                                                nLine = "Sec. ";
                                                break;
                                        case 4: actKast.minute = lineIn.toInt();
                                                sprintf(cTmp, "Minuut  [%2d]", actKast.minute);
                                                sub3Info[3] = cTmp;
                                                nLine = "Min. ";
                                                break;
                                        case 5: actKast.hour = lineIn.toInt();
                                                sprintf(cTmp, "Uur     [%2d]", actKast.hour);
                                                sub3Info[4] = cTmp;
                                                nLine = "Uur ";
                                                break;
                                        case 6: actKast.weekday = lineIn.toInt();
                                                nLine = "WDag ";
                                                break;
                                        case 7: actKast.day = lineIn.toInt();
                                                sprintf(cTmp, "Dag     [%2d]", actKast.day);
                                                sub3Info[5] = cTmp;
                                                nLine = "Dag ";
                                                break;
                                        case 8: actKast.month = lineIn.toInt();
                                                sprintf(cTmp, "Maand   [%2d]", actKast.month);
                                                sub3Info[6] = cTmp;
                                                nLine = "Maand ";
                                                break;
                                        case 9: actKast.year = lineIn.toInt();
                                                sprintf(cTmp, "Jaar  [%4d]", actKast.year);
                                                sub3Info[7] = cTmp;
                                                nLine = "Jaar ";
                                                break;
                                        case 10:actKast.Vcc = lineIn.toFloat();
                                                sprintf(cTmp, "Batt.[%s]Vcc", lineIn.c_str());
                                                sub3Info[8] = cTmp;
                                                nLine = "Vcc ";
                                                break;
                                        case 11:actKast.tempIn = lineIn.toFloat();
                                                sprintf(cTmp, "*C In [%s]", lineIn.c_str());
                                                sub3Info[9] = cTmp;
                                                nLine = "TempIn ";
                                                break;
                                        case 12:actKast.tempOut = lineIn.toFloat();
                                                sprintf(cTmp, "*C Out[%s]", lineIn.c_str());
                                                sub3Info[10] = cTmp;
                                                nLine = "TempOut ";
                                                break;
                                    }
                                    nLine += String(lineIn.toInt()) + "     ";
                                    dspl_Print_Msg(_TOTAL_ROWS, nLine, 50);
                                }
                                if (lineIn.equals("<DATA>")) {  
                                    //Serial.println("<DATA> found ..");
                                    doData   = true;
                                    numData     = 0;
                                }
                                menuId = 3; // show KastInfo
                                redrawMenu = _MOVE_LIST;
                                stillSelecting = false;
                                yield();
                                
                            } while (!endResponse);
                            break;

                case 1:  // menu item 2 selected    <lees log>
                            Serial.println(menuItems[Selection]);
                            Serial.println(F("==> send <LEESLOG> .."));
                            //dspl_Clear();
                            dspl_Print_Msg((_TOTAL_ROWS - 1), "--- Lees Log ---    ", 0);
                            serverClient.println(F("<LEESLOG>"));
                            pingTime = millis() + _PINGTIME;

                            sprintf(cTmp, "Kast_%03d.csv", actKast.kastID);
                            logFile = openLogFile(cTmp);
                            doData      = false;
                            endResponse = false;
                            progressBar(_TOTAL_ROWS, BAR_INIT);
                            I = 1;
                            do {
                                lineIn = readFromKast();
                                Serial.print(F("==> ["));
                                Serial.print(lineIn);
                                Serial.println(F("]"));
                                //dspl_Print_Msg(_TOTAL_ROWS, "                    ", 0);
                                if (lineIn.equals("<END>")) {  
                                    //Serial.println("<END> found ..");
                                    doData   = false;
                                    numData  = 0;
                                    delay(2000);
                                }
                                if (doData) {
                                    writeLogFile(logFile, lineIn);
                                    //toLCD = lineIn.substring(22);
                                    //toLCD.replace(";", "");
                                    //dspl_Print_Msg(_TOTAL_ROWS, toLCD, 10);
                                    I++;
                                }
                                if (lineIn.equals("<OK>")) {
                                    endResponse = true;
                                }
                                if (lineIn.equals("<ERROR>") || lineIn.equals("<TIMEOUT>")) {
                                    dspl_Print_Msg(_TOTAL_ROWS, lineIn, 300);
                                    endResponse = true;
                                }
                                if (lineIn.equals("<DATA>")) {  
                                    //Serial.println("<DATA> found ..");
                                    doData   = true;
                                    numData     = 0;
                                }
                                progressBar(_TOTAL_ROWS, BAR_TICK);
                                yield();
                                
                            } while (!endResponse);
                            
                            closeLogFile(logFile);
                            //redrawMenu = _MOVE_LIST;
                            break;

                case 2:  // menu item 3 selected    <schrijf log>
                            Serial.println(menuItems[Selection]);
                            Serial.println(F("==> send <WRITE2LOG> .."));
                            //dspl_Clear();
                            dspl_Print_Msg((_TOTAL_ROWS - 1), "-- Schrijf Log -- ", 0);
                            serverClient.println(F("<WRITE2LOG>"));
                            pingTime = millis() + _PINGTIME;
                            
                            lineIn = "";
                            doData      = false;
                            endResponse = false;
                            I = 0;
                            do {
                                lineIn = readFromKast();
                                Serial.print(F("==> ["));
                                Serial.print(lineIn);
                                Serial.println(F("]"));
                                if (lineIn.equals("<END>")) {  
                                    //Serial.println("==> <END> found ..");
                                    doData   = false;
                                    numData     = 0;
                                    delay(2000);
                                }
                                if (doData) {
                                    I++;
                                    toLCD = "--------------------------------";
                                    if (I == 1) toLCD = "Vcc[" + lineIn + "]";
                                    if (I == 2) toLCD = "Temp(In) [" + lineIn + "]";
                                    if (I == 3) toLCD = "Temp(Out)[" + lineIn + "]";
                                    Serial.println(toLCD);
                                    dspl_Print_Msg(_TOTAL_ROWS, toLCD, 300);
                                    //delay(2000);
                                }
                                if (lineIn.equals("<OK>")) {
                                    //dspl_Print_Msg(_TOTAL_ROWS, lineIn, 300);
                                    endResponse = true;
                                }
                                if (lineIn.equals("<ERROR>") || lineIn.equals("<TIMEOUT>")) {
                                    dspl_Print_Msg(_TOTAL_ROWS, lineIn, 300);
                                    endResponse = true;
                                }
                                if (lineIn.equals("<DATA>")) {  
                                    //Serial.println("==> <DATA> found ..");
                                    doData  = true;
                                    I       = 0;
                                    numData = 0;
                                }

                                yield();
                                
                            } while (!endResponse);
                            //redrawMenu = _MOVE_LIST;
                            break;

                case 3:  // menu item 4 selected    <setup>
                            Serial.println(menuItems[Selection]);
                            menuId = 1;
                            redrawMenu = _MOVE_LIST;
                            stillSelecting = false;
                            break;

                case 4:  // menu item 5 selected    <leeg log>
                            //Serial.print(F("\n==> Menu item "));
                            //Serial.print(Selection);
                            //Serial.print(F(" selected - "));
                            Serial.println(menuItems[Selection]);
                            stillSelecting = true;
                            dspl_Print_Msg((_TOTAL_ROWS - 1), "-- Maak Leeg --    ", 0);
                            
                            if (confirm("Nee", "Ja")) {
                                Serial.println("==> send <EMPTYLOG> ..");
                                serverClient.println("<EMPTYLOG>");
                                pingTime = millis() + _PINGTIME;
                                lineIn = "";
                                endResponse = false;
                                do {
                                    lineIn = readFromKast();
                                    Serial.print(F("==> ["));
                                    Serial.print(lineIn);
                                    Serial.println(F("]"));
                                    if (lineIn.equals("<OK>") || lineIn.equals("<ERROR>") || lineIn.equals("<TIMEOUT>")) {
                                        dspl_Print_Msg(_TOTAL_ROWS, lineIn, 1000);
                                        endResponse = true;
                                    }

                                    yield();
                                
                                } while (!endResponse);
                            }
                            break;

                case 5:  // menu item 6 selected    // - go2sleep
                            dspl_Print_Msg((_TOTAL_ROWS - 1), "--- Slapen ---", 0);
                            if (confirm("Nee", "Ja")) {
                                Serial.println("==> send <GO2SLEEP> ..");
                                //dspl_Print_Msg((_TOTAL_ROWS - 1), "--- Ga Slapen ---   ", 0);
                                serverClient.println("\n<GO2SLEEP>");
                                pingTime = millis() + _PINGTIME;
                                lineIn = "";
                                endResponse = false;
                                do {
                                    lineIn = readFromKast();
                                    Serial.print(F("==> ["));
                                    Serial.print(lineIn);
                                    Serial.println(F("]"));
                                    //dspl_Print_Msg(_TOTAL_ROWS, lineIn, 1000);
                                    if (lineIn.equals("<OK>")) {
                                        endResponse     = true;
                                        redrawMenu      = _MOVE_LIST;
                                        stillSelecting  = false;
                                        menuId          = 0;
                                    }
                                    if (lineIn.equals("<ERROR>") || lineIn.equals("<TIMEOUT>")) {
                                        dspl_Print_Msg(_TOTAL_ROWS, lineIn, 500);
                                        endResponse = true;
                                    }
                                    clientConnected = false;
                                    dspl_Clear();
                                    dspl_Print_Msg(2, "Disconnect", 2000);
                                    ESP.reset();
                                    yield();
                                
                                } while (!endResponse);
                            }   // confirm()
                            break;
                            
                case 6:  // menu item 6 selected    <lees directory>
                            Serial.println(menuItems[Selection]);
                            Serial.println(F("==> send <LEESDIR> .."));
                            //dspl_Clear();
                            dspl_Print_Msg((_TOTAL_ROWS - 1), "--- Lees Map ---    ", 0);
                            serverClient.println(F("<LEESDIR>"));
                            pingTime = millis() + _PINGTIME;
                            
                            doData      = false;
                            endResponse = false;
                            I = 1;
                            do {
                                lineIn = readFromKast();
                                Serial.print(F("==> ["));
                                Serial.print(lineIn);
                                Serial.println(F("]"));
                                if (lineIn.equals("<OK>")) {
                                    //dspl_Print_Msg(_TOTAL_ROWS, lineIn, 100);
                                    endResponse = true;
                                }
                                if (lineIn.equals("<ERROR>") || lineIn.equals("<TIMEOUT>")) {
                                    dspl_Print_Msg(_TOTAL_ROWS, lineIn, 100);
                                    endResponse = true;
                                }

                                yield();
                                
                            } while (!endResponse);
                            //redrawMenu = _MOVE_LIST;
                            break;
        
    }   // switch(selection)

    return menuId;
    
}       // processHoofdMenu()


uint8_t processSub1Menu(String subItems[], uint8_t Selection) {

    switch(Selection) {     // adding these values together = where on 
                            // menuItems cursor is.
                case 0:     // menu item 1 selected <Terug>
                            Serial.println(subItems[Selection]);
                            stillSelecting = false;
                            menuId = 0;
                            redrawMenu = _MOVE_LIST;
                            break;

                case 1:  // menu item 2 selected    <kastID>
                            Serial.println(subItems[Selection]);
                            tempNr = enterNumber("Kast ID:", actKast.kastID, 1, 254);
                            if (tempNr == -1) return -1;   // long press in enterNumber()
                            if (tempNr != actKast.kastID) {
                                Serial.print("==> Process new value ["); Serial.print(tempNr); Serial.println(F("]"));
                                serverClient.println("<SETKASTID>");
                                serverClient.println(tempNr);
                                serverClient.println("<END>");
                                pingTime = millis() + _PINGTIME;
                                lineIn = "";
                                endResponse = false;
                                do {
                                    lineIn = readFromKast();
                                    Serial.print(F("==> ["));
                                    Serial.print(lineIn);
                                    Serial.println(F("]"));
                                    if (lineIn.equals("<ERROR>") || lineIn.equals("<TIMEOUT>")) {
                                        dspl_Print_Msg(_TOTAL_ROWS, lineIn, 100);
                                        endResponse = true;
                                    }
                                    if (lineIn.equals("<OK>")) {
                                        actKast.kastID = tempNr;
                                        Serial.println(actKast.kastID);
                                        endResponse = true;
                                    }

                                    yield();
                                    
                                } while (!endResponse);
                            }
                            redrawMenu = _MOVE_LIST;

                            //return;
                            break;

                case 2:  // menu item 3 selected    <Interval>
                            Serial.println(subItems[Selection]);
                            // ===> actKast.interval in uren! <==
                            tempNr = enterNumber("Interval:", actKast.interval, 1, 6, 1);
                            if (tempNr == -1) return -1;   // long press in enterNumber()
                            if (tempNr != actKast.interval) {
                                Serial.print("==> Process new value ["); Serial.print(tempNr); Serial.println(F("]"));
                                serverClient.println("<SETINTERVAL>");
                                serverClient.println(tempNr);
                                serverClient.println("<END>");
                                pingTime = millis() + _PINGTIME;
                                lineIn = "";
                                endResponse = false;
                                do {
                                    lineIn = readFromKast();
                                    Serial.print(F("==> ["));
                                    Serial.print(lineIn);
                                    Serial.println(F("]"));
                                    if (lineIn.equals("<ERROR>") || lineIn.equals("<TIMEOUT>")) {
                                        dspl_Print_Msg(_TOTAL_ROWS, lineIn, 100);
                                        endResponse = true;
                                    }
                                    if (lineIn.equals("<OK>")) {
                                        actKast.interval = tempNr;
                                        Serial.println(actKast.interval);
                                        endResponse = true;
                                    }

                                    yield();
                                    
                                } while (!endResponse);
                            }
                            redrawMenu = _MOVE_LIST;
                            break;

                case 3:  // menu item 4 selected
                            //Serial.print("\n==> SubMenu item ");
                            //Serial.print(Selection);
                            //Serial.print(F(" selected - "));
                            //Serial.println(subItems[Selection]);
                            stillSelecting = false;
                            menuId = 2;
                            redrawMenu = _MOVE_LIST;
                            break;

    }   // switch(selection)

    return menuId;

}       // processSub1Menu()


uint8_t processSub2Menu(String subItems[], uint8_t Selection) {

    switch(Selection) { // adding these values together = where on 
                                                            // menuItems cursor is.
                case 0:     // menu item 1 selected
                            //Serial.print(F("\n==> Sub2Menu item "));
                            //Serial.print(Selection);
                            //Serial.print(F(" selected - "));
                            //Serial.println(subItems[Selection]);
                            stillSelecting = false;
                            menuId = 1;
                            redrawMenu = _MOVE_LIST;
                            //return;
                            break;

                case 1:  // menu item 2 selected    <klok - Year>
                            //Serial.print(F("\n==> Sub2Menu item "));
                            //Serial.print(Selection);
                            //Serial.print(F(" selected - "));
                            //Serial.println(subItems[Selection]);
                            tempNr = enterNumber("Jaar:", actKast.year, 2000, 2999);
                            if (tempNr != actKast.year) {
                                Serial.print("==> Process new value ["); Serial.print(tempNr); Serial.println(F("]"));
                                serverClient.println("<SETJAAR>");
                                serverClient.println(tempNr);
                                serverClient.println("<END>");
                                pingTime = millis() + _PINGTIME;
                                lineIn = "";
                                endResponse = false;
                                do {
                                    lineIn = readFromKast();
                                    Serial.print(F("==> ["));
                                    Serial.print(lineIn);
                                    Serial.println(F("]"));
                                    if (lineIn.equals("<ERROR>") || lineIn.equals("<TIMEOUT>")) {
                                        dspl_Print_Msg(_TOTAL_ROWS, lineIn, 100);
                                        endResponse = true;
                                    }
                                    if (lineIn.equals("<OK>")) {
                                        actKast.interval = tempNr;
                                        Serial.println(actKast.interval);
                                        endResponse = true;
                                    }

                                    yield();
                                    
                                } while (!endResponse);
                            }
                            redrawMenu = _MOVE_LIST;
                            //return;
                            break;

                case 2:  // menu item 3 selected    <klok - Month>
                            //Serial.print(F("\n==> Sub2Menu item "));
                            //Serial.print(Selection);
                            //Serial.print(F(" selected - "));
                            //Serial.println(subItems[Selection]);
                            tempNr = enterNumber("Maand:", actKast.month, 1, 12);
                            if (tempNr != actKast.month) {
                                Serial.print("==> Process new value ["); Serial.print(tempNr); Serial.println(F("]"));
                                serverClient.println("<SETMAAND>");
                                serverClient.println(tempNr);
                                serverClient.println("<END>");
                                pingTime = millis() + _PINGTIME;
                                lineIn = "";
                                endResponse = false;
                                do {
                                    lineIn = readFromKast();
                                    Serial.print(F("==> ["));
                                    Serial.print(lineIn);
                                    Serial.println(F("]"));
                                    if (lineIn.equals("<ERROR>") || lineIn.equals("<TIMEOUT>")) {
                                        dspl_Print_Msg(_TOTAL_ROWS, lineIn, 100);
                                        endResponse = true;
                                    }
                                    if (lineIn.equals("<OK>")) {
                                        actKast.month = tempNr;
                                        Serial.println(actKast.month);
                                        endResponse = true;
                                    }

                                    yield();
                                    
                                } while (!endResponse);
                            }
                            redrawMenu = _MOVE_LIST;
                            break;

                case 3:  // menu item 4 selected    <klok - Day>
                            //Serial.print(F("\n==> Sub2Menu item "));
                            //Serial.print(Selection);
                            //Serial.print(F(" selected - "));
                            //Serial.println(subItems[Selection]);
                            tempNr = enterNumber("Dag:", actKast.day, 1, 31);
                            if (tempNr != actKast.day) {
                                Serial.print("==> Process new value ["); Serial.print(tempNr); Serial.println(F("]"));
                                serverClient.println("<SETDAG>");
                                serverClient.println(tempNr);
                                serverClient.println("<END>");
                                pingTime = millis() + _PINGTIME;
                                lineIn = "";
                                endResponse = false;
                                do {
                                    lineIn = readFromKast();
                                    Serial.print(F("==> ["));
                                    Serial.print(lineIn);
                                    Serial.println(F("]"));
                                    if (lineIn.equals("<ERROR>") || lineIn.equals("<TIMEOUT>")) {
                                        dspl_Print_Msg(_TOTAL_ROWS, lineIn, 100);
                                        endResponse = true;
                                    }
                                    if (lineIn.equals("<OK>")) {
                                        actKast.day = tempNr;
                                        Serial.println(actKast.day);
                                        endResponse = true;
                                    }

                                    yield();
                                    
                                } while (!endResponse);
                            }
                            redrawMenu = _MOVE_LIST;
                            break;

                case 4:  // menu item 5 selected    <klok - Hour>
                            //Serial.print(F("\n==> Sub2Menu item "));
                            //Serial.print(Selection);
                            //Serial.print(F(" selected - "));
                            //Serial.println(subItems[Selection]);
                            tempNr = enterNumber("Uur:", actKast.hour, 0, 23);
                            if (tempNr != actKast.hour) {
                                Serial.print("==> Process new value ["); Serial.print(tempNr); Serial.println(F("]"));
                                serverClient.println("<SETUUR>");
                                serverClient.println(tempNr);
                                serverClient.println("<END>");
                                pingTime = millis() + _PINGTIME;
                                lineIn = "";
                                endResponse = false;
                                do {
                                    lineIn = readFromKast();
                                    Serial.print(F("==> ["));
                                    Serial.print(lineIn);
                                    Serial.println(F("]"));
                                    if (lineIn.equals("<ERROR>") || lineIn.equals("<TIMEOUT>")) {
                                        dspl_Print_Msg(_TOTAL_ROWS, lineIn, 100);
                                        endResponse = true;
                                    }
                                    if (lineIn.equals("<OK>")) {
                                        actKast.hour = tempNr;
                                        Serial.println(actKast.hour);
                                        endResponse = true;
                                    }

                                    yield();
                                    
                                } while (!endResponse);
                            }
                            redrawMenu = _MOVE_LIST;
                            break;

                case 5:  // menu item 6 selected
                            tempNr = enterNumber("Minuten:", actKast.minute, 0, 59);
                            if (tempNr != actKast.minute) {
                                Serial.print(F("==>Process new value [")); Serial.print(tempNr); Serial.println(F("]"));
                                serverClient.println(F("<SETMINUUT>"));
                                serverClient.println(tempNr);
                                serverClient.println(F("<END>"));
                                pingTime = millis() + _PINGTIME;
                                lineIn = "";
                                endResponse = false;
                                do {
                                    lineIn = readFromKast();
                                    Serial.print(F("==> ["));
                                    Serial.print(lineIn);
                                    Serial.println(F("]"));
                                    if (lineIn.equals("<ERROR>") || lineIn.equals("<TIMEOUT>")) {
                                        dspl_Print_Msg(_TOTAL_ROWS, lineIn, 100);
                                        endResponse = true;
                                    }
                                    if (lineIn.equals("<OK>")) {
                                        actKast.minute = tempNr;
                                        Serial.println(actKast.minute);
                                        endResponse = true;
                                    }

                                    yield();
                                    
                                } while (!endResponse);
                            }
                            redrawMenu = _MOVE_LIST;
                            break;

    }   // switch(selection)

    return menuId;

}       // processSub2Menu()


uint8_t processKastInfo(String subItems[], uint8_t Selection) {

    switch(Selection) {     // adding these values together = where on 
                            // menuItems cursor is.
                default:    // all menu item's selected
                            stillSelecting = false;
                            menuId = 0;
                            redrawMenu = _MOVE_LIST;
                            break;

    }   // switch(selection)

    return menuId;

}       // processKastInfo()


bool confirm(String noText, String yesText) {

bool prevDoConfirm, doConfirm = false;
bool stillSececting = true;

    do {
        switch(readEncoder()) {
            case 'U':   // ENCODER ROTATED UP. EQUIVALENT OF 'UP' BUTTON PUSHED
                        doConfirm = !doConfirm;
                        break;
                        
            case 'D':   // ENCODER ROTATED UP. EQUIVALENT OF 'UP' BUTTON PUSHED
                        doConfirm = !doConfirm;
                        break;
    
            case 'S':   // ENCODER ROTATED UP. EQUIVALENT OF 'UP' BUTTON PUSHED
                        stillSelecting = false;
                        //swState = _NOKEYPRESSED;
                        break;
                    
        }   // switch()

        if (!doConfirm) { 
            dspl_Print(0,_TOTAL_ROWS, ">"); 
            dspl_Print(1,_TOTAL_ROWS, noText); 
            if (prevDoConfirm != doConfirm) {
                Serial.print(">"); Serial.println(noText); 
                prevDoConfirm = doConfirm;
            }
        } else {
            dspl_Print(0,_TOTAL_ROWS, " "); 
            dspl_Print(1,_TOTAL_ROWS, noText); 
        }
        if (doConfirm) { 
            dspl_Print((_TOTAL_COLS  - (yesText.length() + 2)), _TOTAL_ROWS, ">"); 
            dspl_Print((_TOTAL_COLS  - (yesText.length() + 1)), _TOTAL_ROWS, yesText);
            if (prevDoConfirm != doConfirm) {
                Serial.print(">"); Serial.println(yesText); 
                prevDoConfirm = doConfirm;
            }
        } else {
            dspl_Print((_TOTAL_COLS  - (yesText.length() + 2)), _TOTAL_ROWS, " ");
            dspl_Print((_TOTAL_COLS  - (yesText.length() + 1)), _TOTAL_ROWS, yesText);
        }
        yield();
        
    } while (stillSelecting);

    return doConfirm;
    
}   // confirm()

