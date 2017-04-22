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

void flowbar(short gx, short gy){
	int x_length, y_length;
	x_length = (int)((float)gx/16500*64);
	y_length = (int)((float)gy/16500*64);
	int abs_x_length = x_length, abs_y_length = y_length;
	
	if(x_length<0){
		abs_x_length = -x_length;
	}
	if(y_length<0){
		abs_y_length = -y_length;
	}
	int i,j;
	clearBar(0,128,64-BARWIDTH,64+BARWIDTH,WHITE);
	clearBar(64-BARWIDTH,64+BARWIDTH,0,128,WHITE);
	for (i=0; i<abs_x_length; i++){
		for(j=0; j<BARWIDTH; j++){
			LCD_drawPixel(-(x_length/abs_x_length)*i+64,j+64-BARWIDTH/2,RED);
		}
	}
	for (i=0; i<abs_y_length; i++){
		for(j=0; j<BARWIDTH; j++){
			LCD_drawPixel(j+64-BARWIDTH/2,-(y_length/abs_y_length)*i+64,BLUE);
		}
	}
}
