/*
 * wacht op response van bijenkast
 */

#define     _MAX_RESPONSE   70

uint32_t    waitTime;
uint32_t    keepCommAlive;
char        response[_MAX_RESPONSE + 2];

char* readFromKast() {
    
    char     c;
    uint8_t  rPos;

    waitTime        = millis() + _TIMEOUT;
    rPos            = 0;
    memset(response,0,sizeof(response));
    
    while (waitTime > millis()) {
        if (serverClient.available()) {
            c = serverClient.read();
            if (c == '\n' || c == '\r' || c == '\0') {     // end reading response
                if (rPos > 0) {
                    if(strcmp(response, "<GO2SLEEP>") == 0) {
                        dspl_Clear();
                        dspl_Print_Msg(2, "lost Connection", 1000);
                        ESP.reset();
                    }
                    return response;
                }
                waitTime  = millis() + _TIMEOUT;
                pingTime = millis() + _PINGTIME;
            }
            if ( c >= 32 && c <= 126 ) { 
                response[rPos++] = c;
                waitTime  = millis() + _TIMEOUT;
                pingTime = millis() + _PINGTIME;
            }
            if (rPos >= _MAX_RESPONSE) {
                sprintf(response, "<POS %d>\r<OVRFLW>", rPos);
                return response;
            }
        
        }   // if (serverClient.available())
        
        yield();
        
    }   // while(waitTime > millis()) 

    return "<TIMEOUT>";

}   // readFromKast()


uint16_t getParm() {
    uint32_t  parm;
    String    endString;
    char      c;
    bool      getEnd;

    parm = 0;
    getEnd = false;
    keepCommAlive = millis() + _TIMEOUT;

    parm = serverClient.parseInt();
    
    while (keepCommAlive > millis()) {
        if (serverClient.available()) {
            // wait for "<" lead-in
            pingTime = millis() + _PINGTIME;
            c = serverClient.read();
            if (c == '<') {     // start reading command
                endString = "";
                getEnd = true;
                keepCommAlive = millis() + _TIMEOUT;
            }
            if (getEnd) {
                endString += c;
                if (c == '>') {     // command complete
                    if (endString.equals("<END>")) {
                        return parm;
                    }
                }
                if (endString.length() > 100) {
                    serverClient.println("<ERROR>");
                    parm = -1;
                    return parm;    
                }
                keepCommAlive = millis() + _TIMEOUT;
            } 
        }   // if (serverClient.available)
        yield();            
    }   // while (timeout?)
    
 }  //  getParm()

