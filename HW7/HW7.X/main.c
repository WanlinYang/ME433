#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include "i2c.h"
#include "lcd.h"

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

void I2C_read_multiple(unsigned char address, unsigned char register, unsigned char * data, int length){
	
}

int main() {
    // do your TRIS and LAT commands here
	TRISAbits.TRISA4 = 0;	  // RA4 as output
	TRISBbits.TRISB4 = 1;     // RB4 as input
	LATAbits.LATA4 = 1;      // RA4 is high
	
	SPI1_init();
	LCD_init();
	LCD_clearScreen(WHITE);
	
 	ANSELBbits.ANSB2 = 0;    // turn off analog
	ANSELBbits.ANSB3 = 0;
	i2c_master_setup();
	i2c_master_start();
	i2c_master_send(0xd4);  // (SAD+) write mode
	i2c_master_send(0x0f);  // want to read WHOAMI address
	i2c_master_restart();
	i2c_master_send(0xd5);  // (SAD+) read mode
	char r = i2c_master_recv(); // receive
	i2c_master_ack(1);
	i2c_master_stop();
		
	char str[50];
	sprintf(str,"0x%0x",r);
	print_string(str,48,32,BLACK);
	
	i2c_master_start();     // turn on the accelerometer
	i2c_master_send(0xd4);  // write mode
	i2c_master_send(0x10);  // CTRL1_XL register
	i2c_master_send(0x82);  // sample rate to 1.66 kHz, with 2g sensitivity, and 100 Hz filter
	i2c_master_stop();
	
	i2c_master_start();     // turn on the gyroscope
	i2c_master_send(0xd4);  // write mode
	i2c_master_send(0x11);  // CTRL2_G register
	i2c_master_send(0x88);  // sample rate to 1.66 kHz, with 1000 dps sensitivity
	i2c_master_stop();
	
	i2c_master_start();     // CTRL3_C register, which contains the IF_INC bit.
	i2c_master_send(0xd4);  // write mode
	i2c_master_send(0x04);  // only IF_INC is 1
	i2c_master_stop();
	
	
	while(1){

		
/* 		i2c_master_start();
		i2c_master_send(0x40);
		i2c_master_send(0x0A);
		i2c_master_send(0b1);
		i2c_master_stop();  */
		
		//char level = getExpander();   // value of pin 7
		//setExpander(0, level);
		
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
