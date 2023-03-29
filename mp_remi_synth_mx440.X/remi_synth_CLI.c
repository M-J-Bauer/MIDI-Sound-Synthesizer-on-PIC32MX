/*=======================================================================================
 *
 * Module:      remi_synth_CLI.c 
 * 
 * Overview:    Application-specific CLI command functions for the REMI synth's
 *
 *=======================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "remi_synth_CLI.h"
#include "remi_synth_main.h"
#include "remi_synth_def.h"
#include "wave_table_creator.h"

PRIVATE  void   PrintWaveTableInfo(unsigned oscNum);
PRIVATE  void   DumpActivePatchParams();
PRIVATE  void   SetPatchParameter(char *paramAbbr, int paramVal);
PRIVATE  void   WaveOscSoundTest(int freq, int duration); 
PRIVATE  void   CoreCycleTimerTest();
PRIVATE  void   DisplayControllerTest();
PRIVATE  void   TestFixedPtBase2Exp();

extern  uint8  g_HandsetInfo[];          // REMI handset info from Sys.Ex. msg
extern  int32  g_TraceBuffer[][5];       // Debug usage only
extern  uint32 g_TaskCallFrequency;      // Debug usage only

extern  volatile  bool v_SynthEnable;    // Signal to enable synth engine
extern  volatile  uint32 v_ISRexecTime;  // ISR execution time (core cycle count)

// Variables used for MIDI IN monitor ...
extern  BOOL   g_MidiInputMonitorActive;
extern  uint8  g_MidiInputBuffer[];      // MIDI IN monitor buffer (circular FIFO)
extern  short  g_MidiInWriteIndex;       // MIDI IN monitor buffer write index
extern  int    g_MidiInputByteCount;     // MIDI IN monitor received byte count
extern  int    g_PressureMsgCount;       // Number of Pressure CC msg's Rx'd
extern  int    g_ModulationMsgCount;     // Number of Modulation CC msg's Rx'd
extern  int    g_SysExclusivMsgCount;    // Number of REMI Sys.Ex. msg's Rx'd

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
    { "noiseGain",   'r',  &g_NoiseFilterGain,        0.1,  25    }, 
    { "filtAtten",   'r',  &g_FilterInputAtten,       0.01, 2.5   },
    { "filtGain",    'r',  &g_FilterOutputGain,       0.1,  25    },
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
    static char *pitchBendModeName[] = 
            { "Disabled", "MIDI Pitch-Bend", "MIDI Exprn CC", "Analog CV" };
    char   textBuf[100];
    bool   updateConfig = 0;
    bool   isCmdError = 0;
    int    arg;

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
        sprintf(textBuf, "mib | MIDI IN Baudrate: %d \n", g_Config.MidiInBaudrate);
        putstr("\t");  putstr(textBuf);
        sprintf(textBuf, "mim | MIDI IN Mode: %d ", g_Config.MidiInMode);
        putstr("\t");  putstr(textBuf);
        if (g_Config.MidiInMode == 2) putstr("= Omni-On-Mono \n");
        else if (g_Config.MidiInMode == 4) putstr("= Omni-Off-Mono \n");  
        else  putstr("(invalid) \n");
        sprintf(textBuf, "mic | MIDI IN Channel: %d \n", g_Config.MidiInChannel);
        putstr("\t");  putstr(textBuf);
        sprintf(textBuf, "mix | MIDI IN Expression CC #: %d \n", g_Config.MidiInExpressionCCnum);
        putstr("\t");  putstr(textBuf);
        
        sprintf(textBuf, "moe | MIDI OUT Enabled:  %d ", g_Config.MidiOutEnabled);
        putstr("\t");  putstr(textBuf);
        if (g_Config.MidiOutEnabled == 0) putstr("= Disabled \n");
        sprintf(textBuf, "moc | MIDI OUT Channel:  %d \n", g_Config.MidiOutChannel);
        putstr("\t");  putstr(textBuf);
        sprintf(textBuf, "mox | MIDI OUT Expression CC #: %d \n", g_Config.MidiOutExpressionCCnum);
        putstr("\t");  putstr(textBuf);
        sprintf(textBuf, "mom | MIDI OUT Modulation Enab: %d \n", g_Config.MidiOutModnEnabled);
        putstr("\t");  putstr(textBuf);
    
        sprintf(textBuf, "pbc | Pitch Bend Control: %d = ", g_Config.PitchBendCtrlMode);
        putstr("\t");  putstr(textBuf);
        putstr(pitchBendModeName[g_Config.PitchBendCtrlMode]);  
        putNewLine();
        sprintf(textBuf, "pbr | Pitch Bend Range (cents): %d \n", g_Config.PitchBendRange);
        putstr("\t");  putstr(textBuf);
        
        sprintf(textBuf, "aac | Audio Ampld Control: %d = ", g_Config.AudioAmpldControlMode);
        putstr("\t");  putstr(textBuf);
        if (g_Config.AudioAmpldControlMode == 1) putstr("ENV & Velocity \n");
        else if (g_Config.AudioAmpldControlMode == 2) putstr("Expression CC \n");  
        else if (g_Config.AudioAmpldControlMode == 3) putstr("Auto-detect \n"); 
        else  putstr("Fixed level (max)\n");  // Ctrl.Mode == 0
        
        sprintf(textBuf, "rva | Reverb Attenuator gain: %d %%\n", g_Config.ReverbAtten_pc);
        putstr("\t");  putstr(textBuf);
        sprintf(textBuf, "rvm | Reverb Mix (wet/dry ratio): %d %%\n", g_Config.ReverbMix_pc);
        putstr("\t");  putstr(textBuf);
        return;
    }
    
    if (argCount >= 3)  arg = atoi(argValue[2]);
    else  return;

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
    else if (strmatch(argValue[1], "mix"))  // MIDI IN Expression CC #
    {
        if (argCount >= 3 && (arg >= 0 && arg <= 31))
        {
            g_Config.MidiInExpressionCCnum = arg;
            updateConfig = 1;
        }
        else  isCmdError = 1;
    }
    else if (strmatch(argValue[1], "moe"))  // MIDI OUT Enabled
    {
        if (argCount >= 3 && (arg == 0 || arg == 1))
        {
            g_Config.MidiOutEnabled = arg;
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
    else if (strmatch(argValue[1], "mox"))  // MIDI OUT Exprn CC #
    {
        if (argCount >= 3 && (arg >= 0 && arg <= 31))
        {
            g_Config.MidiOutExpressionCCnum = arg;
            updateConfig = 1;
        }
        else  isCmdError = 1;
    }
    else if (strmatch(argValue[1], "mom"))  // MIDI OUT Modulation enabled
    {
        if (argCount >= 3 && (arg == 0 || arg == 1))
        {
            g_Config.MidiOutModnEnabled = arg;
            updateConfig = 1;
        }
        else  isCmdError = 1;
    }
    else if (strmatch(argValue[1], "pbc"))  // Pitch-Bend Control Mode
    {
        if (argCount >= 3 && (arg >= 0 && arg <= 3))  // todo: arg == 3 (analog CV)
        {
            g_Config.PitchBendCtrlMode = arg;
            updateConfig = 1;
        }
        else  isCmdError = 1;
    }
    else if (strmatch(argValue[1], "pbr"))  // Pitch-Bend Range
    {
        if (argCount >= 3 && (arg >= 0 && arg <= 1200))  // max. 1 octave
        {
            g_Config.PitchBendRange = arg;
            updateConfig = 1;
        }
        else  isCmdError = 1;
    }
    else if (strmatch(argValue[1], "aac"))  // Audio Amplitude Control Mode
    {
        if (argCount >= 3 && (arg >= 0 && arg <= 3))
        {
            g_Config.AudioAmpldControlMode = arg;
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

    if (isCmdError)  putstr("! Invalid <arg> value \n");
    
    if (updateConfig)  
    {
        putstr("* Done... config param ");
        putstr(argValue[1]);  putstr(" = ");  putDecimal(arg, 1);
        putNewLine();
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
    char    option;
    int     arg, arg_n;
    int     activePreset = g_Config.PresetLastSelected;  // 0..7
    int     i, preset, pr_idx, patch_ID, patch_idx = 0;

    if (argCount == 1 || *argValue[1] == '?')   // help wanted
    {
        putstr( "Select or configure one of the 8 Instrument Presets...    \n" );
        putstr( "Usage (1):  preset  <n>            | Select Preset (1..8) \n" );
        putstr( "Usage (2):  preset  <opt>  [arg]   | Configure Preset...  \n" );
        putstr( "  where <opt> = \n" );
        putstr( " -l  : List all Preset configurations \n");
        putstr( " -p  : Set Patch ID (0..99) for active Preset \n");
        putstr( " -n  : Set MIDI OUT program/voice Number (0..127) \n");
        putstr( " -v  : Set Vibrato mode (O:off, M:mod'n, A:auto-ramp)\n");
        putstr( " -t  : Set pitch Transpose, semitones (-24..+24) \n");
        ////
        return;
    }
    
    if (argCount >= 2)  arg_n = atoi(argValue[1]);  // Preset #
    if (argCount >= 3)  arg = atoi(argValue[2]);    // param value

    if (isdigit(*argValue[1]) && arg_n >= 0 && arg_n <= 8)  // Select Preset
    {
        InstrumentPresetSelect(arg_n);
        activePreset = g_Config.PresetLastSelected;  // 0..7
        patch_ID = g_Preset.Descr[activePreset].PatchNumber;
        
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
        option = tolower(argValue[1][1]);
        
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
                g_Preset.Descr[activePreset].PatchNumber = arg;
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
                if (c1 == 'O')  g_Preset.Descr[activePreset].VibratoMode = VIBRATO_DISABLED;
                if (c1 == 'M')  g_Preset.Descr[activePreset].VibratoMode = VIBRATO_BY_MODN_CC;
                if (c1 == 'A')  g_Preset.Descr[activePreset].VibratoMode = VIBRATO_AUTOMATIC;
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
        default:  option = 'l';  break;  // list Presets
        
        } // end switch
    }

    if (option == 'l')   // List all Presets configurations
    {
        putstr("PRESET | Patch                   | MIDI Prgm | Vibrato   | Transpose   \n");
        putstr("```````````````````````````````````````````````````````````````````````\n");
            
        for (preset = 1;  preset <= 8;  preset++)   // List order: 1..8
        {
            if (preset == 8) pr_idx = 0;  else pr_idx = preset;  // wrap 8 -> 0
                    
            patch_ID = g_Preset.Descr[pr_idx].PatchNumber;
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

            if (g_Preset.Descr[pr_idx].VibratoMode == VIBRATO_BY_MODN_CC)
                putstr("Modn CC01 | ");
            else if (g_Preset.Descr[pr_idx].VibratoMode == VIBRATO_AUTOMATIC)
                putstr("Auto ramp | ");
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
 * Function:     Console CLI command to manage the synth patch, incl...
 *                 -  view/modify active patch parameters
 *                 -  save active patch (param set) as 'User Patch' in EEPROM
 *                 -  load active patch from 'User Patch' in EEPROM
 *                 -  load active patch from predefined patch table in MCU flash
 *                 -  dump active patch to console as C data definition
 */
void  Cmnd_patch(int argCount, char *argValue[])
{
    char    paramAcronym[4];
    char    option;
    int     i, paramValue;

    if (argCount == 1 || (argCount == 2 && *argValue[1] == '?'))   // help wanted
    {
        putstr( "View or modify the active (selected) patch \n" );
        putstr( "``````````````````````````````````````````````````````````` \n" );
        putstr( "Usage (1):  patch  <opt>  [arg] \n" );
        putstr( "  where <opt> = \n" );
        putstr( " -l  : List/enumerate predefined patches. \n");
        putstr( " -d  : Dump active patch param's in C format \n");
        putstr( " -s  : Save active patch as User Patch [arg = name] \n");
        putstr( " -u  : Select User patch \n");
        putstr( " -p  : Select Predefined patch (arg = Patch ID) \n");
        putstr( " -w  : Wave-table info (active patch) \n");
        putstr( "``````````````````````````````````````````````````````````` \n" );
        putstr( "Usage (2):  patch  <param_ID> [=] <value> \n" );
        putstr( "  where <param_ID> is a 2-letter mnemonic (see dump) \n");
        putstr( "  Example: To set Vibrato Depth to 50 cents, enter:- \n" );
        putstr( "           patch VD 50   [Alt:  patch vd = 50] \n" );
        putstr( "___________________________________________________________ \n" );
        return;
    }
    
    if (argCount >= 2)  option = tolower(argValue[1][1]);

    if (*argValue[1] == '-')  // Usage is type (1)
    {
        switch (option)
        {
        case 'l':  // List/enumerate pre-defined patches in flash PM
        {
            int   spaces;

            putstr("      ID#  Patch Name              WT1  WT2  \n");
            putstr("```````````````````````````````````````````` \n");

            for (i = 0;  i < GetNumberOfPatchesDefined();  i++)
            {
                putDecimal(i, 3);
                putstr("  ");
                putDecimal(g_PatchProgram[i].PatchNumber, 4);
                putstr("  ");
                putstr((char *) g_PatchProgram[i].PatchName);
                spaces = 24 - strlen(g_PatchProgram[i].PatchName);
                while (spaces > 0)
                    { putch(' ');  spaces--; }
                putDecimal(g_PatchProgram[i].Osc1WaveTable, 3);
                putstr("  ");
                putDecimal(g_PatchProgram[i].Osc2WaveTable, 3);
                putstr("\n");
            }

            putstr("____________________________________________ \n");
            break;
        }
        case 'd':  // Dump active patch param's in C source format
        {
            DumpActivePatchParams();
            break;
        }
        case 's':  // Save active patch as User Patch
        {
            g_Patch.PatchNumber = 0;
            memset(&g_Patch.PatchName[0], 0, 22);  // clear existing name
            if (argCount >= 3)  // new name supplied
                strncpy(&g_Patch.PatchName[0], argValue[2], 20);
            else  strcpy(&g_Patch.PatchName[0], "User Patch");

            memcpy(&g_Config.UserPatch, &g_Patch, sizeof(PatchParamTable_t));

            if (StoreConfigData())  putstr("* Saved OK.\n");
            else  putstr("! Error writing to EEPROM.\n");
            break;
        }
        case 'u':  // Load User Patch
        {
            SynthPatchSelect(0);
            DumpActivePatchParams();
            break;
        }
        case 'p':  // Load & activate a predefined patch (from flash PM)
        {
            int   patchNum = atoi(argValue[2]);
            int   status = 0;

            if (argCount == 3 && patchNum != 0)
                status = SynthPatchSelect(patchNum);
            else  status = ERROR;

            if (status == ERROR)  putstr("! Missing or invalid patch ID number.\n");
            else  DumpActivePatchParams();
            break;
        }
        case 'w':  // View patch active wave-table data
        {
            PrintWaveTableInfo(1);  
            PrintWaveTableInfo(2);  
            break;
        }
        default:
        {
            putstr("! Invalid cmd option.");
            break;
        }
        } // end switch
    }
    else  // Usage is type (2) -- set a param value
    {
        char   arg2_1st_char = argValue[2][0];
        paramAcronym[0] = toupper(argValue[1][0]);
        paramAcronym[1] = toupper(argValue[1][1]);
            
        if (argCount == 4 && arg2_1st_char == '=') 
        {
            paramValue = atoi(argValue[3]);
            SetPatchParameter(paramAcronym, paramValue);
        }
        else if (argCount == 3 && (isdigit(arg2_1st_char) || arg2_1st_char == '-'))  
        {
            paramValue = atoi(argValue[2]);
            SetPatchParameter(paramAcronym, paramValue);
        }
        else  putstr("! Missing arg(s). Check cmd syntax.\n");
    }
}


PRIVATE  void   PrintWaveTableInfo(unsigned oscNum)
{
    char   textBuf[120];
    int    waveTableID;
    int    activeTableSize;
    float  activeOscFreqDiv;

    WaveformDesc_t    *waveFormDesc;
    FlashWaveTable_t  *waveTableDesc;

    if (oscNum == 1)  
    {
        waveTableID = g_Patch.Osc1WaveTable;
        activeTableSize = g_Osc1WaveTableSize;
        activeOscFreqDiv = g_Osc1FreqDiv;
    }
    else if (oscNum == 2)  
    {
        waveTableID = g_Patch.Osc2WaveTable;
        activeTableSize = g_Osc2WaveTableSize;
        activeOscFreqDiv = g_Osc2FreqDiv;
    }
    else  return;  // undefined oscNum

    sprintf(textBuf, "Wave-table assigned to OSC%d \n", oscNum);
    putstr(textBuf);  
    putstr("```````````````````````````````\n");

    if (waveTableID != 0)   // Wave-table stored in flash PM
    {
        waveTableDesc = (FlashWaveTable_t *) &g_FlashWaveTableDef[waveTableID];

        sprintf(textBuf, "Table ID num:  %d\n", waveTableID);
        putstr(textBuf);
        sprintf(textBuf, "Table Size:    %d\n", waveTableDesc->Size);
        putstr(textBuf);
        sprintf(textBuf, "Freq Divider:  %5.3f\n\n", waveTableDesc->FreqDiv);
        putstr(textBuf);
    }
    else  // RAM-based user wave-table
    {
        waveFormDesc = (WaveformDesc_t *) &g_Config.UserWaveform;
        
        sprintf(textBuf, "Table Size:    %d\n", waveFormDesc->Size);
        putstr(textBuf);
        sprintf(textBuf, "Freq Divider:  %5.3f\n", waveFormDesc->FreqDiv);
        putstr(textBuf);
        putstr("User wave-table selected. May be affected by 'wav' utility.\n");
        putstr("<!> Data shown above is from the wave-form descriptor in EEPROM.\n");
    }
    
    putstr("Currently active settings:\n");
    sprintf(textBuf, "Table Size:    %d\n", activeTableSize);
    putstr(textBuf);
    sprintf(textBuf, "Freq Divider:  %5.3f\n", activeOscFreqDiv);
    putstr(textBuf);
    putstr("```````````````````````````````\n");
}


/*
 * This function lists names and values of the active patch parameters in a format
 * suitable for import into the source code as a predefined patch definition.
 *
 * Parameters are listed in the order in which they are defined in the structure
 * PatchParamTable_t. Any change to the patch structure will require a corresponding
 * change to the listing.
 */
PRIVATE  void   DumpActivePatchParams()
{
    char    textBuf[120];

    putstr("Patch ID#:  ");  putDecimal(g_Patch.PatchNumber, 1);
    putstr(" -> ");  putstr(g_Patch.PatchName);
    putstr("\n\n");

    //-------------  Oscillators, Detune and Vibrato ------------------------------------

    sprintf(textBuf, "\t%d,\t// W1: OSC1 Wave-table ID (0..250)\n",
            (int) g_Patch.Osc1WaveTable);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// W2: OSC2 Wave-table ID (0..250)\n",
            (int) g_Patch.Osc2WaveTable);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// OD: OSC2 Detune, cents (+/-1200)\n",
            (int) g_Patch.Osc2Detune);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// LF: LFO Freq x10 Hz (1..250)\n",
            (int) g_Patch.LFO_Freq_x10);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// VD: Vibrato Depth, cents (0..200)\n",
            (int) g_Patch.LFO_FM_Depth);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// VR: Vibrato Ramp time (5..5000+ ms)\n",
            (int) g_Patch.LFO_RampTime);
    putstr(textBuf);
    putstr("\n");

    //-------------  Wave Mixer & Contour Envelope --------------------------------------

    sprintf(textBuf, 
            "\t%d,\t// MC: Mixer Control (0:Fixed, 1:Contour, 2:LFO, 3:Exprn, 4:Modn)\n",
            (int) g_Patch.MixerControl);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// ML: Mixer OSC2 Level in Fixed mode (0..100 %%)\n",
            (int) g_Patch.MixerOsc2Level);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// CS: Contour Env Start level (0..100 %%)\n",
            (int) g_Patch.ContourStartLevel);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// CD: Contour Env Delay time (5..5000+ ms)\n",
            (int) g_Patch.ContourDelay_ms);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// CR: Contour Env Ramp time (5..5000+ ms)\n",
            (int) g_Patch.ContourRamp_ms);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// CH: Contour Env Hold level (0..100 %%)\n",
            (int) g_Patch.ContourHoldLevel);
    putstr(textBuf);
    putstr("\n");

    //-------------  Noise Mixer & Bi-quad resonant Filter ------------------------------

    sprintf(textBuf,
            "\t%d,\t// NM: Noise Mode (0:Off, 1:Noise, 2:Add wave, 3:Mix wave; +4:Pitch)\n",
            (int) g_Patch.NoiseMode);
    putstr(textBuf);
    sprintf(textBuf,
            "\t%d,\t// NC: Noise Level Ctrl (0:Fixed, 1:Amp.Env, 2:LFO, 3:Exprn, 4:Modn)\n",
            (int) g_Patch.NoiseLevelCtrl);
    putstr(textBuf);
    sprintf(textBuf,
            "\t%d,\t// FC: Filter Ctrl (0:Fixed, 1:Contour, 2:LFO, 3:Exprn, 4:Modn)\n",
            (int) g_Patch.FilterControl);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// FR: Filter Resonance x10000  (0..9999, 0:Bypass)\n",
            (int) g_Patch.FilterResonance);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// FF: Filter Cutoff Freq (MIDI note - 12, 0..108)\n",
            (int) g_Patch.FilterFrequency);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// FT: Filter Note Tracking (0:Off, 1:ON) \n",
            (int) g_Patch.FilterNoteTrack);
    putstr(textBuf);
    putstr("\n");

    //-------------  Amplitude Envelope Generator  --------------------------------------

    sprintf(textBuf, "\t%d,\t// EA: Envelope Attack time (5..5000+ ms)\n",
            (int) g_Patch.AmpldEnvAttack_ms);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// EP: Envelope Peak time (0..5000+ ms)\n",
            (int) g_Patch.AmpldEnvPeak_ms);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// ED: Envelope Decay time (5..5000+ ms)\n",
            (int) g_Patch.AmpldEnvDecay_ms);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// ER: Envelope Release time (5..5000+ ms)\n",
            (int) g_Patch.AmpldEnvRelease_ms);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// ES: Envelope Sustain level (0..100 %%)\n",
            (int) g_Patch.AmpldEnvSustain);
    putstr(textBuf);
    putstr("\n");
}


/*
 *  Function sets an active patch parameter to a given value, if the value is valid.
 *
 *  Entry arg(s):  paramAbbr = pointer to a 2-char string identifying the parameter
 *                             to be set.
 *
 *                 paramVal = new value for the parameter
 *
 *  <!>  All param's except 'OD' (OSC2 Detune) must have positive values (>= 0).
 */
PRIVATE  void   SetPatchParameter(char *paramAbbr, int paramVal)
{
    int   paramHash = PARAM_HASH_VALUE(paramAbbr[0], paramAbbr[1]);
    bool  isBadValue = 0;

    if (paramVal < 0 && paramHash != PARAM_HASH_VALUE('O', 'D'))  
    {
        isBadValue = 1;   // negative value not accepted
    }
    else switch (paramHash)
    {
        //-------------  Oscillators, Pitch Bend and Vibrato --------------------------------
        case PARAM_HASH_VALUE('W', '1'):
        {
            if (paramVal <= GetHighestWaveTableID())  
                g_Patch.Osc1WaveTable = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('W', '2'):
        {
            if (paramVal <= GetHighestWaveTableID())
                g_Patch.Osc2WaveTable = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('O', 'D'):
        {
            if (paramVal >= -4800 && paramVal <= 4800)
                g_Patch.Osc2Detune = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('L', 'F'):
        {
            if (paramVal > 1 && paramVal <= 250)  
                g_Patch.LFO_Freq_x10 = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('V', 'D'):
        {
            if (paramVal <= 200)  g_Patch.LFO_FM_Depth = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('V', 'R'):
        {
            if (paramVal >= 5 && paramVal <= 10000)  
                g_Patch.LFO_RampTime = paramVal;
            else  isBadValue = 1;
            break;
        }
        //-------------  Wave Mixer & Contour Envelope --------------------------------------
        case PARAM_HASH_VALUE('M', 'C'):
        {
            if (paramVal <= 15)  g_Patch.MixerControl = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('M', 'L'):
        {
            if (paramVal <= 100)  g_Patch.MixerOsc2Level = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('C', 'S'):
        {
            if (paramVal <= 100)  g_Patch.ContourStartLevel = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('C', 'D'):
        {
            if (paramVal >= 5 && paramVal <= 10000)  
                g_Patch.ContourDelay_ms = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('C', 'R'):
        {
            if (paramVal >= 5 && paramVal <= 10000)  
                g_Patch.ContourRamp_ms = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('C', 'H'):
        {
            if (paramVal <= 100)  g_Patch.ContourHoldLevel = paramVal;
            else  isBadValue = 1;
            break;
        }
        //-------------  Noise Generator & Bi-Quad Variable Filter --------------------------------
        case PARAM_HASH_VALUE('N', 'M'):
        {
            if (paramVal <= 7)  g_Patch.NoiseMode = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('N', 'L'):
        case PARAM_HASH_VALUE('N', 'C'):
        {
            if (paramVal <= 15)  g_Patch.NoiseLevelCtrl = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('F', 'C'):
        {
            if (paramVal <= 15)  g_Patch.FilterControl = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('F', 'R'):
        {
            if (paramVal <= 9999)  g_Patch.FilterResonance = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('F', 'F'):
        {
            if (paramVal <= 108)  g_Patch.FilterFrequency = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('F', 'T'):
        {
            if (paramVal <= 1)  g_Patch.FilterNoteTrack = paramVal;
            else  isBadValue = 1;
            break;
        }
        //-------------  Amplitude Envelope and Output Amplitude Control  -------------------
        case PARAM_HASH_VALUE('E', 'A'):
        {
            if (paramVal >= 5 && paramVal <= 10000)  
                g_Patch.AmpldEnvAttack_ms = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('E', 'P'):
        {
            if (paramVal <= 10000)  g_Patch.AmpldEnvPeak_ms = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('E', 'D'):
        {
            if (paramVal >= 5 && paramVal <= 10000)  
                g_Patch.AmpldEnvDecay_ms = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('E', 'R'):
        {
            if (paramVal >= 5 && paramVal <= 10000)  
                g_Patch.AmpldEnvRelease_ms = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('E', 'S'):
        {
            if (paramVal <= 100)  g_Patch.AmpldEnvSustain = paramVal;
            else  isBadValue = 1;
            break;
        }
        default:
        {
            putstr("! Parameter acronym undefined: ");
            putch(paramAbbr[0]);
            putch(paramAbbr[1]);
            putstr(" \n");
            return;
        }
    }  // end switch

    if (isBadValue)  putstr("! Value rejected - out of bounds.\n");
    else  
    {
        SynthPrepare();
        DumpActivePatchParams();
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

    sprintf(textBuf, "Bauer {REMI} Sound Synth -- firmware version: %d.%d.%02d \n",
            g_FW_version[0], g_FW_version[1], g_FW_version[2]);
    putstr(textBuf);
    
#if defined (SYNTH_MK2_MX340_LITE)
    putstr("Hardware platform/variant: mk2 'Lite' (PIC32MX340) \n");
#elif defined (SYNTH_MK2_MX340_STD)   
    putstr("Hardware platform/variant: mk2 'Std' (PIC32MX340) \n");
#elif defined (SYNTH_MK3_MX440_MAM)   
    putstr("Hardware platform/variant: mk3 'MAM' (PIC32MX440) \n");    
#endif
    
    if (isLCDModulePresent())
    {
#if defined (USE_LCD_CONTROLLER_ST7920)
        putstr("LCD controller driver: ST7920 \n");
#elif defined (USE_LCD_CONTROLLER_KS0108)
        putstr("LCD controller driver: KS0108 \n");
#elif defined (USE_OLED_CONTROLLER_SH1106)
        putstr("OLED display controller: SH1106 \n");
#endif        
    }
    else   putstr("LCD module not detected. Local UI disabled.\n");
    
    if (POT_MODULE_CONNECTED)
        putstr("Control Panel (detachable pot module) connected.\n");
    else
        putstr("Control Panel (detachable pot module) not detected.\n");
    
    if (isHandsetConnected())
    {
        putstr("REMI handset connected...\n");

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
    char    option;
    
    if (argCount == 1 || (argCount == 2 && *argValue[1] == '?'))   // help wanted
    {
        putstr( "MIDI Input Monitor -- diagnostic utility \n" );
        putstr( "Usage:  mimon  <opt>  [n] \n" );
        putstr( "where <opt> = \n" );
        putstr( "  -a : Activate the monitor \n" );
        putstr( "  -d : De-activate the monitor and display stats \n" );
        putstr( "  -n : Output last <n> buffer entries (n <= 512) \n" );
        putstr( "Note: Real-time messages (clocks, etc) are ignored. \n" );
        return;
    }
    
    if (argCount >= 2)  option = tolower(argValue[1][1]);
    
    if (option == 'a' || option == 'e')  // Activate/enable the monitor
    {
        putstr("MIDI IN monitor activated. \n");
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
    char    option = 0;

    if (argCount == 1 || (argCount == 2 && *argValue[1] == '?'))   // help wanted
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

    if (argCount >= 2)  option = tolower(argValue[1][1]);
    if (argCount >= 3)  blockNum = atoi(argValue[2]) & 3;

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
    
    int  arg1 = 0, arg2 = 0, arg3 = 0;
    
    if ( argCount == 2 && *argValue[1] == '?' )  // help wanted
    {
        putstr( "Start (or stop) a sound generated by the REMI synth engine.\n" );
        putstr( "On the first 'sound' after power-on/reset, the active patch is set \n" );
        putstr( "to the 'Test Patch' (ID 90) where OSC1 wave-form is a sine-wave. \n" );
        putstr( "Audio output level is controlled as configured by parameter 'AAC'.\n" );
        putstr( "``````````````````````````````````````````````````````````````````\n" );
        putstr( "Usage:  sound  [note] [durn] [vib] \n" );
        putstr( "where ... \n" );
        putstr( "  <note> = MIDI note number (12..108,  0 => Note-Off) \n" );
        putstr( "  <durn> = duration of note, seconds (0 => continuous) \n" );
        putstr( "  <vib>  = set vibrato mode (0:off, 3:auto-ramp, etc.) \n" );
        putstr( "If no note or durn value given, use previous non-zero value(s). \n" );
        putstr( "If durn is 0, sound continuously.  If note is 0, stop sound. \n" );
        return;
    }
    
    if (argCount >= 2)  arg1 = atoi(argValue[1]);
    if (argCount >= 3)  arg2 = atoi(argValue[2]);
    if (argCount >= 4)  arg3 = atoi(argValue[3]);
    
    if (argCount == 2 && arg1 == 0)
    {
        SynthNoteOff(noteNumber);
        return;
    }
    
    if (argCount >= 2)  noteNumber = arg1 & 0x7F;
    if (argCount >= 3 && arg2 > 0)  duration = arg2 * 1000;  // ms
    if (argCount >= 4)  vibratoMode = arg3 & 7;
    
    if (!init_done)  // Power-on/reset initialization -- load default patch
    {
        SynthPatchSelect(90);  // "Test Patch" (Osc1: sine wave;  Osc2: sawtooth)
        putstr("Test Patch (#90) loaded.\n");
        init_done = 1;
    }
    
    noteEndTime = milliseconds() + duration;  // msec
    
    SetVibratoMode(vibratoMode);   // Override Preset vibrato mode
    SynthNoteOn(noteNumber, 80);   // Velocity scale is non-linear
    SynthExpression(8000);         // full scale is 16000
    
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
    
    SynthNoteOff(noteNumber);
}


/*```````````````````````````````````````````````````````````````````````````````````````
 *   Function called by "diag" command.
 * 
 *   NB:  If the firmware build includes support for timed "Diagnostic Mode",
 *        then all diag command options (except '-x') activate Diagnostic Mode
 *        which causes the REMI synth process (B/G task) to be suspended.
 *        Command "diag -x" cancels Diagnostic Mode and re-activates the synth
 *        process. A time-out (default 30 mins) is imposed on Diagnostic Mode.
 */
void  DiagnosticCommandExec(int argCount, char * argValue[])
{
    static bool    diagEnabled;
    static uint32  intervalStart;
    char   option;
    int    arg;

    if (argCount == 1 || (argCount == 2 && *argValue[1] == '?'))   // help wanted  g_TaskCallFrequency
    {
        putstr( "PIC32MX synth module :: low-level diagnostics \n" );
        putstr( "`````````````````````````````````````````````` \n" );
        putstr( "Usage:  diag  <option>  [arg's] ... \n" );
        putstr( "Options:  \n" );
        putstr( " -a  :  Audio ISR execution time \n");
        putstr( " -b  :  Background task frequency \n");
        putstr( " -c  :  Control pot readings \n");
        putstr( " -d  :  LCD backlight toggle \n");
        putstr( " -e  :  Expression peak value \n");
        putstr( " -i  :  Test I2C bus signals \n");
        putstr( " -p  :  Test Pitch Bend (arg: +/-8000) \n");
        putstr( " -r  :  Reverb mix setting \n");
        putstr( " -s  :  Disable/enable Synth ISR (arg: 0|1) \n");
        putstr( " -u  :  UART errors.\n");
        putstr( " -y  :  CPU core cYcle timer \n");
#ifdef SUPPORT_DIAG_MODE        
        putstr( " -t  :  Set timeout for diag mode (mins) \n");
        putstr( " -x  :  Exit diagnostic mode. \n");
#endif
        return;
    }
    
#ifdef SUPPORT_DIAG_MODE    
    if (option != 'x')  
    {
        EnableDiagnosticMode();
        diagEnabled = 1;
    }
#endif
    
    if (argCount >= 2)  option = tolower(argValue[1][1]);
    
    switch (option)
    {
    case 'a':  // Show Audio ISR execution time (us)
    {
        int  execTime_us = (int) v_ISRexecTime / 40 + 1;  // 40 counts per microsecond
        int  duty_pc = (execTime_us * 100) / 25;  // duty = % of ISR period
        
        putstr("Audio ISR execution time: ");
        putDecimal(execTime_us, 1);
        putstr(" us;  Portion of CPU time: ");
        putDecimal(duty_pc, 1);
        putstr(" % \n");
        putstr("excluding context-switching overhead (negligible).\n");
        break;
    }
    case 'b':  // Background task frequency check
    {
        uint32 loopStartTime = milliseconds();
        uint32 taskFreq;
        
        while ((milliseconds() - loopStartTime) < 2000)
        {
            BackgroundTaskExec();
            GUI_NavigationExec();
        }
        taskFreq = g_TaskCallFrequency;  // capture before serial output
        putstr("Main B/G process (1ms task) calls per second: ");
        putDecimal(taskFreq, 1);
        putNewLine();
        break;
    }
    case 'c':  // Show Control Panel pot readings (8 bits, filtered) 
    {
        char  key = 0;  short n;
        
		if (!isLCDModulePresent())
        {
            putstr("! LCD module not detected. \n");
            break;  // bail
        }
        
        if (POT_MODULE_CONNECTED == FALSE)
        {
            putstr("! Pot Control Panel not detected. \n");
            break;  // bail
        }
		
		putstr("  Hit [Esc] to exit ... \n");
        
        intervalStart = milliseconds();
        while (key != ASCII_ESC)
        {
            GUI_NavigationExec();  // Service pots, buttons & LCD panel
            
            if ((milliseconds() - intervalStart) >= 200)
            {
                putch(ASCII_CR);
                for (n = 0; n < 6; n++)  { putDecimal(PotReading(n), 6); }
                putstr("    ");
                intervalStart = milliseconds();
            }
            if (kbhit())  key = getch();
        }
        putNewLine();
        break;
    }
    case 'd':  // LCD backlight Dim/bright toggle
    {
        ToggleBacklight();
        if (LCD_BACKLIGHT_IS_LOW) putstr("* LCD backlight LOW.\n");
        else  putstr("* LCD backlight HIGH.\n");
        break;
    }
    case 'e':  // Show Expression peak value (normalized)
    {
        char   textBuf[80];
        float  exprnPeak = FixedToFloat(g_ExpressionPeak);
        
        sprintf(textBuf, "Last Expression peak: %6.3f (normalized) \n", exprnPeak);
        putstr(textBuf);
        g_ExpressionPeak = 0;  // reset
        break;
    }
    case 'i':  // Test I2C bus signals
    {
        putstr("  Writing 0x55 continuously to I2C address 0x01 \n ");  
        putstr("  Hit any key to exit... \n ");  
        I2C1MasterStart(0x02);  // write slave address 0x01
        while (!kbhit())
        {
            I2C1MasterSend(0x55);
        }
        Stop_I2C1();
        break;
    }
    case 'p':  // Pitch bend test
    {
        if (argCount >= 3) 
        {
            arg = atoi(argValue[2]) & 0x3FFF;
            SynthPitchBend(arg);
        }
        else  putstr("! Need arg (0..+/-8000) \n ");  
        break;
    }
    case 'r':  // Show *active* Reverb Mix setting (% wet)
    {
        int  setting = (GetReverbMixSetting() * 100 + 5) / 128;  // rounded
        
        putstr("Active Reverb Mix (% wet): ");
        putDecimal(setting, 1);
        putNewLine();        
        break;
    }
    case 's':  // Suspend or activate synth audio ISR
    {
        if (argCount == 3 && *argValue[2] == '0') 
        {
            v_SynthEnable = FALSE;
            putstr("* Synth audio ISR suspended.\n "); 
        }
        else  
        {
            v_SynthEnable = TRUE;
            putstr("* Synth audio ISR active.\n "); 
        }
        break;
    }
    case 'u':  // UART errors 
    {
        putstr("UART #1 error count: ");  
        putDecimal(UART1_getErrorCount(), 5);
        putNewLine();
        putstr("UART #2 error count: ");  
        putDecimal(UART2_getErrorCount(), 5);
        putNewLine();
        break;
    }
    case 'y':  // Core cycle timer test
    {
        CoreCycleTimerTest();
        break;
    }

#ifdef SUPPORT_DIAG_MODE  // If supported, DIAG MODE suspends the Synth Process task  
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

    if (argCount == 2 && *argValue[1] == '?')   // help wanted
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
    char   option;
    short  wavetableFreq[64];
    short  ipat, iwav;
    
    if (argCount == 1 || (argCount == 2 && *argValue[1] == '?'))   // help wanted
    {
        putstr( "Run a specified Utility...   \n" );
        putstr( "Usage:  util  <opt>  [args] \n" );
        putstr( "<opt> \n" );
        putstr( "  -a : Test fixed-pt (arg1) x integer (arg2) / 1024 \n");
        putstr( "  -b : Test Base2exp() function (no arg's) \n");
        putstr( "  -c : Count wavetable instances in patch def's \n");
            ;
            ;
            ;
        return;
    }
    
    if (argCount >= 2)  option = tolower(argValue[1][1]);
    else  return;  // No option

    switch (option)
    {
    case 'a':
    {
        float    arg1;
        fixed_t  multiplicand, product;
        int      multiplier;
        
        if (argCount >= 4)
        {
            arg1 = atof(argValue[2]);  // float
            multiplicand = FloatToFixed(arg1);  // fixed-point
            multiplier = atoi(argValue[3]);     // integer
            product = (multiplicand * multiplier) >> 10;  // fixed-point
            
            sprintf(textbuf, "  Scalar multiply: (%f x %d) / 1024 = %f \n", 
                    FixedToFloat(multiplicand),  multiplier,  FixedToFloat(product));
            putstr(textbuf);
        }
        break;
    }
    case 'b':
    {
        TestFixedPtBase2Exp();
        break;
    }
    case 'c':
    {
        for (iwav = 0;  iwav <= GetHighestWaveTableID();  iwav++ )
        {
            wavetableFreq[iwav] = 0;
        }
        
        for (ipat = 0;  ipat < GetNumberOfPatchesDefined();  ipat++)
        {
            iwav = g_PatchProgram[ipat].Osc1WaveTable & 0x3F;  // W1
            wavetableFreq[iwav] += 1;
            iwav = g_PatchProgram[ipat].Osc2WaveTable & 0x3F;  // W2
            wavetableFreq[iwav] += 1;
        }
        
        putstr("Wavetable # | Occurrences in patch definitions \n");
        for (iwav = 0;  iwav <= GetHighestWaveTableID();  iwav++ )
        {
            sprintf(textbuf, "     %3d    |   %3d \n", iwav, wavetableFreq[iwav]);
            putstr(textbuf);
        }
        break;
    }
    default:  break;
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


/*```````````````````````````````````````````````````````````````````````````````````````
 *   Function called by "watch" command function...
 *   Variables to be "watched" are output on a single line (no newline).
 */
void  WatchCommandExec(void)
{
    char   txtbuf[100];
    
    float  exprsn = FixedToFloat(GetExpressionLevel());  // CC02 (normalized)
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
    g_Config.FilterInputAtten = g_FilterInputAtten;
    g_Config.FilterOutputGain = g_FilterOutputGain;
    g_Config.NoiseFilterGain = g_NoiseFilterGain;
        
    StoreConfigData();
    SynthPrepare();
}
