/*________________________________________________________________________________________*\

  File:        console_cli.c

  Author:      M.J.Bauer  [www.mjbauer.biz]

  This module implements the generic console Command Line user Interface (CLI),
  including command functions common to most applications.

  Additional application-specific command functions should be placed in a separate
  source file and declared in a separate header file.

\*________________________________________________________________________________________*/

#include  "console_cli.h"

#include  <ctype.h>
#include  <string.h>
#include  <stdlib.h>
#include  <math.h>

//---------------------  Application-specific externals  ------------------------------------------
//
extern volatile BOOL g_MDD_MediaMounted;    // flag: True if MDD storage media inserted
extern uint8  g_FW_version[];    // firmware version # (major, minor, build)
extern char  *g_AppTitleCLI;     // Title string output by "ver" command

// The application must define a table of app-specific commands.
// This table has the same structure as CommonCommands[].
extern const  CmndTableEntry_t  AppCommands[];

// The application must define an array UserParam[], even if it is not used...
extern UserSettableParameter_t  UserParam[];    // for "set" command

// The application must provide these "callback" functions, even if they do nothing:
extern void  DefaultPersistentData(void);   // Restore factory defaults to EEPROM
extern void  CommitPersistentParams(void);  // Write specific variables to EEPROM
extern void  ActivateSetParamValues(void);  // Apply modified parameter value(s))
extern void  WatchCommandExec(void);        // real-time display of selected variables
extern void  DiagnosticCommandExec(int argCount, char * argValue[]);  // for "diag" cmd

// External Functions -- (maybe application-specific)
extern void  BootReset(void);
extern int   RTCC_Synchronize( void );


//-------------------------------  Private data  --------------------------------------------------
//                     
static  char     CLIprompt[] = "\r> ";                // CLI Prompt
static  char     CmndLineBuff[CMND_LINE_MAX_LEN+2];   // Command Line buffer
static  char    *cmndLinePtr;   // Pointer into Cmnd Line buffer
static  int      cmndLineLen;   // Cmnd line length (char count)

static  char     CmndHistoryBuff[CMND_HIST_BUF_SIZE][CMND_LINE_MAX_LEN+2];
static  int16    cmndHistoryMarker;   // Index to next free place in Command History Buf
static  int16    cmndRecallMarker;    // Index of command to be recalled
static  char     SuperUser = FALSE;


/*----------------------------------------------------------------------------------------
*                      C O M M O N   C O M M A N D   T A B L E
*/
const  CmndTableEntry_t  CommonCommands[] =
{
    //   Cmd Name      Attribute      Cmd Function
    //----------     -------------   -------------------
    {    "*",          GEN_CMD,       Cmnd_remark     },
    {    "boot",       GEN_CMD,       Cmnd_reset      },
    {    "cmds",       GEN_CMD,       Cmnd_list       },
    {    "commit",     GEN_CMD,       Cmnd_commit     },
    {    "default",    GEN_CMD,       Cmnd_default    },
    {    "diag",       GEN_CMD,       Cmnd_diag       },
    {    "dump",       GEN_CMD,       Cmnd_dump       },
    {    "help",       GEN_CMD,       Cmnd_help       },
    {    "lc",         GEN_CMD,       Cmnd_list       },
    {    "set",        GEN_CMD,       Cmnd_set        },
    {    "su",         GEN_CMD,       Cmnd_su         },
    {    "ver",        GEN_CMD,       Cmnd_ver        },
    {    "watch",      GEN_CMD,       Cmnd_watch      },
    
#ifdef INCLUDE_KERNEL_RTC_SUPPORT
    {    "date",       SYS_CMD,       Cmnd_time       },
    {    "time",       SYS_CMD,       Cmnd_time       },
    {    "rtcc",       SYS_CMD,       Cmnd_rtcc       },
#endif
    
#ifdef USE_MDD_FILE_SYSTEM  // FAT32 MSD
    {    "format",     SYS_CMD,       Cmnd_format     },  // MDD media commands
    {    "ls",         SYS_CMD,       Cmnd_ls         },
    {    "rm",         SYS_CMD,       Cmnd_rm         },
    {    "del",        SYS_CMD,       Cmnd_rm         },
    {    "view",       SYS_CMD,       Cmnd_view       },
    {    "fcre8",      SYS_CMD,       Cmnd_fcre8      },
#endif

#ifdef USE_SDF_FILE_SYSTEM  // Serial Data Flash
    {    "sst25",      SYS_CMD,       Cmnd_sst25      },  // SDF memory commands
    {    "sdf",        SYS_CMD,       Cmnd_sdf        },
#ifdef USE_MDD_FILE_SYSTEM
    {    "export",     SYS_CMD,       Cmnd_export     },  // SDF with MDD commands
    {    "import",     SYS_CMD,       Cmnd_import     },
#endif
#endif
    //---------------------------------------------------
    {    "$",          0,             NULL            }   // Dummy last entry
} ;


/*----------------------------------------------------------------------------------------
*                         COMMAND LINE INPUT STREAM HANDLER
*
*   Called frequently from any wait loop(s) in the main process.
*   Must not be called from the Background Task Executive, because command functions
*   which incorporate time delays do call the B/G Task Executive and hence there
*   would be potential risk of infinite recursion; e.g. see Cmnd_watch().
*
*   If the serial port input (RX) buffer is not empty, the function will...
*     .  Input a char from the serial RX buffer;
*     .  Provide simple line editing (Backspace, Ctrl-X);
*     .  Append the char, if printable, to the command line input buffer;
*     .  If CR received (cmd line terminator), call the CommandLineInterpreter();
*     .  If Ctrl+R received, recall previously entered command from history buffer;
*     .  If TAB received, it is converted to a single space;
*     .  Otherwise, ASCII CTRL chars are ignored.
*/
void  ConsoleCLI_Service( void )
{
    static bool prep_done;
    char  c;
    
    if (!prep_done)  // one-time initialization at start-up
    {
        cmndLineLen = 0;  // prepare for new command
        CmndLineBuff[0] = 0;
        cmndLinePtr = CmndLineBuff;
        putstr(CLIprompt);
        prep_done = TRUE;
    }

    if ( RxDataAvail() )     // char(s) available in serial input buffer
    {
        c = getch();             // Fetch the char... no echo (yet)
        switch ( c )
        {
        case ASCII_CAN:                 // Ctrl+X... cancel line...
            EraseLine();
            cmndLineLen = 0;  // prepare for new command
            CmndLineBuff[0] = 0;
            cmndLinePtr = CmndLineBuff;
            putstr(CLIprompt);
            break;

        case ASCII_BS:                  // BACKSPACE...
            if ( cmndLineLen > 0 )
            {
                cmndLinePtr-- ;           // Remove last-entered char from buffer
                cmndLineLen-- ;
                putch( ASCII_BS );          // Backspace the VDU cursor
                putch( ' ' );               // Erase offending char at VDU cursor
                putch( ASCII_BS );          // Re-position the VDU cursor
            }
            break;

        case ASCII_CR:                  // ENTER...
            putch( '\r' );                  // Echo NewLine
            putch( '\n' );
            if ( cmndLineLen > 0 )         // Got a command string...
            {
                *cmndLinePtr = 0;           // Terminate the command string
                EnterCommandInHistory();    // Enter it into the history buffer
                CommandLineInterpreter();   // Interpret and execute the cmnd.
            }
            cmndLineLen = 0;                // prepare for new command
            CmndLineBuff[0] = 0;
            cmndLinePtr = CmndLineBuff;
            putstr(CLIprompt);
            break;

        case ASCII_DC2:                 // Ctrl+R... recall command
            EraseLine();                    // Trash the current command line
            cmndLineLen = 0;                // prepare for new command
            CmndLineBuff[0] = 0;
            cmndLinePtr = CmndLineBuff;
            putstr(CLIprompt);
            RecallCommand();                // Retrieve previous command from history
            break;

        case ASCII_TAB:                 // TAB...  convert to single space
            c = ' ';
        //  no break... fall thru to default case
        default:
            if ( isprint( c ) && cmndLineLen < CMND_LINE_MAX_LEN )
            {
                putch( c );                 // Echo char
                *cmndLinePtr++ = c;       // Append char to Cmnd Line buffer
                cmndLineLen++ ;
            }
            break;
        } // end_switch
    }
}


/*----------------------------------------------------------------------------------------
*                    C O M M A N D   L I N E   I N T E R P R E T E R
*
*   Processes the received command line when entered.
*
*   The CommandLineInterpreter() function is called by ConsoleCLI_Service()
*   when a valid command string is entered (CR received). It finds & counts "arguments"
*   in the command line and makes them NUL-terminated strings in-situ.
*   It then searches the Command Table for command name, argValue[0],
*   and if the name is found, executes the respective command function.
*
*   If there is a cmd line argument following the cmd name and it is "-help" (Help option),
*   the argument is converted to "?", which is the (simpler) alternative syntax.
*
*   A command string is comprised of a command name and one or more "arguments" separated
*   by one or more spaces, or TABs. The ordering of arguments is determined by particular
*   command functions, so may or may not be important. The degree of syntax checking is also
*   the responsibility of command functions.
*
*   Command names, hexadecimal arg's, and option switches (e.g. "-a") are not case-sensitive;
*   other arguments may or may not be case-sensitive, as interpreted by particular command
*   functions. (The strmatch() function is not case-sensitive.)
*/
void  CommandLineInterpreter( void )
{
    static char  *argValue[CLI_MAX_ARGS];
    int     argCount = 0;
    char    c;
    short   i, icmd;
    char    commonCmndNameFound = 0;
    char    appCmndNameFound = 0;

    cmndLinePtr = CmndLineBuff;            // point to start of Cmnd Line buffer
    for (i = 1;  i < CLI_MAX_ARGS;  i++)   // Clear all command arg's
        { argValue[i] = NULL; }

    // This loop finds and terminates (with a NUL) any user-supplied arguments...
    for ( i = 0;  i < CLI_MAX_ARGS;  i++ )
    {
        if ( !isprint( *cmndLinePtr ) )               // stop at end of line
            break;
        while ( *cmndLinePtr == ' ' )                 // skip leading spaces
            cmndLinePtr++ ;
        if ( !isprint( *cmndLinePtr ) )               // end of line found
            break;
        argValue[i] = cmndLinePtr;                  // Make ptr to arg
        argCount++ ;

        while ( ( c = *cmndLinePtr ) != ' ' )         // find first space after arg
        {
            if ( !isprint( c ) )                        // end of line found
                break;
            cmndLinePtr++ ;
        }
        if ( !isprint( *cmndLinePtr ) )               // stop at end of line
            break;
        *cmndLinePtr++ = 0;                           // NUL-terminate the arg
    }

    // This loop searches the common command table for the supplied command name...
    for ( i = 0;  i < MAX_COMMANDS;  i++ )
    {
        if ( *CommonCommands[i].phzName == '$' )      // reached end of table
            break;
        if ( strmatch(argValue[0], CommonCommands[i].phzName ))
        {
            commonCmndNameFound = 1;
            icmd = i;
            break;
        }
    }
    
    // This loop searches the (external) application-specific command table...
    for ( i = 0;  i < MAX_COMMANDS;  i++ )
    {
        if ( *AppCommands[i].phzName == '$' )         // reached end of table
            break;
        if ( strmatch(argValue[0], AppCommands[i].phzName) )
        {
            appCmndNameFound = 1;
            icmd = i;
            break;
        }
    }
    
    if ( argCount > 1 )   // If there are user-supplied arg(s)...
    {
        if ( strmatch( argValue[1], "-help" ) )    // convert "-help" to '?' ...
            *argValue[1] = '?';                    // ... to simplify cmd fn
    }

    if (appCmndNameFound)
    {
        (*AppCommands[icmd].Function)(argCount, argValue);  // execute cmd fn
    }
    else if (commonCmndNameFound)
    {
        (*CommonCommands[icmd].Function)(argCount, argValue);  // execute fn
    }
    else  putstr( "? Undefined command.\n" );
}


/*
*   Copy the newly entered command into the history buffer for later recall...
*   if it's length is non-zero.
*/
void  EnterCommandInHistory( void )
{
    static bool  initialised = FALSE;
    short  line;

    if ( !initialised )
    {
        for ( line = 0 ; line < CMND_HIST_BUF_SIZE ; line++ )
        {
            CmndHistoryBuff[line][0] = 0;    // make empty cmnd string
        }
        cmndHistoryMarker = 0;
        cmndRecallMarker = 0;
        initialised = 1;
    }

    if ( strlen( CmndLineBuff ) != 0 )   // Not an empty cmnd string
    {
        strncpy( CmndHistoryBuff[cmndHistoryMarker], CmndLineBuff, CMND_LINE_MAX_LEN );
        cmndRecallMarker = cmndHistoryMarker;
        cmndHistoryMarker++ ;
        if ( cmndHistoryMarker >= CMND_HIST_BUF_SIZE ) cmndHistoryMarker = 0;
    }
}


/*
*   Recall a previously entered command from the history buffer...
*   The function selects the next previous command from the buffer (if any)
*   as indicated by cmndRecallMarker, and outputs the command string to the user's
*   terminal for editing. At the same time, the selected command is copied to the
*   current command line buffer.
*   The selected command is not executed until the user hits ENTER (CR).
*/
void  RecallCommand( void )
{
    strncpy( CmndLineBuff, CmndHistoryBuff[cmndRecallMarker], CMND_LINE_MAX_LEN );
    if ( cmndRecallMarker == 0 ) cmndRecallMarker = CMND_HIST_BUF_SIZE;
    --cmndRecallMarker;
    cmndLineLen = strlen( CmndLineBuff );
    cmndLinePtr = CmndLineBuff + cmndLineLen;
    *cmndLinePtr = 0;
    putstr( CmndLineBuff );
}


/*
*   Erase the command line on user terminal; cursor remains on same line at col 1.
*/
void  EraseLine( void )
{
    short  col;

    putch('\r');
    for ( col=0 ; col < (CMND_LINE_MAX_LEN+2) ; col++ )
    {
        putch(' ');
    }
    putch('\r');
}


/*****
*   Function:   strmatch
*
*   Purpose:    Compares two NUL-terminated strings, each up to 255 chars in length.
*               Returns 1 if the strings have the same contents, up to the terminator
*               of either string, except that the comparison is not case-sensitive.
*
*   Args:       (char *) pcStr1       :  pointer to string variable
*               (const char *) phStr2 :  pointer to string literal
*
*   Returns:    (FLAG) 1 if string #1 matches string #2 (case-insensitive), else 0.
*/
uint8  strmatch( char *pcStr1, const char *phStr2 )
{
    char    c1, c2;
    uint8   b = 255;
    uint8   yResult = 1;

    while ( b-- != 0 )
    {
        c1 = tolower( *pcStr1++ );
        c2 = tolower( *phStr2++ );
        if ( c1 != c2 )  yResult = 0;
        if ( c1 == 0 || c2 == 0 )  break;
    }
    return yResult;
}


/*****
*   Function:   getstr
*
*   Purpose:    Gets a text string from the console input stream (user terminal).
*               While waiting for input, background tasks are executed.
*               Input is terminated (with a NUL) and the function returns when a CR
*               (ASCII 0x0D) code is received.
*               BACKSPACE removes last received char from the input buffer.
*               TAB is converted to a single space.
*               Otherwise, CTRL chars are ignored.
*
*   Args:       (char *) strBuf :  pointer to input buffer
*               (int) maxLen : maximum length of input string (not incl. NUL)
*
*   Returns:    Length of input string (bytes) up to NUL terminator.
*/
int  getstr(char *strBuf, int maxLen)
{
    char  c;
    char  *inBuf = strBuf;
    char  exit = 0;
    char  length = 0;

    while (!exit)
    {
        if ( RxDataAvail() ) 
        {
            c = getch();  // no echo (yet)
            switch ( c )
            {
            case ASCII_BS:     // BACKSPACE
                if (length > 0)
                {
                    inBuf-- ;  length--;
                    putch( ASCII_BS );
                    putch( ' ' );
                    putch( ASCII_BS );
                }
                break;

            case ASCII_CR:     // ENTER
                putstr("\n");
                *inBuf = 0;
                exit = 1;
                break;

            case ASCII_TAB:
                c = ' ';
                //\/\/\/\/
            default:
                if (isprint(c) && length < maxLen)
                {
                    putch( c );    // echo
                    *inBuf++ = c;
                    *inBuf = 0;
                    length++;
                }
                break;
            } // end_switch
        }
        BackgroundTaskExec();
    }

    return length;
}


/*________________________________________________________________________________________*\
*
*              G E N E R I C   C L I   C O M M A N D   F U N C T I O N S
*
*   All CLI command functions have access to user-supplied command line arguments.
*   These are passed to the function via two arguments (parameters)...
*
*     (int) argCount : number of user-supplied command line arguments + 1
*                       (i.e. including the command name, argValue[0])
*
*     (char *) argValue[] : array of pointers to user-supplied argument strings
*
\*________________________________________________________________________________________*/
/*
*
*   CLI command function:  Cmnd_help
*
*   The "help" command (alias "?") gives brief help on CLI usage.
*/
void  Cmnd_help( int argCount, char * argValue[] )
{
    putstr( "Command arg's and options must be separated by spaces.\n" );
    putstr( "To list available commands, type 'lc' or 'cmds'.\n" );
    putstr( "To recall a previously entered command, hit Ctrl+R.\n" );
    putstr( "Some commands, when entered with the switch '-help' or '?', show command \n" );
    putstr( "usage information;  otherwise, a command which expects at least one arg, \n" );
    putstr( "when entered without any, should also show command usage. \n" );
    putstr( "In usage info, square brackets enclose optional parameters, e.g. [<x1>]; \n" );
    putstr( "braces enclose alternative options, e.g. {-x|-y }, where '|' means 'OR'. \n" );
}


/**
*   CLI command function:  Cmnd_remark
*
*   The "rem" command (alias "*") allows a remark to be entered in a command
*   line without getting an error message back from the CLI.
*/
void  Cmnd_remark( int argCount, char * argValue[] )
{
    if ( *argValue[1] == '?' )   // help wanted
    {
        putstr( "Usage:  * <remark> \n");
        putstr( "Enter a remark, e.g. for a command log file.\n");
        return;
    }
}


/*****
*   CLI command function:  Cmnd_list
*
*   The "lc" command (alias "cmds") lists CLI command names, formatted into 6 columns.
*   Common commands are listed first, then app-specific commands.
*/
void  Cmnd_list( int argCount, char * argValue[] )
{
    char    *commandName;
    short   iTable;
    short   iCmd;
    short   iSpacesToPad;
    short   iColumn = 0;
    
    CmndTableEntry_t  *CmdTable = (CmndTableEntry_t *) CommonCommands;

    for (iTable = 0;  iTable < 2;  iTable++)  // 2 tables
    {
        for (iCmd = 0;  iCmd < MAX_COMMANDS;  iCmd++)
        {
            commandName = CmdTable[iCmd].phzName;
            
            if (commandName[0] == '$')  // Reached end of table
                break;            

            putstr( commandName );
            iSpacesToPad = 12 - strlen(commandName);
            while (iSpacesToPad-- != 0)
            {
                putch(' ');
            }
            if (++iColumn >= 6)  { iColumn = 0;  NEW_LINE; }

        }
        CmdTable = (CmndTableEntry_t *) AppCommands;    // next table
    }

    if (iColumn != 0) NEW_LINE;
}


/*****
*   CLI command function:  ver
*
*   The "ver" command displays firmware version number and other build information.
*/
void  Cmnd_ver( int argCount, char * argValue[] )
{
    putstr(g_AppTitleCLI);
    putstr( "Firmware version " );
    putDecimal( g_FW_version[0], 1 );
    putch( '.' );
    putDecimal( g_FW_version[1], 1 );
    putch( '.' );
    if (g_FW_version[2] < 10) putch('0');
    putDecimal( g_FW_version[2], 1 );
    putstr( ", " );
    putstr( __DATE__ );
    NEW_LINE;
}


/*****
*   CLI command function:  Cmnd_default
*
*   The "default" command restores "factory default" values to all persistent data,
*   typically held in EEPROM, the same as when non-volatile memory becomes corrupted.
*/
void  Cmnd_default(int argCount, char * argValue[])
{
    char  c = 0;
    BOOL  confirm = FALSE;

//  if (!SuperUserAccess()) return;

    if (argCount == 1)  // Cmd name only...
    {
        putstr( "Restore persistent data to factory defaults? (Y/N): " );
        while (c < 0x20)
        {
            if (RxDataAvail()) c = getch();
        }
        putch(c);  putstr("\n");
        if (toupper(c) == 'Y') confirm = TRUE;
    }
    else if (strmatch(argValue[1], "-y")) confirm = TRUE;

    if (confirm) 
    {
        DefaultPersistentData();
        putstr( "* Default configuration restored.\n" );
    }
}


/*****
*   CLI command function:  Cmnd_reset
*
*   The "boot" command invokes a boot/reset of the system.
*   The mode of reset is determined by the application "call-back" function.
*/
void  Cmnd_reset( int argCount, char * argValue[] )
{
//  if (!SuperUserAccess()) return;

    BootReset();
}


/*****
*   CLI command function:  Cmnd_watch
*
*   The "watch" command activates a real-time display which shows the value(s)
*   of one or more global application variables.
*   The function exits when it receives ASCII_ESC from the serial input stream,
*   ie. when user hits the [Esc] key on their terminal.
*/
void  Cmnd_watch( int argCount, char * argValue[] )
{
    static  uint16 period_end_time;
    static  uint16 elapsed_time = 0;  // millisec since watch started
    char c = 0;

//  if (!SuperUserAccess()) return;

    putstr( "Hit [Esc] to quit. \n\n" );

    while ( c != ASCII_ESC )
    {
        // Callback defined in app-specific code module...
        // Variables to be "watched" are output on a single line (no newline).
        WatchCommandExec();

        // Delay 200mS for refresh rate of 5/sec.
        // While waiting, any pending background tasks are executed.
        period_end_time = milliseconds() + 200;
        while (milliseconds() < period_end_time)
        {
            BackgroundTaskExec();
        }
        putch( '\r' );       // Return VDU cursor to start of output line
        
        if ( kbhit() ) c = getch();       // Check for key hit
        if ( c == ASCII_CR )  { c = 0;  NEW_LINE; }
        elapsed_time += 50 ;
    }
    NEW_LINE;
}


/*****
*   CLI command function:  Cmnd_flags
*
*   The "flags" command outputs various debug flags, error flags, etc.
*   Debug flags and transient Error flags are usually cleared after being output.
**
void  Cmnd_flags( int argCount, char * argValue[] )
{
    if (!SuperUserAccess()) return;

    putstr("! Not implemented.\n");
}
*/


/*****
*   CLI command function:  Cmnd_diag
*
*   The "diag" command runs various application-specific system diagnostics.
*   DiagnosticCommandExec() function should be in an application-specific code module.
*/
void  Cmnd_diag(int argCount, char * argValue[])
{
//  if (!SuperUserAccess()) return;

    DiagnosticCommandExec(argCount, argValue);
}


/**
*   CLI command function:  Cmnd_set
*
*   The "set" command may be used to list all user-settable global parameters,
*   by their nicknames, or to set a new value for a specified parameter.
*
*   All user-settable parameters are stored as floats, but may be output (displayed)
*   in "real" (%8.3f) or "integer" (%d) format to suit the application.
*
*   If any parameters are to be made persistent, i.e. written to non-volatile memory,
*   the "commit" command should be used to perform the required operation.
*   Because user-settable parameters are application-specific, the "commit" command
*   function should be implemented in an application code module.
*/
void  Cmnd_set( int argCount, char * argValue[] )
{
    float  fValue = 0.0;

    if (argCount == 2 && *argValue[1] == '?' )   // help wanted
    {
        putstr( "Usage:  set  [name [=] value] \n" );
        putstr( "... where <name> is a parameter nickname (not case-sensitive).\n" );
        putstr( "List all, or set one global parameter, by nickname. \n" );
        putstr( "If no arg is supplied, all settable param's are listed.\n" );
        return;
    }

    if (argCount == 3) fValue = atof(argValue[2]);
    else if (argCount > 3 && argValue[2][0] == '=') fValue = atof(argValue[3]);

    if (argCount >= 3) SetParameterValue(argValue[1], fValue);
    else  ListParamNamesValues();
}

// List nick-names and current values of global user-settable parameters...
//
void  ListParamNamesValues(void)
{
    char   outBuf[40];
    char  *nickName;
    short  i, col = 0;
    float *pfValue;

    for (i = 0;  TRUE;  i++)
    {
        if (strmatch(UserParam[i].name, "$"))  // end_of_list
            break;

        nickName = UserParam[i].name;
        pfValue = (float *) UserParam[i].address;

        if (UserParam[i].format == 'r')  // show as floating point
        {
            if (fabs(*pfValue) < 1000.0)
                sprintf(outBuf, "%-12s = %8.3f", nickName, (double) *pfValue);
            else  // |value| >= 1000 ... don't show fractional part
                sprintf(outBuf, "%-12s = %8.0f", nickName, (double) *pfValue);
        }
        else  // show as long integer
        {
            sprintf(outBuf, "%-12s = %8ld", nickName, (long) (*pfValue + .000001));
        }
        putstr(outBuf);

        if (col == 0) { putstr("    |  "); col = 1; }  // two-up formatting
        else { putstr("\n"); col = 0; }
    }

    if (col == 1) putstr("\n");
}

// Set value of global user-settable parameter from command line argument
//
void  SetParameterValue(char *nickName, float fValue)
{
    short  i;
    char   outBuf[40];
    char   end_of_list = 0;

    for (i = 0; !end_of_list; i++)
    {
        if (strmatch(UserParam[i].name, "$"))
            { end_of_list = 1;  break; }

        if (strmatch(UserParam[i].name, nickName))
        {
            if (fValue >= UserParam[i].minValue && fValue <= UserParam[i].maxValue)
            {
                *UserParam[i].address = fValue;  // accept the value
            }
            else  // reject the value
            {
                sprintf(outBuf, "! %f is out of bounds.  ", (double) fValue);
                putstr(outBuf);
            }
            break;
        }
    }

    if (!end_of_list)
    {
        if (UserParam[i].format == 'r')  // output as real
            sprintf(outBuf, "* %s = %5.3f \n", nickName, (double) *UserParam[i].address);
        else 
            sprintf(outBuf, "* %s = %d \n", nickName, (int) *UserParam[i].address);
    }
    else  sprintf(outBuf, "! Undefined name: %s \n", nickName);

    putstr(outBuf);
    
    SynthPrepare();  // Assume a param value was modified

//  CommitPersistentParams();
}


/**
*   CLI command function:  Cmnd_commit
*
*   The "commit" command invokes a background task to write current values of
*   user-settable parameters to non-volatile memory. Thus, the current parameter
*   values will become the default values used at subsequent system restarts.
*   NB: This function applies only to parameters settable by the 'set' command.
*/
void    Cmnd_commit( int argCount, char * argValue[] )
{
    char  c = 0;
    BOOL  commit = FALSE;

    if (argCount == 2 && *argValue[1] == '?' )   // help wanted
    {
        putstr( "Usage:  commit [-y] \n" );
        putstr( "Commit 'set' parameter values to non-volatile memory, i.e.\n" );
        putstr( "make current values the power-on/restart defaults.\n" );
        return;
    }

    if (argCount == 1)  // Cmd name only...
    {
        putstr( "Make current parameter values the defaults? (Y/N): " );
        while (c < 0x20)
        {
            if (RxDataAvail()) c = getch();
        }
        putch(c);  putstr("\n");
        if (toupper(c) == 'Y') commit = TRUE;
    }
    else if (strmatch(argValue[1], "-y")) commit = TRUE;

    if (commit) CommitPersistentParams();  // Function defined in application module
}


/**
*   CLI command function:  Cmnd_su
*
*   The "su" command enables "super user" (developer, administrator) privilege,
*   e.g. for access to sensitive commands that could potentially cause grief.
*   If the password argValue[1] is a mismatch, super-user access is cancelled.
*/
void  Cmnd_su( int argCount, char * argValue[] )
{
    if (argCount == 2 && *argValue[1] != '?')
    {
        if (strcmp(argValue[1], SU_PASSWORD) == 0) 
        {
            SuperUser = TRUE;
            putstr( "* Super user status enabled.\n" );
        }
        else  
        {
            SuperUser = FALSE;
            putstr( "! Super user status disabled.\n" );
        }
    }
    else  // help wanted
    {
        putstr( "Usage:  su [password] \n" );
        putstr( "Enable/disable 'super-user' privilege.\n" );
    }
}


/**
*   This function may be called by command functions with restricted options to check
*   the status of SU access.  If SU access is disabled, the function will prompt the user
*   to enter the SU password, hence to allow access to the command function.
*/
char  SuperUserAccess(void)
{
    char  inputBuf[20];

    if (!SuperUser)
    {
        putstr("! Password: ");
        getstr(inputBuf, 18);
        if (strcmp(inputBuf, SU_PASSWORD) == 0) SuperUser = TRUE;
    }

    return SuperUser;
}


#ifdef INCLUDE_KERNEL_RTC_SUPPORT
/*^
*   CLI command function:  Cmnd_time
*
*   The "time" command is for reading or setting the real-time clock (RTC).
*/
void  Cmnd_time( int argCount, char * argValue[] )
{
    short i;
    short errcode;
    uint8 syntax_error = 0;
    char  cBuf[80];

#if defined (RTCC_TYPE_ISL1208) || defined (RTCC_TYPE_MCP79410)

    if ( *argValue[1] == '?' )   // help wanted
    {
        putstr( "Usage (1):  time [hh:mm] [yy-mm-dd] \n" );
        putstr( "   Set system time & date; initialize RTCC device (if present). \n" );
        putstr( "Usage (2):  time -s \n" );
        putstr( "   Synchronize system time & date to hardware RTCC device.\n" );
        putstr( "In both cases, or if no arg supplied, show system time & date.\n" );
        return;
    }
#else  // Use software real-time clock (RTI Timer)

    if ( *argValue[1] == '?' )   // help wanted
    {
        putstr( "Usage:  time [hh:mm] [yy-mm-dd] \n" );
        putstr( "Show time & date. If param(s) supplied, set time/date. \n" );
        return;
    }
#endif

#if defined (RTCC_TYPE_ISL1208) || defined (RTCC_TYPE_MCP79410)
    if (argCount == 2 && strmatch(argValue[1], "-s"))
    {
        if (RTCC_Synchronize() < 0) putstr("RTCC device not responding.\n");
    }
    else 
#endif

    if (argCount > 1)   // time/date arg(s) supplied...
    {
        for (i = 1; i < argCount; i++)
        {
            if (argValue[i][2] == ':')  // assume arg = hh:mm
            {
                gsRTCbuf.hour = atoi(&argValue[i][0]);
                gsRTCbuf.mins = atoi(&argValue[i][3]);
                gsRTCbuf.secs = 0;
            }
            else if (argValue[i][2] == '-')  // assume arg = yy-mm-dd
            {
                if (argValue[i][5] != '-') { syntax_error = 1; break; }
                gsRTCbuf.year = atoi(&argValue[i][0]);
                gsRTCbuf.month = atoi(&argValue[i][3]);
                gsRTCbuf.day = atoi(&argValue[i][6]);
            }
            else syntax_error = 1;
        }
        
        if (!syntax_error)
        {
            RTC_date_time_set();            // Set the software RTC "registers" from buffer

#if defined (RTCC_TYPE_ISL1208) || defined (RTCC_TYPE_MCP79410)
            errcode = RTCC_Initialize();    // Initialize RTCC chip and set time/date regs
            if (errcode == -3) putstr("! RTCC PF flag not reset \n");
            else if (errcode < 0) putstr("! RTCC device not detected \n");
            else putstr("RTCC device initialized \n");
#else
            putstr("  RTCC device not in use. \n");
#endif
        }
        else
        {
            putstr("Syntax error! Try: time hh:mm yy-mm-dd \n");
            return;
        }
    }

    if (RTC_date_time_read())
    {
        sprintf(cBuf, "%02d:%02d:%02d ", gsRTCbuf.hour, gsRTCbuf.mins, gsRTCbuf.secs);
        putstr(cBuf);
        sprintf(cBuf, "20%02d-%02d-%02d ", gsRTCbuf.year, gsRTCbuf.month, gsRTCbuf.day);
        putstr(cBuf);
        putstr(asDayOfWeekName[gsRTCbuf.dow]);  NEW_LINE;
    }
    else putstr("System time/date invalid.\n");
}

#endif // INCLUDE_KERNEL_RTC_SUPPORT


/*^***
*   CLI command function:  Cmnd_dump 
*
*   The "dump" command outputs a 256-byte page of PIC32MX MCU memory,
*   which may be either flash program memory (PM) or data memory (RAM).
*
*   Command syntax:  "dump [addr]"
*   ... where <addr> is the start address (hex), masked to 16-byte boundary.
*   If <addr> is omitted, dump next page.
*
*/
void  Cmnd_dump( int argCount, char * argValue[] )
{
    static  uint32 *pAddr = (uint32 *) 0x9D000000;  // Start of User App Flash (PIC32MX)
    short   row, col;

    if (!SuperUserAccess()) return;

    if (argCount >= 2)  // assume arg[1] is <addr>
        pAddr = (uint32 *) (long_hexatoi(argValue[1]) & ~0xF);  // mask to 16-byte boundary

    for (row = 0; row < 16; row++)
    {
        putHexLong((uint32) pAddr);
        putstr(" : ");

        for (col = 0;  col < 4;  col++)
        {
            putHexLong(*pAddr++);
            putch(' ');
        }
        NEW_LINE;
    }
}


/*________________________________________________________________________________________*\
*
*          F O R M A T T E D   N U M E R I C   O U T P U T   F U N C T I O N S
\*________________________________________________________________________________________*/
/*
*
*  Output Boolean value as ASCII '0' or '1'.
*
*  Called by:  CLI command functions (only)
*  Entry args: (uint8) b = Boolean variable (zero or non-zero)
*  Returns:    void
*  Affects:
*/

void  putBoolean( uint8  b )
{
    if ( b )  putch( '1');
    else  putch ( '0' );
}


/*****
*  Output 4 LS bits of a byte as Hex (or BCD) ASCII char.
*
*  Called by:  CLI command functions (only)
*  Entry args: d = value of Hex digit (0 to 0xf)
*  Returns:    void
*  Affects:    --
*/

void  putHexDigit( uint8 d )
{
    d &= 0x0F;
    if ( d < 10 )  putch ( '0' + d );
    else  putch ( 'A' + d - 10 );
}


/*****
*  Output byte as 2 Hex ASCII chars, MSD first.
*
*  Called by:  CLI command functions (only)
*  Entry args: b = byte to output
*  Returns:    void
*  Affects:    --
*/

void  putHexByte( uint8 b )
{
    putHexDigit( b >> 4 );
    putHexDigit( b );
}


/*****
*  Output 16-bit word as 4 Hex ASCII chars, MSD first.
*
*  Called by:  CLI command functions (only)
*  Entry args: uW = word to output
*  Returns:    void
*  Affects:    --
*/

void  putHexWord( uint16 uW )
{
    putHexDigit( (uint8) (uW >> 12) );
    putHexDigit( (uint8) (uW >> 8) );
    putHexDigit( (uint8) (uW >> 4) );
    putHexDigit( (uint8) (uW & 0xF) );
}


/*****
*  Output 32-bit longword as 8 Hex ASCII chars, MSD first.
*
*  Called by:  CLI command functions (only)
*  Entry args: uL = longword to output
*  Returns:    void
*  Affects:    --
*/

void  putHexLong( uint32 uL )
{
    uint8  count, digit;

    for ( count=0 ; count < 8 ; count++ )
    {
        digit = (uint8) (uL >> 28);
        putHexDigit( digit );
        uL = uL << 4;
    }
}


/*****
*   Function:   putDecimal
*   Called by:  CLI command functions and debug trace output functions
*
*   Purpose:    Outputs a 32-bit word as a signed decimal integer, up to 10 digits & sign,
*               right justified in the specified field, with leading zeros suppressed.
*               If the value is too big to fit into the specified minimum field size,
*               the field will be expanded to accommodate the number of digits (& sign).
*               Negative numbers are output with a 'minus' sign to the left of the first
*               non-zero digit. The field size includes the minus sign. Positive numbers
*               are output with space(s) to the left of the first digit, if the space(s)
*               can fit into the specified minimum field width.
*
*   Args:       (int32) lVal = signed longword to be converted and output
*               (uint8) bFieldSize = minimum number of character places (1..12)
*/
void  putDecimal( int32 lVal, uint8 bFieldSize )
{
    uint8    acDigit[12];     /* ASCII result string, acDigit[0] is LSD */
    int      i;
    int      iSignLoc = 0;
    uint8    c;
    uint8    yNegative = 0;
    uint8    yLeadingZero = 1;

    if ( bFieldSize > 12 )  bFieldSize = 12;
    if ( bFieldSize < 1 )  bFieldSize = 1;
    if ( lVal < 0 )  { yNegative = 1;  lVal = 0 - lVal; }   /* make value absolute */

    for ( i = 0;  i < 12;  i++ )      /* begin conversion with LSD */
    {
        c = '0' + (uint8)(lVal % 10);
        acDigit[i] = c;
        lVal = lVal / 10;
    }

    for ( i = 11;  i >= 0;  i-- )    /* begin processing with MSD */
    {
        c = acDigit[i];
        if ( i != 0 && c == '0' && yLeadingZero )    /* leave digit 0 (LSD) alone */
            acDigit[i] = ' ';

        if ( i == 0 || c != '0' )              /* found 1st significant digit (MSD) */
        {
            yLeadingZero = 0;
            if ( iSignLoc == 0 ) iSignLoc = i + 1;
        }
    }
    if ( yNegative ) acDigit[iSignLoc] = '-';     /* if +ve, there will be a SPACE */

    for ( i = 11;  i >= 0;  i-- )    /* begin output with MSD (or sign) */
    {
        c = acDigit[i];
        if ( i < bFieldSize || c != ' ' ) putch( c );
    }
}


#ifdef USE_MDD_FILE_SYSTEM
void  fputstr(char *str, FSFILE *stream)
{
    if (stream == NULL) putstr(str);  // => stdout
    else  FSfwrite(str, 1, strlen(str), stream);
}
#endif


/*________________________________________________________________________________________*\
*
*              N U M E R I C   C O N V E R S I O N   F U N C T I O N S
\*________________________________________________________________________________________*/
/*
*
*  Convert decimal ASCII char to 4-bit BCD value (returned as unsigned byte).
*
*  Entry args: c = decimal digit ASCII encoded
*  Returns:    0xFF if arg is not a decimal digit, else unsigned byte (0..9)
*  Affects:    --
*/
uint8  dectobin( char c )
{
    if ( c >= '0'  &&  c <= '9')
        return ( c - '0' );
    else
        return 0xFF ;
}


/*****
*  Convert decimal ASCII string, up to 5 digits, to 16-bit unsigned word.
*  There may be leading zeros, but there cannot be any leading white space.
*  Conversion is terminated when a non-decimal char is found, or when the
*  specified number of characters has been processed.
*
*  Entry args: (char *) pac = pointer to first char of decimal ASCII string
*              (int8)  bNdigs = number of characters to process (max. 5)
*  Returns:    Unsigned 16bit word ( 0 to 0xffff ).
*              If the target string (1st char) is non-numeric, returns 0.
*/
uint16  decatoi( char * pac, int8 bNdigs )
{
    uint8   ubDigit, ubCount;
    uint16  uwResult = 0;

    for ( ubCount = 0;  ubCount < bNdigs;  ubCount++ )
    {
        if ( (ubDigit = dectobin( *pac++ )) == 0xFF )
            break;
        uwResult = 10 * uwResult + ubDigit;
    }
    return  uwResult;
}


/*****
*  Convert Hexadecimal ASCII char (arg) to 4-bit value (returned as unsigned byte).
*
*  Called by:  various, background only
*  Entry args: c = Hex ASCII character
*  Returns:    0xFF if arg is not hex, else digit value as unsigned byte ( 0..0x0F )
*/

uint8  hexctobin( char c )
{
    if ( c >= '0'  &&  c <= '9')
        return ( c - '0' );
    else if ( c >= 'A'  &&  c <= 'F' )
        return ( c - 'A' + 10 );
    else if ( c >= 'a'  &&  c <= 'f' )
        return ( c - 'a' + 10 );
    else
        return 0xFF ;
}


/*****
*  Convert Hexadecimal ASCII string, up to 4 digits, to 16-bit unsigned word.
*  The string must be stored in the data RAM space.
*  There cannot be any leading white space.
*  Conversion is terminated when a non-Hex char is found.
*
*  Entry args: s = pointer to first char of hex string.
*  Returns:    Unsigned 16bit word ( 0 to 0xffff ).
*              If the target string (1st char) is non-Hex, returns 0.
*/

uint16  hexatoi( char * s )
{
    uint8   ubDigit, ubCount;
    uint16  uwResult = 0;

    for ( ubCount = 0;  ubCount < 4;  ubCount++ )
    {
        if ( (ubDigit = hexctobin( *s++ )) == 0xFF )
            break;
        uwResult = 16 * uwResult + ubDigit;
    }
    return  uwResult;
}


uint8  isHexDigit( char c )
{
    if ( hexctobin( c ) == 0xFF ) return 0;
    else  return 1;
}


/*
*   Convert a 16-bit unsigned word to hexadecimal string (4 hex ASCII digits).
*   The result string is NOT terminated.
*/
void  wordToHexStr( uint16 wVal, char *pcResult )
{
    pcResult[0] = hexitoc( wVal >> 12 );   /* MSB first */
    pcResult[1] = hexitoc( wVal >> 8 );
    pcResult[2] = hexitoc( wVal >> 4 );
    pcResult[3] = hexitoc( wVal );
}


/*
*   Convert integer value (4 LS bits) to hexadecimal ASCII character ('0' to 'F').
*   The input value is masked to use only the 4 LS bits.
*/
char  hexitoc( short wDigit )
{
    char  cRetVal;

    wDigit = wDigit & 0xF;
    if ( wDigit < 10 ) cRetVal = '0' + wDigit;
    else  cRetVal = 'A' + (wDigit - 10);

    return  cRetVal;
}


/*****
*  Convert Hexadecimal ASCII string, up to 8 digits, to 32-bit unsigned long word.
*  There cannot be any leading white space.
*  Conversion is terminated when a non-Hex char is found.
*
*  Entry args: s = pointer to first char of hex string.
*  Returns:    Unsigned 32 bit long word ( 0 to 0xffffffff ).
*              If the target string (1st char) is non-Hex, returns 0.
*/
uint32  long_hexatoi( char * s )
{
    uint8   ubDigit, ubCount;
    uint32  ulResult = (uint32) 0;

    for ( ubCount = 0;  ubCount < 8;  ubCount++ )
    {
        if ( (ubDigit = hexctobin( *s++ )) == 0xFF )
            break;
        ulResult = 16 * ulResult + ubDigit;
    }
    return  ulResult;
}
