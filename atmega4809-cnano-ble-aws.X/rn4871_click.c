/**
 * \file
 *
 *
 (c) 2018 Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms,you may use this software and
    any derivatives exclusively with Microchip products.It is your responsibility
    to comply with third party license terms applicable to your use of third party
    software (including open source software) that may accompany Microchip software.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 */

#include <stdio.h>
#include <string.h>
#include "rn4871_click.h"
#include "driver_rn4871.h"
#include "mcc_generated_files/mcc.h"
#include "util/delay.h"

typedef enum { NO_RESPONSE, OK } rn4871_Responses_t;

volatile uint16_t rn_index = 0;
volatile uint8_t  rn_buffer[RN_BUFF_SIZE];

//*********************************************************
//          Local Function Prototypes
//*********************************************************
static void RN4871_CaptureReceivedMessage(void);
static void RN4871_RegisterISRCallback(void);

//*********************************************************
//          ISR Call back Function
//*********************************************************
static void RN4871_CaptureReceivedMessage()
{
	uint8_t data = USART_RN4871_GetData();
	if (rn_index < RN_BUFF_SIZE)
		rn_buffer[rn_index++] = data;
}

//*********************************************************
//          Other Functions
//*********************************************************
static void RN4871_RegisterISRCallback(void)
{
	//USART_RN4871_set_ISR_cb(&RN4871_CaptureReceivedMessage, RX_CB);
}

void RN4871_Setup(const char *name)
{
	char nameString[32];

	RN4871_RegisterISRCallback();
	RN4871_Reset_Module();
	RN4871_sendAndWait("$$$", "CMD> ", 40);
	RN4871_sendAndWait("SS,C0\r\n", "AOK\r\n", 40);
	RN4871_sendAndWait("SR,00000000\r\n", "AOK\r\n", 40);
	RN4871_sendAndWait("PS,180F\r\n", "AOK\r\n", 40);
	RN4871_sendAndWait("PC,2A19,10,05\r\n", "AOK\r\n", 40);
	sprintf(nameString, "S-,%s\r\n", name);
	RN4871_sendAndWait(nameString, "AOK\r\n", 40);
	RN4871_sendAndWait("R,1\r\n", "Rebooting", 40);
	RN4871_sendAndWait("$$$", "CMD> ", 40);
	RN4871_sendAndWait("A\r\n", "AOK\r\n", 40);
	RN4871_sendAndWait("---\r\n", "END", 40);
	RN4871_ClearReceivedMessage();
}

void RN4871_Reset_Module(void)
{
	RN_Reset_Pin_SetLow(); // Reset using GPIO
	RN4871_blockingWait(1);
	RN_Reset_Pin_SetHigh();
	RN4871_blockingWait(50);
}

void RN4871_ClearReceivedMessage(void)
{
	memset((void *)rn_buffer, 0, RN_BUFF_SIZE);
	rn_index = 0;
}

uint8_t RN4871_CheckResponse(const char *response)
{
	uint8_t ret = 0;
	if (strstr((const char *)rn_buffer, response))
		ret = 1;
	return ret;
}

void RN4871_blockingWait(uint16_t limit)
{
	for (uint16_t counter = 0; counter < limit; counter++)
		_delay_ms(15);
}
void RN4871_sendAndWait(const char *sendString, const char *response, uint16_t delay)
{
	do {
		RN4871_ClearReceivedMessage();
		rn4871_SendString(sendString);
		RN4871_blockingWait(delay);
	} while (RN4871_CheckResponse(response) == NO_RESPONSE);
}
