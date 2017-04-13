#ifndef I2C_H
#define I2C_H

void initExpander(void);
void setExpander(char pin, char level);
char getExpander(void);

#endif