/******************************************************************************
*
* File Name:       TimeDelay.c
* Dependencies:    GetInstructionClock()
* Processor:       PIC32MX
* Compiler:        XC32
* Disclaimer:      Hacked by MJB from code supplied by Microchip.
*
*******************************************************************************/
#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "system_def.h"
#include "TimeDelay.h"
#include "../Drivers/HardwareProfile.h"


/******************************************************************************
  Function:
    void   Delay1us()

  Description:
    This routine performs a software delay of approx. 1 microsec
    (longer if it gets interrupted).
    For delays of 10us or greater, use Delay10us() or DelayMs().

  Precondition:
    Macro GetInstructionClock() evaluates to the correct frequency (Hz).
    GetInstructionClock() should be defined in "HardwareProfile.h".

  Parameters:   none
*
*******************************************************************************/
void Delay1us()
{
    volatile INT32 cyclesReqd;

    // Calculate number of instruction cycles required to delay 1us.
    cyclesReqd = (INT32)(GetInstructionClock() / 1000000);

    // Subtract cycles used up until we reach the while loop below
    // plus the 5 cycle function return overhead.
    cyclesReqd -= 24;   //(19 + 5)

    while (cyclesReqd > 0)
    {
        cyclesReqd -= 8;
    }
}


/******************************************************************************
  Function:
    void Delay10us(UINT32 DelayTime_10us)

  Description:
    This routine performs a software delay in units of 10 microseconds,
    (longer if it gets interrupted).

  Precondition:
    Macro GetInstructionClock() evaluates to the correct frequency (Hz),
    taking into account the number of flash PM wait states.
    GetInstructionClock() should be defined in "HardwareProfile.h".

  Parameters:
    UINT32 DelayTime_10us = number of ten microsecond delays to perform.
*
*******************************************************************************/
void  Delay10us(UINT32 tenMicroSecondCounter)
{
    volatile INT32 cyclesReqd;
        
    // Calculate number of instruction cycles required to delay
    // (10us * tenMicroSecondCounter)
    cyclesReqd = (INT32)(GetInstructionClock() / 100000) * tenMicroSecondCounter;

    // Subtract cycles used up until we reach the while loop below,
    // plus the 5 cycle function return overhead.
    cyclesReqd -= 24;    // (19 + 5)

    while (cyclesReqd > 0)
    {
        cyclesReqd -= 8;
    }
}


/*******************************************************************************
  Function:
    void DelayMs(UINT16 DelayTime_ms)

  Description:
    This routine performs a software delay in intervals of 1 millisecond.

  Precondition:
    None

  Parameters:
    UINT16 ms - number of one millisecond delays to perform at once.
*
*******************************************************************************/
void DelayMs(UINT16  ms)
{
    volatile UINT8 i;

    while (ms--)
    {
        i = 4;
        while (i--)
        {
            Delay10us( 25 );
        }
    }
}

