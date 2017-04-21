#include "lcd.h"

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

/* void bar(unsigned short x, unsigned short y, unsigned short color, unsigned short back_color){
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
} */