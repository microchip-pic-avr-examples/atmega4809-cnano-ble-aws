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

#include "../include/usart0.h"

#if defined(__GNUC__)

int USART_RN4871_printCHAR(char character, FILE *stream)
{
    USART_RN4871_Write(character);
    return 0;
}

FILE USART_RN4871_stream = FDEV_SETUP_STREAM(USART_RN4871_printCHAR, NULL, _FDEV_SETUP_WRITE);

#elif defined(__ICCAVR__)

int putchar(int outChar)
{
    USART_RN4871_Write(outChar);
    return outChar;
}
#endif

/* Static Variables holding the ringbuffer used in IRQ mode */
static uint8_t          USART_RN4871_rxbuf[USART_RN4871_RX_BUFFER_SIZE];
static volatile uint8_t USART_RN4871_rx_head;
static volatile uint8_t USART_RN4871_rx_tail;
static volatile uint8_t USART_RN4871_rx_elements;
static uint8_t          USART_RN4871_txbuf[USART_RN4871_TX_BUFFER_SIZE];
static volatile uint8_t USART_RN4871_tx_head;
static volatile uint8_t USART_RN4871_tx_tail;
static volatile uint8_t USART_RN4871_tx_elements;

void (*USART_RN4871_rx_isr_cb)(void) = &USART_RN4871_DefaultRxIsrCb;

void (*USART_RN4871_tx_isr_cb)(void) = &USART_RN4871_DefaultTxIsrCb;

void USART_RN4871_DefaultRxIsrCb(void)
{
    uint8_t data;
    uint8_t tmphead;

    /* Read the received data */
    data = USART0.RXDATAL;
    /* Calculate buffer index */
    tmphead = (USART_RN4871_rx_head + 1) & USART_RN4871_RX_BUFFER_MASK;
        
    if (tmphead == USART_RN4871_rx_tail) {
            /* ERROR! Receive buffer overflow */
    }else {
    /*Store new index*/
    USART_RN4871_rx_head = tmphead;
    
    /* Store received data in buffer */
    USART_RN4871_rxbuf[tmphead] = data;
    USART_RN4871_rx_elements++;
    }
}

void USART_RN4871_DefaultTxIsrCb(void)
{
    uint8_t tmptail;

    /* Check if all data is transmitted */
    if (USART_RN4871_tx_elements != 0) {
        /* Calculate buffer index */
        tmptail = (USART_RN4871_tx_tail + 1) & USART_RN4871_TX_BUFFER_MASK;
        /* Store new index */
        USART_RN4871_tx_tail = tmptail;
        /* Start transmission */
        USART0.TXDATAL = USART_RN4871_txbuf[tmptail];
        
        USART_RN4871_tx_elements--;
    }

    if (USART_RN4871_tx_elements == 0) {
            /* Disable Tx interrupt */
            USART0.CTRLA &= ~(1 << USART_DREIE_bp);
    }
}

void USART_RN4871_SetISRCb(usart_callback cb, usart_rn4871_cb_t type)
{
    switch (type) {
        case USART_RN4871_RX_CB:
                USART_RN4871_rx_isr_cb = cb;
                break;
        case USART_RN4871_TX_CB:
                USART_RN4871_tx_isr_cb = cb;
                break;
        default:
                // do nothing
                break;
    }
}

void USART_RN4871_SetRXISRCb(usart_callback cb)
{
    USART_RN4871_SetISRCb(cb,USART_RN4871_RX_CB);
}

void USART_RN4871_SetTXISRCb(usart_callback cb)
{
    USART_RN4871_SetISRCb(cb,USART_RN4871_TX_CB);
}

/* Interrupt service routine for RX complete */
ISR(USART0_RXC_vect)
{
    if (USART_RN4871_rx_isr_cb != NULL)
    {
        (*USART_RN4871_rx_isr_cb)();
    }
}

/* Interrupt service routine for Data Register Empty */
ISR(USART0_DRE_vect)
{
    if (USART_RN4871_tx_isr_cb != NULL)
    {
        (*USART_RN4871_tx_isr_cb)();
    }
}

ISR(USART0_TXC_vect)
{
    USART0.STATUS |= USART_TXCIF_bm;
}

bool USART_RN4871_IsTxReady()
{
    return (USART_RN4871_tx_elements != USART_RN4871_TX_BUFFER_SIZE);
}

bool USART_RN4871_IsRxReady()
{
    return (USART_RN4871_rx_elements != 0);
}

bool USART_RN4871_IsTxBusy()
{
    return (!(USART0.STATUS & USART_TXCIF_bm));
}

bool USART_RN4871_IsTxDone()
{
    return (USART0.STATUS & USART_TXCIF_bm);
}

uint8_t USART_RN4871_Read(void)
{
    uint8_t tmptail;

    /* Wait for incoming data */
    while (USART_RN4871_rx_elements == 0)
            ;
    /* Calculate buffer index */
    tmptail = (USART_RN4871_rx_tail + 1) & USART_RN4871_RX_BUFFER_MASK;
    /* Store new index */
    USART_RN4871_rx_tail = tmptail;
    ENTER_CRITICAL(R);
    USART_RN4871_rx_elements--;
    EXIT_CRITICAL(R);

    /* Return data */
    return USART_RN4871_rxbuf[tmptail];
}

void USART_RN4871_Write(const uint8_t data)
{
    uint8_t tmphead;

    /* Calculate buffer index */
    tmphead = (USART_RN4871_tx_head + 1) & USART_RN4871_TX_BUFFER_MASK;
    /* Wait for free space in buffer */
    while (USART_RN4871_tx_elements == USART_RN4871_TX_BUFFER_SIZE)
            ;
    /* Store data in buffer */
    USART_RN4871_txbuf[tmphead] = data;
    /* Store new index */
    USART_RN4871_tx_head = tmphead;
    ENTER_CRITICAL(W);
    USART_RN4871_tx_elements++;
    EXIT_CRITICAL(W);
    /* Enable Tx interrupt */
    USART0.CTRLA |= (1 << USART_DREIE_bp);
}

void USART_RN4871_Initialize()
{
    //set baud rate register
    USART0.BAUD = (uint16_t)USART0_BAUD_RATE(115200);
	
    //RXCIE enabled; TXCIE disabled; DREIE disabled; RXSIE enabled; LBME disabled; ABEIE disabled; RS485 OFF; 
    USART0.CTRLA = 0x90;
	
    //RXEN enabled; TXEN enabled; SFDEN disabled; ODME disabled; RXMODE NORMAL; MPCM disabled; 
    USART0.CTRLB = 0xC0;
	
    //CMODE ASYNCHRONOUS; PMODE DISABLED; SBMODE 1BIT; CHSIZE 8BIT; UDORD disabled; UCPHA disabled; 
    USART0.CTRLC = 0x03;
	
    //DBGCTRL_DBGRUN
    USART0.DBGCTRL = 0x00;
	
    //EVCTRL_IREI
    USART0.EVCTRL = 0x00;
	
    //RXPLCTRL_RXPL
    USART0.RXPLCTRL = 0x00;
	
    //TXPLCTRL_TXPL
    USART0.TXPLCTRL = 0x00;
	

    uint8_t x;

    /* Initialize ringbuffers */
    x = 0;

    USART_RN4871_rx_tail     = x;
    USART_RN4871_rx_head     = x;
    USART_RN4871_rx_elements = x;
    USART_RN4871_tx_tail     = x;
    USART_RN4871_tx_head     = x;
    USART_RN4871_tx_elements = x;

#if defined(__GNUC__)
    stdout = &USART_RN4871_stream;
#endif

}

void USART_RN4871_Enable()
{
    USART0.CTRLB |= USART_RXEN_bm | USART_TXEN_bm;
}

void USART_RN4871_EnableRx()
{
    USART0.CTRLB |= USART_RXEN_bm;
}

void USART_RN4871_EnableTx()
{
    USART0.CTRLB |= USART_TXEN_bm;
}

void USART_RN4871_Disable()
{
    USART0.CTRLB &= ~(USART_RXEN_bm | USART_TXEN_bm);
}

uint8_t USART_RN4871_GetData()
{
    return USART0.RXDATAL;
}