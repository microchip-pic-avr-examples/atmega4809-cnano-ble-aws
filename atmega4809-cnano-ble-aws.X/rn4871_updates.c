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
#include <string.h>
#include "mcc_generated_files/mcc.h"
#include "rn4871_click.h"
#include "driver_rn4871.h"


#define RN4871_DELAY                            40
#define RN4871_BUFFER_SIZE                      (350)

void rn4871_initialize(void);
void rn4871_clearReceivedMessage(void);

static void updateRN4871Services(void);
static void updateRN4871Characteristics(void);
static void rn4871_createSensorService(void);
static void rn4871_createCharacteristic(int characteristic);
static void rn4871_enterCommandMode(void);
static void rn4871_captureReceivedMessage(void);

static const char endOfLine[] = {"\r\n\0"};

//RN4871 commands
static const char rn4871Command_commandMode[] = {"$$$"};
static const char rn4871Command_setServices[] = {"ss,"};
static const char rn4871Command_getServices[] = {"gs"};
static const char rn4871Command_listCharacteristics[] = {"ls"};
static const char rn4871Command_createSensorService[] = {"ps,"};
static const char rn4871Command_createCharacteristic[] = {"pc,"};
static const char rn4871Reply_newCommand[] = {"CMD> "};

static const char mySetServices[] = {"80"};
static const char characteristicAttributes[] = {",1a,14"};
    
extern const char sensorServices[];
extern const char pressureCharacteristic[];
extern const char temperatureCharacteristic[];
extern const char humidityCharacteristic[];

volatile uint16_t RN4871_dataIndex = 0;
volatile uint8_t  RN4871_dataBuffer[RN4871_BUFFER_SIZE];

void rn4871_initialize(void)
{
    USART_RN4871_SetRXISRCb(&rn4871_captureReceivedMessage);
    
    RN4871_Reset_Module();
    rn4871_enterCommandMode();    
   
    updateRN4871Services();
    updateRN4871Characteristics();
    
    RN4871_Reset_Module();
    rn4871_enterCommandMode();
}

static void updateRN4871Services(void)
{
    uint8_t rn4871Response = 0;
    
    rn4871_clearReceivedMessage();
    
    rn4871_SendString(rn4871Command_getServices);
    rn4871_SendString(endOfLine);
    RN4871_blockingWait(RN4871_DELAY);
    
    rn4871Response = RN4871_CheckResponse(mySetServices);
    if (!rn4871Response)
    {
        rn4871_SendString(rn4871Command_setServices);
        rn4871_SendString(mySetServices);
        rn4871_SendString(endOfLine);
        RN4871_blockingWait(RN4871_DELAY);  
    }
}

static void updateRN4871Characteristics(void)
{
    uint8_t rn4871Response = 0;
    
    rn4871_clearReceivedMessage();
    
    rn4871_SendString(rn4871Command_listCharacteristics);
    rn4871_SendString(endOfLine);
    RN4871_blockingWait(RN4871_DELAY);
    
    rn4871Response = RN4871_CheckResponse(sensorServices);
    if (!rn4871Response)
    {
        rn4871_createSensorService();

        rn4871_createCharacteristic(pressureCharacteristic);

        rn4871_createCharacteristic(temperatureCharacteristic);
        
        rn4871_createCharacteristic(humidityCharacteristic);
    }
}

static void rn4871_createSensorService(void)
{
    rn4871_SendString(rn4871Command_createSensorService);
    rn4871_SendString(sensorServices);
    rn4871_SendString(endOfLine);
    RN4871_blockingWait(RN4871_DELAY);    
}

static void rn4871_createCharacteristic(int characteristic)
{
    rn4871_SendString(rn4871Command_createCharacteristic);
    rn4871_SendString(characteristic);
    rn4871_SendString(characteristicAttributes);
    rn4871_SendString(endOfLine);
    RN4871_blockingWait(RN4871_DELAY);    
}

static void rn4871_enterCommandMode(void)
{
    do {
        rn4871_clearReceivedMessage();
        rn4871_SendString(rn4871Command_commandMode);
        RN4871_blockingWait(RN4871_DELAY);
    } while (!strstr(RN4871_dataBuffer, rn4871Reply_newCommand));    
}

static void rn4871_captureReceivedMessage()
{
	uint8_t data = USART_RN4871_GetData();
	if (RN4871_dataIndex < RN4871_BUFFER_SIZE)
    {
		RN4871_dataBuffer[RN4871_dataIndex++] = data;
    }    
}

void rn4871_clearReceivedMessage(void)
{
    memset((void *)RN4871_dataBuffer, 0, RN4871_BUFFER_SIZE);
    RN4871_dataIndex = 0;
}