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

#include <stdbool.h>
#include <string.h>
#include "rn4871_click.h"
#include "driver_rn4871.h"
#include "application_handler.h"
#include "mcc_generated_files/mcc.h"

#define TOGGLE_APPLICATION_STATE_COMMAND                (0x2F)      // '/' character

void dataTransfer_parseDataFromTerminal(void);
void dataTransfer_RN4871ToTerminal(void);
void dataTransfer_terminalToRN4871(void);
void dataTransfer_replyToTerminal(const char *reply, uint16_t replyLength);
void dataTransfer_parseDataFromTerminal(void);
void dataTransfer_updateCharacteristicsValues(uint8_t characteristicsType, volatile char sensorValue[]);
void dataTransfer_updateCharacteristicsHandler(uint8_t dataCharacteristic);

static const char characteristicValueUpdate[] = {"SHW,"};
static const char endOfLine[] = {"\r\n\0"};
static const char commandHandlerSeparator[] = {","};
static const char rn4871Reply_commandDone[] = {"CMD> "};
  
extern const char pressureHandler[];
extern const char temperatureHandler[];
extern const char humidityHandler[];
  
uint8_t dataFromTerminal = 0;
    
volatile bool newDataFlag = false;
 
extern uint8_t applicationState;
    
void dataTransfer_parseDataFromTerminal(void)
{
    switch (dataFromTerminal)
    {        
        case TOGGLE_APPLICATION_STATE_COMMAND:
            applicationEvent = TOGGLE_EVENT;
            break;
        
        default:
            applicationEvent = DATA_FROM_TERMINAL_EVENT;
            break;
    }
}

void dataTransfer_terminalToRN4871(void)
{
    dataFromTerminal = USART_TERMINAL_GetData();
    newDataFlag = true;
}

void dataTransfer_replyToTerminal(const char* reply, uint16_t replyLength)
{
    uint16_t i;
    
    for (i = 0; i < replyLength; i++)
    {
        USART_TERMINAL_Write(reply[i]);
    }
}

void dataTransfer_updateCharacteristicsValues(uint8_t characteristicsType,volatile char sensorValue[])
{
    RN4871_ClearReceivedMessage();
    rn4871_SendString(characteristicValueUpdate);
    
    sensorValue[MAX_LOCATION] = END_OF_STRING;
    
    switch(characteristicsType){
    
        case PRESS_COMMAND_HANDLER:

            rn4871_SendString(pressureHandler);
            break;
        
        case TEMP_COMMAND_HANDLER:
            rn4871_SendString(temperatureHandler);
            break;
            
        case HUM_COMMAND_HANDLER:
            rn4871_SendString(humidityHandler);
            break;
        
        default:
            ;
    }
    
    rn4871_SendString(commandHandlerSeparator);
    rn4871_SendString(sensorValue);
    rn4871_SendString(endOfLine);
    RN4871_blockingWait(1);
}
