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
#include <stdio.h>
#include <xc.h>

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************


uint8_t APP_MAKE_BUFFER_DMA_READY dataOut[APP_READ_BUFFER_SIZE];
uint8_t APP_MAKE_BUFFER_DMA_READY readBuffer[APP_READ_BUFFER_SIZE];
int len, i = 0;
int startTime = 0;
int ii = 0;
//int data1 = 0, data2 = 0, i_data = 0;
static int error = 0, eint = 0, edot = 0, preve = 0;
static int ref_pwm = 0;
static float Kp = 3, Ki = 0, Kd = 0;
static int COM = 0, pre_COM = 0;
//int pwm1 = 0, pwm2 = 0;
char rx[100];
int his_length = 3;
int COM_his[3], h_i = 0;
int direction_l = 0, direction_r = 0;

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

/*******************************************************
 * USB CDC Device Events - Application Event Handler
 *******************************************************/

USB_DEVICE_CDC_EVENT_RESPONSE APP_USBDeviceCDCEventHandler
(
        USB_DEVICE_CDC_INDEX index,
        USB_DEVICE_CDC_EVENT event,
        void * pData,
        uintptr_t userData
        ) {
    APP_DATA * appDataObject;
    appDataObject = (APP_DATA *) userData;
    USB_CDC_CONTROL_LINE_STATE * controlLineStateData;

    switch (event) {
        case USB_DEVICE_CDC_EVENT_GET_LINE_CODING:

            /* This means the host wants to know the current line
             * coding. This is a control transfer request. Use the
             * USB_DEVICE_ControlSend() function to send the data to
             * host.  */

            USB_DEVICE_ControlSend(appDataObject->deviceHandle,
                    &appDataObject->getLineCodingData, sizeof (USB_CDC_LINE_CODING));

            break;

        case USB_DEVICE_CDC_EVENT_SET_LINE_CODING:

            /* This means the host wants to set the line coding.
             * This is a control transfer request. Use the
             * USB_DEVICE_ControlReceive() function to receive the
             * data from the host */

            USB_DEVICE_ControlReceive(appDataObject->deviceHandle,
                    &appDataObject->setLineCodingData, sizeof (USB_CDC_LINE_CODING));

            break;

        case USB_DEVICE_CDC_EVENT_SET_CONTROL_LINE_STATE:

            /* This means the host is setting the control line state.
             * Read the control line state. We will accept this request
             * for now. */

            controlLineStateData = (USB_CDC_CONTROL_LINE_STATE *) pData;
            appDataObject->controlLineStateData.dtr = controlLineStateData->dtr;
            appDataObject->controlLineStateData.carrier = controlLineStateData->carrier;

            USB_DEVICE_ControlStatus(appDataObject->deviceHandle, USB_DEVICE_CONTROL_STATUS_OK);

            break;

        case USB_DEVICE_CDC_EVENT_SEND_BREAK:

            /* This means that the host is requesting that a break of the
             * specified duration be sent. Read the break duration */

            appDataObject->breakData = ((USB_DEVICE_CDC_EVENT_DATA_SEND_BREAK *) pData)->breakDuration;

            /* Complete the control transfer by sending a ZLP  */
            USB_DEVICE_ControlStatus(appDataObject->deviceHandle, USB_DEVICE_CONTROL_STATUS_OK);

            break;

        case USB_DEVICE_CDC_EVENT_READ_COMPLETE:

            /* This means that the host has sent some data*/
            appDataObject->isReadComplete = true;
            break;

        case USB_DEVICE_CDC_EVENT_CONTROL_TRANSFER_DATA_RECEIVED:

            /* The data stage of the last control transfer is
             * complete. For now we accept all the data */

            USB_DEVICE_ControlStatus(appDataObject->deviceHandle, USB_DEVICE_CONTROL_STATUS_OK);
            break;

        case USB_DEVICE_CDC_EVENT_CONTROL_TRANSFER_DATA_SENT:

            /* This means the GET LINE CODING function data is valid. We dont
             * do much with this data in this demo. */
            break;

        case USB_DEVICE_CDC_EVENT_WRITE_COMPLETE:

            /* This means that the data write got completed. We can schedule
             * the next read. */

            appDataObject->isWriteComplete = true;
            break;

        default:
            break;
    }

    return USB_DEVICE_CDC_EVENT_RESPONSE_NONE;
}

/***********************************************
 * Application USB Device Layer Event Handler.
 ***********************************************/
void APP_USBDeviceEventHandler(USB_DEVICE_EVENT event, void * eventData, uintptr_t context) {
    USB_DEVICE_EVENT_DATA_CONFIGURED *configuredEventData;

    switch (event) {
        case USB_DEVICE_EVENT_SOF:

            /* This event is used for switch debounce. This flag is reset
             * by the switch process routine. */
            appData.sofEventHasOccurred = true;
            break;

        case USB_DEVICE_EVENT_RESET:

            /* Update LED to show reset state */

            appData.isConfigured = false;

            break;

        case USB_DEVICE_EVENT_CONFIGURED:

            /* Check the configuratio. We only support configuration 1 */
            configuredEventData = (USB_DEVICE_EVENT_DATA_CONFIGURED*) eventData;
            if (configuredEventData->configurationValue == 1) {
                /* Update LED to show configured state */

                /* Register the CDC Device application event handler here.
                 * Note how the appData object pointer is passed as the
                 * user data */

                USB_DEVICE_CDC_EventHandlerSet(USB_DEVICE_CDC_INDEX_0, APP_USBDeviceCDCEventHandler, (uintptr_t) & appData);

                /* Mark that the device is now configured */
                appData.isConfigured = true;

            }
            break;

        case USB_DEVICE_EVENT_POWER_DETECTED:

            /* VBUS was detected. We can attach the device */
            USB_DEVICE_Attach(appData.deviceHandle);
            break;

        case USB_DEVICE_EVENT_POWER_REMOVED:

            /* VBUS is not available any more. Detach the device. */
            USB_DEVICE_Detach(appData.deviceHandle);
            break;

        case USB_DEVICE_EVENT_SUSPENDED:

            /* Switch LED to show suspended state */
            break;

        case USB_DEVICE_EVENT_RESUMED:
        case USB_DEVICE_EVENT_ERROR:
        default:
            break;
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

/*****************************************************
 * This function is called in every step of the
 * application state machine.
 *****************************************************/

bool APP_StateReset(void) {
    /* This function returns true if the device
     * was reset  */

    bool retVal;

    if (appData.isConfigured == false) {
        appData.state = APP_STATE_WAIT_FOR_CONFIGURATION;
        appData.readTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
        appData.writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
        appData.isReadComplete = true;
        appData.isWriteComplete = true;
        retVal = true;
    } else {
        retVal = false;
    }

    return (retVal);
}

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

void APP_Initialize(void) {
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;

    /* Device Layer Handle  */
    appData.deviceHandle = USB_DEVICE_HANDLE_INVALID;

    /* Device configured status */
    appData.isConfigured = false;

    /* Initial get line coding state */
    appData.getLineCodingData.dwDTERate = 9600;
    appData.getLineCodingData.bParityType = 0;
    appData.getLineCodingData.bParityType = 0;
    appData.getLineCodingData.bDataBits = 8;

    /* Read Transfer Handle */
    appData.readTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;

    /* Write Transfer Handle */
    appData.writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;

    /* Intialize the read complete flag */
    appData.isReadComplete = true;

    /*Initialize the write complete flag*/
    appData.isWriteComplete = true;

    /* Reset other flags */
    appData.sofEventHasOccurred = false;
    //appData.isSwitchPressed = false;

    /* Set up the read buffer */
    appData.readBuffer = &readBuffer[0];
	
	// put these initializations in APP_Initialize()
    RPA0Rbits.RPA0R = 0b0101; // A0 is OC1
    TRISAbits.TRISA1 = 0;
    LATAbits.LATA1 = 0; // A1 is the direction pin to go along with OC1

    RPB2Rbits.RPB2R = 0b0101; // B2 is OC4
    TRISBbits.TRISB3 = 0;
    LATBbits.LATB3 = 0; // B3 is the direction pin to go along with OC4
	
	// also put these in APP_Initialize()
    T2CONbits.TCKPS = 2; // prescaler N=4 
    PR2 = 1200 - 1; // 10kHz
    TMR2 = 0;
    OC1CONbits.OCM = 0b110; // PWM mode without fault pin; other OC1CON bits are defaults
    OC4CONbits.OCM = 0b110;
    OC1RS = 0; // max allowed value is 1119
    OC1R = 0; // read-only initial value
    OC4RS = 0; // max allowed value is 1119
    OC4R = 0; // read-only initial value
    T2CONbits.ON = 1;
    OC1CONbits.ON = 1;
    OC4CONbits.ON = 1;

    startTime = _CP0_GET_COUNT();
}

/******************************************************************************
  Function:
    void APP_Tasks ( void )
  Remarks:
    See prototype in app.h.
 */

void APP_Tasks(void) {
    /* Update the application state machine based
     * on the current state */

    switch (appData.state) {
        case APP_STATE_INIT:

            /* Open the device layer */
            appData.deviceHandle = USB_DEVICE_Open(USB_DEVICE_INDEX_0, DRV_IO_INTENT_READWRITE);

            if (appData.deviceHandle != USB_DEVICE_HANDLE_INVALID) {
                /* Register a callback with device layer to get event notification (for end point 0) */
                USB_DEVICE_EventHandlerSet(appData.deviceHandle, APP_USBDeviceEventHandler, 0);

                appData.state = APP_STATE_WAIT_FOR_CONFIGURATION;
            } else {
                /* The Device Layer is not ready to be opened. We should try
                 * again later. */
            }

            break;

        case APP_STATE_WAIT_FOR_CONFIGURATION:

            /* Check if the device was configured */
            if (appData.isConfigured) {
                /* If the device is configured then lets start reading */
                appData.state = APP_STATE_SCHEDULE_READ;
            }
            break;

        case APP_STATE_SCHEDULE_READ:

            if (APP_StateReset()) {
                break;
            }

            /* If a read is complete, then schedule a read
             * else wait for the current read to complete */

            appData.state = APP_STATE_WAIT_FOR_READ_COMPLETE;
            if (appData.isReadComplete == true) {
                appData.isReadComplete = false;
                appData.readTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;

                USB_DEVICE_CDC_Read(USB_DEVICE_CDC_INDEX_0,
                        &appData.readTransferHandle, appData.readBuffer,
                        APP_READ_BUFFER_SIZE);

                if (appData.readTransferHandle == USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID) {
                    appData.state = APP_STATE_ERROR;
                    break;
                }
            }

            break;

        case APP_STATE_WAIT_FOR_READ_COMPLETE:
        case APP_STATE_CHECK_TIMER:

            if (APP_StateReset()) {
                break;
            }

            /* Check if a character was received or a switch was pressed.
             * The isReadComplete flag gets updated in the CDC event handler. */

            if (appData.isReadComplete) {
                appData.state = APP_STATE_SCHEDULE_WRITE;
            }

            break;


        case APP_STATE_SCHEDULE_WRITE:

            if (APP_StateReset()) {
                break;
            }

            /* Setup the write */

            appData.writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
            appData.isWriteComplete = false;
            appData.state = APP_STATE_WAIT_FOR_WRITE_COMPLETE;

            //len = sprintf(dataOut, "%d\r\n", i);
            i++;
            if (appData.isReadComplete) {
				
				i = 0;   // if the length of the readBuffer is longer than 1
				while(appData.readBuffer[i] != '\0'){
					if(appData.readBuffer[i] == '\r' || appData.readBuffer[i] == '\n'){
						sscanf(rx, "%d", &COM);    // 0 to 640
						len = sprintf(dataOut, "\r\nCOM = %d\r\n", COM);
						USB_DEVICE_CDC_Write(USB_DEVICE_CDC_INDEX_0,
							&appData.writeTransferHandle, dataOut, len,
							USB_DEVICE_CDC_TRANSFER_FLAGS_DATA_COMPLETE);
						error = COM - 320;
						eint = eint + error;
                        
                    // Anti wind-up
/*						int eint_max = 1000;
						if (eint > eint_max)
							eint = eint_max;
						else if (eint < -eint_max)
							eint = -eint_max;
						
						edot = error - preve;
						preve = error;
*/						


                        if ((error>-50 && error<0)  || (error<50 && error>0)){
                            ref_pwm = 4*error;
                        } else if ((error>-150 && error<0)  || (error<150 && error>0)){
							ref_pwm = 3*error;
						}else {
                            ref_pwm = 2.5*error + 0*eint + 0*edot;
                        }
						LATAbits.LATA1 = 1; // direction
						LATBbits.LATB3 = 1; // direction
                        if (ref_pwm > 0){
                            OC1RS = 1200/2;    // turn right
                            OC4RS = (1200 - ref_pwm)/3;
                        } else {
                            OC1RS = (1200 - ref_pwm)/3;     // turn left
                            OC4RS = 1200/2;
                        }
						//OC1RS = 600 - ref_pwm;
						//OC4RS = 600 + ref_pwm;
                        
                        
						if(h_i<his_length){
							COM_his[h_i] = COM;
							h_i++;
						} else {
							int h_ii = 0;
							for(h_ii=0; h_ii<his_length; h_ii++){
								COM_his[h_ii] = COM_his[h_ii+1];
							}
							COM_his[his_length-1] = COM;
						}
						
						if (COM<50 || COM > 590){
							if (pre_COM<300){
								OC1RS = 0;
								OC4RS = 800;
							} else if (pre_COM>340){
								OC1RS = 800;
								OC4RS = 0;
							}
						}
						
/*						if (COM<70 || COM>580){
							if(direction_l == 1){
								OC1RS = 1200/2;
								OC4RS = 0;
							} else if(direction_r == 1){
								OC1RS = 0;
								OC4RS = 1200/2;
							} else{
								direction_l = 1;
								direction_r = 1;
							
								int di_i = 0;
								for(di_i=0; di_i<his_length; di_i++){
									if(COM_his[di_i]>500){
										direction_l = direction_l*1;
									}else{
										direction_l = direction_l*0;
									}
															
									if(COM_his[di_i<140]){
										direction_r = direction_r*1;
									}else{
										direction_r = direction_r*0;
									}
									
									if (direction_l == 1){
										direction_r = 0;
									}
									
									if (direction_r == 1){
										direction_l = 0;
									}
								}
							}
							
							
							if(direction_r == 1){    // turn right
								OC1RS = 0;
								OC4RS = 1200/2;
							} else if(direction_l == 1){     // turn left
								OC1RS = 1200/2;
								OC4RS = 0;
							}
						} */
						if(COM>50 && COM<590){
							pre_COM = COM;
						}
						
						
					}
					
					rx[ii] = appData.readBuffer[i];
					ii++;
					
					
					if(appData.readBuffer[i] == '\r' || appData.readBuffer[i] == '\n'){
						ii = 0;
						memset(rx,'\0',sizeof(rx));
					}
					i++;
				}
				
/* 				while(appData.readBuffer[i] != '\0'){
					if(appData.readBuffer[i] == '\r' || appData.readBuffer[i] == '\n'){
					
					if(i_data == 0){
						sscanf(rx, "%d", &data1);
						len = sprintf(dataOut, "\r\ndata1 = %d\r\n", data1);
						USB_DEVICE_CDC_Write(USB_DEVICE_CDC_INDEX_0,
							&appData.writeTransferHandle, dataOut, len,
							USB_DEVICE_CDC_TRANSFER_FLAGS_DATA_COMPLETE);
						if (data1 < 50){
							LATAbits.LATA1 = 0; // direction
							pwm1 = (50-data1)*24;   // 1200/50 = 24
							OC1RS = pwm1; // velocity, 50%
						} else {
							LATAbits.LATA1 = 1; // direction
							pwm1 = (data1-50)*24;
							OC1RS = pwm1; // velocity, 50%
						}
						i_data++;
					} else if (i_data == 1){
						sscanf(rx, "%d", &data2);
						len = sprintf(dataOut, "\r\ndata2 = %d\r\n", data2);
						USB_DEVICE_CDC_Write(USB_DEVICE_CDC_INDEX_0,
							&appData.writeTransferHandle, dataOut, len,
							USB_DEVICE_CDC_TRANSFER_FLAGS_DATA_COMPLETE);
						if (data2 < 50){
							LATBbits.LATB3 = 0; // direction
							pwm2 = (50-data2)*24;
							OC4RS = pwm2; // velocity, 50%
						} else {
							LATBbits.LATB3 = 1; // direction
							pwm2 = (data2-50)*24;
							OC4RS = pwm2; // velocity, 50%
						}
						i_data = 0;
					}
					
					}
					rx[ii] = appData.readBuffer[i];
					ii++;
					if(appData.readBuffer[i] == '\r' || appData.readBuffer[i] == '\n'){
						ii = 0;
						memset(rx,'\0',sizeof(rx));
					}
					i++;
				}
	 */
                USB_DEVICE_CDC_Write(USB_DEVICE_CDC_INDEX_0,
                        &appData.writeTransferHandle,
                        appData.readBuffer, 1,
                        USB_DEVICE_CDC_TRANSFER_FLAGS_DATA_COMPLETE);
            }
            break;

        case APP_STATE_WAIT_FOR_WRITE_COMPLETE:

            if (APP_StateReset()) {
                break;
            }

            /* Check if a character was sent. The isWriteComplete
             * flag gets updated in the CDC event handler */

            if (appData.isWriteComplete == true) {
                appData.state = APP_STATE_SCHEDULE_READ;
            }

            break;

        case APP_STATE_ERROR:
            break;
        default:
            break;
    }
}



/*******************************************************************************
 End of File
 */