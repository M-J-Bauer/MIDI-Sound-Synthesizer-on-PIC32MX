/**
 *   File: I2C_drv.c
 *
 *   PIC32MX peripheral library supporting up to four I2C channels in master mode.
 */
#include "I2C_drv.h"

static volatile uint16 dummy;

//=================================================================================================

#ifdef USE_I2C_CHANNEL_1

/**
 * Initialize I2C1 controller in master mode.
 * I2C BRG value is set for clock rate FSCL = 400kHz (channel 1 only)
 *
 * IPMI mode not enabled
 * I2CADD is 7-bit address
 * Disable Slew Rate Control (for 100KHz)
 * Enable SM bus specification
 * Disable General Call address
 */
void  Init_I2C1(void)
{
    I2C1CON = 0x0000;
//  I2C1BRG = (485 * (PERIPH_CLOCK_HZ / 1000)) / 100000;  // F.SCL = 100kHz
    I2C1BRG = (485 * (PERIPH_CLOCK_HZ / 1000)) / 400000;  // F.SCL = 400kHz
    
    I2C1CON = 0x8300;       // Configure as noted above
    dummy = I2C1RCV;        // Clear RBF status bit
}

/**
 * Function returns the complement of the I2C ACK status flag;
 * i.e. TRUE if the slave device (SD) generated an ACK state following a byte
 * received from the master (MD), else FALSE.
 */
char  ACKStatus_I2C1(void)
{
    return (!I2C1STATbits.ACKSTAT);
}

/**
 * Function waits for slave to complete ACK state...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  WaitAck_I2C1(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;   // timeout count
    uint16  i2c_stat = 0;

    for (i = 0; i < maxAttempts; i++)
    {
        if (I2C1STATbits.ACKSTAT == 0)  // success!
        {
            i2c_stat = 1;
            break;
        }
    }
    return ( i2c_stat != 0 );
}

/**
 * Function receives a single byte response from I2C slave.
 * Returns RX byte (LSB) if success; ERROR (-1) if timeout occurred.
 */
int  get_I2C1(void)
{
    int16   rxdata;
    uint16  i;
    uint16  maxAttempts = 2000;   // timeout count
    uint16  i2c_stat = 0;

    I2C1CONbits.RCEN = 1;

    for (i = 0; i < maxAttempts; i++)
    {
        if (I2C1STATbits.RBF)
        {
            i2c_stat = 1;
            break;
        }
    }

    if (i2c_stat == 0)    // timeout
    {
        I2C1CONbits.RCEN = 0;   // error recovery!
        rxdata = (-1);
    }
    else  rxdata = I2C1RCV;

    return  rxdata;
}

/**
 * Function receives a multi-byte response packet from I2C slave.
 * All but the final byte are ACK'd.
 * Returns:  (int) Number of bytes successfully received.
 */
int  gets_I2C1(uint8 *rdptr, uint8 length)
{
    int16  rxdata;
    int16  rxcount = 0;

    while ( length-- )
    {
        rxdata = get_I2C1();
        if ( rxdata >= 0 )
        {
            *rdptr++ = rxdata;
            if ( length != 0 ) Ack_I2C1();
            rxcount++;
        }
        else  break;    // timeout occurred
    }

    return  rxcount;
}


/**
 * Generate I2C Start state...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  Start_I2C1(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;
    uint16  i2c_stat = 0;

    I2C1CONbits.SEN = 1;

    for (i = 0; i < maxAttempts; i++)
    {
        if (!I2C1CONbits.SEN)
        {
            i2c_stat = 1;
            break;
        }
    }
    if (i2c_stat == 0) I2C1CONbits.SEN = 0;  // timeout
    return ( i2c_stat != 0 );
}

/**
 * Generate I2C Repeat Start state...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  Restart_I2C1(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;
    uint16  i2c_stat = 0;

    I2C1CONbits.RSEN = 1;

    for (i = 0; i < maxAttempts; i++)
    {
        if (!I2C1CONbits.RSEN)
        {
            i2c_stat = 1;
            break;
        }
    }
    if (i2c_stat == 0) I2C1CONbits.RSEN = 0;  // timeout
    return ( i2c_stat != 0 );
}

/**
 * Generate I2C Stop state...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  Stop_I2C1(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;
    uint16  i2c_stat = 0;

    I2C1CONbits.PEN = 1;

    for (i = 0; i < maxAttempts; i++)
    {
        if(!I2C1CONbits.PEN)
        {
            i2c_stat = 1;
            break;
        }
    }
    if (i2c_stat == 0) I2C1CONbits.PEN = 0;  // timeout
    return ( i2c_stat != 0 );
}


/**
 * Transmit byte and wait till it is sent...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  Write_I2C1(unsigned char byte)
{
    uint16  i;
    uint16  maxAttempts = 2000;
    uint16  i2c_stat = 0;

    I2C1TRN = byte;

    for (i = 0; i < maxAttempts; i++)
    {
        if (!I2C1STATbits.TBF)
        {
            i2c_stat = 1;
            break;
        }
    }

    return ( i2c_stat != 0 );
}

/**
 * Function waits for I2C Idle state detected...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  Idle_I2C1(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;
    uint16  i2c_stat = 0;

    for (i = 0; i < maxAttempts; i++)
    {
        if (I2C1STATbits.TRSTAT == 0 && (I2C1CON & 0x001F) == 0)
        {
            i2c_stat = 1;
            break;
        }
    }

    return ( i2c_stat != 0 );
}

/**
 * Generate I2C ACK# state...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  Ack_I2C1(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;
    uint16  i2c_stat = 0;

    I2C1CONbits.ACKDT = 0;
    I2C1CONbits.ACKEN = 1;

    for (i = 0; i < maxAttempts; i++)
    {
        if (!I2C1CONbits.ACKEN)
        {
            i2c_stat = 1;
            break;
        }
    }

    if (i2c_stat == 0) I2C1CONbits.ACKEN = 0;  // timeout
    return ( i2c_stat != 0 );
}

/**
 * Function generates NACK state...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  NotAck_I2C1(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;   // timeout count
    uint16  i2c_stat = 0;

    I2C1CONbits.ACKDT = 1;
    I2C1CONbits.ACKEN = 1;

    for (i = 0; i < maxAttempts; i++)
    {
        if (!I2C1CONbits.ACKEN)
        {
            i2c_stat = 1;
            break;
        }
    }

    if (i2c_stat == 0) I2C1CONbits.ACKEN = 0;  // timeout
    I2C1CONbits.ACKDT = 0;
    return ( i2c_stat != 0 );
}


int  I2C1MasterStart(uint8 addr)
{
    if ( !Idle_I2C1() )  goto error_exit;
    if ( !Start_I2C1() )  goto error_exit;
    if ( !Write_I2C1(addr) )  goto error_exit;
    if ( !Idle_I2C1() )  goto error_exit;        // wait for TRSTAT == 0
    if ( !WaitAck_I2C1() )  goto error_exit;     
    return 1;

error_exit:
    return 0;
}


int  I2C1MasterSend(uint8 byte)
{
    if ( !Idle_I2C1() )  goto error_exit;
    if ( !Write_I2C1(byte) )  goto error_exit;
    if ( !Idle_I2C1() )  goto error_exit;
    if ( !WaitAck_I2C1() )  goto error_exit;     
    return 1;

error_exit:
    return 0;
}


int  I2C1MasterSendAckStop(uint8 byte)
{
    if ( !Idle_I2C1() )  goto error_exit;
    if ( !Write_I2C1(byte) )  goto error_exit;
    if ( !Idle_I2C1() )  goto error_exit;
    if ( !WaitAck_I2C1() )  goto error_exit;
    if ( !Stop_I2C1() )  goto error_exit;
    return 1;

error_exit:
    return 0;
}

int  I2C1MasterReadIdleStop(uint8 addr, uint8* buffer, uint8 length)
{
    if ( !Restart_I2C1() )  goto error_exit;
    if ( !Write_I2C1(addr | 0x01) )  goto error_exit;
    if ( !Idle_I2C1() )  goto error_exit;
    if ( !WaitAck_I2C1() )  goto error_exit;     
    if ( gets_I2C1(buffer, length) != length )  goto error_exit;
    if ( !Idle_I2C1() )  goto error_exit;
    if ( !Stop_I2C1() )  goto error_exit;
    return 1;

error_exit:
    return 0;
}

int  I2C1MasterReadNackStop(uint8 addr, uint8* buffer, uint8 length)
{
    if ( !Restart_I2C1() )  goto error_exit;
    if ( !Write_I2C1(addr | 0x01) )  goto error_exit;
    if ( !Idle_I2C1() )  goto error_exit;
    if ( !WaitAck_I2C1() )  goto error_exit;     
    if ( gets_I2C1(buffer, length) != length )  goto error_exit;
    if ( !NotAck_I2C1() )  goto error_exit;
    if ( !Stop_I2C1() )  goto error_exit;
    return 1;

error_exit:
    return 0;
}


/**
 * Function provided for systems using I2C bus multiplexer (PCA9540B).
 * The given downstream channel (0 or 1) is selected.
 */
void  I2C1BusMuxChannelSelect(uint8 channel)
{
    I2C1MasterStart(0xE0);
    I2C1MasterSendAckStop(4 + channel);
}

#endif // USE_I2C_CHANNEL_1

//=================================================================================================

#ifdef USE_I2C_CHANNEL_2

/**
 * Initialize I2C2 controller in master mode.
 * I2C BRG value is based on PBCLK = 80MHz, FSCL = 100kHz (SMBus maximum)
 *
 * IPMI mode not enabled
 * I2CADD is 7-bit address
 * Disable Slew Rate Control for 100KHz
 * Enable SM bus specification
 * Disable General Call address
 */
void Init_I2C2(void)
{
    I2C2CON = 0x0000;
    I2C2BRG = (485 * (PERIPH_CLOCK_HZ / 1000)) / 100000;  // BRG = 485 if PBCLK = 100MHz.
    I2C2CON = 0x8300;       // Configure as noted above
//  I2C2CON = 0x9200;
    dummy = I2C2RCV;        // Clear RBF status bit
}

/**
 * Function returns the complement of the I2C ACK status flag;
 * i.e. TRUE if the slave device (SD) generated an ACK state following a byte
 * received from the master (MD), else FALSE.
 */
char  ACKStatus_I2C2(void)
{
    return (!I2C2STATbits.ACKSTAT);
}

/**
 * Function waits for slave to complete ACK state...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  WaitAck_I2C2(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;   // timeout count
    uint16  i2c_stat = 0;

    for (i = 0; i < maxAttempts; i++)
    {
        if (I2C2STATbits.ACKSTAT == 0)  // success!
        {
            i2c_stat = 1;
            break;
        }
    }
    return ( i2c_stat != 0 );
}

/**
 * Function receives a single byte response from I2C slave.
 * Returns RX byte (LSB) if success; ERROR (-1) if timeout occurred.
 */
int  get_I2C2(void)
{
    int16   rxdata;
    uint16  i;
    uint16  maxAttempts = 2000;   // timeout count
    uint16  i2c_stat = 0;

    I2C2CONbits.RCEN = 1;

    for (i = 0; i < maxAttempts; i++)
    {
        if (I2C2STATbits.RBF)
        {
            i2c_stat = 1;
            break;
        }
    }

    if (i2c_stat == 0)    // timeout
    {
        I2C2CONbits.RCEN = 0;   // error recovery!
        rxdata = (-1);
    }
    else  rxdata = I2C2RCV;

    return  rxdata;
}

/**
 * Function receives a multi-byte response packet from I2C slave.
 * All but the final byte are ACK'd.
 * Returns:  (int) Number of bytes successfully received.
 */
int  gets_I2C2(uint8 *rdptr, uint8 length)
{
    int16  rxdata;
    int16  rxcount = 0;

    while ( length-- )
    {
        rxdata = get_I2C2();
        if ( rxdata >= 0 )
        {
            *rdptr++ = rxdata;
            if ( length != 0 ) Ack_I2C2();
            rxcount++;
        }
        else  break;    // timeout occurred
    }

    return  rxcount;
}


/**
 * Generate I2C Start state...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  Start_I2C2(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;
    uint16  i2c_stat = 0;

    I2C2CONbits.SEN = 1;

    for (i = 0; i < maxAttempts; i++)
    {
        if (!I2C2CONbits.SEN)
        {
            i2c_stat = 1;
            break;
        }
    }
    if (i2c_stat == 0) I2C2CONbits.SEN = 0;  // timeout
    return ( i2c_stat != 0 );
}

/**
 * Generate I2C Repeat Start state...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  Restart_I2C2(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;
    uint16  i2c_stat = 0;

    I2C2CONbits.RSEN = 1;

    for (i = 0; i < maxAttempts; i++)
    {
        if (!I2C2CONbits.RSEN)
        {
            i2c_stat = 1;
            break;
        }
    }
    if (i2c_stat == 0) I2C2CONbits.RSEN = 0;  // timeout
    return ( i2c_stat != 0 );
}


/**
 * Generate I2C Stop state...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  Stop_I2C2(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;
    uint16  i2c_stat = 0;

    I2C2CONbits.PEN = 1;

    for (i = 0; i < maxAttempts; i++)
    {
        if(!I2C2CONbits.PEN)
        {
            i2c_stat = 1;
            break;
        }
    }
    if (i2c_stat == 0) I2C2CONbits.PEN = 0;  // timeout
    return ( i2c_stat != 0 );
}


/**
 * Transmit byte and wait till it is sent...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  Write_I2C2(unsigned char byte)
{
    uint16  i;
    uint16  maxAttempts = 2000;
    uint16  i2c_stat = 0;

    I2C2TRN = byte;

    for (i = 0; i < maxAttempts; i++)
    {
        if (!I2C2STATbits.TBF)
        {
            i2c_stat = 1;
            break;
        }
    }

    return ( i2c_stat != 0 );
}

/**
 * Function waits for I2C Idle state detected...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  Idle_I2C2(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;
    uint16  i2c_stat = 0;

    for (i = 0; i < maxAttempts; i++)
    {
        if (I2C2STATbits.TRSTAT == 0 && (I2C2CON & 0x001F) == 0)
        {
            i2c_stat = 1;
            break;
        }
    }

    return ( i2c_stat != 0 );
}

/**
 * Generate I2C ACK# state...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  Ack_I2C2(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;
    uint16  i2c_stat = 0;

    I2C2CONbits.ACKDT = 0;
    I2C2CONbits.ACKEN = 1;

    for (i = 0; i < maxAttempts; i++)
    {
        if (!I2C2CONbits.ACKEN)
        {
            i2c_stat = 1;
            break;
        }
    }

    if (i2c_stat == 0) I2C2CONbits.ACKEN = 0;  // timeout
    return ( i2c_stat != 0 );
}

/**
 * Function generates NACK state...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  NotAck_I2C2(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;   // timeout count
    uint16  i2c_stat = 0;

    I2C2CONbits.ACKDT = 1;
    I2C2CONbits.ACKEN = 1;

    for (i = 0; i < maxAttempts; i++)
    {
        if (!I2C2CONbits.ACKEN)
        {
            i2c_stat = 1;
            break;
        }
    }

    if (i2c_stat == 0) I2C2CONbits.ACKEN = 0;  // timeout
    I2C2CONbits.ACKDT = 0;
    return ( i2c_stat != 0 );
}


int  I2C2MasterStart(uint8 addr)
{
    if ( !Idle_I2C2() )  goto error_exit;
    if ( !Start_I2C2() )  goto error_exit;
    if ( !Write_I2C2(addr) )  goto error_exit;
    if ( !Idle_I2C2() )  goto error_exit;        // wait for TRSTAT == 0
    if ( !WaitAck_I2C2() )  goto error_exit;     
    return 1;

error_exit:
    return 0;
}


int  I2C2MasterSend(uint8 byte)
{
    if ( !Idle_I2C2() )  goto error_exit;
    if ( !Write_I2C2(byte) )  goto error_exit;
    if ( !Idle_I2C2() )  goto error_exit;
    if ( !WaitAck_I2C2() )  goto error_exit;     
    return 1;

error_exit:
    return 0;
}


int  I2C2MasterSendAckStop(uint8 byte)
{
    if ( !Idle_I2C2() )  goto error_exit;
    if ( !Write_I2C2(byte) )  goto error_exit;
    if ( !Idle_I2C2() )  goto error_exit;
    if ( !WaitAck_I2C2() )  goto error_exit;
    if ( !Stop_I2C2() )  goto error_exit;
    return 1;

error_exit:
    return 0;
}

int  I2C2MasterReadIdleStop(uint8 addr, uint8* buffer, uint8 length)
{
    if ( !Restart_I2C2() )  goto error_exit;
    if ( !Write_I2C2(addr | 0x01) )  goto error_exit;
    if ( !Idle_I2C2() )  goto error_exit;
    if ( !WaitAck_I2C2() )  goto error_exit;     
    if ( gets_I2C2(buffer, length) != length )  goto error_exit;
    if ( !Idle_I2C2() )  goto error_exit;
    if ( !Stop_I2C2() )  goto error_exit;
    return 1;

error_exit:
    return 0;
}

int  I2C2MasterReadNackStop(uint8 addr, uint8* buffer, uint8 length)
{
    if ( !Restart_I2C2() )  goto error_exit;
    if ( !Write_I2C2(addr | 0x01) )  goto error_exit;
    if ( !Idle_I2C2() )  goto error_exit;
    if ( !WaitAck_I2C2() )  goto error_exit;     
    if ( gets_I2C2(buffer, length) != length )  goto error_exit;
    if ( !NotAck_I2C2() )  goto error_exit;
    if ( !Stop_I2C2() )  goto error_exit;
    return 1;

error_exit:
    return 0;
}


/**
 * Function provided for systems using I2C bus multiplexer (PCA9540B).
 * The given downstream channel (0 or 1) is selected.
 */
void  I2C2BusMuxChannelSelect(uint8 channel)
{
    I2C2MasterStart(0xE0);
    Delay10us(1);
    I2C2MasterSendAckStop(4 + channel);
    Delay10us(2);
}

#endif // USE_I2C_CHANNEL_2

//=================================================================================================

#ifdef USE_I2C_CHANNEL_3

/**
 * Initialize I2C3 controller in master mode.
 * I2C BRG value is set for clock rate FSCL = 100kHz (SMBus maximum)
 *
 * IPMI mode not enabled
 * I2CADD is 7-bit address
 * Disable Slew Rate Control for 100KHz
 * Enable SM bus specification
 * Disable General Call address
 */
void Init_I2C3(void)
{
    I2C3CON = 0x0000;
    I2C3BRG = (485 * (PERIPH_CLOCK_HZ / 1000)) / 100000;  // BRG = 485 if PBCLK = 100MHz.
    I2C3CON = 0x8300;       // Configure as noted above
    dummy = I2C3RCV;        // Clear RBF status bit
}

/**
 * Function returns the complement of the I2C ACK status flag;
 * i.e. TRUE if the slave device (SD) generated an ACK state following a byte
 * received from the master (MD), else FALSE.
 */
char  ACKStatus_I2C3(void)
{
    return (!I2C3STATbits.ACKSTAT);
}

/**
 * Function waits for slave to complete ACK state...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  WaitAck_I2C3(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;   // timeout count
    uint16  i2c_stat = 0;

    for (i = 0; i < maxAttempts; i++)
    {
        if (I2C3STATbits.ACKSTAT == 0)  // success!
        {
            i2c_stat = 1;
            break;
        }
    }
    return ( i2c_stat != 0 );
}

/**
 * Function receives a single byte response from I2C slave.
 * Returns RX byte (LSB) if success; ERROR (-1) if timeout occurred.
 */
int  get_I2C3(void)
{
    int16   rxdata;
    uint16  i;
    uint16  maxAttempts = 2000;   // timeout count
    uint16  i2c_stat = 0;

    I2C3CONbits.RCEN = 1;

    for (i = 0; i < maxAttempts; i++)
    {
        if (I2C3STATbits.RBF)
        {
            i2c_stat = 1;
            break;
        }
    }

    if (i2c_stat == 0)    // timeout
    {
        I2C3CONbits.RCEN = 0;   // error recovery!
        rxdata = (-1);
    }
    else  rxdata = I2C3RCV;

    return  rxdata;
}

/**
 * Function receives a multi-byte response packet from I2C slave.
 * All but the final byte are ACK'd.
 * Returns:  (int) Number of bytes successfully received.
 */
int  gets_I2C3(uint8 *rdptr, uint8 length)
{
    int16  rxdata;
    int16  rxcount = 0;

    while ( length-- )
    {
        rxdata = get_I2C3();
        if ( rxdata >= 0 )
        {
            *rdptr++ = rxdata;
            if ( length != 0 ) Ack_I2C3();
            rxcount++;
        }
        else  break;    // timeout occurred
    }

    return  rxcount;
}


/**
 * Generate I2C Start state...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  Start_I2C3(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;
    uint16  i2c_stat = 0;

    I2C3CONbits.SEN = 1;

    for (i = 0; i < maxAttempts; i++)
    {
        if (!I2C3CONbits.SEN)
        {
            i2c_stat = 1;
            break;
        }
    }
    if (i2c_stat == 0) I2C3CONbits.SEN = 0;  // timeout
    return ( i2c_stat != 0 );
}

/**
 * Generate I2C Repeat Start state...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  Restart_I2C3(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;
    uint16  i2c_stat = 0;

    I2C3CONbits.RSEN = 1;

    for (i = 0; i < maxAttempts; i++)
    {
        if (!I2C3CONbits.RSEN)
        {
            i2c_stat = 1;
            break;
        }
    }
    if (i2c_stat == 0) I2C3CONbits.RSEN = 0;  // timeout
    return ( i2c_stat != 0 );
}


/**
 * Generate I2C Stop state...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  Stop_I2C3(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;
    uint16  i2c_stat = 0;

    I2C3CONbits.PEN = 1;

    for (i = 0; i < maxAttempts; i++)
    {
        if(!I2C3CONbits.PEN)
        {
            i2c_stat = 1;
            break;
        }
    }
    if (i2c_stat == 0) I2C3CONbits.PEN = 0;  // timeout
    return ( i2c_stat != 0 );
}


/**
 * Transmit byte and wait till it is sent...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  Write_I2C3(unsigned char byte)
{
    uint16  i;
    uint16  maxAttempts = 2000;
    uint16  i2c_stat = 0;

    I2C3TRN = byte;

    for (i = 0; i < maxAttempts; i++)
    {
        if (!I2C3STATbits.TBF)
        {
            i2c_stat = 1;
            break;
        }
    }

    return ( i2c_stat != 0 );
}

/**
 * Function waits for I2C Idle state detected...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  Idle_I2C3(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;
    uint16  i2c_stat = 0;

    for (i = 0; i < maxAttempts; i++)
    {
        if (I2C3STATbits.TRSTAT == 0 && (I2C3CON & 0x001F) == 0)
        {
            i2c_stat = 1;
            break;
        }
    }

    return ( i2c_stat != 0 );
}

/**
 * Generate I2C ACK# state...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  Ack_I2C3(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;
    uint16  i2c_stat = 0;

    I2C3CONbits.ACKDT = 0;
    I2C3CONbits.ACKEN = 1;

    for (i = 0; i < maxAttempts; i++)
    {
        if (!I2C3CONbits.ACKEN)
        {
            i2c_stat = 1;
            break;
        }
    }

    if (i2c_stat == 0) I2C3CONbits.ACKEN = 0;  // timeout
    return ( i2c_stat != 0 );
}

/**
 * Function generates NACK state...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  NotAck_I2C3(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;   // timeout count
    uint16  i2c_stat = 0;

    I2C3CONbits.ACKDT = 1;
    I2C3CONbits.ACKEN = 1;

    for (i = 0; i < maxAttempts; i++)
    {
        if (!I2C3CONbits.ACKEN)
        {
            i2c_stat = 1;
            break;
        }
    }

    if (i2c_stat == 0) I2C3CONbits.ACKEN = 0;  // timeout
    I2C3CONbits.ACKDT = 0;
    return ( i2c_stat != 0 );
}


int  I2C3MasterStart(uint8 addr)
{
    if ( !Idle_I2C3() )  goto error_exit;
    if ( !Start_I2C3() )  goto error_exit;
    if ( !Write_I2C3(addr) )  goto error_exit;
    if ( !Idle_I2C3() )  goto error_exit;        // wait for TRSTAT == 0
    if ( !WaitAck_I2C3() )  goto error_exit;     
    return 1;

error_exit:
    return 0;
}


int  I2C3MasterSend(uint8 byte)
{
    if ( !Idle_I2C3() )  goto error_exit;
    if ( !Write_I2C3(byte) )  goto error_exit;
    if ( !Idle_I2C3() )  goto error_exit;
    if ( !WaitAck_I2C3() )  goto error_exit;     
    return 1;

error_exit:
    return 0;
}


int  I2C3MasterSendAckStop(uint8 byte)
{
    if ( !Idle_I2C3() )  goto error_exit;
    if ( !Write_I2C3(byte) )  goto error_exit;
    if ( !Idle_I2C3() )  goto error_exit;
    if ( !WaitAck_I2C3() )  goto error_exit;
    if ( !Stop_I2C3() )  goto error_exit;
    return 1;

error_exit:
    return 0;
}

int  I2C3MasterReadIdleStop(uint8 addr, uint8* buffer, uint8 length)
{
    if ( !Restart_I2C3() )  goto error_exit;
    if ( !Write_I2C3(addr | 0x01) )  goto error_exit;
    if ( !Idle_I2C3() )  goto error_exit;
    if ( !WaitAck_I2C3() )  goto error_exit;     
    if ( gets_I2C3(buffer, length) != length )  goto error_exit;
    if ( !Idle_I2C3() )  goto error_exit;
    if ( !Stop_I2C3() )  goto error_exit;
    return 1;

error_exit:
    return 0;
}

int  I2C3MasterReadNackStop(uint8 addr, uint8* buffer, uint8 length)
{
    if ( !Restart_I2C3() )  goto error_exit;
    if ( !Write_I2C3(addr | 0x01) )  goto error_exit;
    if ( !Idle_I2C3() )  goto error_exit;
    if ( !WaitAck_I2C3() )  goto error_exit;     
    if ( gets_I2C3(buffer, length) != length )  goto error_exit;
    if ( !NotAck_I2C3() )  goto error_exit;
    if ( !Stop_I2C3() )  goto error_exit;
    return 1;

error_exit:
    return 0;
}


/**
 * Function provided for systems using I2C bus multiplexer (PCA9540B).
 * The given downstream channel (0 or 1) is selected.
 */
void  I2C3BusMuxChannelSelect(uint8 channel)
{
    I2C3MasterStart(0xE0);
    I2C3MasterSendAckStop(4 + channel);
}

#endif // USE_I2C_CHANNEL_3

//=================================================================================================

#ifdef USE_I2C_CHANNEL_4

/**
 * Initialize I2C4 controller in master mode.
 * I2C BRG value is set for clock rate FSCL = 100kHz (SMBus maximum)
 *
 * IPMI mode not enabled
 * I2CADD is 7-bit address
 * Disable Slew Rate Control for 100KHz
 * Enable SM bus specification
 * Disable General Call address
 */
void Init_I2C4(void)
{
    I2C4CON = 0x0000;
    I2C4BRG = (485 * (PERIPH_CLOCK_HZ / 1000)) / 100000;  // BRG = 485 if PBCLK = 100MHz.
    I2C4CON = 0x8300;       // Configure as noted above
    dummy = I2C4RCV;        // Clear RBF status bit
}

/**
 * Function returns the complement of the I2C ACK status flag;
 * i.e. TRUE if the slave device (SD) generated an ACK state following a byte
 * received from the master (MD), else FALSE.
 */
char  ACKStatus_I2C4(void)
{
    return (!I2C4STATbits.ACKSTAT);
}

/**
 * Function waits for slave to complete ACK state...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  WaitAck_I2C4(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;   // timeout count
    uint16  i2c_stat = 0;

    for (i = 0; i < maxAttempts; i++)
    {
        if (I2C4STATbits.ACKSTAT == 0)  // success!
        {
            i2c_stat = 1;
            break;
        }
    }
    return ( i2c_stat != 0 );
}

/**
 * Function receives a single byte response from I2C slave.
 * Returns RX byte (LSB) if success; ERROR (-1) if timeout occurred.
 */
int  get_I2C4(void)
{
    int16   rxdata;
    uint16  i;
    uint16  maxAttempts = 2000;   // timeout count
    uint16  i2c_stat = 0;

    I2C4CONbits.RCEN = 1;

    for (i = 0; i < maxAttempts; i++)
    {
        if (I2C4STATbits.RBF)
        {
            i2c_stat = 1;
            break;
        }
    }

    if (i2c_stat == 0)    // timeout
    {
        I2C4CONbits.RCEN = 0;   // error recovery!
        rxdata = (-1);
    }
    else  rxdata = I2C4RCV;

    return  rxdata;
}

/**
 * Function receives a multi-byte response packet from I2C slave.
 * All but the final byte are ACK'd.
 * Returns:  (int) Number of bytes successfully received.
 */
int  gets_I2C4(uint8 *rdptr, uint8 length)
{
    int16  rxdata;
    int16  rxcount = 0;

    while ( length-- )
    {
        rxdata = get_I2C4();
        if ( rxdata >= 0 )
        {
            *rdptr++ = rxdata;
            if ( length != 0 ) Ack_I2C4();
            rxcount++;
        }
        else  break;    // timeout occurred
    }

    return  rxcount;
}


/**
 * Generate I2C Start state...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  Start_I2C4(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;
    uint16  i2c_stat = 0;

    I2C4CONbits.SEN = 1;

    for (i = 0; i < maxAttempts; i++)
    {
        if (!I2C4CONbits.SEN)
        {
            i2c_stat = 1;
            break;
        }
    }
    if (i2c_stat == 0) I2C4CONbits.SEN = 0;  // timeout
    return ( i2c_stat != 0 );
}

/**
 * Generate I2C Repeat Start state...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  Restart_I2C4(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;
    uint16  i2c_stat = 0;

    I2C4CONbits.RSEN = 1;

    for (i = 0; i < maxAttempts; i++)
    {
        if (!I2C4CONbits.RSEN)
        {
            i2c_stat = 1;
            break;
        }
    }
    if (i2c_stat == 0) I2C4CONbits.RSEN = 0;  // timeout
    return ( i2c_stat != 0 );
}


/**
 * Generate I2C Stop state...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  Stop_I2C4(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;
    uint16  i2c_stat = 0;

    I2C4CONbits.PEN = 1;

    for (i = 0; i < maxAttempts; i++)
    {
        if(!I2C4CONbits.PEN)
        {
            i2c_stat = 1;
            break;
        }
    }
    if (i2c_stat == 0) I2C4CONbits.PEN = 0;  // timeout
    return ( i2c_stat != 0 );
}


/**
 * Transmit byte and wait till it is sent...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  Write_I2C4(unsigned char byte)
{
    uint16  i;
    uint16  maxAttempts = 2000;
    uint16  i2c_stat = 0;

    I2C4TRN = byte;

    for (i = 0; i < maxAttempts; i++)
    {
        if (!I2C4STATbits.TBF)
        {
            i2c_stat = 1;
            break;
        }
    }

    return ( i2c_stat != 0 );
}

/**
 * Function waits for I2C Idle state detected...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  Idle_I2C4(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;
    uint16  i2c_stat = 0;

    for (i = 0; i < maxAttempts; i++)
    {
        if (I2C4STATbits.TRSTAT == 0 && (I2C4CON & 0x001F) == 0)
        {
            i2c_stat = 1;
            break;
        }
    }

    return ( i2c_stat != 0 );
}

/**
 * Generate I2C ACK# state...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  Ack_I2C4(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;
    uint16  i2c_stat = 0;

    I2C4CONbits.ACKDT = 0;
    I2C4CONbits.ACKEN = 1;

    for (i = 0; i < maxAttempts; i++)
    {
        if (!I2C4CONbits.ACKEN)
        {
            i2c_stat = 1;
            break;
        }
    }

    if (i2c_stat == 0) I2C4CONbits.ACKEN = 0;  // timeout
    return ( i2c_stat != 0 );
}

/**
 * Function generates NACK state...
 * Returns TRUE if success; FALSE if timeout occurred.
 */
char  NotAck_I2C4(void)
{
    uint16  i;
    uint16  maxAttempts = 2000;   // timeout count
    uint16  i2c_stat = 0;

    I2C4CONbits.ACKDT = 1;
    I2C4CONbits.ACKEN = 1;

    for (i = 0; i < maxAttempts; i++)
    {
        if (!I2C4CONbits.ACKEN)
        {
            i2c_stat = 1;
            break;
        }
    }

    if (i2c_stat == 0) I2C4CONbits.ACKEN = 0;  // timeout
    I2C4CONbits.ACKDT = 0;
    return ( i2c_stat != 0 );
}


int  I2C4MasterStart(uint8 addr)
{
    if ( !Idle_I2C4() )  goto error_exit;
    if ( !Start_I2C4() )  goto error_exit;
    if ( !Write_I2C4(addr) )  goto error_exit;
    if ( !Idle_I2C4() )  goto error_exit;        // wait for TRSTAT == 0
    if ( !WaitAck_I2C4() )  goto error_exit;     
    return 1;

error_exit:
    return 0;
}


int  I2C4MasterSend(uint8 byte)
{
    if ( !Idle_I2C4() )  goto error_exit;
    if ( !Write_I2C4(byte) )  goto error_exit;
    if ( !Idle_I2C4() )  goto error_exit;
    if ( !WaitAck_I2C4() )  goto error_exit;     
    return 1;

error_exit:
    return 0;
}


int  I2C4MasterSendAckStop(uint8 byte)
{
    if ( !Idle_I2C4() )  goto error_exit;
    if ( !Write_I2C4(byte) )  goto error_exit;
    if ( !Idle_I2C4() )  goto error_exit;
    if ( !WaitAck_I2C4() )  goto error_exit;
    if ( !Stop_I2C4() )  goto error_exit;
    return 1;

error_exit:
    return 0;
}

int  I2C4MasterReadIdleStop(uint8 addr, uint8* buffer, uint8 length)
{
    if ( !Restart_I2C4() )  goto error_exit;
    if ( !Write_I2C4(addr | 0x01) )  goto error_exit;
    if ( !Idle_I2C4() )  goto error_exit;
    if ( !WaitAck_I2C4() )  goto error_exit;     
    if ( gets_I2C4(buffer, length) != length )  goto error_exit;
    if ( !Idle_I2C4() )  goto error_exit;
    if ( !Stop_I2C4() )  goto error_exit;
    return 1;

error_exit:
    return 0;
}

int  I2C4MasterReadNackStop(uint8 addr, uint8* buffer, uint8 length)
{
    if ( !Restart_I2C4() )  goto error_exit;
    if ( !Write_I2C4(addr | 0x01) )  goto error_exit;
    if ( !Idle_I2C4() )  goto error_exit;
    if ( !WaitAck_I2C4() )  goto error_exit;     
    if ( gets_I2C4(buffer, length) != length )  goto error_exit;
    if ( !NotAck_I2C4() )  goto error_exit;
    if ( !Stop_I2C4() )  goto error_exit;
    return 1;

error_exit:
    return 0;
}


/**
 * Function provided for systems using I2C bus multiplexer (PCA9540B).
 * The given downstream channel (0 or 1) is selected.
 */
void  I2C4BusMuxChannelSelect(uint8 channel)
{
    I2C4MasterStart(0xE0);
    I2C4MasterSendAckStop(4 + channel);
}

#endif // USE_I2C_CHANNEL_4
