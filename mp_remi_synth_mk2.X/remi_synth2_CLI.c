/*=======================================================================================
 *
 * Module:      remi_synth2_CLI.c 
 * 
 * Overview:    Application-specific CLI command functions for the REMI mk2 synth.
 *
 *=======================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "remi_synth2_CLI.h"
#include "main_remi_synth2.h"
#include "wave_table_creator.h"


PRIVATE  void   WaveOscSoundTest(int freq, int duration); 
PRIVATE  void   MidiLoopBackTest();
PRIVATE  void   KeypadDriverTest();
PRIVATE  void   CoreCycleTimerTest();
PRIVATE  void   DisplayControllerTest();
PRIVATE  void   TestFixedPtBase2Exp();

extern   int32  g_TraceBuffer[][5];      // Debug usage only

// Settable parameters (using "set" command), for test & debug purposes...
extern   float  g_PressureGain;          // Expression/Pressure gain adjust (.1 ~ 10)
extern   float  g_FilterInputAtten;      // Filter input attenuator gain (.01 ~ 2.5)
extern   float  g_FilterOutputGain;      // Filter output atten/gain (0.1 ~ 25)
extern   float  g_NoiseFilterGain;       // Noise gen. gain adjustment (0.1 ~ 25)
extern   float  g_ReverbLoopTime_sec;    // Reverb param. (0.001 ~ 0.1 sec, typ. 0.04)
extern   float  g_ReverbDecayTime_sec;   // Reverb param. (1.0 ~ 2.0 sec, typ. 1.5)

extern   uint8  g_HandsetInfo[];         // REMI handset info from Sys.Ex. msg

// Variables used for MIDI IN monitor ...
extern   BOOL   g_MidiInputMonitorActive;
extern   uint8  g_MidiInputBuffer[];     // MIDI IN monitor buffer (circular FIFO)
extern   short  g_MidiInWriteIndex;      // MIDI IN monitor buffer write index
extern   int    g_MidiInputByteCount;    // MIDI IN monitor received byte count
extern   int    g_PressureMsgCount;      // Number of Pressure CC msg's Rx'd
extern   int    g_ModulationMsgCount;    // Number of Modulation CC msg's Rx'd
extern   int    g_SysExclusivMsgCount;   // Number of REMI Sys.Ex. msg's Rx'd

char    *g_AppTitleCLI;                  // Title string output by "ver" command
uint8    dummy_byte;


/*----------------------------------------------------------------------------------------
*                  A P P L I C A T I O N   C O M M A N D   T A B L E
*/
const  CmndTableEntry_t  AppCommands[] =
{
    //   Cmd Name      Attribute      Cmd Function
    //----------     -------------   -------------------
    {    "trace",      DBG_CMD,       Cmnd_trace      },
    {    "eeprom",     DBG_CMD,       Cmnd_eeprom     },
    {    "util",       GEN_CMD,       Cmnd_util       },
    {    "info",       APP_CMD,       Cmnd_info       },
    {    "config",     APP_CMD,       Cmnd_config     },
    {    "mimon",      APP_CMD,       Cmnd_mimon      },
    {    "preset",     APP_CMD,       Cmnd_preset     },
    {    "patch",      APP_CMD,       Cmnd_patch      },
    {    "sound",      APP_CMD,       Cmnd_sound      },
    {    "wav",        APP_CMD,       Cmnd_wav        },
    //---------------------------------------------------
    {    "$",          0,             NULL            }   // Dummy last entry
} ;


// Table of user-settable parameters, for use with the "set" command.
// These parameters are stored in data RAM, must be declared as global and type float.
// Default values are assigned at start-up. Intended mainly for development purposes.
//
const  UserSettableParameter_t  UserParam[] =
{
    // nickname      disp   (float *) &g_VarName      min,  max
    //----------------------------------------------------------------
    { "pressGain",   'r',  &g_PressureGain,           0.1,  25    },
    { "noiseGain",   'r',  &g_NoiseFilterGain,        0.1,  25    }, 
    { "filtAtten",   'r',  &g_FilterInputAtten,       0.01, 2.5   },
    { "filtGain",    'r',  &g_FilterOutputGain,       0.1,  25    },
    { "rvbLoop",     'r',  &g_ReverbLoopTime_sec,     0,    0.05  },
    { "rvbDecay",    'r',  &g_ReverbDecayTime_sec,    0.01, 2.0   },
    //----------------------------------------------------------------
    { "$",           'r',  NULL,                      0,    0     }  // end of table
};


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Note:  The "patch" command function is in source file "remi_synth2_engine.c".
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/*
 *   CLI command function:  Cmnd_config
 *
 *   The "config" command allows synth configuration parameters to be set.
 */
void  Cmnd_config(int argCount, char * argValue[])
{
    char   textBuf[100];
    bool   updateConfig = 0;
    bool   isCmdError = 0;
    int    arg = atoi(argValue[2]);

    if (argCount == 2 && argValue[1][0] == '?')   // show help
    {
        putstr( "View/set synth config. parameter(s) ...\n" );
        putstr( "Usage (1):  config {no arg}       |  list Config param's \n" );
        putstr( "Usage (2):  config <param> <arg>  |  set param value to <arg> \n" );
        putstr( "  ... where <param> = parameter mnemonic (3LA - see listing)\n" );
        putstr( "Example:    config mic 4          |  Set MIDI IN channel to 4 \n" );
		return;
    }
	
    if (argCount == 1)   // list current config param's
    {
        sprintf(textBuf, "mib | MIDI Baudrate: %d \n", g_Config.MidiInBaudrate);
        putstr("\t");  putstr(textBuf);
        sprintf(textBuf, "mim | MIDI IN Mode: %d ", g_Config.MidiInMode);
        putstr("\t");  putstr(textBuf);
        if (g_Config.MidiInMode == 2) putstr("(Omni-On-Mono) \n");
        else if (g_Config.MidiInMode == 4) putstr("(Omni-Off-Mono) \n");  
        else  putstr("(invalid) \n");
        sprintf(textBuf, "mic | MIDI IN Channel: %d \n", g_Config.MidiInChannel);
        putstr("\t");  putstr(textBuf);
        sprintf(textBuf, "mip | MIDI IN Pressure CC #: %d \n", g_Config.MidiInPressureCCnum);
        putstr("\t");  putstr(textBuf);
        sprintf(textBuf, "min | MIDI IN Modulation CC #: %d \n", g_Config.MidiInModulationCCnum);
        putstr("\t");  putstr(textBuf);
        
        sprintf(textBuf, "mom | MIDI OUT Mode: %d ", g_Config.MidiOutMode);
        putstr("\t");  putstr(textBuf);
        if (g_Config.MidiOutMode == 0) putstr("(Disabled) \n");
        if (g_Config.MidiOutMode == 1) putstr("(Omni-On-Poly) \n");
        if (g_Config.MidiOutMode == 2) putstr("(Omni-Off-Mono) \n");  
        if (g_Config.MidiOutMode == 3) putstr("(Omni-On-Poly) \n");
        if (g_Config.MidiOutMode == 4) putstr("(Omni-Off-Mono) \n");  
        sprintf(textBuf, "moc | MIDI OUT Channel: %d \n", g_Config.MidiOutChannel);
        putstr("\t");  putstr(textBuf);
        sprintf(textBuf, "mop | MIDI OUT Pressure CC #: %d \n", g_Config.MidiOutPressureCCnum);
        putstr("\t");  putstr(textBuf);
        sprintf(textBuf, "mon | MIDI OUT Modulation CC #: %d \n", 
                g_Config.MidiOutModulationCCnum);
        putstr("\t");  putstr(textBuf);
    
        sprintf(textBuf, "rva | Reverb Attenuator gain: %d %%\n", 
                g_Config.ReverbAtten_pc);
        putstr("\t");  putstr(textBuf);
        sprintf(textBuf, "rvm | Reverb Mix (wet/dry) ratio: %d %%\n", 
                g_Config.ReverbMix_pc);
        putstr("\t");  putstr(textBuf);
        
        sprintf(textBuf, "aco | Amplitude Ctrl Override: %d ", g_Config.AmpldControlOverride);
        putstr("\t");  putstr(textBuf);
        if (g_Config.AmpldControlOverride == 0) putstr("(Off) \n");
        else  putstr("(On) \n");

        return;
    }

    if (strmatch(argValue[1], "mib"))  // MIDI IN Baudrate
    {
        if (argCount >= 3 && (arg >= 100 && arg <= 57600))
        {
            g_Config.MidiInBaudrate = arg;
            updateConfig = 1;
        }
        else  isCmdError = 1;
    }    
    else if (strmatch(argValue[1], "mim"))  // MIDI IN Mode
    {
        if (argCount >= 3 && (arg == 2 || arg == 4))
        {
            g_Config.MidiInMode = arg;
            updateConfig = 1;
        }
        else  isCmdError = 1;
    }
    else if (strmatch(argValue[1], "mic"))  // MIDI IN Channel
    {
        if (argCount >= 3 && (arg >= 1 && arg <= 16))
        {
            g_Config.MidiInChannel = arg;
            updateConfig = 1;
        }
        else  isCmdError = 1;
    }
    else if (strmatch(argValue[1], "mip"))  // MIDI IN Pressure CC #
    {
        if (argCount >= 3 && (arg == 2 || arg == 7 || arg == 11))
        {
            g_Config.MidiInPressureCCnum = arg;
            updateConfig = 1;
        }
        else  isCmdError = 1;
    }
    else if (strmatch(argValue[1], "min"))  // MIDI IN Modulation CC #
    {
        if (argCount >= 3 && (arg >= 0 && arg < 32))
        {
            g_Config.MidiInModulationCCnum = arg;
            updateConfig = 1;
        }
        else  isCmdError = 1;
    }
    else if (strmatch(argValue[1], "mom"))  // MIDI OUT Mode
    {
        if (argCount >= 3 && arg <= 4)
        {
            g_Config.MidiOutMode = arg;
            updateConfig = 1;
        }
        else  isCmdError = 1;
    }
    else if (strmatch(argValue[1], "moc"))  // MIDI OUT Channel
    {
        if (argCount >= 3 && (arg >= 1 && arg <= 16))
        {
            g_Config.MidiOutChannel = arg;
            updateConfig = 1;
        }
        else  isCmdError = 1;
    }
    else if (strmatch(argValue[1], "mop"))  // MIDI OUT Pressure CC #
    {
        if (argCount >= 3 && (arg == 0 || arg == 2 || arg == 7 || arg == 11))
        {
            g_Config.MidiOutPressureCCnum = arg;
            updateConfig = 1;
        }
        else  isCmdError = 1;
    }
    else if (strmatch(argValue[1], "mon"))  // MIDI OUT Modulation CC #
    {
        if (argCount >= 3 && arg >= 1 && arg <= 16)
        {
            g_Config.MidiOutModulationCCnum = arg;
            updateConfig = 1;
        }
        else  isCmdError = 1;
    }
    else if (strmatch(argValue[1], "rva"))  // Reverb. loop attenuation (1..100 %)
    {
        if (argCount >= 3 && (arg >= 1 && arg <= 100))
        {
            g_Config.ReverbAtten_pc = arg;
            updateConfig = 1;
        }
        else  isCmdError = 1;
    }
    else if (strmatch(argValue[1], "rvm"))  // Reverb. wet/dry mix (0..100 %)
    {
        if (argCount >= 3 && (arg >= 0 && arg <= 100))
        {
            g_Config.ReverbMix_pc = arg;
            updateConfig = 1;
        }
        else  isCmdError = 1;
    }
    else if (strmatch(argValue[1], "aco"))  // Ampld Ctrl Override (0 | 1)
    {
        if (argCount >= 3 && (arg == 0 || arg == 1))
        {
            g_Config.AmpldControlOverride = arg;
            updateConfig = 1;
        }
        else  isCmdError = 1;
    }

    if (isCmdError)  putstr("! Invalid <arg> value \n");
    
    if (updateConfig)  
    {
        putstr("* Parameter updated.\n");
        StoreConfigData();  // commit new config setting
    }
}


/*
 *   CLI command function:  Cmnd_preset
 *
 *   The "preset" command allows selection or configuration of the active Preset.
 */
void  Cmnd_preset(int argCount, char * argValue[])
{
    char    textBuf[80];
    char   *patchName;
    char    option = tolower(argValue[1][1]);
    int     arg_n = atoi(argValue[1]);
    int     arg = atoi(argValue[2]);
    int     activePreset = g_Config.PresetLastSelected;  // 0..7
    int     i, preset, pr_idx, patch_ID, patch_idx = 0;

    if (argCount == 1 || *argValue[1] == '?')   // help wanted
    {
        putstr( "Select or configure one of the 8 Instrument Presets...    \n" );
        putstr( "Usage (1):  preset  <n>            | Select Preset (1..8) \n" );
        putstr( "Usage (2):  preset  <opt>  [arg]   | Configure Preset...  \n" );
        putstr( "  where <opt> = \n" );
        putstr( " -i  : View Preset configurations (alias -l : List ..)\n");
        putstr( " -p  : Set REMI synth Patch ID number (0..999) \n");
        putstr( " -n  : Set MIDI OUT program/voice Number (0..127) \n");
	    putstr( " -v  : Set Vibrato mode (O:off, E:fx.sw, M:mod'n, A:auto, D:detune)\n");
        putstr( " -t  : Set pitch Transpose, semitones (-24..+24) \n");
        ////
        return;
    }

    if (isdigit(*argValue[1]) && arg_n >= 0 && arg_n <= 8) 
    {
        InstrumentPresetSelect(arg_n);
        activePreset = g_Config.PresetLastSelected;  // 0..7
        patch_ID = g_Preset.Descr[activePreset].RemiSynthPatch;
        
        // Search table of predefined patches for patch_ID in the active Preset...
        for (i = 0;  i < GetNumberOfPatchesDefined();  i++)
        {
            if (g_PatchProgram[i].PatchNumber == patch_ID)  patch_idx = i;
        }
        
        if (patch_ID == 0)  patchName = g_Config.UserPatch.PatchName;  // User Patch
        else  patchName = (char *) g_PatchProgram[patch_idx].PatchName;
        
        if (activePreset == 0)  preset = 8;  else  preset = activePreset;  // for display
        
        sprintf(textBuf, "Active Preset: %d : Patch #%d %s \n", preset, patch_ID, patchName);  
        putstr(textBuf);
    }
    else  // Set Preset parameter
    {
        switch (option)
        {
        case 'p':  // set REMI synth patch number for activePreset
        {
            int  patchCount = GetNumberOfPatchesDefined();

            // Search table of defined patches for patchNum...
            for (i = 0;  i < patchCount;  i++)
            {
                if (g_PatchProgram[i].PatchNumber == arg)  break;
            }

            if (argCount == 3 && (arg == 0 || i < patchCount)) 
            {
                g_Preset.Descr[activePreset].RemiSynthPatch = arg;
                StorePresetData();
                InstrumentPresetSelect(activePreset);  // activate the new setting
            }
            else
            {
                putstr("! Undefined Patch ID number: ");
                putDecimal(arg, 1);
                putstr(" -- No change. \n");
            }
            break;
        }
        case 'n':  // set MIDI program/voice number
        {
            if (argCount == 3 && arg > 0 && arg < 128)
            {
                g_Preset.Descr[activePreset].MidiProgram = arg;
                StorePresetData();
                InstrumentPresetSelect(activePreset);  // activate the new setting
            }
            break;
        }
        case 'v':  // set remi synth Vibrato control mode
        {
            char  c1 = toupper(*argValue[2]);

            if (argCount == 3)
            {
                if (c1 == '0')  g_Preset.Descr[activePreset].VibratoMode = VIBRATO_DISABLED;
                if (c1 == 'E')  g_Preset.Descr[activePreset].VibratoMode = VIBRATO_BY_EFFECT_SW;
                if (c1 == 'M')  g_Preset.Descr[activePreset].VibratoMode = VIBRATO_BY_MODN_CC;
                if (c1 == 'A')  g_Preset.Descr[activePreset].VibratoMode = VIBRATO_AUTOMATIC;
                if (c1 == 'D')  g_Preset.Descr[activePreset].VibratoMode = VIBRATO_AUTO_ASYMM;
                StorePresetData();
                InstrumentPresetSelect(activePreset);  // activate the new setting
            }
            break;
        }
        case 't':  // set pitch transpose, semitones
        case 'o':  // alias (pitch offset)
        {
            if (argCount == 3 && arg >= -24  && arg <= 24)
            {
                g_Preset.Descr[activePreset].PitchTranspose = arg;
                StorePresetData();
            }
            break;
        }
        default:
        {
            option = 'i';  break;
        }
        } // end switch
    }

    if (option == 'i')   // List all Presets configurations
    {
        putstr("PRESET | Patch                   | MIDI Prgm | Vibrato   | Transpose   \n");
        putstr("```````````````````````````````````````````````````````````````````````\n");
            
        for (preset = 1;  preset <= 8;  preset++)   // List order: 1..8
        {
            if (preset == 8) pr_idx = 0;  else pr_idx = preset;  // wrap 8 -> 0
                    
            patch_ID = g_Preset.Descr[pr_idx].RemiSynthPatch;
            // Search table of defined patches for patch_ID in the indexed Preset...
            for (i = 0;  i < GetNumberOfPatchesDefined();  i++)
            {
                if (g_PatchProgram[i].PatchNumber == patch_ID)  break;
            }
            
            if (pr_idx == activePreset)  patch_idx = i;  // capture patch_idx for later

            if (patch_ID == 0) patchName = g_Config.UserPatch.PatchName;  // user patch
            else if (i < GetNumberOfPatchesDefined())    // found patch_ID in array
                patchName = (char *) g_PatchProgram[i].PatchName;
            else  patchName = "Undefined patch!";   // Unlikely to occur!

            sprintf(textBuf, "   %d   | %2d %-20s", preset, patch_ID, patchName);
            putstr(textBuf);

            sprintf(textBuf, " |    %3d    | ", g_Preset.Descr[pr_idx].MidiProgram);
            putstr(textBuf);

            if (g_Preset.Descr[pr_idx].VibratoMode == VIBRATO_BY_EFFECT_SW)
                putstr("FxSw CC86 | ");
            else if (g_Preset.Descr[pr_idx].VibratoMode == VIBRATO_BY_MODN_CC)
                putstr("Modn CC01 | ");
            else if (g_Preset.Descr[pr_idx].VibratoMode == VIBRATO_AUTOMATIC)
                putstr("Auto ramp | ");
            else if (g_Preset.Descr[pr_idx].VibratoMode == VIBRATO_AUTO_ASYMM)
                putstr("Auto Osc1 | ");   // vibrato on OSC1 only
            else  putstr("Disabled  | ");

            sprintf(textBuf, "%+4d \n", g_Preset.Descr[pr_idx].PitchTranspose);
            putstr(textBuf);
        }
        putstr("_______________________________________________________________________\n");
        
        if (activePreset == 0)  preset = 8;  else  preset = activePreset;
        patch_ID = g_PatchProgram[patch_idx].PatchNumber;
        patchName = (char *) g_PatchProgram[patch_idx].PatchName;
        sprintf(textBuf, "Active Preset: %d : Patch #%d %s \n", preset, patch_ID, patchName);  
        putstr(textBuf);
    }
}


/*
 *  CLI command function:  Cmnd_info
 *
 *  The "info" command shows "hard-wired" (not user-settable) synth configuration info
 *  plus handset configuration info (if available).
 * 
 *  System Exclusive 'PRESET' message format:
 *  `````````````````````````````````````````
 *  g_HandsetInfo[4]  =  handset FW_version[0]
 *  g_HandsetInfo[5]  =  handset FW_version[1]
 *  g_HandsetInfo[6]  =  handset FW_version[2]
 *  g_HandsetInfo[7]  =  handset SelfTestErrors
 *  g_HandsetInfo[8]  =  handset FingeringScheme
 *  g_HandsetInfo[9]  =  handset LegatoModeEnabled
 *  g_HandsetInfo[10] =  handset VelocitySenseEnabled
 * 
 */
void  Cmnd_info(int argCount, char * argValue[])
{
    char  textBuf[80];
    uint8  HWconfig = GetHardwareConfig();

    sprintf(textBuf, "REMI mk2 Synth. firmware version: %d.%d.%02d \n",
            g_FW_version[0], g_FW_version[1], g_FW_version[2]);
    putstr(textBuf);
    
    sprintf(textBuf, "HW config. jumper setting:  %d \n", HWconfig);
    putstr(textBuf);

    if (!isLCDModulePresent())
        putstr("LCD module not detected. Local UI disabled.\n");

#ifdef USE_LCD_CONTROLLER_ST7920 
    putstr("LCD controller driver: ST7920 \n");
#else  // assume USE_LCD_CONTROLLER_KS0108
    putstr("LCD controller driver: KS0108 \n");
#endif
    
    if (isHandsetConnected())
    {
        putstr("REMI mk2 handset connected...\n");

        if (g_HandsetInfo[0])   // A valid Sys.Ex. 'Preset' msg has been received
        {
            putstr("Handset information (from Preset Sys.Ex. msg):\n");
            sprintf(textBuf, "    Firmware version: %d.%d.%02d \n",
            g_HandsetInfo[4], g_HandsetInfo[5], g_HandsetInfo[6]);
            putstr(textBuf);
            putstr("    Error code: 0x");  putHexByte(g_HandsetInfo[7]);
            putNewLine();
            putstr("    Legato mode is ");
            if (g_HandsetInfo[9])  putstr("enabled.\n");
            else  putstr("disabled.\n");
            putstr("    Velocity sense is ");
            if (g_HandsetInfo[10])  putstr("enabled.\n");
            else  putstr("disabled.\n");
        }
        else  putstr("but handset config info N/A. (Select a Preset.)\n");
    }
    else putstr("REMI handset not detected.\n");
}


/*```````````````````````````````````````````````````````````````````````````````````````
 * CLI command function:  Cmnd_mimon
 *
 * Controls the MIDI-IN monitor routine.
 */
void  Cmnd_mimon( int argCount, char * argValue[] )
{
    static  uint32  MonitorStartTime;
    uint32  elapsedTime_ms;
    int     i;
    uint16  nbytes = 0;
    short   buffReadIndex = 0, col = 0;
    char    option = tolower(argValue[1][1]);
    
    if (argCount == 1 || *argValue[1] == '?' )  // help
    {
        putstr( "MIDI Input Monitor -- diagnostic utility \n" );
        putstr( "Usage:  mimon  <opt>  [n] \n" );
        putstr( "where <opt> = \n" );
        putstr( "  -a : Activate the monitor \n" );
        putstr( "  -d : De-activate the monitor and display stats \n" );
        putstr( "  -n : Output last <n> buffer entries (n <= 512) \n" );
        return;
    }
    
    if (option == 'a' || option == 'e')  // Activate/enable the monitor
    {
        putstr("MIDI IN monitor activated.\n");
        g_MidiInputByteCount = 0;
        g_PressureMsgCount = 0;          // Clear Pressure CC msg's Rx'd
        g_ModulationMsgCount = 0;        // Clear Modulation CC msg's Rx'd
        g_SysExclusivMsgCount = 0;       // Clear Sys.Ex. IDENT msg's Rx'd
        g_MidiInputMonitorActive = TRUE;
        MonitorStartTime = milliseconds();
    }
    else if (option == 'd')  // De-activate the monitor and display stats
    {
        g_MidiInputMonitorActive = FALSE;
        elapsedTime_ms = milliseconds() - MonitorStartTime;
        
        putstr("Monitor elapsed time:   ");
        putDecimal((elapsedTime_ms + 500) / 1000, 3);
        putstr(" sec.\n");
        
        putstr("MIDI IN byte count:   ");
        putDecimal(g_MidiInputByteCount, 5);
        putNewLine();
        
        putstr("Pressure msg count:   ");
        putDecimal(g_PressureMsgCount, 5);
        putNewLine();
        
        putstr("Modulation msg count: ");
        putDecimal(g_ModulationMsgCount, 5);
        putNewLine();
        
        putstr("Sys. Excl. msg count: ");
        putDecimal(g_SysExclusivMsgCount, 5); 
        putNewLine();
    }
    else if (option == 'n')  // Output last N buffer entries (N <= 512)
    {
        if (argCount == 2)  nbytes = 256;  // default: dump half of buffer
        else  nbytes = atoi(argValue[2]);
        if (nbytes > MIDI_MON_BUFFER_SIZE) nbytes = MIDI_MON_BUFFER_SIZE;  // 512
        if (g_MidiInputByteCount < nbytes)  nbytes = g_MidiInputByteCount;
        
        putstr("Last ");  putDecimal(nbytes, 1);
        putstr(" bytes of ");  putDecimal(g_MidiInputByteCount, 1);
        putstr(" received: \n");
        
        if (g_MidiInputByteCount == 0)  putstr("Monitor activated? \n");
        
        buffReadIndex = g_MidiInWriteIndex - nbytes;
        if (buffReadIndex < 0)  buffReadIndex += MIDI_MON_BUFFER_SIZE;
        
        for (i = 0 ; i < nbytes ; i++)
        {
            putHexByte(g_MidiInputBuffer[buffReadIndex++]);
            putch(' ');
            if (buffReadIndex == MIDI_MON_BUFFER_SIZE)  
                buffReadIndex = 0;   // wrap
            if (++col == 16)  { col = 0;  putNewLine(); }   // new line
        }
        putNewLine();
    }
}


/*```````````````````````````````````````````````````````````````````````````````````````
 * CLI command function:  Cmnd_eeprom
 *
 * Examine or erase EEPROM contents. Also tests EEPROM hardware interface.
 */
void  Cmnd_eeprom(int argCount, char * argValue[])
{
    uint8   blockNum = 0;
    uint8   buffer[20];
    uint8   eebyte;
    int     offset, row, col, count;
    char    option = tolower(argValue[1][1]);

    if (argCount == 1 || *argValue[1] == '?' )  // help
    {
        putstr( "Usage:  eeprom  <opt>  <block> \n" );
        putstr( "where <opt> = \n" );
        putstr( "  -d : Dump  EEPROM block (0..3) \n" );
        putstr( "  -e : Erase EEPROM block (0..3) \n" );
        putstr( "<!> Re-boot required after 'eeprom -e' \n" );
        return;
    }

    if (EepromReadData(buffer, 0, 0, 16) != 16)
    {
        putstr("! Error: EEPROM read access failed. \n");
        return;
    }

    for (count = 0;  count < 16;  count++)
    {
        buffer[count] = 0xFF;
    }

    if (argCount > 2)  blockNum = atoi(argValue[2]) & 3;

    if (option == 'd')  // Dump block contents
    {
        putstr("EEPROM block #");
        putHexDigit(blockNum);
        putstr(" = \n");

        for (offset = 0, row = 0;  row < 16;  row++, offset += 16)
        {
            EepromReadData((uint8 *) buffer, blockNum, offset, 16);
            putHexByte(offset);
            putch(':');

            for (col = 0;  col < 16;  col++)        // Hex dump
            {
                if ((col % 4) == 0)  putch(' ');
                putHexByte(buffer[col]);
            }
            putch(' ');

            for (col = 0;  col < 16;  col++)        // ASCII dump
            {
                eebyte = buffer[col];
                if (isprint(eebyte))  putch(eebyte);
                else  putch('.');
            }
            putNewLine();
        }
    }

    if (option == 'e')  // Erase block contents
    {
        putstr("EEPROM block #");
        putHexDigit(blockNum);

        for (offset = 0, count = 0;  count < 256;  count += 16)
        {
            if (EepromWriteData(buffer, blockNum, offset, 16) == ERROR)
                break;
            offset += 16;
        }

        if (count == 256) putstr(" erased OK.\n");
        else  putstr(" erase failed.\n");
    }
}


/*
 *   CLI command function:  Cmnd_sound
 *
 *   The "sound" command initiates a sound generated by the synth engine.
 *   (See usage text for details).
 */
void  Cmnd_sound(int argCount, char * argValue[])
{
    static  bool    init_done = 0;
    static  uint32  noteEndTime;
    static  uint8   noteNumber = 69;    // default (A4-440Hz)
    static  uint16  duration = 2000;    // default (ms)
    static  uint8   vibratoMode = 0;    // default (off))
    
    int  arg1 = atoi(argValue[1]);
    int  arg2 = atoi(argValue[2]);
    int  arg3 = atoi(argValue[3]);
    
    if ( argCount == 2 && *argValue[1] == '?' )  // help wanted
    {
        putstr( "Start or stop a sound generated by the REMI synth engine. \n" );
        putstr( "On the first 'sound' after power-on/reset, the active patch is set  \n" );
        putstr( "to the 'Test Patch' (ID 90) where OSC1 wave-form is a sine-wave and \n" );
        putstr( "audio output level is controlled by the amplidude envelope shaper.  \n" );
        putstr( "The active patch param's may be modified using the 'patch' command. \n" );
        putstr( "````````````````````````````````````````````````````````````````````\n" );
        putstr( "Usage:  sound  [note] [durn] [vib] \n" );
        putstr( "where ... \n" );
        putstr( "  <note> = MIDI note number (12..108,  0 => Note-Off) \n" );
        putstr( "  <durn> = duration of note, seconds (0 => continuous) \n" );
        putstr( "  <vib>  = set vibrato mode (0:off, 3:auto-ramp, etc.) \n" );
        putstr( "If no note or durn value given, use previous non-zero value(s). \n" );
        putstr( "If durn is 0, sound continuously.  If note is 0, stop sound. \n" );
        return;
    }
    
    if (argCount == 2 && arg1 == 0)
    {
        RemiSynthNoteOff(noteNumber);
        return;
    }
    
    if (argCount >= 2)  noteNumber = arg1 & 0x7F;
    if (argCount >= 3 && arg2 > 0)  duration = arg2 * 1000;  // ms
    if (argCount >= 4)  vibratoMode = arg3 & 7;
    
    if (!init_done)  // Power-on/reset initialization -- load default patch
    {
        RemiSynthPatchSelect(90);  // "Test Patch" (Osc1: sine wave;  Osc2: sawtooth)
        init_done = 1;
    }
    
    noteEndTime = milliseconds() + duration;  // msec
    
    SetVibratoModeTemp(vibratoMode);     // Override Preset
    RemiSynthNoteOn(noteNumber, 64);     // velocity value ignored
    RemiSynthExpression(4000);           // full scale is 16000
    
    if (argCount >= 3 && arg2 == 0)  return;  // sound indefinitely
    
    while (milliseconds() < noteEndTime)
    {
        BackgroundTaskExec();
        
        if (UART1_RxDataAvail())    
        {
            dummy_byte = UART1_getch();  // read MIDI IN data byte
            Delay10us(5);                // simulated process delay
        }
    }
    
    RemiSynthNoteOff(noteNumber);
}


/*```````````````````````````````````````````````````````````````````````````````````````
 *   Function called by "diag" command.
 * 
 *   NB:  All diag command options (except '-x') activate Diagnostic Mode
 *        which causes the REMI synth process (B/G task) to be suspended.
 *        Command "diag -x" cancels Diagnostic Mode and re-activates the synth
 *        process. A time-out (default 30 mins) is imposed on Diagnostic Mode.
 */
void  DiagnosticCommandExec(int argCount, char * argValue[])
{
//  static bool  diagEnabled;
    static bool  outputLevelSet;
    char   option = tolower(argValue[1][1]);
    char   opt_c2 = tolower(argValue[1][2]);

    if ( argCount == 1 || *argValue[1] == '?' )   // help wanted  Osc1FreqDividerGet()
    {
        putstr( "REMI mk2 synth module :: low-level diagnostics \n" );
        putstr( "`````````````````````````````````````````````` \n" );
        putstr( "Usage:  diag  <option>  [arg's] ... \n" );
        putstr( "Options:  \n" );
        putstr( " -a  :  Audio output level (0..100), range (0..3) \n");
        putstr( " -c  :  Core cycle timer test \n");
        putstr( " -d  :  Display controller/driver test \n");
        putstr( " -k  :  Keypad/button test \n");
//      putstr( " -m  :  MIDI I/O loop-back test \n");
        putstr( " -p  :  Pitch bend test (+/-0..1023) \n");
//      putstr( " -t  :  Set timeout for diag mode (mins) \n");
        putstr( " -u  :  Show UART error counts, then clear. \n");
//      putstr( " -x  :  Exit diagnostic mode. \n");
        putstr( " <!> Most options (except -x) activate diagnostic mode.\n");
        ////
        return;
    }
#ifdef SUPPORT_DIAG_MODE    
    if (option != 'x')  
    {
        EnableDiagnosticMode();
        diagEnabled = 1;
    }
#endif
    
    switch (option)
    {
    case 'a':  // Set audio output level (%), temporarily
    {
        int   level_pc = atoi(argValue[2]);  // percent
        fixed_t level_pu = IntToFixedPt(level_pc) / 100;  // per unit

        SetAudioOutputLevel(level_pu);
        outputLevelSet = 1;
        break;
    }
    case 'c':  // Core cycle timer test
    {
        CoreCycleTimerTest();
        break;
    }
    case 'd':  // Display controller/driver  test
    {
        DisplayControllerTest();
        break;
    }
    case 'k':  // Keypad/button test
    {
        KeypadDriverTest();
        break;
    }
    case 'm':  // MIDI comm's loop-back test (*incomplete*)
    {
        MidiLoopBackTest();  
//      CancelDiagnosticMode();
//      diagEnabled = 0;
        outputLevelSet = 0;
        break;
    }
    case 'p':  // Pitch bend test
    {
        float   arg = atof(argValue[2]);

        RemiSynthPitchBend((int) arg);
        break;
    }
/*    
    case 'u':  // UART errors
    {
        putstr("UART #1 error count: ");  
        putDecimal(UART1_getErrorCount(), 5);
        putNewLine();
        putstr("UART #2 error count: ");  
        putDecimal(UART2_getErrorCount(), 5);
        putNewLine();
        CancelDiagnosticMode();
        diagEnabled = 0;
        break;
    }
*/
#ifdef SUPPORT_DIAG_MODE      
    case 't':  // Set timeout for diagnostic mode;  arg = timeout (mins)
    {
        int   arg = atoi(argValue[2]);  // minutes

        if (argCount >= 3 && arg > 0)
        {
            g_DiagnosticModeTimeout = arg;
        }
        break;
    }
    case 'x':  // Exit (cancel) diagnostic mode
    {
        CancelDiagnosticMode();
        diagEnabled = 0;
        outputLevelSet = 0;
        putstr("* Normal synth operation resumed.\n");
        break;
    }
#endif    
    
    default:  break;
    } // end switch

#ifdef SUPPORT_DIAG_MODE        
    if (diagEnabled) 
    {
        putstr("! The 'Synth Process' routine is suspended.\n");
        putstr("  Use 'diag -x' to resume normal operation.\n");
    }
#endif
}


/*`````````````
 * Function:  Command option "diag -c"
 *
 * Processor core cycle timer test (reading COUNT register).
 */
PRIVATE  void   CoreCycleTimerTest()
{
    uint32  count;

    TIMER2_IRQ_DISABLE();  // Stop IRQ's from audio ISR during test

    while ((milliseconds() & 0xF) != 15)  {/* Wait for ms timer pending... */}
    while ((milliseconds() & 0xF) != 0)   {/* ... rollover */}
    count = ReadCoreCountReg();
    while ((milliseconds() & 0xF) != 10)  {/* Count for 10 milliseconds */}
    count = ReadCoreCountReg() - count;

    putstr("Cycle count during 10ms: ");
    putDecimal(count, 1);
    putstr(" (400000 expected)\n");
/*
    while (1)  // Generate square wave, T = 2.5us (approx.)
    {
        TESTPOINT_RC14_SET_HI();
        Delay_Nx25ns(40);   
        TESTPOINT_RC14_SET_LO();
        Delay_Nx25ns(40);
    }
*/    
    TIMER2_IRQ_ENABLE();
}


/*`````````````
 * Function:  Command option "diag -d"
 *
 * Diagnostics for GLCD controller/driver.
 */
PRIVATE  void  DisplayControllerTest()
{
    uint8  statusByte;
    char   c = 0;
    
    LCD_Reset();
    Delay_ms(50);
    putstr("LCD controller was reset.\n");

    LCD_CS1_ON();
    statusByte = LCD_ReadStatus();
    LCD_CS1_OFF();
    putstr("Status byte read, CS1 asserted: $");
    putHexByte(statusByte);
    putNewLine();
    
    LCD_CS2_ON();
    statusByte = LCD_ReadStatus();
    LCD_CS2_OFF();
    putstr("Status byte read, CS2 asserted: $");
    putHexByte(statusByte);
    putNewLine();
    
    LCD_CS1_ON();
    LCD_WriteCommand(LCD_SET_DISPLAY_ON);
    LCD_CS1_OFF();
    LCD_CS2_ON();
    LCD_WriteCommand(LCD_SET_DISPLAY_ON);
    LCD_CS2_OFF();
    Delay_ms(50);
    putstr("Display ON command issued.\n");
    
    LCD_CS1_ON();
    statusByte = LCD_ReadStatus();
    LCD_CS1_OFF();
    putstr("Status byte read, CS1 asserted: $");
    putHexByte(statusByte);
    putNewLine();
    
    LCD_CS2_ON();
    statusByte = LCD_ReadStatus();
    LCD_CS2_OFF();
    putstr("Status byte read, CS2 asserted: $");
    putHexByte(statusByte);
    putNewLine();
    
    LCD_Init();
    LCD_ClearGDRAM();
    LCD_Test();
    
    putstr("A test pattern should be shown on the LCD screen.\n");
    putstr("Hit [Esc] to exit test...\n");
    
    while (c != ASCII_ESC)
    {
        if (kbhit()) c = getch();
    }
    putNewLine();
    
/*    
    putstr("ReadStatus(CS1)/WriteData(CS2) loop test running...\n");
    putstr("Check signal timing on 'scope. Trigger on CS1 or CS2. \n");
    putstr("Reset board to exit. \n");

    TIMER2_IRQ_DISABLE();
    
    while (1)
    {
        LCD_CS1_ON();
        Delay_Nx25ns(40);
        LCD_CS1_OFF();
        Delay_Nx25ns(40);

        LCD_CS1_ON();
        LCD_ReadStatus();
        LCD_CS1_OFF();
        
        LCD_CS2_ON();
        LCD_WriteData(0xAA);
        LCD_CS2_OFF();
    }
*/
}


/*`````````````
 * Function:  Command option "diag -k"
 *
 * Real-time diagnostic for button input service.
 */
PRIVATE  void  KeypadDriverTest()
{
    char  c = 0;

    putstr("Press a key on the keypad to test it. \n");
    putstr("Press [Esc] on terminal to exit test. \n\n");

    while (c != ASCII_ESC)
    {
        BackgroundTaskExec();  // Button input service
        
        if (KeyHit())  putch(GetKey());  // GUI button hit

        if (kbhit())  c = getch();  // Console key hit
    }

    putNewLine();
}


/*````````````````  
 * Function:  Command option "diag -m"    *** todo ***
 *
 * MIDI comm's loop-back test.  Connect TX output to RX input.
 * 
 * Function transmits continuous System Exclusive messages (REMI IDENT, 4 bytes)
 * at intervals of 2.0ms. Received messages are checked for data integrity.
 * Normal synth operation is disabled while the loop-back test is running.
 */
PRIVATE  void  MidiLoopBackTest()
{
    static  uint32  start_time;
    char    c;
    char    commsError = 0;

    putstr("Press [Esc] to exit test... \n\n");

    while (c != ASCII_ESC /* && g_DiagnosticModeActive */)
    {
        start_time = milliseconds();
        while (milliseconds() - start_time < 50)  // Delay 50ms
        {
            BackgroundTaskExec();
			;
			;
        }
        
 		;
		;

        if (kbhit()) c = getch();
    }
    putNewLine();
    
//  if (!g_DiagnosticModeActive) putstr("* Diag mode timer expired.\n");
//  putstr("* Diag mode cancelled.\n");
//  CancelDiagnosticMode();
}


/*`````````````````````````````````````````````````````````````````````````````````````````````````
 *   CLI command function:  Cmnd_trace
 *
 *   The "trace" command outputs the debug trace buffer.
 *   Contents of the trace buffer depends on application-specific debug code.
 */
void  Cmnd_trace(int argCount, char *argValue[])
{
    char   textBuf[100];
    int    line;
    float  var1, var2, var3, var4;
    
    if (!SuperUserAccess()) return;

    if (*argValue[1] == '?')   // help wanted
    {
        putstr( "Usage:  trace \n" );
        putstr( "Dumps the trace buffer. \n" );
        ////
        return;
    }

    putstr("   T ms    final  smoothed  level(13b) \n");

    for (line = 0;  line < 100;  line++)
    {
        var1 = FixedToFloat(g_TraceBuffer[line][1]);  // final ampld
        var2 = FixedToFloat(g_TraceBuffer[line][2]);  // smoothed ampld

        sprintf(textBuf, "%7d  %7.3f  %7.3f  %7d \n",
                g_TraceBuffer[line][0], var1, var2, g_TraceBuffer[line][3]);
        putstr(textBuf);
    }
}


/*
 *   CLI command function:  Cmnd_util
 *
 *   The "util" command executes a specified utility.
 */
void  Cmnd_util(int argCount, char *argValue[])
{
    char   textbuf[120];
    char   option = tolower(argValue[1][1]);
    
    if (!SuperUserAccess()) return;

    if (argCount == 1 || *argValue[1] == '?')   // help wanted
    {
        putstr( "Run a specified Utility...   \n" );
        putstr( "Usage:  util  <opt>  [args] \n" );
        putstr( "<opt> \n" );
        putstr( "  -a : Test fixed-pt (arg1) x integer (arg2) / 1024 \n");
        putstr( "  -b : Test Base2exp() function (no arg's) \n");
//      putstr( "  -c : ... \n");
            ;
            ;
            ;
        return;
    }

    switch (option)
    {
    case 'a':
    {
        float    arg1 = atof(argValue[2]);
        int      arg2 = atoi(argValue[3]);
        fixed_t  mp_cand = FloatToFixed(arg1);
        fixed_t  prod = (mp_cand * arg2) >> 10;
        
        sprintf(textbuf, "(fixed_t) %f x (int) %d / 1024 = %f \n", 
                FixedToFloat(mp_cand),  arg2,  FixedToFloat(prod));
        putstr(textbuf);
        break;
    }
    case 'b':
    {
        TestFixedPtBase2Exp();
        break;
    }
	default: break;
    }
}


PRIVATE  void  TestFixedPtBase2Exp()
{
    static  float  test_val[] =
            { -1.0, -0.999, -0.5, -0.001,
                 0, 0.0001, 0.0005, 0.001, 0.0015, 0.002, 0.5, 0.9995, 1.0 };

    fixed_t  x, y;
    char     txtBuf[100];
    int      i;

    for (i = 0;  i < ARRAY_SIZE(test_val);  i++)
    {
        x = FloatToFixed(test_val[i]);
        y = Base2Exp(x);
        sprintf(txtBuf, "    x = %8.5f,  y = %8.5f \n", FixedToFloat(x), FixedToFloat(y));
        putstr(txtBuf);
    }
}


#ifdef USE_WIRELESS_MODULE  // future option
/*```````````````````````````````````````````````````````````````````````````````````````
 * CLI command function:  Cmnd_mrf
 *
 * Diagnostic utility functions to support wireless module MRF24J40MA.
 */
void  Cmnd_mrf(int argCount, char * argVal[])
{
    static  rx_info_t  *rx_inf;   // pointer to rx_info
    static  uint16  destNodeAddr = 0x6901;  // default dest. node
    static  char    tx_msg[64];
    static  uint16  pktSeqNum = 0;
    char    textBuf[80];
    char    opt_c1, opt_c2 = 0;
    int     arg1, arg2;
    int     i, rx_len = 0;   // size of rx packet payload
    uint8  *rx_dat;

    if (argCount == 1 || *argVal[1] == '?' )  // help
    {
        putstr( "Diagnostic utility for MRF24J40 wireless module.\n" );
        putstr( "Usage:  mrf  <option>  [arg1] [arg2] ... \n" );
        putstr( "options \n" );
        putstr( "  -z  : Initialize the device (reset & configure) \n" );
/*        
        putstr( "  -rr : Read MRF24J40 register (arg1 = short.addr, hex) \n" );
        putstr( "  -wr : Write MRF24J40 reg. (arg2:data = decimal or hex^) \n" );
        putstr( "        ^ Use $ prefix for hex data (eg. $0F) \n" );
*/
        putstr( "  -p  : Get/Set wireless PAN ID [arg1 = PAN.id, hex] \n" );
        putstr( "  -sn : Get/Set source node address [arg1 = addr, hex] \n" );
        putstr( "  -tx : Transmit test packet [arg1 = destin.node, hex] \n" );
        putstr( "  -rx : View last received packet (hex bytes) \n" );
        return;
    }

    if (argVal[1][0] == '-')
    {
        opt_c1 = tolower(argVal[1][1]);
        opt_c2 = tolower(argVal[1][2]);
    }
    
    if (opt_c1 == 'z')    // Initialize the device (reset & configure)
    {
        // todo: arg1 = RF channel (11..26)
        if (WirelessModuleInit() == ERROR)
        {
            putstr("! MRF24J40 module not detected.\n");
        }
        pktSeqNum = 0;
    }
    else if (opt_c1 == 'p')    // Get/Set PAN ID  [arg1 = PAN.id, hex]
    {
        if (argCount >= 3)
        {
            arg1 = hexatoi(argVal[2]);
            if (arg1 > 0) 
            {
                g_Wireless_PAN_ID = arg1;
                MRF_SetPANID(g_Wireless_PAN_ID);
            }
        }
        g_Wireless_PAN_ID = MRF_GetPANID();
        putstr("PAN ID (from RF module): $");
        putHexWord(g_Wireless_PAN_ID);
        putNewLine();
    }
    else if (opt_c1 == 's')    // Get/Set local node address [arg1]
    {
        if (argCount >= 3)
        {
            arg1 = hexatoi(argVal[2]);
            if (arg1 > 0) 
            {
                g_WirelessNodeAddr = arg1;
                MRF_SetNodeAddr(g_WirelessNodeAddr);
            }
        }
        g_WirelessNodeAddr = MRF_GetNodeAddr();
        putstr("Node Addr (from RF module): $");
        putHexWord(g_WirelessNodeAddr);
        putNewLine();
    }
    else if (opt_c1 == 'r' && opt_c2 == 'x')  // Dump data from last Rx packet
    {
        rx_inf = MRF_RxInfoGet();
        rx_len = MRF_RxDataLength();
        rx_dat = (uint8 *) &rx_inf->rx_data[0];

        if (rx_len <= 64)
        {
            putstr("\n  Rx Data: ");
            for (i = 0;  i < rx_len;  i++)
            {
                putHexByte(*rx_dat++);  putch(' ');
            }
            putstr("\n  Pkt len: ");
            putDecimal(rx_len, 1); 
            putstr(" bytes \n");

            putstr("  LQ: ");  
            putDecimal(rx_inf->lqi / 25, 1);   // Link Quality Index
            putstr("/10 | ");
            putstr("RSS: ");  
            putDecimal(rx_inf->rssi / 25, 1);  // Rx Signal Strength Index
            putstr("/10 \n");
        }
    }
    else if (opt_c1 == 't' && opt_c2 == 'x')  // Transmit test packet [arg1 = dest.node]
    {
        if (argCount >= 3)
        {
            arg1 = hexatoi(argVal[2]);
            if (arg1 > 0)  destNodeAddr = arg1;
        }
        pktSeqNum++;
        wordToHexStr(pktSeqNum, textBuf);
        textBuf[4] = 0;  // term.char
        strcpy(tx_msg, "Test Packet #");
        strcat(tx_msg, textBuf);
        MRF_TransmitData(destNodeAddr, tx_msg, strlen(tx_msg));
    }
    // 
    // ...to be continued...
    //
    else  putstr("! What? \n");  // Cmd option undefined
}

#else
void  Cmnd_mrf(int argCount, char * argVal[])
{
	putstr("! Wireless comm's not supported in this firmware version.\n");
}

#endif  // USE_WIRELESS_MODULE


/*```````````````````````````````````````````````````````````````````````````````````````
 *   Function called by "watch" command function...
 *   Variables to be "watched" are output on a single line (no newline).
 */
void  WatchCommandExec(void)
{
    char   txtbuf[100];
    
    float  exprsn = FixedToFloat(GetExpressionLevel());  // Breath pressure (CC02)
    float  moduln = FixedToFloat(GetModulationLevel());  // CC01 (normalized)
    
    sprintf(txtbuf, "  MIDI CC02: %5.3f | CC01: %5.3f ", exprsn, moduln);  
    putstr(txtbuf);
}


/*```````````````````````````````````````````````````````````````````````````````````````
 *   Function called by "default" command function...
 *   Restores "factory default" values to persistent data (in EEPROM).
 */
void  DefaultPersistentData(void)
{
    DefaultConfigData();
    DefaultPresetData();
}


/*```````````````````````````````````````````````````````````````````````````````````````
 *   Function called by "commit" command...
 * 
 *   Where global param's are stored in NV memory, stored values are updated
 *   so that the new values become power-on/reset defaults.
 */
void  CommitPersistentParams()
{
    g_Config.PressureGain = g_PressureGain;
    g_Config.FilterInputAtten = g_FilterInputAtten;
    g_Config.FilterOutputGain = g_FilterOutputGain;
    g_Config.NoiseFilterGain = g_NoiseFilterGain;
        
    StoreConfigData();
}


/*```````````````````````````````````````````````````````````````````````````````````````
 *   Function called by "set" command whenever a global synth parameter is updated.
 * 
 *   Performs whatever actions are necessary following a synth parameter change;
 *   typically, corresponding working variables must be updated.
 */
void  ActivateSetParamValues(void)
{
    RemiSynthPrepare();
}

