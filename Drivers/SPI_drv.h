/****************************************************************************\
 *
 * FileName:        SPI_drv.h
 * Processor:       PIC32MX...
 * Compiler:        MPLAB XC-32
 * Overview:        Basic SPI Driver
 *
\*****************************************************************************/

#ifndef _SPI_DRV_H
#define _SPI_DRV_H

#include "../Common/GenericTypeDefs.h"
#include "../Common/system_def.h"

/*
 * Function SPI_Init() -- Entry arg(s):  
 *                channel  = SPI channel (1 or 2)
 *                clk_mode = SPI clock mode (0..3), see table below
 *                brg_val  = BRG reg value to set SPI clock freq.
 * 
 * CKE specifies clock edge to begin TX cycle: 0 => Idle-to-Active, 1 => Active-to-Idle
 * CKP specifies clock polarity in Idle state: 0 => Low, 1 => High
 * 
 *    CK mode   |  CKP  |  CKE
 *   -----------+-------+--------
 *    0  (0,0)  |   0   |   1
 *    1  (0,1)  |   0   |   0
 *    2  (1,0)  |   1   |   1
 *    3  (1,1)  |   1   |   0
 * 
 *  SPI CLK freq = PBCLK / (2 x (SPIxBRG + 1))
 *
 *  Examples --- Assume PBCLK = FSYS = 80MHz -------------
 *      (1)  SPIxBRG =  1 :  SPI.CLK = 80 / 4   = 20 MHz
 *      (2)  SPIxBRG =  3 :  SPI.CLK = 80 / 8   = 10 MHz
 *      (3)  SPIxBRG =  4 :  SPI.CLK = 80 / 10  =  8 MHz
 *      (4)  SPIxBRG =  7 :  SPI.CLK = 80 / 16  =  5 MHz
 *      (5)  SPIxBRG =  9 :  SPI.CLK = 80 / 20  =  4 MHz
 *      (6)  SPIxBRG = 19 :  SPI.CLK = 80 / 40  =  2 MHz
 *      (7)  SPIxBRG = 39 :  SPI.CLK = 80 / 80  =  1 MHz
 */

// Definition required for compatibility with Microchip Library modules...
typedef struct
{
    int     channel;
    int	    baudRate;
    int     dummy;
    char    cke;
    char    ckp;
    char    smp;
    char    mode;

} DRV_SPI_INIT_DATA;


// Dummy macro required for compatibility with Microchip Library modules...
#define DRV_SPI_Initialize(ch,pd)


// Preferred functions...
//
void  SPI_Init(char channel, char clk_mode, unsigned int brg_val);
BYTE  SPI_Transfer(unsigned int channel, unsigned char data);
int   SPILock(unsigned int channel);
void  SPIUnLock(unsigned int channel);

#define SPI_Lock(ch)     SPILock(ch)
#define SPI_UnLock(ch)   SPIUnLock(ch)

// Deprecated functions required for compatibility with Microchip Library...
//
void  SPIPut(unsigned int channel, unsigned char data);
BYTE  SPIGet(unsigned int channel);


#endif // _SPI_DRV_H

