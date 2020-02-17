/*
    \file   sensors_handling.c

    \brief  Sensors handling handler source file.

    (c) 2018 Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip software and any
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party
    license terms applicable to your use of third party software (including open source software) that
    may accompany Microchip software.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS
    FOR A PARTICULAR PURPOSE.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS
    SOFTWARE.
*/

#include "mcc_generated_files/mcc.h"
#include "driver_rn4871.h"
#include "mcc_generated_files/weather.h"
#include "rn4871_click.h"
#include <string.h>
#include "application_handler.h"


#define MAX_SENSORS_BUFFER_SIZE                (MAX_LOCATION + 1)
#define END_OF_CMD                              ('\n')

void application_initialize(void);
void application_updateStateMachine(void);

static void Timer_start(void);
static void Timer_stop(void);
static void updateCharacteristicsValue(void);
static void getSensorsValues(void);
static void Timer_Callback(void);

static void commandStateHandler(void);
static void applicationStateHandler(void);

extern void dataTransfer_updateCharacteristicsHandler(uint8_t newData);
extern void dataTransfer_replyToTerminal(const char *reply, uint16_t replyLength);
extern void dataTransfer_updateCharacteristicsValues(uint8_t characteristicsType, volatile char sensorValue[]);
extern void dataTransfer_RN4871ToTerminal(void);
extern void dataTransfer_terminalToRN4871(void);

extern void rn4871_clearReceivedMessage(void);

const volatile char commandStateReply[] = {"\r\nCommand State Activated\r\n"};
const volatile char applicationStateReply[] = {"\r\nApplication State Activated\r\n"};

uint8_t applicationState = APPLICATION_STATE;

enum applicationEvents applicationEvent = NO_EVENT;

volatile char pressSensorValue[MAX_SENSORS_BUFFER_SIZE];
volatile char tempSensorValue[MAX_SENSORS_BUFFER_SIZE];
volatile char humSensorValue[MAX_SENSORS_BUFFER_SIZE];

float volatile temperature = 0;
float volatile pressure = 0;
float volatile humidity = 0;

extern uint8_t dataFromTerminal;
extern volatile uint8_t  RN4871_dataBuffer[];

void application_initialize(void)
{
   USART_TERMINAL_SetRXISRCb(dataTransfer_terminalToRN4871);
   
   TIMER_0_SetOVFIsrCallback(Timer_Callback);
    
    Timer_start();
    
    applicationEvent = NO_EVENT;
}

void application_updateStateMachine(void)
{
    switch (applicationState)
    {
        case COMMAND_STATE:
            commandStateHandler();
            break;
        
        case APPLICATION_STATE:
            applicationStateHandler();
            break;
        
        default:
            break;
    }
    
    applicationEvent = NO_EVENT;
}

static void commandStateHandler(void)
{
    switch (applicationEvent)
    {
        case DATA_FROM_TERMINAL_EVENT:
            rn4871_SendByte(dataFromTerminal);
            if (dataFromTerminal == END_OF_CMD)
            {
                rn4871_clearReceivedMessage();
                RN4871_blockingWait(5);
                dataTransfer_replyToTerminal(RN4871_dataBuffer, strlen(RN4871_dataBuffer));
            }

            break;
        
        case TOGGLE_EVENT:
            applicationState = APPLICATION_STATE;
            dataTransfer_replyToTerminal(applicationStateReply, strlen(applicationStateReply));
            Timer_start();
            break;
        
        default:
            break;
    }    
}

static void applicationStateHandler(void)
{
    uint16_t volatile length;
    switch (applicationEvent)
    {
        case TIME_TO_TRANSMIT_EVENT:
            getSensorsValues();
            updateCharacteristicsValue();
            break;
        
        case TOGGLE_EVENT:
            applicationState = COMMAND_STATE;
            Timer_stop();
            length = strlen(commandStateReply);
            dataTransfer_replyToTerminal(commandStateReply, length);
            break;
        
        default:
            break;
    }
}

static void getSensorsValues(void)
{
    
    Weather_readSensors();  
    
    temperature = Weather_getTemperatureDegC();
    long int valueToSend  = 0;
    valueToSend = (((long int)temperature) * 10000 + (((int)(temperature*100))%100));
    sprintf(tempSensorValue, "%08ld", valueToSend);
    
    
    pressure = Weather_getPressureKPa();
   sprintf(pressSensorValue, "%08ld", (long int) (pressure*1000));   
    
    humidity = Weather_getHumidityRH();
    valueToSend = (((long int)humidity) * 10000 + (((int)(humidity*100))%100));
    sprintf(humSensorValue, "%08ld",valueToSend);
    asm("NOP");
}

static void updateCharacteristicsValue(void)
{
    dataTransfer_updateCharacteristicsValues(PRESS_COMMAND_HANDLER, pressSensorValue);
    dataTransfer_updateCharacteristicsValues(TEMP_COMMAND_HANDLER, tempSensorValue);
    dataTransfer_updateCharacteristicsValues(HUM_COMMAND_HANDLER, humSensorValue);
}

static void Timer_start(void)
{
    // set the starting count value to 0 and enable the module
    TCA0.SINGLE.CNT = 0x0;
    TCA0.SINGLE.CTRLA |= 1 << TCA_SINGLE_ENABLE_bp;
}

static void Timer_stop(void)
{
    TCA0.SINGLE.CTRLA &= ~(1 << TCA_SINGLE_ENABLE_bp);
}

static void Timer_Callback(void)
{
    applicationEvent = TIME_TO_TRANSMIT_EVENT;
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}