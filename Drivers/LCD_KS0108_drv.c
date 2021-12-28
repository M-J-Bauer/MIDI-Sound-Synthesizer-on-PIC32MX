/*
 * File:    LCD_KS0108_drv.c
 *
 * Low-level driver functions for monochrome Graphic LCD module, 128 x 64 pixels.
 *
 * Customized for LCD controller chip-set:  KS0107, KS0108.
 */
#include "LCD_KS0108_drv.h"


/*----------------------------------------------------------------------------------
 * Name               :  LCD_WaitNotBusy()
 * Function           :  Wait until LCD controller is not busy, or 500us timeout.
 * Pre-condition      :  CS1 or CS2 asserted.
 * Entry arg(s)       :  --
 * Return             :  --
 * Note               :  Returns unconditionally after 500us timeout (approx.) 
------------------------------------------------------------------------------------*/
PRIVATE  void  LCD_WaitNotBusy()
{
    uint32  retries = 0;
    
    while (retries++ < 1000)
    {
        if ( !(LCD_ReadStatus() & LCD_STATUS_BUSY) )  // not busy
            break;
        Delay_Nx25ns(20);  // delay 0.5us
    }
}


/*----------------------------------------------------------------------------------
 * Name               :  LCD_WriteCommand()
 * Function           :  Write 8-bit command to LCD controller
 * Pre-condition      :  CS1 or CS2 asserted.
 * Entry arg(s)       :  cmd byte
 * Return             :  --
------------------------------------------------------------------------------------*/
void  LCD_WriteCommand(uint8 cmd)
{
    LCD_WaitNotBusy();      
    LCD_DATA_DIRN_OUT();
    LCD_E_LOW();
    LCD_RS_LOW();             // select Instr'n reg
    LCD_RW_LOW();
    LCD_DATA_OUTPUT(cmd);
    Delay_Nx25ns(20);         // E pulse low >= 450ns
    LCD_E_HIGH();
    Delay_Nx25ns(20);         // E pulse high >= 450ns
    LCD_E_LOW();
    Delay_Nx25ns(1);          // Tah >= 10ns, Tdhw >= 10ns
    LCD_RW_HIGH();
}


/*----------------------------------------------------------------------------------
 * Name               :  LCD_WriteData()
 * Function           :  Write 8-bit data to LCD controller
 * Pre-condition      :  CS1 or CS2 asserted.
 * Entry arg(s)       :  data byte
 * Return             :  --
------------------------------------------------------------------------------------*/
void  LCD_WriteData(uint8 data)
{
    LCD_WaitNotBusy();
    LCD_DATA_DIRN_OUT();
    LCD_E_LOW();
    LCD_RS_HIGH();            // select Data reg
    LCD_RW_LOW();
    LCD_DATA_OUTPUT(data);
    Delay_Nx25ns(20);         // E pulse low >= 450ns
    LCD_E_HIGH();
    Delay_Nx25ns(20);         // E pulse high >= 450ns
    LCD_E_LOW();
    Delay_Nx25ns(1);          // Tah >= 10ns, Tdhw >= 10ns
    LCD_RW_HIGH();
}


/*----------------------------------------------------------------------------------
 * Name               :  LCD_ReadStatus()
 * Function           :  Read 8-bit status reg from LCD controller
 * Pre-condition      :  CS1 or CS2 asserted.
 * Entry arg(s)       :  --
 * Return             :  status byte
------------------------------------------------------------------------------------*/
uint8  LCD_ReadStatus()
{
    uint8  status;

    LCD_DATA_DIRN_IN();
    LCD_RS_LOW();             // select Status reg 
    LCD_RW_HIGH();
    LCD_E_LOW();
    Delay_Nx25ns(20);         // E pulse low >= 450ns
    LCD_E_HIGH();
    Delay_Nx25ns(20);         // E pulse high >= 450ns
    status = LCD_DATA_INPUT();
    Delay_Nx25ns(1);          // Tah >= 10ns
    LCD_E_LOW();

    return status;
}


/*----------------------------------------------------------------------------------
 * Name               :  LCD_Reset()
 * Function           :  Hard reset LCD controller.
 * Input              :  --
 * Return             :  False (0) if KS0108 not detected, else True (1)
 * Note               :  LCD GDRAM not cleared
------------------------------------------------------------------------------------*/
void  LCD_Reset(void)
{
    LCD_CTRL_DIRN_OUT();
    Delay_Nx25ns(1);
    LCD_DATA_DIRN_IN();
    Delay_Nx25ns(1);

    LCD_CS1_OFF();     // de-select 2 KS0108 chips
    LCD_CS2_OFF();
    LCD_E_LOW();       // set control outputs in quiescent state
    LCD_RS_HIGH();
    LCD_RW_HIGH();

    LCD_RST_LOW();     // strobe RST# pin low
    DelayMs(2);
    LCD_RST_HIGH();
    DelayMs(50);
}


/*----------------------------------------------------------------------------------
 * Name               :  LCD_Init()
 * Function           :  Initialize 2 x KS0108 controller chips.
 * Input              :  --
 * Return             :  False (0) if KS0108 not detected, else True (1)
 * Note               :  LCD GDRAM not cleared
------------------------------------------------------------------------------------*/
bool  LCD_Init(void)
{
    bool    result = 1;

    LCD_Reset();

    LCD_CS1_ON();
    if ( !(LCD_ReadStatus() & LCD_STATUS_OFF) ) result = 0;
    LCD_WriteCommand(LCD_SET_DISPLAY_ON);    // Turn on graphics mode
    LCD_WriteCommand(LCD_SET_START_LINE);    // Set "Start Line" = 0
    if ( LCD_ReadStatus() & LCD_STATUS_OFF ) result = 0;
    LCD_CS1_OFF();
    
    LCD_CS2_ON();
    if ( !(LCD_ReadStatus() & LCD_STATUS_OFF) ) result = 0;
    LCD_WriteCommand(LCD_SET_DISPLAY_ON);    // Turn on graphics mode
    LCD_WriteCommand(LCD_SET_START_LINE);    // Set "Start Line" = 0
    if ( LCD_ReadStatus() & LCD_STATUS_OFF ) result = 0;
    LCD_CS2_OFF();

    return result;
}


/*----------------------------------------------------------------------------------
 * Name               :  LCD_ClearGDRAM()
 *
 * Function           :  Clear entire LCD graphics data RAM.
 *
 * GDRAM format in KS0108 controller:
 * ```````````````````````````````````
 *              Left Half Screen  (CS1)    Right Half Screen  (CS2)
 *              ```````````````````````    ````````````````````````
 *               0     Y-ADDRESS^    63    0     Y-ADDRESS^    63
 *      Page 0   ||||||||||||||||||||||    ||||||||||||||||||||||
 *      Page 1   ||||||||||||||||||||||    ||||||||||||||||||||||
 *      Page 2   ||||||||||||||||||||||    ||||||||||||||||||||||
 *      Page 3   ||||||||||||||||||||||    ||||||||||||||||||||||
 *      Page 4   ||||||||||||||||||||||    ||||||||||||||||||||||
 *      Page 5   ||||||||||||||||||||||    ||||||||||||||||||||||
 *      Page 6   ||||||||||||||||||||||    ||||||||||||||||||||||
 *      Page 7   ||||||||||||||||||||||    ||||||||||||||||||||||
 *
 *      Legend:  | = data byte, 8 pixels vertical, bit 0 is at the top.
 *
 *  ^NB: In the Samsung KS0108B datasheet, "Y-ADDRESS" = horizontal address!
 *
------------------------------------------------------------------------------------*/
void   LCD_ClearGDRAM(void)
{
    int   y_addr, page;

    for (page = 0;  page < 8;  page++)
    {
        LCD_CS1_ON();   // Select Left Half Screen (LHS)
        LCD_WriteCommand(LCD_SET_PAGE_ADDR | page);  // vert addr
        LCD_WriteCommand(LCD_SET_Y_ADDR | 0);   // horiz addr
        for (y_addr = 0;  y_addr < 64;  y_addr++)
        {
            LCD_WriteData(0);  // Y-ADDRESS auto increments
        }
        LCD_CS1_OFF();

        LCD_CS2_ON();   // Select Right Half Screen (RHS)
        LCD_WriteCommand(LCD_SET_PAGE_ADDR | page);  // vert addr
        LCD_WriteCommand(LCD_SET_Y_ADDR | 0);   // horiz addr
        for (y_addr = 0;  y_addr < 64;  y_addr++)
        {
            LCD_WriteData(0);  // Y-ADDRESS auto increments
        }
        LCD_CS2_OFF();
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
 *
 * Notes:     :  The (external) screen buffer is formatted as 8 "columns" horizonally
 *               by 64 "rows" vertically. Each column has 16 pixels. The leftmost
 *               pixel is stored in the MS bit (bit15) of the column.
------------------------------------------------------------------------------------*/
void   LCD_WriteBlock(uint16 *scnBuf, uint16 x, uint16 y, uint16 w, uint16 h)
{
    uint8   page;                   // page address in KS0108 (1 page = 8 rows)
    uint8   x_coord;                // pixel horizontal coordinate
    int     row, col;               // row and column word index for screen buffer
    int     firstPage;              // first (top) page to be modified
    int     lastPage;               // last (bottom) page to be modified
    int     firstCol;               // first (leftmost) column in a row to be modified
    int     lastCol;                // last (rightmost) column in a row to be modified
    int     offset;                 // address offset of pixel word in scnBuf
    uint8   pixelByte;              // 8 pixels aligned vertically (LS bit at top)
    uint8   bitmask;
    uint16  pixelWord[8];           // 8 rows x 16 pixels in a column to be modified
    uint16  *pBuf;

    if (y > 63)  y = 0;             // prevent writing past end-of-screen
    if ((y + h) > 63)  h = 63 - y;

    if (x > 127)  x = 0;            // prevent writing past end-of-row
    if ((x + w) > 128)  w = 128 - x;

    firstPage = y / 8;
    lastPage = (y + h - 1) / 8;

    firstCol = x / 16;
    lastCol = (x + w - 1) / 16;

    for (page = firstPage;  page <= lastPage;  page++)
    {
        for (col = firstCol;  col <= lastCol && col < 8;  col++)
        {
            // Grab a bunch of 16 (H) x 8 (V) pixels from buffer...
            offset = page * 64 + col;  // 64 words per buffer "page"
            pBuf = scnBuf + offset;
            for (row = 0;  row < 8;  row++)  // 8 rows in LCD page
            {
                pixelWord[row] = *pBuf;
                pBuf += 8;  // next row (advance 8 col's)
            }

            if (col < 4)  // select LHS controller chip
            {
                LCD_CS1_ON();
                LCD_WriteCommand(LCD_SET_PAGE_ADDR | page);   // vert (page) addr
                LCD_WriteCommand(LCD_SET_Y_ADDR | col * 16);  // horiz (Y) addr
            }
            else  // select RHS controller chip
            {
                LCD_CS2_ON();
                LCD_WriteCommand(LCD_SET_PAGE_ADDR | page);   // vert (page) addr
                LCD_WriteCommand(LCD_SET_Y_ADDR | (col - 4) * 16);  // horiz (Y) addr
            }
            // Write a bunch of pixels 16 (H) x 8 (V) to selected KS0108 chip
            for (x_coord = 0;  x_coord < 16;  x_coord++)
            {
                // transpose horiz pixels in pixelWord[8] to vertical pixelByte
                pixelByte = 0;
                bitmask = 0x01;
                for (row = 0;  row < 8;  row++)
                {
                    if (pixelWord[row] & 0x8000) pixelByte |= bitmask;
                    bitmask = bitmask << 1;   // next pixel down
                    pixelWord[row] = pixelWord[row] << 1;  // next pixel right
                }
                // write 8 pixels vertically to KS0108 chip
                LCD_WriteData(pixelByte);  // Y-ADDRESS auto increments
            }
            LCD_CS1_OFF();  // de-select both KS0108 chips
            LCD_CS2_OFF();
        }
    }
}


/*----------------------------------------------------------------------------------
 * Name               :  LCD_Test()
 * Function           :  Low-level test of KS0108 graphics write sequence.
 *                       The screen should show a pattern of diagonal lines,
 *                       slope 45 degrees, thickness 1 pixel, spaced 8 pixels apart.
 * Input              :  --
------------------------------------------------------------------------------------*/
void  LCD_Test()
{
    int     x_coord, page;
    uint8   pixels;   // 8 pixels, vertical orientation
    
    LCD_ClearGDRAM();

    for (page = 0;  page < 8;  page++)
    {
        LCD_CS1_ON();   // Select Left Half Screen (LHS)
        LCD_WriteCommand(LCD_SET_PAGE_ADDR | page);  // vert addr
        LCD_WriteCommand(LCD_SET_Y_ADDR | 0);   // horiz addr
        for (x_coord = 0;  x_coord < 64;  x_coord++)
        {
            if ((x_coord % 8) == 0) pixels = 0x01;
            LCD_WriteData(pixels);
            pixels = pixels << 1;  // move 1 pixel down
        }
        LCD_CS1_OFF();

        LCD_CS2_ON();   // Select Right Half Screen (RHS)
        LCD_WriteCommand(LCD_SET_PAGE_ADDR | page);  // vert addr
        LCD_WriteCommand(LCD_SET_Y_ADDR | 0);   // horiz addr
        for (x_coord = 0;  x_coord < 64;  x_coord++)
        {
            if ((x_coord % 8) == 0) pixels = 0x01;
            LCD_WriteData(pixels);
            pixels = pixels << 1;  // move 1 pixel down
        }
        LCD_CS2_OFF();
    }
}


/*
 * Function:     Toggle LCD backlight switch (dim/bright).
 */
void  LCD_BacklightToggle()
{
    static  short  LCDbacklightDim = 0;  // 0: Bright, 1: Dim

    if (LCDbacklightDim)
    {
        LCD_BACKLIGHT_HIGH();
        LCDbacklightDim = 0;
    }
    else
    {
        LCD_BACKLIGHT_LOW();
        LCDbacklightDim = 1;
    }
}

