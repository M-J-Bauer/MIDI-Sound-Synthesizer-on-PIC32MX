/* ================================================================================================
 *
 * FileName:    main_remi_synth2.c
 * `````````
 * Overview:    Main module for REMI mk2 Sound Synthesizer application.
 * `````````
 * Processor:   PIC32MX340F512H  (See "HardwareProfile.h")
 * ``````````
 * Hardware:    Olimex PIC32-MX340 Prototyping Board (plus synth add-ons), or
 * `````````    Custom PIC32-MX340 Board (REMI mk2 Synth Module, Chua dongle, etc)
 *
 * Compiler:    Microchip MPLAB XC32 (free version) under MPLAB'X IDE.
 * `````````    The project contains no Microchip "PLIB" dependencies.
 *
 * Author:      M.J.Bauer, Copyright 2016-2022, All rights reserved
 * ```````
 * Reference:   www.mjbauer.biz/Build_the_REMI_by_MJB.htm
 * ``````````  
 * ================================================================================================
 */
#include "main_remi_synth2.h"

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
static  bool    m_LCD_ModuleDetected;    // Flag: LCD module detected
static  bool    m_REMI2handsetConnected;  // Flag: REMI Handset detected on MIDI-IN port
static  short   m_HandsetTimeout_ms;     // Timer: Handset connection lost (at 1000 ms)

//=================================================================================================

void  Init_Application(void)
{
    int   i;

    for (i = 1;  i < NUMBER_OF_SELFTEST_ITEMS;  i++)
        g_SelfTestFault[i] = 0;  // clear fault codes (except item 0)

    g_FW_version[0] = BUILD_VER_MAJOR;
    g_FW_version[1] = BUILD_VER_MINOR;
    g_FW_version[2] = BUILD_VER_DEBUG;

    // Check that the MCU device ID matches the firmware build...
    if ((GET_DEVICE_ID() & DEVICE_ID_MASK) != DEVICE_ID_PIC32MX340F512H)
        g_SelfTestFault[TEST_DEVICE_ID] = 1;

    if (LCD_Init())  // LCD module detected
    {
        m_LCD_ModuleDetected = 1;
        LCD_ClearScreen();
        LCD_BACKLIGHT_HIGH();
    }
    
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
    int  preset = g_Config.PresetLastSelected;
    if (preset == 0)  preset = 8;  // for display
    putDecimal(preset, 1);
    putNewLine();
    
    g_AppTitleCLI = "Bauer {REMI} Sound Synth mk2 ...\n";  // Start-up msg
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
 * e.g. MidiInputService(), MidiOutputQueueHandler().  
 */
void  BackgroundTaskExec()
{
    MidiInputService(); 
       
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
            m_REMI2handsetConnected = 0;   // connection lost
            g_HandsetInfo[0] = 0;          // Info now invalid
        }
        else  m_HandsetTimeout_ms += 50;
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
    static  short  msgBytesExpected = 0;
    static  short  msgLength = 0;
    static  short  msgIndex = 0;
    
    uint8   msgByte;
    uint8   msgChannel;  // 1..16 !
    BOOL    gotSysExMsg = FALSE;
    
    if (UART1_RxDataAvail())    // unread byte(s) available in Rx buffer
    {
        msgByte = UART1_getch();
        
        if (g_MidiInputMonitorActive)  // diagnostic aid
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
                gotSysExMsg = TRUE;
                midiMessage[msgIndex++] = msgByte;
                msgLength++;
            }
            else
            {
                midiMessage[0] = msgByte;
                msgIndex = 1;
                msgLength = 1;
                msgBytesExpected = MIDI_GetMessageLength(msgByte);
            }
        }
        else    // data byte received (bit7 LOW)
        {
            if (msgIndex < MIDI_MSG_MAX_LENGTH)
            {
                midiMessage[msgIndex++] = msgByte;
                msgLength++;
            }
        }
        
        if (msgLength != 0 && (msgLength == msgBytesExpected) ||  gotSysExMsg )    
        {
            // Have complete message
            msgChannel = (midiMessage[0] & 0x0F) + 1;  // 1..16
            
            if (msgChannel == g_Config.MidiInChannel 
            ||  g_Config.MidiInMode == OMNI_ON_MONO
            ||  midiMessage[0] == SYS_EXCLUSIVE_MSG)
            {
                ProcessMidiMessage(midiMessage, msgLength); 
                msgBytesExpected = 0;
                msgLength = 0;
                msgIndex = 0;
            }
        }
    }
}


/*^
 * Function:  ProcessMidiMessage()
 *
 * This function processes a complete MIDI command/status message when received.
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
            break;
        }
        case NOTE_ON_CMD:
        {
            uint8  noteNumber = midiMessage[1];
            uint8  velocity = midiMessage[2];
            
            if (velocity == 0)  SynthNoteOff(noteNumber);
            else if (noteNumber >= 12 && noteNumber <= 120)
                SynthNoteOn(noteNumber, velocity);

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

            if (!m_REMI2handsetConnected && g_Config.MidiOutEnabled) 
                MIDI_SendProgramChange(channel, program);   // MIDI msg pass-through
            break;
        }
        case PITCH_BEND_CMD:
        {
            uint8  leverPosn_Hi = midiMessage[1];  // PB lever position, 7 MS bits
            uint8  leverPosn_Lo = midiMessage[2];  // PB lever position, 7 MS bits
            int    signed14b = ((int16) leverPosn_Hi << 7) | leverPosn_Lo;
            
            SynthPitchBend(signed14b);
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
    
    if (midiMessage[1] == g_Config.MidiInExpressionCCnum)
    {
        pressureHi = midiMessage[2];
        data14 = (((int) pressureHi) << 7) + pressureLo;
        SynthExpression(data14);
        midiCCnum = g_Config.MidiOutExpressionCCnum;
        if (g_Config.MidiOutEnabled) 
            MIDI_SendControlChange(channel, midiCCnum, pressureHi);
    }
    else if (midiMessage[1] == (g_Config.MidiInExpressionCCnum + 32))
    {
        pressureLo = midiMessage[2];
        data14 = (((int) pressureHi) << 7) + pressureLo;
        SynthExpression(data14);
        midiCCnum = g_Config.MidiOutExpressionCCnum + 32;
        if (g_Config.MidiOutEnabled)
            MIDI_SendControlChange(channel, midiCCnum, pressureLo);
    }
    else if (midiMessage[1] == 0x01)  // CC-01 = modulation Hi byte
    {
        modulationHi = midiMessage[2];
        data14 = (((int) modulationHi) << 7) + modulationLo;
        SynthModulation(data14);
        midiCCnum = 0x01;
        if (g_Config.MidiOutEnabled)
            MIDI_SendControlChange(channel, midiCCnum, modulationHi);
    }
    else if (midiMessage[1] == 0x21)  // CC-33 = modulation Lo byte
    {
        modulationLo = midiMessage[2];
        data14 = (((int) modulationHi) << 7) + modulationLo;
        SynthModulation(data14);
        midiCCnum = 0x21;
        if (g_Config.MidiOutEnabled)
            MIDI_SendControlChange(channel, midiCCnum, modulationLo);
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
        m_REMI2handsetConnected = 1;  // REMI wired handset detected...
        m_HandsetTimeout_ms = 0;     // Reset the "connection lost" timer
        
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
    return (m_REMI2handsetConnected);
}


/*=============================  LICENSE AGREEMENT  ===================================*\
 *
 *  THIS SOURCE CODE MAY BE USED FREELY FOR PERSONAL NON-COMMERCIAL APPLICATIONS.
 *  HOWEVER, THE CODE OR PART THEREOF MAY NOT BE REDISTRIBUTED OR SOLD IN ANY FORM.
 *  USE OF THIS SOURCE CODE OR OBJECT CODE GENERATED FROM IT FOR COMMERCIAL PURPOSES
 *  WITHOUT THE EXPRESS WRITTEN PERMISSION OF THE AUTHOR IS PROHIBITED.
 *
\*=====================================================================================*/
