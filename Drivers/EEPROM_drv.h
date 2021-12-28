/**
 * File:  EEPROM_drv.h
 *
 * Declarations for serial (I2C) EEPROM device, 24LC04B, 24LC08B, etc.
 *
 * The symbol EEPROM_I2C_PORT_NUMBER should be defined in "hardwareProfile.h"
 */
#ifndef EEPROM_DEF_H
#define EEPROM_DEF_H

#include "../Common/system_def.h"

#ifndef EEPROM_I2C_PORT_NUMBER       // May be defined in Project Properties...
#define EEPROM_I2C_PORT_NUMBER  1    // if not, use default IIC channel = 1
#endif


// These macros should be defined if the signal WP is controlled by the MCU;
// otherwise it is assumed that WP is tied low and the macros do nothing.
#ifndef EEPROM_WRITE_ENABLE
#define EEPROM_WRITE_ENABLE()    {}   // Set WP Low 
#define EEPROM_WRITE_INHIBIT()   {}   // Set WP High
#endif

int   EepromWriteData( uint8 *pData, uint8 promBlock, uint8 promAddr, int nbytes );
int   EepromReadData( uint8 *pData, uint8 promBlock, uint8 promAddr, int nbytes );
uint8 EepromIsBusy( void );


#endif  // EEPROM_DEF_H
