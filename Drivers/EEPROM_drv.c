/**
 * File:  EEPROM_drv.c
 *
 * I/O driver functions for serial (I2C) EEPROM device, 24LC04B, 24LC08B, etc.
 * This module should not contain application-specific code.
 *
 * The symbol EEPROM_I2C_PORT_NUMBER should be defined in EEPROM_drv.h
 * Only one I2C EEPROM device is supported, on either the I2C1 bus or I2C2 bus.
 *
 * Device characteristics:
 * 24LC04B:  4k bits organised as 2 blocks of 256 bytes = 32 pages (16 bytes per page).
 * 24LC08B:  8k bits organised as 4 blocks of 256 bytes = 64 pages (16 bytes per page).
 */
#include "EEPROM_drv.h"
#include "I2C_drv.h"

#if (EEPROM_I2C_PORT_NUMBER == 1)
/**
 * Function to write one or more bytes of data (up to 16 bytes) sequentially to the EEPROM
 * and initiate a programming cycle. All data must be within the same EEPROM page.
 * The function waits for the programming cycle to complete (which may be up to 5ms).
 * A generous timeout of 100ms is allowed before aborting the write operation.
 *
 * Entry arg's: pData = pointer to source data (byte array)
 *              promBlock = EEPROM block select (0, 1, 2, 3, ...)
 *              promAddr = EEPROM beginning address for write (0..255)
 *              nbytes = number of bytes to write (max = 16, not checked!)
 *
 * Returns:  ERROR (-1) if I2C bus error detected, else 0.
 */
int EepromWriteData( uint8 *pData, uint8 promBlock, uint8 promAddr, int nbytes )
{
    int errcode = 0;
    uint16 npolls = 1000;  // 1 poll takes approx. 100usec @ SCK = 100kHz

    EEPROM_WRITE_ENABLE();                  // Set WP Low
    promBlock = (promBlock & 7) << 1;       // Block # is b1:3 of control byte

    if (I2C1MasterStart(0xA0 | promBlock))  // Control byte ACK'd -- Send write command
    {
        I2C1MasterSend(promAddr);
        while (nbytes != 0)
        {
            I2C1MasterSend(*pData++); 
            --nbytes;
        }
        Stop_I2C1();  // Terminate cmd string and initiate write cycle
        //
        while (npolls--)   // ACK polling -- allow 100ms timeout (approx.)
        {
            if (I2C1MasterStart(0xA0 | promBlock)) break;  // exit when ACK rec'd
        }
		
        if (npolls == 0) errcode = -1;
        else  // Follow ACK'd Start with "dummy" command (send address only -- no data)
        {
            I2C1MasterSend(promAddr);  
            Stop_I2C1();
        }
    }
    else errcode = -1;  // I2C bus error or device not responding

    EEPROM_WRITE_INHIBIT();  // Set WP High (or float)
    return errcode;
}


/**
 * Function to read one or more bytes of data (up to 256 bytes) sequentially from the EEPROM.
 * Does not check if there is a programming cycle in progress.
 *
 * Entry arg's: pData = pointer to destination (byte array)
 *              promBlock = EEPROM block select (0, 1, 2, 3, ...)
 *              promAddr = EEPROM beginning address for read (0..255)
 *              nbytes = number of bytes to read (max. 256, not checked)
 *
 * Returns:  Count of bytes received from EEPROM, or (-1) if I2C bus error detected
 */
int EepromReadData( uint8 *pData, uint8 promBlock, uint8 promAddr, int nbytes )
{
    int bcount = -1;

    promBlock = (promBlock & 7) << 1;  
    
    if (I2C1MasterStart(0xA0 | promBlock))   // Control byte ACK'd --
    {
        if (I2C1MasterSend(promAddr))   // Send eeprom address, wait for ACK
        {
            Delay10us(1);
            // Start again with Read control byte
            if (I2C1MasterStart(0xA0 | promBlock | 1))
            {
                bcount = gets_I2C1(pData, nbytes);  // read data; no ACK after last byte
                NotAck_I2C1();
                Stop_I2C1();
            }
        }
    }
        
    return bcount;
}

/**
 * Function checks for EEPROM programming cycle in progress.
 *
 * Returns: TRUE if a programming cycle in progress (EEPROM busy), else FALSE;
 */
uint8 EepromIsBusy( void )
{
    uint8 status = 1;  // Busy

    if (I2C1MasterStart(0xA0))  // Control byte ACK'd -- 
    {
        // Follow ACK'd Start with "dummy" command (send address only -- no data)
        I2C1MasterSend(0);
        Stop_I2C1();
        status = 0;  // Not busy
    }

    return status;
}

#elif (EEPROM_I2C_PORT_NUMBER == 2)
/**
 * Function to write one or more bytes of data (up to 16 bytes) sequentially to the EEPROM
 * and initiate a programming cycle. All data must be within the same EEPROM page.
 * The function waits for the programming cycle to complete (which may be up to 5ms).
 * A generous timeout of 100ms is allowed before aborting the write operation.
 *
 * Entry arg's: pData = pointer to source data (byte array)
 *              promBlock = EEPROM block select (0, 1, 2, 3, ...)
 *              promAddr = EEPROM beginning address for write (0..255)
 *              nbytes = number of bytes to write (max = 16, not checked!)
 *
 * Returns:  ERROR (-1) if I2C bus error detected, else 0.
 */
int EepromWriteData( uint8 *pData, uint8 promBlock, uint8 promAddr, int nbytes )
{
    int errcode = 0;
    uint16 npolls = 1000;  // 1 poll takes approx. 100usec @ SCK = 100kHz

    EEPROM_WRITE_ENABLE();                  // Set WP Low
    promBlock = (promBlock & 7) << 1;       // Block # is b1:3 of control byte

    if (I2C2MasterStart(0xA0 | promBlock))  // Control byte ACK'd -- Send write command
    {
        I2C2MasterSend(promAddr);
        while (nbytes != 0)
        {
            I2C2MasterSend(*pData++); 
            --nbytes;
        }
        Stop_I2C2();  // Terminate cmd string and initiate write cycle
        //
        while (npolls--)   // ACK polling -- allow 100ms timeout (approx.)
        {
            if (I2C2MasterStart(0xA0 | promBlock)) break;  // exit when ACK rec'd
        }
		
        if (npolls == 0) errcode = -1;
        else  // Follow ACK'd Start with "dummy" command (send address only -- no data)
        {
            I2C2MasterSend(promAddr);  
            Stop_I2C2();
        }
    }
    else errcode = -1;  // I2C bus error or device not responding

    EEPROM_WRITE_INHIBIT();  // Set WP High (or float)
    return errcode;
}


/**
 * Function to read one or more bytes of data (up to 256 bytes) sequentially from the EEPROM.
 * Does not check if there is a programming cycle in progress.
 *
 * Entry arg's: pData = pointer to destination (byte array)
 *              promBlock = EEPROM block select (0, 1, 2, 3, ...)
 *              promAddr = EEPROM beginning address for read (0..255)
 *              nbytes = number of bytes to read (max. 256, not checked)
 *
 * Returns:  Count of bytes received from EEPROM, or (-1) if I2C bus error detected
 */
int EepromReadData( uint8 *pData, uint8 promBlock, uint8 promAddr, int nbytes )
{
    int bcount = -1;

    promBlock = (promBlock & 7) << 1; 
    
    if (I2C2MasterStart(0xA0 | promBlock))   // Control byte ACK'd --
    {
        if (I2C2MasterSend(promAddr))   // Send eeprom address, wait for ACK
        {
            Delay10us(1);
            // Start again with Read control byte
            if (I2C2MasterStart(0xA0 | promBlock | 1))
            {
                bcount = gets_I2C2(pData, nbytes);  // read data; no ACK after last byte
                NotAck_I2C2();
                Stop_I2C2();
            }
        }
    }
        
    return bcount;
}

/**
 * Function checks for EEPROM programming cycle in progress.
 *
 * Returns: TRUE if a programming cycle in progress (EEPROM busy), else FALSE;
 */
uint8 EepromIsBusy( void )
{
    uint8 status = 1;  // Busy

    if (I2C2MasterStart(0xA0))  // Control byte ACK'd -- 
    {
        // Follow ACK'd Start with "dummy" command (send address only -- no data)
        I2C2MasterSend(0);
        Stop_I2C2();
        status = 0;  // Not busy
    }

    return status;
}

#else
#error "! EEPROM_I2C_PORT_NUMBER undefined, or not supported."
#endif
