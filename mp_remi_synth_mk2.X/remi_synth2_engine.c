/*
 * ================================================================================================
 *
 * Module:       remi_synth2_engine.c
 *
 * Overview:     REMI mk2 sound synthesizer implementation.
 *
 * Author:       M.J.Bauer, Copyright 2016++  All rights reserved
 *
 * Reference:    www.mjbauer.biz/Build_the_REMI_by_MJB.htm
 *
 * ================================================================================================
 */
#include <sys/attribs.h>    // For interrupt handlers

#include "main_remi_synth2.h"
#include "remi_synth2_def.h"

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
PRIVATE  void   PrintWaveTableInfo(unsigned oscNum);
PRIVATE  void   DumpActivePatchParams();
PRIVATE  void   SetPatchParameter(char *paramAbbr, int paramVal);

int16    WaveTableBuffer[WAVE_TABLE_MAXIMUM_SIZE];  // signed 16-bit samples
fixed_t  ReverbDelayLine[REVERB_DELAY_MAX_SIZE];    // fixed-point samples

int32  g_TraceBuffer[100][5];

// These global variables may be modified by the CLI 'set' command...
float  g_PressureGain;          // Expression/Pressure gain adjust (.1 ~ 10)
float  g_FilterInputAtten;      // Filter input atten/gain (.01 ~ 2.5)
float  g_FilterOutputGain;      // Filter output atten/gain (0.1 ~ 25)
float  g_NoiseFilterGain;       // Noise gen. gain adjustment (0.1 ~ 25)
float  g_ReverbLoopTime_sec;    // Reverb param. (0.01 ~ 0.05 sec, typ. 0.04)
float  g_ReverbDecayTime_sec;   // Reverb param. (1.0 ~ 2.0 sec, typ. 1.5)

static PatchParamTable_t  m_Patch;        // active (working) patch parameters

static uint16   m_Osc1WaveTableSize;      // Number of samples in OSC1 wave-table
static uint16   m_Osc2WaveTableSize;      // Number of samples in OSC2 wave-table
static int16   *m_WaveTable1;             // Pointer to OSC1 wave-table
static int16   *m_WaveTable2;             // Pointer to OSC2 wave-table
static float    m_Osc1FreqDiv;            // OSC1 frequency divider
static float    m_Osc2FreqDiv;            // OSC2 frequency divider
static fixed_t  m_Osc1StepInit;           // Initial value of v_Osc1Step at Note-On
static fixed_t  m_Osc2StepInit;           // Initial value of v_Osc2Step at Note-On
static fixed_t  m_OscFreqMultiplier;      // Pitch-bend factor (range: 0.5 ~ 2.0)
static fixed_t  m_LFO_output;             // LFO output signal, normalized, bipolar (+/-1.0)

static uint16   m_PressureGain_pc;        // Expression/Pressure gain (percent)
static fixed_t  m_PressureLevel;          // Expression/Pressure level, norm.
static fixed_t  m_ModulationLevel;        // Modulation level, normalized
static fixed_t  m_PitchBendFactor;        // Pitch-Bend factor, normalized
static uint8    m_PitchBendControl;       // Pitch-Bend control mode (Off, PBmsg, CC01, CC02)
static uint8    m_VibratoControl;         // 0:None, 1:FX.Sw, 2:CC(Mod.Lvr), 3:Auto
static fixed_t  m_RampOutput;             // Vibrato Ramp output level, normalized (0..1)
static bool     m_EffectSwitchState;      // Effect Switch state: 0:Off, 1:On
static short    m_AmpldEnvSegment;        // Amplitude envelope shaper phase (state)
static fixed_t  m_AmpldEnvOutput;         // Amplitude envelope output (0 ~ 0.9995)
static short    m_ContourEnvSegment;      // Mixer transient/contour phase (state)
static fixed_t  m_ContourEnvOutput;       // Mixer contour output, normalized (0..1)

static fixed_t  m_AttackVelocity;         // Attack Velocity, normalized (0 ~ 0.999)
static bool     m_TriggerAttack;          // Signal to put ampld envelope into attack
static bool     m_TriggerRelease;         // Signal to put ampld envelope into release
static bool     m_ContourEnvTrigger;      // Signal to start mixer transient/contour
static bool     m_ContourEnvFinish;       // Signal to end mixer transient/contour

static bool     m_LegatoNoteChange;       // Signal Legato note change to Vibrato func.
static uint8    m_Note_ON;                // TRUE if Note ON, ie. "gated", else FALSE
static uint8    m_NotePlaying;            // MIDI note number of note playing

static uint8    m_FilterAtten_pc;         // Filter input atten/gain (%)
static uint8    m_FilterGain_x10;         // Filter output gain x10 (1..250)
static uint8    m_NoiseGain_x10;          // Noise filter gain x10 (1..250)
static fixed_t  m_FiltCoeff_c[110];       // Bi-quad filter coeff. c  (a1 = -c)
static fixed_t  m_FiltCoeff_a2;           // Bi-quad filter coeff. a2
static fixed_t  m_FiltCoeff_b0;           // Bi-quad filter coeff. b0  (b2 = -b0)
static int      m_FilterFcIndex;          // Index into LUT: m_FiltCoeff_c[]

static int      m_RvbDelayLen;            // Reverb. delay line length (samples)
static fixed_t  m_RvbDecay;               // Reverb. decay factor
static fixed_t  m_RvbAtten;               // Reverb. attenuation factor
static fixed_t  m_RvbMix;                 // Reverb. wet/dry mix ratio

volatile uint8    v_SynthEnable;      // Signal to enable synth sampling routine
volatile int32    v_Osc1Angle;        // sample pos'n in wave-table, OSC1 [16:16]
volatile int32    v_Osc2Angle;        // sample pos'n in wave-table, OSC2 [16:16]
volatile int32    v_Osc1Step;         // sample pos'n increment, OSC1 [16:16 fixed-pt]
volatile int32    v_Osc2Step;         // sample pos'n increment, OSC2 [16:16 fixed-pt]
volatile uint16   v_Mix1Level;        // Mixer input_1 level x1024 (0..1024)
volatile uint16   v_Mix2Level;        // Mixer input_2 level x1024 (0..1024)
volatile fixed_t  v_NoiseLevel;       // Noise level control (normalized)
volatile fixed_t  v_OutputLevel;      // Audio output level control (normalized)
volatile fixed_t  v_coeff_b0;         // Bi-quad filter coeff b0 (active)
volatile fixed_t  v_coeff_b2;         // Bi-quad filter coeff b2 (active)
volatile fixed_t  v_coeff_a1;         // Bi-quad filter coeff a1 (active)
volatile fixed_t  v_coeff_a2;         // Bi-quad filter coeff a2 (active)


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
 * Function:  Initialize Timer #2 and OC4 pin for PWM audio DAC operation.
 *
 * Timer_2 is set up to generate the PWM audio output signal using a sampling
 * rate of 40ks/s.  Prescaler = 1:1;  Fclk = FCY = 80MHz;  Tclk = 12.5ns.
 * Timer_2 period := 50.00us (4000 x 12.5ns);  PR2 = 1999;  PWM freq = 40kHz.
 * Maximum duty register value is 1999.
 * Output Compare modules OC4 [and OC5] are setup for PWM (fault-detect disabled).
 */
void  RemiSynthAudioInit(void)
{
    TRISDbits.TRISD3 = 0;    // RD3/OC4 is an output pin
    TRISDbits.TRISD4 = 0;    // RD4/OC5 ..   ..   ..

    OC4CON = 0x0000;         // Disable OC4 while timer is set up

    T2CON = 0;               // Timer_2 setup for 40KHz PWM freq.
    T2CONbits.TCKPS = 0;     // Prescaler set to 1:1
    PR2 = 1999;              // Period = 2000 x 12.5ns (-> freq = 40kHz)
    IFS0bits.T2IF = 0;       // Clear IRQ flag
    IPC2bits.T2IP = 6;       // Set IRQ priority (highest!)
    T2CONbits.TON = 1;       // Start Timer

    OC4R = 1000;             // PWM Set initial duty (50%)
    OC4RS = 1000;
    OC4CON = 0x8006;         // Enable OC4 for PWM

    v_SynthEnable = 0;
    m_WaveTable1 = (int16 *) WaveTableBuffer;
    m_WaveTable2 = (int16 *) WaveTableBuffer;

    SetAudioOutputLevel(IntToFixedPt(1) / 2);  // For diagnostic mode only
    TIMER2_IRQ_ENABLE();
}


/*`````````````````````````````````````````````````````````````````````````````````````````````````
 * Function:     Prepare REMI synth tone generator to play a note.
 *
 * Overview:     This function must be called following any change in the synth patch
 *               configuration or other synth parameter, before playing a note.
 *
 * Note:         On entry, and exit, the signal v_SynthEnable is set False.
 */
void  RemiSynthPrepare()
{
    static  prepDone = FALSE;
    float   res, res_sq, freq_rat;
    float   rvbDecayFactor;
    float   pi_2 = 2.0f * 3.14159265f;
    float   filterFreq;   // Hz
    int     idx;
    int     preset = g_Config.PresetLastSelected;

    v_SynthEnable = 0;    // Disable the synth tone-generator
    m_Note_ON = FALSE;    // no note playing
    
    if (!prepDone)  // One-time initialisation at power-on/reset
    {
        g_PressureGain = g_Config.PressureGain;          // persistent param
        g_FilterInputAtten = g_Config.FilterInputAtten;  // persistent param
        g_FilterOutputGain = g_Config.FilterOutputGain;  // persistent param
        g_NoiseFilterGain = g_Config.NoiseFilterGain;    // persistent param
        g_ReverbLoopTime_sec = 0.04;
        g_ReverbDecayTime_sec = 1.5;
        prepDone = TRUE;
    }

    WaveTableSelect(1, m_Patch.Osc1WaveTable);
    WaveTableSelect(2, m_Patch.Osc2WaveTable);

    m_VibratoControl = g_Preset.Descr[preset].VibratoMode;
    //  m_PitchBendControl = g_Preset.Descr[preset].PitchBendMode;  // <<<<<<<<<<<<< TODO
    m_PitchBendControl = PITCH_BEND_DISABLED;   // <<<<<<<<<<<<<<<<<<<<<<<<<<<<< TEMP 
    
    m_OscFreqMultiplier = IntToFixedPt(1);
    m_LFO_output = 0;
    m_ContourEnvOutput = IntToFixedPt(m_Patch.ContourStartLevel) / 100;

    // Find coefficients for bi-quad filter according to patch param (FilterResonance)
    res = (float) m_Patch.FilterResonance / 10000;   // range 0 ~ 0.9999
    res_sq = res * res;
    m_FiltCoeff_a2 = FloatToFixed(res_sq);
    m_FiltCoeff_b0 = FloatToFixed(0.5f - (res_sq / 2.0f));

    // Coeff a1 (a1 = -c) is both resonance and frequency dependent...  
    for (idx = 0 ; idx <= 108 ; idx++)    // populate LUT
    {
        freq_rat = m_NoteFrequency[idx] / SAMPLE_RATE_HZ;   // = Fc / Fs
        m_FiltCoeff_c[idx] = FloatToFixed(2.0 * res * cosf(pi_2 * freq_rat));
    }
    
    if (m_Patch.FilterFrequency != 0)  // Note Tracking disabled -- Fc is fixed
    {
        if (m_Patch.FilterFrequency >= 16 && m_Patch.FilterFrequency < 10000)
            filterFreq = (float) m_Patch.FilterFrequency;
        else  filterFreq = (float) 10000;   // Effectively bypassed

        for (idx = 0 ; idx < 108 ; idx++)
        {
            if (filterFreq < m_NoteFrequency[idx])  break;
        }
        m_FilterFcIndex = idx;
    }
    
    // Initialize synth working variables from global (non-patch) settable params.
    // (Note: Global set param's are all floating point.)
    m_PressureGain_pc = (uint16) (g_PressureGain * 100); 
    m_FilterAtten_pc = (uint8) (g_FilterInputAtten * 100); 
    m_FilterGain_x10 = (uint8) (g_FilterOutputGain * 10); 
    m_NoiseGain_x10 = (uint8) (g_NoiseFilterGain * 10);

    m_RvbDelayLen = (int) (g_ReverbLoopTime_sec * SAMPLE_RATE_HZ);
    if (m_RvbDelayLen > REVERB_DELAY_MAX_SIZE)  
        m_RvbDelayLen = REVERB_DELAY_MAX_SIZE;
    rvbDecayFactor = g_ReverbLoopTime_sec / g_ReverbDecayTime_sec;

    m_RvbDecay = FloatToFixed( powf(0.001f, rvbDecayFactor) );
    m_RvbAtten = IntToFixedPt((int) g_Config.ReverbAtten_pc) / 100;
    m_RvbMix = IntToFixedPt((int) g_Config.ReverbMix_pc) / 100;
    
    // Initial values for real-time control variables, in case control routine disabled.
    v_NoiseLevel = 0;   
    v_OutputLevel = IntToFixedPt( 1 ) / 2;
}


/*`````````````````````````````````````````````````````````````````````````````````````````````````
 * Function:     Select (activate) a wave-table for a given synth oscillator.
 *               This then becomes the "active" wave-table, over-riding the table
 *               selected by any prior call to RemiSynthPatchSelect().
 *
 * Entry args:   osc_num = oscillator assigned to wave-table, 1 -> OSC1, 2 -> OSC2
 *               wave_id = ID number of wave-table, range 0..250
 *
 * Note:         wave_id = 0 selects the "User Wave-table" (RAM buffer) ^
 *               wave_id > 0 selects a flash-based wave-table
 * 
 *             ^ The User Wave-table is regenerated in the RAM buffer by the function
 *               RemiSynthPatchSelect() from the user waveform descriptor in EEPROM.
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
            m_Osc1WaveTableSize = g_FlashWaveTableDef[wave_id].Size;
            m_Osc1FreqDiv = g_FlashWaveTableDef[wave_id].FreqDiv;
            m_WaveTable1 = (int16 *) g_FlashWaveTableDef[wave_id].Address;
        }
        else if (osc_num == 2)
        {
            m_Osc2WaveTableSize = g_FlashWaveTableDef[wave_id].Size;
            m_Osc2FreqDiv = g_FlashWaveTableDef[wave_id].FreqDiv;
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
short  RemiSynthPatchSelect(int patchNum)
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

        memcpy(&m_Patch, &g_PatchProgram[i], sizeof(PatchParamTable_t));
    }
    else  // if (patchNum == 0)
    {
        // Copy User Patch (persistent data in EEPROM) to active patch
        memcpy(&m_Patch, &g_Config.UserPatch, sizeof(PatchParamTable_t));
    }
    
    // Restore User Wave-table in RAM buffer in case m_Patch.Osc#WaveTable == 0:
    GenerateWaveTable( (WaveformDesc_t *) &g_Config.UserWaveform );
    //
    if (m_Patch.Osc1WaveTable == 0)  // OSC1 needs User Wave-table
    {
        m_Osc1WaveTableSize = g_Config.UserWaveform.Size;
        m_Osc1FreqDiv = g_Config.UserWaveform.FreqDiv;
        m_WaveTable1 = (int16 *) WaveTableBuffer; 
    }
    if (m_Patch.Osc2WaveTable == 0)  // OSC2 needs User Wave-table
    {
        m_Osc2WaveTableSize = g_Config.UserWaveform.Size;
        m_Osc2FreqDiv = g_Config.UserWaveform.FreqDiv;
        m_WaveTable2 = (int16 *) WaveTableBuffer; 
    }   

    // Ensure minimum values are assigned to envelope transition times...
    // (except for peak-hold time, which may be zero)
    if (m_Patch.AmpldEnvAttack_ms < 5) m_Patch.AmpldEnvAttack_ms = 5;
    if (m_Patch.AmpldEnvDecay_ms < 5) m_Patch.AmpldEnvDecay_ms = 5;
    if (m_Patch.AmpldEnvRelease_ms < 5) m_Patch.AmpldEnvRelease_ms = 5;
    if (m_Patch.ContourDelay_ms < 5) m_Patch.ContourDelay_ms = 5;
    if (m_Patch.ContourRamp_ms < 5) m_Patch.ContourRamp_ms = 5;
    if (m_Patch.VibratoRamp_ms < 5) m_Patch.VibratoRamp_ms = 5;

    RemiSynthPrepare();

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
void  RemiSynthNoteOn(uint8 noteNum, uint8 velocity)
{
    if (!m_Note_ON)    // Note OFF -- Initiate a new note...
    {
        RemiSynthNoteChange(noteNum);  // Set OSC1 and OSC2 frequencies, etc

        m_AmpldEnvOutput = 0;   // Zero the ampld envelope output signal

        // A square-law curve is applied to velocity
        m_AttackVelocity = IntToFixedPt((int) velocity) / 128;  // normalized
        m_AttackVelocity = MultiplyFixed(m_AttackVelocity, m_AttackVelocity);  // squared
        
        TIMER2_IRQ_DISABLE();
        v_Osc1Angle = 0;
        v_Osc2Angle = 0;

        // Initialize wave mixer input levels (range 0..1024)
        v_Mix2Level = (1024 * (int) m_Patch.MixerOsc2Level) / 100;
        v_Mix1Level = 1024 - v_Mix2Level;
        TIMER2_IRQ_ENABLE();

        m_LegatoNoteChange = 0;    // Not a Legato event
        m_TriggerAttack = 1;       // Let 'er rip, Boris
        m_ContourEnvTrigger = 1;
    }
    else  // Note already playing -- Legato note change
    {
        RemiSynthNoteChange(noteNum);  // Adjust OSC1 and OSC2 frequencies
        m_LegatoNoteChange = 1;        // Signal Note-Change event (for vibrato fn)
    }
}


/*
 * Function:     Set the pitch of a note to be initiated, or change pitch of the note
 *               in progress, without affecting the amplitude envelope (i.e. no re-attack).
 *               This function may be used where a "legato" effect is required.
 *               (See also: RemiSynthNoteOn() function.)
 *
 * Entry args:   noteNum = MIDI standard note number. (Note #60 = C4 = middle-C.)
 *               The REMI synth supports note numbers in the range: 12 (C0) to 108 (C8).
 *
 * The actual perceived pitch depends on the Frequency Divider values and the dominant
 * partial(s) in the waveform(s). Normally, the Osc.Freq.Div parameter value is chosen to
 * match the wave-table, so that the perceived pitch corresponds to the MIDI note number.
 */
void  RemiSynthNoteChange(uint8 noteNum)
{
    float   osc1Freq, osc2Freq;
    fixed_t osc1Step, osc2Step;
    float   osc2detune;      // ratio:  osc2Freq / osc1Freq;
    fixed_t detuneNorm;
    int     filterCorner;    // unit = MIDI note number (semitone)
    int     cents, noteTransposed;
    int     preset = g_Config.PresetLastSelected;

    // Apply PRESET Pitch Transpose parameter
    noteTransposed = (int) noteNum + g_Preset.Descr[preset].PitchTranspose;

    // Ensure note number (transposed) is within synth range (12 ~ 108)
    while (noteTransposed > 108)  { noteTransposed -= 12; }   // too high
    while (noteTransposed < 12)  { noteTransposed += 12; }   // too low

    noteNum = noteTransposed;
    m_NotePlaying = noteNum;
    m_Note_ON = TRUE;

    // Convert MIDI note number to frequency (Hz);  apply OSC1 freq.divider param.
    osc1Freq = m_NoteFrequency[noteNum - 12] / m_Osc1FreqDiv;

    // Calculate detune factor as a fraction of an octave
    cents = m_Patch.Osc2Detune % 1200;     // signed
    detuneNorm = (IntToFixedPt(1) * cents) / 1200;  // range +/-1.000
    osc2detune = FixedToFloat(Base2Exp(detuneNorm));
    osc2Freq = osc1Freq * osc2detune;      // Apply OSC2 detune factor
    osc2Freq = osc2Freq / m_Osc2FreqDiv;   // Apply OSC2 Freq.Divider parameter

    // Initialize oscillator variables for use in audio ISR
    osc1Step = (int32)((osc1Freq * m_Osc1WaveTableSize * 65536) / SAMPLE_RATE_HZ);
    osc2Step = (int32)((osc2Freq * m_Osc2WaveTableSize * 65536) / SAMPLE_RATE_HZ);
    
    // Calculate filter corner freq (pitch offset) for new note...
    if (m_Patch.FilterFrequency == 0)  // ... if Note Tracking enabled
    {
        filterCorner = noteNum + m_Patch.FilterNoteTrack;   // MIDI note #
        if (filterCorner > 120)  filterCorner = 120;        // Cap at C9 (8370 Hz)
        m_FilterFcIndex = filterCorner - 12;
    }
    
    if (m_Patch.NoiseLevelCtrl && (m_Patch.NoiseMode & NOISE_PITCHED))
        m_FilterFcIndex = 0;      // Use Fc = 16 Hz (minimum)

    TIMER2_IRQ_DISABLE();   // Ensure OSC1 and OSC2 are synchronized (in phase)
    v_Osc1Step = osc1Step;
    m_Osc1StepInit = osc1Step;   // Needed for Osc FM (vibrato, pitch-bend, etc)
    v_Osc2Step = osc2Step;
    m_Osc2StepInit = osc2Step;
    
    // Set filter coefficient values to be applied in the audio ISR
    v_coeff_b0 = m_FiltCoeff_b0;
    v_coeff_b2 = 0 - m_FiltCoeff_b0;      // b2 = -b0
    v_coeff_a1 = 0 - m_FiltCoeff_c[m_FilterFcIndex];  // a1 = -c
    v_coeff_a2 = m_FiltCoeff_a2;
    TIMER2_IRQ_ENABLE();
}


/*
 * Function:     Set the "Pressure Level" according to a given data value.
 *               Equivalent to MIDI Control Change message (CC# = 02, 07 or 11).
 *
 * Entry args:   data14 = MIDI expression/pressure value (14 bits, unsigned).
 *
 * Output:       (fixed_t) m_PressureLevel = normalized pressure level (0..+1.0)
 *
 * Note:         A square-law is applied to the data value to approximate an exponential
 *               response curve. The output pressure level is fixed point, normalized.
 *               The pressure level is adjusted accordiing to m_PressureGain_pc (%)
 *               which is a user-settable param (non-patch). Use CLI "set" command.
 */
void   RemiSynthExpression(unsigned data14)
{
    uint32  ulval;
    fixed_t level;

    ulval = ((uint32) data14 * data14) / 16384;  // apply square law
    ulval = ulval << 6;   // scale to 20 bits (fractional part)

    level = (fixed_t) ulval;  // convert to fixed-point fraction
    level = (level * m_PressureGain_pc) / 100;
    if (level > FIXED_MAX_LEVEL) level = FIXED_MAX_LEVEL;  // clip at 0.99999
    
    m_PressureLevel = level;  
}


/*
 * Function:     End the note playing, if it matches the given note number.
 *
 * Entry args:   noteNum = MIDI standard note number of note to be ended.
 *
 * The function puts envelope shapers into the 'Release' phase. The note will be
 * terminated by the synth process (B/G task) when the release time expires, or if
 * a new note is initiated prior.
 */
void  RemiSynthNoteOff(uint8 noteNum)
{
    int   noteTransposed;
    int   preset = g_Config.PresetLastSelected;

    // Apply PRESET Pitch Transpose parameter:
    noteTransposed = (int) noteNum + g_Preset.Descr[preset].PitchTranspose;
    if (noteTransposed >= 12 && noteTransposed <= 120) noteNum = noteTransposed;

    if (noteNum == m_NotePlaying)
    {
        m_TriggerRelease = 1;
        m_ContourEnvFinish = 1;
        m_Note_ON = FALSE;
    }
}


/*
 * Function:     Modify pitch of note in progress according to Pitch-Bend data value.
 *
 * Entry args:   data14 = signed integer representing Pitch Bend lever position,
 *                        in the range +/-8000 (14 LS bits).  Centre pos'n is 0.
 *
 * Affected:     m_PitchBendFactor, which is processed by the real-time synth function
 *               OscFreqModulation() while a note is in progress.
 *
 * Todo: *****   Test this function using MIDI pitch bend data !!  ******
 */
void   RemiSynthPitchBend(int data14)
{
    // Scale lever pos'n (arg) value according to patch 'PitchBendRange' param.
    // PitchBendRange may be up to 1200 cents (ie. 1 octave maximum).
    int  posnScaled = (data14 * m_Patch.PitchBendRange) / 1200;
    
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
void   RemiSynthModulation(unsigned data14)
{
    if (data14 < (16 * 1024))  
        m_ModulationLevel = (fixed_t) ((uint32) data14 << 6);  
    else   m_ModulationLevel = FIXED_MAX_LEVEL;
}


/*
 * Function:     Control synth effect(s) according to Effect Switch state;
 *               e.g. vibrato on/off, legato on/off, portamento on/off, etc.
 *
 * Entry args:   ctrlnum = effect ID number (= MIDI Control Change number)
 *               enab    = True: enable, or False: disable switched effect(s)
 *
 * Examples:     ctrlnum = 0:   no effect
 *               ctrlnum = 85:  vibrato (ramp) start/stop
 */
void   RemiSynthEffectSwitch(uint8 ctrlnum, uint8 enab)
{
    // In this version, ctrlnum is ignored;  assume effect is vibrato

    if (enab) m_EffectSwitchState = 1;
    else  m_EffectSwitchState = 0;
}


/*`````````````````````````````````````````````````````````````````````````````````````````````````
 * Function:  RemiSynthProcess()
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
void   RemiSynthProcess()
{
    static  int   count5ms;

    AmpldEnvelopeShaper();
    ContourEnvelopeShaper();
    LowFrequencyOscillator();
    VibratoRampGenerator();
    AudioLevelController();

    if (++count5ms == 5)     // 5ms process interval (200Hz)
    {
        count5ms = 0;
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
    static  uint32   ampldEnvPhaseTimer;     // Time elapsed in envelope phase (ms)
    static  fixed_t  ampldSustainLevel;      // Ampld Env. sustain level (0 ~ 0.9995)
    static  fixed_t  timeConstant;           // 20% of decay or release time (ms)
    static  fixed_t  ampldDelta;             // Step change in Env Ampld in 1ms
    static  fixed_t  ampldMaximum;           // Peak value of Envelope Ampld
    static  int      envTimer;               // Envelope elapsed time (ms) -- debug use only

    if (m_TriggerAttack)
    {
        m_TriggerAttack = 0;
        m_TriggerRelease = 0;
        ampldEnvPhaseTimer = 0;
        envTimer = 0;
        ampldSustainLevel = IntToFixedPt((int) m_Patch.AmpldEnvSustain) / 100;    // normalized
        ampldSustainLevel = MultiplyFixed(ampldSustainLevel, ampldSustainLevel);  // squared
        ampldMaximum = FIXED_MAX_LEVEL;  // for Peak-Hold phase
        if (m_Patch.AmpldEnvPeak_ms == 0)
            ampldMaximum = ampldSustainLevel;  // No Peak-Hold phase
        ampldDelta = ampldMaximum / m_Patch.AmpldEnvAttack_ms;  // step change in 1ms

        m_AmpldEnvSegment = ENV_ATTACK;
        v_SynthEnable = 1;   // in case not already enabled
    }

    if (m_TriggerRelease)
    {
        m_TriggerRelease = 0;
        timeConstant = m_Patch.AmpldEnvRelease_ms / 5;
        ampldEnvPhaseTimer = 0;
        m_AmpldEnvSegment = ENV_RELEASE;
    }

    switch (m_AmpldEnvSegment)
    {
    case ENV_IDLE:          // Idle - Sound off - zero output level
    {
        m_AmpldEnvOutput = 0;
        break;
    }
    case ENV_ATTACK:        // Attack - linear ramp up to peak - or to the sustain level
    {
        if (m_AmpldEnvOutput < ampldMaximum) m_AmpldEnvOutput += ampldDelta;

        if (ampldEnvPhaseTimer >= m_Patch.AmpldEnvAttack_ms)  // attack time ended
        {
            m_AmpldEnvOutput = ampldMaximum;
            ampldEnvPhaseTimer = 0;

            if (m_Patch.AmpldEnvPeak_ms == 0)         // skip peak and decay phases
                m_AmpldEnvSegment = ENV_SUSTAIN;
            else  m_AmpldEnvSegment = ENV_PEAK_HOLD;    // run all phases
        }
        break;
    }
    case ENV_PEAK_HOLD:     // Peak - constant output level (0.999)
    {
        if (ampldEnvPhaseTimer >= m_Patch.AmpldEnvPeak_ms)  // Peak time ended
        {
            timeConstant = m_Patch.AmpldEnvDecay_ms / 5;  // for Decay phase
            ampldEnvPhaseTimer = 0;
            m_AmpldEnvSegment = ENV_DECAY;
        }
        break;
    }
    case ENV_DECAY:         // Decay - exponential ramp down to sustain level
    {
        ampldDelta = (m_AmpldEnvOutput - ampldSustainLevel) / timeConstant;  // step in 1ms
        if (ampldDelta == 0)  ampldDelta = FIXED_MIN_LEVEL;

        if (m_AmpldEnvOutput >= (ampldSustainLevel + ampldDelta))
            m_AmpldEnvOutput -= ampldDelta;

        // Allow 10 x time-constant for decay phase to complete
        if (ampldEnvPhaseTimer >= (m_Patch.AmpldEnvDecay_ms * 2))
        {
            m_AmpldEnvSegment = ENV_SUSTAIN;
        }
        break;
    }
    case ENV_SUSTAIN:       // Sustain constant level - waiting for m_TriggerRelease
    {
        break;
    }
    case ENV_RELEASE:       // Release - exponential ramp down to zero level
    {
        // timeConstant and ampldEnvPhaseTimer are set by the trigger condition, above.
        ampldDelta = m_AmpldEnvOutput / timeConstant;
        if (ampldDelta == 0)  ampldDelta = FIXED_MIN_LEVEL;

        if (m_AmpldEnvOutput >= ampldDelta)  m_AmpldEnvOutput -= ampldDelta;

        // Allow 10 x time-constant for release phase to complete
        if (ampldEnvPhaseTimer >= (m_Patch.AmpldEnvRelease_ms * 2))
        {
            ampldEnvPhaseTimer = 0;
            m_AmpldEnvSegment = ENV_IDLE;
        }
        break;
    }
    default:
    {
        ampldEnvPhaseTimer = 0;
        m_AmpldEnvSegment = ENV_IDLE;
        break;
    }
    };  // end switch

    ampldEnvPhaseTimer++;
    envTimer++;
}


/*
 * Function:  AudioLevelController()
 *
 * Overview:  This routine is called by the Synth Process at 1ms intervals.
 *            The output level is controlled (modulated) by one of a variety of options
 *            as determined by the patch parameter: m_Patch.OutputAmpldCtrl.
 *
 * Output:    (fixed_t) v_OutputLevel : normalized output level (0..+1.0)
 *            The output variable is used by the audio ISR to control the audio ampld,
 *            except for the reverberated signal which may continue to sound.
 */
PRIVATE  void   AudioLevelController()
{
    static  fixed_t  outputAmpld;         // Audio output level, normalized
    static  fixed_t  smoothOutputAccum;   // Ampld smoothing filter accumulator
    static  fixed_t  smoothOutputLevel;   // Audio output level, normalized, smoothed

    if (m_Patch.OutputAmpldCtrl == AMPLD_CTRL_FIXED_L2)        // mode 1
    {
        if (m_Note_ON)  outputAmpld = FIXED_MAX_LEVEL / 2;
        else  outputAmpld = 0;
    }
    else if (m_Patch.OutputAmpldCtrl == AMPLD_CTRL_EXPRESS)    // mode 2
    {
	// After Note-Off, Pressure Level is no longer updated
        if (m_Note_ON)  outputAmpld = m_PressureLevel;
        else  outputAmpld = 0;  // mute
    }
    else if (m_Patch.OutputAmpldCtrl == AMPLD_CTRL_AMPLD_ENV)  // mode 3
    {
	outputAmpld = m_AmpldEnvOutput;
    }
    else if (m_Patch.OutputAmpldCtrl == AMPLD_CTRL_ENV_VELO)   // mode 4
    {
        outputAmpld = MultiplyFixed(m_AmpldEnvOutput, m_AttackVelocity);
    }
    else    // default  ...................................... // mode 0
    {
        if (m_Note_ON)  outputAmpld = FIXED_MAX_LEVEL;
        else  outputAmpld = 0;
    }
    
    if (g_Config.AmpldControlOverride)   // Override patch AC parameter
        outputAmpld = MultiplyFixed(m_AmpldEnvOutput, m_AttackVelocity);

    if (outputAmpld > FIXED_MAX_LEVEL)  outputAmpld = FIXED_MAX_LEVEL;

    // Apply smoothing filter to eliminate abrupt step changes
    smoothOutputAccum -= smoothOutputLevel;
    smoothOutputAccum += outputAmpld;
    smoothOutputLevel = smoothOutputAccum >> 4;  // Filter Tc = 16ms

    v_OutputLevel = smoothOutputLevel;  // accessed by audio ISR
}


/*
 * Function:  ContourEnvelopeShaper()
 *
 * Overview:  Routine called by the Synth Process at 1ms intervals.
 *            All segments of the Contour Envelope are linear time-varying.
 *            The contour output may be used to control the oscillator mix ratio,
 *            noise level, filter corner frequency (Fc), or whatever.
 *
 * Output:    (fixed_t) m_ContourEnvOutput = output signal, normalized (0..+0.999)
 */
PRIVATE  void   ContourEnvelopeShaper()
{
    static  uint32   contourSegmentTimer;   // Time elapsed in active phase (ms)
    static  fixed_t  outputDelta;           // Step change in output level in 1ms

    fixed_t  startLevel = IntToFixedPt(m_Patch.ContourStartLevel) / 100;
    fixed_t  holdLevel = IntToFixedPt(m_Patch.ContourHoldLevel) / 100;

    if (m_ContourEnvTrigger)  // Note-On event
    {
        m_ContourEnvTrigger = 0;
        m_ContourEnvFinish = 0;
        m_ContourEnvOutput = startLevel;
        contourSegmentTimer = 0;
        m_ContourEnvSegment = CONTOUR_ENV_DELAY;
    }

    switch (m_ContourEnvSegment)
    {
    case CONTOUR_ENV_IDLE:         // Waiting for trigger signal
    {
        break;
    }
    case CONTOUR_ENV_DELAY:        // Delay before ramp up/down segment
    {
        if (contourSegmentTimer >= m_Patch.ContourDelay_ms)  // Delay segment ended
        {
            // Prepare for ramp up/down phase (next state) to Hold level
            outputDelta = (holdLevel - m_ContourEnvOutput) / m_Patch.ContourRamp_ms;
            contourSegmentTimer = 0;
            m_ContourEnvSegment = CONTOUR_ENV_RAMP;
        }
        break;
    }
    case CONTOUR_ENV_RAMP:         // Linear ramp up/down from Start to Hold level
    {
        if (contourSegmentTimer >= m_Patch.ContourRamp_ms)  // Ramp segment ended
            m_ContourEnvSegment = CONTOUR_ENV_HOLD;
        else
            m_ContourEnvOutput += outputDelta;
        break;
    }
    case CONTOUR_ENV_HOLD:         // Hold constant level - waiting for Note-Off event
    {
        m_ContourEnvOutput = holdLevel;
        break;
    }
    };  // end switch

    contourSegmentTimer++;
}


/*
 * Function:     Synth LFO implementation.
 *
 * Called by RemiSynthProcess() at 1ms intervals, this function generates a sinusoidal
 * waveform in real time.  LFO frequency is a patch parameter, m_Patch.VibratoFreq,
 * unsigned 8-bit value representing LFO freq * 10 Hz; range 1..250 => 0.1 to 25 Hz.
 *
 * Effective sample rate (Fs) is 1000 Hz.
 * 
 * The output of the LFO is m_LFO_output, a normalized fixed-point variable.
 */
PRIVATE  void   LowFrequencyOscillator()
{
    static  int32  oscAngle = 0;   // 24:8 bit fixed-point
    fixed_t sample;
    int     waveIdx;

    int32  oscFreq = (((int) m_Patch.LFO_Freq_x10) << 8) / 10;  // 24:8 bit fixed-point
    int32  oscStep = (oscFreq * SINE_WAVE_TABLE_SIZE) / 1000;   // Fs = 1000Hz

    waveIdx = oscAngle >> 8;  // integer part
    oscAngle = (oscAngle + oscStep) % (SINE_WAVE_TABLE_SIZE * 256);
    sample = (fixed_t) g_sinewave[waveIdx] << 5; 

    m_LFO_output = (fixed_t) sample;   // +/-0.9999
}


/*
 * Function:     Vibrato Ramp Generator implementation.
 *
 * Called by RemiSynthProcess() at 1ms intervals, this function generates a linear ramp
 * signal in real time.
 *
 * If vibrato control mode is set to "Automatic", vibrato is started at Note-On. A delay is
 * imposed before the ramp begins to rise. Otherwise, ramp start/stop is controlled by the
 * Effect Switch state and there is no delay before the ramp begins.
 *
 * If a Legato note change occurs, vibrato is stopped (fast ramp down) and the ramp delay
 * is re-started, so that vibrato will ramp up again after the delay.
 *
 * The delay and ramp-up times are both set by the patch parameter, m_Patch.VibratoRamp_ms,
 * so the delay time value is the same as the ramp-up time.  This works well enough.
 */
PRIVATE  void   VibratoRampGenerator()
{
    static  short   rampState = 0;
    static  uint32  rampTimer_ms;
    fixed_t rampStep;
    int     preset = g_Config.PresetLastSelected;

    if (g_Preset.Descr[preset].VibratoMode == VIBRATO_BY_EFFECT_SW)
    {
        if (m_EffectSwitchState)  rampState = 2;
        else  rampState = 3;
    }

    if (g_Preset.Descr[preset].VibratoMode == VIBRATO_AUTOMATIC)
    {
        // Check for Note-Off or Note-Change event while ramp is progressing
        if (rampState != 3 &&  (m_AmpldEnvSegment == ENV_RELEASE || m_LegatoNoteChange))
            rampState = 3;   //  ramp down

        if (rampState == 0)  // Idle - waiting for Note-On
        {
            m_RampOutput = 0;
            if (m_AmpldEnvSegment == ENV_ATTACK)
            {
                rampTimer_ms = 0;  // start ramp delay timer
                rampState = 1;
            }
        }
        else if (rampState == 1)  // Delaying before ramp-up begins
        {
            if (rampTimer_ms >= m_Patch.VibratoRamp_ms)  rampState = 2;
            rampTimer_ms++;
        }
        else if (rampState == 2)  // Ramping up - hold at max. level (1.00)
        {
            // waiting for Note-Off or Note-Change event
        }
        else if (rampState == 3)  // Ramping down
        {
            if (m_RampOutput <= (IntToFixedPt(1) / 100))
            {
                // If a Note-Change has occurred, re-start the ramp (delay)
                if (m_LegatoNoteChange)
                    { m_LegatoNoteChange = 0;  rampState = 1; }
                else rampState = 0;
                rampTimer_ms = 0;
            }
        }
        else  rampState = 0;
    }

    if (rampState == 2)  // Ramp up using VibratoRamp (time) param.
    {
        rampStep = IntToFixedPt(1) / (int) m_Patch.VibratoRamp_ms;
        if (m_RampOutput < FIXED_MAX_LEVEL)  m_RampOutput += rampStep;
        if (m_RampOutput > FIXED_MAX_LEVEL)  m_RampOutput = FIXED_MAX_LEVEL;
    }

    if (rampState == 3)  // Ramp down fast (fixed 100ms)
    {
        rampStep = IntToFixedPt(1) / 100;
        if (m_RampOutput > 0)  m_RampOutput -= rampStep;
        if (m_RampOutput < 0)  m_RampOutput = 0;
    }
}


/*
 * Function:     Oscillator Frequency Modulation  (Pitch-bend, vibrato, etc)
 *
 * Called by RemiSynthProcess() at 5ms intervals, this function modulates the pitch of
 * the wave-table oscillators according to a multiplier variable, m_OscFreqMultiplier,
 * which may be modified while a note is in progress.
 * 
 * The linear m_PitchBendFactor is transformed into a multiplier in the range 0.5 ~ 2.0.
 * Centre (zero) m_PitchBendFactor value should give a multplier value of 1.00.
 *
 * Note:  This function is used for low frequency modulation, up to about 25Hz,
 *        intended for Pitch Bend OR Vibrato (mutually exclusive).
 */
PRIVATE  void   OscFreqModulation()
{
    static  fixed_t  smoothFreqMultAccum = IntToFixedPt(10);
    static  fixed_t  smoothFreqMult = IntToFixedPt(1);

    fixed_t  LFO_scaled, modnLevel;  // normalized quantities (range 0..+/-1.0)
    int      index;

    if (m_VibratoControl == VIBRATO_BY_MODN_CC)    // Use Mod Lever position
    {
        modnLevel = (m_ModulationLevel * m_Patch.VibratoDepth) / 1200;
    }
    else
    if (m_VibratoControl == VIBRATO_BY_EFFECT_SW   //  |
    ||  m_VibratoControl == VIBRATO_AUTOMATIC      //  |> Use Ramp Gen.
    ||  m_VibratoControl == VIBRATO_AUTO_ASYMM)    //  |
    {
        modnLevel = (m_RampOutput * m_Patch.VibratoDepth) / 1200;
    }

    if (m_VibratoControl && !m_PitchBendControl)  
    {
        LFO_scaled = MultiplyFixed(m_LFO_output, modnLevel); 
        m_OscFreqMultiplier = Base2Exp(LFO_scaled);   // range 0.5 ~ 2.0.
    }
    else if (m_PitchBendControl == PITCH_BEND_BY_PB_MESSAGE)  
    {
         m_OscFreqMultiplier = Base2Exp(m_PitchBendFactor);
    }
    else if (m_PitchBendControl == PITCH_BEND_BY_EXPRN_CC02) 
    {
        modnLevel = (m_PressureLevel * m_Patch.PitchBendRange) / 1200;
        m_OscFreqMultiplier = Base2Exp(modnLevel);
    }
    else  m_OscFreqMultiplier = IntToFixedPt( 1 );  // No pitch modulation

    if (m_VibratoControl || m_PitchBendControl)
    {
        // Apply smoothing filter to m_OscFreqMultiplier to avoid sudden changes.
        smoothFreqMultAccum -= smoothFreqMult;
        smoothFreqMultAccum += m_OscFreqMultiplier;
        smoothFreqMult = smoothFreqMultAccum / 10;  // Filter Tc = 10 * 5ms = 50ms

        TIMER2_IRQ_DISABLE();
        v_Osc1Step = MultiplyFixed(m_Osc1StepInit, smoothFreqMult);
        if (m_VibratoControl != VIBRATO_AUTO_ASYMM)
            v_Osc2Step = MultiplyFixed(m_Osc2StepInit, smoothFreqMult);
        TIMER2_IRQ_ENABLE();
    }
}


/*
 * Oscillator Mix Ratio Modulation
 *
 * Called by the RemiSynthProcess() routine at 5ms intervals, this function controls the ratio
 * of the 2 oscillator signals input to the wave mixer, according to a variable, mixRatio_pK,
 * which is the fraction (x1024) of the output of OSC2 relative to OSC1 fed into the mixer.
 *
 * The instantaneous value of the time-varying quantity mixRatio_pK is determined by the 
 * patch mixer control mode parameter, m_Patch.MixerControl.
 * The actual mixing operation is performed by the audio ISR, using the output variables.
 *
 * Output variables:   v_Mix1Level, v_Mix2Level  (range 0..1024)
 *            where:  (v_Mix1Level + v_Mix2Level) == 1024, always.
 *
 */
PRIVATE  void  OscMixRatioModulation()
{
    fixed_t  LFO_scaled;
    fixed_t  modnLevel;
    fixed_t  mixRatioLFO;   // normalized
    int      mixRatio_pK;   // fraction of OSC2 in the mix x 1024

    if (m_Patch.MixerControl == MIXER_CTRL_CONTOUR)
    {
        mixRatio_pK = IntegerPart(m_ContourEnvOutput * 1024);
    }
    else if (m_Patch.MixerControl == MIXER_CTRL_LFO)
    {
        // Use vibrato depth param (0..1200) to set contour LFO modulation depth.
        modnLevel = (m_RampOutput * m_Patch.VibratoDepth) / 1200;
        LFO_scaled = MultiplyFixed(m_LFO_output, modnLevel);  // range -1.0 ~ +1.0
        mixRatioLFO = (IntToFixedPt( 1 ) + LFO_scaled) / 2;   // range  0.0 ~ +1.0
        mixRatio_pK = IntegerPart(mixRatioLFO * 1024);        // convert to 'per K' units
    }
    else if (m_Patch.MixerControl == MIXER_CTRL_EXPRESS)
    {
        mixRatio_pK = IntegerPart(m_PressureLevel * 1024);
    }
    else if (m_Patch.MixerControl == MIXER_CTRL_MODULN)
    {
        mixRatio_pK = IntegerPart(m_ModulationLevel * 1024);
    }
    else  // assume (m_Patch.MixerControl == MIXER_CTRL_FIXED) -- default
    {
        mixRatio_pK = (1024 * (int) m_Patch.MixerOsc2Level) / 100;  // fixed %
    }

    // Use mixRatio_pK to update individual OSC1 and OSC2 mixer input levels.
    // Prevent Timer_2 interrupt from occurring in the middle of the update!
    TIMER2_IRQ_DISABLE();
    v_Mix2Level = mixRatio_pK;
    v_Mix1Level = 1024 - v_Mix2Level;   // range: 0..1024
    TIMER2_IRQ_ENABLE();
}


/*
 * Noise Generator Output level Control
 *
 * Called by the RemiSynthProcess() routine at 5ms intervals, this function sets the noise
 * generator output level in real-time according to the noise control source (patch param).
 * The actual level control operation is performed by the audio ISR.
 *
 * Output variable:  v_NoiseLevel  (normalized fixed-point)
 */
PRIVATE  void   NoiseLevelControl()
{
    int  noiseCtrlSource = m_Patch.NoiseLevelCtrl & 7;  
    
    if (noiseCtrlSource == NOISE_LVL_FIXED)           // option 1 (Fixed max.)
        v_NoiseLevel = FIXED_MAX_LEVEL;
    else if (noiseCtrlSource == NOISE_LVL_AMPLD_ENV)  // option 2
        v_NoiseLevel = m_AmpldEnvOutput;
    else if (noiseCtrlSource == NOISE_LVL_EXPRESS)    // option 3
        v_NoiseLevel = (m_PressureLevel / 2);
    else if (noiseCtrlSource == NOISE_LVL_MODULN)     // option 4
        v_NoiseLevel = (m_ModulationLevel * 75) / 100;
    else  v_NoiseLevel = 0;  // ..................... // option 0 (Noise OFF)
        
}


/*
 * Bi-quad resonant filter centre frequency control (modulation)
 *
 * Called by the RemiSynthProcess() routine at 5ms intervals, this function updates the
 * bi-quad IIR filter coefficients in real-time according to the active note pitch and
 * selected control parameters. The actual DSP filter algorithm is incorporated in the 
 * audio ISR.
 * 
 * Input variable:  m_FilterFcIndex = LUT index for coeff c (varies with fc)
 *
 * Output variables:  v_coeff_xx  = real-time filter coeff's (4 fixed-point values)
 * 
 * *** todo: Implement remaining control modes  ***********************************
 */
PRIVATE  void   FilterFrequencyControl()
{
    fixed_t  devn;   // deviation from quiescent value (+/-0.5 max)
    int  fc_idx = m_FilterFcIndex;   // determined at Note_ON, note change
    
    if (m_Patch.NoiseLevelCtrl != 0)  // Noise enabled
    {
        return;  // filter Fc is fixed if noise is enabled
    }
    
    if (m_Patch.FilterControl == FILTER_CTRL_CONTOUR)  // mode 1
    {
        fc_idx = IntegerPart(m_ContourEnvOutput * 108);
    }
    else if (m_Patch.FilterControl == FILTER_CTRL_LFO)  // mode 4
    {
        devn = (m_LFO_output * m_Patch.VibratoDepth) / 100;  // LFO deviation (+/-1.0 max)
        fc_idx += IntegerPart(devn * 108);
    }
    else  // Assume m_Patch.FilterControl == FILTER_CTRL_FIXED) ... mode 0
    {
        return;  // filter Fc is fixed
    }

    if (fc_idx > 108)  fc_idx = 108;   // max. (~8400 Hz)
    if (fc_idx < 0)  fc_idx = 0;       // min. (~16 Hz)

    // Update the real-time coefficient values to be applied in the audio ISR
    TIMER2_IRQ_DISABLE();
    v_coeff_b0 = m_FiltCoeff_b0;
    v_coeff_b2 = 0 - m_FiltCoeff_b0;         // b2 = -b0
    v_coeff_a1 = 0 - m_FiltCoeff_c[fc_idx];  // a1 = -c
    v_coeff_a2 = m_FiltCoeff_a2;
    TIMER2_IRQ_ENABLE();
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
    static  int      rvbIndex;            // index into ReverbDelayLine[]
    static  fixed_t  reverbPrev;          // previous sample from reverb processor

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
    fixed_t  attenOut;                    // output from variable-gain attenuator
    fixed_t  reverbOut;                   // output from reverb delay line
    fixed_t  reverbLPF;                   // output from reverb loop filter
    fixed_t  finalOutput;                 // output sample (to PWM DAC) 

    TESTPOINT_RC14_SET_HI();  // for 'scope probe -- to measure ISR execution time

    if (v_SynthEnable)
    {
        // OSC1 Wave-table oscillator 
        idx = v_Osc1Angle >> 16;  // integer part of v_Osc1Angle
        osc1Sample = (fixed_t) m_WaveTable1[idx] << 5;  // normalize
        v_Osc1Angle += v_Osc1Step;
        if (v_Osc1Angle >= (m_Osc1WaveTableSize << 16))
            v_Osc1Angle -= (m_Osc1WaveTableSize << 16);

        // OSC2 Wave-table oscillator 
        idx = v_Osc2Angle >> 16;  // integer part of v_Osc2Angle
        osc2Sample = (fixed_t) m_WaveTable2[idx] << 5;  // normalize
        v_Osc2Angle += v_Osc2Step;
        if (v_Osc2Angle >= (m_Osc2WaveTableSize << 16))
            v_Osc2Angle -= (m_Osc2WaveTableSize << 16);
        
        // Wave Mixer -- add OSC1 and OSC2 samples, scaled according to mix ratio
        mixerIn1 = (osc1Sample * v_Mix1Level) >> 10;
        mixerIn2 = (osc2Sample * v_Mix2Level) >> 10;
        waveMixerOut = mixerIn1 + mixerIn2;

        // White noise generator -- Pseudo-random number algorithm...
        rand_last = (rand_last * 1103515245 + 12345) & 0x7FFFFFFF;  // unsigned
        noiseSample = (int32) (rand_last << 1);    // signed 32-bit value
        noiseSample = noiseSample >> 11;           // normalized fixed-pt (+/-1.0)
        
        if (m_Patch.NoiseMode)  // Noise enabled in patch
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
            if (m_Patch.NoiseMode & NOISE_PITCHED)   
                noiseGenOut = MultiplyFixed(filterOut, osc2Sample);  // Ring Mod.
            else  noiseGenOut = filterOut;   // unmodulated filtered noise
            
            // Add or mix noise with wave mixer output according to patch mode
            if ((m_Patch.NoiseMode & 3) == NOISE_WAVE_ADDED)  // Add noise to total mix
            {
                noiseGenOut = MultiplyFixed(noiseGenOut, v_NoiseLevel);
                totalMixOut = (waveMixerOut + noiseGenOut) / 2;  // avoid clipping
            }
            else if ((m_Patch.NoiseMode & 3) == NOISE_WAVE_MIXED)  // Ratiometric mix
            {
                waveRatio = IntToFixedPt(1) - v_NoiseLevel;  // wave:noise ratio (0 ~ 1.0)
                totalMixOut = MultiplyFixed(waveMixerOut, waveRatio);
                totalMixOut += MultiplyFixed(noiseGenOut, v_NoiseLevel);
            }
            else  totalMixOut = MultiplyFixed(noiseGenOut, v_NoiseLevel);  // Noise only
        }
        else if (m_Patch.FilterResonance)   // Filter enabled (res != 0)
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

        // Variable-gain output attenuator -- Apply expression, envelope, etc.
        attenOut = MultiplyFixed(totalMixOut, v_OutputLevel); 

        // Reverberation effect (Courtesy of Dan Mitchell, author of "BasicSynth")
        reverbOut = MultiplyFixed(ReverbDelayLine[rvbIndex], m_RvbDecay);
        reverbLPF = (reverbOut + reverbPrev) >> 1;  // simple low-pass filter
        reverbPrev = reverbLPF;
        ReverbDelayLine[rvbIndex] = MultiplyFixed(attenOut, m_RvbAtten) + reverbLPF;
        if (++rvbIndex >= m_RvbDelayLen)  rvbIndex = 0;  // wrap

        // Add reverb output to dry signal according to reverb mix setting...
        finalOutput = MultiplyFixed(attenOut, IntToFixedPt(1) - m_RvbMix); 
        finalOutput += MultiplyFixed(reverbOut, m_RvbMix);   // Wet part
        
        // Apply ampld limiter..  (Ideally a non-linear compression curve - TBD)
        if (finalOutput > FIXED_MAX_LEVEL)  finalOutput = FIXED_MAX_LEVEL;
        if (finalOutput < -FIXED_MAX_LEVEL)  finalOutput = -FIXED_MAX_LEVEL;
    }
    else  finalOutput = 0;  // synth engine disabled

    // PWM DAC output (11 bits) -- update PWM duty register (range 1..1999)
    OC4RS = 1000 + (int)(finalOutput >> 10);

    TESTPOINT_RC14_SET_LO();
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
    return  &m_Patch;
}


/*
 * Function:     Get ID number of active patch.
 */
int    GetActivePatchID()
{
    return  m_Patch.PatchNumber;
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
    m_Patch.Osc1WaveTable = 0;
    m_Patch.MixerOsc2Level = 0;
    m_Patch.MixerControl = MIXER_CTRL_FIXED;
}


/*
 * Function:     Get ID number of wave-table assigned to OSC1 in the active patch.
 *
 * Note:         Function intended for use by wave-table creator utility.
 */
int  GetActiveWaveTable(void)
{
    return  m_Patch.Osc1WaveTable;
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
    m_Osc1WaveTableSize = size;
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
    m_Osc1FreqDiv = freqDiv;
}


/*
 * Function:     Get current Frequency Divider value used in OSC1 wave-table.
 *
 * Return val:   (float) freqDiv = Frequency Divider value
 */
float  Osc1FreqDividerGet()
{
    return  m_Osc1FreqDiv;
}


/*
 * Function sets the value of m_OscFreqMultiplier.
 * Low-level diagnostic used only by console "diag" command.
 * Pitch bend function should be disabled.
 */
void  OscFreqMultiplierSet(float  mult)
{
    m_OscFreqMultiplier = FloatToFixed(mult);
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
 * Function:     Set Vibrato (LFO) control mode.
 *               Intended primarily for test and debug purposes.
 *
 * Note:    <!>  Vibrato mode is restored to that defined by the active Preset
 *               whenever an Instrument Preset is selected.
 */
void  SetVibratoModeTemp(unsigned mode)
{
    m_VibratoControl = mode & 7;
}

/*
 * Function:     Get Expression/pressure level (fixed-pt value).
 *               Intended primarily for test and debug purposes.
 */
fixed_t  GetExpressionLevel(void)
{
    return  m_PressureLevel;
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
//                       P A T C H    C L I    F U N C T I O N S
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
    char    option = tolower(argValue[1][1]);
    int     paramValue;
    int     i;

    if (argCount == 1 || *argValue[1] == '?')   // help wanted
    {
        putstr( "View or modify the active (selected) patch \n" );
        putstr( "``````````````````````````````````````````````````````````` \n" );
        putstr( "Usage (1):  patch  <opt>  [arg] \n" );
        putstr( "  where <opt> = \n" );
        putstr( " -l  : List/enumerate predefined patches. \n");
        putstr( " -d  : Dump active patch param's (alias: -i) \n");
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
        case 'd':  // Dump active patch param's
        case 'i':  // alias - patch info
        {
            DumpActivePatchParams();
            break;
        }
        case 's':  // Save active patch as User Patch
        {
            m_Patch.PatchNumber = 0;
            memset(&m_Patch.PatchName[0], 0, 22);  // clear existing name
            if (argCount > 2)  // new name supplied
                strncpy(&m_Patch.PatchName[0], argValue[2], 20);
            else  strcpy(&m_Patch.PatchName[0], "User Patch");

            memcpy(&g_Config.UserPatch, &m_Patch, sizeof(PatchParamTable_t));

            if (StoreConfigData())  putstr("* Saved OK.\n");
            else  putstr("! Error writing to EEPROM.\n");
            break;
        }
        case 'u':  // Load User Patch
        {
            RemiSynthPatchSelect(0);
            DumpActivePatchParams();
            break;
        }
        case 'p':  // Load & activate a predefined patch (from flash PM)
        {
            int   patchNum = atoi(argValue[2]);
            int   status = 0;

            if (argCount == 3 && patchNum != 0)
                status = RemiSynthPatchSelect(patchNum);
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
        waveTableID = m_Patch.Osc1WaveTable;
        activeTableSize = m_Osc1WaveTableSize;
        activeOscFreqDiv = m_Osc1FreqDiv;
    }
    else if (oscNum == 2)  
    {
        waveTableID = m_Patch.Osc2WaveTable;
        activeTableSize = m_Osc2WaveTableSize;
        activeOscFreqDiv = m_Osc2FreqDiv;
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

    putstr("Patch ID#:  ");  putDecimal(m_Patch.PatchNumber, 1);
    putstr(" -> ");  putstr(m_Patch.PatchName);
    putstr("\n\n");

    //-------------  Oscillators, Pitch Bend and Vibrato --------------------------------

    sprintf(textBuf, "\t%d,\t// W1: OSC1 Wave-table ID (0..250)\n",
            (int) m_Patch.Osc1WaveTable);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// W2: OSC2 Wave-table ID (0..250)\n",
            (int) m_Patch.Osc2WaveTable);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// OD: OSC2 Detune, cents (+/-1200)\n",
            (int) m_Patch.Osc2Detune);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// PB: Pitch Bend range, cents (0..1200)\n",
            (int) m_Patch.PitchBendRange);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// LF: LFO Freq x10 Hz (1..250)\n",
            (int) m_Patch.LFO_Freq_x10);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// VD: Vibrato Depth, cents (0..200)\n",
            (int) m_Patch.VibratoDepth);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// VR: Vibrato Ramp time (5..5000+ ms)\n",
            (int) m_Patch.VibratoRamp_ms);
    putstr(textBuf);
    putstr("\n");

    //-------------  Wave Mixer & Contour Envelope --------------------------------------

    sprintf(textBuf, 
            "\t%d,\t// MC: Mixer Control (0:Fixed, 1:Contour, 2:LFO, 3:Exprn, 4:Modn)\n",
            (int) m_Patch.MixerControl);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// ML: Mixer OSC2 Level in Fixed mode (0..100 %%)\n",
            (int) m_Patch.MixerOsc2Level);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// CS: Contour Env Start level (0..100 %%)\n",
            (int) m_Patch.ContourStartLevel);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// CD: Contour Env Delay time (5..5000+ ms)\n",
            (int) m_Patch.ContourDelay_ms);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// CR: Contour Env Ramp time (5..5000+ ms)\n",
            (int) m_Patch.ContourRamp_ms);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// CH: Contour Env Hold level (0..100 %%)\n",
            (int) m_Patch.ContourHoldLevel);
    putstr(textBuf);
    putstr("\n");

    //-------------  Noise Mixer & Bi-quad resonant Filter ------------------------------

    sprintf(textBuf,
            "\t%d,\t// NM: Noise Mode (0:Off, 1:Noise, 2:Add wave, 3:Mix wave; +4:Pitch)\n",
            (int) m_Patch.NoiseMode);
    putstr(textBuf);
    sprintf(textBuf,
            "\t%d,\t// NC: Noise Level Ctrl (0:Off, 1:Fixed, 2:Env, 3:Exprn, 4:Modn)\n",
            (int) m_Patch.NoiseLevelCtrl);
    putstr(textBuf);
    sprintf(textBuf,
            "\t%d,\t// FC: Filter Ctrl (0:Fixed, 1:Contour, 2:Env+, 3:Env-, 4:LFO, 5:Modn)\n",
            (int) m_Patch.FilterControl);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// FR: Filter Resonance x10000  (0..9999, 0:Bypass)\n",
            (int) m_Patch.FilterResonance);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// FF: Filter Freq. (10..10000 Hz,  0:NoteTracking On)\n",
            (int) m_Patch.FilterFrequency);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// FT: Filter Tracking offset (0..60 semitones)\n",
            (int) m_Patch.FilterNoteTrack);
    putstr(textBuf);
    putstr("\n");

    //-------------  Amplitude Envelope and Output Amplitude Control  -------------------

    sprintf(textBuf, "\t%d,\t// AA: Ampld Env Attack time (5..5000+ ms)\n",
            (int) m_Patch.AmpldEnvAttack_ms);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// AP: Ampld Env Peak time (0..5000+ ms)\n",
            (int) m_Patch.AmpldEnvPeak_ms);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// AD: Ampld Env Decay time (5..5000+ ms)\n",
            (int) m_Patch.AmpldEnvDecay_ms);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// AR: Ampld Env Release time (5..5000+ ms)\n",
            (int) m_Patch.AmpldEnvRelease_ms);
    putstr(textBuf);
    sprintf(textBuf, "\t%d,\t// AS: Ampld Env Sustain level (0..100 %%)\n",
            (int) m_Patch.AmpldEnvSustain);
    putstr(textBuf);
    sprintf(textBuf,
            "\t%d \t// AC: Ampld Control (0:Max, 1:Fixed, 2:Exprn, 3:Env, 4:Env*Vel)\n",
            (int) m_Patch.OutputAmpldCtrl);
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
                m_Patch.Osc1WaveTable = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('W', '2'):
        {
            if (paramVal <= GetHighestWaveTableID())
                m_Patch.Osc2WaveTable = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('O', 'D'):
        {
            if (paramVal >= -4800 && paramVal <= 4800)
                m_Patch.Osc2Detune = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('P', 'B'):
        {
            if (paramVal <= 1200)  m_Patch.PitchBendRange = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('L', 'F'):
        {
            if (paramVal > 1 && paramVal <= 250)  
                m_Patch.LFO_Freq_x10 = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('V', 'D'):
        {
            if (paramVal <= 200)  m_Patch.VibratoDepth = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('V', 'R'):
        {
            if (paramVal >= 5 && paramVal <= 10000)  
                m_Patch.VibratoRamp_ms = paramVal;
            else  isBadValue = 1;
            break;
        }
        //-------------  Wave Mixer & Contour Envelope --------------------------------------
        case PARAM_HASH_VALUE('M', 'C'):
        {
            if (paramVal <= 15)  m_Patch.MixerControl = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('M', 'L'):
        {
            if (paramVal <= 100)  m_Patch.MixerOsc2Level = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('C', 'S'):
        {
            if (paramVal <= 100)  m_Patch.ContourStartLevel = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('C', 'D'):
        {
            if (paramVal >= 5 && paramVal <= 10000)  
                m_Patch.ContourDelay_ms = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('C', 'R'):
        {
            if (paramVal >= 5 && paramVal <= 10000)  
                m_Patch.ContourRamp_ms = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('C', 'H'):
        {
            if (paramVal <= 100)  m_Patch.ContourHoldLevel = paramVal;
            else  isBadValue = 1;
            break;
        }
        //-------------  Noise Generator & Bi-Quad Variable Filter --------------------------------
        case PARAM_HASH_VALUE('N', 'M'):
        {
            if (paramVal <= 7)  m_Patch.NoiseMode = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('N', 'L'):
        case PARAM_HASH_VALUE('N', 'C'):
        {
            if (paramVal <= 15)  m_Patch.NoiseLevelCtrl = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('F', 'C'):
        {
            if (paramVal <= 15)  m_Patch.FilterControl = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('F', 'R'):
        {
            if (paramVal <= 9999)  m_Patch.FilterResonance = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('F', 'F'):
        {
            if (paramVal <= 20000)  m_Patch.FilterFrequency = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('F', 'T'):
        {
            if (paramVal <= 255)  m_Patch.FilterNoteTrack = paramVal;
            else  isBadValue = 1;
            break;
        }
        //-------------  Amplitude Envelope and Output Amplitude Control  -------------------
        case PARAM_HASH_VALUE('A', 'A'):
        {
            if (paramVal >= 5 && paramVal <= 10000)  
                m_Patch.AmpldEnvAttack_ms = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('A', 'P'):
        {
            if (paramVal <= 10000)  m_Patch.AmpldEnvPeak_ms = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('A', 'D'):
        {
            if (paramVal >= 5 && paramVal <= 10000)  
                m_Patch.AmpldEnvDecay_ms = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('A', 'R'):
        {
            if (paramVal >= 5 && paramVal <= 10000)  
                m_Patch.AmpldEnvRelease_ms = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('A', 'S'):
        {
            if (paramVal <= 100)  m_Patch.AmpldEnvSustain = paramVal;
            else  isBadValue = 1;
            break;
        }
        case PARAM_HASH_VALUE('A', 'C'):
        {
            if (paramVal <= 15)  m_Patch.OutputAmpldCtrl = paramVal;
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
        RemiSynthPrepare();
        putstr("* OK \n");
    }
}
