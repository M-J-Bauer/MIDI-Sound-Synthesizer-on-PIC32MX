/**
 * File:       pic32_low_level.h
 *
 * Overview:   Low-level platform-specific definitions for REMI Synth mk3
 *             based on Olimex PIC32-PINGUINO-MICRO (PIC32MX440) MCU.
 *
 */
#ifndef PIC32_LOW_LEVEL_H
#define PIC32_LOW_LEVEL_H

#include "../Common/Compiler.h"
#include "../Common/system_def.h"
#include "../Drivers/HardwareProfile.h"
#include "../Drivers/SPI_drv.h"  // for SPI DAC

#include <sys/attribs.h>   // For interrupt handlers
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifdef INCLUDE_KERNEL_RTC_SUPPORT
#include "RTC_support.h"
#endif

// PIC32MX ADC macros
//
// PIC32MX ADC macros for manual conversion
// To start signal acquisition (sampling), let ADC_SAMPLING = 1;
// To stop sampling, i.e. hold signal and begin conversion, let ADC_SAMPLING = 0;
// Wait until ADC_CONV_DONE == 1, then read result from ADC_RESULT_REG
//
#ifndef ADC_SAMPLING
//
#define ADC_INPUT_SEL(n)  (AD1CHS = n << 16)    // ADC Input Channel Selector
#define ADC_SAMPLING      AD1CON1bits.SAMP      // ADC Sample/Hold-Start
#define ADC_CONV_DONE     (AD1CON1bits.DONE)    // ADC Conversion Status
#define ADC_RESULT_REG    ADC1BUF0              // ADC Conversion Result Buffer Register

// Boolean values for AD1PCFG bits
#define PIN_CONFIG_ANALOG    0
#define PIN_CONFIG_DIGITAL   1
//
#endif

#define NUMBER_OF_ANALOG_INPUTS  10   // Application-specific
#define SIGNAL_ACQUISITION        0
#define WAITING_FOR_CONVERSION    1

// Arg u32 is a variable of type uint32
#define READ_CPU_CORE_COUNT_REG(u32)  asm volatile("mfc0   %0, $9" : "=r"(u32));

// Macros to enable/disable audio wave sampling routine (Timer 2 ISR)...
//
#define TIMER2_IRQ_DISABLE()   IEC0bits.T2IE = 0
#define TIMER2_IRQ_ENABLE()    IEC0bits.T2IE = 1
//
#define TIMER3_IRQ_DISABLE()   IEC0bits.T3IE = 0
#define TIMER3_IRQ_ENABLE()    IEC0bits.T3IE = 1

// Macro to set OC2 duty register (value 0..1999) for 11-bit PWM audio DAC...
// Refer to function PWM_audioDAC_init() defined in file "pic32_low_level.c"
#define PWM_AUDIO_DAC_WRITE(duty)  OC2RS = duty

// Macros to set EXT-CS# pin high or low  (RG9/SS2# on UEXT connector)
#define EXT_CS_HIGH()     LATGbits.LATG9 = 1
#define EXT_CS_LOW()      LATFbits.LATG9 = 0

#define ALL_BUTTONS_RELEASED       0x00
#define MASK_BUTTON_A              0b00000001 
#define MASK_BUTTON_B              0b00000010 
#define MASK_BUTTON_C              0b00000100  
#define MASK_BUTTON_D              0b00001000
#define MASK_BUTTON_HASH           0b00010000  
#define MASK_BUTTON_STAR           0b00100000

// Specify the 6 ADC inputs assigned to Control Panel pots -- platform specific:
#define POT_ADC_CHANNEL_LIST       { 1, 2, 3, 4, 8, 9 } 

// Macros to control and monitor LCD backlight LED on pin RD9
#define LCD_BACKLIGHT_SET_HIGH()   LATDbits.LATD4 = 1
#define LCD_BACKLIGHT_SET_LOW()    LATDbits.LATD4 = 0
#define LCD_BACKLIGHT_IS_LOW       (LATDbits.LATD4 == 0)
#define LCD_BACKLIGHT_IS_HIGH      (LATDbits.LATD4 != 0)

// Macros to control "Clipping Indicator" LED on pin RD0 (-- N/A on MX440 --)
#define CLIPPING_LED_ON()          {}
#define CLIPPING_LED_OFF()         {}

// Macros to support SPI DAC MCP4921
#define SPI_DAC_CS_LOW()           LATFbits.LATF0 = 0; 
#define SPI_DAC_CS_HIGH()          LATFbits.LATF0 = 1; 

// Macros to read hardware configuration jumper inputs 
#define READ_HW_CFG_JUMPER_P0      (PORTCbits.RC13)
#define READ_HW_CFG_JUMPER_P1      (PORTCbits.RC14)

extern  uint32  g_TaskRunningCount;  // incremented on every call to 1ms B/G task

// ------------- kernel functions --------------------
//
uint32 ReadCoreCountReg();
uint32 milliseconds(void);
void   WaitMilliseconds(unsigned int timeout_ms);
void   Delay_ms(unsigned int timeout_ms);
void   Delay_Nx25ns(unsigned int count);
void   InitializeMCUclock(void);

BOOL   isTaskPending_1ms();
BOOL   isTaskPending_5ms();
BOOL   isTaskPending_50ms();
BOOL   isTaskPending_500ms();
void   BootReset();
uint8  ReverseOrderBits(uint8 bDat);

void   BackgroundTaskExec(void);  // call-back

// ------------- low-level I/O  functions -----------------
//
void   Init_MCU_IO_ports(void);
void   ADC_Init(void);
void   DebugLEDControl(uint8 state);
void   ToggleBacklight(void);

void   ReadAnalogInputs();
uint16 AnalogResult(uint8 channel);
uint8  ReadButtonInputs();  // Buttons on analog input pin
uint8  GetHardwareConfig();


#endif  // PIC32_LOW_LEVEL_H
