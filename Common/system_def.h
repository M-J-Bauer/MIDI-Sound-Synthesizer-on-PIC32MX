/****************************************************************************************
 *
 * FileName:   system_def.h
 *
 * File contains system definitions common to all applications and build variants
 * developed within the "PIC32MX Projects" domain.
 *
 * =======================================================================================
 */
#ifndef SYSTEM_DEF_H
#define SYSTEM_DEF_H

#include "../Common/Compiler.h"
#include "../Common/GenericTypeDefs.h"
#include "../Common/TimeDelay.h"

//#define _DISABLE_OPENADC10_CONFIGPORT_WARNING  // why? I forget! - MJB

#define SYSTEM_CLOCK_HZ      (80000000UL)
#define PERIPH_CLOCK_HZ      (80000000UL)
#define LITTLE_ENDIAN   1    // True for PIC32 and XC32-gcc
#define CP0_PREFETCH_CACHE_ENABLED

// This macro returns the system clock frequency in Hertz.
#define GetSystemClock()     (80000000UL)  

// This macro returns the peripheral clock frequency in Hertz.
#define GetPeripheralClock()    (GetSystemClock() / (1 << OSCCONbits.PBDIV))

// This macro returns the instruction clock rate in Hertz,
// dependent on the number of flash PM wait states configured
// and/or CP0 kseg0 pre-fetch cache configuration.
// PIC32MX3xx/4xx requires 2 wait states @ SYSCLK = 80MHz.
//
#ifdef CP0_PREFETCH_CACHE_ENABLED
#define GetInstructionClock()   (80000000)  // assume 0 wait states (cache on)
#else
#define GetInstructionClock()   ((80000000 * 10) / 23)  // assume 2 wait states
#endif

#define FIXED_POINT_FORMAT_12_20_BITS   // range: +/-2047,  precision: +/-0.000001

typedef signed char         int8;
typedef unsigned char       uint8,  uchar;

typedef signed short        int16;
typedef unsigned short      uint16, ushort;

typedef signed long         int32;
typedef unsigned long       uint32, ulong;

typedef signed long long    int64;
typedef unsigned long long  uint64;

typedef signed long         fixed_t;   // 32-bit fixed point

#ifndef bool
typedef unsigned char       bool;
#endif

typedef void (* pfnvoid)(void);     // pointer to void function

#ifndef NULL
#define NULL ((void *) 0)
#endif

#ifndef PRIVATE
#define PRIVATE  static    // for module private functions
#endif

#define until(exp)  while(!(exp))   // Usage:  do { ... } until (exp);
#define NOTHING

#define TEST_BIT(u, bm)   ((u) & (bm))
#define SET_BIT(u, bm)    ((u) |= (bm))
#define CLEAR_BIT(u, bm)  ((u) &= ~(bm))

#define SWAP(w)     ((((w) & 0xFF) << 8) | (((w) >> 8) & 0xFF))

#if LITTLE_ENDIAN
#define LSB_MSB(w)  (w)        // LSB is already first
#else
#define LSB_MSB(w)  SWAP(w)    // Swap bytes to put LSB first
#endif

#define HI_BYTE(w)  (((w) >> 8) & 0xFF)   // Extract high-order byte from unsigned word
#define LO_BYTE(w)  ((w) & 0xFF)          // Extract low-order byte from unsigned word

#define LESSER_OF(arg1,arg2)  ((arg1)<=(arg2)?(arg1):(arg2))
#define ARRAY_SIZE(a)  (sizeof(a)/sizeof(a[0]))
#define MIN(x,y)       ((x > y)? y: x)

#if defined FIXED_POINT_FORMAT_18_14_BITS
// Macros for manipulating 32-bit (18:14) fixed-point numbers, type fixed_t.
// Integer part:    18 bits, signed, max. range +/-128K
// Fractional part: 14 bits, precision: +/-0.00005 (approx.)
//
#define IntToFixedPt(i)     (i << 14)                    // convert int to fixed-pt
#define FloatToFixed(r)     (fixed_t)(r * 16384)         // convert float (r) to fixed-pt
#define FixedToFloat(z)     ((float)z / 16384)           // convert fixed-pt (z) to float
#define IntegerPart(z)      (z >> 14)                    // get integer part of fixed-pt
#define FractionPart(z,n)   ((z & 0x3FFF) >> (14 - n))   // get n MS bits of fractional part
#define MultiplyFixed(v,w)  ((v * w) >> 14)              // product of two fixed-pt numbers^
#define LongMultiplyFixed(v,w)  (((int64)v * w) >> 14)   // product of two numbers > 4.0
//
// ^ Use MultiplyFixed(v,w) where (v * w) <= 4.0, otherwise use LongMultiplyFixed(v,w)
// " Disable IRQ while LongMultiplyFixed(v,w) executes, if an ISR uses the product!
//
#elif defined FIXED_POINT_FORMAT_12_20_BITS
// Macros for manipulating 32-bit (12:20) fixed-point numbers, type fixed_t.
// Integer part:    12 bits, signed, max. range +/-2047
// Fractional part: 20 bits, precision: +/-0.000001 (approx.)
//
#define IntToFixedPt(i)     (i << 20)                    // convert int to fixed-pt
#define FloatToFixed(r)     (fixed_t)(r * 1048576)       // convert float (r) to fixed-pt
#define FixedToFloat(z)     ((float)z / 1048576)         // convert fixed-pt (z) to float
#define IntegerPart(z)      (z >> 20)                    // get integer part of fixed-pt
#define FractionPart(z,n)   ((z & 0xFFFFF) >> (20 - n))  // get n MS bits of fractional part
#define MultiplyFixed(v,w)  (((int64)v * w) >> 20)       // product of two fixed-pt numbers
//
// <!> Disable IRQ while MultiplyFixed(v,w) executes if an ISR uses the product!
//
#else 
#error "Fixed-point format undefined!"
#endif  // FIXED_POINT_FORMAT


/***** Commonly used symbolic constants *****/

#define ERROR     (-1)
#define SUCCESS     0
#define LOW         0
#define HI          1
#define DISABLE     0
#define ENABLE      1

// ======================================================================================
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
void   BackgroundTaskExec(void);  // call-back
//
// ======================================================================================

#endif // SYSTEM_DEF_H
