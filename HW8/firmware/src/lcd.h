#ifndef LCD_H
#define LCD_H

#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
#include "ILI9163C.h"

#define BARWIDTH 5

void print_char(char ch, unsigned short x, unsigned short y, unsigned short color);
void print_string(char str[], unsigned short x, unsigned short y, unsigned short color);
void clearBar(unsigned short xmin, unsigned short xmax, unsigned short ymin, unsigned short ymax, unsigned short color);
void flowbar(short gx, short gy);
//void bar(unsigned short x, unsigned short y, unsigned short color, unsigned short back_color);


#endif