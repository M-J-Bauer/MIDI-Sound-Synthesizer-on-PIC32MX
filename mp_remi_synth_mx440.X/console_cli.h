/*________________________________________________________________________________________*\

    File:        console_cli.h

    Author:      M.J.Bauer

    Header file for console CLI module.
\*________________________________________________________________________________________*/

#ifndef  _CONSOLE_CLI_H_
#define  _CONSOLE_CLI_H_

#include "../Common/system_def.h"
#include "../Drivers/HardwareProfile.h"  // must be included before driver def files
#include "../Drivers/UART_drv.h"

#ifdef INCLUDE_KERNEL_RTC_SUPPORT
#include "RTC_support.h"
#endif

#if !defined CLI_USING_UART1 && !defined CLI_USING_UART2
#define CLI_USING_UART2    TRUE      // use default for MJB's PIC32 framework
#endif

#define CLI_MAX_ARGS        12       // Maximum # of Command Line arguments (incl. name)
#define CMND_LINE_MAX_LEN   76       // Maximum length of command string
#define CMND_HIST_BUF_SIZE  20       // Maximum number of commands in recall buffer
#define MAX_COMMANDS       250       // Maximum number of CLI commands

#ifndef SU_PASSWORD
#define SU_PASSWORD    "PIC32"       // Default "super user" password
#endif

//  External functions required by the console CLI
//
#ifdef CLI_USING_UART1
#define RxDataAvail()   UART1_RxDataAvail()
#define RxFlush()       UART1_RxFlush()
#define getch()         UART1_getch()
#define putch(b)        UART1_putch(b)
#define putstr(s)       UART1_putstr(s)     
#endif

#ifdef CLI_USING_UART2
#define RxDataAvail()   UART2_RxDataAvail()
#define RxFlush()       UART2_RxFlush()
#define getch()         UART2_getch()
#define putch(b)        UART2_putch(b)
#define putstr(s)       UART2_putstr(s)     
#endif

#if !defined (CLI_USING_UART1) && !defined (CLI_USING_UART2)
#error "Can't build console-CLI module without serial port UART defined!"
#endif

#define  kbhit()        RxDataAvail()    // alias
#define  putNewLine()   putstr("\n")
#define  NEW_LINE       putstr("\n")     // deprecated, use putNewLine()

/******************************************************************************************/

#ifndef ASCII_NUL  // Common ASCII control codes

#define  ASCII_NUL       0
#define  ASCII_ACK       6
#define  ASCII_BS        8        // Ctrl+H, Backspace 
#define  ASCII_HT        9
#define  ASCII_TAB       9
#define  ASCII_LF       10
#define  ASCII_CR       13
#define  ASCII_DC2      18        // Ctrl+R, Device Control 2 
#define  ASCII_NAK      21
#define  ASCII_CAN      24        // Ctrl+X, Cancel line 
#define  ASCII_ESC      27
#define  ASCII_SP       32

#endif

// Command category codes...
#define  GEN_CMD     'G'
#define  APP_CMD     'A'
#define  SYS_CMD     'S'
#define  DBG_CMD     'D'

enum  eDebugDataType
{
    HEX_BYTE,
    CHAR_BYTE,
    DEC_BYTE,
    HEX_WORD,
    DEC_UWORD,
    DEC_IWORD,
    HEX_LONG,
    DEC_ILONG
};

typedef  void (*CLIfunc)( int argc, char *argv[] );   // pointer to CLI function


// Command table entry looks like this
typedef struct  Cmnd_Table_Entry
{
    char    *phzName;           // pointer to command name (string literal)
    uint8    yAttribute;        // command category (general, app, system, debug, etc)
    CLIfunc  Function;          // pointer to CLI function
    
} CmndTableEntry_t;


typedef struct UserSettableParameters  // For "param" (alias "set") command
{
    char   name[12];     // param nick-name, max. 10 chars (last entry = "$")
    char   format;       // display format: 'r' == real, 'i' == integer
    float *address;      // pointer to global parameter
    float  minValue;     // minimum acceptable value
    float  maxValue;     // maximum acceptable value

} UserSettableParameter_t;


extern  volatile BOOL  g_DiagnosticModeActive;
extern  float  g_DiagnosticModeTimeout;

uint32  millisecTimer();     // Return value of free-running 32-bit counter (unit = 1ms)
void    BackgroundTaskExec(void);  // Background task executive located in main module


// Prototypes of functions defined in console CLI module...

void    ConsoleCLI_Service( void );
void    CommandLineInterpreter( void );
void    PrepareForNewCommand( void );
void    EnterCommandInHistory( void );
void    RecallCommand( void );
void    EraseLine( void );
uint8   strmatch( char *pcStr1, const char *phzStr2 );
int     getstr(char *strBuf, int maxLen);

void    OutputParamValue(char *nickName, float fValue);
void    ListParamNamesValues(void);
void    SetParameterValue(char *nickName, float fValue);
char    SuperUserAccess(void);

// ------------  Generic CLI commands  ----------------------
void    Cmnd_remark( int argCount, char * argValue[] );
void    Cmnd_default( int argCount, char * argValue[] );
void    Cmnd_diag( int argCount, char * argValue[] );
void    Cmnd_flags( int argCount, char * argValue[] );
void    Cmnd_help( int argCount, char * argValue[] );
void    Cmnd_list( int argCount, char * argValue[] );

void    Cmnd_set( int argCount, char * argValue[] );
void    Cmnd_commit( int argCount, char * argValue[] );
void    Cmnd_reset( int argCount, char * argValue[] );
void    Cmnd_ver( int argCount, char * argValue[] );
void    Cmnd_watch( int argCount, char * argValue[] );
void    Cmnd_dump( int argCount, char * argValue[] );
void    Cmnd_su( int argCount, char * argValue[] );

#ifdef INCLUDE_KERNEL_RTC_SUPPORT
void    Cmnd_time( int argCount, char * argValue[] );
void    Cmnd_rtcc( int argCount, char *argValue[] );
#endif

// --------  File-System and memory device CLI commands  -------
#ifdef USE_MDD_FILE_SYSTEM  // FAT32 MSD
void    Cmnd_format( int argCount, char * argValue[] );
void    Cmnd_ls( int argCount, char * argValue[] );
void    Cmnd_rm( int argCount, char * argValue[] );
void    Cmnd_view( int argCount, char * argValue[] );
void    Cmnd_fcre8( int argCount, char * argValue[] );
#endif

#ifdef USE_SDF_FILE_SYSTEM  // Serial Data Flash
void    Cmnd_sst25( int argCount, char * argValue[] );
void    Cmnd_sdf( int argCount, char * argValue[] );
void    Cmnd_export(int argCount, char * argVal[]);
void    Cmnd_import(int argCount, char * argVal[]);
#endif

// ---------------- Debug commands  -------------------------
void    Cmnd_peek( int argCount, char * argValue[] );
void    Cmnd_poke( int argCount, char * argValue[] );
void    Cmnd_pio( int argCount, char * argValue[] );

// Formatted numeric string output functions...

void    putBoolean( uint8 );
void    putHexDigit( uint8 );
void    putHexByte( uint8 );
void    putHexWord( uint16 );
void    putHexLong( uint32 );
void    putDecimal( int32 lVal, uint8 bSize );

// Character & string numeric conversion functions...

uint8   dectobin( char c );
uint16  decatoi( char * pac, int8 bNdigs );
uint32  long_decatoi( char * pac, int8 bNdigs );
uint8   hexctobin( char c );
uint16  hexatoi( char * s );
uint32  long_hexatoi( char * s );
uint8   isHexDigit( char c );
char    hexitoc( short wDigit );
void    wordToHexStr( uint16 wVal, char *pcResult );
void    longToDecimalStr( int32 lVal, char *pcResult, uint8 bFieldSize );

#endif    /* _CONSOLE_CLI_H_ */
