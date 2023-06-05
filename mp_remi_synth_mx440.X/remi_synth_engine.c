/*
 * ================================================================================================
 *
 * Module:       remi_synth_engine.c
 *
 * Overview:     REMI sound synthesizer implementation.
 *
 * Author:       M.J.Bauer, Copyright 2016..2023++  All rights reserved
 *
 * Reference:    www.mjbauer.biz/Build_the_REMI_synth.htm
 *
 * ================================================================================================
 */
#include <sys/attribs.h>    // For interrupt handlers

#include "remi_synth_main.h"
#include "remi_synth_def.h"

PRIVATE  void   WaveTableSelect(uint8 osc_num, uint8 wave_id);
PRIVATE  void   AmpldEnvelopeShaper();
PRIVATE  void   AudioLevelController();
PRIVATE  void   ContourEnvelopeShaper();
PRIVATE  void   OscFreqModulation();
PRIVATE  void   OscMixRatioModulation();
PRIVATE  void   NoiseLevelControl();
PRIVATE  void   FilterFrequencyControl();
PRIVATE  void   LowFrequencyOscillator();
PRIVATE  void   VibratoRampGenerator();

int16    WaveTableBuffer[WAVE_TABLE_MAXIMUM_SIZE];  // signed 16-bit samples
fixed_t  ReverbDelayLine[REVERB_DELAY_MAX_SIZE];    // fixed-point samples

PatchParamTable_t  g_Patch;        // active (working) patch parameters

fixed_t  g_ExpressionPeak;         // Peak (max.) value of expression (fixed-pt)
int32    g_TraceBuffer[100][5];

uint16   g_Osc1WaveTableSize;      // Number of samples in OSC1 wave-table
uint16   g_Osc2WaveTableSize;      // Number of samples in OSC2 wave-table
float    g_Osc1FreqDiv;            // OSC1 frequency divider
float    g_Osc2FreqDiv;            // OSC2 frequency divider

// These global variables may be modified by the CLI 'set' command or by Control Panel pots;
// initialized at power-on/reset to values strored in EEPROM (with config param's)...
float  g_FilterInputAtten;      // Filter input atten/gain (.01 ~ 2.5)
float  g_FilterOutputGain;      // Filter output atten/gain (0.1 ~ 25)
float  g_NoiseFilterGain;       // Noise gen. gain adjustment (0.1 ~ 25)

static int16   *m_WaveTable1;             // Pointer to OSC1 wave-table
static int16   *m_WaveTable2;             // Pointer to OSC2 wave-table
static fixed_t  m_Osc1StepMedian;         // Median value of v_Osc1Step (as at Note-On)
static fixed_t  m_Osc2StepMedian;         // Median value of v_Osc2Step (as at Note-On)
static int32    m_LFO_Step;               // LFO "phase step" (fixed-point 24:8 bit format)
static fixed_t  m_LFO_output;             // LFO output signal, normalized, bipolar (+/-1.0)
static fixed_t  m_PressureLevel;          // Breath pressure, linear response (0..+1.0)
static fixed_t  m_ExpressionLevel;        // Expression (pressure) square-law (0..+1.0)
static fixed_t  m_ModulationLevel;        // Modulation level, normalized
static fixed_t  m_PitchBendFactor;        // Pitch-Bend factor, normalized
static uint8    m_PitchBendControl;       // Pitch-Bend control mode (Off, PBmsg, Exprn, CV)
static uint8    m_VibratoControl;         // 0:None, 1:FX.Sw, 2:CC(Mod.Lvr), 3:Auto
static fixed_t  m_RampOutput;             // Vibrato Ramp output level, normalized (0..1)
static fixed_t  m_AmpldEnvOutput;         // Amplitude envelope output (0 ~ 0.9995)

static fixed_t  m_ContourEnvOutput;       // Mixer contour output, normalized (0..1)
static fixed_t  m_AttackVelocity;         // Attack Velocity, normalized (0 ~ 0.999)
static bool     m_TriggerAttack;          // Signal to put ampld envelope into attack
static bool     m_TriggerRelease;         // Signal to put ampld envelope into release
static bool     m_TriggerContour;         // Signal to start contour envelope gen
static bool     m_LegatoNoteChange;       // Signal Legato note change to Vibrato func.
static uint8    m_Note_ON;                // TRUE if Note ON, ie. "gated", else FALSE
static uint8    m_NotePlaying;            // MIDI note number of note playing
static uint8    m_FilterAtten_pc;         // Filter input atten/gain (%)
static uint8    m_FilterGain_x10;         // Filter output gain x10 (1..250)
static uint8    m_NoiseGain_x10;          // Noise filter gain x10 (1..250)
static fixed_t  m_FiltCoeff_c[110];       // Bi-quad filter coeff. c  (a1 = -c)
static fixed_t  m_FiltCoeff_a2;           // Bi-quad filter coeff. a2
static fixed_t  m_FiltCoeff_b0;           // Bi-quad filter coeff. b0  (b2 = -b0)
static int      m_FilterIndex;            // Index into LUT: m_FiltCoeff_c[]
static int      m_RvbDelayLen;            // Reverb. delay line length (samples)
static fixed_t  m_RvbDecay;               // Reverb. decay factor
static uint16   m_RvbAtten;               // Reverb. attenuation factor (0..127)
static uint16   m_RvbMix;                 // Reverb. wet/dry mix ratio (0..127)

volatile bool     v_SynthEnable;      // Signal to enable synth engine
volatile int32    v_Osc1Angle;        // sample pos'n in wave-table, OSC1 [16:16]
volatile int32    v_Osc2Angle;        // sample pos'n in wave-table, OSC2 [16:16]
volatile int32    v_Osc1Step;         // sample pos'n increment, OSC1 [16:16 fixed-pt]
volatile int32    v_Osc2Step;         // sample pos'n increment, OSC2 [16:16 fixed-pt]
volatile uint16   v_Mix2Level;        // Osc2 Mixer input level x1000 (0..1000)
volatile fixed_t  v_NoiseLevel;       // Noise level control (normalized)
volatile fixed_t  v_OutputLevel;      // Audio output level control (normalized)
volatile fixed_t  v_coeff_b0;         // Bi-quad filter coeff b0 (active)
volatile fixed_t  v_coeff_b2;         // Bi-quad filter coeff b2 (active)
volatile fixed_t  v_coeff_a1;         // Bi-quad filter coeff a1 (active)
volatile fixed_t  v_coeff_a2;         // Bi-quad filter coeff a2 (active)
volatile bool     v_Clipping;         // Mixer output clipping (flag)
volatile uint32   v_ISRexecTime;      // ISR execution time (core cycle count)


// Look-up table giving frequencies of notes on the chromatic scale.
// The array covers a 9-octave range beginning with C0 (MIDI note number 12),
// up to C9 (120).  Subtract 12 from MIDI note number to get table index.
// Table index range:  [0]..[108]
//
const  float  m_NoteFrequency[] =
{
    // C0      C#0       D0      Eb0       E0       F0      F#0       G0
    16.3516, 17.3239, 18.3540, 19.4455, 20.6017, 21.8268, 23.1247, 24.4997,
    // Ab0      A0      Bb0       B1       C1      C#1       D1      Eb1
    25.9566, 27.5000, 29.1353, 30.8677, 32.7032, 34.6478, 36.7081, 38.8909,
    // E1       F1      F#1       G1      Ab1       A1      Bb1       B1
    41.2034, 43.6535, 46.2493, 48.9994, 51.9131, 55.0000, 58.2705, 61.7354,
    // C2      C#2       D2      Eb2       E2       F2      F#2       G2
    65.4064, 69.2957, 73.4162, 77.7817, 82.4069, 87.3071, 92.4986, 97.9989,
    // Ab2      A2      Bb2       B2       C3      C#3       D3      Eb3
    103.826, 110.000, 116.541, 123.471, 130.813, 138.591, 146.832, 155.563,
    // E3       F3      F#3       G3      Ab3       A3      Bb3       B3
    164.814, 174.614, 184.997, 195.998, 207.652, 220.000, 233.082, 246.942,
    // C4      C#4       D4      Eb4       E4       F4      F#4       G4
    261.626, 277.183, 293.665, 311.127, 329.628, 349.228, 369.994, 391.995,
    // Ab4      A4      Bb4       B4       C5      C#5       D5      Eb5
    415.305, 440.000, 466.164, 493.883, 523.251, 554.365, 587.330, 622.254,
    // E5       F5      F#5       G5      Ab5       A5      Bb5       B5
    659.255, 698.456, 739.989, 783.991, 830.609, 880.000, 932.328, 987.767,
    // C6      C#6       D6      Eb6       E6       F6      F#6       G6
    1046.50, 1108.73, 1174.66, 1244.51, 1318.51, 1396.91, 1479.98, 1567.98,
    // Ab6      A6      Bb6       B6       C7      C#7       D7      Eb7
    1661.22, 1760.00, 1864.66, 1975.53, 2093.00, 2217.46, 2349.32, 2489.02,
    // E7       F7      F#7       G7      Ab7       A7      Bb7       B7
    2637.02, 2793.83, 2959.96, 3135.96, 3322.44, 3520.00, 3729.31, 3951.07,
    // C8      C#8       D8      Eb8       E8       F8      F#8       G8
    4186.01, 4434.92, 4698.64, 4978.04, 5274.04, 5587.66, 5919.92, 6271.92,
    // Ab8      A8      Bb8       B8       C9
    6644.88, 7040.00, 7458.62, 7902.14, 8372.02
};


/*`````````````````````````````````````````````````````````````````````````````````````````````````
 * Function:     Prepare REMI synth tone generator to play a note.
 *
 * Overview:     This function must be called following any change in the synth patch
 *               or synth configuration parameter, before playing a note.
 *
 * Note:         On entry, and exit, the signal v_SynthEnable is set False.
 */
void  SynthPrepare()
{
    static  bool prepDone = FALSE;
    float   res, res_sq, freq_rat;
    float   rvbDecayFactor;
    float   pi_2 = 2.0f * 3.14159265f;
    int     idx;
    int     preset = g_Config.PresetLastSelected;

    v_SynthEnable = 0;    // Disable the synth tone-generator
    m_Note_ON = FALSE;    // no note playing
    
    if (!prepDone)  // One-time initialisation at power-on/reset
    {
        g_FilterInputAtten = g_Config.FilterInputAtten;  // persistent param
        g_FilterOutputGain = g_Config.FilterOutputGain;  // persistent param
        g_NoiseFilterGain = g_Config.NoiseFilterGain;    // persistent param
        // Calculate reverb delay-line constants...
        m_RvbDelayLen = (int) (REVERB_LOOP_TIME_SEC * SAMPLE_RATE_HZ);  // samples
        rvbDecayFactor = (float) REVERB_LOOP_TIME_SEC / REVERB_DECAY_TIME_SEC;
        m_RvbDecay = FloatToFixed( powf(0.001f, rvbDecayFactor) );
        prepDone = TRUE;
    }

    WaveTableSelect(1, g_Patch.Osc1WaveTable);
    WaveTableSelect(2, g_Patch.Osc2WaveTable);

    m_VibratoControl = g_Preset.Descr[preset].VibratoMode;
    m_PitchBendControl = g_Config.PitchBendCtrlMode; 
    m_ContourEnvOutput = IntToFixedPt(g_Patch.ContourStartLevel) / 100;
    m_RvbAtten = ((uint16)g_Config.ReverbAtten_pc << 7) / 100;  // = 0..127
    m_RvbMix = ((uint16)g_Config.ReverbMix_pc << 7) / 100;  // = 0..127

    // Find coefficients for bi-quad filter according to patch Filter Resonance
    res = (float) g_Patch.FilterResonance / 10000;   // range 0 ~ 0.9999
    res_sq = res * res;
    m_FiltCoeff_a2 = FloatToFixed(res_sq);
    m_FiltCoeff_b0 = FloatToFixed(0.5f - (res_sq / 2.0f));

    // Coeff a1 (a1 = -c) is both resonance and frequency dependent...  
    for (idx = 0 ; idx <= 108 ; idx++)    // populate LUT
    {
        freq_rat = m_NoteFrequency[idx] / SAMPLE_RATE_HZ;   // Fr = Fc / Fs
        m_FiltCoeff_c[idx] = FloatToFixed(2.0 * res * cosf(pi_2 * freq_rat));
    }
}


/*`````````````````````````````````````````````````````````````````````````````````````````````````
 * Function:     Select (activate) a wave-table for a given synth oscillator.
 *               This then becomes the "active" wave-table, over-riding the table
 *               selected by any prior call to SynthPatchSelect().
 *
 * Entry args:   osc_num = oscillator assigned to wave-table, 1 -> OSC1, 2 -> OSC2
 *               wave_id = ID number of wave-table, range 0..250
 *
 * Note:         wave_id = 0 selects the "User Wave-table" (RAM buffer) ^
 *               wave_id > 0 selects a flash-based wave-table
 * 
 *             ^ The User Wave-table is regenerated in the RAM buffer by the function
 *               SynthPatchSelect() from the user waveform descriptor in EEPROM.
 *               The User Wave-table may be replaced by the Wave-table Creator utility.
 */
PRIVATE  void  WaveTableSelect(uint8 osc_num, uint8 wave_id)
{
    if (wave_id == 0)  // User wave-table -- regenerated in RAM buffer
    {
        if (osc_num == 1)  m_WaveTable1 = (int16 *) WaveTableBuffer; 
        else if (osc_num == 2)  m_WaveTable2 = (int16 *) WaveTableBuffer; 
    }
    else if (wave_id <= GetHighestWaveTableID())  // Flash-based wave-table
    {
        if (osc_num == 1)
        {
            g_Osc1WaveTableSize = g_FlashWaveTableDef[wave_id].Size;
            g_Osc1FreqDiv = g_FlashWaveTableDef[wave_id].FreqDiv;
            m_WaveTable1 = (int16 *) g_FlashWaveTableDef[wave_id].Address;
        }
        else if (osc_num == 2)
        {
            g_Osc2WaveTableSize = g_FlashWaveTableDef[wave_id].Size;
            g_Osc2FreqDiv = g_FlashWaveTableDef[wave_id].FreqDiv;
            m_WaveTable2 = (int16 *) g_FlashWaveTableDef[wave_id].Address;
        }
    }
}


/*`````````````````````````````````````````````````````````````````````````````````````````````````
 * Function:     Copies patch parameters from a given pre-defined patch table in flash
 *               program memory to the "active" patch parameter table in data memory, except
 *               if patchNum is 0, load last saved 'User Patch' from EEPROM image.
 *               If the given patch ID number cannot be found, the function will copy
 *               parameters from a "default" patch (idx == 0) and return ERROR (-1).
 *
 * Entry args:   patchNum = ID number of patch parameter table to be set up.
 *               <!> This is *not* an index into the patch definitions array, g_PatchProgram[].
 *
 * Return val:   ERROR (-1) if the given patch ID cannot be found, else OK (0).
 */
short  SynthPatchSelect(int patchNum)
{
    short  status = 0;
    int    i;
    int    patchCount = GetNumberOfPatchesDefined();

    if (patchNum != 0)
    {
        for (i = 0;  i < patchCount;  i++)
        {
            if (g_PatchProgram[i].PatchNumber == patchNum)  break;
        }

        if (i >= patchCount)   // patchNum not found -- load default patch
        {
            i = 0;
            status = ERROR;
        }

        memcpy(&g_Patch, &g_PatchProgram[i], sizeof(PatchParamTable_t));
    }
    else  // if (patchNum == 0)
    {
        // Copy User Patch (persistent data in EEPROM) to active patch
        memcpy(&g_Patch, &g_Config.UserPatch, sizeof(PatchParamTable_t));
    }
    
    // Restore User Wave-table in RAM buffer in case g_Patch.Osc#WaveTable == 0:
    GenerateWaveTable( (WaveformDesc_t *) &g_Config.UserWaveform );
    //
    if (g_Patch.Osc1WaveTable == 0)  // OSC1 needs User Wave-table
    {
        g_Osc1WaveTableSize = g_Config.UserWaveform.Size;
        g_Osc1FreqDiv = g_Config.UserWaveform.FreqDiv;
        m_WaveTable1 = (int16 *) WaveTableBuffer; 
    }
    if (g_Patch.Osc2WaveTable == 0)  // OSC2 needs User Wave-table
    {
        g_Osc2WaveTableSize = g_Config.UserWaveform.Size;
        g_Osc2FreqDiv = g_Config.UserWaveform.FreqDiv;
        m_WaveTable2 = (int16 *) WaveTableBuffer; 
    }   

    // Ensure minimum values are assigned to envelope transition times...
    // (except for peak-hold time, which may be zero)
    if (g_Patch.AmpldEnvAttack_ms < 5) g_Patch.AmpldEnvAttack_ms = 5;
    if (g_Patch.AmpldEnvDecay_ms < 5) g_Patch.AmpldEnvDecay_ms = 5;
    if (g_Patch.AmpldEnvRelease_ms < 5) g_Patch.AmpldEnvRelease_ms = 5;
    if (g_Patch.ContourDelay_ms < 5) g_Patch.ContourDelay_ms = 5;
    if (g_Patch.ContourRamp_ms < 5) g_Patch.ContourRamp_ms = 5;
    if (g_Patch.LFO_RampTime < 5) g_Patch.LFO_RampTime = 5;

    SynthPrepare();

    return  status;
}


/*
 * Function:     If a note is already playing, perform a Legato note change;
 *               otherwise initiate a new note.
 *
 * Entry args:   noteNum  = MIDI standard note number, range: 12 ~ 120 (C0..C9),
 *                          e.g. note #60 = C4 = middle-C.
 *               velocity = attack velocity (usage dependent on synth settings)
 *
 * When a new note is initiated, the function prepares the synth wave-table oscillators
 * to play the given note, sets filter characteristics according to patch parameters,
 * then triggers the envelope shapers to enter the 'Attack' phase.
 */
void  SynthNoteOn(uint8 noteNum, uint8 velocity)
{
    int32  oscFreqLFO;  // 24:8 bit fixed-point
    
    if (!m_Note_ON)  // Note OFF -- Initiate a new note...
    {
        SynthNoteChange(noteNum);  // Set OSC1 and OSC2 frequencies, etc

        oscFreqLFO = (((int) g_Patch.LFO_Freq_x10) << 8) / 10;  // set LFO freq.
        m_LFO_Step = (oscFreqLFO * SINE_WAVE_TABLE_SIZE) / 1000;  // LFO Fs = 1000Hz
        m_AmpldEnvOutput = 0;   // Zero the ampld envelope output signal

        // A square-law curve is applied to velocity
        m_AttackVelocity = IntToFixedPt((int) velocity) / 128;  // normalized
        m_AttackVelocity = MultiplyFixed(m_AttackVelocity, m_AttackVelocity);  // squared
        
        // Refresh synth working variables from global (non-patch) settable params.
        m_NoiseGain_x10 = (uint8) (g_NoiseFilterGain * 10);
        m_FilterAtten_pc = (uint8) (g_FilterInputAtten * 100); 
        m_FilterGain_x10 = (uint8) (g_FilterOutputGain * 10); 

        TIMER2_IRQ_DISABLE();
        v_Osc1Angle = 0;
        v_Osc2Angle = 0;
        v_Mix2Level = (1000 * (int) g_Patch.MixerOsc2Level) / 100;
        v_NoiseLevel = 0;   
        v_OutputLevel = 0;
        TIMER2_IRQ_ENABLE();

        m_LegatoNoteChange = 0;    // Not a Legato event
        m_TriggerAttack = 1;       // Let 'er rip, Boris
        m_TriggerContour = 1;
        v_SynthEnable = 1; 
    }
    else  // Note already playing -- do legato note change
    {
        SynthNoteChange(noteNum);  // Adjust OSC1 and OSC2 frequencies
        m_LegatoNoteChange = 1;    // Signal Note-Change event (for vibrato fn)
    }
}


/*
 * Function:     Set the pitch of a note to be initiated, or change pitch of the note
 *               in progress, without affecting the amplitude envelope (i.e. no re-attack).
 *               This function may be used where a "legato" effect is required.
 *               (See also: SynthNoteOn() function.)
 *
 * Entry args:   noteNum = MIDI standard note number. (Note #60 = C4 = middle-C.)
 *               The REMI synth supports note numbers in the range: 12 (C0) to 120 (C9).
 *
 * The actual perceived pitch depends on the Frequency Divider values and the dominant
 * partial(s) in the waveform(s). Normally, the Osc.Freq.Div parameter value is chosen to
 * match the wave-table, so that the perceived pitch corresponds to the MIDI note number.
 */
void  SynthNoteChange(uint8 noteNum)
{
    float   osc1Freq, osc2Freq;
    fixed_t osc1Step, osc2Step;
    float   osc2detune;      // ratio:  osc2Freq / osc1Freq;
    fixed_t detuneNorm;
    int     filterIdx;    // unit = MIDI note number (semitone)
    int     cents, noteTransposed;
    int     preset = g_Config.PresetLastSelected;

    // Apply PRESET Pitch Transpose parameter
    noteTransposed = (int) noteNum + g_Preset.Descr[preset].PitchTranspose;

    // Ensure note number is within synth range (12 ~ 120)
    noteTransposed &= 0x7F;
    if (noteTransposed > 120)  noteTransposed -= 12;   // too high
    if (noteTransposed < 12)   noteTransposed += 12;   // too low

    noteNum = noteTransposed;
    m_NotePlaying = noteNum;
    m_Note_ON = TRUE;

    // Convert MIDI note number to frequency (Hz);  apply OSC1 freq.divider param.
    osc1Freq = m_NoteFrequency[noteNum - 12] / g_Osc1FreqDiv;

    // Calculate detune factor as a fraction of an octave
    cents = g_Patch.Osc2Detune % 1200;     // Limit +/- 1 octave
    detuneNorm = (IntToFixedPt(1) * cents) / 1200;  // range +/-1.000
    osc2detune = FixedToFloat(Base2Exp(detuneNorm));
    osc2Freq = osc1Freq * osc2detune;      // Apply OSC2 detune factor
    osc2Freq = osc2Freq / g_Osc2FreqDiv;   // Apply OSC2 Freq.Divider parameter

    // Initialize oscillator variables for use in audio ISR
    osc1Step = (int32)((osc1Freq * g_Osc1WaveTableSize * 65536) / SAMPLE_RATE_HZ);
    osc2Step = (int32)((osc2Freq * g_Osc2WaveTableSize * 65536) / SAMPLE_RATE_HZ);
    
    m_Osc1StepMedian = osc1Step;  // for Osc FM (vibrato, pitch-bend, etc)
    m_Osc2StepMedian = osc2Step;
    
    // Calculate filter corner freq (pitch offset) for new note...
    if (g_Patch.FilterNoteTrack)  // case 1: Note Tracking enabled
    {
        filterIdx = (noteNum - 12) + g_Patch.FilterFrequency;  // MIDI note #, variable
        if (filterIdx < 0)  filterIdx = 0;      // Min at C0 (~16Hz)
        if (filterIdx > 108)  filterIdx = 108;  // Max at C9 (~8kHz)
        m_FilterIndex = filterIdx;
    }
    else  // case 2: Note Tracking disabled
    {
        m_FilterIndex = g_Patch.FilterFrequency;
    }
    
    // Use Fc = 16 Hz (minimum) for pitched noise option
    if (g_Patch.NoiseMode & NOISE_PITCHED)  m_FilterIndex = 0;  

    TIMER2_IRQ_DISABLE();  
    v_Osc1Step = osc1Step;
    v_Osc2Step = osc2Step;
    // Set filter coefficient values to be applied in the audio ISR
    v_coeff_b0 = m_FiltCoeff_b0;
    v_coeff_b2 = 0 - m_FiltCoeff_b0;      // b2 = -b0
    v_coeff_a1 = 0 - m_FiltCoeff_c[m_FilterIndex];  // a1 = -c
    v_coeff_a2 = m_FiltCoeff_a2;
    TIMER2_IRQ_ENABLE();
}


/*
 * Function:     End the note playing, if it matches the given note number.
 *
 * Entry args:   noteNum = MIDI standard note number of note to be ended.
 * 
 * If noteNum == 0, the note will be terminated regardless of m_NotePlaying.  
 * This deviation from the MIDI standard is provided to support the REMI 2 handset.
 *
 * The function puts envelope shapers into the 'Release' phase. The note will be
 * terminated by the synth process (B/G task) when the release time expires, or if
 * a new note is initiated prior.
 */
void  SynthNoteOff(uint8 noteNum)
{
    int   noteTransposed;
    int   preset = g_Config.PresetLastSelected;

    // Apply PRESET Pitch Transpose parameter
    noteTransposed = (int) noteNum + g_Preset.Descr[preset].PitchTranspose;

    // Ensure note number is within synth range (12 ~ 108)
    noteTransposed &= 0x7F;
    if (noteTransposed > 120)  noteTransposed -= 12;   // too high
    if (noteTransposed < 12)   noteTransposed += 12;   // too low

    if (noteNum == 0 || noteTransposed == m_NotePlaying)
    {
        m_TriggerRelease = 1;
        m_Note_ON = FALSE;
    }
}


/*
 * Function:     Set the "Pressure Level" according to a given data value.
 *               Equivalent to MIDI Control Change message (CC# = 02, 07 or 11).
 *
 * Entry args:   data14 = MIDI expression/pressure value (14 bits, unsigned).
 *
 * Outputs:      (fixed_t) m_PressureLevel = normalized pressure level (0..+1.0)
 *                             capped at 0.99 FS, linear response
 *                                                ```````````````
 *               (fixed_t) m_ExpressionLevel = normalized pressure level (0..+1.0)
 *                             capped at 0.99 FS, square-law response
 *                                                ```````````````````
 * Note:         A square-law is applied to the data value to approximate an exponential
 *               response curve. The output pressure level is fixed point, normalized.
 */
void   SynthExpression(unsigned data14)
{
    uint32  ulval;
    fixed_t level;
    fixed_t levelMax = (IntToFixedPt(1) * 99) / 100;
    
    ulval = data14 << 6;  // scale to 20 bits (fractional part)
    level = (fixed_t) ulval;  
    level = (level * g_Config.ExpressionGainAdjust) / 100;  // adjust level
    if (level > levelMax) level = levelMax;  // cap at 0.99
    m_PressureLevel = level;

    ulval = ((uint32) data14 * data14) / 16384;  // apply square law
    ulval = ulval << 6;   // scale to 20 bits (fractional part)
    level = (fixed_t) ulval; 
    level = (level * g_Config.ExpressionGainAdjust) / 100;  // adjust level
    if (level > levelMax) level = levelMax;  // cap at 0.99
    if (level > g_ExpressionPeak)  g_ExpressionPeak = level;  // diagnostic
    m_ExpressionLevel = level;  
}


/*
 * Function:     Modify pitch of note in progress according to Pitch-Bend data value.
 *
 * Entry args:   bipolarPosn = signed integer representing Pitch Bend lever position,
 *                        in the range +/-8000 (14 LS bits).  Centre pos'n is 0.
 *
 * Affected:     m_PitchBendFactor, which is processed by the real-time synth function
 *               OscFreqModulation() while a note is in progress.
 *
 * Todo: *****   Test this function using MIDI pitch bend data !!  ******
 */
void   SynthPitchBend(int bipolarPosn)
{
    // Scale lever position (arg) according to patch 'PitchBendRange' param.
    // PitchBendRange may be up to 1200 cents (ie. 1 octave maximum).
    int  posnScaled = (bipolarPosn * g_Config.PitchBendRange) / 1200;
    
    // Convert to 20-bit *signed* fixed-point fraction  (13 + 7 = 20 bits)
    m_PitchBendFactor = (fixed_t) (posnScaled << 7);
}


/*
 * Function:     Control synth effect(s) according to MIDI Modulation data (CC#1);
 *               e.g. vibrato depth, noise level, filter freq., etc.
 *               The effect(s) to be controlled is a function of the synth patch^.
 *
 * Entry args:   data14 = unsigned integer representing Modulation Lever position;
 *                        range 0..16383 (= 2^14 - 1).
 *
 * Output:       m_ModulationLevel, normalized fixed-pt number in the range 0..+1.0.
 *
 */
void   SynthModulation(unsigned data14)
{
    if (data14 < (16 * 1024))  
        m_ModulationLevel = (fixed_t) ((uint32) data14 << 6);  
    else   m_ModulationLevel = FIXED_MAX_LEVEL;
}


/*`````````````````````````````````````````````````````````````````````````````````````````````````
 * Function:  SynthProcess()
 *
 * Overview:  Periodic background task called at 1ms intervals which performs most of the
 *            real-time sound synthesis computations, except those which need to be executed
 *            at the PCM audio sampling rate; these are done by the Timer_2 ISR.
 *
 * This task implements the envelope shapers, oscillator pitch bend and vibrato (LFO), mixer
 * input ratio control, DSP filter frequency control, audio output amplitude control, etc.
 *
 * Some processing is done at 1ms intervals (1000Hz), while other parts are done at longer
 * intervals, e.g. 5ms/200Hz, where timing resolution is not so critical and/or more intensive
 * computation, e.g. floating point arithmetic, is needed. Speed-critical computations use
 * 32-bit fixed-point arithmetic.
 *
 * While diagnostic mode is active, the synth process is suspended to allow low-level tests
 * to run without being disrupted by continuous synth functions.
 */
void   SynthProcess()
{
    static  int   count5ms;
    
    if (!v_SynthEnable)  return;  // Synth process and audio ISR inactive
    
    LowFrequencyOscillator();
    AmpldEnvelopeShaper();
    AudioLevelController();

    if (++count5ms == 5)     // 5ms process interval (200Hz)
    {
        count5ms = 0;
        ContourEnvelopeShaper();
        VibratoRampGenerator();
        OscFreqModulation();       // Process pitch-bend and/or vibrato
        OscMixRatioModulation();   // Wave-table morphing routine
        NoiseLevelControl();       // Noise level control routine
        FilterFrequencyControl();  // Bi-quad filter freq. control
    }
}


/*
 * Function:  AmpldEnvelopeShaper()
 *
 * Overview:  Audio amplitude envelope shaper.
 *            Routine called by the Synth Process at 1ms intervals.
 *
 * Output:    (fixed_t) m_AmpldEnvOutput = Envelope output level, norm. (0 ~ 0.9995)
 *
 */
PRIVATE  void   AmpldEnvelopeShaper()
{
    static  uint8    envSegment;          // Envelope segment (aka "phase")
    static  uint32   envPhaseTimer;       // Time elapsed in envelope phase (ms)
    static  fixed_t  ampldSustainLevel;   // Ampld Env. sustain level (0 ~ 0.9995)
    static  fixed_t  timeConstant;        // 20% of Decay or Release time (ms))
    static  fixed_t  ampldDelta;          // Step change in Env Ampld per 1ms
    static  fixed_t  ampldMaximum;        // Peak value of Envelope Ampld

    if (m_TriggerAttack)
    {
        m_TriggerAttack = 0;
        m_TriggerRelease = 0;
        envPhaseTimer = 0;
        ampldSustainLevel = IntToFixedPt((int) g_Patch.AmpldEnvSustain) / 100;  // normalized
        ampldMaximum = FIXED_MAX_LEVEL;  // for Peak-Hold phase
        if (g_Patch.AmpldEnvPeak_ms == 0)  ampldMaximum = ampldSustainLevel;  // No Peak-Hold phase
        ampldDelta = ampldMaximum / g_Patch.AmpldEnvAttack_ms;  // step change in 1ms
        envSegment = ENV_ATTACK;
    }

    if (m_TriggerRelease)
    {
        m_TriggerRelease = 0;
        timeConstant = g_Patch.AmpldEnvRelease_ms / 5;
        envPhaseTimer = 0;
        envSegment = ENV_RELEASE;
    }

    switch (envSegment)
    {
    case ENV_IDLE:          // Idle - zero output level
    {
        m_AmpldEnvOutput = 0;
        break;
    }
    case ENV_ATTACK:        // Attack - linear ramp up to peak - or to the sustain level
    {
        if (m_AmpldEnvOutput < ampldMaximum) m_AmpldEnvOutput += ampldDelta;
        if (++envPhaseTimer >= g_Patch.AmpldEnvAttack_ms)  // attack time ended
        {
            m_AmpldEnvOutput = ampldMaximum;
            envPhaseTimer = 0;
            if (g_Patch.AmpldEnvPeak_ms == 0)  envSegment = ENV_SUSTAIN;  // skip peak and decay
            else  envSegment = ENV_PEAK_HOLD;    // run all phases
        }
        break;
    }
    case ENV_PEAK_HOLD:     // Peak - constant output level (0.999)
    {
        if (++envPhaseTimer >= g_Patch.AmpldEnvPeak_ms)  // Peak time ended
        {
            timeConstant = g_Patch.AmpldEnvDecay_ms / 5;  // for Decay phase
            envPhaseTimer = 0;
            envSegment = ENV_DECAY;
        }
        break;
    }
    case ENV_DECAY:         // Decay - exponential ramp down to sustain level
    {
        ampldDelta = (m_AmpldEnvOutput - ampldSustainLevel) / timeConstant;
        if (ampldDelta == 0)  ampldDelta = FIXED_MIN_LEVEL;
        if (m_AmpldEnvOutput > ampldSustainLevel)  m_AmpldEnvOutput -= ampldDelta;
        // Allow 10 x time-constant for decay phase to complete
        if (++envPhaseTimer >= (g_Patch.AmpldEnvDecay_ms * 2))  envSegment = ENV_SUSTAIN;
        break;
    }
    case ENV_SUSTAIN:       // Sustain constant level - waiting for m_TriggerRelease
    {
        m_AmpldEnvOutput = ampldSustainLevel;
        break;
    }
    case ENV_RELEASE:       // Release - exponential ramp down to zero level
    {
        // timeConstant and envPhaseTimer are set by the trigger condition above.
        ampldDelta = m_AmpldEnvOutput / timeConstant;  // step change in 1ms
        if (ampldDelta == 0)  ampldDelta = FIXED_MIN_LEVEL;
        if (m_AmpldEnvOutput >= ampldDelta)  m_AmpldEnvOutput -= ampldDelta;
        // Allow 10 x time-constant for release phase to complete
        if (++envPhaseTimer >= (g_Patch.AmpldEnvRelease_ms * 2))
        {
            m_AmpldEnvOutput = 0;
            envPhaseTimer = 0;
            envSegment = ENV_IDLE;
        }
        break;
    }
    };  // end switch
}


/*
 * Function:  AudioLevelController()
 *
 * Overview:  This routine is called by the Synth Process at 1ms intervals.
 *            The output level is controlled (modulated) by one of a choice of options
 *            determined by the configuration parameter g_Config.AudioAmpldControlMode.
 *
 * Output:    (fixed_t) v_OutputLevel : normalized output level (0..+1.0)
 *            The output variable is used by the audio ISR to control the audio ampld,
 *            except for the reverberated signal which may continue to sound.
 */
PRIVATE  void   AudioLevelController()
{
    static  fixed_t  outputAmpld;         // Audio output level, normalized
    static  fixed_t  smoothExprnLevel;    // Expression level, normalized, smoothed
    static  uint8    clipLEDstate;
    static  uint32   clipLEDduty_ms;
    fixed_t  exprnLevel;

    if ((g_Config.AudioAmpldControlMode == AMPLD_CTRL_EXPRESS)  // modes 2 & 3
    || ((g_Config.AudioAmpldControlMode == AMPLD_CTRL_AUTO) && isHandsetConnected()))
    {
        // After Note-Off, MIDI Pressure/Expression Level is assumed to be zero
        if (m_Note_ON)  exprnLevel = m_ExpressionLevel;
        else  exprnLevel = 0;
        
        // Apply IIR smoothing filter to eliminate abrupt changes (K = 1/8)
        smoothExprnLevel -= smoothExprnLevel >> 3;
        smoothExprnLevel += exprnLevel >> 3;
        outputAmpld = smoothExprnLevel;
    }
    else if ((g_Config.AudioAmpldControlMode == AMPLD_CTRL_ENV_VELO)  // modes 1 & 3
    || ((g_Config.AudioAmpldControlMode == AMPLD_CTRL_AUTO) && !isHandsetConnected()))
    {
        outputAmpld = MultiplyFixed(m_AmpldEnvOutput, m_AttackVelocity);
    }
    else    // default  ...................................... // mode 0
    {
        if (m_Note_ON)  outputAmpld = FIXED_MAX_LEVEL;
        else  outputAmpld = 0;
    }
    
    if (outputAmpld > FIXED_MAX_LEVEL)  outputAmpld = FIXED_MAX_LEVEL;

    v_OutputLevel = outputAmpld;  // accessed by audio ISR
    
    // Check mixer output for clipping -- if so, pulse LED indicator for 50ms
    if (v_Clipping)
    {
        v_Clipping = 0;     // reset flag
        CLIPPING_LED_ON();
        clipLEDstate = 1;
        clipLEDduty_ms = 0;
    }
    if (clipLEDstate == 1 && clipLEDduty_ms >= 50)
    {
        CLIPPING_LED_OFF();
        clipLEDstate = 0;
    }
    clipLEDduty_ms++;
}


/*
 * Function:  ContourEnvelopeShaper()
 *
 * Overview:  Routine called by the Synth Process at 5ms intervals.
 *            All segments of the Contour Envelope are linear time-varying.
 *            The contour output may be used to control the oscillator mix ratio,
 *            noise level, filter corner frequency (Fc), or whatever.
 *
 * Output:    (fixed_t) m_ContourEnvOutput = output signal, normalized (0..+0.999)
 */
PRIVATE  void   ContourEnvelopeShaper()
{
    static  short    contourSegment;  // Contour envelope segment (aka phase)
    static  uint32   contourTimer;    // Time elapsed in active phase (ms)
    static  fixed_t  outputDelta;     // Step change in output level per 5ms
    static  fixed_t  holdLevel;       // Output level held at finish of ramp

    if (m_TriggerContour)  // Note-On event
    {
        m_TriggerContour = 0;
        m_ContourEnvOutput = IntToFixedPt(g_Patch.ContourStartLevel) / 100;
        holdLevel = IntToFixedPt(g_Patch.ContourHoldLevel) / 100;
        contourTimer = 0;
        contourSegment = CONTOUR_ENV_DELAY;
    }

    switch (contourSegment)
    {
    case CONTOUR_ENV_IDLE:  // Waiting for trigger signal
    {
        break;
    }
    case CONTOUR_ENV_DELAY:  // Delay before ramp up/down segment
    {
        if (contourTimer >= g_Patch.ContourDelay_ms)  // Delay segment ended
        {
            outputDelta = ((holdLevel - m_ContourEnvOutput) * 5) / g_Patch.ContourRamp_ms;
            contourTimer = 0;
            contourSegment = CONTOUR_ENV_RAMP;
        }
        break;
    }
    case CONTOUR_ENV_RAMP:  // Linear ramp up/down from Start to Hold level
    {
        if (contourTimer >= g_Patch.ContourRamp_ms)  // Ramp segment ended
            contourSegment = CONTOUR_ENV_HOLD;
        else
            m_ContourEnvOutput += outputDelta;
        break;
    }
    case CONTOUR_ENV_HOLD:  // Hold constant level - waiting for Note-Off event
    {
        m_ContourEnvOutput = holdLevel;
        break;
    }
    };  // end switch

    contourTimer += 5;  // ms
}


/*
 * Function:     Synth LFO implementation.
 *
 * Called by SynthProcess() at 1ms intervals, this function generates a sinusoidal
 * waveform in real time.  LFO frequency is a patch parameter, g_Patch.VibratoFreq,
 * unsigned 8-bit value representing LFO freq * 10 Hz; range 1..250 => 0.1 to 25 Hz.
 *
 * Effective sample rate (Fs) is 1000 Hz.
 * 
 * The output of the LFO is m_LFO_output, a normalized fixed-point variable (+|-1.0)
 */
PRIVATE  void   LowFrequencyOscillator()
{
    static  int32  oscAngleLFO;  // 24:8 bit fixed-point
    fixed_t sample;
    int     waveIdx;

    waveIdx = oscAngleLFO >> 8;  // integer part of oscAngleLFO
    sample = (fixed_t) g_sinewave[waveIdx] << 5;  // normalized
    oscAngleLFO += m_LFO_Step;
    if (oscAngleLFO >= SINE_WAVE_TABLE_SIZE << 8)
        oscAngleLFO -= SINE_WAVE_TABLE_SIZE << 8;

    m_LFO_output = (fixed_t) sample;
}


/*
 * Function:     Vibrato Ramp Generator implementation.
 *
 * Called by the SynthProcess() at 5ms intervals, this function generates a linear ramp.
 *
 * The vibrato (LFO) delayed ramp is triggered by a Note-On event. 
 * If a Legato note change occurs, vibrato is stopped (fast ramp down) and the ramp delay
 * is re-started, so that vibrato will ramp up again after the delay.
 *
 * The delay and ramp-up times are both set by the patch parameter, g_Patch.LFO_RampTime,
 * so the delay time value is the same as the ramp-up time.  This works well enough.
 */
PRIVATE  void   VibratoRampGenerator()
{
    static  short   rampState = 0;
    static  uint32  delayTimer_ms;
    static  fixed_t rampStep;  // Step chnage in output per 5 ms

    if (rampState == 0)  // Idle - waiting for Note-On
    {
		rampStep = IntToFixedPt(5) / (int) g_Patch.LFO_RampTime;
        if (m_Note_ON)  { m_RampOutput = 0;  delayTimer_ms = 0;  rampState = 1; }
    }
    else if (rampState == 1)  // Delaying before ramp-up begins
    {
        if (delayTimer_ms >= g_Patch.LFO_RampTime)  rampState = 2;
        else  delayTimer_ms += 5;
    }
    else if (rampState == 2)  // Ramping up - hold at max. level (1.00)
    {
        if (m_RampOutput < FIXED_MAX_LEVEL)  m_RampOutput += rampStep;
        if (m_RampOutput > FIXED_MAX_LEVEL)  m_RampOutput = FIXED_MAX_LEVEL;
    }
    else  rampState = 0;  // Undefined state... reset
	
	// Check for Note-Off or Note-Change event while ramp is progressing
    if ((rampState != 0) && (!m_Note_ON || m_LegatoNoteChange))
    {
        if (m_LegatoNoteChange)  
		  { m_LegatoNoteChange = 0;  m_RampOutput = 0;  delayTimer_ms = 0;  rampState = 1; }
        else  rampState = 0;
    }
}


/*
 * Function:     Oscillator Frequency Modulation  (Pitch-bend, vibrato, etc)
 *
 * Called by SynthProcess() at 5ms intervals, this function modulates the pitch of
 * the wave-table oscillators according to a multiplier variable, smoothFreqMult,
 * which is continuously updated while a note is in progress.
 * 
 * The linear m_PitchBendFactor is transformed into a multiplier in the range 0.5 ~ 2.0.
 * Centre (zero) m_PitchBendFactor value should give a multplier value of 1.00.
 *
 * Note:  This function is used for low frequency modulation, up to about 25Hz,
 *        intended for Pitch Bend OR Vibrato (mutually exclusive).
 */
PRIVATE  void   OscFreqModulation()
{
    static  fixed_t  smoothFreqMult = IntToFixedPt(1);

    fixed_t  LFO_scaled, modnLevel;  // normalized quantities (range 0..+/-1.0)
    fixed_t  freqMult;

    if (m_VibratoControl == VIBRATO_BY_MODN_CC)  // Use Mod Lever position
        modnLevel = (m_ModulationLevel * g_Patch.LFO_FM_Depth) / 1200;
    
    if (m_VibratoControl == VIBRATO_AUTOMATIC)  // Use LFO ramp generator
        modnLevel = (m_RampOutput * g_Patch.LFO_FM_Depth) / 1200;

    if (m_VibratoControl && !m_PitchBendControl)  
    {
        LFO_scaled = MultiplyFixed(m_LFO_output, modnLevel); 
        freqMult = Base2Exp(LFO_scaled);   // range 0.5 ~ 2.0.
    }
    else if (m_PitchBendControl == PITCH_BEND_BY_MIDI_PB)  
    {
         freqMult = Base2Exp(m_PitchBendFactor);
    }
    else if (m_PitchBendControl == PITCH_BEND_BY_EXPRN_CC) 
    {
        modnLevel = (m_PressureLevel * g_Config.PitchBendRange) / 1200;
        freqMult = Base2Exp(modnLevel);
    }
    else if (m_PitchBendControl == PITCH_BEND_BY_ANALOG_CV) 
    {
        ;  // *****  todo !  *****
    }
    else  freqMult = IntToFixedPt( 1 );  // No pitch modulation

    if (m_VibratoControl || m_PitchBendControl)
    {
        smoothFreqMult -= smoothFreqMult >> 2;  // Tc = 4 * 5ms = 20ms (approx)
        smoothFreqMult += freqMult >> 2; 
        // Update real-time oscillator variables (accessed by audio ISR)
        v_Osc1Step = MultiplyFixed(m_Osc1StepMedian, smoothFreqMult);
        v_Osc2Step = MultiplyFixed(m_Osc2StepMedian, smoothFreqMult);
    }
}


/*
 * Oscillator Mix Ratio Modulation
 *
 * Called by the SynthProcess() routine at 5ms intervals, this function controls the ratio
 * of the 2 oscillator signals input to the wave mixer, according to a variable, mixRatio_pK,
 * which is the fraction (x1024) of the output of OSC2 relative to OSC1 fed into the mixer.
 *
 * The instantaneous value of the time-varying quantity mixRatio_pK is determined by the 
 * patch mixer control mode parameter, g_Patch.MixerControl.
 * The actual mixing operation is performed by the audio ISR, using the output variables.
 *
 * Output variable:   v_Mix2Level   (range 0..1000)
 */
PRIVATE  void  OscMixRatioModulation()
{
    fixed_t  LFO_scaled;
    fixed_t  modnLevel;
    fixed_t  mixRatioLFO;   // normalized
    int      mixRatio_pK;   // fraction of OSC2 in the mix x1000

    if (g_Patch.MixerControl == MIXER_CTRL_CONTOUR)
    {
        mixRatio_pK = IntegerPart(m_ContourEnvOutput * 1000);
    }
    else if (g_Patch.MixerControl == MIXER_CTRL_LFO)
    {
        // Use LFO depth param (0..1200) to set contour LFO modulation depth.
        modnLevel = (m_RampOutput * g_Patch.LFO_FM_Depth) / 1200;
        LFO_scaled = MultiplyFixed(m_LFO_output, modnLevel);  // range -1.0 ~ +1.0
        mixRatioLFO = (IntToFixedPt(1) + LFO_scaled) / 2;     // range  0.0 ~ +1.0
        mixRatio_pK = IntegerPart(mixRatioLFO * 1000);        // convert to 'per K' units
    }
    else if (g_Patch.MixerControl == MIXER_CTRL_EXPRESS)
    {
        mixRatio_pK = IntegerPart(m_PressureLevel * 1000);
    }
    else if (g_Patch.MixerControl == MIXER_CTRL_MODULN)
    {
        mixRatio_pK = IntegerPart(m_ModulationLevel * 1000);
    }
    else  // assume (g_Patch.MixerControl == MIXER_CTRL_FIXED) -- default
    {
        mixRatio_pK = (1000 * (int) g_Patch.MixerOsc2Level) / 100;  // fixed %
    }

    v_Mix2Level = mixRatio_pK;  // accessed by audio ISR
}


/*
 * Noise Generator Output level Control
 *
 * Called by the SynthProcess() routine at 5ms intervals, this function sets the noise
 * generator output level in real-time according to the noise control source (patch param).
 * The actual level control operation is performed by the audio ISR.
 *
 * Output variable:  v_NoiseLevel  (normalized fixed-point)
 */
PRIVATE  void   NoiseLevelControl()
{
    int  noiseCtrlSource = g_Patch.NoiseLevelCtrl & 7;  
    
    if (noiseCtrlSource == NOISE_LVL_FIXED)           // option 0 (Fixed %)
    {
        if ((g_Patch.NoiseMode & 3) == NOISE_WAVE_ADDED
        ||  (g_Patch.NoiseMode & 3) == NOISE_WAVE_MIXED )
            v_NoiseLevel = (IntToFixedPt( 1 ) * g_Patch.MixerOsc2Level) / 100;
    }
    else if (noiseCtrlSource == NOISE_LVL_AMPLD_ENV)  // option 1
        v_NoiseLevel = m_AmpldEnvOutput;
    else if (noiseCtrlSource == NOISE_LVL_LFO)        // option 2
        v_NoiseLevel = (m_LFO_output * g_Patch.LFO_FM_Depth) / 100;
    else if (noiseCtrlSource == NOISE_LVL_EXPRESS)    // option 3
        v_NoiseLevel = (m_ExpressionLevel / 2);
    else if (noiseCtrlSource == NOISE_LVL_MODULN)     // option 4
        v_NoiseLevel = (m_ModulationLevel * 70) / 100;
    else  v_NoiseLevel = 0;  // ..................... // default (Noise OFF)
        
}


/*
 * Bi-quad resonant filter centre frequency control (modulation)
 *
 * Called by the SynthProcess() routine at 5ms intervals, this function updates the
 * bi-quad IIR filter coefficient a1 in real-time according to the note pitch and the
 * filter frequency control (modulation) parameter(s).
 * The filter resonance cannot be changed while a note is in progress, hence other
 * filter coeff's do not need to be updated here.
 * The actual DSP filter algorithm is incorporated in the audio ISR.
 * 
 * Input variable:  m_FilterIndex = index into filter coeff LUT, varies with fc,
 *                                  determined at Note_ON and legato note change.
 *     
 * Output variables:  v_coeff_a1  = real-time filter coeff (fixed-point value)
 */
PRIVATE  void   FilterFrequencyControl()
{
    fixed_t  devn;   // deviation from quiescent value (+/-0.5 max)
    int  fc_idx;     // index into filter coeff (c) look-up table

    if (g_Patch.NoiseMode)  
        return;  // wave filter is bypassed while noise is enabled

    if (g_Patch.FilterControl == FILTER_CTRL_CONTOUR)  // mode 1
    {
        fc_idx = m_FilterIndex + IntegerPart(m_ContourEnvOutput * 108) - 50;
    }
    else if (g_Patch.FilterControl == FILTER_CTRL_LFO)  // mode 2
    {
        devn = (m_LFO_output * g_Patch.LFO_FM_Depth) / 200;  // deviation (+/-0.5 max)
        fc_idx = m_FilterIndex + IntegerPart(devn * 108);
    }
    //
    // else if (g_Patch.FilterControl == FILTER_CTRL_EXPRESS)  // mode 3  *** to do ***
    //
    else if (g_Patch.FilterControl == FILTER_CTRL_MODULN)  // mode 4
    {
        devn = (m_ModulationLevel * 50) / 100;  // deviation (+0.5 max)
        fc_idx = m_FilterIndex - IntegerPart(devn * 108);  // Fc decreases with mod'n level
    }
    else  fc_idx = m_FilterIndex;  // filter Fc is constant or note tracking (offset)

    if (fc_idx > 108)  fc_idx = 108;   // max. (~8 kHz)
    if (fc_idx < 0)  fc_idx = 0;       // min. (~16 Hz)

    v_coeff_a1 = 0 - m_FiltCoeff_c[fc_idx];  // a1 = -c
}


/*`````````````````````````````````````````````````````````````````````````````````````````````````
 * Function:     Timer_2 interrupt service routine (ISR)
 *
 * The ISR performs DSP synthesis computations which need to be executed at the PCM audio
 * sampling rate, as defined by SAMPLE_RATE_HZ (typ. 40 kHz).
 *
 * Signal (sample) computations use 32-bit [12:20] fixed-point arithmetic, except
 * that wave-table samples are stored as 16-bit signed integers. Wave-table samples are
 * converted to normalized fixed-point (20-bit fraction) by shifting left 5 bit places.
 * 
 * The Wave-table Oscillator algorithms use lower precision fixed-point (16:16 bits) to
 * avoid arithmetic overflow, which could occur with 12:20 bit precision.
 *
 * With sample rate = 40kHz, the periodic IRQ interval is 25us.  The ISR execution time
 * should be kept under 15us.  With Fclk = 80 MHz, 1us equals 80 core instruction cycles,
 * so the ISR may be up to about 15 * 80 = 1200 instructions, which is quite a lot!
 */
void  __ISR(_TIMER_2_VECTOR, IPL6AUTO)  Timer_2_IRQService(void)
{
    static  fixed_t  filter_in_1;         // bi-quad filter input delayed 1 sample
    static  fixed_t  filter_in_2;         // bi-quad filter input delayed 2 samples
    static  fixed_t  filter_out_1;        // bi-quad filter output delayed 1 sample
    static  fixed_t  filter_out_2;        // bi-quad filter output delayed 2 samples
    static  uint32   rand_last = 1;       // random number (NB: seed must be odd!)
    static  fixed_t  reverbPrev;          // previous sample from reverb delay line
    static  int      rvbIndex;            // index into ReverbDelayLine[]

    uint32   CC_Reg;
    uint32   entryTime;                   // ISR entry time (core cycle count)
    int      idx;                         // index into wave-tables
    fixed_t  osc1Sample, osc2Sample;      // outputs from OSC1 and OSC2
    fixed_t  noiseSample;                 // output from white noise algorithm
    fixed_t  noiseGenOut;                 // output from noise generator 
    fixed_t  waveRatio;                   // wave to noise ratio (0 ~ 1.0))
    fixed_t  mixerIn1, mixerIn2;          // inputs to wave-osc mixer
    fixed_t  waveMixerOut;                // output from wave-osc mixer 
    fixed_t  totalMixOut;                 // output from wave + noise mixers
    fixed_t  filterIn;                    // input to IIR bi-quad filter 
    fixed_t  filterOut;                   // output from IIR bi-quad filter 
    fixed_t  attenOut = 0;                // output from variable-gain attenuator
    fixed_t  reverbOut;                   // output from reverb delay line
    fixed_t  reverbLPF;                   // output from reverb loop filter
    fixed_t  finalOutput = 0;             // output sample (to PWM DAC) 
    uint16   spiDACdata;                  // SPI DAC register data

    READ_CPU_CORE_COUNT_REG(CC_Reg);
    entryTime = CC_Reg;

    if (v_SynthEnable)
    {
        // OSC1 Wave-table oscillator 
        idx = v_Osc1Angle >> 16;  // integer part of v_Osc1Angle
        osc1Sample = (fixed_t) m_WaveTable1[idx] << 5;  // normalize
        v_Osc1Angle += v_Osc1Step;
        if (v_Osc1Angle >= (g_Osc1WaveTableSize << 16))
            v_Osc1Angle -= (g_Osc1WaveTableSize << 16);

        // OSC2 Wave-table oscillator 
        idx = v_Osc2Angle >> 16;  // integer part of v_Osc2Angle
        osc2Sample = (fixed_t) m_WaveTable2[idx] << 5;  // normalize
        v_Osc2Angle += v_Osc2Step;
        if (v_Osc2Angle >= (g_Osc2WaveTableSize << 16))
            v_Osc2Angle -= (g_Osc2WaveTableSize << 16);
        
        // Wave Mixer -- add OSC1 and OSC2 samples, scaled according to mix ratio
        mixerIn1 = (osc1Sample * (1000 - v_Mix2Level)) >> 10;
        mixerIn2 = (osc2Sample * v_Mix2Level) >> 10;
        waveMixerOut = mixerIn1 + mixerIn2;

        // White noise generator -- Pseudo-random number algorithm...
        rand_last = (rand_last * 1103515245 + 12345) & 0x7FFFFFFF;  // unsigned
        noiseSample = (int32) (rand_last << 1);    // signed 32-bit value
        noiseSample = noiseSample >> 11;           // normalized fixed-pt (+/-1.0)

        if (g_Patch.NoiseMode)  // Noise enabled in patch
        {
            // Adjust noiseSample to a level which avoids overdriving the filter
            filterIn = (noiseSample * m_FilterAtten_pc) / 100;  
            // Apply filter algorithm
            filterOut =  MultiplyFixed(v_coeff_b0, filterIn);
            filterOut += MultiplyFixed(v_coeff_b2, filter_in_2);
            filterOut -= MultiplyFixed(v_coeff_a1, filter_out_1);
            filterOut -= MultiplyFixed(v_coeff_a2, filter_out_2);
            filter_in_2 = filter_in_1;  // update delayed samples
            filter_in_1 = filterIn;
            filter_out_2 = filter_out_1;
            filter_out_1 = filterOut;
            // Adjust noise filter output level to compensate for spectral loss
            filterOut = (filterOut * m_NoiseGain_x10) / 10;  

            // If enabled, Ring Modulate OSC2 output with filtered noise...
            if (g_Patch.NoiseMode & NOISE_PITCHED)   
                noiseGenOut = MultiplyFixed(filterOut, osc2Sample);  // Ring Mod.
            else  noiseGenOut = filterOut;   // unmodulated filtered noise
            
            // Add or mix noise with wave mixer output according to patch mode
            if ((g_Patch.NoiseMode & 3) == NOISE_WAVE_ADDED)  // Add noise to total mix
            {
                noiseGenOut = MultiplyFixed(noiseGenOut, v_NoiseLevel);
                totalMixOut = (waveMixerOut + noiseGenOut) / 2;  // avoid clipping
            }
            else if ((g_Patch.NoiseMode & 3) == NOISE_WAVE_MIXED)  // Ratiometric mix
            {
                waveRatio = IntToFixedPt(1) - v_NoiseLevel;  // wave:noise ratio (0 ~ 1.0)
                totalMixOut = MultiplyFixed(waveMixerOut, waveRatio);
                totalMixOut += MultiplyFixed(noiseGenOut, v_NoiseLevel);
            }
            else  totalMixOut = MultiplyFixed(noiseGenOut, v_NoiseLevel);  // Noise only
        }
        else if (g_Patch.FilterResonance)   // Filter enabled (res != 0)
        {
            // Adjust waveMixerOut to a level which avoids overdriving the filter
            filterIn = (waveMixerOut * m_FilterAtten_pc) / 100;
            // Apply filter algorithm
            filterOut =  MultiplyFixed(v_coeff_b0, filterIn);
            filterOut += MultiplyFixed(v_coeff_b2, filter_in_2);
            filterOut -= MultiplyFixed(v_coeff_a1, filter_out_1);
            filterOut -= MultiplyFixed(v_coeff_a2, filter_out_2);
            filter_in_2 = filter_in_1;  // update delayed samples
            filter_in_1 = filterIn;
            filter_out_2 = filter_out_1;
            filter_out_1 = filterOut;
            // Adjust filter output level
            totalMixOut = (filterOut * m_FilterGain_x10) / 100;  
        }
        else  totalMixOut = waveMixerOut;   // No noise and no filter in patch
        
        // Apply ampld limiter.. 
        if (totalMixOut > FIXED_MAX_LEVEL)  
        {
            totalMixOut = FIXED_MAX_LEVEL;
            v_Clipping = TRUE;  // Trigger LED indicator
        }
        if (totalMixOut < -FIXED_MAX_LEVEL)  totalMixOut = -FIXED_MAX_LEVEL;

        // Variable-gain output attenuator -- Apply expression, envelope, etc.
        attenOut = MultiplyFixed(totalMixOut, v_OutputLevel); 

        // Reverberation effect (Courtesy of Dan Mitchell, ref. "BasicSynth")
        if (m_RvbMix != 0)  
        {
            reverbOut = MultiplyFixed(ReverbDelayLine[rvbIndex], m_RvbDecay);
            reverbLPF = (reverbOut + reverbPrev) >> 1;  // simple low-pass filter
            reverbPrev = reverbOut;
            ReverbDelayLine[rvbIndex] = ((attenOut * m_RvbAtten) >> 7) + reverbLPF;
            if (++rvbIndex >= m_RvbDelayLen)  rvbIndex = 0;  // wrap
            // Add reverb output to dry signal according to reverb mix setting...
            finalOutput = (attenOut * (128 - m_RvbMix)) >> 7;  // Dry portion
            finalOutput += (reverbOut * m_RvbMix) >> 7;   // Wet portion
        }
        else  finalOutput = attenOut;
    }

    // PWM DAC output... Update OC4 duty register (range 1..1999)
    PWM_AUDIO_DAC_WRITE(1000 + (int)(finalOutput >> 10));
    
#ifdef SYNTH_MK3_MX440_MAM  // SPI DAC output (12 bits)
    spiDACdata = (uint16)(2000 + (int)(finalOutput >> 9));  // range 0..4000
    SPI_Lock(2);
    SPI_DAC_CS_LOW();
    SPI_Transfer(2, 0x30 | HI_BYTE(spiDACdata));
    SPI_Transfer(2, LO_BYTE(spiDACdata));
    SPI_DAC_CS_HIGH();
    SPI_UnLock(2);
#endif

    READ_CPU_CORE_COUNT_REG(CC_Reg);
    v_ISRexecTime = CC_Reg - entryTime;
    IFS0bits.T2IF = 0;         // Clear the IRQ
}


/*
 * Function:  Timer_3 interrupt service routine (ISR)
 *
 * Overview:  The ISR handles any task(s) which must be executed
 *            synchronously with the Timer_3 rollover (period).
 */
void __ISR(_TIMER_3_VECTOR, IPL5AUTO)  Timer_3_IRQService(void)
{
    IFS0bits.T3IF = 0;
}


//=================================================================================================
//                                    Sundry functions

/*
 * Function:     Return TRUE if a Note is ON (gated), else FALSE.
 */
bool  isNoteOn()
{
    return  (m_Note_ON != 0);
}


/*
 * Function:     Get pointer to active patch table.
 */
PatchParamTable_t *GetActivePatchTable()
{
    return  &g_Patch;
}


/*
 * Function:     Get ID number of active patch.
 */
int  GetActivePatchID()
{
    return  g_Patch.PatchNumber;
}


/*
 * Function:     Get table index of patch definition for a given patch ID number
 */
int  GetTableIndexOfPatchID(uint16 patchID)
{
    short  i;
    int    patch_idx = 0;
    
    for (i = 0;  i < GetNumberOfPatchesDefined();  i++)
    {
        if (g_PatchProgram[i].PatchNumber == patchID)  { patch_idx = i;  break; }
    }
    
    return  patch_idx;
}


/*
 * Function:     Activate the User Wave-table (id = 0) and effectively disable OSC2
 *               in the active patch.
 *
 * Note:         Function intended for use by wave-table creator utility.
 */
void  SelectUserWaveTableOsc1()
{
    m_WaveTable1 = (int16 *) WaveTableBuffer; 
    g_Patch.Osc1WaveTable = 0;
    g_Patch.MixerOsc2Level = 0;
    g_Patch.MixerControl = MIXER_CTRL_FIXED;
}


/*
 * Function:     Get ID number of wave-table assigned to OSC1 in the active patch.
 *
 * Note:         Function intended for use by wave-table creator utility.
 */
int  GetActiveWaveTable(void)
{
    return  g_Patch.Osc1WaveTable;
}

/*
 * Function:     Set wave-table size parameter for wave-table buffer used by OSC1.
 *               The given value over-rides the value selected by a prior call to
 *               SynthPatchSetup() or WaveTableSelect().
 *
 * Entry arg:    (uint16) size = OSC1 wave-table size (number of samples used).
 *
 * Note:         Function intended for use by wave-table creator utility only.
 */
void  WaveTableSizeSet(uint16 size)
{
    g_Osc1WaveTableSize = size;
}


/*
 * Function:     Set OSC1 Frequency Divider value.
 *               The given value over-rides the value selected by a prior call to
 *               SynthPatchSetup() or WaveTableSelect().
 *
 * Entry arg:    (float) freqDiv = Frequency Divider value
 */
void  Osc1FreqDividerSet(float freqDiv)
{
    g_Osc1FreqDiv = freqDiv;
}


/*
 * Function:     Get current Frequency Divider value used in OSC1 wave-table.
 *
 * Return val:   (float) freqDiv = Frequency Divider value
 */
float  Osc1FreqDividerGet()
{
    return  g_Osc1FreqDiv;
}


/*
 * Function:  Return TRUE if the Remi synth is enabled, i.e. if a note is in progress.
 */
bool  isSynthActive(void)
{
    return  (v_SynthEnable != 0);
}

/*
 * Function:     Set Audio Output Level.
 *
 * Entry args:   level = (fixed_t) audio output level, per unit (0 ~ 0.9999).
 *               Wave samples from the synth tone-generator are scaled by this quantity.
 *
 * Note:         Diagnostic mode must be active for this function to be effective.
 */
void  SetAudioOutputLevel(fixed_t level_pu)
{
    v_OutputLevel = level_pu;
}

/*
 * Function:     Get Expression/pressure level (fixed-pt value).
 *               Intended primarily for test and debug purposes.
 */
fixed_t  GetExpressionLevel(void)
{
    return  m_ExpressionLevel;
}

/*
 * Function:     Get Expression/pressure level (fixed-pt value).
 *               Intended primarily for test and debug purposes.
 */
fixed_t  GetModulationLevel(void)
{
    return  m_ModulationLevel;
}

/*
 * Function:     Get Reverb Mix (wet/dry) setting (0..128).
 *               Intended primarily for test and debug purposes.
 */
int  GetReverbMixSetting(void)
{
    return  m_RvbMix;
}

/*
 * Function:     Set Vibrato (Osc. FM) control mode temporarily.
 *               Called by diagnostic routines only.
 * 
 * Note:    <!>  Vibrato mode is restored to that defined by the active Preset
 *               whenever an Instrument Preset is selected.
 */
void  SetVibratoMode(unsigned mode)
{
    m_VibratoControl = mode & 7;
}

/*
 * Function:     Set bi-quad filter resonant frequency (m_FilterIndex) temporarily.
 *               Called by diagnostic routines only.
 * 
 * Note:    <!>  m_FilterIndex is restored to that defined by the active Preset
 *               whenever an Instrument Preset is selected.
 * 
 * Input arg:    freqIndex = index into filter coeff. LUT (0..108)
 */
void  SetFilterFreqIndex(uint8 freqIndex)
{
    if (freqIndex <= 108)  m_FilterIndex = freqIndex;
}

/*
 * Function:     Get bi-quad filter resonant frequency (m_FilterIndex)
 *               Called by diagnostic routines only.
 */
uint8  GetFilterFreqIndex()
{
    return  m_FilterIndex;
}


/*
 * Function:    Base2Exp()
 *
 * Overview:    Base-2 exponential transfer function using look-up table with interpolation.
 *              Resolution (precision) is better than +/-0.0001 (approx.)
 *
 * Entry arg:   (fixed_t) xval = fixed-point real number, range -1.0 to +1.0
 *
 * Returned:    (fixed_t) yval = 2 ** xval;  range 0.5000 to 2.0000
 *
 * Note 1:      If entry arg (xval) is out of range, returned value is 1.000.
 * 
 * Note 2: <!>  LUT g_base2exp[] values are 18:14 bit fixed-point.
 *              Shift left 6 bit places to convert to 12:20 fixed-pt.
 *
 * Results from unit test function, CLI command "util -b":
 *
 *   x = -1.00000,  y =  0.50000
 *   x = -0.99900,  y =  0.50031
 *   x = -0.50000,  y =  0.70709
 *   x = -0.00100,  y =  0.99908
 *   x =  0.00000,  y =  1.00000
 *   x =  0.00010,  y =  1.00000
 *   x =  0.00050,  y =  1.00031
 *   x =  0.00100,  y =  1.00067
 *   x =  0.00150,  y =  1.00098
 *   x =  0.00200,  y =  1.00134
 *   x =  0.50000,  y =  1.41418
 *   x =  0.99950,  y =  1.99896
 *   x =  1.00000,  y =  2.00000
 */
fixed_t  Base2Exp(fixed_t xval)
{
    int   ixval;        // 13-bit integer representing x-axis coordinate
    int   idx;          // 10 MS bits of ixval = array index into LUT, g_base2exp[]
    int   irem3;        // 3 LS bits of ixval for interpolation
    int32 ydelta;       // change in y value between 2 adjacent points in LUT
    int32 yval;         // y value (from LUT) with interpolation

    if (xval < IntToFixedPt(-1) || xval > IntToFixedPt(1))  xval = 0;

    // Convert real xval (x-coord) to positive 13-bit integer in the range 0 ~ 8K
    ixval = FractionPart((xval + IntToFixedPt(1)) / 2, 13); 
    idx = ixval >> 3;  
    irem3 = ixval & 7; 

    if (xval == IntToFixedPt(1))
        yval = 2 << 14;  // maximum value in 18:14 bit format
    else
    {
        yval = (int32) g_base2exp[idx];
        ydelta = (((int32) g_base2exp[idx+1] - yval) * irem3) / 8;
        yval = yval + ydelta;
    }

    return  (fixed_t)(yval << 6);   // convert to 12:20 fixed-pt format
}

//=================================================================================================
//
// Lookup table to transform linear variable to base-2 exponential.
// Index value range 0..1024 (integer) represents linear axis range -1.0 ~ +1.0.
// Lookup value range is 0.5 to 2.0 (fixed point).  Centre (zero) value is 1.00.
//
// <!>  g_base2exp[] values are in 18:14 bit fixed-point format.
//      Shift left 6 bit places to convert to 12:20 fixed-point.
//      ````````````````````````````````````````````````````````
// For higher precision, where required, use the function: Base2Exp()
//
const  uint16  g_base2exp[] =
{
    0x2000, 0x200B, 0x2016, 0x2021, 0x202C, 0x2037, 0x2042, 0x204E,
    0x2059, 0x2064, 0x206F, 0x207A, 0x2086, 0x2091, 0x209C, 0x20A8,
    0x20B3, 0x20BE, 0x20CA, 0x20D5, 0x20E0, 0x20EC, 0x20F7, 0x2103,
    0x210E, 0x211A, 0x2125, 0x2130, 0x213C, 0x2148, 0x2153, 0x215F,
    0x216A, 0x2176, 0x2181, 0x218D, 0x2199, 0x21A4, 0x21B0, 0x21BC,
    0x21C7, 0x21D3, 0x21DF, 0x21EB, 0x21F6, 0x2202, 0x220E, 0x221A,
    0x2226, 0x2231, 0x223D, 0x2249, 0x2255, 0x2261, 0x226D, 0x2279,
    0x2285, 0x2291, 0x229D, 0x22A9, 0x22B5, 0x22C1, 0x22CD, 0x22D9,
    0x22E5, 0x22F1, 0x22FD, 0x2309, 0x2315, 0x2322, 0x232E, 0x233A,
    0x2346, 0x2352, 0x235F, 0x236B, 0x2377, 0x2384, 0x2390, 0x239C,
    0x23A9, 0x23B5, 0x23C1, 0x23CE, 0x23DA, 0x23E7, 0x23F3, 0x23FF,
    0x240C, 0x2418, 0x2425, 0x2432, 0x243E, 0x244B, 0x2457, 0x2464,
    0x2470, 0x247D, 0x248A, 0x2496, 0x24A3, 0x24B0, 0x24BD, 0x24C9,
    0x24D6, 0x24E3, 0x24F0, 0x24FC, 0x2509, 0x2516, 0x2523, 0x2530,
    0x253D, 0x254A, 0x2557, 0x2564, 0x2570, 0x257D, 0x258A, 0x2598,
    0x25A5, 0x25B2, 0x25BF, 0x25CC, 0x25D9, 0x25E6, 0x25F3, 0x2600,
    0x260D, 0x261B, 0x2628, 0x2635, 0x2642, 0x2650, 0x265D, 0x266A,
    0x2678, 0x2685, 0x2692, 0x26A0, 0x26AD, 0x26BA, 0x26C8, 0x26D5,
    0x26E3, 0x26F0, 0x26FE, 0x270B, 0x2719, 0x2726, 0x2734, 0x2742,
    0x274F, 0x275D, 0x276A, 0x2778, 0x2786, 0x2794, 0x27A1, 0x27AF,
    0x27BD, 0x27CB, 0x27D8, 0x27E6, 0x27F4, 0x2802, 0x2810, 0x281E,
    0x282C, 0x283A, 0x2847, 0x2855, 0x2863, 0x2871, 0x287F, 0x288E,
    0x289C, 0x28AA, 0x28B8, 0x28C6, 0x28D4, 0x28E2, 0x28F0, 0x28FF,
    0x290D, 0x291B, 0x2929, 0x2938, 0x2946, 0x2954, 0x2962, 0x2971,
    0x297F, 0x298E, 0x299C, 0x29AA, 0x29B9, 0x29C7, 0x29D6, 0x29E4,
    0x29F3, 0x2A01, 0x2A10, 0x2A1F, 0x2A2D, 0x2A3C, 0x2A4A, 0x2A59,
    0x2A68, 0x2A77, 0x2A85, 0x2A94, 0x2AA3, 0x2AB2, 0x2AC0, 0x2ACF,
    0x2ADE, 0x2AED, 0x2AFC, 0x2B0B, 0x2B1A, 0x2B29, 0x2B38, 0x2B47,
    0x2B56, 0x2B65, 0x2B74, 0x2B83, 0x2B92, 0x2BA1, 0x2BB0, 0x2BBF,
    0x2BCE, 0x2BDE, 0x2BED, 0x2BFC, 0x2C0B, 0x2C1B, 0x2C2A, 0x2C39,
    0x2C48, 0x2C58, 0x2C67, 0x2C77, 0x2C86, 0x2C95, 0x2CA5, 0x2CB4,
    0x2CC4, 0x2CD3, 0x2CE3, 0x2CF3, 0x2D02, 0x2D12, 0x2D21, 0x2D31,
    0x2D41, 0x2D50, 0x2D60, 0x2D70, 0x2D80, 0x2D8F, 0x2D9F, 0x2DAF,
    0x2DBF, 0x2DCF, 0x2DDF, 0x2DEF, 0x2DFE, 0x2E0E, 0x2E1E, 0x2E2E,
    0x2E3E, 0x2E4E, 0x2E5F, 0x2E6F, 0x2E7F, 0x2E8F, 0x2E9F, 0x2EAF,
    0x2EBF, 0x2ED0, 0x2EE0, 0x2EF0, 0x2F00, 0x2F11, 0x2F21, 0x2F31,
    0x2F42, 0x2F52, 0x2F62, 0x2F73, 0x2F83, 0x2F94, 0x2FA4, 0x2FB5,
    0x2FC5, 0x2FD6, 0x2FE7, 0x2FF7, 0x3008, 0x3018, 0x3029, 0x303A,
    0x304B, 0x305B, 0x306C, 0x307D, 0x308E, 0x309F, 0x30AF, 0x30C0,
    0x30D1, 0x30E2, 0x30F3, 0x3104, 0x3115, 0x3126, 0x3137, 0x3148,
    0x3159, 0x316A, 0x317C, 0x318D, 0x319E, 0x31AF, 0x31C0, 0x31D2,
    0x31E3, 0x31F4, 0x3205, 0x3217, 0x3228, 0x323A, 0x324B, 0x325C,
    0x326E, 0x327F, 0x3291, 0x32A2, 0x32B4, 0x32C6, 0x32D7, 0x32E9,
    0x32FB, 0x330C, 0x331E, 0x3330, 0x3341, 0x3353, 0x3365, 0x3377,
    0x3389, 0x339B, 0x33AC, 0x33BE, 0x33D0, 0x33E2, 0x33F4, 0x3406,
    0x3418, 0x342A, 0x343C, 0x344F, 0x3461, 0x3473, 0x3485, 0x3497,
    0x34AA, 0x34BC, 0x34CE, 0x34E0, 0x34F3, 0x3505, 0x3517, 0x352A,
    0x353C, 0x354F, 0x3561, 0x3574, 0x3586, 0x3599, 0x35AB, 0x35BE,
    0x35D1, 0x35E3, 0x35F6, 0x3609, 0x361C, 0x362E, 0x3641, 0x3654,
    0x3667, 0x367A, 0x368D, 0x369F, 0x36B2, 0x36C5, 0x36D8, 0x36EB,
    0x36FE, 0x3712, 0x3725, 0x3738, 0x374B, 0x375E, 0x3771, 0x3784,
    0x3798, 0x37AB, 0x37BE, 0x37D2, 0x37E5, 0x37F8, 0x380C, 0x381F,
    0x3833, 0x3846, 0x385A, 0x386D, 0x3881, 0x3894, 0x38A8, 0x38BC,
    0x38CF, 0x38E3, 0x38F7, 0x390B, 0x391E, 0x3932, 0x3946, 0x395A,
    0x396E, 0x3982, 0x3996, 0x39AA, 0x39BE, 0x39D2, 0x39E6, 0x39FA,
    0x3A0E, 0x3A22, 0x3A36, 0x3A4A, 0x3A5F, 0x3A73, 0x3A87, 0x3A9B,
    0x3AB0, 0x3AC4, 0x3AD8, 0x3AED, 0x3B01, 0x3B16, 0x3B2A, 0x3B3F,
    0x3B53, 0x3B68, 0x3B7C, 0x3B91, 0x3BA6, 0x3BBA, 0x3BCF, 0x3BE4,
    0x3BF9, 0x3C0D, 0x3C22, 0x3C37, 0x3C4C, 0x3C61, 0x3C76, 0x3C8B,
    0x3CA0, 0x3CB5, 0x3CCA, 0x3CDF, 0x3CF4, 0x3D09, 0x3D1E, 0x3D34,
    0x3D49, 0x3D5E, 0x3D73, 0x3D89, 0x3D9E, 0x3DB3, 0x3DC9, 0x3DDE,
    0x3DF4, 0x3E09, 0x3E1F, 0x3E34, 0x3E4A, 0x3E5F, 0x3E75, 0x3E8B,
    0x3EA0, 0x3EB6, 0x3ECC, 0x3EE2, 0x3EF7, 0x3F0D, 0x3F23, 0x3F39,
    0x3F4F, 0x3F65, 0x3F7B, 0x3F91, 0x3FA7, 0x3FBD, 0x3FD3, 0x3FE9,
    0x4000, 0x4016, 0x402C, 0x4042, 0x4058, 0x406F, 0x4085, 0x409C,
    0x40B2, 0x40C8, 0x40DF, 0x40F5, 0x410C, 0x4122, 0x4139, 0x4150,
    0x4166, 0x417D, 0x4194, 0x41AA, 0x41C1, 0x41D8, 0x41EF, 0x4206,
    0x421D, 0x4234, 0x424A, 0x4261, 0x4278, 0x4290, 0x42A7, 0x42BE,
    0x42D5, 0x42EC, 0x4303, 0x431B, 0x4332, 0x4349, 0x4360, 0x4378,
    0x438F, 0x43A7, 0x43BE, 0x43D6, 0x43ED, 0x4405, 0x441C, 0x4434,
    0x444C, 0x4463, 0x447B, 0x4493, 0x44AA, 0x44C2, 0x44DA, 0x44F2,
    0x450A, 0x4522, 0x453A, 0x4552, 0x456A, 0x4582, 0x459A, 0x45B2,
    0x45CA, 0x45E3, 0x45FB, 0x4613, 0x462B, 0x4644, 0x465C, 0x4675,
    0x468D, 0x46A5, 0x46BE, 0x46D6, 0x46EF, 0x4708, 0x4720, 0x4739,
    0x4752, 0x476A, 0x4783, 0x479C, 0x47B5, 0x47CE, 0x47E7, 0x47FF,
    0x4818, 0x4831, 0x484A, 0x4864, 0x487D, 0x4896, 0x48AF, 0x48C8,
    0x48E1, 0x48FB, 0x4914, 0x492D, 0x4947, 0x4960, 0x497A, 0x4993,
    0x49AD, 0x49C6, 0x49E0, 0x49F9, 0x4A13, 0x4A2D, 0x4A46, 0x4A60,
    0x4A7A, 0x4A94, 0x4AAE, 0x4AC8, 0x4AE1, 0x4AFB, 0x4B15, 0x4B30,
    0x4B4A, 0x4B64, 0x4B7E, 0x4B98, 0x4BB2, 0x4BCC, 0x4BE7, 0x4C01,
    0x4C1B, 0x4C36, 0x4C50, 0x4C6B, 0x4C85, 0x4CA0, 0x4CBA, 0x4CD5,
    0x4CF0, 0x4D0A, 0x4D25, 0x4D40, 0x4D5B, 0x4D75, 0x4D90, 0x4DAB,
    0x4DC6, 0x4DE1, 0x4DFC, 0x4E17, 0x4E32, 0x4E4D, 0x4E69, 0x4E84,
    0x4E9F, 0x4EBA, 0x4ED5, 0x4EF1, 0x4F0C, 0x4F28, 0x4F43, 0x4F5F,
    0x4F7A, 0x4F96, 0x4FB1, 0x4FCD, 0x4FE9, 0x5004, 0x5020, 0x503C,
    0x5058, 0x5074, 0x508F, 0x50AB, 0x50C7, 0x50E3, 0x50FF, 0x511C,
    0x5138, 0x5154, 0x5170, 0x518C, 0x51A9, 0x51C5, 0x51E1, 0x51FE,
    0x521A, 0x5237, 0x5253, 0x5270, 0x528C, 0x52A9, 0x52C5, 0x52E2,
    0x52FF, 0x531C, 0x5339, 0x5355, 0x5372, 0x538F, 0x53AC, 0x53C9,
    0x53E6, 0x5403, 0x5421, 0x543E, 0x545B, 0x5478, 0x5495, 0x54B3,
    0x54D0, 0x54EE, 0x550B, 0x5529, 0x5546, 0x5564, 0x5581, 0x559F,
    0x55BD, 0x55DA, 0x55F8, 0x5616, 0x5634, 0x5652, 0x5670, 0x568E,
    0x56AC, 0x56CA, 0x56E8, 0x5706, 0x5724, 0x5742, 0x5761, 0x577F,
    0x579D, 0x57BC, 0x57DA, 0x57F9, 0x5817, 0x5836, 0x5854, 0x5873,
    0x5891, 0x58B0, 0x58CF, 0x58EE, 0x590D, 0x592B, 0x594A, 0x5969,
    0x5988, 0x59A7, 0x59C7, 0x59E6, 0x5A05, 0x5A24, 0x5A43, 0x5A63,
    0x5A82, 0x5AA1, 0x5AC1, 0x5AE0, 0x5B00, 0x5B1F, 0x5B3F, 0x5B5F,
    0x5B7E, 0x5B9E, 0x5BBE, 0x5BDE, 0x5BFD, 0x5C1D, 0x5C3D, 0x5C5D,
    0x5C7D, 0x5C9D, 0x5CBE, 0x5CDE, 0x5CFE, 0x5D1E, 0x5D3E, 0x5D5F,
    0x5D7F, 0x5DA0, 0x5DC0, 0x5DE1, 0x5E01, 0x5E22, 0x5E42, 0x5E63,
    0x5E84, 0x5EA5, 0x5EC5, 0x5EE6, 0x5F07, 0x5F28, 0x5F49, 0x5F6A,
    0x5F8B, 0x5FAC, 0x5FCE, 0x5FEF, 0x6010, 0x6031, 0x6053, 0x6074,
    0x6096, 0x60B7, 0x60D9, 0x60FA, 0x611C, 0x613E, 0x615F, 0x6181,
    0x61A3, 0x61C5, 0x61E7, 0x6209, 0x622B, 0x624D, 0x626F, 0x6291,
    0x62B3, 0x62D5, 0x62F8, 0x631A, 0x633C, 0x635F, 0x6381, 0x63A4,
    0x63C6, 0x63E9, 0x640B, 0x642E, 0x6451, 0x6474, 0x6497, 0x64B9,
    0x64DC, 0x64FF, 0x6522, 0x6545, 0x6569, 0x658C, 0x65AF, 0x65D2,
    0x65F6, 0x6619, 0x663C, 0x6660, 0x6683, 0x66A7, 0x66CA, 0x66EE,
    0x6712, 0x6736, 0x6759, 0x677D, 0x67A1, 0x67C5, 0x67E9, 0x680D,
    0x6831, 0x6855, 0x6879, 0x689E, 0x68C2, 0x68E6, 0x690B, 0x692F,
    0x6954, 0x6978, 0x699D, 0x69C1, 0x69E6, 0x6A0B, 0x6A2F, 0x6A54,
    0x6A79, 0x6A9E, 0x6AC3, 0x6AE8, 0x6B0D, 0x6B32, 0x6B57, 0x6B7D,
    0x6BA2, 0x6BC7, 0x6BED, 0x6C12, 0x6C38, 0x6C5D, 0x6C83, 0x6CA8,
    0x6CCE, 0x6CF4, 0x6D1A, 0x6D3F, 0x6D65, 0x6D8B, 0x6DB1, 0x6DD7,
    0x6DFD, 0x6E24, 0x6E4A, 0x6E70, 0x6E96, 0x6EBD, 0x6EE3, 0x6F09,
    0x6F30, 0x6F57, 0x6F7D, 0x6FA4, 0x6FCB, 0x6FF1, 0x7018, 0x703F,
    0x7066, 0x708D, 0x70B4, 0x70DB, 0x7102, 0x7129, 0x7151, 0x7178,
    0x719F, 0x71C7, 0x71EE, 0x7216, 0x723D, 0x7265, 0x728D, 0x72B4,
    0x72DC, 0x7304, 0x732C, 0x7354, 0x737C, 0x73A4, 0x73CC, 0x73F4,
    0x741C, 0x7444, 0x746D, 0x7495, 0x74BE, 0x74E6, 0x750F, 0x7537,
    0x7560, 0x7589, 0x75B1, 0x75DA, 0x7603, 0x762C, 0x7655, 0x767E,
    0x76A7, 0x76D0, 0x76F9, 0x7723, 0x774C, 0x7775, 0x779F, 0x77C8,
    0x77F2, 0x781B, 0x7845, 0x786F, 0x7899, 0x78C2, 0x78EC, 0x7916,
    0x7940, 0x796A, 0x7994, 0x79BF, 0x79E9, 0x7A13, 0x7A3D, 0x7A68,
    0x7A92, 0x7ABD, 0x7AE7, 0x7B12, 0x7B3D, 0x7B67, 0x7B92, 0x7BBD,
    0x7BE8, 0x7C13, 0x7C3E, 0x7C69, 0x7C94, 0x7CBF, 0x7CEB, 0x7D16,
    0x7D41, 0x7D6D, 0x7D98, 0x7DC4, 0x7DEF, 0x7E1B, 0x7E47, 0x7E73,
    0x7E9F, 0x7ECA, 0x7EF6, 0x7F22, 0x7F4F, 0x7F7B, 0x7FA7, 0x7FD3,
    0x8000
};

// end of file
