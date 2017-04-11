#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<math.h>
#include"spi.h"

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

#define CS LATBbits.LATB15  // chip select pin
#define SINWAVELENGTH 100
#define TRIWAVELENGTH 200
#define PI 3.14

void makewave(void);
static volatile unsigned char sinwave[SINWAVELENGTH];
static volatile unsigned char triwave[TRIWAVELENGTH];
int main() {
	makewave();
    // do your TRIS and LAT commands here
	TRISAbits.TRISA4 = 0;	  // RA4 as output
	TRISBbits.TRISB4 = 1;     // RB4 as input
	LATAbits.LATA4 = 1;      // RA4 is high
	initSPI1();
	
	char channel = 0;
	int i_sin = 0;
	int i_tri = 0;
	while (1){
		_CP0_SET_COUNT(0);
		
		while(_CP0_GET_COUNT()<24000){   // one stage
			channel = 0;
			write_dac(channel, sinwave[i_sin]);
			channel = 1;
			write_dac(channel, triwave[i_tri]);
		}
		i_sin++;
		i_tri++;
		if(i_sin == SINWAVELENGTH){
			i_sin = 0;
		}
		if(i_tri == TRIWAVELENGTH){
			i_tri = 0;
		}
	}

}

void makewave(){
	int i = 0;
	for (i=0; i<SINWAVELENGTH; i++){
		sinwave[i] = 127*sin((double)2*PI*i/(SINWAVELENGTH))+128;
	}

	for (i=0; i<TRIWAVELENGTH; i++){
		triwave[i] = 255*((double)i/TRIWAVELENGTH);
	}
}