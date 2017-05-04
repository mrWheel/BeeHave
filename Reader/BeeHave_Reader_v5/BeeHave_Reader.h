
#ifndef _BEEHAVE_H
    #define _BEEHAVE_H
    
    const char* ssid = "BeeHave";
    const char* password = "geHeim";
    const uint16_t  port = 8266;

    IPAddress local_IP(192,168,4,1);
    IPAddress gateway(192,168,4,99);
    IPAddress subnet(255,255,255,0);

    #define HAS_LCDKEYS false

    // Half-stepV mode?
    //#define HALF_STEP
    // Arduino pins the encoder is attached to. Attach the center to ground.
    #define ROTARY_PIN1     0   // GPIO00, D3
    #define ROTARY_PIN2     2   // GPIO02, D4
    #define ROTARY_SWITCH   3   // GPIO03, RX (!)
    
    #define _SDA            4   // GPIO04, D2
    #define _SCL            5   // GPIO05, D1
    
    #define _MOSI           13  // D7, pin  7
    #define _MISO           12  // D6, pin  6
    #define _CLK            14  // D5, pin  5
    #define _CS             15  // D8, pin 16
    #define SPI_SPEED SD_SCK_MHZ(10)
    
    // define to enable weak pullups.
    #define ENABLE_PULLUPS
    #define _LONGPRESSTIME  500
    #define _SHORTPRESSTIME 100
    enum  { _NOKEYPRESSED, _LONGPRESS, _SHORTPRESS };

    #define _PINGTIME   6000000     /* 60 seconden */
    #define _TIMEOUT    2000        /* milliSeconden */

#endif
