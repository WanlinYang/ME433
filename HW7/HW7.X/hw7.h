#ifndef I2C_H
#define I2C_H

#include "i2c.h"
#include "lcd.h"

void initExpander(void);
void setExpander(int pin, char level);
char getExpander(void);

#endif