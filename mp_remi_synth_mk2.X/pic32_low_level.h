/**
 *  File:  pic32_low_level.h
 *
 *  Note:  Definitions in this file are mostly hardware platform specific.
 *
 */
#ifndef PIC32_LOW_LEVEL_H
#define PIC32_LOW_LEVEL_H

#include "../Common/system_def.h"
#include "HardwareProfile.h"

// PIC32MX ADC macros
//
#ifndef ADC_INPUT_SEL

#define ADC_INPUT_SEL   AD1CHS              // ADC Input Channel Selector
#define ADC_SAMPLING    AD1CON1bits.SAMP    // ADC Sample/Hold-Start
#define ADC_CONV_DONE   AD1CON1bits.DONE    // ADC Status
#define ADC_RESULT_REG  ADC1BUF0            // ADC Conversion Result Buffer Register

// Boolean values for AD1PCFG bits
#define PIN_CONFIG_ANALOG    0
#define PIN_CONFIG_DIGITAL   1

#endif

// Macros to enable/disable audio wave sampling routine (Timer 2 ISR)...
//
#define TIMER2_IRQ_DISABLE()   IEC0bits.T2IE = 0
#define TIMER2_IRQ_ENABLE()    IEC0bits.T2IE = 1
//
#define TIMER3_IRQ_DISABLE()   IEC0bits.T3IE = 0
#define TIMER3_IRQ_ENABLE()    IEC0bits.T3IE = 1

// Macros to set EXT-CS# pin high or low  (RG9/SS2# on UEXT connector)
#define EXT_CS_HIGH()     LATGbits.LATG9 = 1
#define EXT_CS_LOW()      LATFbits.LATG9 = 0

// Macro to read 6 button input pins (RB15..RB10) into 16-bit word (bits 5:0)
#define READ_BUTTON_INPUTS()   ((PORTB >> 10) & 0x003F)  // active LOW

// Testpoint to measure Timer2 ISR execution time with 'scope...
#define TESTPOINT_T2ISR_SET_HI()   LATAbits.LATA7 = 1
#define TESTPOINT_T2ISR_SET_LO()   LATAbits.LATA7 = 0

#define TESTPOINT_RC14_SET_HI()    LATCbits.LATC14 = 1
#define TESTPOINT_RC14_SET_LO()    LATCbits.LATC14 = 0

void   Init_MCU_IO_ports(void);
void   ADC_Init(void);
void   DebugLEDControl(uint8 state);
void   TestpointOutputT2ISR(uint8 state);
uint8  GetHardwareConfig();
uint8  ReverseOrderBits(uint8 bDat);

#endif  // PIC32_LOW_LEVEL_H
