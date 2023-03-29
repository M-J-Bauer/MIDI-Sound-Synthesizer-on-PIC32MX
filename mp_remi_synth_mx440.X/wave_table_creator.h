/**
 *   File:    wave_table_creator.h
 */
#ifndef _WAVE_TABLE_CREATOR_H
#define _WAVE_TABLE_CREATOR_H

#include "../Common/system_def.h"
#include "console_cli.h"
#include "remi_synth_config.h"
#include "remi_synth_def.h"

void   GenerateWaveTable(WaveformDesc_t  *waveDesc);
void   Cmnd_wav(int argCount, char * argVal[]);


#endif // _WAVE_TABLE_CREATOR_H
