/*
 *   File:  remi_synth2_config.c
 *
 *   Module handles persistent data storage in non-volatile memory.
 *   Customized for the REMI mk2 sound synth using 24LCXX EEPROM.
 */
#include "main_remi_synth2.h"
#include "remi_synth2_config.h"

EepromBlock0_t  g_Config;     // structure holding configuration data
EepromBlock1_t  g_Preset;     // structure holding Preset parameters


/*`````````````````````````````````````````````````````````````````````````````````````````````````
 *
 *   Function writes default values for persistent data in EEPROM block 0.
 *   These are "factory" defaults which are applied only in the event of erasure or
 *   corruption of EEPROM data, and of course in first-time programming.
 */
void  DefaultConfigData(void)
{
    int   i, ampld;

    g_Config.checkDword = 0xFEEDFACE;
    g_Config.EndOfDataBlockCode = 0xE0DBC0DE;

    // Assign defaults to application-specific param's...
    g_Config.MidiInBaudrate = 31250;
    g_Config.MidiInMode = 2;                // 2:omni-on-mono, 4:omni-off-mono
    g_Config.MidiInChannel = 1;
    g_Config.MidiInExpressionCCnum = 2;     // 0:none, 2:breath, 7:chan-vol, 11:expr'n
    
    g_Config.MidiOutEnabled = 0;            // 0:disabled, 1:enabled
    g_Config.MidiOutChannel = 1;
    g_Config.MidiOutExpressionCCnum = 2;    // 0:none, 2:breath, 7:chan-vol, 11:expr'n
    g_Config.MidiOutModnEnabled = 1;        // 0:none, 1:mod-lever (CC01:33)
    
    g_Config.ReverbAtten_pc = 80;
    g_Config.ReverbMix_pc = 20;
    g_Config.PitchBendCtrlMode = 0;         // 0:disabled, 1 = MIDI PB, 2 = MIDI CC2
    g_Config.PitchBendRange = 200;          // 0..1200 cents
    g_Config.PresetLastSelected = 1;
    
    // Calibration constants (default settings)
    g_Config.FilterInputAtten = 0.25; 
    g_Config.FilterOutputGain = 4.0;   
    g_Config.NoiseFilterGain = 4.0; 

    // Copy Patch[0] to User Patch
    memcpy(&g_Config.UserPatch, &g_PatchProgram[0], sizeof(PatchParamTable_t));
    g_Config.UserPatch.PatchNumber = 0;
    strcpy(g_Config.UserPatch.PatchName, "User Patch");

    // Create a default User Waveform descriptor
    g_Config.UserWaveform.Size = 1260;
    g_Config.UserWaveform.FreqDiv = 1.0;

    for ((ampld = 500, i = 0);  i < 16;  i++)
    {
        g_Config.UserWaveform.Partial[i] = ampld / 10;  // 50, 25, 12, 6, ...
        ampld = ampld / 2;
    }

    StoreConfigData(); 
    UART1_init(g_Config.MidiInBaudrate);
    InstrumentPresetSelect(1);
}


/*
 *   Function writes default values for persistent data in EEPROM block 1.
 *   These are "factory" defaults which are applied only in the event of erasure or
 *   corruption of EEPROM data, and of course in first-time programming.
 */
void  DefaultPresetData(void)
{
    //                             PRESET:   8   1   2   3   4   5   6   7
    static  uint8   defaultMidiProgram[] = { 67, 75, 72, 69, 74, 17, 20, 23 };
    static  uint8   defaultSynthPatch[]  = { 41, 10, 11, 21, 26, 48, 45, 46 }; 
    int  i;

    g_Preset.checkDword = 0xDEADBEEF;
    g_Preset.EndOfDataBlockCode = 0xE0DBC0DE;

    for (i = 0;  i < 8;  i++)
    {
        g_Preset.Descr[i].PatchNumber = defaultSynthPatch[i];
        g_Preset.Descr[i].MidiProgram = defaultMidiProgram[i];
        g_Preset.Descr[i].VibratoMode = VIBRATO_DISABLED;
        g_Preset.Descr[i].PitchTranspose = 0;
    }
    StorePresetData();
}


/*
 *  Function checks the integrity of persistent data stored in EEPROM block 0.
 *
 *  If the block is erased or the data is found to be corrupt, or if the structure size
 *  exceeds the block size (256 bytes), the function returns FALSE;  otherwise TRUE.
 */
bool  CheckConfigData(void)
{
    BOOL   result = TRUE;

    g_Config.checkDword = 0xFFFFFFFF;
    g_Config.EndOfDataBlockCode = 0xFFFFFFFF;

    if (FetchConfigData() == ERROR) result = FALSE;
    if (g_Config.checkDword != 0xFEEDFACE) result = FALSE;
    if (g_Config.EndOfDataBlockCode != 0xE0DBC0DE) result = FALSE;

    return result;
}


/*
 *  Function checks the integrity of persistent data stored in EEPROM block 1.
 *
 *  If the block is erased or the data is found to be corrupt, or if the structure size
 *  exceeds the block size (256 bytes), the function returns FALSE;  otherwise TRUE.
 */
bool  CheckPresetData(void)
{
    BOOL   result = TRUE;

    g_Preset.checkDword = 0xFFFFFFFF;
    g_Preset.EndOfDataBlockCode = 0xFFFFFFFF;

    if (FetchPresetData() == ERROR) result = FALSE;
    if (g_Preset.checkDword != 0xDEADBEEF) result = FALSE;
    if (g_Preset.EndOfDataBlockCode != 0xE0DBC0DE) result = FALSE;

    return result;
}


/*
 *  Function copies data from EEPROM block #0 to a RAM buffer where persistent data
 *  can be accessed by the application. If the operation is successful, the return
 *  value will be equal to the size of the structure g_Config (bytes).
 *
 *  Return val:  (int) number of bytes successfully copied to the buffer, or,
 *               ERROR (-1) if the operation failed, 
 *               e.g. if the EEPROM could not be accessed or sizeof(g_Config) > 256
 */
int  FetchConfigData()
{
    int  result = EepromReadData((uint8 *) &g_Config, 0, 0, sizeof(g_Config));

    if (sizeof(g_Config) > 256)  result = ERROR;

    return  result;
}


/*
 *  Function copies data from EEPROM block #1 to a RAM buffer where persistent data
 *  can be accessed by the application. If the operation is successful, the return
 *  value will be equal to the size of the structure g_Preset (bytes).
 *
 *  Return val:  (int) number of bytes successfully copied to the buffer, or,
 *               ERROR (-1) if the operation failed,
 *               e.g. if the EEPROM could not be accessed or sizeof(g_Preset) > 256
 */
int  FetchPresetData()
{
    int  result = EepromReadData((uint8 *) &g_Preset, 1, 0, sizeof(g_Preset));

    if (sizeof(g_Preset) > 256)  result = ERROR;

    return  result;
}


/*
 *  Function copies data from a RAM buffer (holding current working
 *  values of persistent parameters) to the EEPROM block #0.
 *  <!> The size of the structure g_Config must not exceed 256 bytes.
 *
 *  Return val:  TRUE if the operation was successful, else FALSE.
 */
bool  StoreConfigData()
{
    int    promAddr = 0;
    int    bytesToCopy = sizeof(g_Config);
    uint8  *pData = (uint8 *) &g_Config;
    BOOL   result = TRUE;

    if (sizeof(g_Config) > 256)
    {
        bytesToCopy = 256;
        result = FALSE;
    }

    while (bytesToCopy > 0)
    {
        if (EepromWriteData(pData, 0, promAddr, 16) == ERROR)
        {
            result = FALSE;
            break;
        }
        promAddr += 16;
        pData += 16;
        bytesToCopy -= 16;
    }

    return result;
}


/*
 *  Function copies data from a RAM buffer (holding current working
 *  values of persistent parameters) to the EEPROM block #1.
 *  <!> The size of the structure g_Preset must not exceed 256 bytes.
 *
 *  Return val:  TRUE if the operation was successful, else FALSE.
 */
bool  StorePresetData()
{
    int    promAddr = 0;
    int    bytesToCopy = sizeof(g_Preset);
    uint8  *pData = (uint8 *) &g_Preset;
    BOOL   result = TRUE;

    if (sizeof(g_Preset) > 256)
    {
        bytesToCopy = 256;
        result = FALSE;
    }

    while (bytesToCopy > 0)
    {
        if (EepromWriteData(pData, 1, promAddr, 16) == ERROR)
        {
            result = FALSE;
            break;
        }
        promAddr += 16;
        pData += 16;
        bytesToCopy -= 16;
    }

    return result;
}
