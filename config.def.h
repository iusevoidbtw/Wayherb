#include "wayland.h"

#define LEFT ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT
#define RIGHT ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT
#define TOP ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP
#define BOTTOM ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM

#define UNUSED __attribute__((unused))
#define NORETURN __attribute__((noreturn))
/* #define HAVE_MKOSTEMP */

/*
 * font options.
 */
#define FONT "serif"
static double falpha = 1.0;
static float font_size = 16.0;

/*
 * window options.
 */
static int32_t margin_right = 0, margin_bottom = 0, margin_left = 0, margin_top = 0;
static const unsigned int duration = 5;
static uint32_t anchor = TOP + RIGHT;
static uint32_t width = 450;
static const unsigned int border_size = 4;
static double alpha = 1.0; /* background alpha */
static double bralpha = 1.0; /* border alpha */

/*
 * setting this to true makes it so windows will attempt to move out of the
 * way for the notification.
 */
static const int exclusive_zone_on = 1;


/*
 * colors, these are all RGB in the range 0.0 to 1.0.
 */

/* background color */
static const float bgr = 0.156;
static const float bgg = 0.164;
static const float bgb = 0.211;

/* border color */
static const float brr = 0.384;
static const float brg = 0.447;
static const float brb = 0.643;

/* font color */
static const float fr = 0.972;
static const float fg = 0.972;
static const float fb = 0.949;
