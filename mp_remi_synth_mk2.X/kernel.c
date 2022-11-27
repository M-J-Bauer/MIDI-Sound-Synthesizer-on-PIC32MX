/**
 * Module:     kernel.c
 *
 * Overview:   Real-time "kernel" module common to all PIC32MX build variants.
 *             Includes MCU Configuration bits, system RTC (optional), various timers,
 *             and periodic task schedulers (foreground and background).
 *
 * Author:     M.J.Bauer, 2014   [www.mjbauer.biz]
 *
 * Notes:      Timer #1 is used for Real-Time Interrupt (1ms "tick").
 *
 *             The module contains no Microchip "PLIB" dependencies.
 */
#include <sys/attribs.h>    // For interrupt handlers
#include <math.h>

#include "../Common/system_def.h"
#include "HardwareProfile.h"

#ifdef INCLUDE_KERNEL_RTC_SUPPORT
#include "RTC_support.h"
#endif

// =====================  MCU Configuration bits  ==================================
//
#ifndef __32MX340F512H__    // 'MX340 doesn't have a USB peripheral
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

extern  uint32  g_TaskRunningCount;  // incremented on every call to 1ms B/G task

uint32  g_TaskCallFrequency;  // # calls to 1ms background task per second
uint32  g_SoftTimerError;

volatile unsigned int  v_RTI_tick_counter;
volatile unsigned char v_RTI_flag_1ms_task;
volatile unsigned char v_RTI_flag_5ms_task;
volatile unsigned char v_RTI_flag_50ms_task;
volatile unsigned char v_RTI_flag_500ms_task;
volatile unsigned char v_RTI_flag_NewDayRollover;


/***************************************************************************************************
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


/*----------------------------------------------------------------------------------
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


/*----------------------------------------------------------------------------------
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


/***************************************************************************************************
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


/***************************************************************************************************
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


/*^
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


#ifdef USE_USB_MSD_HOST_INTERFACE 
/**************************************************************************************************
 *  Function:   USB_ApplicationEventHandler()
 *
 *  Description:
 *    This is the application event handler.  It is called when the USB stack has
 *    an event that needs to be handled by the application layer rather than
 *    by the client driver.  If the application is able to handle the event, it
 *    returns TRUE.  Otherwise, it returns FALSE.
 *
 *  Entry Arg's:
 *    BYTE address    - Address of device where event occurred
 *    USB_EVENT event - Identifies the event that occured
 *    void *data      - Pointer to event-specific data
 *    DWORD size      - Size of the event-specific data
 *
 *  Return Value:
 *    TRUE    - The event was handled
 *    FALSE   - The event was not handled
 *
 *  Remarks:
 *    The application may also implement an event handling routine if it
 *    requires knowledge of events.  To do so, it must implement a routine that
 *    matches this function signature and define the USB_HOST_APP_EVENT_HANDLER
 *    macro as the name of that function.
 */
BOOL USB_ApplicationEventHandler(BYTE address, USB_EVENT event, void *data, DWORD size)
{
    BOOL  result = TRUE;

    switch (event)
    {
    case EVENT_VBUS_REQUEST_POWER:
        // The data pointer points to a byte that represents the amount of Vbus current
        // requested (Ibus = mA/2).  If the device wants too much power, we reject it.
        break;

    case EVENT_VBUS_RELEASE_POWER:
        // Turned off Vbus power.
        // This means that the device was removed
        g_MDD_MediaMounted = FALSE;
        break;

    case EVENT_HUB_ATTACH:
        break;

    case EVENT_UNSUPPORTED_DEVICE:
        break;

    case EVENT_CANNOT_ENUMERATE:
        putstr( "\n! USB Error - cannot enumerate device.\n" );
        break;

    case EVENT_CLIENT_INIT_ERROR:
        putstr( "\n! USB Error - client driver init error.\n" );
        break;

    case EVENT_OUT_OF_MEMORY:
        putstr( "\n! USB Error - insufficient heap memory.\n" );
        break;

    case EVENT_UNSPECIFIED_ERROR:   // Should never happen!
        putstr( "\n! USB Error - unspecified.\n" );
        break;

    default:
        result = FALSE;
        break;
    }

    return result;
}

#endif // USE_USB_MSD_HOST_INTERFACE
