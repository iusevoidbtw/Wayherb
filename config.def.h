/* See LICENSE file for copyright and license details. */

#include <linux/input-event-codes.h>

#include <stdint.h>

#include "draw.h"

#ifndef CONFIG_H
#define CONFIG_H

/*
 * config options start here.
 */
#define UNUSED __attribute__((__unused__))
#define NORETURN __attribute__((__noreturn__))

/*
 * action options.
 */
#define DISMISS_BUTTON BTN_LEFT
#define ACTION_BUTTON BTN_RIGHT
#define EXIT_DISMISS 2

/*
 * font options.
 */
static const char *font  = "serif";
static double font_alpha = 1.0;
static float font_size   = 16.0;

/*
 * notification options.
 */
static double duration = 5.0; /* set to 0.0 to disable auto-dismiss */

/*
 * window options. 
 */

/* margins */
static int32_t margin_right  = 0;
static int32_t margin_bottom = 0;
static int32_t margin_left   = 0;
static int32_t margin_top    = 0;

/*
 * anchor options, e.g:
 * TOP + RIGHT = anchor window to top right, window will show up on the top right corner of the screen
 * BOTTOM + RIGHT = bottom right corner of screen
 * etc etc
 */
static uint32_t anchor                = TOP + RIGHT;

/*
 * window width & border size
 * to change the height of the window, make the font size larger.
 */
static uint32_t width                 = 450;
static const unsigned int border_size = 4;

/* background & border alpha */
static double background_alpha = 1.0;
static double border_alpha     = 1.0;

/*
 * setting this to true makes it so windows will attempt to move out of the
 * way for the notification.
 */
static const int exclusive_zone_on = 1; /* 0 to disable, 1 to enable */

/*
 * colors, these are all RGB in the range 0.0 to 1.0.
 */

/* background color */
static const float bgr = 0.156f;
static const float bgg = 0.164f;
static const float bgb = 0.211f;

/* border color */
static const float brr = 0.384f;
static const float brg = 0.447f;
static const float brb = 0.643f;

/* font color */
static const float fr = 0.972f;
static const float fg = 0.972f;
static const float fb = 0.949f;

#endif
