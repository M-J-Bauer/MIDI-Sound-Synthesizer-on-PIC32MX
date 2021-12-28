/*
 *   File:    remi_synth2_config.h 
 *
 *   Definitions for REMI synth persistent configuration data held in 24LC64 EEPROM.
 */
#ifndef REMI_SYNTH2_CONFIG_H
#define REMI_SYNTH2_CONFIG_H

#include "../Common/system_def.h"
#include "HardwareProfile.h"
#include "remi_synth2_def.h"

typedef struct Instrument_Preset_Descriptor
{
    uint16  RemiSynthPatch;           // REMI synth patch ID # for this Preset
    uint8   MidiProgram;              // MIDI program (voice) # for this Preset
    uint8   VibratoMode;              // Vibrato Control Mode [see note 2]
    int8    PitchTranspose;           // Pitch transpose/offset semitones (+/-24)

} InstrumentPreset_t;

// Note 1:  EffectSwitchCC = 0:None (disabled), 65:Portamento, 68:Legato, 85:Vibrato


typedef struct Eeprom_block0_structure
{
    uint32  checkDword;               // Constant value used to check data integrity
    ////
    uint16  MidiInBaudrate;           // MIDI IN baudrate (dflt 31250, max. 57600)
    uint8   MidiInMode;               // MIDI IN mode, range 1..4, dflt: 2 (Omni-Mono)
    uint8   MidiInChannel;            // MIDI IN channel, range: 1..16, default: 1
    uint8   MidiInPressureCCnum;      // MIDI IN breath/pressure CC number, dflt: 2
    uint8   MidiInModulationCCnum;    // MIDI IN effect modulation CC number, dflt: 1
    
    uint8   MidiOutMode;              // MIDI OUT mode, range 1..4, dflt: 2 (Omni-Mono)
    uint8   MidiOutChannel;           // MIDI OUT channel, range: 1..16, default: 1
    uint8   MidiOutPressureCCnum;     // MIDI OUT breath/pressure CC number, dflt: 2
    uint8   MidiOutModulationCCnum;   // MIDI OUT effect modulation CC number, dflt: 1
    
    uint8   ReverbAtten_pc;           // Reverberation attenuator gain (1..100 %)
    uint8   ReverbMix_pc;             // Reverberation wet/dry mix (1..100 %)
    
    uint8   PresetLastSelected;       // Preset last selected (0..7)
    uint8   AmpldControlOverride;     // Override Patch Audio Ampld Control **
    
    // Calibration param's (not settable via "config" cmd; use "set" cmd.)
    float   PressureGain;             //
    float   FilterInputAtten;         //
    float   FilterOutputGain;         //
    float   NoiseFilterGain;          //
    //
    PatchParamTable_t  UserPatch;     // User-programmable patch parameters
    WaveformDesc_t     UserWaveform;  // User Wave-table descriptor
    //
    uint32  EndOfDataBlockCode;       // Last entry, used to test if format has changed

} EepromBlock0_t;

// ** Flag to override patch Audio Ampld Control (AC) parameter...
//    0: use patch AC param (default);  1: use Ampld Envelope (override)


typedef struct Eeprom_block1_structure
{
    uint32 checkDword;
    ////
    InstrumentPreset_t  Descr[8];     // Array of 8 Preset descriptors
    ////
    uint32 EndOfDataBlockCode;

} EepromBlock1_t;


typedef struct Eeprom_block2_structure
{
    uint32 checkDword;
    ////
    //     (block not used yet)
    ////
    uint32 EndOfDataBlockCode;

} EepromBlock2_t;


typedef struct Eeprom_block3_structure
{
    uint32 checkDword;
    ////
    //     (block not used yet)
    ////
    uint32 EndOfDataBlockCode;

} EepromBlock3_t;


// global data defined in "remi_config.c" code module
//
extern  EepromBlock0_t  g_Config;     // structure holding configuration data
extern  EepromBlock1_t  g_Preset;     // structure holding Preset parameters


// EEPROM API functions defined in "remi_config.c" code module
//
void  DefaultConfigData(void);
void  DefaultPresetData(void);
bool  CheckConfigData(void);
bool  CheckPresetData(void);
int   FetchConfigData(void);
int   FetchPresetData(void);
bool  StoreConfigData(void);
bool  StorePresetData(void);


#endif // REMI_SYNTH2_CONFIG_H
