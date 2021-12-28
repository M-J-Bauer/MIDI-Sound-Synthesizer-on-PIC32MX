/*^************************************************************************************************
 *
 * Module:      remi_synth2_GUI.c
 *
 * Overview:    Graphical User Interface (GUI) of the REMI mk2 Sound Synth.
 *
 * Author:      M.J.Bauer, Copyright 2016++  All rights reserved
 *
 * Reference:   www.mjbauer.biz/Build_the_REMI_by_MJB.htm
 *
 * ================================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "main_remi_synth2.h"
#include "remi_synth2_GUI.h"

PRIVATE  void  DisplayTitleBar(uint16 scnIndex);

PRIVATE  void  ScreenFunc_Startup(bool);
PRIVATE  void  ScreenFunc_SelfTestReport(bool);
PRIVATE  void  ScreenFunc_Home(bool);
PRIVATE  void  ScreenFunc_SetupMidiInParams(bool);
PRIVATE  void  ScreenFunc_SetupMidiOutParams(bool);
PRIVATE  void  ScreenFunc_PresetEditMenu(bool);
PRIVATE  void  ScreenFunc_MiscControlsMenu(bool);

PRIVATE  void  ScreenFunc_SetMidiInMode(bool);
PRIVATE  void  ScreenFunc_SetMidiInChannel(bool);
PRIVATE  void  ScreenFunc_SetMidiInPressureCC(bool);
PRIVATE  void  ScreenFunc_SetMidiInModulationCC(bool);

PRIVATE  void  ScreenFunc_SetMidiOutMode(bool);
PRIVATE  void  ScreenFunc_SetMidiOutChannel(bool);
PRIVATE  void  ScreenFunc_SetMidiOutPressureCC(bool);
PRIVATE  void  ScreenFunc_SetMidiOutModulationCC(bool);

PRIVATE  void  ScreenFunc_EditPresetPatch(bool);
PRIVATE  void  ScreenFunc_EditPresetMidiProgram(bool);
PRIVATE  void  ScreenFunc_EditPresetVibratoMode(bool);
PRIVATE  void  ScreenFunc_EditPresetTranspose(bool);

PRIVATE  void  ScreenFunc_EditReverbAtten(bool);
PRIVATE  void  ScreenFunc_EditReverbMix(bool);

PRIVATE  void  ScreenFunc_SystemInfoPage1(bool);
PRIVATE  void  ScreenFunc_SystemInfoPage2(bool);
PRIVATE  void  ScreenFunc_CustomFuncMenu(bool);
PRIVATE  void  ScreenFunc_DataEntry(bool);
PRIVATE  void  ScreenFunc_DataEntryTest(bool);


// Screen descriptors (below) may be arranged in any arbitrary order in the array
// m_ScreenDesc[], i.e. the table doesn't need to be sorted into screen_ID order.
// Function ScreenIndexFind() is used to find the index of an element within
// the array m_ScreenDesc[], for a specified screen_ID.
//
static  const  GUI_ScreenDescriptor_t  m_ScreenDesc[] =
{
    {
        SCN_STARTUP,                 // screen ID
        ScreenFunc_Startup,          // screen update function
        NULL                         // title bar text (none)
    },
    {
        SCN_SELFTEST_REPORT,         // screen ID
        ScreenFunc_SelfTestReport,   // screen update function
        "SELF-TEST Failed"           // title bar text
    },
    {
        SCN_HOME,
        ScreenFunc_Home,
        NULL                         // title bar text (none)
    },
    {
        SCN_SETUP_MIDI_IN,
        ScreenFunc_SetupMidiInParams,
        "MIDI IN CONFIG."            // title bar text  SCN_MISC_CONTROL_MENU
    },
    {
        SCN_SETUP_MIDI_OUT,
        ScreenFunc_SetupMidiOutParams,
        "MIDI OUT CONFIG."  
    },
    {
        SCN_MISC_CONTROL_MENU,
        ScreenFunc_MiscControlsMenu,
        "MISC. CONTROLS"  
    },
    {
        SCN_PRESET_EDIT_MENU,
        ScreenFunc_PresetEditMenu,
        "PRESET ## PARAMs"
    },
    {
        SCN_SET_MIDI_IN_MODE,
        ScreenFunc_SetMidiInMode,
        "SET MIDI IN MODE"
    },
    {
        SCN_SET_MIDI_IN_CHANNEL,
        ScreenFunc_SetMidiInChannel,
        "SET MIDI IN CHANNEL"
    },
    {
        SCN_SET_MIDI_IN_PRESS_CC,
        ScreenFunc_SetMidiInPressureCC,
        "SET PRESSURE CC #"
    },
    {
        SCN_SET_MIDI_IN_MODLN_CC,
        ScreenFunc_SetMidiInModulationCC,
        "SET MODULATION CC #"
    },
    {
        SCN_SET_MIDI_OUT_MODE,
        ScreenFunc_SetMidiOutMode,
        "SET MIDI OUT MODE"
    },
    {
        SCN_SET_MIDI_OUT_CHANNEL,
        ScreenFunc_SetMidiOutChannel,
        "SET MIDI OUT CHANNEL"
    },
    {
        SCN_SET_MIDI_OUT_PRESS_CC,
        ScreenFunc_SetMidiOutPressureCC,
        "SET PRESSURE CC #"
    },
    {
        SCN_SET_MIDI_OUT_MODLN_CC,
        ScreenFunc_SetMidiOutModulationCC,
        "SET MODULATION CC #"
    },
    {
        SCN_EDIT_PRESET_PATCH,
        ScreenFunc_EditPresetPatch,
        "EDIT PRESET PATCH"
    },
    {
        SCN_EDIT_PRESET_MIDI_PGRM,
        ScreenFunc_EditPresetMidiProgram, 
        "EDIT PRESET MIDI PGRM"
    },
    {
        SCN_EDIT_PRESET_VIBRATO_MODE,
        ScreenFunc_EditPresetVibratoMode,
        "EDIT VIBRATO MODE"
    },
    {
        SCN_EDIT_PRESET_TRANSPOSE,
        ScreenFunc_EditPresetTranspose,
        "EDIT PITCH TRANSPOSE"
    },
    {
        SCN_EDIT_REVERB_ATTEN,
        ScreenFunc_EditReverbAtten,
        "EDIT REVERB ATTEN."
    },
    {
        SCN_EDIT_REVERB_MIX,
        ScreenFunc_EditReverbMix,
        "EDIT REVERB MIX."
    },
    {
        SCN_SYSTEM_INFO_PAGE1,
        ScreenFunc_SystemInfoPage1,
        "System Info   Page 1"
    },
    {
        SCN_SYSTEM_INFO_PAGE2,
        ScreenFunc_SystemInfoPage2,
        "System Info   Page 2"
    },  
    {
        SCN_CUSTOM_FUNC_MENU,
        ScreenFunc_CustomFuncMenu,
        "Custom Functions"
    },  
    {
        SCN_DATA_ENTRY,
        ScreenFunc_DataEntry,
        NULL      // Title is variable, context-dependent
    },
    {
        SCN_DATA_ENTRY_TEST,
        ScreenFunc_DataEntryTest,
        " Data Entry Result"
    },
};

#define NUMBER_OF_SCREENS_DEFINED  ARRAY_SIZE(m_ScreenDesc)


//------------------- Private data ----------------------------------------------------
//
static  bool    m_screenSwitchDone;      // flag: Screen switch occurred
static  uint32  m_lastUpdateTime;        // captured timer value (ms)
static  uint16  m_CurrentScreen;         // ID number of current screen displayed
static  uint16  m_PreviousScreen;        // ID number of previous screen displayed
static  uint16  m_NextScreen;            // ID number of next screen to be displayed
static  bool    m_ScreenSwitchFlag;      // Trigger to change to next screen
static  uint32  m_ElapsedTime_ms;        // Time elapsed since last key hit (ms)
static  int     m_presetLastShown;       // Preset # of last display update
static  int     m_editPreset;            // Preset # for editing param's
static  bool    m_HandsetDetectedLast;   // flag: REMI (MIDI) handset detected

static  int     m_DataEntryValue;        // Number entered via Data Entry screen
static  bool    m_DataEntryAccept;       // User entered a valid number (flag)
static  char   *m_DataEntryTitle;        // String to display in Data Entry Title Bar
static  uint16  m_DataEntryNextScreen;   // ID # of screen to follow data entry
static  uint8   m_DataEntryNumDigits;    // Number of digits to show (1..5)

static  BOOL    m_ButtonHitDetected;     // flag: button hit detected
static  uint8   m_ButtonStates;          // Button states, de-glitched (bits 5:0)
static  short   m_ButtonLastHit;         // ASCII-encoded value of last button hit


/*=================================================================================================
 *
 * Function returns the number of screens defined (with initialized data) in the
 * array of screen descriptors, m_ScreenDesc[].
 */
int  GetNumberOfScreensDefined()
{
    return  ARRAY_SIZE(m_ScreenDesc);
}

/*
 * Function returns the screen ID number of the currently displayed screen.
 */
uint16  GetCurrentScreenID()
{
    return  m_CurrentScreen;
}

/*
 * Function returns the screen ID number of the previously displayed screen.
 */
uint16  GetPreviousScreenID()
{
   return  m_PreviousScreen;
}

/*
 * Function triggers a screen switch to a specified new screen.
 * The real screen switch business is done by the routine GUI_NavigationExec(),
 * when next executed following the call to GoToNextScreen().
 *
 * Entry arg:  nextScreenID = ID number of next screen required.
 */
void  GoToNextScreen(uint16 nextScreenID)
{
    m_NextScreen = nextScreenID;
    m_ScreenSwitchFlag = 1;
}


/*
 * GUI initialization function...
 * Call this before GUI_NavigationExec() to avoid trouble!
 */
void  GUI_NavigationInit()
{
    m_CurrentScreen = SCN_STARTUP;
    m_PreviousScreen = SCN_STARTUP;
    m_DataEntryNextScreen = SCN_HOME;
    m_ScreenSwitchFlag = 1;
    m_screenSwitchDone = 0;
}


/*
 * GUI navigation engine (service routine, or whatever you want to call it).
 * This is the "executive hub" of the Graphical User Interface.
 *
 * The function is called frequently from the main application loop, but...
 * must not be called from BackgroundTaskExec() !!
 */
void  GUI_NavigationExec(void)
{
    short  current, next;  // index values of current and next screens

    if (m_ScreenSwitchFlag)   // Screen switch requested
    {
        m_ScreenSwitchFlag = 0;
        m_ElapsedTime_ms = 0;
        m_lastUpdateTime = milliseconds();
        next = ScreenDescIndexFind(m_NextScreen);

        if (next < NUMBER_OF_SCREENS_DEFINED)  // found next screen ID
        {
            m_PreviousScreen = m_CurrentScreen;     // Make the switch...
            m_CurrentScreen = m_NextScreen;         // next screen => current

            if (m_NextScreen != m_PreviousScreen)
            {
                GUI_EraseScreen();

                if (m_ScreenDesc[next].TitleBarText != NULL)
                    DisplayTitleBar(next);
            }

            (*m_ScreenDesc[next].ScreenFunc)(1);  // Render new screen
            m_screenSwitchDone = TRUE;
        }
    }
    else  // no screen switch -- check update timer
    {
        if (milliseconds() - m_lastUpdateTime >= SCREEN_UPDATE_INTERVAL)
        {
            current = ScreenDescIndexFind(m_CurrentScreen);

            (*m_ScreenDesc[current].ScreenFunc)(0);  // Update current screen

            m_lastUpdateTime = milliseconds();
            m_ElapsedTime_ms += SCREEN_UPDATE_INTERVAL;
        }
    }
}


/*
 * Function returns TRUE if a screen switch has occurred since the previous call.
 */
bool  ScreenSwitchOccurred(void)
{
    bool  result = m_screenSwitchDone;

    m_screenSwitchDone = FALSE;
    return result;
}


/*
 * Function returns the index of a specified screen in the array of Screen Descriptors,
 * (GUI_ScreenDescriptor_t)  m_ScreenDesc[].
 *
 * Entry arg(s):  search_ID = ID number of required screen descriptor
 *
 * Return value:  index of screen descriptor in array m_ScreenDesc[], or...
 *                NUMBER_OF_SCREENS_DEFINED, if search_ID not found.
 */
int  ScreenDescIndexFind(uint16 searchID)
{
    int   index;

    for (index = 0; index < NUMBER_OF_SCREENS_DEFINED; index++)
    {
        if (m_ScreenDesc[index].screen_ID == searchID)  break;
    }

    return index;
}


/*
 * Function renders the Title Bar (background plus text) of a specified screen.
 * The title bar text (if any) is defined in the respective screen descriptor
 * given by the argument scnIndex. The function is called by GUI_NavigationExec();
 * it is not meant to be called directly by application-specific screen functions.
 *
 * The location and size of the Title Bar and the font used for its text string
 * are fixed inside the function.
 *
 * Entry arg:  scnIndex = index (not ID number!) of respective screen descriptor
 *
 */
PRIVATE  void  DisplayTitleBar(uint16 scnIndex)
{
    char  *titleString = m_ScreenDesc[scnIndex].TitleBarText;

    LCD_Mode(SET_PIXELS);
    LCD_PosXY(0, 0);
    LCD_BlockFill(128, 10);

    if (titleString != NULL)
    {
        LCD_Mode(CLEAR_PIXELS);
        LCD_SetFont(MONO_8_NORM);
        LCD_PosXY(2, 1);
        LCD_PutText(titleString);
    }

    LCD_Mode(SET_PIXELS);
}


/*
 * This function displays a single-line menu option, i.e. keytop image plus text string.
 * The keytop image is simply a square with a character drawn inside it in reverse video.
 * The given text string is printed just to the right of the keytop image.
 * The character font(s) used are fixed within the function.
 *
 * Entry args:   x = X-coord of keytop image (2 pixels left of key symbol)
 *               y = Y-coord of keytop symbol, same as text to be printed after
 *               symbol = ASCII code of keytop symbol (5 x 7 mono font)
 *               text = string to print to the right of the keytop image
 */
void  DisplayMenuOption(uint16 x, uint16 y, char symbol, char *text)
{
    uint16  xstring = x + 12;  // x-coord on exit

    LCD_Mode(SET_PIXELS);
    LCD_PosXY(x, y-1);
    LCD_DrawBar(9, 9);

    LCD_SetFont(MONO_8_NORM);
    LCD_Mode(CLEAR_PIXELS);
    LCD_PosXY(x+2, y);
    if (symbol > 0x20) LCD_PutChar(symbol);

    LCD_SetFont(PROP_8_NORM);
    LCD_Mode(SET_PIXELS);
    LCD_PosXY(xstring, y);
    if (text != NULL) LCD_PutText(text);
}


/*-------------------------------------------------------------------------------------------------
 * Functions to support GUI input using 6 push-buttons on the front-panel.
 *-------------------------------------------------------------------------------------------------
 *
 * Function:  ButtonInputService()
 *
 * Overview:  Service Routine for 6-button input.
 *
 * Detail:    Background task called periodically at 5ms intervals from the main loop.
 *            The routine reads the button inputs looking for a change in states.
 *            When a button "hit" is detected, the function sets a flag to register the event.
 *            The state of the flag can be read by a call to function KeyHit().
 *            An ASCII key-code is stored to identify the button last pressed.
 *            The keycode can be read anytime by a call to function GetKey().
 */
void  ButtonInputService()
{
    static  short   taskState = 0;  // startup/reset state
	static  uint16  buttonStatesLastRead = 0;
	static  int     debounceTimer_ms = 0;
	
	uint16  buttonStatesNow = READ_BUTTON_INPUTS() ^ 0x003F;  // 6 LS bits, active HIGH
	
	if (taskState == 0)  // Waiting for all buttons released
	{
		if (buttonStatesNow == ALL_BUTTONS_RELEASED)
		{
			debounceTimer_ms = 0;
			taskState = 3;
		}
	}
	else if (taskState == 1)  // Waiting for any button(s) pressed
	{
		if (buttonStatesNow != ALL_BUTTONS_RELEASED) 
		{
			buttonStatesLastRead = buttonStatesNow;
			debounceTimer_ms = 0;
			taskState = 2;
		}
	}
	else if (taskState == 2)  // De-bounce delay after hit (30ms)
	{
		if (buttonStatesNow != buttonStatesLastRead)
			taskState = 1;    // glitch -- retry
			
		if (debounceTimer_ms >= 30)
		{
			m_ButtonHitDetected = 1;
			m_ButtonStates = buttonStatesNow; 
			if (m_ButtonStates & MASK_BUTTON_A)  m_ButtonLastHit = 'A';
			else if (m_ButtonStates & MASK_BUTTON_B)  m_ButtonLastHit = 'B';
			else if (m_ButtonStates & MASK_BUTTON_C)  m_ButtonLastHit = 'C';
			else if (m_ButtonStates & MASK_BUTTON_D)  m_ButtonLastHit = 'D';
			else if (m_ButtonStates & MASK_BUTTON_STAR)  m_ButtonLastHit = '*';
			else if (m_ButtonStates & MASK_BUTTON_HASH)  m_ButtonLastHit = '#';
			else  m_ButtonLastHit = 0;  // NUL
            m_ElapsedTime_ms = 0;  // reset screen timeout
			taskState = 0;
		}
	}
	else if (taskState == 3)  // De-bounce delay after release (100ms)
	{
		if (buttonStatesNow != ALL_BUTTONS_RELEASED)  // glitch - retry
			taskState = 0;
	
		if (debounceTimer_ms >= 100)
		{
			m_ButtonStates = buttonStatesNow;
			taskState = 1;
		}
	}
	
	debounceTimer_ms += 5;
}


/*
 * Function:     GetButtonStates()
 *
 * Overview:     Returns the current input states of the 6 buttons, de-bounced,
 *               as 6 LS bits of the data.  A button pressed is represented by a High bit.
 *               Button bit assignments are defined in "remi_synth2_GUI.h".
 *
 * Return val:   (uint8) m_ButtonStates
 */
uint8  GetButtonStates(void)
{
    return  m_ButtonStates;  
}


/*
 * Function:     uint8 KeyHit()
 *
 * Overview:     Tests for a button hit, i.e. transition from not pressed to pressed.
 *               A private flag, m_ButtonHitDetected, is cleared on exit so that the 
 *               function will return TRUE once only for each button press event.
 *
 * Return val:   TRUE (1) if a key hit was detected since the last call, else FALSE (0).
 */
uint8  KeyHit(void)
{
	uint8  result = m_ButtonHitDetected;
	
	m_ButtonHitDetected = 0;
	
    return  result;  
}


/*
 * Function:     short GetKey()
 *
 * Overview:     Returns the ASCII keycode of the last button press detected, i.e.
 *               following a call to function KeyHit() which returned TRUE.
 *
 * Note:         GetKey() may be called multiple times after a call to KeyHit().
 *               The function will return the same keycode on each call, up to
 *               100 milliseconds after the last button hit registered.
 *
 * Return val:   (uint8) ASCII keycode, one of: 'A', 'B', 'C', 'D', '*', or '#'.
 */
uint8  GetKey(void)
{
    return  m_ButtonLastHit;
}


/*================================================================================================
 *============  Following are application-specific Screen Functions  =============================
 *
 * These functions are not called directly by the application. They are called by
 * GUI_NavigationExec() with arg isNewScreen = TRUE on the first call following a screen
 * switch to render initial text and images (if any);  isNewScreen = FALSE on subsequent
 * periodic calls to refresh screen information and to monitor events, usually keypad
 * button hits, which need to be actioned. Some events may cause a screen switch.
 *
 * The table below shows vertical (y) coords for up to 6 lines of text, size 8px,
 * assuming a "Title bar" is shown along the top of the display (10px high).
 * Add 2 to y-coord where text is shown below a horizontal line separating menu options.
 *
 *            Text  |   Option 1   |   Option 2
 *            Line  | Spacing 10px | Spacing 8px
 *           -------+--------------+--------------
 *              1   |  y = 12      |  y = 12
 *              2   |      22      |      20
 *              3   |      32      |      28
 *              4   |      42      |      36
 *              5   |     (52)     |      42
 *              6   |      --      |     (50)
 *           -------+--------------+--------------
 *
 * Example screen function:
 * The function below displays the startup screen for 3 seconds, then triggers a
 * switch to the Main Menu screen. The screen timer, m_ElapsedTime_ms, is managed by
 * GUI_NavigationExec() -- the timer is re-started whenever a screen switch occurs
 * or if a key hit is actioned.
 * 
 */
PRIVATE  void  ScreenFunc_Startup(bool isNewScreen)
{
    int   i;
    bool  isFailedSelfTest;

    if (isNewScreen)  
    {
        LCD_Mode(SET_PIXELS);
        LCD_PosXY(21, 2);
        LCD_PutImage(Bauer_remi_logo_85x45, 85, 45);

        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(3, 56);
        LCD_PutText("Running self-test...");
    }
    else  // do periodic update...
    {
        if (m_ElapsedTime_ms >= SELF_TEST_WAIT_TIME_MS)
        {
            isFailedSelfTest = 0;
            // Check self-test results... if fail, go to test results screen
            for (i = 0;  i < NUMBER_OF_SELFTEST_ITEMS;  i++)
            {
                if (g_SelfTestFault[i] != 0) isFailedSelfTest = 1;
            }

            if (isFailedSelfTest)  GoToNextScreen(SCN_SELFTEST_REPORT);
            else  GoToNextScreen(SCN_HOME);
        }
    }
}


PRIVATE  void  ScreenFunc_SelfTestReport(bool isNewScreen)
{
    static  char  *SelfTestName[] = { "Software Timer", "MCU device ID", "MIDI comm's",
                                      "EEPROM defaulted", };
    int     i;
    uint16  y;

    if (isNewScreen)  
    {
        LCD_SetFont(PROP_8_NORM);
        LCD_Mode(SET_PIXELS);
        LCD_PosXY(0, 53);
        LCD_DrawLineHoriz(128);
        DisplayMenuOption(0, 56, '*', "Restart");
        DisplayMenuOption(80, 56, '#', "Ignore");

        for ((y = 12, i = 0);  i < NUMBER_OF_SELFTEST_ITEMS;  i++)
        {
            if (g_SelfTestFault[i])  // this test failed...
            {
                LCD_PosXY(10, y);
                LCD_PutText(SelfTestName[i]);
                y = y + 10;
            }
            if (y >= (12 + 40))  break;  // screen full
        }
    }
    else  // do periodic update...
    {
        if (KeyHit())
        {
            if (GetKey() == '*') BootReset();
            else if (GetKey() == '#') GoToNextScreen(SCN_HOME);
        }
    }
}


PRIVATE  void  ScreenFunc_Home(bool isNewScreen)
{
    char    textBuf[40];
    uint8   preset, digit;

    if (isNewScreen)  
    {
        LCD_Mode(SET_PIXELS);
        LCD_PosXY(22, 12);
        LCD_PutImage(Remi_logo_85x30, 85, 30);
        LCD_PosXY(2, 2);
        LCD_PutImage(treble_clef_16x40, 16, 40);

        DisplayMenuOption(0,  46, 'A', "+ Pr -");
        DisplayMenuOption(44, 46, 'B', "");
        DisplayMenuOption(60, 46, 'C', "Control");
        DisplayMenuOption(0,  56, '*', "SETUP");
        DisplayMenuOption(60, 56, '#', "PRESET");

        LCD_PosXY(32, 1);
        LCD_SetFont(PROP_8_NORM);
        LCD_PutText("EWI Synth");

        sprintf(textBuf, "v%d.%d.%02d", g_FW_version[0], g_FW_version[1], g_FW_version[2]);
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(90, 1);
        LCD_PutText(textBuf);
        
        m_presetLastShown = 999;        // force Preset # display update
        m_HandsetDetectedLast = FALSE;  // force handset status update
    }
    else  // do periodic update...
    {
        if (KeyHit())
        {
            // Main menu options...
            if (GetKey() == '*')  GoToNextScreen(SCN_SETUP_MIDI_IN);
            if (GetKey() == '#')  GoToNextScreen(SCN_PRESET_EDIT_MENU);
            if (GetKey() == 'A')  // Increment Preset
            {
                preset = g_Config.PresetLastSelected + 1;
                if (preset >= 8)  preset = 0;  // wrap
                InstrumentPresetSelect(preset);
            }
            if (GetKey() == 'B')  // Decrement Preset
            {
                preset = g_Config.PresetLastSelected - 1;
                if (preset >= 8)  preset = 7;  // unsigned byte
                InstrumentPresetSelect(preset);
            }
            if (GetKey() == 'C')  GoToNextScreen(SCN_MISC_CONTROL_MENU);
            if (GetKey() == 'D')  GoToNextScreen(SCN_CUSTOM_FUNC_MENU);
        }
        
        if (g_Config.PresetLastSelected != m_presetLastShown)
        {
            // Show active Preset number (digit = 1..8)
            digit = g_Config.PresetLastSelected;   // index 0..7
            if (digit == 0) digit = 8;  // Preset '8' is stored as '0'
            LCD_Mode(CLEAR_PIXELS);
            LCD_PosXY(114, 48);
            LCD_DrawBar(12, 16);
            LCD_SetFont(MONO_16_NORM);
            LCD_Mode(SET_PIXELS);
            LCD_PosXY(114, 48);
            LCD_PutChar('0' + digit);
            m_presetLastShown = g_Config.PresetLastSelected;
        }
        
        if (isHandsetConnected() && !m_HandsetDetectedLast)   // re-connected
        {
            LCD_Mode(SET_PIXELS);  // Show MIDI IN (DIN5) icon
            LCD_PosXY(114, 18);
            LCD_PutImage(midi_conn_icon_9x9, 9, 9);
            m_HandsetDetectedLast = TRUE;
        }
        else if (m_HandsetDetectedLast && !isHandsetConnected())   // unplugged
        {
            LCD_Mode(CLEAR_PIXELS);  // Erase MIDI IN icon
            LCD_PosXY(114, 18);
            LCD_DrawBar(10, 10);
            m_HandsetDetectedLast = FALSE;
        }
    }
}


PRIVATE  void  ScreenFunc_SetupMidiInParams(bool isNewScreen)
{
    char    textBuf[40];

    if (isNewScreen)  // new screen...
    {
        DisplayMenuOption(0,  12, 'A', "");
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(12, 12);
        sprintf(textBuf, "MIDI IN mode: %d", g_Config.MidiInMode);
        LCD_PutText(textBuf);
        
        DisplayMenuOption(0,  22, 'B', "");
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(12, 22);
        sprintf(textBuf, "MIDI IN channel: %d", g_Config.MidiInChannel);
        LCD_PutText(textBuf);
        
        DisplayMenuOption(0,  32, 'C', "");
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(12, 32);
        sprintf(textBuf, "Pressure CC#: %d", g_Config.MidiInPressureCCnum);
        LCD_PutText(textBuf);

        DisplayMenuOption(0,  42, 'D', "");
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(12, 42);
        sprintf(textBuf, "Modulation CC#: %d", g_Config.MidiInModulationCCnum);
        LCD_PutText(textBuf);

        LCD_PosXY(0, 53);
        LCD_DrawLineHoriz(128);
        DisplayMenuOption(0,  56, '*', "Exit");
        DisplayMenuOption(88, 56, '#', "Next");
        LCD_PosXY(56, 56);
    }
    else  // update screen & check for button hit
    {
        if (KeyHit())
        {
            if (GetKey() == '*') GoToNextScreen(SCN_HOME);
            else if (GetKey() == 'A') GoToNextScreen(SCN_SET_MIDI_IN_MODE);
            else if (GetKey() == 'B') GoToNextScreen(SCN_SET_MIDI_IN_CHANNEL);
            else if (GetKey() == 'C') GoToNextScreen(SCN_SET_MIDI_IN_PRESS_CC);
            else if (GetKey() == 'D') GoToNextScreen(SCN_SET_MIDI_IN_MODLN_CC);
            else if (GetKey() == '#') GoToNextScreen(SCN_SETUP_MIDI_OUT);
        }
    }

    if (m_ElapsedTime_ms >= GUI_INACTIVE_TIMEOUT)
    {
        GoToNextScreen(SCN_HOME);
    }
}


PRIVATE  void  ScreenFunc_SetupMidiOutParams(bool isNewScreen)
{
    char    textBuf[40];

    if (isNewScreen)  // new screen...
    {
        DisplayMenuOption(0,  12, 'A', "");
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(12, 12);
        sprintf(textBuf, "MIDI OUT mode: %d", g_Config.MidiOutMode);
        LCD_PutText(textBuf);
        
        DisplayMenuOption(0,  22, 'B', "");
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(12, 22);
        sprintf(textBuf, "MIDI OUT channel: %d", g_Config.MidiOutChannel);
        LCD_PutText(textBuf);
        
        DisplayMenuOption(0,  32, 'C', "");
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(12, 32);
        sprintf(textBuf, "Pressure CC#: %d", g_Config.MidiOutPressureCCnum);
        LCD_PutText(textBuf);

        DisplayMenuOption(0,  42, 'D', "");
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(12, 42);
        sprintf(textBuf, "Modulation CC#: %d", g_Config.MidiOutModulationCCnum);
        LCD_PutText(textBuf);

        LCD_PosXY(0, 53);
        LCD_DrawLineHoriz(128);
        DisplayMenuOption(0,  56, '*', "Exit");
        DisplayMenuOption(88, 56, '#', "Next");
        LCD_PosXY(56, 56);
    }
    else
    {
        if (KeyHit())
        {
            if (GetKey() == '*') GoToNextScreen(SCN_HOME);
            else if (GetKey() == 'A') GoToNextScreen(SCN_SET_MIDI_OUT_MODE);
            else if (GetKey() == 'B') GoToNextScreen(SCN_SET_MIDI_OUT_CHANNEL);
            else if (GetKey() == 'C') GoToNextScreen(SCN_SET_MIDI_OUT_PRESS_CC);
            else if (GetKey() == 'D') GoToNextScreen(SCN_SET_MIDI_OUT_MODLN_CC);
            else if (GetKey() == '#') GoToNextScreen(SCN_SYSTEM_INFO_PAGE1);
        }
    }

    if (m_ElapsedTime_ms >= GUI_INACTIVE_TIMEOUT)
    {
        GoToNextScreen(SCN_HOME);
    }    
}


PRIVATE  void  ScreenFunc_PresetEditMenu(bool isNewScreen)
{
    char    textBuf[40];
    int     i;
    uint16  patchNum = 0;
    uint8   digit;
    char    patchName[32];
    bool    doRefresh = 0;
    
    if (isNewScreen)  // new screen...
    {
        m_editPreset = g_Config.PresetLastSelected;  // currently active preset

        LCD_PosXY(0, 53);
        LCD_DrawLineHoriz(128);
        DisplayMenuOption(0,  56, '*', "Home");
        DisplayMenuOption(88, 56, '#', "Next");
        
        doRefresh = 1;
    }
    else   // update screen, check button hit
    {
        if (KeyHit())
        {
            if (GetKey() == '*') GoToNextScreen(SCN_HOME);
            else if (GetKey() == 'A')  GoToNextScreen(SCN_EDIT_PRESET_PATCH);
            else if (GetKey() == 'B')  // Edit Preset Midi Pgrm
            {
                m_DataEntryTitle = "Enter MIDI Pgrm #";
                m_DataEntryNextScreen = SCN_EDIT_PRESET_MIDI_PGRM;
                m_DataEntryNumDigits = 3;
                m_DataEntryValue = g_Preset.Descr[m_editPreset].MidiProgram;
                GoToNextScreen(SCN_DATA_ENTRY);
            }
            else if (GetKey() == 'C')  GoToNextScreen(SCN_EDIT_PRESET_VIBRATO_MODE);
            else if (GetKey() == 'D')  // Edit Preset Transpose
            {
                m_DataEntryTitle = "Enter Transpose Qty";
                m_DataEntryNextScreen = SCN_EDIT_PRESET_TRANSPOSE;
                m_DataEntryNumDigits = 2;
                m_DataEntryValue = g_Preset.Descr[m_editPreset].PitchTranspose;
                GoToNextScreen(SCN_DATA_ENTRY);
            }
            else if (GetKey() == '#')  // select next preset to edit
            {
                if (++m_editPreset >= 8)  m_editPreset = 0;  
                doRefresh = 1;
            }
        }
    }
    
    if (doRefresh)
    {
        LCD_Mode(CLEAR_PIXELS);  // Erase information display area
        LCD_PosXY(0, 12);
        LCD_BlockFill(128, 40);
        LCD_Mode(SET_PIXELS);    // Erase Preset # in Title Bar
        LCD_PosXY(50, 1);        
        LCD_BlockFill(8, 9);
        
        // Display the Preset number (digit, 1..8) in the Title Bar
        if (m_editPreset == 0) digit = '8';
        else  digit = m_editPreset + '0';   // ASCII char
        LCD_Mode(CLEAR_PIXELS);
        LCD_SetFont(MONO_8_NORM);
        LCD_PosXY(51, 1);
        LCD_PutChar(digit);
        LCD_Mode(SET_PIXELS);
        
        // Search table of pre-defined patches for patchNum...
        patchNum = g_Preset.Descr[m_editPreset].RemiSynthPatch;  // Patch ID #

        for (i = 0;  i < GetNumberOfPatchesDefined();  i++)
        {
            if (g_PatchProgram[i].PatchNumber == patchNum)  break;
        }

        if (i < GetNumberOfPatchesDefined())  // found patchNum in predefined patches
        {
            strncpy(patchName, (char *) g_PatchProgram[i].PatchName, 20);
            patchName[18] = 0;   // crop to fit display line
        }
        else if (patchNum == 0)  strcpy(patchName, "User Patch (ID: 0)");
        else  strcpy(patchName, "Undefined Patch!");

        DisplayMenuOption(0,  12, 'A', "");
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(12, 12);
        LCD_PutImage(patch_icon_7x7, 7, 7);
        LCD_PosXY(22, 12);
        LCD_PutText(patchName);
        
        DisplayMenuOption(0,  22, 'B', "");
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(12, 22);
        sprintf(textBuf, "MIDI OUT Pgrm: %03d", g_Preset.Descr[m_editPreset].MidiProgram);
        LCD_PutText(textBuf);
        
        DisplayMenuOption(0,  32, 'C', "Vibrato Mode: ");
        
        if (g_Preset.Descr[m_editPreset].VibratoMode == VIBRATO_BY_EFFECT_SW)
            LCD_PutText("Fx Sw");
        else if (g_Preset.Descr[m_editPreset].VibratoMode == VIBRATO_BY_MODN_CC)
            LCD_PutText("Mod.CC");
        else if (g_Preset.Descr[m_editPreset].VibratoMode == VIBRATO_AUTOMATIC)
            LCD_PutText("Auto");
        else  LCD_PutText("Off");

        DisplayMenuOption(0,  42, 'D', "Pitch Transpose: ");
        sprintf(textBuf, "%+d", (int) g_Preset.Descr[m_editPreset].PitchTranspose);
        LCD_PutText(textBuf);       
        
        doRefresh = 0;   // inhibit refresh until next preset selected
    }

    if (m_ElapsedTime_ms >= GUI_INACTIVE_TIMEOUT)
    {
        GoToNextScreen(SCN_HOME);
    }    
}


PRIVATE  void  ScreenFunc_MiscControlsMenu(bool isNewScreen)
{
    char    textBuf[40];
    
    if (isNewScreen)  // new screen...
    {
        DisplayMenuOption(0,  12, 'A', "");
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(12, 12);
        sprintf(textBuf, "Reverb Atten:  %02d %%", g_Config.ReverbAtten_pc);
        LCD_PutText(textBuf);
        
        DisplayMenuOption(0,  22, 'B', "");
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(12, 22);
        sprintf(textBuf, "Reverb Mix:  %02d %%", g_Config.ReverbMix_pc);
        LCD_PutText(textBuf);
/*        
        DisplayMenuOption(0,  32, 'C', "");
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(12, 32);
        sprintf(textBuf, "Reserved: %d", g_Config.Reserved101);
        LCD_PutText(textBuf);
*/
        DisplayMenuOption(0,  42, 'D', "");
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(12, 42);
        if (LCD_BACKLIGHT_IS_LOW)  LCD_PutText("LCD brightness: Lo");
        else  LCD_PutText("LCD brightness: Hi");

        LCD_PosXY(0, 53);
        LCD_DrawLineHoriz(128);
        DisplayMenuOption(0,  56, '*', "Exit");
        DisplayMenuOption(88, 56, '#', "Next");
        LCD_PosXY(56, 56);
    }
    else  // check for button hit
    {
        if (KeyHit())
        {
            if (GetKey() == '*') GoToNextScreen(SCN_HOME);
            else if (GetKey() == 'A') 
            {
                m_DataEntryTitle = "Enter 0..100 (%)";
                m_DataEntryNextScreen = SCN_EDIT_REVERB_ATTEN;
                m_DataEntryNumDigits = 3;
                m_DataEntryValue = g_Config.ReverbAtten_pc;
                GoToNextScreen(SCN_DATA_ENTRY);
            }
            else if (GetKey() == 'B')
            {
                m_DataEntryTitle = "Enter 0..100 (%)";
                m_DataEntryNextScreen = SCN_EDIT_REVERB_MIX;
                m_DataEntryNumDigits = 3;
                m_DataEntryValue = g_Config.ReverbMix_pc;
                GoToNextScreen(SCN_DATA_ENTRY);
            }
//          else if (GetKey() == 'C')  { ... }   // reserved
            else if (GetKey() == 'D')
            {
                if (LCD_BACKLIGHT_IS_LOW)  LCD_BACKLIGHT_HIGH();
                else  LCD_BACKLIGHT_LOW();   // Toggle LCD backlight switch
                GoToNextScreen(SCN_MISC_CONTROL_MENU);  // Update display
            }
            else if (GetKey() == '#') GoToNextScreen(SCN_HOME);  // todo: Next page (if any)
        }
    }

    if (m_ElapsedTime_ms >= GUI_INACTIVE_TIMEOUT)
    {
        GoToNextScreen(SCN_HOME);
    }
}


PRIVATE  void  ScreenFunc_SetMidiInMode(bool isNewScreen)
{
    bool   doRefresh = 0;
    uint8  mode = g_Config.MidiInMode;   // can be 2 or 4

    if (isNewScreen)  // new screen
    {
        LCD_Mode(SET_PIXELS);
        LCD_PosXY(0, 53);
        LCD_DrawLineHoriz(128);
        LCD_SetFont(PROP_8_NORM);
        DisplayMenuOption( 0, 56, '*', "Home");
        DisplayMenuOption(40, 56, 'C', "Change");
        DisplayMenuOption(90, 56, '#', "Back");
        doRefresh = 1;
    }
    else  // check for button press, update screen
    {
        if (KeyHit())
        {
            if (GetKey() == 'C')  // change mode
            {
                if (mode != 2)  mode = 2;  else  mode = 4;
                g_Config.MidiInMode = mode;
                StoreConfigData();
            }
            else if (GetKey() == '*')  GoToNextScreen(SCN_HOME);
            else if (GetKey() == '#')  GoToNextScreen(SCN_SETUP_MIDI_IN);
            doRefresh = 1;
        }
    }

    if (doRefresh)
    {
        LCD_Mode(CLEAR_PIXELS);  // erase existing info
        LCD_PosXY(0, 22);
        LCD_BlockFill(128, 20);
        
        LCD_Mode(SET_PIXELS);
        LCD_SetFont(MONO_8_NORM);
        LCD_PosXY(4, 22);
        LCD_PutText("Current setting: ");
        
        if (mode == 2)  
        {
            LCD_PutText("2");
            LCD_PosXY(16, 32);
            LCD_PutText("Omni-On, Mono");
        }
        else if (mode == 4)  
        {
            LCD_PutText("4");
            LCD_PosXY(16, 32);
            LCD_PutText("Omni-Off, Mono");
        }
        else  LCD_PutText("?");  // unlikely!
        
        doRefresh = 0;   // inhibit refresh until next key hit
    }
}


PRIVATE  void  ScreenFunc_SetMidiInChannel(bool isNewScreen)
{
    static uint8  channel;  // 1..16
    char   textBuf[40];
    bool   doRefresh = 0;

    if (isNewScreen)  // new screen
    {
        channel = g_Config.MidiInChannel;  // 1..16
        
        LCD_Mode(SET_PIXELS);
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(4, 22);
        LCD_PutText("Current setting:");
        LCD_PosXY(0, 42);
        LCD_DrawLineHoriz(128);
        
        DisplayMenuOption(32, 46, 'A', "+4");
        DisplayMenuOption(64, 46, 'B', "-1");
        DisplayMenuOption( 0, 56, '*', "Home");
        DisplayMenuOption(88, 56, '#', "Back");
        doRefresh = 1;
    }
    else  // check for button press, update screen
    {
        if (KeyHit())
        {
            if (GetKey() == '*')  
            {
                g_Config.MidiInChannel = channel;
                StoreConfigData();
                GoToNextScreen(SCN_HOME);
            }
            else if (GetKey() == '#')
            {
                g_Config.MidiInChannel = channel;
                StoreConfigData();
                GoToNextScreen(SCN_SETUP_MIDI_IN);
            }
            else if (GetKey() == 'A')
            {
                channel += 4;
                channel = channel % 16;
                if (channel == 0) channel = 16;
            }
            else if (GetKey() == 'B')
            {
                if (channel == 1)  channel = 16;
                else  channel--;
            }

            doRefresh = 1;
        }
    }

    if (doRefresh)
    {
        LCD_Mode(CLEAR_PIXELS);  // erase existing value
        LCD_PosXY(90, 22);
        LCD_BlockFill(30, 10);
        
        LCD_Mode(SET_PIXELS);
        LCD_SetFont(MONO_8_NORM);
        LCD_PosXY(90, 22);
        sprintf(textBuf, "%d", (int) channel);
        LCD_PutText(textBuf);
        doRefresh = 0;   // inhibit refresh until next key hit
    }
}


PRIVATE  void  ScreenFunc_SetMidiInPressureCC(bool isNewScreen)
{
    bool   doRefresh = 0;
    uint8  ccNumber = g_Config.MidiInPressureCCnum;   // can be: 0, 2, 7 or 11

    if (isNewScreen)  // new screen
    {
        LCD_Mode(SET_PIXELS);
        LCD_PosXY(0, 53);
        LCD_DrawLineHoriz(128);
        LCD_SetFont(PROP_8_NORM);
        DisplayMenuOption( 0, 56, '*', "Home");
        DisplayMenuOption(40, 56, 'C', "Change");
        DisplayMenuOption(90, 56, '#', "Back");
        doRefresh = 1;
    }
    else  // check for button press, update screen
    {
        if (KeyHit())
        {
            if (GetKey() == 'C')  // change CC number
            {
                if (ccNumber == 0)  ccNumber = 2;  
                else if (ccNumber == 2)  ccNumber = 7;  
                else if (ccNumber == 7)  ccNumber = 11;
                else  ccNumber = 0;
                g_Config.MidiInPressureCCnum = ccNumber;
                StoreConfigData();
            }
            else if (GetKey() == '*')  GoToNextScreen(SCN_HOME);
            else if (GetKey() == '#')  GoToNextScreen(SCN_SETUP_MIDI_IN);
            doRefresh = 1;
        }
    }

    if (doRefresh)
    {
        LCD_Mode(CLEAR_PIXELS);  // erase existing info
        LCD_PosXY(0, 22);
        LCD_BlockFill(128, 20);
        
        LCD_Mode(SET_PIXELS);
        LCD_SetFont(MONO_8_NORM);
        LCD_PosXY(4, 22);
        LCD_PutText("Pressure CC #: ");
        
        if (ccNumber == 0)  
        {
            LCD_PutText("0");
            LCD_PosXY(32, 32);
            LCD_PutText("No response");
        }
        else if (ccNumber == 2)  
        {
            LCD_PutText("02");
            LCD_PosXY(32, 32);
            LCD_PutText("Breath pressure");
        }
        else if (ccNumber == 7)  
        {
            LCD_PutText("07");
            LCD_PosXY(32, 32);
            LCD_PutText("Channel volume");
        }
        else if (ccNumber == 11)  
        {
            LCD_PutText("11");
            LCD_PosXY(32, 32);
            LCD_PutText("Expression");
        }
        else  LCD_PutText("?");  // unlikely!
        
        doRefresh = 0;   // inhibit refresh until next key hit
    }
}


PRIVATE  void  ScreenFunc_SetMidiInModulationCC(bool isNewScreen)
{
    char   textBuf[40];
    bool   doRefresh = 0;
    uint8  ccNumber = g_Config.MidiInModulationCCnum;   // can be: 0 ~ 31

    if (isNewScreen)  // new screen
    {
        LCD_Mode(SET_PIXELS);
        LCD_PosXY(0, 42);
        LCD_DrawLineHoriz(128);
        
        DisplayMenuOption(32, 46, 'A', " +5");
        DisplayMenuOption(72, 46, 'B', " -1");
        DisplayMenuOption( 0, 56, '*', "Home");
        DisplayMenuOption(88, 56, '#', "Back");
        doRefresh = 1;
    }
    else  // check for button press, update screen
    {
        if (KeyHit())
        {
            if (GetKey() == '*')  
            {
                g_Config.MidiInModulationCCnum = ccNumber;
                StoreConfigData();
                GoToNextScreen(SCN_HOME);
            }
            else if (GetKey() == '#')
            {
                g_Config.MidiInModulationCCnum = ccNumber;
                StoreConfigData();
                GoToNextScreen(SCN_SETUP_MIDI_IN);
            }
            else if (GetKey() == 'A')
            {
                ccNumber += 5;
                if (ccNumber >= 32) ccNumber = 1;
            }
            else if (GetKey() == 'B')
            {
                if (ccNumber == 0)  ccNumber = 31;
                else  ccNumber--;
            }
            else if (GetKey() == 'C')  ccNumber = 0;  // set zero value
            else if (GetKey() == 'D')  ccNumber = 1;  // set default value
            
            doRefresh = 1;
        }
    }

    if (doRefresh)
    {
        LCD_Mode(CLEAR_PIXELS);  // erase existing value
        LCD_PosXY(0, 22);
        LCD_BlockFill(128, 10);
        
        LCD_Mode(SET_PIXELS);
        LCD_SetFont(MONO_8_NORM);
        LCD_PosXY(6, 22);
        sprintf(textBuf, "Modulation CC #: %02d", (int) ccNumber);
        LCD_PutText(textBuf);
        
        doRefresh = 0;   // inhibit refresh until next key hit
    }
}


PRIVATE  void  ScreenFunc_SetMidiOutMode(bool isNewScreen)
{
    bool   doRefresh = 0;
    uint8  mode = g_Config.MidiOutMode;  // may be 0, 1, 2, 3 or 4

    if (isNewScreen)  // new screen
    {
        LCD_Mode(SET_PIXELS);
        LCD_PosXY(0, 53);
        LCD_DrawLineHoriz(128);
        LCD_SetFont(PROP_8_NORM);
        DisplayMenuOption( 0, 56, '*', "Home");
        DisplayMenuOption(40, 56, 'C', "Change");
        DisplayMenuOption(90, 56, '#', "Back");
        doRefresh = 1;
    }
    else  // check for button press, update screen
    {
        if (KeyHit())
        {
            if (GetKey() == 'C')  // change mode
            {
                if (++mode >= 4)  mode = 0;
                g_Config.MidiOutMode = mode;
                StoreConfigData();
            }
            else if (GetKey() == '*')  GoToNextScreen(SCN_HOME);
            else if (GetKey() == '#')  GoToNextScreen(SCN_SETUP_MIDI_OUT);
            doRefresh = 1;
        }
    }

    if (doRefresh)
    {
        LCD_Mode(CLEAR_PIXELS);  // erase existing info
        LCD_PosXY(0, 22);
        LCD_BlockFill(128, 20);
        
        LCD_Mode(SET_PIXELS);
        LCD_SetFont(MONO_8_NORM);
        LCD_PosXY(4, 22);
        LCD_PutText("Current setting: ");
        
        if (mode == 2)  
        {
            LCD_PutText("2");
            LCD_PosXY(16, 32);
            LCD_PutText("Omni-On, Mono");
        }
        else if (mode == 4)  
        {
            LCD_PutText("4");
            LCD_PosXY(16, 32);
            LCD_PutText("Omni-Off, Mono");
        }
        else  LCD_PutText("?");  // unlikely!
        
        doRefresh = 0;   // inhibit refresh until next key hit
    }
}


PRIVATE  void  ScreenFunc_SetMidiOutChannel(bool isNewScreen)
{
    static uint8  channel;  // 1..16
    char   textBuf[40];
    bool   doRefresh = 0;

    if (isNewScreen)  // new screen
    {
        channel = g_Config.MidiOutChannel;  // 1..16
        
        LCD_Mode(SET_PIXELS);
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(4, 22);
        LCD_PutText("Current setting:");
        LCD_PosXY(0, 42);
        LCD_DrawLineHoriz(128);
        
        DisplayMenuOption(32, 46, 'A', "+4");
        DisplayMenuOption(64, 46, 'B', "-1");
        DisplayMenuOption( 0, 56, '*', "Home");
        DisplayMenuOption(88, 56, '#', "Back");
        doRefresh = 1;
    }
    else  // check for button press, update screen
    {
        if (KeyHit())
        {
            if (GetKey() == '*')  
            {
                g_Config.MidiOutChannel = channel;
                StoreConfigData();
                GoToNextScreen(SCN_HOME);
            }
            else if (GetKey() == '#')
            {
                g_Config.MidiOutChannel = channel;
                StoreConfigData();
                GoToNextScreen(SCN_SETUP_MIDI_OUT);
            }
            else if (GetKey() == 'A')
            {
                channel += 4;
                channel = channel % 16;
                if (channel == 0) channel = 16;
            }
            else if (GetKey() == 'B')
            {
                if (channel == 1)  channel = 16;
                else  channel--;
            }

            doRefresh = 1;
        }
    }

    if (doRefresh)
    {
        sprintf(textBuf, "%d", (int) channel);
        LCD_Mode(CLEAR_PIXELS);  // erase existing value
        LCD_PosXY(90, 22);
        LCD_BlockFill(30, 10);
        LCD_Mode(SET_PIXELS);
        LCD_SetFont(MONO_8_NORM);
        LCD_PosXY(90, 22);
        LCD_PutText(textBuf);
        doRefresh = 0;   // inhibit refresh until next key hit
    }
}


PRIVATE  void  ScreenFunc_SetMidiOutPressureCC(bool isNewScreen)
{
    bool   doRefresh = 0;
    uint8  ccNumber = g_Config.MidiOutPressureCCnum;   // can be: 0, 2, 7 or 11

    if (isNewScreen)  // new screen
    {
        LCD_Mode(SET_PIXELS);
        LCD_PosXY(0, 53);
        LCD_DrawLineHoriz(128);
        LCD_SetFont(PROP_8_NORM);
        DisplayMenuOption( 0, 56, '*', "Home");
        DisplayMenuOption(40, 56, 'C', "Change");
        DisplayMenuOption(90, 56, '#', "Back");
        doRefresh = 1;
    }
    else  // check for button press, update screen
    {
        if (KeyHit())
        {
            if (GetKey() == 'C')  // change CC number
            {
                if (ccNumber == 0)  ccNumber = 2;  
                else if (ccNumber == 2)  ccNumber = 7;  
                else if (ccNumber == 7)  ccNumber = 11;
                else  ccNumber = 0;
                g_Config.MidiOutPressureCCnum = ccNumber;
                StoreConfigData();
            }
            else if (GetKey() == '*')  GoToNextScreen(SCN_HOME);
            else if (GetKey() == '#')  GoToNextScreen(SCN_SETUP_MIDI_OUT);
            doRefresh = 1;
        }
    }

    if (doRefresh)
    {
        LCD_Mode(CLEAR_PIXELS);  // erase existing info
        LCD_PosXY(0, 22);
        LCD_BlockFill(128, 20);
        
        LCD_Mode(SET_PIXELS);
        LCD_SetFont(MONO_8_NORM);
        LCD_PosXY(4, 22);
        LCD_PutText("Pressure CC #: ");
        
        if (ccNumber == 0)  
        {
            LCD_PutText("0");
            LCD_PosXY(32, 32);
            LCD_PutText("No response");
        }
        else if (ccNumber == 2)  
        {
            LCD_PutText("02");
            LCD_PosXY(32, 32);
            LCD_PutText("Breath pressure");
        }
        else if (ccNumber == 7)  
        {
            LCD_PutText("07");
            LCD_PosXY(32, 32);
            LCD_PutText("Channel volume");
        }
        else if (ccNumber == 11)  
        {
            LCD_PutText("11");
            LCD_PosXY(32, 32);
            LCD_PutText("Expression");
        }
        else  LCD_PutText("?");  // unlikely!
        
        doRefresh = 0;   // inhibit refresh until next key hit
    }
}


PRIVATE  void  ScreenFunc_SetMidiOutModulationCC(bool isNewScreen)
{
    char   textBuf[40];
    bool   doRefresh = 0;
    uint8  ccNumber = g_Config.MidiOutModulationCCnum;   // can be: 0 ~ 31

    if (isNewScreen)  // new screen
    {
        LCD_Mode(SET_PIXELS);
        LCD_PosXY(0, 42);
        LCD_DrawLineHoriz(128);
        
        DisplayMenuOption(32, 46, 'A', " +5");
        DisplayMenuOption(72, 46, 'B', " -1");
        DisplayMenuOption( 0, 56, '*', "Home");
        DisplayMenuOption(88, 56, '#', "Back");
        doRefresh = 1;
    }
    else  // check for button press, update screen
    {
        if (KeyHit())
        {
            if (GetKey() == '*')  
            {
                g_Config.MidiOutModulationCCnum = ccNumber;
                StoreConfigData();
                GoToNextScreen(SCN_HOME);
            }
            else if (GetKey() == '#')
            {
                g_Config.MidiOutModulationCCnum = ccNumber;
                StoreConfigData();
                GoToNextScreen(SCN_SETUP_MIDI_OUT);
            }
            else if (GetKey() == 'A')
            {
                ccNumber += 5;
                if (ccNumber >= 32) ccNumber = 1;
            }
            else if (GetKey() == 'B')
            {
                if (ccNumber == 0)  ccNumber = 31;
                else  ccNumber--;
            }
            else if (GetKey() == 'C')  ccNumber = 0;  // set zero value
            else if (GetKey() == 'D')  ccNumber = 1;  // set default value
            
            doRefresh = 1;
        }
    }

    if (doRefresh)
    {
        LCD_Mode(CLEAR_PIXELS);  // erase existing value
        LCD_PosXY(0, 22);
        LCD_BlockFill(128, 10);
        
        LCD_Mode(SET_PIXELS);
        LCD_SetFont(MONO_8_NORM);
        LCD_PosXY(6, 22);
        sprintf(textBuf, "Modulation CC #: %02d", (int) ccNumber);
        LCD_PutText(textBuf);
        
        doRefresh = 0;   // inhibit refresh until next key hit
    }
}


/*
 * Function allows the user to select a REMI synth patch for the Preset being edited.
 * A list of four patch names is displayed from the array of predefined patches.
 * The user can select a patch from the list or scroll down to the next 4 names.
 * If the Preset being edited is the current Preset, then the new patch is activated.
  */
PRIVATE  void  ScreenFunc_EditPresetPatch(bool isNewScreen)
{
    static  int   itop = 0;   // index into g_PatchProgram[], top line of 4 listed
    int     line, ypos;       // line # of displayed/selected patch (0..3)
    uint16  selectedPatchID;
    char    symbol;
    bool    doRefresh = 0;
    int     activePreset = g_Config.PresetLastSelected;  // current preset index (0..7)

    if (isNewScreen)  // new screen
    {
        LCD_PosXY(0, 53);
        LCD_DrawLineHoriz(128);
        DisplayMenuOption(0, 56, '*', "Exit");
        DisplayMenuOption(88, 56, '#', "Page");
        itop = 0;
        doRefresh = 1;
    }
    else  // do periodic update
    {
        if (KeyHit())
        {
            if (GetKey() == '*')  GoToNextScreen(SCN_PRESET_EDIT_MENU);  // exit
            else if (GetKey() == '#')    // next page
            {
                itop = itop + 4;
                if (itop >= GetNumberOfPatchesDefined())  itop = 0;  // wrap
                doRefresh = 1;
            }
            else if (GetKey() >= 'A')    // A, B, C or D
            {
                line = GetKey() - 'A';   // 0, 1, 2 or 3
                selectedPatchID = g_PatchProgram[itop+line].PatchNumber;
                g_Preset.Descr[m_editPreset].RemiSynthPatch = selectedPatchID;
                StorePresetData();
                if (m_editPreset == activePreset)
                    InstrumentPresetSelect(m_editPreset);  // activate the new patch
                GoToNextScreen(SCN_PRESET_EDIT_MENU);
            }
        }
    }
    
    if (doRefresh)   // List first/next four patch names
    {
        LCD_Mode(CLEAR_PIXELS);  // erase existing list
        LCD_PosXY(0, 12);
        LCD_BlockFill(128, 40);
        LCD_Mode(SET_PIXELS);
        
        for (line = 0;  line < 4;  line++)
        {
            if ((itop + line) < GetNumberOfPatchesDefined())
            {
                ypos = 12 + (line * 10);
                symbol = 'A' + line;
                DisplayMenuOption(8, ypos, symbol, "");
                LCD_SetFont(PROP_8_NORM);
                LCD_PosXY(20, ypos);
                LCD_PutText((uint8 *) g_PatchProgram[itop+line].PatchName);
            }
        }
        doRefresh = 0;   // inhibit refresh until next key hit
    }
}


/*
 * The MIDI (OUT) Program number for the Preset being edited is selected using the
 * general-purpose numeric Data Entry Screen.
 * This function validates the number which was entered in the Data Entry Screen.
 * If validated, the value is saved in the Preset Descriptor of the editing Preset.
 */
PRIVATE  void  ScreenFunc_EditPresetMidiProgram(bool isNewScreen)
{
    int    activePreset = g_Config.PresetLastSelected;  // current preset index
    int    value = 0;

    if (isNewScreen)  // first call after screen switch
    {
        if (m_DataEntryAccept)  // Accept key was hit
        {
            value = abs(m_DataEntryValue);
            
            if (value > 127)  // value out of bounds
            {
                LCD_SetFont(PROP_8_NORM);
                LCD_PosXY(8, 22);
                LCD_PutText("Data entry error!");
                LCD_PosXY(8, 32);
                LCD_PutText("Maximum value: 127");
                LCD_PosXY(8, 42);
                LCD_PutText("Preset unchanged");
                LCD_PosXY(0, 53);
                LCD_DrawLineHoriz(128);
                DisplayMenuOption(0, 56, '*', "Quit");
                DisplayMenuOption(80, 56, '#', "Retry");
            }
            else  // value OK
            {
                g_Preset.Descr[m_editPreset].MidiProgram = value;
                StorePresetData();
                if (m_editPreset == activePreset)
                    InstrumentPresetSelect(m_editPreset);  // activate the new setting
                GoToNextScreen(SCN_PRESET_EDIT_MENU);
            }
        }
        else  GoToNextScreen(SCN_PRESET_EDIT_MENU);  // Cancel key was hit
    }
    else   // do periodic update
    {
        if (KeyHit())
        {
            if (GetKey() == '*')  GoToNextScreen(SCN_PRESET_EDIT_MENU);  
            if (GetKey() == '#')  GoToNextScreen(SCN_DATA_ENTRY);
        }
    }
}


PRIVATE  void  ScreenFunc_EditPresetVibratoMode(bool isNewScreen)
{
    static  uint8   vib_mode;
    bool    doRefresh = 0;
    uint8   activePreset = g_Config.PresetLastSelected;  // current preset

    if (isNewScreen)  // new screen
    {
        LCD_Mode(SET_PIXELS);
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(8, 22);
        LCD_PutText("Vibrato Control Mode:");

        LCD_PosXY(0, 42);
        LCD_DrawLineHoriz(128);
        DisplayMenuOption(80, 45, 'C', "Change");
        DisplayMenuOption( 0, 56, '*', "Home");
        DisplayMenuOption(80, 56, '#', "Back");

        vib_mode = g_Preset.Descr[m_editPreset].VibratoMode;
        doRefresh = 1;
    }
    else  // monitor keypad
    {
        if (KeyHit())
        {
            if (GetKey() == '*')  GoToNextScreen(SCN_HOME);  else
            if (GetKey() == '#')  GoToNextScreen(SCN_PRESET_EDIT_MENU);  else
            if (GetKey() == 'C')  // Change mode
            {
                if (vib_mode == VIBRATO_DISABLED) vib_mode = VIBRATO_BY_EFFECT_SW;
                else if (vib_mode == VIBRATO_BY_EFFECT_SW) vib_mode = VIBRATO_BY_MODN_CC;
                else if (vib_mode == VIBRATO_BY_MODN_CC) vib_mode = VIBRATO_AUTOMATIC;
                else if (vib_mode == VIBRATO_AUTOMATIC) vib_mode = VIBRATO_DISABLED;
                else  vib_mode = VIBRATO_DISABLED;

                g_Preset.Descr[m_editPreset].VibratoMode = vib_mode;
                StorePresetData();
                if (m_editPreset == activePreset)
                    InstrumentPresetSelect(m_editPreset);  // activate the new setting
                doRefresh = 1;
            }
        }
    }

    if (doRefresh)
    {
        LCD_Mode(CLEAR_PIXELS);  // erase existing text
        LCD_PosXY(0, 32);
        LCD_BlockFill(128, 10);
        LCD_Mode(SET_PIXELS);    // write updated text
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(16, 32);

        if (vib_mode == VIBRATO_BY_EFFECT_SW)  LCD_PutText("Effect Switch");
        else if (vib_mode == VIBRATO_BY_MODN_CC)  LCD_PutText("Modulation CC");
        else if (vib_mode == VIBRATO_AUTOMATIC)  LCD_PutText("Automatic");
        else  LCD_PutText("Disabled");

        doRefresh = 0;   // inhibit refresh until next key hit
    }
}


/*
 * The Pitch Transpose value for the Preset being edited is selected using the
 * general-purpose numeric Data Entry Screen.
 * This function validates the number which was entered in the Data Entry Screen.
 * If validated, the value is saved in the Preset Descriptor of the editing Preset.
 */
PRIVATE  void  ScreenFunc_EditPresetTranspose(bool isNewScreen)
{
    int   activePreset = g_Config.PresetLastSelected;  // current preset index
    int   value = 0;

    if (isNewScreen)
    {
        if (m_DataEntryAccept)  // Accept key was hit
        {
            value = m_DataEntryValue;
            
            if (value < -24 || value > 24)  // offset out of bounds
            {
                LCD_SetFont(PROP_8_NORM);
                LCD_PosXY(8, 22);
                LCD_PutText("Value out of range!");
                LCD_PosXY(8, 32);
                LCD_PutText("(Legal range: +/-24)");
                LCD_PosXY(8, 42);
                LCD_PutText("Preset unchanged.");
                LCD_PosXY(0, 53);
                LCD_DrawLineHoriz(128);
                DisplayMenuOption(0, 56, '*', "Quit");
                DisplayMenuOption(80, 56, '#', "Retry");
            }
            else  // value OK
            {
                g_Preset.Descr[activePreset].PitchTranspose = value;
                StorePresetData();     // save the new setting
                if (m_editPreset == activePreset)
                    InstrumentPresetSelect(m_editPreset);  // activate the new setting
                GoToNextScreen(SCN_PRESET_EDIT_MENU);
            }
        }
        else  GoToNextScreen(SCN_PRESET_EDIT_MENU);  // Cancel key was hit
    }
    else   // do periodic update
    {
        if (KeyHit())
        {
            if (GetKey() == '*') GoToNextScreen(SCN_PRESET_EDIT_MENU);
            if (GetKey() == '#') GoToNextScreen(SCN_DATA_ENTRY);
        }
    }
}

/*
 * This function validates the number which was entered in the Data Entry Screen.
 * If validated, the value is saved in the Config. structure in EEPROM.
 */
PRIVATE  void  ScreenFunc_EditReverbAtten(bool isNewScreen)
{
    if (isNewScreen)  // first call after screen switch
    {
        if (m_DataEntryAccept)  // Accept key was hit
        {
            if (m_DataEntryValue < 0 || m_DataEntryValue > 100)  // value out of bounds
            {
                LCD_SetFont(PROP_8_NORM);
                LCD_PosXY(8, 22);
                LCD_PutText("Data entry error!");
                LCD_PosXY(8, 32);
                LCD_PutText("Min = 0, Max = 100");

                LCD_PosXY(0, 53);
                LCD_DrawLineHoriz(128);
                DisplayMenuOption(0, 56, '*', "Quit");
                DisplayMenuOption(80, 56, '#', "Retry");
            }
            else  // value OK
            {
                g_Config.ReverbAtten_pc = m_DataEntryValue;
                StoreConfigData();
                RemiSynthPrepare();   // Instate new value
                GoToNextScreen(SCN_MISC_CONTROL_MENU);
            }
        }
        else  GoToNextScreen(SCN_MISC_CONTROL_MENU);  // Cancel key was hit
    }
    else   // do periodic update
    {
        if (KeyHit())
        {
            if (GetKey() == '*')  GoToNextScreen(SCN_MISC_CONTROL_MENU);  
            if (GetKey() == '#')  GoToNextScreen(SCN_DATA_ENTRY);
        }
    }
}


PRIVATE  void  ScreenFunc_EditReverbMix(bool isNewScreen)
{
    if (isNewScreen)  // first call after screen switch
    {
        if (m_DataEntryAccept)  // Accept key was hit
        {
            if (m_DataEntryValue < 0 || m_DataEntryValue > 100)  // value out of bounds
            {
                LCD_SetFont(PROP_8_NORM);
                LCD_PosXY(8, 22);
                LCD_PutText("Data entry error!");
                LCD_PosXY(8, 32);
                LCD_PutText("Min = 0, Max = 100");

                LCD_PosXY(0, 53);
                LCD_DrawLineHoriz(128);
                DisplayMenuOption(0, 56, '*', "Quit");
                DisplayMenuOption(80, 56, '#', "Retry");
            }
            else  // value OK
            {
                g_Config.ReverbMix_pc = m_DataEntryValue;
                StoreConfigData();
                RemiSynthPrepare();   // Instate new value
                GoToNextScreen(SCN_MISC_CONTROL_MENU);
            }
        }
        else  GoToNextScreen(SCN_MISC_CONTROL_MENU);  // Cancel key was hit
    }
    else   // do periodic update
    {
        if (KeyHit())
        {
            if (GetKey() == '*')  GoToNextScreen(SCN_MISC_CONTROL_MENU);  
            if (GetKey() == '#')  GoToNextScreen(SCN_DATA_ENTRY);
        }
    }
}


PRIVATE  void  ScreenFunc_SystemInfoPage1(bool isNewScreen)
{
    char    textBuf[32];

    if (isNewScreen)  
    {
        LCD_Mode(SET_PIXELS);
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(0, 12);
        LCD_PutText("Bauer EWI Synth mk2");
        
        sprintf(textBuf, "Firmware: v%d.%d.%02d", g_FW_version[0], g_FW_version[1], 
                g_FW_version[2]);
        LCD_PosXY(0, 22);
        LCD_PutText(textBuf);
        
        LCD_SetFont(MONO_8_NORM);  // line 4 of 4
        LCD_PosXY(0, 44);
        LCD_PutText("   www.mjbauer.biz");
        
        LCD_PosXY(0, 53);
        LCD_DrawLineHoriz(128);
        DisplayMenuOption(0,  56, '*', "Home");
        DisplayMenuOption(88, 56, '#', "Page");
        LCD_PosXY(56, 56);
        LCD_PutText("-1-");
    }
    else  // do periodic update...
    {
        if (KeyHit())
        {
            if (GetKey() == '*')  GoToNextScreen(SCN_HOME);
            if (GetKey() == '#')  GoToNextScreen(SCN_SYSTEM_INFO_PAGE2);
        }
    }

    if (m_ElapsedTime_ms >= GUI_INACTIVE_TIMEOUT)
    {
        GoToNextScreen(SCN_HOME);
    }
}


PRIVATE  void  ScreenFunc_SystemInfoPage2(bool isNewScreen)
{
    char    textBuf[32];
    
    if (isNewScreen)  
    {
        LCD_Mode(SET_PIXELS);
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(0, 12);
        LCD_PutText("REMI Handset info: "); 
        
        if (isHandsetConnected() && g_HandsetInfo[0] == SYS_EXCLUSIVE_MSG)
        {
            sprintf(textBuf, "Firmware: v%d.%d.%02d", g_HandsetInfo[4], g_HandsetInfo[5], 
                    g_HandsetInfo[6]);
            LCD_PosXY(0, 22);
            LCD_PutText(textBuf);
            
            LCD_PosXY(0, 32);
            LCD_PutText("Legato mode is "); 
            if (g_HandsetInfo[9] != 0)  LCD_PutText("ON"); 
            else  LCD_PutText("OFF"); 
            
            LCD_PosXY(0, 42);
            LCD_PutText("Veloc. sense is "); 
            if (g_HandsetInfo[10] != 0)  LCD_PutText("ON"); 
            else  LCD_PutText("OFF"); 
        }
        else  LCD_PutText("N/A"); 
        
        LCD_PosXY(0, 53);
        LCD_DrawLineHoriz(128);
        DisplayMenuOption(0,  56, '*', "Home");
        DisplayMenuOption(88, 56, '#', "Page");
        LCD_PosXY(56, 56);
        LCD_PutText("-2-");
    }
    else  // do periodic update...
    {
        if (KeyHit())
        {
            if (GetKey() == '*')  GoToNextScreen(SCN_HOME);
            if (GetKey() == '#')  GoToNextScreen(SCN_SYSTEM_INFO_PAGE1);
        }
    }

    if (m_ElapsedTime_ms >= GUI_INACTIVE_TIMEOUT)
    {
        GoToNextScreen(SCN_HOME);
    }
}


PRIVATE  void  ScreenFunc_CustomFuncMenu(bool isNewScreen)
{
    if (isNewScreen)  // new screen...
    {
        DisplayMenuOption(8, 12, 'A', "Future option - TBD");
        DisplayMenuOption(8, 22, 'B', "  ..     ..");
        DisplayMenuOption(8, 32, 'C', "  ..     ..");
        DisplayMenuOption(8, 42, 'D', "  ..     ..");

        LCD_PosXY(0, 53);
        LCD_DrawLineHoriz(128);
        DisplayMenuOption(0,  56, '*', "Exit");
        DisplayMenuOption(88, 56, '#', "Next");
    }
    else  // check for button hit
    {
        if (KeyHit())
        {
            if (GetKey() == '*') GoToNextScreen(SCN_HOME);
            else if (GetKey() == 'A') GoToNextScreen(SCN_HOME);  // <<<<<<<<<<<<< todo >>>>>
            else if (GetKey() == 'B') GoToNextScreen(SCN_HOME);  // <<<<<<<<<<<<< todo >>>>>
            else if (GetKey() == 'C') GoToNextScreen(SCN_HOME);  // <<<<<<<<<<<<< todo >>>>>
            else if (GetKey() == 'D') GoToNextScreen(SCN_HOME);  // <<<<<<<<<<<<< todo >>>>>
            else if (GetKey() == '#') GoToNextScreen(SCN_HOME);  // <<<<<<<<<<<<< todo >>>>>
        }
    }

    if (m_ElapsedTime_ms >= GUI_INACTIVE_TIMEOUT)
    {
        GoToNextScreen(SCN_HOME);
    }
}


PRIVATE  void  ScreenFunc_DataEntry(bool isNewScreen)
{
    static  int     ipow10[] = { 1, 10, 100, 1000, 10000, 100000 };
    static  int     digitOrder;   // digit at edit cursor (0 => LSD)
    static  int     maxValue;     // absolute value
    static  int     initValue;    // initial value
    uint8   key;
    uint16  xpos;
    char    textBuf[40];
    char    numFormat[8];
    bool    do_refresh = 0;

    if (isNewScreen)  
    {
        LCD_Mode(SET_PIXELS);  // Draw title bar
        LCD_PosXY(0, 0);
        LCD_BlockFill(128, 10);
        LCD_Mode(CLEAR_PIXELS);
        LCD_SetFont(MONO_8_NORM);  // Title text
        LCD_PosXY(2, 1);
        if (m_DataEntryTitle != NULL)  LCD_PutText(m_DataEntryTitle);
        else  LCD_PutText("Enter value...");

        LCD_Mode(SET_PIXELS);
        LCD_PosXY(0, 40);
        LCD_DrawLineHoriz(128);
        DisplayMenuOption(0, 44, 'A', "+");
        DisplayMenuOption(24, 44, 'B', "-");
        DisplayMenuOption(48, 44, 'C', "Clear");
        DisplayMenuOption(90, 44, 'D', "Digit");
        DisplayMenuOption(0, 56, '*', "Cancel");
        DisplayMenuOption(83, 56, '#', "Accept");

        m_DataEntryAccept = 0;
        initValue = m_DataEntryValue;
        if (m_DataEntryNumDigits == 0) m_DataEntryNumDigits = 1;
        if (m_DataEntryNumDigits > 5)  m_DataEntryNumDigits = 5;
        maxValue = ipow10[m_DataEntryNumDigits] - 1;
        digitOrder = m_DataEntryNumDigits - 1;  // start editing with MSD
        do_refresh = 1;
    }
    else  // do periodic update...
    {
        if (KeyHit())
        {
            if ((key = GetKey()) == '*')  // Cancel data entry
            {
                m_DataEntryValue = initValue;  // restore
                m_DataEntryAccept = 0;
                GoToNextScreen(m_DataEntryNextScreen);
            }
            else if (key == '#')  // Accept value entered
            {
                m_DataEntryAccept = 1;
                GoToNextScreen(m_DataEntryNextScreen);
            }
            else if (key == 'A')  // Increment value
            {
                m_DataEntryValue += ipow10[digitOrder];
                if (m_DataEntryValue > maxValue)  m_DataEntryValue = maxValue;
                do_refresh = 1;
            }
            else if (key == 'B')  // Decrement value
            {
                m_DataEntryValue -= ipow10[digitOrder];
                if (m_DataEntryValue < (0 - maxValue))  m_DataEntryValue = 0 - maxValue;
                do_refresh = 1;
            }
            else if (key == 'C')  // Clear input field
            {
                m_DataEntryValue = 0;
                digitOrder = m_DataEntryNumDigits - 1;
                do_refresh = 1;
            }
            else if (key == 'D')  // Digit order rotation
            {
                if (digitOrder == 0)  // at LSD
                    digitOrder = m_DataEntryNumDigits - 1;  // wrap to MSD
                else  digitOrder--;   // Next LS digit (right)
                do_refresh = 1;
            }
        }
    }

    if (do_refresh)
    {
        LCD_Mode(CLEAR_PIXELS);   // clear data entry field
        LCD_PosXY(28, 12);
        LCD_BlockFill(82, 26);
        LCD_Mode(SET_PIXELS);
        LCD_SetFont(MONO_16_NORM);

        LCD_PosXY(28, 18);    // write minus sign if negative
        if (m_DataEntryValue < 0)  LCD_PutChar('-');

        strcpy(numFormat, "%0?d");
        numFormat[2] = '0' + m_DataEntryNumDigits;  // field width
        sprintf(textBuf, numFormat, abs(m_DataEntryValue));
        LCD_PosXY(40, 18);    // write (abs) edited data value
        LCD_PutText(textBuf);
//      LCD_PutDecimalWord( abs(m_DataEntryValue), m_DataEntryNumDigits );

        xpos = 29 + 12 * (m_DataEntryNumDigits - digitOrder);  
        LCD_PosXY(xpos, 12);
        LCD_PutImage(flat_up_arrow_8x4, 8, 4);  // draw cursor carats
        LCD_PosXY(xpos, 34);
        LCD_PutImage(flat_down_arrow_8x4, 8, 4);

        do_refresh = 0;   // done
    }
}


PRIVATE  void  ScreenFunc_DataEntryTest(bool isNewScreen)
{
    char    textBuf[32];

    if (isNewScreen)  
    {
        LCD_Mode(SET_PIXELS);
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(30, 22);
        LCD_PutText("Input value...");
        LCD_SetFont(PROP_8_NORM);

        if (m_DataEntryAccept)  sprintf(textBuf, "%8d", m_DataEntryValue);
        else  strcpy(textBuf, "N/A");

        LCD_SetFont(MONO_8_NORM);
        LCD_PosXY(30, 32);
        LCD_PutText(textBuf);

        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(0, 53);
        LCD_DrawLineHoriz(128);
        DisplayMenuOption(0, 56, '*', "Exit");
        DisplayMenuOption(80, 56, '#', "Retry");
    }
    else  // do periodic update...
    {
        if (KeyHit())
        {
            if (GetKey() == '*') GoToNextScreen(SCN_HOME);  else
            if (GetKey() == '#') GoToNextScreen(SCN_DATA_ENTRY);
        }
    }
}
