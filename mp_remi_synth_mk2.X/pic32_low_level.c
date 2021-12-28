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
    
   // RB3:RB0 (4 pins) are configured as ADC inputs by default, whereas...
    AD1PCFGbits.PCFG8 = 1;    // RB8 is a logic input (HW.CFG0 jumper)
    AD1PCFGbits.PCFG9 = 1;    // RB9 is a logic input (HW.CFG1 jumper)
    AD1PCFG |= 0xFC00;        // RB15:RB10 are logic inputs (6 push-buttons)
	
    TRISCbits.TRISC14 = 0;    // RC14 pin is output (Testpoint TP2)
    LATCbits.LATC14 = 0;
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

