/**
 * Module:     pic32_low_level.c
 *
 * Overview:   Low-level platform-specific I/O functions for REMI Synth mk2 variants
 *             based on PIC32MX340 MCU (Olimex PIC32-MX340 or Chua board).
 *
 * Note:       May contain conditional compilation directives to suit various
 *             application build variants. May contain traces of nut products.
 *
 */
#include "pic32_low_level.h"

#include <stdlib.h>
#include <string.h>

static  uint16  m_AnalogReading[16];     // ADC inputs -- raw readings


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
    TRISFbits.TRISF5 = 0;     // RF5 pin is output (U2TX)
    TRISFbits.TRISF6 = 1;     // RF6 is an input (MRF-INT))

    TRISGbits.TRISG9 = 0;     // RG9 pin is output (SS2#/UEXT-CS#)
    LATGbits.LATG9 = 1;       // RG9 pin set High

    // RB6:RB0 (7 pins) are configured as ADC inputs by default, whereas...
    AD1PCFG |= 0xFF80;        // RB15:RB7 (9 pins) are GPIO

#ifdef SYNTH_MK2_MX340_LITE
    TRISBbits.TRISB7 = 0;     // RB7 pin is BATT_CHG_ENAB# (active low)
    TRISCbits.TRISC13 = 1;    // RC13 pin is button_A input
    TRISCbits.TRISC14 = 1;    // RC14 pin is button_B input
    CNPUE = 0x0003;           // Enable pullup resistors on RC13 (CN1) and RC14 (CN0)
#else
    TRISCbits.TRISC14 = 0;    // RC14 pin is output (Testpoint TP2)
    LATCbits.LATC14 = 0;
#endif

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


#ifndef SYNTH_MK2_MX340_LITE
/*
 * Function:     Read push-button input states.  (Not used in 'Lite' variant.)
 *
 * Return val:   buttonStates = 6 bit value;  1 bit per button;  Hi = Pressed.
 *
 *               bit:  |  5  |  4  |  3  |  2  |  1  |  0  |
 *               key:  |  *  |  #  |  D  |  C  |  B  |  A  |
 */
uint8  ReadButtonInputs()
{
    return  (uint8) (((PORTB >> 10) ^ 0x3F) & 0x03F);  // active HIGH
}
#endif


/*
 * Function:     Read analog inputs.
 *               Non-blocking "task" called *frequently* from main background loop.
 *
 * Detail:       The acquisition time allowed for each input is 3ms, so 10 channels
 *               are serviced in around 30ms.
 *
 * Output:       Raw ADC counts (10 bit) are stored in array m_AnalogReading[],
 *               accessible by a call to AnalogResult(chan);
 */
void  ReadAnalogInputs()
{
    static uint8   channelList[16] = ADC_CHANNEL_LIST;  // defined in pic32-low-level.h
    static bool    prep_done;
    static short   chanIdx;  // index into array channelList[]
    static short   channel;  // ADC channel #
    static uint8   state;
    static uint32  acquisitionStartTime; // start of 3ms interval

    if (!prep_done)  // One-time initialization at power-on/reset
    {
        chanIdx = 0;
        channel = channelList[0];
        ADC_INPUT_SEL(channel);
        ADC_SAMPLING = 1;  // Start signal acquisition
        acquisitionStartTime = milliseconds();
        state = SIGNAL_ACQUISITION;
        prep_done = TRUE;
    }

    if (state == SIGNAL_ACQUISITION)
    {
        if ((milliseconds() - acquisitionStartTime) >= 3)  // 3ms interval
        {
            ADC_SAMPLING = 0;  // End sampling, start conversion
            state = WAITING_FOR_CONVERSION;
        }
    }
    else  // state == WAITING_FOR_CONVERSION
    {
        if (ADC_CONV_DONE)  // Every 3ms (approx)...
        {
            m_AnalogReading[channel] = ADC_RESULT_REG;  // get 10 bit raw result
            if (++chanIdx >= NUMBER_OF_ANALOG_INPUTS) chanIdx = 0;  // next channel
            channel = channelList[chanIdx];
            ADC_INPUT_SEL(channel);
            ADC_SAMPLING = 1;  // Start signal acquisition
            acquisitionStartTime = milliseconds();
            state = SIGNAL_ACQUISITION;
        }
    }
}


/*
 * Function:     Get raw ADC conversion result for a specified input (channel).
 *
 * Entry arg:    channel = ADC input pin number;  e.g. 7 for pin AN7/RB7;
 *               maximum value is 15.
 *
 * Return val:   (uint16) Raw ADC count, 10 bits, range 0..1023
 */
uint16  AnalogResult(uint8 channel)
{
    return  m_AnalogReading[channel & 15];
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
