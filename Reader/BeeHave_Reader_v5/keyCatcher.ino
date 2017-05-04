
#if HAS_LCDKEYS == true

#define _RIGHT    50
#define _UP      120
#define _DOWN    300
#define _LEFT    450
#define _SELECT  700

char getKey() {
  
  int x;
  x = analogRead (0);
  if (x < _RIGHT) {
    //Serial.println("<R>");
    while (x < _RIGHT) { x = analogRead(0); delay(100); }
    return 'R';
  }
  else if (x < _UP) {
    //Serial.println("<U>");
    while (x < _UP) { x = analogRead(0); delay(100); }
    return 'U';
  }
  else if (x < _DOWN){
    //Serial.println("<D>");
    while (x < _DOWN) { x = analogRead(0); delay(100); }
    return 'D';
  }
  else if (x < _LEFT){
    //Serial.println("<L>");
    while (x < _LEFT) { x = analogRead(0); delay(100); }
    return 'L';
  }
  else if (x < _SELECT){
    //Serial.println("<S>");
    while (x < _SELECT) { x = analogRead(0); delay(100); }
    return 'S';
  }
  return ' ';

}   // getKey()

#endif
