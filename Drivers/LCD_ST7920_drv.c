/*
 * File:    LCD_ST7920_drv.c
 *
 * Low-level driver functions for monochrome Graphic LCD module, 128 x 64 pixels.
 *
 * Customized for LCD controller silicon:  ST7920.
 */
#include "LCD_ST7920_drv.h"


/*----------------------------------------------------------------------------------
 * Name               :  Delay_2xTCY()
 *
 * Function           :  Delay a specified number of MCU instruction cycles (x2).
 * Dependency         :  PIC32MX3/4/7 CPU @ Fsys = 80MHz, Pre-fetch cache enabled.
 *
 * Input              :  loops = delay time, unit 25ns  (TCY = 12.5ns)
 * Example            :  Delay_2xTCY(4) gives a 100ns delay
 * Return             :  --
 *
------------------------------------------------------------------------------------*/
PRIVATE  void  Delay_2xTCY(unsigned int loops)
{
    do { loops--; } until (loops == 0);
}


/*----------------------------------------------------------------------------------
 * Name               :  LCD_WriteCommand()
 * Function           :  Write 8-bit command to LCD controller
 *                       Delay 100us (minimum) before exit.
 * Entry arg(s)       :  cmd
 * Return             :  --
------------------------------------------------------------------------------------*/
PRIVATE  void  LCD_WriteCommand(uint8 cmd)
{
    LCD_DATA_DIRN_OUT();
    LCD_RS_LOW();
    LCD_RW_LOW();
    NOP();
    LCD_E_HIGH();
    LCD_DATA_OUTPUT(cmd);
    Delay_2xTCY(20);         // E pulse width >= 320ns
    LCD_E_LOW();
    NOP();
    NOP();
    LCD_RW_HIGH();
    Delay10us(8);           // Delay after command write (80us min.)
}


/*----------------------------------------------------------------------------------
 * Name               :  LCD_WriteData()
 * Function           :  Write 8-bit data to LCD controller
 * Entry arg(s)       :  data
 * Return             :  --
------------------------------------------------------------------------------------*/
PRIVATE  void  LCD_WriteData(uint8 data)
{
    LCD_DATA_DIRN_OUT();
    LCD_RS_HIGH();
    LCD_RW_LOW();
    NOP();
    LCD_E_HIGH();
    LCD_DATA_OUTPUT(data);
    Delay_2xTCY(20);           // E pulse width >= 320ns
    LCD_E_LOW();
    NOP();
    NOP();
    LCD_RW_HIGH();
    Delay10us(8);             // Delay after write data (80us min.)
}


/*----------------------------------------------------------------------------------
 * Name               :  LCD_ReadData()
 * Function           :  Read 8-bit data from LCD controller
 * Entry arg(s)       :  --
 * Return             :  data byte
------------------------------------------------------------------------------------*/
PRIVATE  uint8  LCD_ReadData()
{
    uint8  data;

    LCD_DATA_DIRN_IN();
    LCD_RS_HIGH();
    LCD_RW_HIGH();
    NOP();
    LCD_E_HIGH();
    Delay_2xTCY(20);           // E pulse width >= 320ns
    data = LCD_DATA_INPUT();
    LCD_E_LOW();
    Delay10us(8);             // Delay after read data (80us min.)

    return data;
}


/*----------------------------------------------------------------------------------
 * Name               :  LCD_ReadStatus()
 * Function           :  Read 8-bit status reg from LCD controller
 * Entry arg(s)       :  --
 * Return             :  status byte
------------------------------------------------------------------------------------*/
PRIVATE  uint8  LCD_ReadStatus()
{
    uint8  status;

    LCD_DATA_DIRN_IN();
    LCD_RS_LOW();
    LCD_RW_HIGH();
    NOP();
    LCD_E_HIGH();
    Delay_2xTCY(20);           // E pulse width >= 320ns
    status = LCD_DATA_INPUT();
    LCD_E_LOW();
    Delay_2xTCY(80);           // E cycle time >= 1800ns

    return status;
}


/*----------------------------------------------------------------------------------
 * Name               :  LCD_WaitNotBusy()
 * Function           :  Wait until LCD controller is not busy.
 * Entry arg(s)       :  --
 * Return             :  --
------------------------------------------------------------------------------------*/
PRIVATE  void  LCD_WaitNotBusy()
{
    while (LCD_ReadStatus() & 0x80)  {/* WAIT */}
}


/*----------------------------------------------------------------------------------
 * Name               :  LCD_Init()
 * Function           :  Initialize ST7920 LCD controller in graphics mode.
 * Input              :  none.
 * Return             :  False (0) if ST7920 not detected, else True (1)
------------------------------------------------------------------------------------*/
bool  LCD_Init(void)
{
    bool    result = 1;
    uint8   dummy;

    LCD_CTRL_DIRN_OUT();
    NOP();
    LCD_E_LOW();
    LCD_RS_HIGH();
    LCD_RW_HIGH();

    LCD_RST_LOW();             // LCD controller hard reset
    DelayMs(10);
    LCD_RST_HIGH();
    DelayMs(50);

    LCD_WriteCommand(0x30);    // Function Set: basic
    LCD_WriteCommand(0x30);    // Function Set: basic commands
    LCD_WriteCommand(0x0C);    // LCD on/off control: display ON, no blink
    LCD_WriteCommand(0x30);    // Function Set: basic
    LCD_WriteCommand(0x34);    // Function Set: extended commands
    LCD_WriteCommand(0x36);    // Function Set: select graphic mode
    DelayMs(10);
    
    // Write test data to GDRAM to detect if LCD module is connected...
    LCD_WriteCommand(0x80);    // set vert addr = 0
    LCD_WriteCommand(0x80);    // set horiz addr = 0
    LCD_WriteData(0x69);       // write GDRAM data MSB
    LCD_WriteData(0xC3);       // write GDRAM data LSB
    
    LCD_WriteCommand(0x80);    // set vert addr = 0
    LCD_WriteCommand(0x80);    // set horiz addr = 0
    dummy = LCD_ReadData();
    if (LCD_ReadData() != 0x69) result = 0;  // read back MSB
    if (LCD_ReadData() != 0xC3) result = 0;  // read back LSB

    return result;
}


/*----------------------------------------------------------------------------------
 * Name               :  LCD_ClearGDRAM()
 *
 * Function           :  Clear entire LCD graphics data RAM.
 *              
 * GDRAM format in ST7920 controller:
 * ```````````````````````````````````
 *       UPPER HALF SCREEN           LOWER HALF SCREEN
 *     ----------------------      ----------------------
 *     Vert Addr   Horiz Addr      Vert Addr   Horiz Addr
 *        0         0 .. 7            0         8 .. 15
 *        1         0 .. 7            1         8 .. 15
 *        2         0 .. 7            2         8 .. 15
 *        3         0 .. 7            3         8 .. 15
 *        .            .              .            .
 *        .            .              .            .
 *       31         0 .. 7           31         8 .. 15
 * 
------------------------------------------------------------------------------------*/
void   LCD_ClearGDRAM(void)
{
    uint8    x, y;

    for (y = 0;  y < 64;  y++)
    {
        if (y < 32)
        {
            LCD_WriteCommand(0x80 | y);
            LCD_WriteCommand(0x80);
        }
        else
        {
            LCD_WriteCommand(0x80 | (y - 32));
            LCD_WriteCommand(0x80 + 8);
        }

        for (x = 0;  x < 8;  x++)
        {
            LCD_WriteData(0);
            LCD_WriteData(0);
        }
    }
}


/*----------------------------------------------------------------------------------
 * Function   :  LCD_WriteBlock()
 *
 * Overview   :  Copies a rectangular block of pixels from an external screen
 *               buffer in MCU RAM to the LCD controller GDRAM (graphics data RAM).
 *               In general, the GDRAM pixel format differs from the MCU RAM screen
 *               buffer, so a transformation of some sort needs to be performed.
 *
 * Input      :  scnBuf = address of screen buffer in MCU RAM
 *               x, y = pixel coords of upper LHS of block to be copied
 *               w, h = width and height (pixels) of block to be copied
 *  
 * Return     :  --
------------------------------------------------------------------------------------*/
void   LCD_WriteBlock(uint16 *scnBuf, uint16 x, uint16 y, uint16 w, uint16 h)
{
    uint8   vertAddr, horizAddr;    // row and column address to be sent to ST7920
    int     row, col;               // row and column word index for screen buffer
    int     firstCol;               // first (leftmost) column-word in a row to be modified
    int     lastCol;                // last (rightmost) column-word in a row to be modified
    uint16  pixelWord;              // row of 16 pixels in column-word to be modified
    uint16  *pBuf;

    if (x > 127)  x = 0;            // prevent writing past end-of-row
    if ((x + w) > 128) w = 128 - x;

    firstCol = x / 16;   
    lastCol = (x + w - 1) / 16;

    for (row = y;  row < (y + h) && row < 64;  row++)
    {
        for (col = firstCol;  col <= lastCol && col < 8;  col++)
        {
            pBuf = scnBuf + (row * 8 + col);
            pixelWord = *pBuf;

            if (row < 32)  { vertAddr = row;  horizAddr = col; }  // Top half of screen
            else  { vertAddr = row - 32;  horizAddr = col + 8; }  // Bottom half

            LCD_WriteCommand(0x80 | vertAddr);
            LCD_WriteCommand(0x80 | horizAddr);
            LCD_WriteData(HI_BYTE(pixelWord));
            LCD_WriteData(LO_BYTE(pixelWord));
        }
    }
}


/*----------------------------------------------------------------------------------
 * Name               :  LCD_Test()
 * Function           :  Low-level test of ST7920 graphics write sequence.
 *                       The screen should show a pattern of diagonal lines,
 *                       slope 45 degrees, thickness 2 pixels, spaced 16 pixels apart.
 * Input              :  --
------------------------------------------------------------------------------------*/
void  LCD_Test()
{
    uint8   vertAddr, horizAddr;    // row and column address to be sent to ST7920
    int     row, col;               // row and column word index for screen buffer
    uint16  pixelWord;

    for (row = 0;  row < 64;  row++)
    {
        if ((row % 16) == 0) pixelWord = 0xC000;

        for (col = 0;  col < 8;  col++)
        {
            if (row < 32)  { vertAddr = row;  horizAddr = col; }  // Top half of screen
            else  { vertAddr = row - 32;  horizAddr = col + 8; }  // Bottom half

            LCD_WriteCommand(0x80 | vertAddr);
            LCD_WriteCommand(0x80 | horizAddr);
            LCD_WriteData(HI_BYTE(pixelWord));
            LCD_WriteData(LO_BYTE(pixelWord));
        }
        
        pixelWord = pixelWord >> 1;
        if (pixelWord == 0x0001) pixelWord = 0x8001;
    }
}

