/****************************************************************************************
 *
 * FileName:   main_remi_synth2.h 
 *
 * File contains build options and particular def's for the application.
 * See also: "HardwareProfile.h"
 *
 * =======================================================================================
 */
#ifndef _MAIN_REMI_SYNTH2_H
#define _MAIN_REMI_SYNTH2_H

// Symbol must be defined in Project Properties / GCC / Preprocessor macros...
#ifndef BUILD_REMI_SYNTH2_APP
#error "Symbol BUILD_REMI_SYNTH2_APP must be defined for this project!"
#endif

// =======================================================================================
//                       FIRMWARE VERSION NUMBER AND BUILD OPTIONS
//
#define BUILD_VER_MAJOR   2
#define BUILD_VER_MINOR   4
#define BUILD_VER_DEBUG   0
//
// =======================================================================================

#include "../Common/system_def.h"
#include "HardwareProfile.h"       // must be included before driver defs

#include "../Drivers/EEPROM_drv.h"
#include "../Drivers/MRF24J40_lib.h"
#include "../Drivers/UART_drv.h"
#include "../Drivers/SPI_drv.h"
#include "../Drivers/I2C_drv.h"

#include "pic32_low_level.h"
#include "LCD_graphics_lib.h"
#include "gfx_image_data.h"
#include "console_cli.h"
#include "MIDI_comms_lib.h"
#include "remi_synth2_config.h"
#include "remi_synth2_def.h"
#include "remi_synth2_CLI.h"
#include "remi_synth2_GUI.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#if UART1_TX_USING_QUEUE
#define MidiOutputQueueHandler()  UART1_TxQueueHandler() 
#else
#define MidiOutputQueueHandler()  {}
#endif

// MIDI System Exclusive message types unique to REMI...
#define REMI_PRESET_MSG   0x07     // 'REMI PRESET' msg type (set Preset #)
#define REMI_IDENT_MSG    0x30     // 'REMI IDENT' msg type (periodic ID)
#define MIDI_MON_BUFFER_SIZE        512    // bytes
#define HANDSET_CONNECTION_TIMEOUT  1000   // ms

// global data
extern  uint8   g_FW_version[];          // firmware version # (major, minor, build, 0)
extern  int     g_SoftTimerError;        // % error
extern  uint8   g_SelfTestFault[];       // Self-test fault codes (0 => no fault)
extern  uint32  g_counter;               // temp, for debug
extern  int32   g_TraceBuffer[][5];      // Debug usage only
extern  float   g_SynthNoiseGain ;       // Noise gen. output adjustment (0..250)
extern  float   g_SynthNoiseFilterFc;    // Noise filter Fc = 40000/Kt Hz 
extern  uint8   g_HandsetInfo[];         // REMI handset info from Sys.Ex. msg
extern  BOOL    g_MidiInputMonitorActive;
extern  uint8   g_MidiInputBuffer[];     // MIDI IN monitor buffer (circular FIFO)
extern  short   g_MidiInWriteIndex;      // MIDI IN monitor buffer write index
extern  int     g_MidiInputByteCount;    // MIDI IN monitor received byte count
extern  int     g_PressureMsgCount;      // Number of Pressure CC msg's Rx'd
extern  int     g_ModulationMsgCount;    // Number of Modulation CC msg's Rx'd
extern  int     g_SysExclusivMsgCount;   // Number of REMI Sys.Ex. msg's Rx'd

enum  Self_test_categories
{
    TEST_SOFTWARE_TIMER = 0,
    TEST_DEVICE_ID,
    TEST_MIDI_IN_COMMS,
    TEST_EEPROM,
    NUMBER_OF_SELFTEST_ITEMS
};

// Public functions defined in "main_remi_synth2.c" ----------------------
//
void   MidiInputService();
void   InstrumentPresetSelect(uint8 preset);
bool   isLCDModulePresent();
bool   isHandsetConnected();

// Public functions defined in "wave_table_creator.c" -------------------------
void   GenerateWaveTable(WaveformDesc_t *waveDesc);

#endif // _MAIN_REMI_SYNTH2_H
