/*
 * File: LCD_graphics_lib.h
 *
 * API Library definitions for Monochrome Graphic LCD module, 128 x 64 pixels.
 */
 
#ifndef LCD_GRAPHICS_LIB_H
#define LCD_GRAPHICS_LIB_H

#include "../Common/system_def.h"
#include "HardwareProfile.h"

// Determine LCD controller device based on def's in HardwareProfile.h
#ifdef USE_LCD_CONTROLLER_ST7920 
#include "../Drivers/LCD_ST7920_drv.h"
#endif
#ifdef USE_LCD_CONTROLLER_KS0108  
#include "../Drivers/LCD_KS0108_drv.h"
#endif
#ifdef USE_OLED_CONTROLLER_SH1106  
#include "../Drivers/OLED_SH1106_drv.h"
#include "../Drivers/I2C_drv.h"
#endif

// Rendering modes for LCD write functions...
#define CLEAR_PIXELS          0
#define SET_PIXELS            1
#define FLIP_PIXELS           2

//#define SUPPORT_12PT_AND_24PT_FONTS    // Maybe someday! - MJB

// Character font styles;  size is cell height in pixels.
// Use one of the font names defined here as the arg value in function: LCD_SetFont(arg).
// Note:  Font size 16 is monospace only -- N/A in proportional spacing.
//        Font sizes 12 and 24 use proportional spacing -- N/A in monospace.
//
enum  Graphics_character_fonts
{
    MONO_8_NORM = 0,   // (0) Mono-spaced font;  char width is 5 pix
    MONO_8_BOLD_X,     // (1) N/A 
    PROP_8_NORM,       // (2) Proportional font;  char width is 3..5 pix
    PROP_8_BOLD_X,     // (3) N/A 

    MONO_12_NORM_X,    // (4) N/A
    MONO_12_BOLD_X,    // (5) N/A
    PROP_12_NORM,      // (6) Proportional font;  char width is 4..7 pix
    PROP_12_BOLD,      // (7) as above, but bold weight

    MONO_16_NORM,      // (8) Mono-spaced font;  char width is 10 pix
    MONO_16_BOLD,      // (9) as above, but bold weight
    PROP_16_NORM_X,    // (10) N/A
    PROP_16_BOLD_X,    // (10) N/A

    MONO_24_NORM_X,    // (12) N/A
    MONO_24_BOLD_X,    // (13) N/A
    PROP_24_NORM,      // (14) Proportional font;  char width is 7..15 pix
    PROP_24_BOLD_X     // (15) N/A  (already bold!)
};


typedef  const unsigned char  bitmap_t;  // Bitmap image is an array of bytes in flash


//---------- Controller low-level functions, defined in driver module -------------------
//
extern  bool  LCD_Init(void);      // LCD controller initialisation
extern  void  LCD_ClearGDRAM();
extern  void  LCD_WriteBlock(uint16 *scnBuf, uint16 x, uint16 y, uint16 w, uint16 h);

//---------- LCD function & macro library (API) -----------------------------------------
//
#define LCD_GetMaxX()       (127)
#define LCD_GetMaxY()       (63)

void    LCD_ClearScreen(void);            // Clear LCD GDRAM and MCU RAM buffers
void    LCD_Mode(uint8 mode);             // Set pixel write mode (set, clear, flip)
void    LCD_PosXY(uint16 x, uint16 y);    // Set graphics cursor position to (x, y)
uint16  LCD_GetX(void);                   // Get graphics cursor pos x-coord
uint16  LCD_GetY(void);                   // Get graphics cursor pos y-coord
void    LCD_SetFont(uint8 font_ID);       // Set font for char or text display
uint8   LCD_GetFont();                    // Get current font ID
void    LCD_PutChar(char uc);             // Show ASCII char at (x, y)
void    LCD_PutText(char *str);           // Show text string at (x, y)
void    LCD_PutDigit(uint8 bDat);         // Show hex/decimal digit value (1 char)
void    LCD_PutHexByte(uint8 bDat);       // Show hexadecimal byte value (2 chars)

void    LCD_PutDecimalWord(uint16 val, uint8 fieldSize);  // Show uint16 in decimal
void    LCD_BlockFill(uint16 w, uint16 h);   // Fill area, w x h pixels, at cursor (x, y)
void    LCD_BlockClear(uint16 w, uint16 h);  // Clear area, w x h pixels, at cursor (x, y)
uint8   LCD_PutImage(bitmap_t *image, uint16 w, uint16 h);  // Show bitmap image at (x, y)
uint16 *LCD_ScreenCapture();              // Return a pointer to the screen buffer

// These macros draw various objects at the current graphics cursor position...
#define LCD_PutPixel()           LCD_BlockFill(1, 1)
#define LCD_DrawBar(w, h)        LCD_BlockFill(w, h)
#define LCD_DrawLineHoriz(len)   LCD_BlockFill(len, 1)
#define LCD_DrawLineVert(len)    LCD_BlockFill(1, len)


//---------- Aliases for OLED (or other 128 x 64 pixel display) ----------------------------
//
#define Disp_GetMaxX()      (127)                   // Screen width, pixels
#define Disp_GetMaxY()      (63)                    // Screen height, pixels
#define Disp_Init()         LCD_Init()              // Controller initialisation
#define Disp_ClearScreen()  LCD_ClearScreen()       // Clear GDRAM and MCU RAM buffers
#define Disp_Mode(mode)     LCD_Mode(mode)          // Set pixel write mode (set, clear, flip)
#define Disp_PosXY(x, y)    LCD_PosXY(x, y)         // Set graphics cursor position
#define Disp_GetX()         LCD_GetX()              // Get cursor pos'n x-coord
#define Disp_GetY()         LCD_GetY()              // Get cursor pos'n y-coord
#define Disp_SetFont(font)  LCD_SetFont(font)       // Set font for text
#define Disp_GetFont()      LCD_GetFont()           // Get current font ID
#define Disp_PutChar(c)     LCD_PutChar(c)          // Show ASCII char at (x, y)
#define Disp_PutText(s)     LCD_PutText(s)          // Show text string at (x, y)
#define Disp_PutDigit(d)    LCD_PutDigit(d)         // Show hex/decimal digit (1 char)
#define Disp_PutHexByte(h)  LCD_PutHexByte(h)       // Show hexadecimal byte (2 chars)

#define Disp_PutDecimal(w, n)     LCD_PutDecimalWord(w, n)  // Show uint16 in decimal (n places)
#define Disp_BlockFill(w, h)      LCD_BlockFill(w, h)       // Fill area w x h pixels at (x, y)
#define Disp_PutImage(img, w, h)  LCD_PutImage(img, w, h)   // Show bitmap image at (x, y)
#define Disp_ScreenCapture()      LCD_ScreenCapture()       // Return a pointer to the screen buffer

#define Disp_PutPixel()           LCD_BlockFill(1, 1)
#define Disp_DrawBar(w, h)        LCD_BlockFill(w, h)
#define Disp_DrawLineHoriz(len)   LCD_BlockFill(len, 1)
#define Disp_DrawLineVert(len)    LCD_BlockFill(1, len)


#endif  // LCD_GRAPHICS_LIB_H
