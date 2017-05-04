
/*
REQUIRED GLOBAL VARIABLES & DEFINITIONS
*/
#define _MOVE_CURSOR    1       // constants for indicating whether cursor should be redrawMenun
#define _MOVE_LIST      2       // constants for indicating whether cursor should be redrawMenun
#define _MENUTIMEOUT    60000   // time to timeout in a menu when user doesn't do anything.
#define _TOTAL_ROWS     3       // total rows of LCD
#define _TOTAL_COLS     16      // total columns of LCD
unsigned long menuTimeOut = 0;  // this is set and compared to millis to see when the user last did something.

