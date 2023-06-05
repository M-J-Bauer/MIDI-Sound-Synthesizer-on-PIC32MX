/*
 * Module:       wave_table_creator.c
 * ```````
 * Overview:     Command-line utility to generate and test wave-table data
 * `````````     for use with the "REMI" sound synthesizer.
 *               The command syntax is:  wav  <option>  [args ...]
 *               Wave-table data is output in C source code format as an array definition
 *               to be included in source file "remi_synth_data.c".
 *
 * Originated:   M.J.Bauer, 2016
 * ```````````
 * Reference:    www.mjbauer.biz/Build_the_REMI_synth.htm
 * ``````````
 *=================================================================================================
 */

#include "../Common/system_def.h"
#include "wave_table_creator.h"

#include <math.h>

// Supported wave shapes
#define  SINE_WAVE       1
#define  TRIANG_WAVE     2
#define  SQUARE_WAVE     3
#define  RAMP_WAVE       4   // Sawtooth waveform
#define  NOISE_WAVE      5   // White noise
#define  SQUOND_WAVE     6   // Rounded square-wave

#define  CLIP_THRESHOLD   31900   // = 32 x 999

#define  PI  (3.141592654)

extern   const  int16  rounded_square[];   // "cog wheel" wave-table, size 1260

PRIVATE  void   ClearWaveTable(void);
PRIVATE  void   ListWaveParameters(void);
PRIVATE  int    AddPartialToWaveTable(void);
PRIVATE  void   RemovePartialFromWaveTable(int order);
PRIVATE  void   ScaleTablePeakMagnitude(uint16 peak_pc);
PRIVATE  void   DumpWaveTable(void);
PRIVATE  int16  CalcWaveSample(uint8 wave_shape, int sample_point, int period);
PRIVATE  void   ApplyAntiAliasFilter();
PRIVATE  void   GenerateWaveTableFromDrawbars(int argCount, char *argVal[]);
PRIVATE  void   GeneratePseudoRandomWave(int srlen, int fbbits, int init);
PRIVATE  uint16 PseudoRandomSequence(uint8 srlen, uint16 seed, uint16 fbmask);


// Private data...
static  char   WaveTableName[42];    // Assign using -dump cmd (max. 40 chars)
static  int    TableSize;            // Table size -- default 1260 samples, 2520 max. 
static  uint8  PartialAmpldHist[20]; // Histogram of partial amplitudes, 0 -> 1st order
static  int    PartialAmpld;         // Amplitude of partial to be added (0..100 %FS)
static  int    PartialOrder;         // Order of partial to be added (1..16)
static  uint16 AliasFilter_K;        // Filter time-constant (unit = sample intrvl)
static  bool   isTableEmpty;         // True when wave-table buffer is cleared
static  bool   isPrepDone;           // True when 'wav' utility has been initialized
static  bool   isHammond;            // True if last wave-table created by Hammond cmd
static  int    previousWaveTable;    // Wave-table in use prior to using 'wav' utility
static  int    previousPatch;        // Patch selected prior to using 'wav' utility
static  uint8  drawbar_setting[10];  // Hammond drawbar settings (9 values each 0..8)


/*`````````````````````````````````````````````````````````````````````````````````````````````````
 *  Function populates the wave-table buffer (array in data RAM) with sample data
 *  according to a specified waveform descriptor (arg1).
 *
 *  Entry arg:   waveDesc = pointer to a structure of type WaveformDesc_t
 */
void  GenerateWaveTable(WaveformDesc_t  *waveDesc)
{
    int   i;
    
    TableSize = waveDesc->Size;
    Osc1FreqDividerSet(waveDesc->FreqDiv);
    ClearWaveTable();

    for (i = 0;  i < 16;  i++)
    {
        PartialOrder = i + 1;
        PartialAmpld = waveDesc->Partial[i];
        if (PartialAmpld != 0) AddPartialToWaveTable();
    }
}


/*`````````````````````````````````````````````````````````````````````````````````````````````````
 * CLI command function:  Cmnd_wav
 *
 * The "wav" command provides facilities for the creation of wave-table data for use with
 * the oscillators in the "REMI" built-in sound synthesizer.
 * Sample values are signed 16-bit integers in the range +/-32000.
 *
 * A wave-table is populated by adding together one or more periodic waveforms called
 * "partials", of different relative frequency and phase. In the usual context, a "partial"
 * is a pure sine-wave, the frequency of which is not necessarily harmonically
 * related to the "fundamental" (dominant, usually lowest frequency, partial).
 *
 * In this application, the definition of "partial" may be broadened to include non-sinusoidal
 * waveforms such as square, triangle, ramp and "pitched" psuedo-random noise.
 * Be aware that non-sinusoidal partials can result in "aliasing" with the oscillator
 * sampling frequency, resulting in the generation of spurious unwanted frequencies.
 * Sinusoidal partials minimize the adverse effects of aliasing.
 *
 * Partial amplitudes are entered as percentages of full-scale, not as absolute values.
 * If the addition of a partial exceeds amplitude limits, i.e. if clipping occurs, a warning
 * will be issued by the "add" (-a) command.
 * It is recommended to plan the mix in advance so that the sum of the partial amplidudes
 * (percentages) is under 100%.  Note that the addition of partials does not generally
 * result in a waveform peak magnitude equal to the sum of the individual partial magnitudes.
 * This is particularly apparent when adding odd harmonics (i.e. 3rd, 5th, 7th order).
 *
 * The partial "order" is simply the harmonic number (1, 2, 3, 4, 5, ...) relative to the
 * "fundamental" (order = 1). One cycle of the fundamental fits the wave-table size exactly.
 * Choose a table size which results in an exact fit of partial period for all required
 * partials;  i.e. zero remainder when the table size is divided by the order, for all
 * partials with non-zero amplitude in the composite wave-form. Note that it is possible to
 * set zero amplitude for any partial, including the "fundamental" (order = 1).
 * 
 * A dedicated command option supports additive synthesis based on a Hammond organ drawbar
 * registration. See the 'help' text for the command option "-h" for details.
 * Note: Hammond drawbars use a log scale.  Each increment adds 3dB to the partial ampld.
 * Thus, partial ampld (%) = 100 x sqrt(2)^(N-8);  where N = drawbar setting (1..8).
 *
 * Recommended (default) table size is 1200, which fits most partial orders up to the 16th,
 * except for odd orders 7th and higher, but if these partials have relatively low amplitude,
 * the resulting waveform distortion will not be preceptible.
 */
void  Cmnd_wav(int argCount, char * argVal[])
{
    char    option;
    char    textBuf[80];

    if (argCount == 1 || *argVal[1] == '?' )  // help
    {
        putstr( "Utility to create, test and save REMI synth wave-tables.  \n" );
        putstr( "Any cmd option except '-x' will activate User wave-table. \n" );
        putstr( "Enter 'wav -x' cmd when done to restore previous patch. \n\n" );
        putstr( "Usage:  wav  <option>  [arg1] [arg2] ... \n" );
        putstr( "<option>----------------------------------------\n" );
        putstr( "  -n : New wave-table  [arg1 = table size]      \n" );
        putstr( "  -f : Set Osc.Freq.Div <arg1> for sound test   \n" );
        putstr( "  -l : List wave-table and partial param's      \n" );
        putstr( "  -a : Add partial to wave-table buffer --      \n" );
        putstr( "           <arg1> = partial order (1..16)       \n" );
        putstr( "           <arg2> = partial ampld (1..99 %FS)   \n" );
        putstr( "  -c : Clear partial, <arg1> = order (1..16)    \n" );
        putstr( "  -h : Create table from Hammond organ drawbars \n" );
        putstr( "         (Enter 'wav -h' for usage details)     \n" );
        putstr( "  -z : Scale table peak magnitude [arg = %FS]   \n" );
        putstr( "  -k : Apply anti-alias filter (K = 5..500)     \n" );
        putstr( "  -d : Dump wave-table as C array def'n         \n" );
        putstr( "           [arg1 = Waveform_name (no spaces)]   \n" );
        putstr( "  -s : Save user wave-table def'n in EEPROM     \n" );
        putstr( "  -x : Exit -- restore last patch selected      \n" );
        putstr( "------------------------------------------------\n" );
        return;
    }

    if (argVal[1][0] == '-') option = tolower(argVal[1][1]);  else option = '$';

    // One-time initialization on first use of 'wav' command, except option '-x'
    if (!isPrepDone && option != 'x' && option != '$')
    {
        previousPatch = GetActivePatchID();  // Save active patch before corrupting it
        WaveTableName[0] = 0;
        SelectUserWaveTableOsc1();           // Use RAM wave-table buffer for OSC1
        TableSize = 1260;                    // Set defaults for user wave-table
        WaveTableSizeSet(TableSize);
        Osc1FreqDividerSet(1.0);
        AliasFilter_K = 50;                  // Set Anti-alias filter TC default
        ClearWaveTable();
        srand(TMR2 | 1);                     // Seed rand() with an odd number
        isPrepDone = 1;
    }

    switch (option)
    {
    case 'n':           // New wave-table
    {
        int     arg1 = atoi(argVal[2]);

        if (argCount == 2)  // size not specified - use default
        {
            putstr("Table size not specified - using default: 1260 \n");
            TableSize = 1260;
        }
        else if (arg1 >= 200 && arg1 <= WAVE_TABLE_MAXIMUM_SIZE)
        {
            TableSize = arg1;
        }
        else  // size out of range
        {
            sprintf(textBuf, "! Table size must be 200..%d samples. \n", WAVE_TABLE_MAXIMUM_SIZE);
            putstr(textBuf);
            break;
        }

        WaveTableSizeSet(TableSize);
        Osc1FreqDividerSet(1.0);
        WaveTableName[0] = 0;
        ClearWaveTable();
        isHammond = FALSE;
        putstr("Output buffer is empty... Ready to add partial.\n");
        break;
    }
    case 'f':           // Set Osc. Freq. Divider for sound test
    {
        float   arg = atof(argVal[2]);

        if (argCount == 3 && arg >= 0.001)  Osc1FreqDividerSet(arg);

        sprintf(textBuf, "Osc.Freq.Div: %8.4f \n", Osc1FreqDividerGet());
        putstr(textBuf);
        break;
    }
    case 'l':           // List wave-table & partial stat's
    {
        ListWaveParameters();
        break;
    }
    case 'a':           // Add partial to wave-table buffer
    {
        int   order = atoi(argVal[2]);
        int   ampld = atoi(argVal[3]);
        int   peak_ampld;

        if (order < 1 || order > 16)
        {
            putstr("! Invalid order - Partial not generated. \n");
            break;
        }

        if (ampld <= 0 || ampld > 99)
        {
            putstr("! Ampld must be 1..99 (%) - Partial not generated.\n");
            break;
        }

        PartialOrder = order;
        PartialAmpld = ampld;
        peak_ampld = AddPartialToWaveTable();
        isHammond = FALSE;

        putstr("* Partial order ");
        putDecimal(order, 1);
        putstr(" added to wave-table.\n");

        if (peak_ampld == ERROR)
            putstr("! Clipping occurred in composite waveform.\n");
        else
        {
            putstr("  Waveform peak amplitude: ");
            putDecimal(peak_ampld, 1);
            putstr(" (");
            putDecimal((100 * peak_ampld) / 32000, 1);
            putstr("% FS) \n");
        }
        break;
    }
    case 'c':           // Clear partial from wave-table
    {
        int   order = atoi(argVal[2]);

        if (order < 1 || order > 16)
        {
            putstr("! Invalid order - nothing changed. \n");
            break;
        }
        
        RemovePartialFromWaveTable(order);
        putstr("* Partial removed from waveform.\n");
        break;
    }
    case 'h':           // Create wave-table using Hammond drawbar settings
    {
        if (argCount < 3)   // more help wanted
        {
            putstr("Create a wave-table using Hammond organ drawbar settings. \n");
            putstr("Command usage: \n");
            putstr("wav -h  [arg1] [arg2] [arg3] ... [arg9] \n");
            putstr("   where <arg1>..<arg9> are drawbar settings (range 0..8) \n");
            putstr("The table peak level is scaled to the highest drawbar setting.\n");
            putstr("Set Osc.Freq.Divider = 2.0 in the wave-table descriptor.\n\n");

            putstr("  +----------------------------------------------------------+ \n");
            putstr("  | Drawbar # |  1  |  2  |  3 |  4 |  5 |  6 |  7 |  8 |  9 | \n");
            putstr("  | Harmonic  | 0.5 | 1.5 |  1 |  2 |  3 |  4 |  5 |  6 |  8 | \n");
            putstr("  | Partial # |  1  |  3  |  2 |  4 |  6 |  8 | 10 | 12 | 16 | \n");
            putstr("  +----------------------------------------------------------+ \n\n");
        }
        else  GenerateWaveTableFromDrawbars(argCount, argVal);
        
        break;
    }
    case 'z':           // Scale samples in wave buffer
    {
        uint16   peak_pc = atoi(argVal[2]);

        if (argCount < 3 || peak_pc > 100)  peak_pc = 96;

        if (!isTableEmpty)
        {
            ScaleTablePeakMagnitude(peak_pc);
            ListWaveParameters();
        }
        else  putstr("! Buffer is empty. \n");
        break;
    }
    case 'd':           // Dump wave-table as C array def'n
    {
        if (argCount >= 3 && strlen(argVal[2]) > 0)  // waveform name supplied
        {
            strncpy(WaveTableName, argVal[2], 40);
            WaveTableName[40] = 0;  // NUL term-char
        }

        if (!isTableEmpty)  DumpWaveTable();
        else  putstr("! Buffer is empty. \n");
        break;
    }
    case 's':           // Save User WaveTable param's in EEPROM
    {
        int   i;

        if (isTableEmpty)
        {
            putstr("! Buffer is empty. \n");
            break;
        }

        g_Config.UserWaveform.Size = TableSize;
        g_Config.UserWaveform.FreqDiv = Osc1FreqDividerGet();

        for (i = 0;  i < 16;  i++)
        {
            g_Config.UserWaveform.Partial[i] = PartialAmpldHist[i];
        }

        if (StoreConfigData())
            putstr("* User Wave-table param's saved OK.\n");
        else  putstr("! Error: EEPROM write failed.\n");
        break;
    }
    case 'k':           // Apply anti-alias filter, time-constant K = arg1
    {
        int  k = 50;

        if (argCount >= 3) k = atoi(argVal[2]);
        else  putstr("! Missing arg - using default (K = 50).\n");
        
        if (k <= 0 || k > 500)  
            putstr("! Arg out of bounds. - Filter not applied.\n");
        else
        {
            AliasFilter_K = k;
            ApplyAntiAliasFilter();
            putstr("* Filter applied. Normalization (-z) recommended.\n");
        }
        break;
    }
    case 'x':           // Exit 'wav' utility, restore previous patch
    {
        SynthPatchSelect(previousPatch);
        isPrepDone = FALSE;
        isHammond = FALSE;
        break;
    }

    default:
        putstr("! Undefined cmd option.\n");
        break;

    } // end switch
}


PRIVATE  void  ClearWaveTable(void) 
{
    int   i;

    for (i = 0;  i < TableSize;  i++)
    {
        WaveTableBuffer[i] = 0;
    }

    for (i = 0;  i < 16;  i++)
    {
        PartialAmpldHist[i] = 0;
    }

    isTableEmpty = TRUE;
}


/*`````````````````````````````````````````````````````````````````````````````````````````````````
 * Function outputs wave-table parameters in human-readable form.
 * Called by "wav -l" command option.
 */
PRIVATE  void   ListWaveParameters(void)
{
    int   order, i;
    int   absValue, peakAmpld = 0;
    char  textBuf[80];
    bool  isAdditiveWave = 0;  

    if (strlen(WaveTableName) > 2)  // have name
    {
        putstr("Table name:   ");  putstr(WaveTableName);  putstr("\n");
    }
    putstr("Table size:   ");  putDecimal(TableSize, 1);  putstr(" samples \n");
    sprintf(textBuf, "Osc.Freq.Div: %7.3f \n", Osc1FreqDividerGet());
    putstr(textBuf);

    if (isTableEmpty)  
    {
        putstr("! Buffer is empty. \n");
        return;
    }
    
    for (order = 1;  order <= 16;  order++)
    {
        if (PartialAmpldHist[order-1] != 0) isAdditiveWave = 1;
    }
    
    if (isHammond)
    {
        putstr("Hammond registration: \n");
        putstr("Drawbar | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | \n");
        putstr("Setting | ");
        for (i = 0;  i < 9;  i++)
        {
            putDecimal((int)drawbar_setting[i], 1);
            putstr(" | ");
        }
        putstr("\n");
    }
    
    if (isAdditiveWave)
    {
        putstr("Partial Distribution: \n");
        putstr("Order # |  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 | 10 | 11 | 12 ");
        putstr("| 13 | 14 | 15 | 16 |\n");
        putstr("Ampld % | ");
        for (order = 1;  order <= 16;  order++)
        {
            if (PartialAmpldHist[order-1] != 0)
                putDecimal(PartialAmpldHist[order-1], 2);
            else  putstr("  ");
            putstr(" | ");
        }
        putstr("\n");
    }
    
    // This loop finds the peak magnitude of samples in the wave-table
    for (i = 0;  i < TableSize;  i++)
    {
        absValue = abs(WaveTableBuffer[i]);
        if (absValue > peakAmpld) peakAmpld = absValue;
    }
    
    sprintf(textBuf, "Peak ampld:  %d (%d %%FS) \n", peakAmpld, (peakAmpld * 100) / 32000);
    putstr(textBuf);
}


/*`````````````````````````````````````````````````````````````````````````````````````````````````
 * Function adds the specified partial to the wave-table output buffer.
 * Called by "wav -a" command option.
 *
 * It is assumed that the required partial wave parameters have been set up prior, i.e.
 *     PartialOrder, PartialAmpld, PartialPhase.
 *
 * Return val:  (int)  positive peak sample value of updated wave-table, or
 *                     ERROR (-1) if clipping occurred.
 *                     Clipping threshold is +/-31900 (= 32 x 999 approx.)
 */
PRIVATE  int  AddPartialToWaveTable(void)
{
    int   sample_val, partial_val;
    int   new_sample_val, peak_val = 0;
    int   i = 0;
    int   order = PartialOrder;  // 1..16
    int   period = TableSize / PartialOrder;

    for (i = 0;  i < TableSize;  i++)
    {
        sample_val = CalcWaveSample(SINE_WAVE, i, period);
        partial_val = (sample_val * PartialAmpld) / 100;
        new_sample_val = WaveTableBuffer[i] + partial_val;

        if (new_sample_val < -CLIP_THRESHOLD)
        {
            WaveTableBuffer[i] = -CLIP_THRESHOLD;
            peak_val = ERROR;
        }
        else if (new_sample_val > CLIP_THRESHOLD)
        {
            WaveTableBuffer[i] = CLIP_THRESHOLD;
            peak_val = ERROR;
        }
        else  WaveTableBuffer[i] = new_sample_val;

        if (peak_val != ERROR && new_sample_val > peak_val)
            peak_val = new_sample_val;
    }

    // Update the Partial Histogram array...
    if (order <= 16 && (PartialAmpldHist[order-1] + PartialAmpld) < 200)
        PartialAmpldHist[order-1] += PartialAmpld;
    else  PartialAmpldHist[order-1] = 200;  // Limit

    isTableEmpty = FALSE;
    return  peak_val;
}


/*`````````````````````````````````````````````````````````````````````````````````````````````````
 * Function effectively removes the specified partial from the wave-table buffer.
 * Called by "wav -c <n>" (Clear) command option.
 *
 * The buffered waveform is regenerated using the partial distribution stored in the
 * array PartialAmpldHist[], except that the unwanted partial has ampld set to zero.
 * 
 * All partials are assumed to be sinusoidal.
 */
PRIVATE  void  RemovePartialFromWaveTable(int order)
{
    int  i;
    
    ClearWaveTable();

    PartialAmpldHist[order-1] = 0;  // unwanted partial

    for (i = 0;  i < 16;  i++)
    {
        PartialOrder = i + 1;
        PartialAmpld = PartialAmpldHist[i];
        if (PartialAmpld != 0) AddPartialToWaveTable();
    }
}


/*`````````````````````````````````````````````````````````````````````````````````````````````````
 * Function calculates and returns a sample value in a periodic waveform at a given point
 * in the cycle.
 *
 * Entry arg's: wave_shape   = waveform ID code (sine = 1, triangle = 2, etc)
 *              sample_point = wave angle relative to period as a number of sample points
 *                               e.g. sample_point = period/4 represents an angle of 90 deg.
 *                               (sample_point may be higher than period.)
 *              period       = value of sample_point at angle = 360 deg.
 *
 * Return val:  (int16)  sample_value, signed 16-bit integer.
 *                       Full-scale (peak value) is +/-32000 for all wave-shapes.
 */
PRIVATE  int16  CalcWaveSample(uint8 wave_shape, int sample_point, int period)
{
    static  int32   sample_value = 0;
    int16   noise_value;
    float   angle;  // radians

    sample_point = sample_point % period;  // in case sample_point > period

    if (wave_shape == SQUARE_WAVE)
    {
        sample_value = (sample_point < (period/2)) ? 32000 : -32000;
    }
    else if (wave_shape == TRIANG_WAVE)
    {
        if (sample_point < (period/4))  // up to 90 deg.
            sample_value = (32000 * sample_point) / (period/4);
        else if (sample_point < (period/2))  // more than 90 deg, up to 180 deg.
            sample_value = 32000 - (32000 * (sample_point - period/4)) / (period/4);
        else if (sample_point < (3 * period) / 4)  // more than 180 deg, up to 270 deg
            sample_value = 0 - (32000 * (sample_point - period/2)) / (period/4);
        else  // more than 270 deg, up to 360 deg.
            sample_value = (32000 * (sample_point - (3 * period) / 4)) / (period/4) - 32000;
    }
    else if (wave_shape == RAMP_WAVE)
    {
        sample_value = (2 * 32000 * sample_point) / period - 32000;
    }
    else if (wave_shape == NOISE_WAVE)
    {
        do  { noise_value = (int16)(rand() & 0xFFFF); }
        until (noise_value > -32000 && noise_value < 32000);  // limits
    }
    else  // assume (wave_shape == SINE_WAVE)
    {
        if (period == SINE_WAVE_TABLE_SIZE)  // fastest execution
            sample_value = (int) g_sinewave[sample_point];
        else
        {
            angle = (2 * PI * (float) sample_point) / period;
            sample_value = (int) (32000 * sinf(angle));
        }
    }

    return  (int16) sample_value;
}


/*`````````````````````````````````````````````````````````````````````````````````````````````````
 * Function scales all sample values in the wave-table buffer, so that the peak
 * magnitude is a given percentage of full-scale (FS = 32000). If no peak value is
 * supplied (arg == 0), then the peak magnitude will be 30000 (approx. 95% FS).
 * The Partial Histogram array is scaled by the same factor.
 *
 * Called by "wav -z" (normalize) command option.
 */
PRIVATE  void  ScaleTablePeakMagnitude(uint16 peakPercent)
{
    int    i;
    int    abs_value;
    int    peakTarget;
    int    peakAmpld = 0;      // peak magnitude of samples
    float  scaleFactor;

    // This loop finds the peak magnitude of samples in the wave-table
    for (i = 0;  i < TableSize;  i++)
    {
        abs_value = abs(WaveTableBuffer[i]);
        if (abs_value > peakAmpld) peakAmpld = abs_value;
    }

    if (peakAmpld != 0)
    {
        peakTarget = 32000 * peakPercent / 100;
        scaleFactor = (float) peakTarget / peakAmpld;

        for (i = 0;  i < TableSize;  i++)
        {
            WaveTableBuffer[i] = (int16) (scaleFactor * WaveTableBuffer[i]);
        }

        for (i = 0;  i < 16;  i++)    // Adjust partial histogram
        {
            PartialAmpldHist[i] = (float) PartialAmpldHist[i] * scaleFactor;
            if (PartialAmpldHist[i] > 99) PartialAmpldHist[i] = 99;
        }
    }
}


/*`````````````````````````````````````````````````````````````````````````````````````````````````
 * Function:    ApplyAntiAliasFilter()
 *
 * The buffered waveform is low-pass filtered to remove high-order partials which might
 * otherwise cause aliasing with the sample frequency.  The filter works by averaging a
 * finite number of consecutive samples (= AliasFilter_K).
 */
PRIVATE  void   ApplyAntiAliasFilter()
{
    int16   saveBuf[512];
    int32   sum;
    int     i, j, k;

    if (isTableEmpty)
    {
        putstr("! Buffer is empty. \n");
        return;
    }

    for (k = 0;  k < AliasFilter_K;  k++)  // Save the first K samples
    {
        saveBuf[k] = WaveTableBuffer[k];
    }

    for (i = 0;  i < TableSize;  i++)
    {
        // Find the sum of K consecutive samples at WaveTableBuffer[i]
        for (sum = 0, k = 0;  k < AliasFilter_K;  k++)
        {
            j = i + k;
            if (j < TableSize)  sum += WaveTableBuffer[j];
            else  sum += saveBuf[k];  // use preserved value
        }
        WaveTableBuffer[i] = sum / AliasFilter_K;  // filtered sample = mean
    }
}


/*`````````````````````````````````````````````````````````````````````````````````````````````````
 * Function outputs the wave-table as an array of sample values (decimal integers),
 * in C source syntax, i.e. initialized array of constants.
 * Called by "wav -c" command option. Intended for development purposes only.
 *
 * Entry arg. format = 0 for brief, 1 to show partial distribution
 */
PRIVATE  void  DumpWaveTable(void)
{
    char   outBuf[80];
    int16  sample;
    int    i, order, column = 0;

    // Print wave-table info as C comment block
    putstr("\n\n/* \n");
    if (strlen(WaveTableName) > 2)  // Have table name
    {
        putstr(" * Wave-table name: ");  putstr(WaveTableName);  putstr("\n");
    }
    sprintf(outBuf, " * Size: %d samples \n", WaveTableName, TableSize);
    putstr(outBuf);
    sprintf(outBuf, " * Osc. Freq. Divider: %7.3f \n", Osc1FreqDividerGet());
    putstr(outBuf);

    if (isHammond)
    {
        putstr(" * Hammond registration: \n");
        putstr(" * Drawbar | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | \n");
        putstr(" * Setting | ");
        for (i = 0;  i < 9;  i++)
        {
            putDecimal((int)drawbar_setting[i], 1);
            putstr(" | ");
        }
        putstr("\n");
    }
    putstr(" */ \n");

    if (strlen(WaveTableName) > 2)  // Have table name
    {
        sprintf(outBuf, "const  int16  %s[] = \n{ ", WaveTableName);  putstr(outBuf);
    }
    else  putstr("const  int16  untitled_wave[] = \n{ ");

    for (i = 0;  i < TableSize;  i++)
    {
        if ((column % 10) == 0) putstr("\n    ");
        sample = WaveTableBuffer[i];
        sprintf(outBuf, "%6d", sample);
        putstr(outBuf);
        if (i < (TableSize - 1)) putstr(", ");
        if (++column >= 10)  column = 0;
    }

    putstr("\n}; \n\n");
}


/*`````````````````````````````````````````````````````````````````````````````````````````````````
 * Populate a wave-table according to Hammond organ drawbar settings.
 * Called by "wav -h" command option.  (See help text for -h option.)
 */
PRIVATE  void   GenerateWaveTableFromDrawbars(int argCount, char * argVal[])
{
    static  int   partial[] = { 1, 3, 2, 4, 6, 8, 10, 12, 16 };
    static  int   ampld_pc[] = { 0, 3, 5, 10, 18, 32, 50, 71, 99 };  // %FS

    int  idx, setting, peak_ampld;
    int  ampld_sum = 0;
    int  max_setting = 0;

    isHammond = TRUE;
    WaveTableSizeSet(1260);
    Osc1FreqDividerSet(2.0);
    WaveTableName[0] = 0;
    ClearWaveTable();
    
    for (idx = 0;  idx < 9;  idx++)  // Extract up to 9 args;  find max_setting
    {
        if (idx >= (argCount - 2))  setting = 0;  // no more arg's
        else  setting = atoi(argVal[idx + 2]);    

        if (setting >= 0 && setting <= 8)  // allowed range: 0..8
        {
            drawbar_setting[idx] = setting;
            ampld_sum += ampld_pc[setting];
            if (setting > max_setting)  max_setting = setting;
        }
        else  drawbar_setting[idx] = 0;  // arg out of range
    }

    for (idx = 0;  idx < 9;  idx++)  // process 9 drawbar settings
    {
        PartialOrder = partial[idx];
        setting = drawbar_setting[idx];
        PartialAmpld = (100 * ampld_pc[setting]) / ampld_sum;  // %FS
        peak_ampld = AddPartialToWaveTable();
        if (peak_ampld == ERROR)  break;
    }

    if (peak_ampld == ERROR)  // Highly unlikely!
        putstr("! Clipping occurred... Wave-table build terminated.\n");
    else  
    {   // Scale wave-table samples according to highest drawbar setting
        ScaleTablePeakMagnitude(ampld_pc[max_setting]);
        ListWaveParameters();  // done!
    }
}


/*`````````````````````````````````````````````````````````````````````````````````````````````````
 * Populate wave-table with pseudo-random sample valuess.
 */
PRIVATE  void  GeneratePseudoRandomWave(int srlen, int fbbits, int init)
{
    int     i;
	uint16  so;     // SR serial out (bit_0)
	int16   sample;

	if (srlen < 5) srlen = 5;
	if (srlen > 15) srlen = 15;
    if (init <= 0) init = 1;

	PseudoRandomSequence(srlen, init, fbbits);  // Initialize the sequence

    for (i = 0;  i < TableSize;  i++)
    {
		so = PseudoRandomSequence(srlen, 0, fbbits) & 1;
		if (so == 0)  sample = -16000;  else sample = 16000;
        WaveTableBuffer[i] = sample;
    }
}


/*`````````````````````````````````````````````````````````````````````````````````````````````````
 * Pseudo-Random Sequence Generator using Shift-Register with Feedback.
 * Register length is variable, 5 to 15 bits, set by arg1 (srlen).
 * The register is "clocked" (shifted 1 bit posn) on each call.
 * To generate a binary sequence, use only bit_0 of the return value.
 *
 * Entry arg's: (uint8)  srlen  = shift-register length (5..15 bits)
 *
 *              (uint16) seed   = initial shiftreg data if non-zero (0..0x7FFF)
 *                                ... shiftreg unchanged if seed is zero.
 * 
 *              (uint16) fbmask = feedback bit mask (0..0x3FFF)
 *
 * Return val:  (uint16) Next number in sequence, unsigned integer
 */
PRIVATE  uint16  PseudoRandomSequence(uint8 srlen, uint16 seed, uint16 fbmask)
{
    static  uint32  shiftreg;  // shift register (5..15 LS bits)
    int     i;
    uint32  exorbits;  // bits to be XOR'd together to feed shiftreg input (si)
    uint32  si = 1;    // shiftreg serial input bit
	uint32  carrybit = (1 << srlen);  // bit position of si in shiftreg

    if (seed != 0)  // one-time initialization
    {
        shiftreg = (uint32) seed;
    }
    else  // normal running
    {
        exorbits = shiftreg & fbmask;

        for (i = 0;  i < srlen;  i++)   // calculate si = XOR of feedback bits
        {
            si = si ^ (exorbits & 1);
            exorbits = exorbits >> 1;
        }

        shiftreg &= ~carrybit;   // Clear carry bit
        if (si == 1) shiftreg |= carrybit;  // Set carry bit
        shiftreg = shiftreg >> 1;  // Shift right 1 bit (unsigned)
    }

    return  (uint16) shiftreg;
}


/*=============================  LICENSE AGREEMENT  ===================================*\
 *
 *  THIS SOURCE CODE MAY BE USED FREELY FOR PERSONAL NON-COMMERCIAL APPLICATIONS.
 *  HOWEVER, THE CODE OR PART THEREOF MAY NOT BE REDISTRIBUTED OR SOLD IN ANY FORM.
 *  USE OF THIS SOURCE CODE OR OBJECT CODE GENERATED FROM IT FOR COMMERCIAL PURPOSES
 *  WITHOUT THE EXPRESS WRITTEN PERMISSION OF THE AUTHOR IS PROHIBITED.
 *
\*=====================================================================================*/

