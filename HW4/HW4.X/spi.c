#include"spi.h"
#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro

#define CS LATBbits.LATB15  // chip select pin

char SPI1_IO(char write){
	SPI1BUF = write;        // write is sent data
	while(!SPI1STATbits.SPIRBF){;}  // wait to receive the byte
	return SPI1BUF;
}

void write_dac(char channel,unsigned char voltage){
	unsigned char b1;
	unsigned char b2;
	b1 = channel << 7;    // channel select, 0 to channel A, 1 to channel B
	b1 = b1|0b01110000;
	b1 = b1|(voltage >> 4);
	b2 = voltage << 4;

	CS = 0;	
	SPI1_IO(b1);
	SPI1_IO(b2);
	CS = 1;
}

void initSPI1(void){
//	RPB15Rbits.RPB15R = 0b0011;  // B15 pin as SS1
	SDI1Rbits.SDI1R = 0b0100;    // B8 pin as SDI1
	RPB13Rbits.RPB13R = 0b0011;  // B13 pin as SDO1
	// B14 pin as SCK
	
 	// SPI initialization for reading from the decoder chip
    SPI1CON = 0;              // stop and reset SPI1
    SPI1BUF;                  // read to clear the rx receive buffer
    SPI1BRG = 0x1;            // bit rate to 8 MHz, SPI4BRG = 80000000/(2*desired)-1
    SPI1STATbits.SPIROV = 0;  // clear the overflow
    SPI1CONbits.MSTEN = 1;    // master mode
//  SPI1CONbits.MSSEN = 1;    // slave select enable
//  SPI1CONbits.MODE16 = 1;   // 16 bit mode      these two commands are not necessary
//  SPI1CONbits.MODE32 = 0; 
//	SPI1CONbits.CKP = 1;      // clock is idle when high, active when low
	SPI1CONbits.CKE = 1;      // data changes when clock goes from hi to lo
//  SPI1CONbits.SMP = 1;      // sample at the end of the clock
    SPI1CONbits.ON = 1;       // turn SPI on
	
	TRISBbits.TRISB15 = 0;
}