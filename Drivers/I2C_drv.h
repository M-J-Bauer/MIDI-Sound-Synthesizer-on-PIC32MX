/**
 * File:  I2C_drv.h
 *
 * Declarations of public functions defined in I2C_drv.c
 *
 * If any I2C ports are used in the project, the relevant symbol(s) USE_I2C_CHANNEL_#
 * (# = 1, 2, 3, 4) should be defined in "HardwareProfile.h".
 */
#ifndef I2C_DRV_H
#define I2C_DRV_H

#include "../Common/system_def.h"    // for typedefs, etc.

#if !defined USE_I2C_CHANNEL_1 && !defined USE_I2C_CHANNEL_2 \
&&  !defined USE_I2C_CHANNEL_3 && !defined USE_I2C_CHANNEL_4
# define USE_I2C_CHANNEL_1   // default in case no channels selected
#endif

#define Read_I2C1()   get_I2C1()
#define Read_I2C2()   get_I2C2()
#define Read_I2C3()   get_I2C3()
#define Read_I2C4()   get_I2C4()

#ifdef USE_I2C_CHANNEL_1
//
void   Init_I2C1(void);
char   ACKStatus_I2C1(void);
char   WaitAck_I2C1(void);
char   NotAck_I2C1(void);
int    get_I2C1(void);
int    gets_I2C1(uint8 *rdptr, uint8 length);
char   Start_I2C1(void);
char   Restart_I2C1(void);
char   Stop_I2C1(void);
char   Write_I2C1(uint8 byte);
char   Idle_I2C1(void);
char   Ack_I2C1(void);
//
int    I2C1MasterStart(uint8 addr);
int    I2C1MasterSend(uint8 byte);
int    I2C1MasterSendAckStop(uint8 byte);
int    I2C1MasterReadIdleStop(uint8 addr, uint8* buffer, uint8 length);
int    I2C1MasterReadNackStop(uint8 addr, uint8* buffer, uint8 length);
void   I2C1BusMuxChannelSelect(uint8 channel);
//
#endif  // USE_I2C_CHANNEL_1

#ifdef USE_I2C_CHANNEL_2
//
void   Init_I2C2(void);
char   ACKStatus_I2C2(void);
char   WaitAck_I2C2(void);
char   NotAck_I2C2(void);
int    get_I2C2(void);
int    gets_I2C2(uint8 *rdptr, uint8 length);
char   Start_I2C2(void);
char   Restart_I2C2(void);
char   Stop_I2C2(void);
char   Write_I2C2(uint8 byte);
char   Idle_I2C2(void);
char   Ack_I2C2(void);
//
int    I2C2MasterStart(uint8 addr);
int    I2C2MasterSend(uint8 byte);
int    I2C2MasterSendAckStop(uint8 byte);
int    I2C2MasterReadIdleStop(uint8 addr, uint8* buffer, uint8 length);
int    I2C2MasterReadNackStop(uint8 addr, uint8* buffer, uint8 length);
void   I2C2BusMuxChannelSelect(uint8 channel);
BOOL   I2C2BusErrorDetected();
int    I2C2BusErrorCount();
//
#endif  // USE_I2C_CHANNEL_2

#ifdef USE_I2C_CHANNEL_3
//
void   Init_I2C3(void);
char   ACKStatus_I2C3(void);
char   WaitAck_I2C3(void);
char   NotAck_I2C3(void);
int    get_I2C3(void);
int    gets_I2C3(uint8 *rdptr, uint8 length);
char   Start_I2C3(void);
char   Restart_I2C3(void);
char   Stop_I2C3(void);
char   Write_I2C3(uint8 byte);
char   Idle_I2C3(void);
char   Ack_I2C3(void);
//
int    I2C3MasterStart(uint8 addr);
int    I2C3MasterSend(uint8 byte);
int    I2C3MasterSendAckStop(uint8 byte);
int    I2C3MasterReadIdleStop(uint8 addr, uint8* buffer, uint8 length);
int    I2C3MasterReadNackStop(uint8 addr, uint8* buffer, uint8 length);
void   I2C3BusMuxChannelSelect(uint8 channel);
//
#endif  // USE_I2C_CHANNEL_3

#ifdef USE_I2C_CHANNEL_4
//
void   Init_I2C4(void);
char   ACKStatus_I2C4(void);
char   WaitAck_I2C4(void);
char   NotAck_I2C4(void);
int    get_I2C4(void);
int    gets_I2C4(uint8 *rdptr, uint8 length);
char   Start_I2C4(void);
char   Restart_I2C4(void);
char   Stop_I2C4(void);
char   Write_I2C4(uint8 byte);
char   Idle_I2C4(void);
char   Ack_I2C4(void);
//
int    I2C4MasterStart(uint8 addr);
int    I2C4MasterSend(uint8 byte);
int    I2C4MasterSendAckStop(uint8 byte);
int    I2C4MasterReadIdleStop(uint8 addr, uint8* buffer, uint8 length);
int    I2C4MasterReadNackStop(uint8 addr, uint8* buffer, uint8 length);
void   I2C4BusMuxChannelSelect(uint8 channel);
//
#endif  // USE_I2C_CHANNEL_4

#endif  // I2C_DRV_H
