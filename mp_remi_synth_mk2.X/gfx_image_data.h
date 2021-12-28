/*****************************************************************************
 * FileName:        gfx_image_data.h
 *
 * Bitmap image declarations for use with LCD_PutImage() function
 */
#ifndef GFX_IMAGE_DATA__H
#define GFX_IMAGE_DATA__H

#include "../Common/system_def.h"
#include "HardwareProfile.h"
#include "LCD_graphics_lib.h"

extern  bitmap_t  big_right_arrow[];         // width: 21, height: 18 pixels
extern  bitmap_t  treble_clef_16x40[];
extern  bitmap_t  flat_up_arrow_8x4[];
extern  bitmap_t  flat_down_arrow_8x4[];
extern  bitmap_t  patch_icon_7x7[];
extern  bitmap_t  midi_conn_icon_9x9[];      // DIN5 skt icon
extern  bitmap_t  Bauer_remi_logo_85x45[];

#define Bauer_logo_85x15  Bauer_remi_logo_85x45  // first 15 rows only
#define Remi_logo_85x30   ((bitmap_t *) &Bauer_remi_logo_85x45[165])  // last 30 rows

// ---------------------------------------------------------------------------------------

#ifdef BUILD_LCD_GRAPHICS_DEMO
/*
 * Image name: coffee_cup_icon, width: 39, height: 41 pixels
 */
extern  bitmap_t  coffee_cup_icon[];

/*
 * Image name: chess_knight, width: 44, height: 44 pixels
 */
extern  bitmap_t  chess_knight[];

/*
 * Image name: padlock_icon, width: 23, height: 33 pixels
 */
extern  bitmap_t  padlock_23x33[];

/*
 * Image name: car_front_56x46, width: 56, height: 46 pixels
 */
extern  bitmap_t  car_front_56x46[];

/*
 * Image name: trash_can_40x40, width: 40, height: 40 pixels
 */
extern  bitmap_t  trash_can_40x40[];

/*
 * Bitmap image definition
 * Image name: tweety_bird_63x54, width: 63, height: 54 pixels
 */
extern  bitmap_t  tweety_bird_63x54[];

#endif  // BUILD_LCD_GRAPHICS_DEMO

#endif  // GFX_IMAGE_DATA__H
