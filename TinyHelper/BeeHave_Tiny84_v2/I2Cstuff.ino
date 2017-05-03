/*
 * Basically the arduino pin numbers map directly to the PORTB bit numbers.
 *
 *  // I2C
 *  arduino D0 = not(OC1A) = PORTB <- _BV(0) = SOIC DIL 5 (I2C SDA, PWM)
 *  arduino D2 =           = PORTB <- _BV(2) = SOIC DIL 7 (I2C SCL, Analog 1)
 *  // Timer1 -> PWM
 *  arduino D1 =     OC1A  = PORTB <- _BV(1) = SOIC DIL 6 (PWM)
 *  arduino D3 = not(OC1B) = PORTB <- _BV(3) = SOIC DIL 2 (Analog 3)
 *  arduino D4 =     OC1B  = PORTB <- _BV(4) = SOIC DIL 3 (Analog 2)
*/
//
// SETUP: ATTiny85
// AtTiny D0 - DIL 5 (PB0/SDA) = I2C SDA 
//     connect to SDA on master with external pull-up (~4.7K)
// AtTiny D2 - DIL 7 (PB2/SCL) = I2C SCL 
//     connect to SCL on master with external pull-up (~4.7K)
// AtTiny    - DIL 1 (PB5/!RST)
//     connect to reset on master (or just pull-up)
//
// SETUP: ATTiny84
// AtTiny D6 - DIL 7 (PA6/SDA) = I2C SDA 
//     connect to SDA on master with external pull-up (~4.7K)
// AtTiny D4 - DIL 9 (PA4/SCL) = I2C SCL 
//     connect to SCL on master with external pull-up (~4.7K)
// AtTiny    - DIL 4 (PB3/!RST)
//     connect to reset on master (or just pull-up)

// The default buffer size
#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE ( 16 )
#endif

volatile uint8_t i2c_regs[TWI_RX_BUFFER_SIZE];

// Tracks the current register pointer position
volatile byte reg_position;
const byte reg_size = sizeof(i2c_regs);

/**
 * This is called for each read request we receive, never put more than 
 * one byte of data (with TinyWireS.send) to the send-buffer when using 
 * this callback
 */
void requestEvent() {  
    I2Ctimer = millis() + I2CtimerWait; // keep alive time
    TinyWireS.send(i2c_regs[reg_position]);
    // Increment the reg position on each read, and loop back to zero
    reg_position++;
    if (reg_position >= reg_size) {
        reg_position = 0;
    }
}   // requestEvent()


/**
 * The I2C data received -handler
 *
 * This needs to complete before the next incoming transaction (start, 
 * data, restart/stop) on the bus does, so be quick, set flags for long 
 * running tasks to be called from the mainloop instead of running them 
 * directly,
 */
void receiveEvent(uint8_t howMany) {
    I2Ctimer = millis() + I2CtimerWait;
    if (howMany < 1) {
        // Sanity-check
        return;
    }
    if (howMany > TWI_RX_BUFFER_SIZE) {
        // Also insane number
        return;
    }

    reg_position = TinyWireS.receive();
    howMany--;
    if (!howMany) {
        // This write was only to set the buffer for next read
        return;
    }
    while(howMany--) {
        byte bIn = TinyWireS.receive();
        if (reg_position == 0x0E) {             // address 14(DEC) it's a command
            if (bcdToDec(bIn) == 12) {          // read Vcc
                doReadVcc = true;
                reg_position = 0;
                return;
            } else if (bcdToDec(bIn) == 21) {   // erase all regs
                memset(i2c_regs,0,sizeof(i2c_regs));
                reg_position = 0;
                return;
            }
        } else {
            i2c_regs[reg_position] = bIn;
        }
        reg_position++;
        if (reg_position >= reg_size) {
            reg_position = 0;
        }
    }
    
}   // receiveEvent()


void regInit() {
    memset(i2c_regs,0,sizeof(i2c_regs));

}   // regInit()

