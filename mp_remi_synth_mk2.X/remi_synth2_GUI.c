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
PRIVATE  void  ScreenFunc_PresetEditMenu(bool);
PRIVATE  void  ScreenFunc_EditPresetPatch(bool);
PRIVATE  void  ScreenFunc_EditPresetMidiProgram(bool);
PRIVATE  void  ScreenFunc_EditPresetVibratoMode(bool);
PRIVATE  void  ScreenFunc_EditPresetTranspose(bool);

PRIVATE  void  ScreenFunc_MainSettingsMenu(bool isNewScreen);
PRIVATE  void  ScreenFunc_SetMidiInMode(bool);
PRIVATE  void  ScreenFunc_SetMidiInChannel(bool);
PRIVATE  void  ScreenFunc_SetMidiInExpression(bool);
PRIVATE  void  ScreenFunc_SetMidiOutEnable(bool);
PRIVATE  void  ScreenFunc_SetPitchBendMode(bool isNewScreen);
PRIVATE  void  ScreenFunc_SystemInfoPage1(bool);
PRIVATE  void  ScreenFunc_SystemInfoPage2(bool);
PRIVATE  void  ScreenFunc_ValidateReverbAtten(bool);
PRIVATE  void  ScreenFunc_ValidateReverbMix(bool);

PRIVATE  void  ScreenFunc_ControlPanel1(bool isNewScreen);
PRIVATE  void  ScreenFunc_ControlPanel2(bool isNewScreen);
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
        " SELF-TEST FAIL"             // title bar text
    },
    {
        SCN_HOME,
        ScreenFunc_Home,
        NULL                         // title bar text (none)
    },
    {
        SCN_MAIN_SETTINGS_MENU,
        ScreenFunc_MainSettingsMenu,
        " SETTINGS MENU"
    },
    {
        SCN_PRESET_EDIT_MENU,
        ScreenFunc_PresetEditMenu,
        " PRESET # PARAMs"
    },
    {
        SCN_SET_MIDI_IN_MODE,
        ScreenFunc_SetMidiInMode,
        " MIDI IN MODE"
    },
    {
        SCN_SET_MIDI_IN_CHANNEL,
        ScreenFunc_SetMidiInChannel,
        " MIDI IN CHANNEL"
    },
    {
        SCN_SET_MIDI_IN_EXPRESS,
        ScreenFunc_SetMidiInExpression,
        " EXPRESSION CC #"
    },
    {
        SCN_SET_MIDI_OUT_ENABLE,
        ScreenFunc_SetMidiOutEnable,
        " MIDI OUT ENABLE"
    },
    {
        SCN_SET_PITCH_BEND_MODE,
        ScreenFunc_SetPitchBendMode,
        " PITCH-BEND MODE"
    },
    {
        SCN_EDIT_PRESET_PATCH,
        ScreenFunc_EditPresetPatch,
        " PRESET PATCH"
    },
    {
        SCN_EDIT_PRESET_MIDI_PGRM,
        ScreenFunc_EditPresetMidiProgram, 
        " PRESET MIDI PGRM"
    },
    {
        SCN_EDIT_PRESET_VIBRATO,
        ScreenFunc_EditPresetVibratoMode,
        " VIBRATO MODE"
    },
    {
        SCN_EDIT_PRESET_TRANSPOSE,
        ScreenFunc_EditPresetTranspose,
        " PITCH TRANSPOSE"
    },
    {
        SCN_EDIT_REVERB_ATTEN,
        ScreenFunc_ValidateReverbAtten,
        " REVERB ATTEN"
    },
    {
        SCN_EDIT_REVERB_MIX,
        ScreenFunc_ValidateReverbMix,
        " REVERB MIX"
    },
    {
        SCN_SYSTEM_INFO_PAGE1,
        ScreenFunc_SystemInfoPage1,
        " System info  Page 1"
    },
    {
        SCN_SYSTEM_INFO_PAGE2,
        ScreenFunc_SystemInfoPage2,
        " System info  Page 2"
    },  
    {
        SCN_CONTROL_PANEL_1,
        ScreenFunc_ControlPanel1,
        " CONTROL PANEL 1"
    },  
    {
        SCN_CONTROL_PANEL_2,
        ScreenFunc_ControlPanel2,
        " CONTROL PANEL 2"
    },  
    {
        SCN_CUSTOM_FUNC_MENU,
        ScreenFunc_CustomFuncMenu,
        " OTHER FUNCTIONS"
    },  
    {
        SCN_DATA_ENTRY,
        ScreenFunc_DataEntry,
        NULL      // Title is variable, context-dependent
    },
    {
        SCN_DATA_ENTRY_TEST,
        ScreenFunc_DataEntryTest,
        " Data Entry Test"
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
 * GUI navigation engine (service routine, or whatever you want to call it).
 * This is the "executive hub" of the Graphical User Interface.
 *
 * The function is called frequently from the main application loop, but...
 * must not be called from BackgroundTaskExec() !!
 */
void  GUI_NavigationExec(void)
{
    static uint32 buttonScanPeriodStart;
    static bool  init_done;
    short  current, next;  // index values of current and next screens
    
    if (!isLCDModulePresent())  return;  // LCD not detected... bail
    
    if (!init_done)
    {
        m_CurrentScreen = SCN_STARTUP;
        m_PreviousScreen = SCN_STARTUP;
        m_DataEntryNextScreen = SCN_HOME;
        m_ScreenSwitchFlag = 1;
        m_screenSwitchDone = 0;
        m_lastUpdateTime = milliseconds();
        init_done = TRUE;
    }

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
        if ((milliseconds() - m_lastUpdateTime) >= SCREEN_UPDATE_INTERVAL)
        {
            current = ScreenDescIndexFind(m_CurrentScreen);

            (*m_ScreenDesc[current].ScreenFunc)(0);  // Update current screen

            m_lastUpdateTime = milliseconds();
            m_ElapsedTime_ms += SCREEN_UPDATE_INTERVAL;
        }
    }
    
    if (POT_MODULE_CONNECTED) ControlPotService();
    
    if ((milliseconds() - buttonScanPeriodStart) >= 6)  // every 6ms...
    {
        ButtonInputService();
        buttonScanPeriodStart = milliseconds();
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

/*
 * This function displays a text string (str) centred in a specified field width (nplaces)
 * using 8pt mono-spaced font, at the specified upper-left screen position (x, y).
 * On exit, the display write mode is restored to 'SET_PIXELS'.
 */
void  DisplayTextCenteredInField(uint16 x, uint16 y, char *str, uint8 nplaces)
{
    int  len = strlen(str);
    int  i;
    
    if (len > 20) len = 20;
    x += 3 * (nplaces - len);  
    
    LCD_SetFont(MONO_8_NORM);
    LCD_PosXY(x, y);
    
    for (i = 0;  i < len;  i++)
    {
        LCD_PutChar(*str++);
    }
    
    LCD_Mode(SET_PIXELS);
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
 * Function:   Quantizes an unsigned integer (max. 100,000) so that the output value is a 
 *             multiple of 1, 2 or 5. The multiple is a power of 10 which depends on the  
 *             magnitude of the input value. Thus, the output value is a member of the set:
 *             { 0, 1, 2, 5, 10, 20, 50, 100, ... 100000 }.
 *
 * Entry arg:  (uint16) inValue = 16-bit integer to be quantized
 *
 * Return val: (uint16) outValue = inValue, quantized in "1-2-5" series
 */
unsigned  QuantizeValue_1_2_5(unsigned inValue)
{
    unsigned  outValue = 0;
    unsigned  pow10 = 1;
    
    if (inValue < 3)  return inValue;
    
    while (pow10 <= 10000)
    {
        outValue = 2 * pow10;
        if (inValue <= (3 * pow10))  break;
        outValue = 5 * pow10;
        if (inValue <= (7 * pow10))  break;
        outValue = 10 * pow10;
        if (inValue <= (15 * pow10))  break;
        pow10 = pow10 * 10;
    }
    
    return  outValue;
}

/*
 * Function:   Quantizes an unsigned integer (max. 100,000) so that the output value is
 *             rounded to the most significant digit times a power of 10.  Examples:
 *             0 -> 0, 9 -> 9, 14 -> 10, 15 -> 20, 23 -> 20, 94 -> 90, 99 -> 100, 105 -> 110
 *
 * Entry arg:  (uint16) inValue = 16-bit integer to be quantized
 *
 * Return val: (uint16) outValue = inValue, quantized per decade
 */
unsigned  QuantizeValuePerDecade(unsigned inValue)
{
    unsigned  outValue = 0;
    unsigned  pow10 = 10;
    
    if (inValue < 10)  return inValue;
    
    while (pow10 <= 100000)
    {
        outValue = ((inValue + pow10 / 2) / pow10) * pow10;  // rounded
        pow10 = pow10 * 10;
        if (inValue < pow10)  break;
    }
    
    return  outValue;
}


/*-------------------------------------------------------------------------------------------------
 * Functions to support GUI input using 6 push-buttons on the front-panel.
 *-------------------------------------------------------------------------------------------------
 *
 * Function:  ButtonInputService()
 *
 * Overview:  Service Routine for 6 push-buttons on GUI front panel.
 *            Called periodically at 6 ms intervals (approx).
 *
 * Detail:    The routine reads 6 button inputs looking for a change in state.
 *            When a button "hit" is detected, the function sets a flag to register the event.
 *            The state of the flag can be read by a call to function ButtonHit().
 *            An ASCII key-code is stored to identify the button last pressed.
 *            The keycode can be read anytime by a call to function ButtonCode().
 */
void  ButtonInputService()
{
    static short taskState = 0; // startup/reset state
    static uint16 buttonStatesLastRead = 0;
    static int debounceTimer_ms;
    uint16 buttonStatesNow = READ_BUTTON_INPUTS() ^ 0x003F; // 6 LS bits, active HIGH
    
    debounceTimer_ms += 6;  // ms

    if (taskState == 0) // Waiting for all buttons released
    {
        if (buttonStatesNow == ALL_BUTTONS_RELEASED)
        {
            debounceTimer_ms = 0;
            taskState = 3;
        }
    }
    else if (taskState == 1) // Waiting for any button(s) pressed
    {
        if (buttonStatesNow != ALL_BUTTONS_RELEASED)
        {
            buttonStatesLastRead = buttonStatesNow;
            debounceTimer_ms = 0;
            taskState = 2;
        }
    }
    else if (taskState == 2) // De-bounce delay after hit (30ms)
    {
        if (buttonStatesNow != buttonStatesLastRead)
            taskState = 1; // glitch -- retry

        if (debounceTimer_ms >= 30)
        {
            m_ButtonHitDetected = 1;
            m_ButtonStates = buttonStatesNow;
            if (m_ButtonStates & MASK_BUTTON_A) m_ButtonLastHit = 'A';
            else if (m_ButtonStates & MASK_BUTTON_B) m_ButtonLastHit = 'B';
            else if (m_ButtonStates & MASK_BUTTON_C) m_ButtonLastHit = 'C';
            else if (m_ButtonStates & MASK_BUTTON_D) m_ButtonLastHit = 'D';
            else if (m_ButtonStates & MASK_BUTTON_STAR) m_ButtonLastHit = '*';
            else if (m_ButtonStates & MASK_BUTTON_HASH) m_ButtonLastHit = '#';
            else m_ButtonLastHit = 0; // NUL
            m_ElapsedTime_ms = 0; // reset screen timeout
            taskState = 0;
        }
    }
    else if (taskState == 3) // De-bounce delay after release (150ms)
    {
        if (buttonStatesNow != ALL_BUTTONS_RELEASED) // glitch - retry
            taskState = 0;

        if (debounceTimer_ms >= 150)
        {
            m_ButtonStates = buttonStatesNow;
            taskState = 1;
        }
    }
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
 * Function:     uint8 ButtonHit()
 *
 * Overview:     Tests for a button hit, i.e. transition from not pressed to pressed.
 *               A private flag, m_ButtonHitDetected, is cleared on exit so that the 
 *               function will return TRUE once only for each button press event.
 *
 * Return val:   TRUE (1) if a button hit was detected since the last call, else FALSE (0).
 */
uint8 ButtonHit(void)
{
    uint8 result = m_ButtonHitDetected;

    m_ButtonHitDetected = 0;

    return result;  
}


/*
 * Function:     short ButtonCode()
 *
 * Overview:     Returns the ASCII keycode of the last button press detected, i.e.
 *               following a call to function ButtonHit() which returned TRUE.
 *
 * Note:         ButtonCode() may be called multiple times after a call to ButtonHit().
 *               The function will return the same keycode on each call, up to
 *               100 milliseconds after the last button hit registered.
 *
 * Return val:   (uint8) ASCII keycode, one of: 'A', 'B', 'C', 'D', '*', or '#'.
 */
uint8  ButtonCode(void)
{
    return  m_ButtonLastHit;
}


//-------------------------------------------------------------------------------------------------
// Functions to support up to 6 control potentiometers on the front-panel.
//-------------------------------------------------------------------------------------------------

#define SIGNAL_ACQUISITION      0
#define WAITING_FOR_CONVERSION  1

static  int32  m_PotReadingAve[6];  // rolling average of pot readings
static  bool   m_PotMoved[6];  // Flags: Pot reading changed since last read

/*
 * Function:  ControlPotService()
 *
 * Overview:  Service Routine for 6 front-panel control pots.
 *            Non-blocking "task" called frequently as possible.
 *
 * Detail:    The routine reads the 6 pot inputs and keeps a rolling average of raw ADC
 *            readings in fixed-point format (24:8 bits). The acquisition time allowed for 
 *            each pot reading is 3ms, so all 6 pots are serviced in under 20ms.
 * 
 *            Each pot reading is compared with its respective reading on the previous pass.
 *            If a change of more than about 1% is found, then a flag is raised for the
 *            respective pot.
 * 
 * Outputs:   (bool) m_PotMoved[6],  (int32) m_PotReadingAve[6].
 *            The state of the flag can be read by a call to function PotMoved(potnum).
 *            The last pot position can be read by a call to function PotReading(n).
 */
void  ControlPotService()
{
    static bool    prep_done;
    static uint8   state;
    static int32   pastReading[6];  // readings on past scan
    static uint32  acquisitionStartTime; // start of 3ms interval
    static uint32  intervalStart37ms;    // start of 37ms interval
    static uint8   potSel;  // pot index, also current ADC input selected
    int32  potReading;
    uint8  potr;
    
    if (!prep_done)  // One-time initialization at power-on/reset
    {
        potSel = 0;
        ADC_INPUT_SEL(0);
        ADC_SAMPLING = 1;  // Start signal acquisition
        acquisitionStartTime = milliseconds();
        intervalStart37ms = milliseconds();
        state = SIGNAL_ACQUISITION;
        prep_done = TRUE;
    }

    if (state == SIGNAL_ACQUISITION)
    {
        if ((milliseconds() - acquisitionStartTime) >= 3)  // 3ms interval
        {
            ADC_SAMPLING = 0;  // End sampling, start conversion
            state = WAITING_FOR_CONVERSION;
        }
    }
    else  // state == WAITING_FOR_CONVERSION
    {
        if (ADC_CONV_DONE)  // Every 3ms (approx)...
        {
            potReading = (int32) ADC_RESULT_REG;  // get 10 bit raw result
            potReading = potReading << 8;  // convert to fixed-point (24:8 bits)
            
            // Apply rolling average algorithm (1st-order IIR filter, K = 0.25)
            m_PotReadingAve[potSel] -= m_PotReadingAve[potSel] >> 2;
            m_PotReadingAve[potSel] += potReading >> 2;
            
            if (++potSel >= 6)  potSel = 0;  // next pot to be serviced
            ADC_INPUT_SEL(potSel);
            ADC_SAMPLING = 1;  // Start signal acquisition
            acquisitionStartTime = milliseconds();
            state = SIGNAL_ACQUISITION;
        }
    }
    
    // Every 37ms, choose a pot at random, check if it has been moved.
    // On average, 6 pots will be checked in 220ms (approx).
    if ((milliseconds() - intervalStart37ms) >= 37)
    {
        potr = rand() % 6;  // 0..5
        if (abs(m_PotReadingAve[potr] - pastReading[potr]) > (10 << 8))
        {
            m_PotMoved[potr] = TRUE;
            pastReading[potr] = m_PotReadingAve[potr];  // update
        }
        intervalStart37ms = milliseconds();
    }
}

/*
 * Function:     PotFlagsClear()
 *
 * Overview:     Clears all (6) "pot moved" flags in array m_PotMoved[].
 * 
 * Note:         To clear one individual pot flag, call PotMoved(num).
 */
void  PotFlagsClear()
{
    short  i;
    
    for (i = 0;  i < 6;  i++)  { m_PotMoved[i] = FALSE; }
}

/*
 * Function:     PotMoved()
 *
 * Overview:     Returns a flag (TRUE or FALSE) indicating if the specified control pot
 *               (potnum) position has changed since the previous call to the function.
 *
 * Note:         The flag is cleared on exit, so subsequent calls will return FALSE
 *               until the pot position changes again.
 *
 * Arg val:      (uint8) ID number of required pot (0..5)
 *
 * Return val:   (bool) status flag, value = TRUE or FALSE
 */
bool  PotMoved(uint8 potnum)
{
    bool  result = m_PotMoved[potnum];

    if (potnum < 6) m_PotMoved[potnum] = FALSE;
    
    return  result;
}

/*
 * Function:     PotReading()
 *
 * Overview:     Returns the current setting (position) of the specified control pot,
 *               averaged over several ADC readings, as an 8-bit integer.
 *
 * Arg val:      (uint8) ID number of required pot (0..5)
 *
 * Return val:   (uint8) Pot reading, 8 bits unsigned, range 0..255.
 */
uint8  PotReading(uint8 potnum)
{
    return  (uint8) (m_PotReadingAve[potnum] >> 10);
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
        
        CLIPPING_LED_ON();
    }
    else  // do periodic update...
    {
        if (m_ElapsedTime_ms >= SELF_TEST_WAIT_TIME_MS)
        {
            CLIPPING_LED_OFF();
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
        if (ButtonHit())
        {
            if (ButtonCode() == '*') BootReset();
            else if (ButtonCode() == '#') GoToNextScreen(SCN_HOME);
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

        DisplayMenuOption(0,  46, 'A', "+ Pr -");  // A: Preset++
        DisplayMenuOption(44, 46, 'B', "");        // B: Preset--
        DisplayMenuOption(60, 46, 'C', "Control");
        DisplayMenuOption(0,  56, '*', "SETUP");
        DisplayMenuOption(60, 56, '#', "PRESET");

        LCD_PosXY(26, 1);
        LCD_SetFont(PROP_8_NORM);
        LCD_PutText("Bauer Synth");

        sprintf(textBuf, "v%d.%d.%02d", g_FW_version[0], g_FW_version[1], g_FW_version[2]);
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(90, 1);
        LCD_PutText(textBuf);
        
        m_presetLastShown = 999;        // force Preset # display update
        m_HandsetDetectedLast = FALSE;  // force handset status update
    }
    else  // do periodic update...
    {
        if (ButtonHit())
        {
            // Main menu options...
            if (ButtonCode() == '*')  GoToNextScreen(SCN_MAIN_SETTINGS_MENU);  // SETUP
            if (ButtonCode() == '#')  GoToNextScreen(SCN_PRESET_EDIT_MENU);  // PRESET
            if (ButtonCode() == 'A')  // Increment Preset
            {
                preset = (g_Config.PresetLastSelected + 1) & 7;
                InstrumentPresetSelect(preset);
            }
            if (ButtonCode() == 'B')  // Decrement Preset
            {
                preset = (g_Config.PresetLastSelected - 1) & 7;
                InstrumentPresetSelect(preset);
            }
            if (ButtonCode() == 'C')  GoToNextScreen(SCN_CONTROL_PANEL_1);
            if (ButtonCode() == 'D')  GoToNextScreen(SCN_CUSTOM_FUNC_MENU);
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
        if (ButtonHit())
        {
            if (ButtonCode() == '*') GoToNextScreen(SCN_HOME);
            if (ButtonCode() == 'A')  GoToNextScreen(SCN_EDIT_PRESET_PATCH);
            if (ButtonCode() == 'B')  // Edit Preset Midi Pgrm
            {
                m_DataEntryTitle = "Enter MIDI Pgrm #";
                m_DataEntryNextScreen = SCN_EDIT_PRESET_MIDI_PGRM;
                m_DataEntryNumDigits = 3;
                m_DataEntryValue = g_Preset.Descr[m_editPreset].MidiProgram;
                GoToNextScreen(SCN_DATA_ENTRY);
            }
            if (ButtonCode() == 'C')  GoToNextScreen(SCN_EDIT_PRESET_VIBRATO);
            if (ButtonCode() == 'D')  // Edit Preset Transpose
            {
                m_DataEntryTitle = "Enter Transpose Qty";
                m_DataEntryNextScreen = SCN_EDIT_PRESET_TRANSPOSE;
                m_DataEntryNumDigits = 2;
                m_DataEntryValue = g_Preset.Descr[m_editPreset].PitchTranspose;
                GoToNextScreen(SCN_DATA_ENTRY);
            }
            if (ButtonCode() == '#')  // select next preset to edit
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
        patchNum = g_Preset.Descr[m_editPreset].PatchNumber;  // Patch ID #

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

    if (m_ElapsedTime_ms >= GUI_INACTIVE_TIMEOUT) GoToNextScreen(SCN_HOME);
}


PRIVATE  void  ScreenFunc_MainSettingsMenu(bool isNewScreen)
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

//      DisplayMenuOption(0,  32, 'C', " - TBD - ");  // reserved

        DisplayMenuOption(0,  42, 'D', "");
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(12, 42);
        LCD_PutText("Display brightness");
     
        LCD_PosXY(0, 53);
        LCD_DrawLineHoriz(128);
        DisplayMenuOption(0,  56, '*', "Exit");
        DisplayMenuOption(88, 56, '#', "Next");
        LCD_PosXY(56, 56);
    }
    else  // check for button hit
    {
        if (ButtonHit())
        {
            if (ButtonCode() == '*') GoToNextScreen(SCN_HOME);
            if (ButtonCode() == 'A') 
            {
                m_DataEntryTitle = "Enter 0..99 (%)";
                m_DataEntryNextScreen = SCN_EDIT_REVERB_ATTEN;
                m_DataEntryNumDigits = 2;
                m_DataEntryValue = g_Config.ReverbAtten_pc;
                GoToNextScreen(SCN_DATA_ENTRY);
            }
            if (ButtonCode() == 'B')
            {
                m_DataEntryTitle = "Enter 0..99 (%)";
                m_DataEntryNextScreen = SCN_EDIT_REVERB_MIX;
                m_DataEntryNumDigits = 2;
                m_DataEntryValue = g_Config.ReverbMix_pc;
                GoToNextScreen(SCN_DATA_ENTRY);
            }
//          if (ButtonCode() == 'C')  GoToNextScreen(SCN_HOME);  // reserved
            if (ButtonCode() == 'D')  
            {
                if (LCD_BACKLIGHT_IS_LOW)  LCD_BACKLIGHT_HIGH();
                else  LCD_BACKLIGHT_LOW();   // Toggle LCD backlight switch
                GoToNextScreen(SCN_MAIN_SETTINGS_MENU);  // Update display
            }
            if (ButtonCode() == '#') GoToNextScreen(SCN_SET_MIDI_IN_MODE);
        }
    }
    if (m_ElapsedTime_ms >= GUI_INACTIVE_TIMEOUT) GoToNextScreen(SCN_HOME);
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
        DisplayMenuOption(90, 56, '#', "Next");
        doRefresh = 1;
    }
    else  // check for button press, update screen
    {
        if (ButtonHit())
        {
            if (ButtonCode() == 'C')  // change mode
            {
                if (mode == 2)  mode = 4;  else  mode = 2;
                g_Config.MidiInMode = mode;
                StoreConfigData();
            }
            if (ButtonCode() == '*')  GoToNextScreen(SCN_HOME);
            if (ButtonCode() == '#')  GoToNextScreen(SCN_SET_MIDI_IN_CHANNEL);
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
    if (m_ElapsedTime_ms >= GUI_INACTIVE_TIMEOUT) GoToNextScreen(SCN_HOME);
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
        DisplayMenuOption(88, 56, '#', "Next");
        doRefresh = 1;
    }
    else  // check for button press, update screen
    {
        if (ButtonHit())
        {
            if (ButtonCode() == '*')  
            {
                g_Config.MidiInChannel = channel;
                StoreConfigData();
                GoToNextScreen(SCN_HOME);
            }
            if (ButtonCode() == '#')
            {
                g_Config.MidiInChannel = channel;
                StoreConfigData();
                GoToNextScreen(SCN_SET_MIDI_IN_EXPRESS);
            }
            if (ButtonCode() == 'A')
            {
                channel += 4;
                channel = channel % 16;
                if (channel == 0) channel = 16;
            }
            if (ButtonCode() == 'B')
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
    if (m_ElapsedTime_ms >= GUI_INACTIVE_TIMEOUT) GoToNextScreen(SCN_HOME);
}


PRIVATE  void  ScreenFunc_SetMidiInExpression(bool isNewScreen)
{
    bool   doRefresh = 0;
    uint8  ccNumber = g_Config.MidiInExpressionCCnum;   // can be: 0, 2, 7 or 11

    if (isNewScreen)  // new screen
    {
        LCD_Mode(SET_PIXELS);
        LCD_PosXY(0, 53);
        LCD_DrawLineHoriz(128);
        LCD_SetFont(PROP_8_NORM);
        DisplayMenuOption( 0, 56, '*', "Home");
        DisplayMenuOption(40, 56, 'C', "Change");
        DisplayMenuOption(90, 56, '#', "Next");
        doRefresh = 1;
    }
    else  // check for button press, update screen
    {
        if (ButtonHit())
        {
            if (ButtonCode() == 'C')  // change CC number
            {
                if (ccNumber == 0)  ccNumber = 2;  
                else if (ccNumber == 2)  ccNumber = 7;  
                else if (ccNumber == 7)  ccNumber = 11;
                else  ccNumber = 0;
                g_Config.MidiInExpressionCCnum = ccNumber;
                StoreConfigData();
            }
            if (ButtonCode() == '*')  GoToNextScreen(SCN_HOME);
            if (ButtonCode() == '#')  GoToNextScreen(SCN_SET_MIDI_OUT_ENABLE);
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
        LCD_PutText("Expression CC #: ");
        
        if (ccNumber == 0)  
        {
            LCD_PutText("0");
            LCD_PosXY(20, 32);
            LCD_PutText("Not recognised");
        }
        else if (ccNumber == 2)  
        {
            LCD_PutText("02");
            LCD_PosXY(20, 32);
            LCD_PutText("Breath pressure");
        }
        else if (ccNumber == 7)  
        {
            LCD_PutText("07");
            LCD_PosXY(20, 32);
            LCD_PutText("Channel volume");
        }
        else if (ccNumber == 11)  
        {
            LCD_PutText("11");
            LCD_PosXY(20, 32);
            LCD_PutText("Expression");
        }
        else  LCD_PutText("?");  // unlikely!
        
        doRefresh = 0;   // inhibit refresh until next key hit
    }
    if (m_ElapsedTime_ms >= GUI_INACTIVE_TIMEOUT) GoToNextScreen(SCN_HOME);
}


PRIVATE  void  ScreenFunc_SetMidiOutEnable(bool isNewScreen)
{
    bool   doRefresh = 0;
    uint8  setting = g_Config.MidiOutEnabled;  // 0 or 1

    if (isNewScreen)  // new screen
    {
        LCD_Mode(SET_PIXELS);
        LCD_PosXY(0, 53);
        LCD_DrawLineHoriz(128);
        LCD_SetFont(PROP_8_NORM);
        DisplayMenuOption( 0, 56, '*', "Home");
        DisplayMenuOption(40, 56, 'C', "Change");
        DisplayMenuOption(90, 56, '#', "Next");
        doRefresh = 1;
    }
    else  // check for button press, update screen
    {
        if (ButtonHit())
        {
            if (ButtonCode() == 'C')  // change setting
            {
                if (setting == 0)  setting = 1;  else  setting = 0;  // toggle
                g_Config.MidiOutEnabled = setting;
                StoreConfigData();
            }
            if (ButtonCode() == '*')  GoToNextScreen(SCN_HOME);
            if (ButtonCode() == '#')  GoToNextScreen(SCN_SET_PITCH_BEND_MODE);
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
        LCD_PutDigit(setting);
        LCD_PosXY(16, 32);
        if (setting == 0)  LCD_PutText("(Disabled)");
        else  LCD_PutText("(Enabled)");
        doRefresh = 0;   // inhibit refresh until next key hit
    }
    if (m_ElapsedTime_ms >= GUI_INACTIVE_TIMEOUT) GoToNextScreen(SCN_HOME);
}


PRIVATE  void  ScreenFunc_SetPitchBendMode(bool isNewScreen)
{
    static char *pitchBendModeName[] = 
            { "Disabled", "MIDI PB msg", "Expr'n (CC2)", "(N/A)" };
    bool   doRefresh = 0;
    uint8  ctrlMode = g_Config.PitchBendCtrlMode;   // may be: 0, 1, 2, or 3

    if (isNewScreen)  // new screen
    {
        LCD_Mode(SET_PIXELS);
        LCD_PosXY(0, 53);
        LCD_DrawLineHoriz(128);
        LCD_SetFont(PROP_8_NORM);
        DisplayMenuOption( 0, 56, '*', "Home");
        DisplayMenuOption(40, 56, 'C', "Change");
        DisplayMenuOption(90, 56, '#', "Next");
        doRefresh = 1;
    }
    else  // check for button press, update screen
    {
        if (ButtonHit())
        {
            if (ButtonCode() == 'C')  // change mode
            {
                if (++ctrlMode >= 4)  ctrlMode = 0;  
                g_Config.PitchBendCtrlMode = ctrlMode;
                StoreConfigData();
            }
            if (ButtonCode() == '*')  GoToNextScreen(SCN_HOME);
            if (ButtonCode() == '#')  GoToNextScreen(SCN_SYSTEM_INFO_PAGE1);
            doRefresh = 1;
        }
    }

    if (doRefresh)
    {
        LCD_Mode(CLEAR_PIXELS);  // erase existing info
        LCD_PosXY(0, 22);
        LCD_BlockFill(128, 20);
        
        LCD_Mode(SET_PIXELS);
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(4, 22);
        LCD_PutText("Pitch-Bend Control: ");
        LCD_SetFont(MONO_8_NORM);
        LCD_PutDigit(ctrlMode & 3);
        LCD_PosXY(20, 32);
        LCD_PutText(pitchBendModeName[ctrlMode & 3]);
        doRefresh = 0;   // inhibit refresh until next key hit
    }
    
    if (m_ElapsedTime_ms >= GUI_INACTIVE_TIMEOUT) GoToNextScreen(SCN_HOME);
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
        if (ButtonHit())
        {
            if (ButtonCode() == '*')  GoToNextScreen(SCN_PRESET_EDIT_MENU);  // exit
            if (ButtonCode() == '#')    // next page
            {
                itop = itop + 4;
                if (itop >= GetNumberOfPatchesDefined())  itop = 0;  // wrap
                doRefresh = 1;
            }
            if (ButtonCode() >= 'A')    // A, B, C or D
            {
                line = ButtonCode() - 'A';   // 0, 1, 2 or 3
                selectedPatchID = g_PatchProgram[itop+line].PatchNumber;
                g_Preset.Descr[m_editPreset].PatchNumber = selectedPatchID;
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
        if (ButtonHit())
        {
            if (ButtonCode() == '*')  GoToNextScreen(SCN_PRESET_EDIT_MENU);  
            if (ButtonCode() == '#')  GoToNextScreen(SCN_DATA_ENTRY);
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
        if (ButtonHit())
        {
            if (ButtonCode() == '*')  GoToNextScreen(SCN_HOME);  else
            if (ButtonCode() == '#')  GoToNextScreen(SCN_PRESET_EDIT_MENU);  else
            if (ButtonCode() == 'C')  // Change mode
            {
                if (vib_mode == VIBRATO_DISABLED) vib_mode = VIBRATO_BY_MODN_CC;
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

        if (vib_mode == VIBRATO_BY_MODN_CC)  LCD_PutText("Modulation (CC1)");
        else if (vib_mode == VIBRATO_AUTOMATIC)  LCD_PutText("Automatic (ramp)");
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
        if (ButtonHit())
        {
            if (ButtonCode() == '*') GoToNextScreen(SCN_PRESET_EDIT_MENU);
            if (ButtonCode() == '#') GoToNextScreen(SCN_DATA_ENTRY);
        }
    }
}

/*
 * This function validates the number which was entered in the Data Entry Screen.
 * If validated, the value is saved in the Config. structure in EEPROM.
 */
PRIVATE  void  ScreenFunc_ValidateReverbAtten(bool isNewScreen)
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
                SynthPrepare();   // Instate new value
                GoToNextScreen(SCN_MAIN_SETTINGS_MENU);
            }
        }
        else  GoToNextScreen(SCN_MAIN_SETTINGS_MENU);  // Cancel key was hit
    }
    else   // do periodic update
    {
        if (ButtonHit())
        {
            if (ButtonCode() == '*')  GoToNextScreen(SCN_MAIN_SETTINGS_MENU);  
            if (ButtonCode() == '#')  GoToNextScreen(SCN_DATA_ENTRY);
        }
    }
}


PRIVATE  void  ScreenFunc_ValidateReverbMix(bool isNewScreen)
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
                SynthPrepare();   // Instate new value
                GoToNextScreen(SCN_MAIN_SETTINGS_MENU);
            }
        }
        else  GoToNextScreen(SCN_MAIN_SETTINGS_MENU);  // Cancel key was hit
    }
    else   // do periodic update
    {
        if (ButtonHit())
        {
            if (ButtonCode() == '*')  GoToNextScreen(SCN_MAIN_SETTINGS_MENU);  
            if (ButtonCode() == '#')  GoToNextScreen(SCN_DATA_ENTRY);
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
        LCD_PutText("Bauer REMI Synth mk2");
        
        sprintf(textBuf, "Firmware: v%d.%d.%02d", g_FW_version[0], g_FW_version[1], 
                g_FW_version[2]);
        LCD_PosXY(0, 22);
        LCD_PutText(textBuf);
        
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(0, 32);
        if (POT_MODULE_CONNECTED)  LCD_PutText("Pot. module connected");
        else  LCD_PutText("Pot. module not found");
        
        LCD_SetFont(MONO_8_NORM);  // line 4 of 4
        LCD_PosXY(0, 44);
        LCD_PutText("   www.mjbauer.biz");
        
        LCD_PosXY(0, 53);
        LCD_DrawLineHoriz(128);
        DisplayMenuOption(0,  56, '*', "Home");
        DisplayMenuOption(88, 56, '#', "Next");
        LCD_PosXY(56, 56);
        LCD_PutText("-1-");
    }
    else  // do periodic update...
    {
        if (ButtonHit())
        {
            if (ButtonCode() == '*')  GoToNextScreen(SCN_HOME);
            if (ButtonCode() == '#')  GoToNextScreen(SCN_SYSTEM_INFO_PAGE2);
        }
    }

    if (m_ElapsedTime_ms >= GUI_INACTIVE_TIMEOUT)  GoToNextScreen(SCN_HOME);
}


PRIVATE  void  ScreenFunc_SystemInfoPage2(bool isNewScreen)
{
    char    textBuf[32];
    
    if (isNewScreen)  
    {
        LCD_Mode(SET_PIXELS);
        LCD_SetFont(PROP_8_NORM);
        LCD_PosXY(0, 12);
        LCD_PutText("REMI Handset info "); 
        
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
        DisplayMenuOption(88, 56, '#', "Back");
        LCD_PosXY(56, 56);
        LCD_PutText("-2-");
    }
    else  // do periodic update...
    {
        if (ButtonHit())
        {
            if (ButtonCode() == '*')  GoToNextScreen(SCN_HOME);
            if (ButtonCode() == '#')  GoToNextScreen(SCN_MAIN_SETTINGS_MENU);
        }
    }

    if (m_ElapsedTime_ms >= GUI_INACTIVE_TIMEOUT)  GoToNextScreen(SCN_HOME);
}


PRIVATE  void  ScreenFunc_ControlPanel1(bool isNewScreen)
{
    static char *potLabel[] = { "Detune", "Osc2mix", "Filt Fc", "Contour", "Time ms", "Filt Res" };
    static bool  doRefresh[6];
    char   textBuf[40];
    int    pot, setting;
    uint16 xpos, ypos;  // display coords 
    
    if (isNewScreen)  // new screen...
    {
        LCD_Mode(SET_PIXELS);
        LCD_SetFont(PROP_8_NORM); 
        
        for (pot = 0; pot < 6; pot++)  
        {
            xpos = (pot % 3) * 43 + 2;
            ypos = (pot < 3) ? 12 : 34;
            LCD_PosXY(xpos, ypos);
            LCD_PutText(potLabel[pot]);
            xpos = (pot % 3) * 43 + 1;
            ypos = (pot < 3) ? 20 : 42;
            LCD_PosXY(xpos, ypos);
            LCD_BlockFill(40, 11);
            doRefresh[pot] = TRUE;
        }
        DisplayMenuOption(4,  56, '*', "Exit");
        DisplayMenuOption(92, 56, '#', "Next");
        PotFlagsClear();
    }
    else  // check for button hit or any pot position changed
    {
        if (ButtonHit())
        {
            if (ButtonCode() == '*')  GoToNextScreen(SCN_HOME);
            if (ButtonCode() == '#')  GoToNextScreen(SCN_CONTROL_PANEL_2);
            if (ButtonCode() == 'A')  // Activate new setting(s) and refresh display
            {
                SynthPrepare();  
                memset(doRefresh, TRUE, 6);
                LCD_PosXY(44, 55);  // Erase 'Assert' menu option
                LCD_Mode(CLEAR_PIXELS);
                LCD_BlockFill(46, 9);
            }
        }
        
        if (PotMoved(0))  // OSC2 Detune
        {
            setting = (int) PotReading(0) - 128;  // bipolar setting -128..+127
            setting = (setting * setting * 100) / (127 * 127);  // square-law curve
            if (PotReading(0) < 128)  setting = 0 - setting;  // negate
            g_Patch.Osc2Detune = (int16) setting;  // range 0..+/-100 (cents))
            doRefresh[0] = TRUE;
        }
        if (PotMoved(1))  // OSC2 Mix Level (0..100 %)
        {
            if (g_Patch.MixerControl == MIXER_CTRL_FIXED)
            {
                setting = (int) PotReading(1); 
                setting = (setting * 100) / 255;    // range 0..100
                g_Patch.MixerOsc2Level = (uint8) setting;
                doRefresh[1] = TRUE;
            }
        }
        if (PotMoved(2))  // Filter Frequency (offset), semitones
        {
            if (g_Patch.FilterResonance != 0 
            || (g_Patch.NoiseMode && !(g_Patch.NoiseMode & NOISE_PITCHED)))
            {
                setting = (int) PotReading(2);  
                setting = (setting * 108) / 255;  // range 0..108
                g_Patch.FilterFrequency = (uint8) setting;
                doRefresh[2] = TRUE;
                doRefresh[5] = TRUE;
            }
        }
        if (PotMoved(3))  // Contour profile -- ramp slope (+ or -), bias +50
        {
            setting = (int) PotReading(3) - 128;  // bipolar setting -128..+127
            setting = (setting * 50) / 127;  // range 0..+/-50
            setting = (setting / 5) * 5;   // nearest multiple of 5
            g_Patch.ContourStartLevel = 50 - setting;  // setting is 0..+/-50 units
            g_Patch.ContourHoldLevel = 50 + setting;
            doRefresh[3] = TRUE;
        }
        if (PotMoved(4))  // Contour Ramp Time (ms)
        {
            setting = (int) PotReading(4);  // unipolar setting 0..255
            setting = (setting * setting * 2000) / (255 * 255);  // square-law curve
            setting = QuantizeValuePerDecade(setting);  // range 0..2000
            if (setting < 10)  setting = 0;  // floor at 10, but allow 0
            g_Patch.ContourRamp_ms = setting;
            doRefresh[4] = TRUE;
        }
        if (PotMoved(5))  // Filter Resonance (Q), display normalised value
        {
            setting = (int) PotReading(5);  // unipolar setting 0..255
            if (setting != 0) 
            {
                if (PotReading(5) < 128)  // Below half-way mark
                {
                    setting = (setting * 8000) / 127 + 1000;  // range 1000..9000
                    setting = (setting / 100) * 100;  // quantize into 100's
                }
                else  // Above half-way mark
                {
                    setting = ((setting - 128) * 1000) / 127 + 9000;  // range 9000..10K
                    setting = (setting / 10) * 10;  // quantize into 10's
                }
                if (setting > 9990)  setting = 9990;  // cap at 9990
            }
            g_Patch.FilterResonance = (uint16) setting;  // range 0 | 1000..9990
            
            DisplayMenuOption(44, 56, 'A', "Assert");  // Assert required
            doRefresh[2] = TRUE;
            doRefresh[5] = TRUE;
        }
    }
    
    // Update variable data displayed, if changed or isNewScreen
    for (pot = 0; pot < 6; pot++)  
    {
        if (doRefresh[pot])
        {
            if (pot == 0)  sprintf(textBuf, "%+3d", (int)g_Patch.Osc2Detune);
            if (pot == 1)  
            {
                if (g_Patch.MixerControl == MIXER_CTRL_FIXED)
                    sprintf(textBuf, "%3d%c", (int)g_Patch.MixerOsc2Level, '%');
                else  strcpy(textBuf, "-");
            }
            if (pot == 2)  
            {
                if (g_Patch.FilterResonance != 0 
                || (g_Patch.NoiseMode && !(g_Patch.NoiseMode & NOISE_PITCHED)))
                    sprintf(textBuf, "%3d", (int)g_Patch.FilterFrequency);
                else  strcpy(textBuf, "-");
            }
            if (pot == 3)  
            {
                // Contour ramp is symmetrical about 50%
                setting = (int) g_Patch.ContourHoldLevel - (int) g_Patch.ContourStartLevel;
                sprintf(textBuf, "%+3d%c", setting, '%');  // show slope as %FS
            }
            if (pot == 4)  sprintf(textBuf, "%4d", (int) g_Patch.ContourRamp_ms);
            if (pot == 5)  
            {
                if (g_Patch.FilterResonance)
                    sprintf(textBuf, ".%03d", (int)g_Patch.FilterResonance/10);
                else  strcpy(textBuf, "Off");
            }

            xpos = (pot % 3) * 43 + 3;
            ypos = (pot < 3) ? 22 : 44;
            LCD_PosXY(xpos, ypos);
            LCD_Mode(SET_PIXELS);  // Erase existing data
            LCD_BlockFill(36, 8);
            LCD_Mode(CLEAR_PIXELS);  // Write new data
            DisplayTextCenteredInField(xpos, ypos, textBuf, 6);
            doRefresh[pot] = FALSE;
        }
    }
}


PRIVATE  void  ScreenFunc_ControlPanel2(bool isNewScreen)
{
    static char  *potLabel[] = { "Filt mod", "Pot 2", "Pot 3", "Noise G", "Flt Atn", "Filt G" };
    static float optionAtten[] = { 0.1, 0.15, 0.2, 0.25, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0 };
    static float optionGain[] = { 0.1, 0.2, 0.5, 1.0, 1.5, 2.0, 2.5, 4.0, 5.0, 10.0, 20.0, 25.0 };
    static bool  doRefresh[6];
    char   textBuf[40];
    int    pot, setting;
    uint16 xpos, ypos;  // display coords 
    
    if (isNewScreen)  // new screen...
    {
        LCD_Mode(SET_PIXELS);
        LCD_SetFont(PROP_8_NORM); 
        
        for (pot = 0; pot < 6; pot++)  
        {
            xpos = (pot % 3) * 43 + 2;
            ypos = (pot < 3) ? 12 : 34;
            LCD_PosXY(xpos, ypos);
            LCD_PutText(potLabel[pot]);
            xpos = (pot % 3) * 43 + 1;
            ypos = (pot < 3) ? 20 : 42;
            LCD_PosXY(xpos, ypos);
            LCD_BlockFill(40, 11);
            doRefresh[pot] = TRUE;
        }
        DisplayMenuOption(4,  56, '*', "Exit");
//      DisplayMenuOption(44, 56, 'A', "Assert");  // Nothing to assert (yet)
        DisplayMenuOption(92, 56, '#', "Back");
        PotFlagsClear();
    }
    else  // check for button hit or any pot position changed
    {
        if (ButtonHit())
        {
            if (ButtonCode() == '*') GoToNextScreen(SCN_HOME);
            if (ButtonCode() == '#') GoToNextScreen(SCN_CONTROL_PANEL_1);
//          if (ButtonCode() == 'A') SynthPrepare();  // Activate new setting(s)
        }
        
        if (PotMoved(0))  // Modify filter freq. while note in progress
        {
            setting = (int) PotReading(0);  
            setting = (setting * 108) / 255;  // range 0..108
            SetFilterFreqIndex(setting);  
            doRefresh[0] = TRUE;
        }
        if (PotMoved(1))  //  TBD
        {
            //
            doRefresh[1] = TRUE;
        }
        if (PotMoved(2))  //  TBD
        {
            //
            doRefresh[2] = TRUE;
        }
        if (PotMoved(3))  // Noise Filter output Gain
        {
            setting = ((int) PotReading(3) * 3) / 64;  // one of 12 options
            if (setting > 11)  setting = 11;
            g_NoiseFilterGain = optionGain[setting];
            doRefresh[3] = TRUE;
        }
        if (PotMoved(4))  // Filter input Attenuation (gain)
        {
            setting = ((int) PotReading(4) * 3) / 64;  // one of 12 options
            if (setting > 11)  setting = 11;
            g_FilterInputAtten = optionAtten[setting];
            doRefresh[4] = TRUE;
        }
        if (PotMoved(5))  // Wave Filter output Gain
        {
            setting = ((int) PotReading(5) * 3) / 64;  // one of 12 options
            g_FilterOutputGain = optionGain[setting];
            doRefresh[5] = TRUE;
        }
    }
    
    // Update variable data displayed, if changed or isNewScreen
    for (pot = 0; pot < 6; pot++)  
    {
        if (doRefresh[pot])
        {
            if (pot == 0)  sprintf(textBuf, "%3d", (int) GetFilterFreqIndex());
            if (pot == 1)  sprintf(textBuf, "%3d", PotReading(1));  // temp
            if (pot == 2)  sprintf(textBuf, "%3d", PotReading(2));  // temp
            if (pot == 3)  sprintf(textBuf, "%4.1f", g_NoiseFilterGain);
            if (pot == 4)  sprintf(textBuf, "%4.2f", g_FilterInputAtten);
            if (pot == 5)  sprintf(textBuf, "%4.1f", g_FilterOutputGain);

            xpos = (pot % 3) * 43 + 3;
            ypos = (pot < 3) ? 22 : 44;
            LCD_PosXY(xpos, ypos);
            LCD_Mode(SET_PIXELS);  // Erase existing data
            LCD_BlockFill(36, 8);
            LCD_Mode(CLEAR_PIXELS);  // Write new data
            DisplayTextCenteredInField(xpos, ypos, textBuf, 6);
            doRefresh[pot] = FALSE;
        }
    }
}


PRIVATE  void  ScreenFunc_CustomFuncMenu(bool isNewScreen)
{
    if (isNewScreen)  // new screen...
    {
        LCD_Mode(SET_PIXELS);
        LCD_SetFont(PROP_8_NORM); 
        LCD_PosXY(8, 12);
        LCD_PutText("No custom function yet!");  // temporary
        
//      DisplayMenuOption(8, 12, 'A', "  ..     ..");  // todo
//      DisplayMenuOption(8, 22, 'B', "  ..     ..");  // todo
//      DisplayMenuOption(8, 32, 'C', "  ..     ..");  // todo
//      DisplayMenuOption(8, 42, 'D', "  ..     ..");  // todo

        LCD_PosXY(0, 53);
        LCD_DrawLineHoriz(128);
        DisplayMenuOption(0,  56, '*', "Exit");
//      DisplayMenuOption(88, 56, '#', "Next");
    }
    else  // check for button hit
    {
        if (ButtonHit())
        {
            if (ButtonCode() == '*') GoToNextScreen(SCN_HOME);
            if (ButtonCode() == 'A') GoToNextScreen(SCN_HOME);  // reserved 
            if (ButtonCode() == 'B') GoToNextScreen(SCN_HOME);  // reserved
            if (ButtonCode() == 'C') GoToNextScreen(SCN_HOME);  // reserved
            if (ButtonCode() == 'D') GoToNextScreen(SCN_HOME);  // reserved
            if (ButtonCode() == '#') GoToNextScreen(SCN_HOME);  // reserved
        }
    }

    if (m_ElapsedTime_ms >= GUI_INACTIVE_TIMEOUT) GoToNextScreen(SCN_HOME);
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
        DisplayMenuOption(0,  56, '*', "Cancel");
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
        if (ButtonHit())
        {
            if ((key = ButtonCode()) == '*')  // Cancel data entry
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
        if (ButtonHit())
        {
            if (ButtonCode() == '*') GoToNextScreen(SCN_HOME);  else
            if (ButtonCode() == '#') GoToNextScreen(SCN_DATA_ENTRY);
        }
    }
}

// end of file
