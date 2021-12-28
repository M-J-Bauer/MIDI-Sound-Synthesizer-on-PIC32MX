/*
 * Module:     UART_drv.c
 *
 * Overview:   PIC32MX UART driver library supporting UART1 and UART2.
 *
 * Author:     M.J.Bauer  2016  [www.mjbauer.biz]
 */
#include <stdlib.h>
#include <string.h>

#include "UART_drv.h"


#if UART1_RX_INTERRUPT_DRIVEN
static  uint8   U1RxBuffer[UART1_RXBUFSIZE];    // UART1 serial input RX FIFO buffer
static  uint8  *U1RxHead;                       // Pointer to next available unread char
static  uint8  *U1RxTail;                       // Pointer to next free place for writing
static  short   U1RxCount;                      // Number of unread chars in RX buffer
static  short   U1ErrCount;                     // UART1 Error count
#endif

#if UART1_TX_USING_QUEUE 
static  uint8   U1TxBuffer[UART1_TXBUFSIZE];    // UART1 serial output TX FIFO buffer
static  uint8  *U1TxHead;                       // Pointer to next char to be sent out
static  uint8  *U1TxTail;                       // Pointer to next free place in queue
static  short   U1TxCount;                      // Number of unsent chars in TX buffer
#endif

#if UART2_RX_INTERRUPT_DRIVEN
static  uint8   U2RxBuffer[UART2_RXBUFSIZE];    // UART2 serial input RX FIFO buffer
static  uint8  *U2RxHead;                       // Pointer to next available unread char
static  uint8  *U2RxTail;                       // Pointer to next free place in buffer
static  short   U2RxCount;                      // Number of unread chars in RX buffer
static  short   U2ErrCount;                     // UART2 Error count
#endif

#if UART2_TX_USING_QUEUE
static  uint8   U2TxBuffer[UART2_TXBUFSIZE];    // UART2 serial output TX FIFO buffer
static  uint8  *U2TxHead;                       // Pointer to next char to be sent out
static  uint8  *U2TxTail;                       // Pointer to next free place in queue
static  short   U2TxCount;                      // Number of unsent chars in TX buffer
#endif

static volatile uint8 dummy;


//========================================================================================
//
// Initialize the serial port on UART1;  Arg = baudrate (e.g. 57600)
// No parity, 8 data bits, 1 stop bit.
//
void  UART1_init(uint16 baudrate)
{
    float BRG_reg = ((float) GetSystemClock() / (baudrate * 16)) - 1;

    U1MODE = 0x0000;                    // ON = 0 ... reset UART
    U1BRG = (uint32) (BRG_reg + 0.5);   // round to nearest whole #
    U1STA = 0x1400;                     // UTXEN = 1; URXEN = 1
    U1MODE = 0x8000;                    // ON = 1 ... enable UART

#if UART1_TX_USING_QUEUE
    U1TxHead = U1TxBuffer;
	U1TxTail = U1TxBuffer;
	U1TxCount = 0;
#endif

#if UART1_RX_INTERRUPT_DRIVEN
    UART1_RX_IRQ_DISABLE();
    IPC6bits.U1IP = 3;       // IRQ priority
    U1RxHead = U1RxBuffer;
	U1RxTail = U1RxBuffer;
	U1RxCount = 0;
    UART1_RX_IRQ_ENABLE();
    UART1_ERR_IRQ_ENABLE();
#endif
}


#if UART1_RX_INTERRUPT_DRIVEN

/*
*   UART#_RxDataAvail() - Checks receive buffer for data available.
*
*   Returns:  TRUE if UART Rx buffer contains unread char(s), else FALSE.
*/
uint8  UART1_RxDataAvail(void)
{
    return (U1RxCount > 0);
}

/*
*   UART#_RxFlush() - Clears UART hardware and software Rx FIFO buffers.
*
*   Returns:  --
*/
void  UART1_RxFlush(void)
{
    UART1_RX_IRQ_DISABLE();

    while (U1STAbits.URXDA)  // Clear UART periph Rx FIFO
    {
        dummy = U1RXREG;
    }

    U1RxHead = U1RxBuffer;    // Clear RAM Rx FIFO buffer
	U1RxTail = U1RxBuffer;
	U1RxCount = 0;

    UART1_RX_IRQ_ENABLE();
}

/*
*   UART#_getch() - Fetches next unread char from UART RX input buffer.
*
*   The function DOES NOT WAIT for data available in the input buffer;
*   the caller should first check using the function UARTx_RXdataAvail().
*   If there is no data available, the function returns NUL (0).
*   The input char is NOT echoed back to the UART output stream.
*
*   Returns:    Byte from UART RX input buffer (or 0, if buffer is empty).
*/
uint8  UART1_getch( void )
{
    uint8  b = 0;

    if (U1RxCount > 0)
    {
        UART1_RX_IRQ_DISABLE();  // hold off IRQ while accessing buffer

        b = *U1RxHead++;

        if ( U1RxHead >= (U1RxBuffer + UART1_RXBUFSIZE) )  // wrap
			U1RxHead = U1RxBuffer;
        U1RxCount--;

        UART1_RX_IRQ_ENABLE();
    }

    return  b;
}


int  UART1_getErrorCount(void)
{
    int  errcount = U1ErrCount;
    
    U1ErrCount = 0;  // clear the error count before returning
    
    return  errcount;
}


/*
*   UART1 Interrupt Service Routine
*
*   Received bytes are placed into the RX FIFO buffer in data memory.
*
*/
void  __ISR(_UART_1_VECTOR, IPL4AUTO)  UART1_IRQ_Handler(void)
{
    if (UART1_RX_IRQ_FLAG())
    {
        UART1_RX_IRQ_CLEAR();

        while (U1STAbits.URXDA && U1RxCount < UART1_RXBUFSIZE)
        {
            *U1RxTail++ = (uint8) U1RXREG;

            if ( U1RxTail >= (U1RxBuffer + UART1_RXBUFSIZE) )  // wrap
                U1RxTail = U1RxBuffer;
            U1RxCount++;
        }
    }
    else  // if (UART1_ERR_IRQ_FLAG())  
    {
        UART1_ERR_IRQ_CLEAR();
    
        U1ErrCount++;
        
        // todo: Clear the *source* of the error ???
    }
}

#else  // USE POLLED RX INPUT

/*
*   UART#_RxDataAvail() - Checks UART receive buffer for data available.
*
*   Returns:  TRUE if UART Rx buffer contains unread char(s), else FALSE.
*/
uint8  UART1_RxDataAvail(void)
{
    return (U1STAbits.URXDA != 0);
}

/*
*   UART#_RxFlush() - Clears UART hardware Rx FIFO buffer.
*
*   Returns:  --
*/
void  UART1_RxFlush(void)
{
    while (U1STAbits.URXDA)  // Clear Rx FIFO Buffer
    {
        dummy = U1RXREG;
    }
}

/*
*   UART#_getch() - Fetches next unread char from UART RX input buffer.
*
*   The function DOES NOT WAIT for data available in the input buffer;
*   the caller should first check using the function UARTx_RXdataAvail().
*   If there is no data available, the function returns NUL (0).
*   The input char is NOT echoed back to the UART output stream.
*
*   Returns:    Byte from UART RX input buffer (or 0, if buffer is empty).
*/
uint8  UART1_getch( void )
{
    uint8  b = 0;

    if (U1STAbits.URXDA) b = U1RXREG;
    return  b;
}

#endif

#if UART1_TX_USING_QUEUE
/*
*   UART#_putch() - Places a byte (arg1) into the Tx queue (FIFO buffer).
*
*   If the queue is full, the function returns immediately with value 0xFF,
*   so that application processes are not blocked.
*
*   Returns:  byte queued (arg1), or 0xFF if the queue is full.
*/
uint8  UART1_putch( uint8 b )
{
    if (U1TxCount < UART1_TXBUFSIZE)
    {
        *U1TxTail++ = b;

        if ( U1TxTail >= (U1TxBuffer + UART1_TXBUFSIZE) )  // wrap
			U1TxTail = U1TxBuffer;
        U1TxCount++;
    }
    else  b = 0xFF;

    return  b;
}

/*
|  UART#_putstr() - Places a NUL-terminated string into the output queue.
|
|  Newline (0x0A) is expanded to CR + LF (0x0D + 0x0A).
*/
void  UART1_putstr( char *pstr )
{
    char c;

    while ( (c = *pstr++) != '\0' )
    {
        if ( c == '\n' )
        {
            UART1_putch( '\r' );
            UART1_putch( '\n' );
        }
        else   UART1_putch( c );
    }
}

/*
|  UART# Tx Queue Handler...
|  Routine called by the application to transmit a byte stored in the output queue.
|  Only one byte is sent on each call, so that blocking of time-critical application
|  functions can be prevented. If the Tx queue is empty, the function just returns.
*/
void  UART1_TxQueueHandler()
{
    if (U1TxCount > 0)
    {
        while (U1STAbits.UTXBF)  { ;;; }   // wait for TX reg empty

        U1TXREG = *U1TxHead++;

        if ( U1TxHead >= (U1TxBuffer + UART1_TXBUFSIZE) )  // wrap!
			U1TxHead = U1TxBuffer;
        U1TxCount--;
    }
}

#else  // USE DIRECT TX OUTPUT FUNCTIONS

/*
*   UART#_putch() - Outputs a byte (arg1) to the UART TX register, if empty.
*
*   The function waits for the TX register to be cleared first.
*
*   Returns:  byte sent (arg1).
*/
uint8  UART1_putch( uint8 b )
{
    while (U1STAbits.UTXBF)  { ;;; }   // wait for TX reg empty

    U1TXREG = b;
    return  b;
}

/*
|  Output a NUL-terminated string.
|  The string is expected to be in the data memory (RAM) space.
|  Newline (0x0A) is expanded to CR + LF (0x0D + 0x0A).
*/
void  UART1_putstr( char *pstr )
{
    char c;

    while ( (c = *pstr++) != '\0' )
    {
        if ( c == '\n' )
        {
            UART1_putch( '\r' );
            UART1_putch( '\n' );
        }
        else   UART1_putch( c );
    }
}

#endif


//========================================================================================
//                        ~~~~~  UART2  ~~~~~
//
// Initialize the serial port on UART2;  Arg = baudrate (e.g. 57600)
// No parity, 8 data bits, 1 stop bit.
//
void UART2_init(uint16 baudrate)
{
    float BRG_reg = ((float) GetSystemClock() / (baudrate * 16)) - 1;

    U2MODE = 0x0000;                    // ON = 0 ... reset UART
    U2BRG = (uint32) (BRG_reg + 0.5);   // round to nearest whole #
    U2STA = 0x1400;                     // UTXEN = 1; URXEN = 1
    U2MODE = 0x8000;                    // ON = 1 ... enable UART

#if UART2_TX_USING_QUEUE
    U2TxHead = U2TxBuffer;
	U2TxTail = U2TxBuffer;
	U2TxCount = 0;
#endif

#if UART2_RX_INTERRUPT_DRIVEN
    UART2_RX_IRQ_DISABLE();
    IPC8bits.U2IP = 3;       // IRQ priority
    U2RxHead = U2RxBuffer;
	U2RxTail = U2RxBuffer;
	U2RxCount = 0;
    UART2_RX_IRQ_ENABLE();
    UART2_ERR_IRQ_ENABLE();
#endif
}


#if UART2_RX_INTERRUPT_DRIVEN

/*^
*   UART#_RxDataAvail() - Checks receive buffer for data available.
*
*   Returns:  TRUE if UART Rx buffer contains unread char(s), else FALSE.
*/
uint8  UART2_RxDataAvail(void)
{
    return (U2RxCount > 0);
}

/*^
*   UART#_RxFlush() - Clears UART hardware and software Rx FIFO buffers.
*
*   Returns:  --
*/
void  UART2_RxFlush(void)
{
    UART2_RX_IRQ_DISABLE();

    while (U2STAbits.URXDA)   // Clear UART periph Rx FIFO
    {
        dummy = U2RXREG;
    }

    U2RxHead = U2RxBuffer;    // Clear RAM Rx FIFO buffer
	U2RxTail = U2RxBuffer;
	U2RxCount = 0;

    UART2_RX_IRQ_ENABLE();
}

/*^
*   UART#_getch() - Fetches next unread char from UART RX input buffer.
*
*   The function DOES NOT WAIT for data available in the input buffer;
*   the caller should first check using the function UARTx_RXdataAvail().
*   If there is no data available, the function returns NUL (0).
*   The input char is NOT echoed back to the UART output stream.
*
*   Returns:    Byte from UART RX input buffer (or 0, if buffer is empty).
*/
uint8  UART2_getch( void )
{
    uint8  b = 0;

    if (U2RxCount > 0)
    {
        UART2_RX_IRQ_DISABLE();  // hold off IRQ while accessing buffer

        b = *U2RxHead++;

        if ( U2RxHead >= (U2RxBuffer + UART2_RXBUFSIZE) )  // wrap
			U2RxHead = U2RxBuffer;
        U2RxCount--;

        UART2_RX_IRQ_ENABLE();
    }

    return  b;
}


int  UART2_getErrorCount(void)
{
    int  errcount = U2ErrCount;
    
    U2ErrCount = 0;  // clear the error count before returning
    
    return  errcount;
}


/*^
*   UART2 Interrupt Service Routine
*
*   Received bytes are placed into the RX FIFO buffer in data memory.
* 
*/
void  __ISR(_UART_2_VECTOR, IPL3AUTO)  UART2_IRQ_Handler(void)
{
    if (UART2_RX_IRQ_FLAG())
    {
        UART2_RX_IRQ_CLEAR();

        while (U2STAbits.URXDA && U2RxCount < UART2_RXBUFSIZE)
        {
            *U2RxTail++ = (uint8) U2RXREG;

            if ( U2RxTail >= (U2RxBuffer + UART2_RXBUFSIZE) )  // wrap
                U2RxTail = U2RxBuffer;
            U2RxCount++;
        }
    }
    else  // if (UART2_ERR_IRQ_FLAG())  
    {
        UART2_ERR_IRQ_CLEAR();
        
        U2ErrCount++;
    
        // todo: Clear the *source* of the error ???
    }    
}

#else  // USE POLLED RX INPUT

/*
*   UART#_RxDataAvail() - Checks UART receive buffer for data available.
*
*   Returns:  TRUE if UART Rx buffer contains unread char(s), else FALSE.
*/
uint8 UART2_RxDataAvail(void)
{
    return (U2STAbits.URXDA != 0);
}

/*
*   UART#_RxFlush() - Clears UART hardware Rx FIFO buffer.
*
*   Returns:  --
*/
void UART2_RxFlush(void)
{
    while (U2STAbits.URXDA)   // Clear Rx FIFO Buffer
    {
        dummy = U2RXREG;
    }
}

/*
*   UART#_getch() - Fetches next unread char from UART RX input buffer.
*
*   The function DOES NOT WAIT for data available in the input buffer;
*   the caller should first check using the function UARTx_RXdataAvail().
*   If there is no data available, the function returns NUL (0).
*   The input char is NOT echoed back to the UART output stream.
*
*   Returns:    Byte from UART RX input buffer (or 0, if buffer is empty).
*/
uint8 UART2_getch( void )
{
    uint8  b = 0;

    if (U2STAbits.URXDA) b = U2RXREG;
    return  b;
}

#endif

#if UART2_TX_USING_QUEUE
/*
|   UART#_putch() - Places a byte (arg1) into the Tx queue (FIFO buffer).
|
|   If the queue is full, the function returns immediately with value 0xFF,
|   so that application processes are not blocked.
|
|   Returns:  byte queued (arg1), or 0xFF if the queue is full.
*/
uint8  UART2_putch( uint8 b )
{
    if (U2TxCount < UART2_TXBUFSIZE)
    {
        *U2TxTail++ = b;

        if ( U2TxTail >= (U2TxBuffer + UART2_TXBUFSIZE) )  // wrap
			U2TxTail = U2TxBuffer;
        U2TxCount++;
    }
    else  b = 0xFF;

    return  b;
}

/*
|  UART#_putstr() - Places a NUL-terminated string into the output queue.
|
|  Newline (0x0A) is expanded to CR + LF (0x0D + 0x0A).
*/
void  UART2_putstr( char *pstr )
{
    char c;

    while ( (c = *pstr++) != '\0' )
    {
        if ( c == '\n' )
        {
            UART2_putch( '\r' );
            UART2_putch( '\n' );
        }
        else   UART2_putch( c );
    }
}

/*
|  UART# Tx Queue Handler...
|  Routine called by the application to transmit a byte stored in the output queue.
|  Only one byte is sent on each call, so that blocking of time-critical application
|  functions can be prevented. If the Tx queue is empty, the function just returns.
*/
void  UART2_TxQueueHandler()
{
    if (U2TxCount > 0)
    {
        while (U2STAbits.UTXBF)  { ;;; }   // wait for TX reg empty

        U2TXREG = *U2TxHead++;

        if ( U2TxHead >= (U2TxBuffer + UART2_TXBUFSIZE) )  // wrap!
			U2TxHead = U2TxBuffer;
        U2TxCount--;
    }
}

#else  // USE DIRECT TX OUTPUT FUNCTIONS

/*
*   UART#_putch() - Outputs a byte (arg1) to the UART TX register, if empty.
*
*   The function waits for the TX register to be cleared first.
*
*   Returns:  byte sent (arg1).
*/
uint8  UART2_putch( uint8 b )
{
    while (U2STAbits.UTXBF)  { ;;; }   // wait for TX reg empty

    U2TXREG = b;
    return  b;
}

/*
|  Output a NUL-terminated string.
|  The string is expected to be in the data memory (RAM) space.
|  Newline (0x0A) is expanded to CR + LF (0x0D + 0x0A).
*/
void  UART2_putstr( char *pstr )
{
    char c;

    while ( (c = *pstr++) != '\0' )
    {
        if ( c == '\n' )
        {
            UART2_putch( '\r' );
            UART2_putch( '\n' );
        }
        else   UART2_putch( c );
    }
}

#endif

/* end of file */
