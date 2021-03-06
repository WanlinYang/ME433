#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include "i2c.h"
#include "hw5.h"

// DEVCFG0
#pragma config DEBUG = OFF // no debugging
#pragma config JTAGEN = OFF // no jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // no write protect
#pragma config BWP = OFF // no boot write protect
#pragma config CP = OFF // no code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // turn off secondary oscillator
#pragma config IESO = OFF // no switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // free up secondary osc pins
#pragma config FPBDIV = DIV_1 // divide CPU freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // do not enable clock switch
#pragma config WDTPS = PS1048576 // slowest wdt
#pragma config WINDIS = OFF // no wdt window
#pragma config FWDTEN = OFF // wdt off by default
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the CPU clock to 48MHz
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz
#pragma config UPLLIDIV = DIV_2 // divider for the 8MHz input clock, then multiply by 12 to get 48MHz for USB
#pragma config UPLLEN = ON // USB clock on

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations
#pragma config FUSBIDIO = ON // USB pins controlled by USB module
#pragma config FVBUSONIO = ON // USB BUSON controlled by USB module



int main() {
    // do your TRIS and LAT commands here
	TRISAbits.TRISA4 = 0;	  // RA4 as output
	TRISBbits.TRISB4 = 1;     // RB4 as input
	LATAbits.LATA4 = 0;      // RA4 is high
	
	initExpander();
	
/* 	ANSELBbits.ANSB2 = 0;    // turn off analog
	ANSELBbits.ANSB3 = 0;
	i2c_master_setup();
	i2c_master_start();
	i2c_master_send(0x40); // write mode
	i2c_master_send(0x00);  // IODIR
	i2c_master_send(0xf0);  // pin 0-3 are output, 4-7 are input
	i2c_master_stop();  */
	
	while(1){
/* 		i2c_master_start();
		i2c_master_send(0x40);
		i2c_master_send(0x0A);
		i2c_master_send(0b1);
		i2c_master_stop();  */
		
		char level = getExpander();   // value of pin 7
		setExpander(0, level);
		
/*  	i2c_master_start();
		i2c_master_send(0x40);
		i2c_master_send(0x09);           // read from GPIO
		i2c_master_restart();
		i2c_master_send(0x41);
		char r = (i2c_master_recv()>>7);      // save the value returned
		i2c_master_ack(1);               // make the ack so the slave knows we got it
		i2c_master_stop();
		
		i2c_master_start();
		i2c_master_send(0x40);   	 // A0 to A2 are 0, write mode
		i2c_master_send(0x0A);    	 // GPIO
		if (r==0)
			i2c_master_send(0b0);       // value of GPIO, pin is a binary command having 8 bits
		else if (r==1)
			i2c_master_send(0b1);
		i2c_master_stop(); */
	}
}
