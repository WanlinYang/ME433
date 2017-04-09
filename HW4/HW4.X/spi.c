#include"spi.h"
#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro

void initSPI1(void){
	RPB15Rbits.RPB15R = 0b0011;  // B15 pin as SS1
	SDI1Rbits.SDI1R = 0b0100;    // B8 pin as SDI1
	RPB13Rbits.RPB13R = 0b0011;  // B13 pin as SDO1
	// B14 pin as SCK
	
	    // SPI initialization for reading from the decoder chip
    SPI1CON = 0;              // stop and reset SPI4
    SPI1BUF;                  // read to clear the rx receive buffer
    SPI1BRG = 0x1;            // bit rate to 8 MHz, SPI4BRG = 80000000/(2*desired)-1
    SPI1STATbits.SPIROV = 0;  // clear the overflow
    SPI1CONbits.MSTEN = 1;    // master mode
    SPI1CONbits.MSSEN = 1;    // slave select enable
    //SPI1CONbits.MODE16 = 1;   // 16 bit mode
    //SPI1CONbits.MODE32 = 0; 
    SPI1CONbits.SMP = 1;      // sample at the end of the clock
    SPI1CONbits.ON = 1;       // turn SPI on
}