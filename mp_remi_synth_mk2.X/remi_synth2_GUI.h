/**
 *   File:    remi_synth2_GUI.h 
 *
 *   Definitions for the Graphical User Interface (GUI) of the REMI mk2 Sound Synth.
 * 
 */
#ifndef REMI_SYNTH2_GUI_H
#define REMI_SYNTH2_GUI_H

#include "../Common/system_def.h"
#include "HardwareProfile.h"
#include "gfx_image_data.h"

#define POT_MODULE_CONNECTED  (READ_JUMPER_PIN_RB9 == 0)

#define SCREEN_UPDATE_INTERVAL     (50)     // Time (ms) between active screen updates
#define GUI_INACTIVE_TIMEOUT    (30*1000)   // Time (ms) before revert to quiescent screen
#define SELF_TEST_WAIT_TIME_MS     2000     // Duration of self-test message display (ms)

#define GUI_EraseScreen()   LCD_ClearScreen()

#define ALL_BUTTONS_RELEASED      0x0000
#define MASK_BUTTON_A             0x0001
#define MASK_BUTTON_B             0x0002
#define MASK_BUTTON_C             0x0004
#define MASK_BUTTON_D             0x0008
#define MASK_BUTTON_HASH          0x0010
#define MASK_BUTTON_STAR          0x0020

#define LCD_BACKLIGHT_IS_LOW    (LATDbits.LATD9 == 0)
#define LCD_BACKLIGHT_IS_HIGH   (LATDbits.LATD9 != 0)

// An object of this type is needed for each GUI screen.
// An array of structures of this type is held in flash memory (const data area).
// For screens which have no Title Bar, initialize TitleBarText = NULL;
//
typedef struct GUI_screen_descriptor
{
    uint16  screen_ID;             // Screen ID number (0..NUMBER_OF_SCREEN_IDS)
    void    (*ScreenFunc)(bool);   // Function to prepare/update the screen
    char     *TitleBarText;        // Pointer to title string;  NULL if no title bar

} GUI_ScreenDescriptor_t;


// ------- Screens defined in the REMI Local User Interface  -----------------
//
enum  Set_of_Screen_ID_numbers   // Any arbitrary order
{
    SCN_STARTUP = 0,
    SCN_SELFTEST_REPORT,
    SCN_HOME,
    SCN_MAIN_SETTINGS_MENU,
    SCN_PRESET_EDIT_MENU,
    SCN_EDIT_PRESET_PATCH,
    SCN_EDIT_PRESET_MIDI_PGRM,
    SCN_EDIT_PRESET_VIBRATO,
    SCN_EDIT_PRESET_TRANSPOSE,
    SCN_EDIT_REVERB_ATTEN,
    SCN_EDIT_REVERB_MIX,
    SCN_SET_MIDI_IN_MODE,
    SCN_SET_MIDI_IN_CHANNEL,
    SCN_SET_MIDI_IN_EXPRESS,
    SCN_SET_MIDI_OUT_ENABLE,
    SCN_SET_PITCH_BEND_MODE,
    SCN_SYSTEM_INFO_PAGE1,
    SCN_SYSTEM_INFO_PAGE2,
    SCN_CONTROL_PANEL_1,
    SCN_CONTROL_PANEL_2,
    SCN_CUSTOM_FUNC_MENU,
    SCN_DATA_ENTRY,
    SCN_DATA_ENTRY_TEST
};


// --------  GUI Navigation Engine -- generic functions  --------
//
int     GetNumberOfScreensDefined();
uint16  GetCurrentScreenID();
uint16  GetPreviousScreenID();
void    GoToNextScreen(uint16 nextScreenID);

void    GUI_NavigationExec();
void    ButtonInputService();
void    ControlPotService();

bool    ScreenSwitchOccurred(void);
int     ScreenDescIndexFind(uint16 searchID);
void    DisplayMenuOption(uint16 x, uint16 y, char symbol, char *text);
void    DisplayTextCenteredInField(uint16 x, uint16 y, char *str, uint8 nplaces);

#endif // REMI_SYNTH2_GUI_H
