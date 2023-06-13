/****************************************************************************************
 *
 * FileName:   remi_synth_main.h 
 *
 * File contains build options and particular def's for the application.
 *
 * =======================================================================================
 */
#ifndef _REMI_SYNTH_MAIN_H
#define _REMI_SYNTH_MAIN_H

// =======================================================================================
// FIRMWARE VERSION NUMBER 
//
#define BUILD_VER_MAJOR   3
#define BUILD_VER_MINOR   0
#define BUILD_VER_DEBUG   50
//
// =======================================================================================

#include "../Common/system_def.h"
#include "../Drivers/HardwareProfile.h"
#include "../Drivers/EEPROM_drv.h"
#include "../Drivers/UART_drv.h"
#include "../Drivers/SPI_drv.h"
#include "../Drivers/I2C_drv.h"

#include "pic32_low_level.h"
#include "LCD_graphics_lib.h"
#include "console_cli.h"
#include "MIDI_comms_lib.h"
#include "remi_synth_config.h"
#include "remi_synth_def.h"
#include "remi_synth_CLI.h"
#ifdef SYNTH_MK2_MX340_LITE  // Symbol defined in 'Project Properties'
#include "remi_synth_GUI_lite.h"
#else
#include "remi_synth_GUI.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#if UART1_TX_USING_QUEUE
#define MidiOutputQueueHandler()  UART1_TxQueueHandler() 
#else
#define MidiOutputQueueHandler()  {}
#endif

#define POT_MODULE_CONNECTED  (READ_HW_CFG_JUMPER_P0 == 0)

// MIDI System Exclusive message types unique to REMI...
#define REMI_PRESET_MSG   0x07     // 'REMI PRESET' msg type (set Preset #)
#define REMI_IDENT_MSG    0x30     // 'REMI IDENT' msg type (periodic ID)
#define MIDI_MON_BUFFER_SIZE        512    // bytes
#define HANDSET_CONNECTION_TIMEOUT  2000   // ms

// global data
extern  uint8   g_FW_version[];          // firmware version # (major, minor, build, 0)
extern  int     g_SoftTimerError;        // % error
extern  uint8   g_SelfTestFault[];       // Self-test fault codes (0 => no fault)
extern  uint32  g_TaskRunningCount;      // Task execution counter (debug usage only)
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

#endif // _REMI_SYNTH_MAIN_H
