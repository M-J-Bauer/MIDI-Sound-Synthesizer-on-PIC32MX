/**
 *   File:  UART_drv.h 
 *
 *   Def's for PIC32MX UART driver library supporting UART1 and UART2.
 */
#ifndef UART_LIB_H
#define UART_LIB_H

#include <sys/attribs.h>    // For interrupt handlers
#include "../Common/system_def.h"

//================  UART DRIVER BUILD OPTIONS  ========================
//
#ifdef BUILD_REMI_SYNTH2_APP  // Application-specific config. 
#define UART1_RX_INTERRUPT_DRIVEN  1
#define UART2_RX_INTERRUPT_DRIVEN  1
#define UART1_TX_USING_QUEUE  1
#define UART2_TX_USING_QUEUE  0
#endif

// If any of these symbols are not already defined above,
// the following default settings will be applied...
//
#ifndef UART1_RX_INTERRUPT_DRIVEN
#define UART1_RX_INTERRUPT_DRIVEN  0   // if 0, use polled RX input
#endif

#ifndef UART2_RX_INTERRUPT_DRIVEN
#define UART2_RX_INTERRUPT_DRIVEN  0   // if 0, use polled RX input
#endif

#ifndef UART1_TX_USING_QUEUE
#define UART1_TX_USING_QUEUE  0    // if 0, use direct TX functions
#endif 

#ifndef UART2_TX_USING_QUEUE
#define UART2_TX_USING_QUEUE  0    // if 0, use direct TX functions
#endif

#ifndef UART1_TXBUFSIZE
#define UART1_TXBUFSIZE      128   // TX FIFO buffer size, chars
#endif

#ifndef UART2_TXBUFSIZE
#define UART2_TXBUFSIZE      128
#endif

#ifndef UART1_RXBUFSIZE
#define UART1_RXBUFSIZE      256   // RX FIFO buffer size, chars
#endif

#ifndef UART2_RXBUFSIZE
#define UART2_RXBUFSIZE      256
#endif
//
//=====================================================================

#define UART1_RX_IRQ_DISABLE()   IEC0bits.U1RXIE = 0
#define UART1_RX_IRQ_ENABLE()    IEC0bits.U1RXIE = 1
#define UART1_RX_IRQ_CLEAR()     IFS0bits.U1RXIF = 0  
#define UART1_RX_IRQ_FLAG()      (IFS0bits.U1RXIF)

#define UART1_ERR_IRQ_DISABLE()  IEC0bits.U1EIE = 0
#define UART1_ERR_IRQ_ENABLE()   IEC0bits.U1EIE = 1
#define UART1_ERR_IRQ_CLEAR()    IFS0bits.U1EIF = 0  
#define UART1_ERR_IRQ_FLAG()     (IFS0bits.U1EIF)

#define UART2_RX_IRQ_DISABLE()   IEC1bits.U2RXIE = 0
#define UART2_RX_IRQ_ENABLE()    IEC1bits.U2RXIE = 1
#define UART2_RX_IRQ_CLEAR()     IFS1bits.U2RXIF = 0
#define UART2_RX_IRQ_FLAG()      (IFS1bits.U2RXIF)

#define UART2_ERR_IRQ_DISABLE()  IEC1bits.U2EIE = 0
#define UART2_ERR_IRQ_ENABLE()   IEC1bits.U2EIE = 1
#define UART2_ERR_IRQ_CLEAR()    IFS1bits.U2EIF = 0  
#define UART2_ERR_IRQ_FLAG()     (IFS1bits.U2EIF)


void   UART1_init( uint16 br );
uint8  UART1_RxDataAvail(void);
void   UART1_RxFlush(void);
uint8  UART1_getch( void );
uint8  UART1_putch( uint8 b );
void   UART1_putstr( char *pstr );
void   UART1_TxQueueHandler();
int    UART1_getErrorCount(void);

void   UART2_init( uint16 br );
uint8  UART2_RxDataAvail(void);
void   UART2_RxFlush(void);
uint8  UART2_getch( void );
uint8  UART2_putch( uint8 b );
void   UART2_putstr( char *pstr );
void   UART2_TxQueueHandler();
int    UART2_getErrorCount(void);

#endif // UART_LIB_H
