/**
 * Module:     pic32_low_level.c
 *
 * Overview:   Low-level platform-specific I/O functions for REMI Synth mk3
 *             based on 'MAM' board with Olimex PIC32-PINGUINO-MICRO (PIC32MX440).
 *
 *             Includes real-time O/S kernel and system timer functions.
 */
#include "pic32_low_level.h"

// =====================  MCU Configuration bits  ==================================
//
#ifdef __32MX440F256H__    // MX440 has a USB peripheral
#pragma config UPLLEN = ON, UPLLIDIV = DIV_2
#endif

#pragma config FPLLMUL = MUL_20, FPLLIDIV = DIV_2, FPLLODIV = DIV_1, FPBDIV = DIV_1
// Disable secondary osc. (use pins RC13, RC14 for GPIO)
#pragma config POSCMOD = HS, FNOSC = PRIPLL, FSOSCEN = OFF, OSCIOFNC = OFF
#pragma config FCKSM = CSECMD    // System clock Switching Mode Disabled
#pragma config ICESEL = ICS_PGx2   // DEBUG = ON (for ICD-3)
#pragma config WDTPS = PS1, FWDTEN = OFF       // watchdog timer
#pragma config CP = OFF, BWP = OFF, PWP = OFF  // code protect options
//
//==================================================================================

uint32  g_TaskCallFrequency;  // # calls to 1ms background task per second
uint32  g_SoftTimerError;

volatile unsigned int  v_RTI_tick_counter;
volatile unsigned char v_RTI_flag_1ms_task;
volatile unsigned char v_RTI_flag_5ms_task;
volatile unsigned char v_RTI_flag_50ms_task;
volatile unsigned char v_RTI_flag_500ms_task;
volatile unsigned char v_RTI_flag_NewDayRollover;

static  uint16  m_AnalogReading[16];     // ADC inputs -- raw readings


/**``````````````````````````````````````````````````````````````````````````````````````
 * General-purpose wait timer -- (resolution = RTI timer period = 1ms)
 * This function simply waits for a specified time, in milliseconds.
 * While waiting, any active background tasks are executed.
 *
 * NB:  Must not be called from within any background task!
 */
void WaitMilliseconds(unsigned int timeout_ms)
{
    unsigned int start_time = v_RTI_tick_counter;

    while (v_RTI_tick_counter - start_time < timeout_ms)
    {
        BackgroundTaskExec();
    }
}


/**
 * General-purpose millisecond timer -- (resolution = RTI timer period = 1ms)
 * Returns the value of a free-running counter variable incremented every millisecond.
 * The difference between values returned on two successive calls is the time elapsed
 * between calls, in milliseconds, up to a maximum interval of 2**(sizeof(int)*8) ms.
 */
uint32  milliseconds(void)
{
    return v_RTI_tick_counter;
}


/**
 * General-purpose delay timer -- (resolution = RTI timer period = 1ms)
 * This function simply delays for a specified time, in milliseconds, up to a maximum
 * interval of 2**(sizeof(int)*8) ms.
 * While waiting, all background tasks are suspended (delayed).
 */
void  Delay_ms(unsigned int timeout_ms)
{
    unsigned int start_time = v_RTI_tick_counter;

    while ((v_RTI_tick_counter - start_time) < timeout_ms)
    {
        /* Waste time */
    }
}


/**----------------------------------------------------------------------------------
 * Name               :  ReadCoreCountReg()
 *
 * Function           :  Returns value of CPU core cycle COUNT register.
 *
 * Input              :  --
 * Note               :  The COUNT register is incremented at half the instruction 
 *                       cycle frequency, i.e. count interval = 25ns @ Fsys = 80MHz.
------------------------------------------------------------------------------------*/
uint32  ReadCoreCountReg()
{
    unsigned int CC_Reg;
    
    asm volatile("mfc0   %0, $9" : "=r"(CC_Reg));

    return  CC_Reg;
}


/**----------------------------------------------------------------------------------
 * Name               :  Delay_Nx25ns()
 *
 * Function           :  Delay a multiple of 25ns (approx).
 * Dependency         :  PIC32MX 3/4/7 family CPU clocked @ Fsys = 80MHz
 *
 * Input              :  count = delay time, unit = 25ns  (@ TCY = 12.5ns)
 * Example            :  Delay_Nx25ns(4) gives a 100ns delay (minimum) 
 * Note               :  Delay will be longer if the function is interrupted.
 * Return             :  --
------------------------------------------------------------------------------------*/
void  Delay_Nx25ns(unsigned int count)
{
    unsigned int CC_Reg;
    unsigned int termCount;
    
    asm volatile("mfc0   %0, $9" : "=r"(CC_Reg));
    termCount = CC_Reg + count;
    
    while (CC_Reg < termCount)
    {
        asm volatile("mfc0   %0, $9" : "=r"(CC_Reg));
    } 
}


/**```````````````````````````````````````````````````````````````````````````````````````
 * Function:  InitializeMCUclock()
 *
 * Overview:  Initializes processor clock and RTI timer (1ms "tick") interrupt;
 *            enables vectored interrupts.
 *
 */
void  InitializeMCUclock(void)
{
    unsigned int cache_con_reg;
    unsigned int val;
    unsigned int status = 0;     // arg in asm instrn "ei"
    register unsigned long tmp;

    while (!OSCCONbits.LOCK) { ;;; }  // wait until SYS PLL stable

    // Configure wait states -- Flash PM access needs 2 wait states @ 80MHz
    CHECON = 2;           // 
    BMXCONCLR = _BMXCON_BMXWSDRM_MASK;   // No wait state on DRM access

    // Configure and enable MCU Pre-fetch cache...
    cache_con_reg = CHECON;
    cache_con_reg |= (3 << _CHECON_PREFEN_POSITION);
    CHECON = cache_con_reg;

    asm("mfc0 %0,$16,0" :  "=r"(tmp));   // CP0 Config reg bits[2:0] = 3
    tmp = (tmp & ~7) | 3;
    asm("mtc0 %0,$16,0" :: "r" (tmp));

    // Configure vectored interrupt mode
    asm volatile("mfc0   %0,$13" : "=r"(val));   // Read CP0 reg
    val |= 0x00800000;                           // Set CAUSE IV bit = 1
    asm volatile("mtc0   %0,$13" : "+r"(val));   // Write CP0 reg
    INTCONbits.MVEC = 1;

    asm volatile("ei    %0" : "=r"(status));     // Enable Interrupts

    DDPCON = 0;           // JTAGEN = 0 .. Disable JTAG pins, allow GPIO

    // Initialize Timer_1 for 1ms periodic interrupt
    PR1 = 10000 - 1;      // PBCLK = 80MHz, Tpbclk = 12.5ns
    IFS0bits.T1IF = 0;
    T1CONbits.TCKPS = 1;  // Prescaler set to 1:8
    IPC1bits.T1IP = 4;    // Timer1 IRQ priority
    T1CONbits.TON = 1;    // Start timer/counter
    IEC0bits.T1IE = 1;    // Enable T1 IRQ
}


/**```````````````````````````````````````````````````````````````````````````````````````
* Function:  Timer #1 ISR  -- System RTI timer "tick"
*
* PreCondition: Timer 1 initialization must be done.
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
* Overview:  Real-Time Interrupt (RTI) service routine (1ms "tick")...
*            Schedules periodic background tasks at 1ms, 5ms, 50ms and 500ms intervals.
*            Schedules periodic foreground tasks at 1ms intervals.
*            Maintains general-purpose timer with 1ms resolution.
*/
void  __ISR(_TIMER_1_VECTOR, IPL5AUTO)  Timer_1_RTI_Handler(void)
{
    static short count_to_5 = 0;
    static short count_to_50 = 0;
    static short count_to_500 = 0;
    static short count_to_1000 = 0;

    IFS0bits.T1IF = 0;

    v_RTI_tick_counter++;
    v_RTI_flag_1ms_task = 1;

    if (++count_to_5  >= 5) { v_RTI_flag_5ms_task = 1;  count_to_5 = 0; }
    if (++count_to_50 >= 50) { v_RTI_flag_50ms_task = 1;  count_to_50 = 0; }
    if (++count_to_500 >= 500) { v_RTI_flag_500ms_task = 1;  count_to_500 = 0; }
    if (++count_to_1000 >= 1000)
    {
        count_to_1000 = 0;
        g_TaskCallFrequency = g_TaskRunningCount;
        g_TaskRunningCount = 0;
    }

#ifdef INCLUDE_KERNEL_RTC_SUPPORT
    if (count_to_1000 == 0)  // seconds rollover
    {
        if ( ++sRTC.secs >= 60 )
        {
            sRTC.secs = 0;
            if ( ++sRTC.mins >= 60 )
            {
                sRTC.mins = 0;
                if ( ++sRTC.hour >= 24 )
                {
                    sRTC.hour = 0;
                    v_RTI_flag_NewDayRollover = 1;
                }
            }
        }
    }
#endif
}


/*
 * These functions return TRUE if their respective Task Flag is raised;
 * otherwise they return FALSE.  The Task Flag is cleared before the function exits,
 * so that on subsequent calls it will return FALSE, until the next task period ends.
 */
BOOL  isTaskPending_1ms()
{
    BOOL  result = v_RTI_flag_1ms_task;

    if (result) v_RTI_flag_1ms_task = 0;
    return  result;
}

BOOL  isTaskPending_5ms()
{
    BOOL  result = v_RTI_flag_5ms_task;

    if (result) v_RTI_flag_5ms_task = 0;
    return  result;
}

BOOL  isTaskPending_50ms()
{
    BOOL  result = v_RTI_flag_50ms_task;

    if (result) v_RTI_flag_50ms_task = 0;
    return  result;
}

BOOL  isTaskPending_500ms()
{
    BOOL  result = v_RTI_flag_500ms_task;

    if (result) v_RTI_flag_500ms_task = 0;
    return  result;
}


/*
* Function:  BootReset()
*
* Overview:  Software-driven MCU reset, equivalent to hardware MCLR# activation.
*            The reset vector normally points to the bootloader entry address.
*            If there is no bootloader installed, the reset vector points to the
*            application program entry point (startup code).
**/
void  BootReset()
{
    unsigned int status = 0;

    asm volatile("di    %0" : "=r"(status));     // Disable Interrupts

    SYSKEY = 0x00000000;   // Execute register unlock sequence
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;

    RSWRST = 1;
    v_RTI_tick_counter = RSWRST;  // dummy read triggers SWR

    while ((1 + 1) == 2) ;;;      // Wait for it
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


//=============  Application-specific Low-level functions  ==============================

void  Init_MCU_IO_ports(void)
{
    // Unless reconfigured subsequently, e.g. by a device driver ...
    TRISB = 0xFFFF;           // PORTB pins are all inputs  (incl. ADC)
    TRISD = 0x0000;           // PORTD pins are all outputs (except RD0, U1RX, SDA1, SCL1)
    TRISE = 0x0000;           // PORTE pins are all outputs (LCD data bus)
	
    TRISDbits.TRISD0 = 1;     // RD0 is high-Z (= BUT--RD4)
    TRISDbits.TRISD2 = 1;     // RD2 is an input pin (U1RX)
    TRISDbits.TRISD9 = 1;     // RD9 is high-Z (= I2C SDA1)
    TRISDbits.TRISD10 = 1;    // RD10 is high-Z (= I2C SCL1)

    TRISFbits.TRISF0 = 0;     // RF0 pin is output (SPI DAC CS#)
    LATFbits.LATF0 = 1;       // RF0 pin set High
    
    TRISFbits.TRISF1 = 0;     // RF1 pin is output (LCD-RST#)
    LATFbits.LATF1 = 1;       // RF1 pin set High
	
    TRISFbits.TRISF4 = 1;     // RF4/U2RX pin is input (CLI-RXD)
    TRISFbits.TRISF5 = 0;     // RF5/U2TX pin is output (CLI-TXD)
    
    TRISGbits.TRISG9 = 0;     // RG9 pin is output
    LATGbits.LATG9 = 0;       // RG9 pin set LOW
    
    // Pins RB1, RB2, RB3, RB4, RB8, RB9, RB10, RB11, RB12, RB14 are analog inputs
    AD1PCFG |= 0b1010000011100001;  // Low bits are ADC inputs; High bits are GPIO
	
    TRISCbits.TRISC13 = 1;    // RC13 pin is input (HW_CFG_0)
    TRISCbits.TRISC14 = 1;    // RC14 pin is input (HW_CFG_1)
    CNPUE = 0x0003;           // Enable pull-ups on RC13 (CN1), RC14 (CN0)
    
    Init_I2C1();              // for ext. EEPROM, etc
    ADC_Init();               // for Control Panel, etc
    SPI_Init(2, 0, 3);        // for audio SPI DAC  (ch2, mode 0, 10MHz)
}


/**
 * Function:  Initialize Timer #2 and OC2/RD1 pin for PWM audio DAC operation.
 *
 * Timer_2 is set up to generate the PWM audio output signal using a sampling
 * rate of 40ks/s.  Prescaler = 1:1;  Fclk = FCY = 80MHz;  Tclk = 12.5ns.
 * Timer_2 period := 50.00us (4000 x 12.5ns);  PR2 = 1999;  PWM freq = 40kHz.
 * Maximum duty register value is 1999.
 * Output Compare module OC2 is set up for PWM (fault-detect disabled).
 */
void  PWM_audioDAC_init(void)
{
    TRISDbits.TRISD1 = 0;    // RD1/OC2 is PWM output pin
    OC2CON = 0x0000;         // Disable OC2 while timer is set up

    T2CON = 0;               // Timer_2 setup for 40KHz PWM freq.
    T2CONbits.TCKPS = 0;     // Prescaler set to 1:1
    PR2 = 1999;              // Period = 2000 x 12.5ns (-> freq = 40kHz)
    IFS0bits.T2IF = 0;       // Clear IRQ flag
    IPC2bits.T2IP = 6;       // Set IRQ priority (highest!)
    T2CONbits.TON = 1;       // Start Timer

    OC2R = 1000;             // PWM Set initial duty (50%)
    OC2RS = 1000;
    OC2CON = 0x8006;         // Enable OC2/RD1 for PWM using TIMER_2

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


/**
 * Function:     Determine push-button states from analog input pin AN14/RB14.
 *
 * Return val:   buttonStates = 6 bit value;  1 bit per button;  Hi = Pressed.
 * 
 *               bit:  |  5  |  4  |  3  |  2  |  1  |  0  |
 *               key:  |  *  |  #  |  D  |  C  |  B  |  A  |
 */
uint8  ReadButtonInputs()
{
    uint8  buttonStates = 0;
    
    if (m_AnalogReading[14] < 90) buttonStates = MASK_BUTTON_HASH;
    else if (m_AnalogReading[14] < 250) buttonStates = MASK_BUTTON_D;
    else if (m_AnalogReading[14] < 370) buttonStates = MASK_BUTTON_C;
    else if (m_AnalogReading[14] < 440) buttonStates = MASK_BUTTON_B;
    else if (m_AnalogReading[14] < 490) buttonStates = MASK_BUTTON_A;
    else if (m_AnalogReading[14] < 600) buttonStates = MASK_BUTTON_STAR;
    
    return  buttonStates;
}


/***
 * Function:   Read analog inputs.
 *             Non-blocking "task" called *frequently* from main background loop.
 * 
 * Detail:     The acquisition time allowed for each input is 3ms, so 10 channels
 *             are serviced in around 30ms.
 * 
 * Output:     Raw ADC counts (10 bit) are stored in array m_AnalogReading[],
 *             accessible by a call to AnalogResult(chan);
 */
void  ReadAnalogInputs()
{
    static uint8   channelList[] = ADC_CHANNEL_LIST;  // defined in pic32-low-level.h
    static bool    prep_done;
    static short   chanIdx;  // index into array channelList[]
    static uint8   state;
    static uint32  acquisitionStartTime; // start of 3ms interval
    uint8  channel;  // actual ADC input number (AN##)
    
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
            channel = channelList[chanIdx];
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


/**
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


/**
 * Function:     Get hardware configuration jumper setting.
 *
 * Return val:   (uint8) H/W config jumpers = RC14:RC13 (2 bit value)
 */
uint8  GetHardwareConfig()
{
    return  ((uint8) READ_HW_CFG_JUMPER_P1 << 1 + READ_HW_CFG_JUMPER_P0);
}

