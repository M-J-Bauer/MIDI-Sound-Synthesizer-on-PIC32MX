/* ================================================================================================
 *
 * FileName:    MIDI_comms_lib.c
 * 
 * Note:  UART1 Receive and Transmit modes are defined in UART_drv.h
 * 
 * It is highly recommended to enable interrupt-driven input and queued output to
 * prevent blocking of time-critical application functions.
 * 
 * ================================================================================================
 */
#include "MIDI_comms_lib.h"


/*
 * Function:     Transmit MIDI Note-On/Velocity message.
 *
 * Entry args:   chan = MIDI channel number (1..16)
 *
 *               noteNum = MIDI standard note number. (Note #60 = C4 = middle-C.)
 *
 *               velocity = initial output level (gain) of output envelope shaper,
 *                          typically, but depends on external MIDI device operation.
 *                          A value of zero should terminate the note.
 *
 * Output amplitude may be subsequently modified by a MIDI "after-touch" command,
 * or a Control Change message (e.g. breath pressure, expression, channel volume, etc),
 * if the connected MIDI device supports dynamic amplitude control.
 */
void  MIDI_SendNoteOn(uint8 chan, uint8 noteNum, uint8 velocity)
{
    uint8   statusByte = 0x90 | ((chan - 1) & 0xF);

    UART1_putch(statusByte);
    UART1_putch(noteNum & 0x7F);
    UART1_putch(velocity & 0x7F);
}


/*
 * Function:     Transmit MIDI Note-Off message.
 *
 * Entry args:   chan = MIDI channel number (1..16)
 *               noteNum = MIDI note number.
 *
 * Attn:         Note-Off Velocity is set to zero (0). Most MIDI devices ignore it.
 *
 */
void  MIDI_SendNoteOff(uint8 chan, uint8 noteNum)
{
    uint8   statusByte = 0x80 | ((chan - 1) & 0xF);

    UART1_putch(statusByte);
    UART1_putch(noteNum & 0x7F);
    UART1_putch(0);
}


/*
 * Function:     Transmit MIDI Channel After-Touch (Pressure) message.
 *
 * Entry args:   chan = MIDI channel number (1..16)
 *               level = Aftertouch (pressure) level.
 */
void  MIDI_SendAfterTouch(uint8 chan, uint8 level)
{
    uint8   statusByte = 0xD0 | ((chan - 1) & 0xF);

    UART1_putch(statusByte);
    UART1_putch(level & 0x7F);
}


/*
 * Function:     Transmit MIDI Pitch Bend message.
 *
 * Entry args:   chan = MIDI channel number (1..16)
 *               value = Pitch deviation value (14 bits). Unit depends on MIDI device.
 */
void  MIDI_SendPitchBend(uint8 chan, uint16 value)
{
    uint8   statusByte = 0xE0 | ((chan - 1) & 0xF);

    UART1_putch(statusByte);
    UART1_putch(value & 0x7F);           // 7 LS bits
    UART1_putch((value >> 7) & 0x7F);    // 7 MS bits
}


/*
 * Function:     Transmit MIDI Control Change message.
 *
 * Entry args:   chan = MIDI channel number (1..16)
 *               ctrlNum = Control Number (0..119) -- not range checked.
 *               value = Controller data value (MSB or LSB).
 *
 * Examples:     ctrlNum = 0x01:0x21  Modulation lever, wheel, etc
 *               ctrlNum = 0x02:0x22  Breath controller
 *               ctrlNum = 0x07:0x27  Channel volume
 *               ctrlNum = 0x0B:0x2B  Expression controller
 *               ctrlNum = 0x0C:0x2C  Effects controller #1
 *               ctrlNum = 0x0D:0x2D  Effects controller #2
 *
 * The sound parameter affected by a Control Change message is dependent on the MIDI
 * sound module design and the patch (program) selected. It may be that some or all
 * controllers are unsupported, hence will have no effect on the sound generated.
 *
 * If ctrlNum < 0x20, value is controller MSB; else value is controller LSB.
 * If ctrlNum >= 0x40, controller data is single byte only.
 */
void  MIDI_SendControlChange(uint8 chan, uint8 ctrlNum, uint8 value)
{
    uint8   statusByte = 0xB0 | ((chan - 1) & 0xF);

    UART1_putch(statusByte);
    UART1_putch(ctrlNum & 0x7F);
    UART1_putch(value & 0x7F);
}


/*
 * Function:     Transmit MIDI Program Change message.
 *
 * Entry args:   chan = MIDI channel number (1..16)
 *               progNum = Program (instrument/voice) number. Depends on MIDI device.
 */
void  MIDI_SendProgramChange(uint8 chan, uint8 progNum)
{
    uint8   statusByte = 0xC0 | ((chan - 1) & 0xF);

    UART1_putch(statusByte);
    UART1_putch(progNum & 0x7F);
}


/*
 * Function:     Transmit MIDI Channel Omni/Poly/Mono (mode change) messages
 *               to set receiver mode. In Mono mode, only one channel is selected (M = 1).
 *
 * Entry args:   chan = MIDI channel number (1..16)
 *               mode = Omni/Poly/Mono  (mode = 1, 2, 3 or 4)
 *
 * Note:         This function also causes 'All Notes Off' to happen.
 */
void  MIDI_SendReceiverMode(uint8 chan, uint8 mode)
{
    uint8   statusByte = 0xB0 | ((chan - 1) & 0xF);

    UART1_putch(statusByte);

    if (mode == OMNI_ON_POLY || mode == OMNI_ON_MONO)
    {
        UART1_putch(125);   // Omni On
        UART1_putch(0);
    }
    else  // Omni Off
    {
        UART1_putch(124);   // Omni Off
        UART1_putch(0);
    }

    UART1_putch(statusByte);

    if (mode == OMNI_ON_POLY || mode == OMNI_OFF_POLY)
    {
        UART1_putch(127);   // Poly mode
        UART1_putch(0);
    }
    else
    {
        UART1_putch(126);   // Mono mode (with M = 1)
        UART1_putch(1);
    }
}


/*
 * Function:     Transmit MIDI Channel 'All Sound Off' (mode change) message.
 *               Status Byte for Mode Change is the same as Control Change (0xBn).
 *
 * Entry args:   chan = MIDI channel number (1..16) = n + 1
 */
void  MIDI_SendAllSoundOff(uint8 chan)
{
    uint8   statusByte = 0xB0 | ((chan - 1) & 0xF);

    UART1_putch(statusByte);
    UART1_putch(120);
    UART1_putch(0);
}


/*
 * Function:     Transmit MIDI Channel 'Reset All Controllers' (mode change) message.
 *
 * Entry args:   chan = MIDI channel number (1..16)
 */
void  MIDI_SendResetAllControllers(uint8 chan)
{
    uint8   statusByte = 0xB0 | ((chan - 1) & 0xF);

    UART1_putch(statusByte);
    UART1_putch(121);
    UART1_putch(0);
}


/*
 * Function:     Find length of a given MIDI command/status message.
 *
 * Entry args:   MIDI command/status byte
 * 
 * Return:       (int) message length (bytes), usually 2 or 3, but...
 *               if status byte is SYS_EXCLUSIVE_CMD, return maximum length, or...
 *               if status byte is not supported (unrecognised), return 0.
 * 
 * todo:         support All Sound off, System reset, etc
 */
int  MIDI_GetMessageLength(uint8 statusByte)
{
    uint8  command = statusByte & 0xF0;
    uint8  length = 0;
    
    if (command == PROGRAM_CHANGE_CMD)  
    {
        length = 2;
    }
    else if (command == NOTE_ON_CMD || command == NOTE_OFF_CMD
    ||  command == CONTROL_CHANGE_CMD || command == PITCH_BEND_CMD)  
    {
        length = 3;
    }
    else if (statusByte == SYS_EXCLUSIVE_MSG)
    {
        length = MIDI_MSG_MAX_LENGTH;
    }
    else  length = 0;  // unsupported message type
    
    return  length;
}
