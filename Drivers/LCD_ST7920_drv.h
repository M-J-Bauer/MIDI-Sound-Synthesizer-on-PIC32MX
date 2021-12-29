/*
 * File:    LCD_ST7920_drv.h
 *
 * Low-level driver def's for monochrome Graphic LCD module, 128 x 64 pixels.
 *
 * Customized for LCD controller silicon:  ST7920.
 */
#ifndef LCD_ST7920_DRV_H
#define LCD_ST7920_DRV_H

#include "../system_def.h"
#include "../Common/GenericTypeDefs.h"

#define LCD_DATA_DIRN_IN()     (TRISE |= 0x00FF)
#define LCD_DATA_DIRN_OUT()    (TRISE &= ~0x00FF)
#define LCD_DATA_INPUT()       (PORTE & 0x00FF)
#define LCD_DATA_OUTPUT(d)     (LATE = (LATE & 0xFF00) | (d & 0x00FF))
#define LCD_CTRL_DIRN_OUT()    (TRISD &= ~(0x1F << 5))   // RD5..RD9
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

#ifndef NOP
#define NOP()                  asm volatile ("NOP")
#endif


//  LCD controller low-level functions defined in this driver module,
//  accessible by higher-level modules...
//
bool    LCD_Init(void);
void    LCD_ClearGDRAM();
void    LCD_WriteBlock(uint16 *scnBuf, uint16 x, uint16 y, uint16 w, uint16 h);
void    LCD_Test();


#endif  // LCD_ST7920_DRV_H
