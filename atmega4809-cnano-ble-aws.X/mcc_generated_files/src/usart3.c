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

#include "../include/usart3.h"


/* Static Variables holding the ringbuffer used in IRQ mode */
static uint8_t          USART_TERMINAL_rxbuf[USART_TERMINAL_RX_BUFFER_SIZE];
static volatile uint8_t USART_TERMINAL_rx_head;
static volatile uint8_t USART_TERMINAL_rx_tail;
static volatile uint8_t USART_TERMINAL_rx_elements;
static uint8_t          USART_TERMINAL_txbuf[USART_TERMINAL_TX_BUFFER_SIZE];
static volatile uint8_t USART_TERMINAL_tx_head;
static volatile uint8_t USART_TERMINAL_tx_tail;
static volatile uint8_t USART_TERMINAL_tx_elements;

void (*USART_TERMINAL_rx_isr_cb)(void) = &USART_TERMINAL_DefaultRxIsrCb;

void (*USART_TERMINAL_tx_isr_cb)(void) = &USART_TERMINAL_DefaultTxIsrCb;

void USART_TERMINAL_DefaultRxIsrCb(void)
{
    uint8_t data;
    uint8_t tmphead;

    /* Read the received data */
    data = USART3.RXDATAL;
    /* Calculate buffer index */
    tmphead = (USART_TERMINAL_rx_head + 1) & USART_TERMINAL_RX_BUFFER_MASK;
        
    if (tmphead == USART_TERMINAL_rx_tail) {
            /* ERROR! Receive buffer overflow */
    }else {
    /*Store new index*/
    USART_TERMINAL_rx_head = tmphead;
    
    /* Store received data in buffer */
    USART_TERMINAL_rxbuf[tmphead] = data;
    USART_TERMINAL_rx_elements++;
    }
}

void USART_TERMINAL_DefaultTxIsrCb(void)
{
    uint8_t tmptail;

    /* Check if all data is transmitted */
    if (USART_TERMINAL_tx_elements != 0) {
        /* Calculate buffer index */
        tmptail = (USART_TERMINAL_tx_tail + 1) & USART_TERMINAL_TX_BUFFER_MASK;
        /* Store new index */
        USART_TERMINAL_tx_tail = tmptail;
        /* Start transmission */
        USART3.TXDATAL = USART_TERMINAL_txbuf[tmptail];
        
        USART_TERMINAL_tx_elements--;
    }

    if (USART_TERMINAL_tx_elements == 0) {
            /* Disable Tx interrupt */
            USART3.CTRLA &= ~(1 << USART_DREIE_bp);
    }
}

void USART_TERMINAL_SetISRCb(usart_callback cb, usart_terminal_cb_t type)
{
    switch (type) {
        case USART_TERMINAL_RX_CB:
                USART_TERMINAL_rx_isr_cb = cb;
                break;
        case USART_TERMINAL_TX_CB:
                USART_TERMINAL_tx_isr_cb = cb;
                break;
        default:
                // do nothing
                break;
    }
}

void USART_TERMINAL_SetRXISRCb(usart_callback cb)
{
    USART_TERMINAL_SetISRCb(cb,USART_TERMINAL_RX_CB);
}

void USART_TERMINAL_SetTXISRCb(usart_callback cb)
{
    USART_TERMINAL_SetISRCb(cb,USART_TERMINAL_TX_CB);
}

/* Interrupt service routine for RX complete */
ISR(USART3_RXC_vect)
{
    if (USART_TERMINAL_rx_isr_cb != NULL)
    {
        (*USART_TERMINAL_rx_isr_cb)();
    }
}

/* Interrupt service routine for Data Register Empty */
ISR(USART3_DRE_vect)
{
    if (USART_TERMINAL_tx_isr_cb != NULL)
    {
        (*USART_TERMINAL_tx_isr_cb)();
    }
}

ISR(USART3_TXC_vect)
{
    USART3.STATUS |= USART_TXCIF_bm;
}

bool USART_TERMINAL_IsTxReady()
{
    return (USART_TERMINAL_tx_elements != USART_TERMINAL_TX_BUFFER_SIZE);
}

bool USART_TERMINAL_IsRxReady()
{
    return (USART_TERMINAL_rx_elements != 0);
}

bool USART_TERMINAL_IsTxBusy()
{
    return (!(USART3.STATUS & USART_TXCIF_bm));
}

bool USART_TERMINAL_IsTxDone()
{
    return (USART3.STATUS & USART_TXCIF_bm);
}

uint8_t USART_TERMINAL_Read(void)
{
    uint8_t tmptail;

    /* Wait for incoming data */
    while (USART_TERMINAL_rx_elements == 0)
            ;
    /* Calculate buffer index */
    tmptail = (USART_TERMINAL_rx_tail + 1) & USART_TERMINAL_RX_BUFFER_MASK;
    /* Store new index */
    USART_TERMINAL_rx_tail = tmptail;
    ENTER_CRITICAL(R);
    USART_TERMINAL_rx_elements--;
    EXIT_CRITICAL(R);

    /* Return data */
    return USART_TERMINAL_rxbuf[tmptail];
}

void USART_TERMINAL_Write(const uint8_t data)
{
    uint8_t tmphead;

    /* Calculate buffer index */
    tmphead = (USART_TERMINAL_tx_head + 1) & USART_TERMINAL_TX_BUFFER_MASK;
    /* Wait for free space in buffer */
    while (USART_TERMINAL_tx_elements == USART_TERMINAL_TX_BUFFER_SIZE)
            ;
    /* Store data in buffer */
    USART_TERMINAL_txbuf[tmphead] = data;
    /* Store new index */
    USART_TERMINAL_tx_head = tmphead;
    ENTER_CRITICAL(W);
    USART_TERMINAL_tx_elements++;
    EXIT_CRITICAL(W);
    /* Enable Tx interrupt */
    USART3.CTRLA |= (1 << USART_DREIE_bp);
}

void USART_TERMINAL_Initialize()
{
    //set baud rate register
    USART3.BAUD = (uint16_t)USART3_BAUD_RATE(115200);
	
    //RXCIE enabled; TXCIE disabled; DREIE disabled; RXSIE enabled; LBME disabled; ABEIE disabled; RS485 OFF; 
    USART3.CTRLA = 0x90;
	
    //RXEN enabled; TXEN enabled; SFDEN disabled; ODME disabled; RXMODE NORMAL; MPCM disabled; 
    USART3.CTRLB = 0xC0;
	
    //CMODE ASYNCHRONOUS; PMODE DISABLED; SBMODE 1BIT; CHSIZE 8BIT; UDORD disabled; UCPHA disabled; 
    USART3.CTRLC = 0x03;
	
    //DBGCTRL_DBGRUN
    USART3.DBGCTRL = 0x00;
	
    //EVCTRL_IREI
    USART3.EVCTRL = 0x00;
	
    //RXPLCTRL_RXPL
    USART3.RXPLCTRL = 0x00;
	
    //TXPLCTRL_TXPL
    USART3.TXPLCTRL = 0x00;
	

    uint8_t x;

    /* Initialize ringbuffers */
    x = 0;

    USART_TERMINAL_rx_tail     = x;
    USART_TERMINAL_rx_head     = x;
    USART_TERMINAL_rx_elements = x;
    USART_TERMINAL_tx_tail     = x;
    USART_TERMINAL_tx_head     = x;
    USART_TERMINAL_tx_elements = x;

}

void USART_TERMINAL_Enable()
{
    USART3.CTRLB |= USART_RXEN_bm | USART_TXEN_bm;
}

void USART_TERMINAL_EnableRx()
{
    USART3.CTRLB |= USART_RXEN_bm;
}

void USART_TERMINAL_EnableTx()
{
    USART3.CTRLB |= USART_TXEN_bm;
}

void USART_TERMINAL_Disable()
{
    USART3.CTRLB &= ~(USART_RXEN_bm | USART_TXEN_bm);
}

uint8_t USART_TERMINAL_GetData()
{
    return USART3.RXDATAL;
}