/*
 *   File:    remi_synth_config.h 
 *
 *   Definitions for REMI synth persistent configuration data held in 24LC64 EEPROM.
 */
#ifndef REMI_SYNTH_CONFIG_H
#define REMI_SYNTH_CONFIG_H

#include "../Common/system_def.h"
#include "remi_synth_def.h"

// Possible values for PRESET parameter: g_Preset.Descr[preset].VibratoMode
#define VIBRATO_DISABLED            0    // Vibrato always off
#define VIBRATO_BY_EFFECT_SW        1    // Vibrato controlled by effect switch (N/A)
#define VIBRATO_BY_MODN_CC          2    // Vibrato controlled by Modulation (CC1)
#define VIBRATO_AUTOMATIC           3    // Vibrato automatic, delay + ramp, OSC1, OSC2

// Possible values for configuration parameter: g_Config.PitchBendCtrlMode
#define PITCH_BEND_DISABLED         0    // Pitch Bend disabled
#define PITCH_BEND_BY_MIDI_PB       1    // Pitch Bend uses MIDI pitch-bend data
#define PITCH_BEND_BY_EXPRN_CC      2    // Pitch Bend uses MIDI expression CC data
#define PITCH_BEND_BY_ANALOG_CV     3    // Pitch Bend uses analog CV input (todo)

// Possible values for configuration parameter: g_Config.AudioAmpldControlMode
#define AMPLD_CTRL_FIXED_FS         0    // Output ampld is fixed (full-scale)
#define AMPLD_CTRL_ENV_VELO         1    // Output ampld control by Env * Velocity
#define AMPLD_CTRL_EXPRESS          2    // Output ampld control by Expression (CC2/7/11)
#define AMPLD_CTRL_AUTO             3    // Output ampld control automatic:- 
                                         // -- by Exprn if REMI handset connected, else
                                         // -- by Env * Velocity

typedef struct Instrument_Preset_Descriptor
{
    uint8   PatchNumber;              // REMI synth patch ID # for this Preset
    uint8   MidiProgram;              // MIDI OUT program (voice) for this Preset
    uint8   VibratoMode;              // Vibrato Control Mode
    int8    PitchTranspose;           // Pitch transpose/offset semitones (+/-24)

} InstrumentPreset_t;

// Note 1:  EffectSwitchCC = 0:None (disabled), 65:Portamento, 68:Legato, 85:Vibrato


typedef struct Eeprom_block0_structure
{
    uint32  checkDword;               // Constant value used to check data integrity
    ////
    uint16  MidiInBaudrate;           // MIDI IN baudrate (dflt 31250, max. 57600)
    uint8   MidiInMode;               // MIDI IN mode, 2 or 4 only; dflt: 2 (Omni-Mono)
    uint8   MidiInChannel;            // MIDI IN channel, range: 1..16, default: 1
    uint8   MidiInExpressionCCnum;    // MIDI IN breath/pressure CC number, dflt: 2
    
    uint8   MidiOutEnabled;           // MIDI OUT messages (all) enabled (dflt: 0)
    uint8   MidiOutChannel;           // MIDI OUT channel, range: 1..16, default: 1
    uint8   MidiOutExpressionCCnum;   // MIDI OUT breath/pressure CC number, dflt: 2
    uint8   MidiOutModnEnabled;       // MIDI OUT modulation messages enabled (dflt 1)
    
    uint8   ExpressionGainAdjust;     // MIDI Expression gain adjustment (25..250 %)
    uint8   PitchBendCtrlMode;        // Pitch-Bend control mode (MIDI, analog CV, etc)
    uint16  PitchBendRange;           // Pitch-Bend range, cents (0..1200)
    uint8   ReverbAtten_pc;           // Reverberation attenuator gain (1..100 %)
    uint8   ReverbMix_pc;             // Reverberation wet/dry mix (1..100 %)
    uint8   AudioAmpldControlMode;    // Ampld ctrl = 0:Fixed, 1:Env*Vel, 2:Exprn, 3:Auto
    uint8   PresetLastSelected;       // Preset last selected (0..7)
    
    // Calibration param's (not settable via "config" cmd; use "set" cmd)
    float   FilterInputAtten;         // 
    float   FilterOutputGain;         //
    float   NoiseFilterGain;          //
    //
    PatchParamTable_t  UserPatch;     // User-programmable patch parameters
    WaveformDesc_t     UserWaveform;  // User Wave-table descriptor
    //
    uint32  EndOfDataBlockCode;       // Last entry, to test if block format has changed

} EepromBlock0_t;


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


#endif // REMI_SYNTH_CONFIG_H
