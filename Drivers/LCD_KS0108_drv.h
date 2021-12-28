/*
 * File:    LCD_KS0108_drv.h
 *
 * Low-level driver def's for monochrome Graphic LCD module, 128 x 64 pixels.
 *
 * Adapted for KS0108 LCD controller chipset and PIC32MX micro-controller.
 */
#ifndef LCD_KS0108_DRV_H
#define LCD_KS0108_DRV_H

#include "../Common/GenericTypeDefs.h"
#include "../Common/system_def.h"

// Comment out the next line for Active-High Chip Select...
//#define LCD_CHIPSELECT_ACTIVE_LOW  1

#define LCD_CTRL_DIRN_OUT()    (TRISD &= ~(0x7F << 5))   // RD5..RD11
#define LCD_DATA_DIRN_IN()     (TRISE |= 0x00FF)
#define LCD_DATA_DIRN_OUT()    (TRISE &= ~0x00FF)
#define LCD_DATA_INPUT()       (PORTE & 0x00FF)
#define LCD_DATA_OUTPUT(d)     (LATE = d & 0x00FF)

// Control (CTRL) signals...
#define LCD_E_HIGH()           (LATDbits.LATD5 = 1)
#define LCD_E_LOW()            (LATDbits.LATD5 = 0)
#define LCD_RS_HIGH()          (LATDbits.LATD6 = 1)
#define LCD_RS_LOW()           (LATDbits.LATD6 = 0)
#define LCD_RW_HIGH()          (LATDbits.LATD7 = 1)
#define LCD_RW_LOW()           (LATDbits.LATD7 = 0)
#define LCD_RST_HIGH()         (LATDbits.LATD8 = 1)
#define LCD_RST_LOW()          (LATDbits.LATD8 = 0)
#define LCD_BACKLIGHT_HIGH()   (LATDbits.LATD9 = 1)
#define LCD_BACKLIGHT_LOW()    (LATDbits.LATD9 = 0)

#ifdef LCD_CHIPSELECT_ACTIVE_LOW  
#define LCD_CS1_OFF()          (LATDbits.LATD10 = 1)
#define LCD_CS1_ON()           (LATDbits.LATD10 = 0)
#define LCD_CS2_OFF()          (LATDbits.LATD11 = 1)
#define LCD_CS2_ON()           (LATDbits.LATD11 = 0)
#else  // LCD_CHIPSELECT_ACTIVE_HIGH
#define LCD_CS1_OFF()          (LATDbits.LATD10 = 0)
#define LCD_CS1_ON()           (LATDbits.LATD10 = 1)
#define LCD_CS2_OFF()          (LATDbits.LATD11 = 0)
#define LCD_CS2_ON()           (LATDbits.LATD11 = 1)
#endif

// KS0108 Command Codes  (NB: 'Y-address' is *horizontal* pixel coord!)
#define LCD_SET_Y_ADDR         0x40    // bits 5:0 => Y-address (0..63)
#define LCD_SET_PAGE_ADDR      0xB8    // bits 2:0 => Page addr (0..7)
#define LCD_SET_START_LINE     0xC0    // bits 5:0 => Start line (0..63)
#define LCD_SET_DISPLAY_ON     0x3F    // bit 0: 1 => display ON
#define LCD_SET_DISPLAY_OFF    0x3E    // bit 0: 0 => display OFF

// KS0108 Status Register bitmasks
#define LCD_STATUS_BUSY        0x80    // bit 7
#define LCD_STATUS_OFF         0x20    // bit 5
#define LCD_STATUS_RESET       0x10    // bit 4

//  This module calls an external function which provides a time delay
//  of a multiple of 25ns. The arg (count) is that multiple, maximum value 20.
void    Delay_Nx25ns(unsigned int count);

//  LCD controller low-level functions defined in this driver module,
//  accessible by higher-level modules...
//
void    LCD_WriteCommand(uint8 cmd);
void    LCD_WriteData(uint8 data);
uint8   LCD_ReadStatus();
void    LCD_Reset(void);
bool    LCD_Init(void);
void    LCD_ClearGDRAM();
void    LCD_WriteBlock(uint16 *scnBuf, uint16 x, uint16 y, uint16 w, uint16 h);
void    LCD_Test();
void    LCD_BacklightToggle();


#endif  // LCD_KS0108_DRV_H
