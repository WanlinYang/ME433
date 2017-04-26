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

#define STRLENGTH 100
#define FREQUENCY 0.5

void I2C_read_multiple(unsigned char address, unsigned char regis, unsigned char *data, int length);
void IMUinit(void);


int main() {
    // do your TRIS and LAT commands here
	TRISAbits.TRISA4 = 0;	  // RA4 as output
	TRISBbits.TRISB4 = 1;     // RB4 as input
	LATAbits.LATA4 = 1;       // RA4 is high
	
	SPI1_init();
	LCD_init();
	LCD_clearScreen(WHITE);
	IMUinit();	
	

	unsigned char data[STRLENGTH];
	short data_combine[STRLENGTH];
	long count = 24000000/(FREQUENCY*100);
	int i = 0;
	
	while(1){
		_CP0_SET_COUNT(0);
		while (_CP0_GET_COUNT()<count) {;}
		
		I2C_read_multiple(0x6a, 0x20, data, 14);

		for(i=0; i<7; i++){
			data_combine[i] = data[2*i+1]<<8 | data[2*i];
		}
/*		data_combine:
		0: temperature
		1: gyroscope x
		2: gyroscope y
		3: gyroscope z
		4: accelerometer x
		5: accelerometer y
		6: accelerometer z
*/

/*		char message[STRLENGTH];

		clearBar(45,120,50,69,WHITE);
		sprintf(message,"x:%d",data_combine[4]);
		print_string(message,48,50,RED);
		
		clearBar(45,120,70,89,WHITE);
		sprintf(message,"y:%d",data_combine[5]);
		print_string(message,48,70,RED);
		
		clearBar(45,120,90,109,WHITE);
		sprintf(message,"z:%d",data_combine[6]);
		print_string(message,48,90,RED);
*/		
		flowbar(data_combine[4], data_combine[5]);
		
	}
}


void I2C_read_multiple(unsigned char address, unsigned char regis, unsigned char *data, int length){
	i2c_master_start();
	unsigned char write = address<<1 | 0;        // address is 0xd4
	unsigned char read = address<<1 | 1;

	i2c_master_send(write);
	i2c_master_send(regis);         //for OUT_TEMP_L, register is 0x20
	i2c_master_restart();
	i2c_master_send(read);
	
	int i = 0;
	for(i=0; i<length; i++){
		*data++ = i2c_master_recv();
		if(i==(length-1)){
			i2c_master_ack(1);  // make the ack so the slave knows we got it
			break;
		}
		i2c_master_ack(0);      // continue to read
	}         
	i2c_master_stop();
}

void IMUinit(void){
	ANSELBbits.ANSB2 = 0;    // turn off analog
	ANSELBbits.ANSB3 = 0;
	i2c_master_setup();
/*	i2c_master_start();
	i2c_master_send(0xd4);  // (SAD+) write mode
	i2c_master_send(0x0f);  // want to read WHOAMI address
	i2c_master_restart();
	i2c_master_send(0xd5);  // (SAD+) read mode
	char who_am_i = i2c_master_recv(); // receive
	i2c_master_ack(1);
	i2c_master_stop();
		
	char message[STRLENGTH];
	sprintf(message,"0x%0x",who_am_i);
	print_string(message,48,32,BLACK);   // should be 0x69 or 0b01101001	
*/
	
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
}
