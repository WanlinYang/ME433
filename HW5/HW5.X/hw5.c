#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include"i2c.h"
#include"hw5.h"


void initExpander(){
	ANSELBbits.ANSB2 = 0;
    ANSELBbits.ANSB3 = 0;
	i2c_master_setup();
	i2c_master_start();   // make the start bit
	i2c_master_send(0x40); // write mode
	i2c_master_send(0x00);  // IODIR
	i2c_master_send(0xf0);  // pin 0-3 are output, 4-7 are input
	i2c_master_stop();
} 

void setExpander(int pin, char level){   // read level, send value to pin "pin" (0-7)
	i2c_master_start();
	i2c_master_send((0x20<<1)|0);   	 // A0 to A2 are 0, write mode
	i2c_master_send(0x0A);    			 // GPIO
	if (level==0){
		i2c_master_send(0b0<<pin);       // value of GPIO, pin is a binary command having 8 bits
	}
	else if (level==1){
		i2c_master_send(0b1<<pin);
	}
	i2c_master_stop();
}

char getExpander(){
	i2c_master_start(); 
	i2c_master_send(0x40);           // write firstly
	i2c_master_send(0x09);           // read from GPIO
	
	i2c_master_restart();
	i2c_master_send(0x41);           // read mode
	char r = i2c_master_recv()>>7;      // save the value returned
	i2c_master_ack(1);               // make the ack so the slave knows we got it
	i2c_master_stop();
	return r;
}

/*you want to write the value 123 to register 5 in the chip with address 12*/
/*
	i2c_master_setup();      // enable I2C module
	
	i2c_master_start();      // make the start bit

	i2c_master_send(12<<1|0); // write the address, shifted left by 1, or'ed with a 0 to indicate writing

	i2c_master_send(5);      // the register to write to

	i2c_master_send(123);    // the value to put in the register

	i2c_master_stop();       // make the stop bit
*/	
/**********************************************************/

/*To read the value from register 7 in a chip with address 12, the code would look like*/
/*
	i2c_master_start();      // make the start bit

	i2c_master_send(12<<1|0); // write the address, shifted left by 1, or'ed with a 0 to indicate writing

	i2c_master_send(7);      // the register to read from

	i2c_master_restart();    // make the restart bit

	i2c_master_send(12<<1|1); // write the address, shifted left by 1, or'ed with a 1 to indicate reading

	char r = i2c_master_recv(); // save the value returned

	i2c_master_ack(1);       // make the ack so the slave knows we got it

	i2c_master_stop();       // make the stop bit
*/