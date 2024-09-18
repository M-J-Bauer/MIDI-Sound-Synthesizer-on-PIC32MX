/**
 *  File:      pic32_low_level.h
 *
 * Overview:   Low-level platform-specific definitions for REMI Synth mk2 variants
 *             based on PIC32MX340 MCU (Olimex PIC32-MX340 or Chua board).
 *
 */
#ifndef PIC32_LOW_LEVEL_H
#define PIC32_LOW_LEVEL_H

#include "../Drivers/HardwareProfile.h"

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

#define SIGNAL_ACQUISITION        0
#define WAITING_FOR_CONVERSION    1

#define NUMBER_OF_ANALOG_INPUTS   7
#define ADC_CHANNEL_LIST    { 0, 1, 2, 3, 4, 5, 6 }  // ADC inputs serviced (any order)
#define POT_CHANNEL_LIST    { 0, 1, 2, 3, 4, 5 }  // Pot inputs (sub-set of ADC chan list)

// Arg u32 is a variable of type uint32
#define READ_CPU_CORE_COUNT_REG(u32)  asm volatile("mfc0   %0, $9" : "=r"(u32));

// Macros to enable/disable audio wave sampling routine (Timer 2 ISR)...
//
#define TIMER2_IRQ_DISABLE()   IEC0bits.T2IE = 0
#define TIMER2_IRQ_ENABLE()    IEC0bits.T2IE = 1
//
#define TIMER3_IRQ_DISABLE()   IEC0bits.T3IE = 0
#define TIMER3_IRQ_ENABLE()    IEC0bits.T3IE = 1

// Macro to set OC4 duty register (value 0..1999) for 11-bit PWM audio DAC...
// Refer to function PWM_audioDAC_init() defined in file "pic32_low_level.c"
#define PWM_AUDIO_DAC_WRITE(duty)  OC4RS = duty

// Macros to set EXT-CS# pin high or low  (RG9/SS2# on UEXT connector)
#define EXT_CS_HIGH()     LATGbits.LATG9 = 1
#define EXT_CS_LOW()      LATFbits.LATG9 = 0

// Macro to read 6 button input pins (RB15..RB10) into 16-bit word (bits 5:0)
#define READ_BUTTON_INPUTS()   ((PORTB >> 10) & 0x003F)  // active LOW

#define ALL_BUTTONS_RELEASED       0x00
#define MASK_BUTTON_A              0b00000001 
#define MASK_BUTTON_B              0b00000010 
#define MASK_BUTTON_C              0b00000100  
#define MASK_BUTTON_D              0b00001000
#define MASK_BUTTON_HASH           0b00010000  
#define MASK_BUTTON_STAR           0b00100000

// Macros to read 2 button inputs -- synth LITE variant only
#define READ_BUTTON_A_PIN          (PORTCbits.RC13)
#define READ_BUTTON_B_PIN          (PORTCbits.RC14)

// Macros to control and monitor LCD backlight LED on pin RD9 (mk2/mx340 platform)
#define LCD_BACKLIGHT_SET_HIGH()   LATDbits.LATD9 = 1
#define LCD_BACKLIGHT_SET_LOW()    LATDbits.LATD9 = 0
#define LCD_BACKLIGHT_IS_LOW       (LATDbits.LATD9 == 0)
#define LCD_BACKLIGHT_IS_HIGH      (LATDbits.LATD9 != 0)

// Macros to control "Clipping Indicator" LED on pin RD0 (mk2/mx340 platform)
#define CLIPPING_LED_ON()          LATDbits.LATD0 = 1
#define CLIPPING_LED_OFF()         LATDbits.LATD0 = 0

// Macros to read configuration jumper inputs (pins RB8, RB9) (mk2/mx340 platform)
#define READ_HW_CFG_JUMPER_P0      (PORTBbits.RB9)
#define READ_HW_CFG_JUMPER_P1      (PORTBbits.RB8)

// Macros to enable/disable battery charge circuit ('Lite' variant only)
#define BATT_CHARGE_ENABLE()       LATBbits.LATB7 = 0;  // set LOW (0V) = active
#define BATT_CHARGE_DISABLE()      LATBbits.LATB7 = 1;  // set HIGH (+3.3V)

#define UART2_ENABLE()             U2MODE = 0x8000     // U2MODE.ON = 1
#define UART2_DISABLE()            U2MODE = 0x0000     // U2MODE.ON = 0
#define U2TX_PIN_SET_LOW()         LATFbits.LATF5 = 0  // RF5 = LOW


void   Init_MCU_IO_ports(void);
void   ADC_Init(void);
void   DebugLEDControl(uint8 state);
void   ToggleBacklight(void);
void   TestpointOutputT2ISR(uint8 state);
void   ReadAnalogInputs();
uint16 AnalogResult(uint8 channel);
uint8  ReadButtonInputs();
uint8  GetHardwareConfig();
uint8  ReverseOrderBits(uint8 bDat);

#endif  // PIC32_LOW_LEVEL_H
