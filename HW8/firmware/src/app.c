/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include "app.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;
	TRISAbits.TRISA4 = 0;	  // RA4 as output
	TRISBbits.TRISB4 = 1;     // RB4 as input
	LATAbits.LATA4 = 1;       // RA4 is high
	
	SPI1_init();
	LCD_init();
	LCD_clearScreen(WHITE);
	IMUinit();
    
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            bool appInitialized = true;
       
        
            if (appInitialized)
            {
            
                appData.state = APP_STATE_SERVICE_TASKS;
            }
            break;
        }

        case APP_STATE_SERVICE_TASKS:
        {
			unsigned char data[STRLENGTH];
			short data_combine[STRLENGTH];
			long count = 24000000/(FREQUENCY);
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
				flowbar(data_combine[4], data_combine[5]);
		
	}
        }

        /* TODO: implement your application state machine.*/
        

        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
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

/*******************************************************************************
 End of File
 */
