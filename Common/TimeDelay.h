/******************************************************************************
*
* File Name:   TimeDelay.h
*
* Functions defined in TimeDelay.c
*
*******************************************************************************/
#ifndef TIME_DELAY__H
#define TIME_DELAY__H

#include "GenericTypeDefs.h"

void  Delay1us();
void  Delay10us(UINT32 DelayTime_10us);
void  DelayMs(UINT16 DelayTime_ms);

#endif // TIME_DELAY__H
