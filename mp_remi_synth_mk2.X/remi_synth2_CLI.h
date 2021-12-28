/*________________________________________________________________________________________*\

    File:        remi_synth2_CLI.h

    Author:      M.J.Bauer

    Header file for PIC32 Audio Synth Test App. command-line interface (CLI).
\*________________________________________________________________________________________*/

#ifndef  _REMI_SYNTH2_CLI_H_
#define  _REMI_SYNTH2_CLI_H_

#include "../Common/system_def.h"
#include "HardwareProfile.h"
#include "console_cli.h"


extern   char  *g_AppTitleCLI;     // Title string output by "ver" command

// ----------  Application-Specific CLI commands  -----------
//
void    Cmnd_patch(int argCount, char * argValue[]);
void    Cmnd_config(int argCount, char * argValue[]);
void    Cmnd_preset(int argCount, char * argValue[]);
void    Cmnd_eeprom( int argCount, char * argValue[] );
void    Cmnd_info( int argCount, char * argValue[] );
void    Cmnd_mimon( int argCount, char * argValue[] );
void    Cmnd_sound( int argCount, char * argValue[] );
void    Cmnd_util(int argCount, char *argValue[]);
void    Cmnd_trace( int argCount, char * argValue[] );


#endif  // _REMI_SYNTH2_CLI_H_
