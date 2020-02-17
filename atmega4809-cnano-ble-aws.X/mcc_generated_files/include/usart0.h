/*
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

#ifndef USART0_H_INCLUDED
#define USART0_H_INCLUDED

#include <stdbool.h>
#include <stdio.h>
#include "../utils/compiler.h"
#include "../utils/atomic.h"
#include "../config/clock_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Normal Mode, Baud register value */
#define USART0_BAUD_RATE(BAUD_RATE) (((float)2000000 * 64 / (16 * (float)BAUD_RATE)) + 0.5)

/* USART_RN4871 Ringbuffer */

#define USART_RN4871_RX_BUFFER_SIZE 8
#define USART_RN4871_TX_BUFFER_SIZE 8
#define USART_RN4871_RX_BUFFER_MASK (USART_RN4871_RX_BUFFER_SIZE - 1)
#define USART_RN4871_TX_BUFFER_MASK (USART_RN4871_TX_BUFFER_SIZE - 1)

typedef enum { USART_RN4871_RX_CB = 1, USART_RN4871_TX_CB } usart_rn4871_cb_t;
typedef void (*usart_callback)(void);

/**
 * \brief Initialize USART interface
 * If module is configured to disabled state, the clock to the USART is disabled
 * if this is supported by the device's clock system.
 *
 * \return Initialization status.
 * \retval 0 the USART init was successful
 * \retval 1 the USART init was not successful
 */
void USART_RN4871_Initialize();

/**
 * \brief Enable RX and TX in USART_RN4871
 * 1. If supported by the clock system, enables the clock to the USART
 * 2. Enables the USART module by setting the RX and TX enable-bits in the USART control register
 *
 * \return Nothing
 */
void USART_RN4871_Enable();

/**
 * \brief Enable RX in USART_RN4871
 * 1. If supported by the clock system, enables the clock to the USART
 * 2. Enables the USART module by setting the RX enable-bit in the USART control register
 *
 * \return Nothing
 */
void USART_RN4871_EnableRx();

/**
 * \brief Enable TX in USART_RN4871
 * 1. If supported by the clock system, enables the clock to the USART
 * 2. Enables the USART module by setting the TX enable-bit in the USART control register
 *
 * \return Nothing
 */
void USART_RN4871_EnableTx();

/**
 * \brief Disable USART_RN4871
 * 1. Disables the USART module by clearing the enable-bit(s) in the USART control register
 * 2. If supported by the clock system, disables the clock to the USART
 *
 * \return Nothing
 */
void USART_RN4871_Disable();

/**
 * \brief Get recieved data from USART_RN4871
 *
 * \return Data register from USART_RN4871 module
 */
uint8_t USART_RN4871_GetData();

/**
 * \brief Check if the usart can accept data to be transmitted
 *
 * \return The status of USART TX data ready check
 * \retval false The USART can not receive data to be transmitted
 * \retval true The USART can receive data to be transmitted
 */
bool USART_RN4871_IsTxReady();

/**
 * \brief Check if the USART has received data
 *
 * \return The status of USART RX data ready check
 * \retval true The USART has received data
 * \retval false The USART has not received data
 */
bool USART_RN4871_IsRxReady();

/**
 * \brief Check if USART_RN4871 data is transmitted
 *
 * \return Receiver ready status
 * \retval true  Data is not completely shifted out of the shift register
 * \retval false Data completely shifted out if the USART shift register
 */
bool USART_RN4871_IsTxBusy();



bool USART_RN4871_IsTxDone();
/**
 * \brief Read one character from USART_RN4871
 *
 * Function will block if a character is not available.
 *
 * \return Data read from the USART_RN4871 module
 */
uint8_t USART_RN4871_Read(void);

/**
 * \brief Write one character to USART_RN4871
 *
 * Function will block until a character can be accepted.
 *
 * \param[in] data The character to write to the USART
 *
 * \return Nothing
 */
void USART_RN4871_Write(const uint8_t data);

/**
 * \brief Set call back function for USART_RN4871
 *
 * \param[in] cb The call back function to set
 *
 * \param[in] type The type of ISR to be set
 *
 * \return Nothing
 */
 
void USART_RN4871_DefaultRxIsrCb(void);

void USART_RN4871_DefaultTxIsrCb(void);

void USART_RN4871_SetISRCb(usart_callback cb, usart_rn4871_cb_t type);

void USART_RN4871_SetRXISRCb(usart_callback cb);

void USART_RN4871_SetTXISRCb(usart_callback cb);

#ifdef __cplusplus
}
#endif

#endif /* USART0_H_INCLUDED */