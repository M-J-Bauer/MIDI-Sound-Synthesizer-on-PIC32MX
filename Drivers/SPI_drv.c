/*^**************************************************************************\
 *
 * FileName:        SPI_drv.c
 * Processor:       PIC32MX...
 * Compiler:        MPLAB XC-32
 * Overview:        Basic SPI Driver
 * Author:          M.J.Bauer
 *
 * This driver assumes that all slave devices on the same SPI channel
 * use the same SPI clock mode and data transfer size (8 bits).
 *
 * Note:  PIC32MX460 has 2 SPI channels... 1 & 2.
 *        PIC32MX440 and PIC32MX340 have only SPI channel 2.
 *
\*****************************************************************************/

#include "../Common/Compiler.h"
#include "SPI_drv.h"

static int spiMutex[5] = { 0, 0, 0, 0, 0 };   // up to 4 channels


/*^
 * Entry arg(s):  channel  = SPI channel (1 or 2)
 *                clk_mode = SPI clock mode (0..3)
 *                brg_val  = BRG reg value to set SPI clock freq.
 * 
 * See file "SPI_drv.h" for details of clocking modes, etc.
 */
void  SPI_Init(char channel, char clk_mode, unsigned int brg_val)
{
#ifdef __32MX460F512L__ 
    if (channel == 1)  
    {
        SPI1BRG = brg_val;
        SPI1STAT = 0;
        SPI1CON = 0;
        SPI1CONbits.MSTEN = 1;
        SPI1CONbits.MODE16 = 0;
        SPI1CONbits.MODE32 = 0;
        SPI1CONbits.CKP = (clk_mode & 2) >> 1;  // = bit1
        SPI1CONbits.CKE = (clk_mode & 1) ^ 1;   // = !bit0
        SPI1CONbits.SMP = 1;
        SPI1CON |= (1 << 15);    // SPI1CONbits.ON = 1;
        spiMutex[1] = 0;
    }
#endif        
    if (channel == 2) 
    {
        SPI2BRG = brg_val;
        SPI2STAT = 0;
        SPI2CON = 0;
        SPI2CONbits.MSTEN = 1;
        SPI2CONbits.MODE16 = 0;
        SPI2CONbits.MODE32 = 0;
        SPI2CONbits.CKP = (clk_mode & 2) >> 1;  // = bit1
        SPI2CONbits.CKE = (clk_mode & 1) ^ 1;   // = !bit0
        SPI2CONbits.SMP = 1;
        SPI2CON |= (1 << 15);    // SPI2CONbits.ON = 1;
        spiMutex[2] = 0;
    }
}


// Function performs a complete data exchange, i.e. transmits a byte on MOSI
// while receiving a byte on MISO. The return value is the received byte.
//
BYTE  SPI_Transfer(unsigned int channel, unsigned char data)
{
    BYTE spiData;

#ifdef __32MX460F512L__ 
    if (channel == 1)
    {
        while (!SPI1STATbits.SPITBE)
            ;;;  // wait for TX buffer empty

        SPI1BUF = data;

        while (!SPI1STATbits.SPIRBF)
            ;;;  // wait for RX buffer full

        spiData = SPI1BUF;    // read RX buffer
    }
#endif        
    if (channel == 2)
    {
        while (!SPI2STATbits.SPITBE)
            ;;;  // wait for TX buffer empty

        SPI2BUF = data;

        while (!SPI2STATbits.SPIRBF)
            ;;;  // wait for RX buffer full

        spiData = SPI2BUF;    // read RX buffer
    }

    return  spiData;
}


// Function requests permission to use a given SPI channel...
// If the channel is already locked, it returns FALSE , i.e. permission denied;
// otherwise, it locks the channel and returns TRUE, i.e. permission granted.
// Device drivers must call this function before using an SPI channel to avoid
// contention with another device already using the required channel.
// Alias SPI_Lock(ch) is a macro defined in SPI_drv.h.
//
int  SPILock(unsigned int channel)
{
    int  result = 0;  // assume channel is locked

    if (!spiMutex[channel])  // channel is unlocked...
    {
        spiMutex[channel] = 1;  // lock it
        result = 1;
    }

    return  result;
}


// Function unlocks (releases) a specified SPI channel...
// A device driver which has locked an SPI channel must unlock it when finished using it,
// so that other devices can get access to the channel.
// Alias SPI_UnLock(ch) is a macro defined in SPI_drv.h.
//
void  SPIUnLock(unsigned int channel)
{
    spiMutex[channel] = 0;
}


// The functions defined below are deprecated -- not recommended for future use.
// They may be required for compatibility with some Microchip App Library modules.
//
// Function initiates a data transfer by writing a byte to the TX buffer.
// It waits for the cycle to complete (RX data availbale) before returning.
//
void  SPIPut(unsigned int channel, unsigned char data)
{
#ifdef __32MX460F512L__ 
    if (channel == 1)
    {
        while (!SPI1STATbits.SPITBE)
            ;;;  // wait for TX buffer empty

        SPI1BUF = data;

        while (!SPI1STATbits.SPIRBF)
            ;;;  // wait for RX buffer full
    }
#endif
    if (channel == 2)
    {
        while (!SPI2STATbits.SPITBE)
            ;;;  // wait for TX buffer empty

        SPI2BUF = data;

        while (!SPI2STATbits.SPIRBF)
            ;;;  // wait for RX buffer full
    }
}


// Function reads a byte from the RX buffer, assuming that the buffer has valid data.
// Normally, this function is called immediately after SPIPut().
//
BYTE  SPIGet(unsigned int channel)
{
    BYTE spiData;

#ifdef __32MX460F512L__ 
    if (channel == 1)  spiData = SPI1BUF;
#endif    
    if (channel == 2)  spiData = SPI2BUF;

    return  spiData;
}

