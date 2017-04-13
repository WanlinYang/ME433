#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include"i2c.h"
#include"hw5.h"

void initExpander(){
	i2c_master_start();   // make the start bit
}

void setExpander(char pin, char level){
	
}

char getExpander(){
	
}

/*you want to write the value 123 to register 5 in the chip with address 12*/

	i2c_master_setup();      // enable I2C module
	
	i2c_master_start();      // make the start bit

	i2c_master_send(12<<1|0); // write the address, shifted left by 1, or'ed with a 0 to indicate writing

	i2c_master_send(5);      // the register to write to

	i2c_master_send(123);    // the value to put in the register

	i2c_master_stop();       // make the stop bit
	
/**********************************************************/

/*To read the value from register 7 in a chip with address 12, the code would look like*/

	i2c_master_start();      // make the start bit

	i2c_master_send(12<<1|0); // write the address, shifted left by 1, or'ed with a 0 to indicate writing

	i2c_master_send(7);      // the register to read from

	i2c_master_restart();    // make the restart bit

	i2c_master_send(12<1|1); // write the address, shifted left by 1, or'ed with a 1 to indicate reading

	char r = i2c_master_recv(); // save the value returned

	i2c_master_ack(1);       // make the ack so the slave knows we got it

	i2c_master_stop();       // make the stop bit