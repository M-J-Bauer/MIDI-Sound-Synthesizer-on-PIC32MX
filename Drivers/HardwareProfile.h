/***************************************************************************************\
 * File:            HardwareProfile.h   (customized for PIC32-PINGUINO-MICRO)
 *
 * Purpose:         Hardware Platform definitions used in peripheral driver modules.
 *
 * Target H/W:      Olimex PIC32-PINGUINO-MICRO module piggy-backed on REMI synth board
 *
 * Processor:       PIC32MX440F256H
 *
 * Compiler:        Microchip XC32 under MPLAB.X
 *
\***************************************************************************************/

#ifndef __HARDWARE_PROFILE_H
#define __HARDWARE_PROFILE_H

#include "../Common/Compiler.h"
#include "../Common/GenericTypeDefs.h"
#include "../Common/system_def.h"

// Device ID numbers (from "PIC32MX Flash Programming Spec" DS-61145)
#define DEVICE_ID_PIC32MX340F512H  0x00916053
#define DEVICE_ID_PIC32MX440F256H  0x00952053  
#define DEVICE_ID_PIC32MX460F512L  0x00978053

#define DEVICE_ID_MASK             0x0FFFFFFF
#define FLASH_APP_ENTRY_POINT      0x9D000000
#define GET_DEVICE_ID()            (DEVID)     // Register in MCU flash PM

#define MCU_ID_CHECK()  ((GET_DEVICE_ID() & DEVICE_ID_MASK) == DEVICE_ID_PIC32MX440F256H)

//========================  Determine Target Platform ===================================
//
// Check that the correct MCU type is selected in MPLAB'X Project Properties
#ifndef __32MX440F256H__
#error "Processor type PIC32MX440F256H must be selected in this project!"
#endif

//========================  Determine RTC Requirements ==================================
//
// Some of MJB's PIC32 projects use an RTCC device. Select from the following:
//#define RTCC_TYPE_MCP79410
//#define RTCC_TYPE_ISL1208

// Kernel RTC software support is needed if a hardware RTCC device is used
#if defined RTCC_TYPE_ISL1208 || defined RTCC_TYPE_MCP79410
#define INCLUDE_KERNEL_RTC_SUPPORT
#endif

#if defined RTCC_TYPE_ISL1208 && defined RTCC_TYPE_MCP79410
#error "System can define only one RTCC device type!"
#endif

//========================  Select LCD Controller Type ==================================
//
#define USE_LCD_CONTROLLER_KS0108    // Using 128x64 mono graphics LCD with KS0108 chip
//#define USE_LCD_CONTROLLER_ST7920
//#define USE_OLED_CONTROLLER_SH1106

#define LCD_CHIPSELECT_ACTIVE_LOW    // Comment out if CS is active high

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
#define LCD_RST_HIGH()         (LATFbits.LATF1 = 1)
#define LCD_RST_LOW()          (LATFbits.LATF1 = 0)
#define LCD_BACKLIGHT_HIGH()   (LATDbits.LATD4 = 1)
#define LCD_BACKLIGHT_LOW()    (LATDbits.LATD4 = 0)

#ifdef LCD_CHIPSELECT_ACTIVE_LOW  
#define LCD_CS1_OFF()          (LATDbits.LATD8 = 1)
#define LCD_CS1_ON()           (LATDbits.LATD8 = 0)
#define LCD_CS2_OFF()          (LATDbits.LATD11 = 1)
#define LCD_CS2_ON()           (LATDbits.LATD11 = 0)
#else  // LCD_CHIPSELECT_ACTIVE_HIGH
#define LCD_CS1_OFF()          (LATDbits.LATD8 = 0)
#define LCD_CS1_ON()           (LATDbits.LATD8 = 1)
#define LCD_CS2_OFF()          (LATDbits.LATD11 = 0)
#define LCD_CS2_ON()           (LATDbits.LATD11 = 1)
#endif

//========================== Select other custom hardware ===============================
//
// Assign PIC32 I2C peripheral channel(s) that the hardware uses.
#define USE_I2C_CHANNEL_1           // I2C1 on UEXT connector
#define EEPROM_I2C_PORT_NUMBER  1   // I2C1  

//================  UART DRIVER BUILD OPTIONS  =======================
//
#define UART1_RX_INTERRUPT_DRIVEN  1   // if 0, use polled RX input
#define UART2_RX_INTERRUPT_DRIVEN  1   // if 0, use polled RX input

#define UART1_TX_USING_QUEUE  1    // if 0, use direct TX functions
#define UART2_TX_USING_QUEUE  0    // if 0, use direct TX functions

#define UART1_TXBUFSIZE      128   // TX FIFO buffer size, chars
#define UART2_TXBUFSIZE      128

#define UART1_RXBUFSIZE      256   // RX FIFO buffer size, chars
#define UART2_RXBUFSIZE      256
//
//====================================================================

// Debug LED pin not allocated (RD1 reassigned)
// (Should be defined in pic32_low_level module in any case.)
#define DEBUG_LED_INIT()    {}
#define DEBUG_LED_ON()      {}
#define DEBUG_LED_OFF()     {}
#define DEBUG_LED_TOGGLE()  {}

#endif // __HARDWARE_PROFILE_H
