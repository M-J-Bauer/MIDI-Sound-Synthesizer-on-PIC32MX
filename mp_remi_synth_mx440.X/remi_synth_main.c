/* ================================================================================================
 *
 * FileName:    remi_synth_main.c
 * `````````
 * Overview:    Main module for REMI Sound Synthesizer application.
 * `````````
 * Note:        Source files prefixed "remi_synth_" are common to all REMI synth
 * `````        build variants.  Hardware dependencies are confined to files...
 *              HardwareProfile.h, pic32_low_level.h, pic32_low_level.c, kernel.c.
 * 
 * Processor:   Synth mk2: PIC32MX340F512H;  Synth mk3: PIC32MX440F256H
 * ``````````
 * Compiler:    Microchip MPLAB XC32 (free version) under MPLAB'X IDE.
 * `````````    
 * Author:      M.J.Bauer, Copyright 2016-2022, All rights reserved
 * ```````
 * Reference:   www.mjbauer.biz/Build_the_REMI_by_MJB.htm
 * ``````````  
 * ================================================================================================
 */
#include "remi_synth_main.h"

PRIVATE  void   ProcessMidiMessage(uint8 *midiMessage, short msgLength); 
PRIVATE  void   ProcessControlChange(uint8 *midiMessage);
PRIVATE  void   ProcessMidiSystemExclusive(uint8 *midiMessage, short msgLength);
PRIVATE  void   MidiInputMonitor(uint8 *midiMessage, short msgLength);

// -------------  Global data  --------------------------------------------------------
//
uint8   g_FW_version[4];           // firmware version # (major, minor, build, 0)
uint8   g_SelfTestFault[16];       // Self-test fault codes (0 => no fault)
uint32  g_TaskRunningCount;        // Task execution counter (debug usage only)
uint8   g_HandsetInfo[16];         // REMI handset info rec'd from Sys.Ex. msg

uint8   g_MidiInputBuffer[MIDI_MON_BUFFER_SIZE];  // MIDI monitor buffer (circ. FIFO)
BOOL    g_MidiInputMonitorActive;  // Switch controlled by "mimon" commamnd

short   g_MidiInWriteIndex;        // MIDI IN monitor buffer write index
int     g_MidiInputByteCount;      // MIDI IN monitor received byte count
int     g_PressureMsgCount;        // Number of Pressure CC msg's Rx'd
int     g_ModulationMsgCount;      // Number of Modulation CC msg's Rx'd
int     g_SysExclusivMsgCount;     // Number of System Exclusive msg's Rx'd

// --------------  Private data  ----------------------
//
static  bool    m_NotePlaying;           // Flag: Note in progress (gated))
static  bool    m_LCD_ModuleDetected;    // Flag: LCD module detected
static  bool    m_RemiHandsetConnected;  // Flag: REMI Handset detected on MIDI-IN port
static  short   m_HandsetTimeout_ms;     // Timer: Handset connection lost (at 1000 ms)

//=================================================================================================

void  Init_Application(void)
{
    int   i, preset;

    for (i = 1;  i < NUMBER_OF_SELFTEST_ITEMS;  i++)
        g_SelfTestFault[i] = 0;  // clear fault codes (except item 0)

    g_FW_version[0] = BUILD_VER_MAJOR;
    g_FW_version[1] = BUILD_VER_MINOR;
    g_FW_version[2] = BUILD_VER_DEBUG;

    // Check that the MCU device ID matches the firmware build...
    if (MCU_ID_CHECK() == FALSE) g_SelfTestFault[TEST_DEVICE_ID] = 1;

#ifdef SYNTH_MK2_MX340_LITE  // use timy I2C OLED display
    {
        m_LCD_ModuleDetected = FALSE;
        SH1106_Init();
        SH1106_SetContrast(50);
        SH1106_Wake();
        Disp_ClearScreen();
    }
#else  // SYNTH_MK2_MX340_STD or SYNTH_MK3_MX440_MAM
    if (LCD_Init())  // if LCD module detected...
    {
        m_LCD_ModuleDetected = TRUE;
        LCD_ClearScreen();
        LCD_BACKLIGHT_SET_HIGH();
    }
#endif    
    g_MidiInputMonitorActive = FALSE;
    g_MidiInWriteIndex = 0;
    g_MidiInputByteCount = 0;
    g_HandsetInfo[0] = 0;        // Info not yet received
    
    if (g_SelfTestFault[TEST_DEVICE_ID])
        putstr("! PIC32 device type is incompatible with firmware build.\n");

    if (!m_LCD_ModuleDetected)
        putstr("! LCD module not detected - GUI and Control Panel disabled.\n");
    
    if (!POT_MODULE_CONNECTED)
        putstr("! Control Panel (detachable pot module) not detected.\n");

    if (CheckConfigData() == FALSE)    // Read Config data from EEPROM
    {
        g_SelfTestFault[TEST_EEPROM] = 1;
        putstr("! Error reading EEPROM Config data - Loading defaults.\n");
        DefaultConfigData();
    }

    if (CheckPresetData() == FALSE)    // Read Preset data from EEPROM
    {
        g_SelfTestFault[TEST_EEPROM] = 1;
        putstr("! Error reading EEPROM Preset data - Loading defaults.\n");
        DefaultPresetData();
    }     
    
    UART1_init(g_Config.MidiInBaudrate);    
    PWM_audioDAC_init();
    
    InstrumentPresetSelect(g_Config.PresetLastSelected);  // Activate last Preset
    putstr("Active Preset: ");
    preset = g_Config.PresetLastSelected;
    if (preset == 0)  preset = 8;  // for display only
    putDecimal(preset, 1);
    putNewLine();
    
    g_AppTitleCLI = "Bauer {REMI} Sound Synth ...\n";  // Start-up msg
    Cmnd_ver(1, NULL);   
}


int  main(void)
{
    InitializeMCUclock();
    Init_MCU_IO_ports();   // initializes I2C(1) and ADC also

    UART2_init(57600);     // for console CLI terminal
    putstr("\n");
    putstr("* MCU reset/startup \n");
    putstr("* Running self-test routine \n");

    Init_Application();
    
    while ( TRUE )   // main process loop
    {
        BackgroundTaskExec();  // run MIDI and Synth processes
        ////
        GUI_NavigationExec();
        ////
        ConsoleCLI_Service();
    }
}


/*
 * Background task executive...  
 * Runs periodic tasks scheduled by the RTI timer ISR.
 *
 * This routine is called frequently from the main loop and from inside wait loops;
 * e.g. 1. while waiting for a timer to expire (see WaitMilliseconds())
 *      2. while CLI "watch" function is executing (see Cmnd_watch() in "console_cli.c").
 *
 * Some (asynchronous) background task functions are called as frequently as possible;
 * e.g. MidiInputService(), ReadAnalogInputs().  
 */
void  BackgroundTaskExec()
{
    MidiInputService();
    ReadAnalogInputs();
       
    if (g_Config.MidiOutEnabled)  MidiOutputQueueHandler();
        
    if (isTaskPending_1ms())  // Do 1ms periodic task(s)
    {
        SynthProcess();
        g_TaskRunningCount++;
    }

    if (isTaskPending_50ms())  // Do 50ms periodic tasks
    {
        if (m_HandsetTimeout_ms >= HANDSET_CONNECTION_TIMEOUT)   // 2 sec timeout
        {
            m_RemiHandsetConnected = FALSE;  // connection lost
            g_HandsetInfo[0] = 0;            // Info now invalid
        }
        else if (!m_NotePlaying) m_HandsetTimeout_ms += 50;  // Pause timer if note playing
    }
}


//=================================================================================================
/*
 * Function:     Select an Instrument Preset for the REMI synth and send MIDI program
 *               number to external MIDI device (via MIDI OUT).
 *               The Preset defines the REMI synth Patch number and MIDI control modes.
 *               Preset parameters are stored in the I2C EEPROM.
 *
 * Entry args:   preset = PRESET number (0..7 or 8 == 0)
 *               NB: The user interface displays preset 0 as preset 8.
 */
void  InstrumentPresetSelect(uint8 preset)
{
    uint8   channel = g_Config.MidiOutChannel;
    uint8   program;  // MIDI OUT program number

    if (preset > 7) preset = 0;   // wrap 8 -> 0
    g_Config.PresetLastSelected = preset;  
    StoreConfigData();   // Save this Preset for next power-on/reset

    // Load and activate the REMI synth patch assigned to this Preset:
    SynthPatchSelect(g_Preset.Descr[preset].PatchNumber);
    
    // If this Preset's MIDI program # is non-zero, send it to the MIDI OUT port:
    if (g_Config.MidiOutEnabled)
    {
        program = g_Preset.Descr[preset].MidiProgram;
        if (program != 0) MIDI_SendProgramChange(channel, program);
    }
}


/*^
 * Function:  MidiInputService()
 *
 * MIDI IN service routine, executed frequently from within main loop.
 * This routine monitors the MIDI INPUT stream and whenever a complete message is 
 * received, it is processed.
 */
void  MidiInputService()
{
    static  uint8  midiMessage[MIDI_MSG_MAX_LENGTH];
    static  short  msgBytesExpected;
    static  short  msgByteCount;
    static  short  msgIndex;
    static  uint8  msgStatus;     // last command/status byte rx'd
    static  bool   msgComplete;   // flag: got msg status & data set
    
    uint8   msgByte;
    uint8   msgChannel;  // 1..16 !
    BOOL    gotSysExMsg = FALSE;
    
    if (UART1_RxDataAvail())    // unread byte(s) available in Rx buffer
    {
        msgByte = UART1_getch();
        
        // If MIDI IN monitor (diagnostic) is active, write msgByte in circular buffer.
        // Ignore real-time system messages (0xF8..0xFF)
        if (g_MidiInputMonitorActive && msgByte <= SYSTEM_MSG_EOX)  
        {
            g_MidiInputBuffer[g_MidiInWriteIndex++] = msgByte;
            if (g_MidiInWriteIndex >= MIDI_MON_BUFFER_SIZE) 
                g_MidiInWriteIndex = 0;  // wrap
            g_MidiInputByteCount++;  
        }
        
        if (msgByte & 0x80)  // command/status byte received (bit7 High)
        {
            if (msgByte == SYSTEM_MSG_EOX)  
            {
                msgComplete = TRUE;
                gotSysExMsg = TRUE;
                midiMessage[msgIndex++] = SYSTEM_MSG_EOX;
                msgByteCount++;
            }
            else if (msgByte <= SYS_EXCLUSIVE_MSG)  // Regular command (not RTC, etc)
            {
                msgStatus = msgByte;
                msgComplete = FALSE;  // expecting data byte(s))
                midiMessage[0] = msgStatus;
                msgIndex = 1;
                msgByteCount = 1;  // have cmd already
                msgBytesExpected = MIDI_GetMessageLength(msgStatus);
            }
            // otherwise ignore command/status byte
        }
        else    // data byte received (bit7 LOW)
        {
            if (msgComplete && msgStatus != SYS_EXCLUSIVE_MSG)  
            {
                // already processed complete message -- data repeating without command
                if (msgByteCount == 0)  // start of new data set 
                {
                    msgIndex = 1;
                    msgByteCount = 1;
                    msgBytesExpected = MIDI_GetMessageLength(msgStatus);
                }
            }
            if (msgIndex < MIDI_MSG_MAX_LENGTH)
            {
                midiMessage[msgIndex++] = msgByte;
                msgByteCount++;
            }
        }
        
        if ((msgByteCount != 0 && msgByteCount == msgBytesExpected) || gotSysExMsg)    
        {
            msgComplete = TRUE;
            msgChannel = (midiMessage[0] & 0x0F) + 1;  // 1..16
            
            if (msgChannel == g_Config.MidiInChannel 
            ||  g_Config.MidiInMode == OMNI_ON_MONO
            ||  msgStatus == SYS_EXCLUSIVE_MSG)
            {
                ProcessMidiMessage(midiMessage, msgByteCount); 
                msgBytesExpected = 0;   // maybe redundant ?
                msgByteCount = 0;
                msgIndex = 0;
            }
        }
    }
}


/*^
 * Function:  ProcessMidiMessage()
 *
 * This function processes a complete MIDI message when received.
 * The message is analysed to determine what actions are required.
 * Both the REMI built-in synth and external 'MIDI OUT' device(s) are controlled.
 */
PRIVATE  void  ProcessMidiMessage(uint8 *midiMessage, short msgLength)
{
    uint8  statusByte = midiMessage[0] & 0xF0;
    uint8  channel = g_Config.MidiOutChannel;
    
    switch (statusByte)
    {
        case NOTE_OFF_CMD:
        {
            uint8  noteNumber = midiMessage[1];
            SynthNoteOff(noteNumber);
            if (g_Config.MidiOutEnabled) MIDI_SendNoteOff(channel, noteNumber);
            m_NotePlaying = FALSE;
            break;
        }
        case NOTE_ON_CMD:
        {
            uint8  noteNumber = midiMessage[1];
            uint8  velocity = midiMessage[2];
            if (velocity == 0)  
            {
                SynthNoteOff(noteNumber);
                m_NotePlaying = FALSE;
            }
            else 
            {
                SynthNoteOn(noteNumber, velocity);
                m_NotePlaying = TRUE;
            }

            if (g_Config.MidiOutEnabled)
                MIDI_SendNoteOn(channel, noteNumber, velocity);  // MIDI msg pass-through
            break;
        }
        case CONTROL_CHANGE_CMD:  // same for MODE CHANGE CMD
        {
            ProcessControlChange(midiMessage);
            break;
        }
        case PROGRAM_CHANGE_CMD:
        {
            uint8  program = midiMessage[1];  // MIDI-IN pgrm number

            if (!m_RemiHandsetConnected && g_Config.MidiOutEnabled) 
                MIDI_SendProgramChange(channel, program);   // MIDI msg pass-through
            break;
        }
        case PITCH_BEND_CMD:
        {
            uint8  leverPosn_Lo = midiMessage[1];  // PB lever position, 7 LS bits
            uint8  leverPosn_Hi = midiMessage[2];  // PB lever position, 7 MS bits
            int16  bipolarPosn;
            
            bipolarPosn = ((int16)(leverPosn_Hi << 7) | leverPosn_Lo) - 0x2000;
            SynthPitchBend(bipolarPosn);
            break;
        }
        case SYS_EXCLUSIVE_MSG: 
        {
            ProcessMidiSystemExclusive(midiMessage, msgLength);
            break;
        }
        default:  break;
    }  // end switch
    
    MidiInputMonitor(midiMessage, msgLength);
}


PRIVATE  void  ProcessControlChange(uint8 *midiMessage)
{
    static  uint8  modulationHi = 0;  // High byte of CC data (7 bits)
    static  uint8  modulationLo = 0;  // Low byte  ..   ..
    static  uint8  pressureHi = 0;    // High byte of CC data (7 bits)
    static  uint8  pressureLo = 0;    // Low byte  ..   ..
    int    data14;
    uint8  midiCCnum;
    uint8  channel = g_Config.MidiOutChannel;
    
    if (midiMessage[1] == g_Config.MidiInExpressionCCnum)  // Pressure Hi byte
    {
        pressureHi = midiMessage[2];
        pressureLo = 0;  // compliant with MIDI spec v4.2 (1995)
        data14 = (((int) pressureHi) << 7);
        SynthExpression(data14);
        midiCCnum = g_Config.MidiOutExpressionCCnum;
        if (g_Config.MidiOutEnabled) 
            MIDI_SendControlChange(channel, midiCCnum, pressureHi);
    }
    else if (midiMessage[1] == (g_Config.MidiInExpressionCCnum + 0x20))  // Pressure Lo byte
    {
        pressureLo = midiMessage[2];
        data14 = (((int) pressureHi) << 7) + pressureLo;
        SynthExpression(data14);
        midiCCnum = g_Config.MidiOutExpressionCCnum + 0x20;
        if (g_Config.MidiOutEnabled)
            MIDI_SendControlChange(channel, midiCCnum, pressureLo);
    }
    else if (midiMessage[1] == 0x01)  // CC-01 = modulation Hi byte
    {
        modulationHi = midiMessage[2];
        modulationLo = 0;  // compliant with MIDI spec v4.2 (1995)
        data14 = (((int) modulationHi) << 7);
        SynthModulation(data14);
        if (g_Config.MidiOutEnabled)
            MIDI_SendControlChange(channel, 0x01, modulationHi);
    }
    else if (midiMessage[1] == 0x21)  // CC-33 = modulation Lo byte
    {
        modulationLo = midiMessage[2];
        data14 = (((int) modulationHi) << 7) + modulationLo;
        SynthModulation(data14);
        if (g_Config.MidiOutEnabled)
            MIDI_SendControlChange(channel, 0x21, modulationLo);
    }
    else if (midiMessage[1] == 120 || midiMessage[1] == 121)  // 'Mode Change' command
    {
        SynthPrepare();  // All Sound Off - Kill note playing and reset synth engine
    }
}


/*^
 * Function:  ProcessMidiSystemExclusive()
 *
 * This function processes a REMI System Exclusive message when received.
 * 
 * The "manufacturer ID" (2nd byte of msg) is first validated to ensure the message
 * can be correctly interpreted, i.e. it's a 'REMI exclusive' message which contains
 * information about the handset connected to the MIDI IN serial port.
 * Byte 3 of the message is a code to identify the type of message content.
 * 
 *  System Exclusive 'PRESET' message format:
 *  `````````````````````````````````````````
 *  RemiPresetMsg[0]  = SYS_EXCLUSIVE_MSG;  // status byte
 *  RemiPresetMsg[1]  = SYS_EXCL_REMI_ID;   // manuf/product ID
 *  RemiPresetMsg[2]  = REMI_PRESET_MSG;    // Msg type
 *  RemiPresetMsg[3]  = preset;             // selected preset # (0..7)
 *  RemiPresetMsg[4]  = g_FW_version[0];
 *  RemiPresetMsg[5]  = g_FW_version[1];
 *  RemiPresetMsg[6]  = g_FW_version[2];
 *  RemiPresetMsg[7]  = g_SelfTestErrors;
 *  RemiPresetMsg[8]  = g_Config.FingeringScheme;
 *  RemiPresetMsg[9]  = g_Config.LegatoModeEnabled;
 *  RemiPresetMsg[10] = g_Config.VelocitySenseEnabled;
 *  RemiPresetMsg[11] = SYSTEM_MSG_EOX;     // end-of-msg code
 */
PRIVATE  void  ProcessMidiSystemExclusive(uint8 *midiMessage, short msgLength)
{
    uint8  preset;  int i;
                
    if (midiMessage[1] == SYS_EXCL_REMI_ID)  // "Manufacturer ID" match
    {
        m_RemiHandsetConnected = TRUE;  // REMI wired handset detected...
        m_HandsetTimeout_ms = 0;        // Reset the "connection lost" timer
        
        if (midiMessage[2] == REMI_PRESET_MSG)
        {
            preset = midiMessage[3];
            InstrumentPresetSelect(preset);
            
            // Extract sundry data from message (for UI info)
            for (i = 0 ; i < 16 ; i++)
            {
                g_HandsetInfo[i] = midiMessage[i];
            }
        }
    }
}


/*^
 * Function:  MidiInputMonitor()
 *
 * Diagnostic function which, when enabled, monitors messages received from the MIDI input
 * port and generates statistics about the messages, e.g. the frequency of each type of
 * message. Note-On, Note-Off, Program Change and Sys.Excl.Preset messages are notified in
 * real time via the console port.
 * 
 * Use the CLI command "mimon" to activate or de-activate the MIDI IN monitor function.
 */
PRIVATE  void   MidiInputMonitor(uint8 *midiMessage, short msgLength)
{
    uint8  statusByte = midiMessage[0] & 0xF0;
    uint8  preset;
    
    if (!g_MidiInputMonitorActive) return;  // Monitor is disabled... bail
    
    switch (statusByte)
    {
        case NOTE_OFF_CMD:
        {
            uint8  noteNumber = midiMessage[1];

            putstr("Note-Off = ");  putDecimal(noteNumber, 3);  putstr("\n");
            break;
        }
        case NOTE_ON_CMD:
        {
            uint8  noteNumber = midiMessage[1];

            putstr("Note-On  = ");  putDecimal(noteNumber, 3);  putstr("\n");
            break;
        }
        case CONTROL_CHANGE_CMD:
        {
            if (midiMessage[1] == g_Config.MidiInExpressionCCnum) 
                g_PressureMsgCount++;
            if (midiMessage[1] == 0x01) g_ModulationMsgCount++;  // CC-01 modulation
            break;
        }
        case PROGRAM_CHANGE_CMD:
        {
            putstr("Program  = ");  putDecimal(midiMessage[1], 3);  putstr("\n");
            break;
        }
        case PITCH_BEND_CMD: 
        {
            // todo
            break;
        }
        case SYS_EXCLUSIVE_MSG: 
        {
            if (midiMessage[2] == REMI_PRESET_MSG)  
            {
                preset = midiMessage[3];
                if (preset == 0)  preset = 8;  // for display
                putstr("PRESET # = ");  putDecimal(preset, 3);  
                putstr("\n");
            }
            g_SysExclusivMsgCount++;
            break;
        }
        default:  break;
    }  // end switch
}


/*
 * Function:  isLCDModulePresent()
 *
 * Overview:  Return TRUE if LCD Module is present, as detected at startup.
 */
bool  isLCDModulePresent()
{
    return (m_LCD_ModuleDetected);
}


/*
 * Function:  isHandsetConnected()
 *
 * Overview:  Return TRUE if a REMI (mk2) handset is detected on the MIDI-IN port,
 *            else return False (0).
 */
bool  isHandsetConnected()
{
    return (m_RemiHandsetConnected);
}


/*=============================  LICENSE AGREEMENT  ===================================*\
 *
 *  THIS SOURCE CODE MAY BE USED FREELY FOR PERSONAL NON-COMMERCIAL APPLICATIONS.
 *  HOWEVER, THE CODE OR PART THEREOF MAY NOT BE REDISTRIBUTED OR SOLD IN ANY FORM.
 *  USE OF THIS SOURCE CODE OR OBJECT CODE GENERATED FROM IT FOR COMMERCIAL PURPOSES
 *  WITHOUT THE EXPRESS WRITTEN PERMISSION OF THE AUTHOR IS PROHIBITED.
 *
\*=====================================================================================*/
