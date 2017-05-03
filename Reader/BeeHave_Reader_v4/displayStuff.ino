
#include "font.h"

char dspl_Buf[_TOTAL_ROWS + 1][_TOTAL_COLS + 1];

uint8_t dRow, dCol;

void dspl_Init() {
    display.init();
    display.resetDisplay();
    display.flipScreenVertically();
    display.displayOn();
  //display.setFont(ArialMT_Plain_16);
    display.setFont(DejaVu_LGC_Sans_Mono_13);
    display.setTextAlignment(TEXT_ALIGN_LEFT);

}   // dspl_Init()

void dspl_Clear() {
    memset(dspl_Buf,' ',sizeof(dspl_Buf));
    //display.clear();
    for(int R = 0; R <= _TOTAL_ROWS; R++) {
        dspl_Buf[R][_TOTAL_COLS] = '\0';
    }

}   // dspl_Clear


void dspl_Print(uint8_t pos, uint8_t line, String message) {
    
    uint8_t mPos;

    if (pos  < 0 || pos  > _TOTAL_COLS) { Serial.printf("pos error [%d]\n", pos); return; }
    if (line < 0 || line > _TOTAL_ROWS) { Serial.printf("line error [%d]\n", line); return; }

    for(mPos = 0; ((mPos < message.length()) && ((mPos + pos) < _TOTAL_COLS)); mPos++) {
        dspl_Buf[line][(mPos + pos)] = message[mPos];
    }
    dspl_Buf[line][_TOTAL_COLS] = '\0';
    display.clear();
    for(dRow = 0; dRow <= _TOTAL_ROWS; dRow++) {
        display.drawString(0, (dRow * 15), String(dspl_Buf[dRow]));
        //Serial.printf("[%d] [%s]\n", dRow, dspl_Buf[dRow]); // << geeft ERG veel output!!!
    }
    display.display();
    
}   // dspl_Print()

void dspl_Print_Msg(uint8_t line, String message, uint16_t wait) {
    message += "                    ";
    dspl_Print(0, line, message);
    delay(wait);
    redrawMenu = _MOVE_LIST;
    
}   // dspl_Print_Msg()



void progressBar(uint8_t line, uint8_t mode) {
    
    if (line < 0 || line > _TOTAL_ROWS) line = _TOTAL_ROWS;
    
    switch (mode) {
        case BAR_INIT:  dspl_Print_Msg(line, "                    ", 0);
                        barPos = 0;
                        barDirection = 1;
                        break;
        case BAR_TICK:  barPos = barPos + barDirection; 
                        if (barDirection == 1) {
                            if (barPos > (_TOTAL_COLS - 3)) {
                                barDirection = -1; 
                                barPos = _TOTAL_COLS - 3;
                                dspl_Print(barPos, line, "<o ");
                            } else {
                                dspl_Print(barPos, line, " o>");
                            }
                        } else {
                            if (barPos <= 0) {
                                barDirection = 1; 
                                barPos = 0;
                                dspl_Print(barPos, line, " o>");
                            } else {
                                dspl_Print(barPos, line, "<o ");
                            }
                        }   // (barDirection) 
                        break;
    }   // switch(mode)
    
}   // progressBar()


