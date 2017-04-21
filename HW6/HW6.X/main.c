#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
#include"ILI9163C.h"

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

#define STRINGLENTH 100
#define BARWIDTH 10
#define FREQUENCY 5

void print_char(char ch, unsigned short x, unsigned short y, unsigned short color);
void print_string(char str[], unsigned short x, unsigned short y, unsigned short color);
void bar(unsigned short x, unsigned short y, unsigned short color, unsigned short back_color);
void clearBar(unsigned short xmin, unsigned short xmax, unsigned short ymin, unsigned short ymax, unsigned short color);

int main() {
    // do your TRIS and LAT commands here
	TRISAbits.TRISA4 = 0;	  // RA4 as output
	TRISBbits.TRISB4 = 1;     // RB4 as input
	LATAbits.LATA4 = 1;      // RA4 is high
	
	SPI1_init();
	LCD_init();
	LCD_clearScreen(0xffff);
	char str[STRINGLENTH];
	sprintf(str, "Hello World%%d!");
	print_string(str,28,32,0x0000);
	
	while(1){
		bar(10,50,0xf000,0xffff);
	}
	
}

void print_char(char ch, unsigned short x, unsigned short y, unsigned short color){
	char row;
	unsigned short i,j;
	for (j=0; j<=15; j++){  // jth row of the char
		for (i=0; i<=4; i++){  // ith pixel of the row
			row = ASCII[ch-0x20][i];
			if ( (row>>j)&0x01 ){
				LCD_drawPixel(i+x,j+y,color);
			}
		}
	}
}

void print_string(char str[], unsigned short x, unsigned short y, unsigned short color){
	char ch = str[0];
	int i = 0, j = 0;       // ith character of string, jth pixel of the entire row
	while (1){
		ch = str[i];
		if (ch=='\0')
			break;
		print_char(ch, x+j, y, color);
		j = j+6;            // 1 pixel between two characters
		i++;
	}
}

void clearBar(unsigned short xmin, unsigned short xmax, unsigned short ymin, unsigned short ymax, unsigned short color){
	unsigned short i,j;
	for(i=xmin; i<=xmax; i++){
		for(j=ymin; j<=ymax; j++){
			LCD_drawPixel(i,j,color);
		}
	}
}

void bar(unsigned short x, unsigned short y, unsigned short color, unsigned short back_color){
	unsigned short i,j;
	long int count = 24000000/(FREQUENCY*100);
	for (i=0; i<=100; i++){
		for (j=0; j<=BARWIDTH; j++){
			LCD_drawPixel(i+x,y+j,color);
		}
		char ch1[10],ch2[10];
		sprintf(ch1,"%d",i);
		print_string(ch1,x+45,y+5+BARWIDTH,color);
		
		clearBar(x+47,x+90,y+BARWIDTH+25,y+BARWIDTH+41,back_color);
		sprintf(ch2,"FPS:%.2f",(double)_CP0_GET_COUNT()/count);
		print_string(ch2,x+25,y+25+BARWIDTH,color);
		
		_CP0_SET_COUNT(0);
		while (_CP0_GET_COUNT()<count) {;}
		clearBar(x+45,x+70,y+BARWIDTH+5,y+BARWIDTH+21,back_color);
	}
	clearBar(x,x+100,y,y+BARWIDTH,back_color);
}









