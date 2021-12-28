/***************************************************************************************\
 * File:            HardwareProfile.h   (customized for 'REMI SYNTH mk2' project)
 *
 * Purpose:         Hardware Platform definitions specific to the application
 *
 * Target H/W:      Olimex PIC32-MX340 Prototyping Board
 *                  Custom PIC32-MX340 Board (e.g. REMI Sound Synth Module)
 *
 * Processor:       PIC32MX340F512H
 *
 * Compiler:        Microchip XC32 under MPLAB.X (v2.20++)
 *
\***************************************************************************************/

#ifndef __HARDWARE_PROFILE_H
#define __HARDWARE_PROFILE_H

#include "../Common/Compiler.h"
#include "../Common/GenericTypeDefs.h"
#include "../Common/system_def.h"

#define DEVICE_ID_PIC32MX460F512L  0x00978053
#define DEVICE_ID_PIC32MX340F512H  0x00916053
#define DEVICE_ID_MASK             0x0FFFFFFF
#define FLASH_APP_ENTRY_POINT      0x9D000000
#define GET_DEVICE_ID()            (DEVID)     // Register in MCU flash PM


//========================  Determine Target Platform ===================================
//
#define USE_OLIMEX_PIC32MX340_BOARD  // REMI mk2 prototype platform 

// Check that a valid processor type is selected in MPLAB'X Project Properties
#ifndef __32MX340F512H__
#error "Processor type PIC32MX340F512H must be selected in project!"
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

//========================== Select other custom hardware ===============================
//
// Assign PIC32 I2C peripheral channel(s) that the project uses...
#define USE_I2C_CHANNEL_1       // use I2C1 for all I2C devices
#define EEPROM_I2C_PORT_NUMBER   1    

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
