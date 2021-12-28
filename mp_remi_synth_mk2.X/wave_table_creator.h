/**
 *   File:    wave_table_creator.h
 */
#ifndef _WAVE_TABLE_CREATOR_H
#define _WAVE_TABLE_CREATOR_H

#include "../Common/system_def.h"
#include "HardwareProfile.h"
#include "console_cli.h"

#ifdef BUILD_REMI_SYNTH2_APP
#include "remi_synth2_config.h"
#include "remi_synth2_def.h"

#else  // assume REMI mk1 synth app
#include "remi_config.h"
#include "remi_synth.h"
#endif


void   GenerateWaveTable(WaveformDesc_t  *waveDesc);
void   Cmnd_wav(int argCount, char * argVal[]);


#endif // _WAVE_TABLE_CREATOR_H
