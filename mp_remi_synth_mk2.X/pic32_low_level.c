/**
 * Module:     pic32_low_level.c
 *
 * Overview:   Low-level platform-specific I/O functions for PIC32MX projects.
 *
 * Note:       May contain conditional compilation directives to suit various
 *             application build variants. May contain traces of nut products.
 *
 */
#include "pic32_low_level.h"

#include <stdlib.h>
#include <string.h>


void  Init_MCU_IO_ports(void)
{
    // Unless reconfigured subsequently, e.g. by a device driver ...
    TRISB = 0xFFFF;           // PORTB pins are all inputs (incl. ADC)
    TRISD = 0x0000;           // PORTD pins are all outputs
    TRISE = 0x0000;           // PORTE pins are all outputs

    TRISFbits.TRISF0 = 0;     // RF0 pin is output (MRF-SS#)
    LATFbits.LATF0 = 1;       // RF0 pin set High
    
    TRISFbits.TRISF1 = 0;     // RF1 pin is output (MRF-RST#)
    LATFbits.LATF1 = 1;       // RF1 pin set High
	
    TRISFbits.TRISF6 = 1;     // RF6 is an input (MRF-INT))
    
    TRISGbits.TRISG9 = 0;     // RG9 pin is output (SS2#/UEXT-CS#)
    LATGbits.LATG9 = 1;       // RG9 pin set High
    
    // RB7:RB0 (8 pins) are configured as ADC inputs by default, whereas...
    AD1PCFG |= 0xFF00;        // RB15:RB8 (8 pins) are LOGIC inputs
	
    TRISCbits.TRISC14 = 0;    // RC14 pin is output (Testpoint TP2)
    LATCbits.LATC14 = 0;
    
    Init_I2C1();              // for ext. EEPROM, etc
    ADC_Init();               // for Control Panel, etc
}


/*`````````````````````````````````````````````````````````````````````````````````````````````````
 * Function:  Initialize Timer #2 and OC4 pin for PWM audio DAC operation.
 *
 * Timer_2 is set up to generate the PWM audio output signal using a sampling
 * rate of 40ks/s.  Prescaler = 1:1;  Fclk = FCY = 80MHz;  Tclk = 12.5ns.
 * Timer_2 period := 50.00us (4000 x 12.5ns);  PR2 = 1999;  PWM freq = 40kHz.
 * Maximum duty register value is 1999.
 * Output Compare module OC4 is set up for PWM (fault-detect disabled).
 */
void  PWM_audioDAC_init(void)
{
    TRISDbits.TRISD3 = 0;    // RD3/OC4 is an output pin
    TRISDbits.TRISD4 = 0;    // RD4/OC5 ..   ..   ..

    OC4CON = 0x0000;         // Disable OC4 while timer is set up

    T2CON = 0;               // Timer_2 setup for 40KHz PWM freq.
    T2CONbits.TCKPS = 0;     // Prescaler set to 1:1
    PR2 = 1999;              // Period = 2000 x 12.5ns (-> freq = 40kHz)
    IFS0bits.T2IF = 0;       // Clear IRQ flag
    IPC2bits.T2IP = 6;       // Set IRQ priority (highest!)
    T2CONbits.TON = 1;       // Start Timer

    OC4R = 1000;             // PWM Set initial duty (50%)
    OC4RS = 1000;
    OC4CON = 0x8006;         // Enable OC4 for PWM

    TIMER2_IRQ_ENABLE();
}


// Initialize the ADC for manual sampling mode
//
void  ADC_Init(void)
{
    AD1CON1 = 0;            // reset
    AD1CON2 = 0;            // Vref+ = AVdd, Vref- = AVss (defaults)
    AD1CON3 = 0x0002;       // Tad = 4 * Tcy
    AD1CON1 = 0x8000;       // Turn on A/D module, use manual convert
    AD1CSSL = 0;            // No scanned inputs
}


void  DebugLEDControl(uint8 state)
{
    if (state == 0) { DEBUG_LED_OFF(); }
    else if (state == 1) { DEBUG_LED_ON(); }
    else { DEBUG_LED_TOGGLE(); }
}


void  ToggleBacklight(void)
{
    static uint8 flipflop;
    
    if (flipflop == 0) { LCD_BACKLIGHT_SET_HIGH();  flipflop = 1; }
    else  { LCD_BACKLIGHT_SET_LOW();  flipflop = 0; }
}


/*
 * Function:     Get hardware configuration jumper setting.
 *
 * Return val:   (uint8) H/W config jumpers = RB9:RB8 (2 bit value)
 */
uint8  GetHardwareConfig()
{
    return  (uint8) (PORTB >> 8) & 3;  // only 2 bits wanted
}


/*
 * Function reverses the order of bits in the byte passed as argument.
 */
uint8  ReverseOrderBits(uint8 bDat)
{
    int    i;
    uint8  revByte = 0x00;

    for (i = 0;  i < 8;  i++)
    {
        revByte <<= 1;
        if (bDat & 1) revByte |= 0x01;
        bDat >>= 1;
    }

    return  revByte;
}

