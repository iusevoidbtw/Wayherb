/* See LICENSE file for copyright and license details. */

#ifndef DRAW_H
#define DRAW_H

#include <cairo/cairo.h>

#include <wayland-client.h>
#include <wayland-cursor.h>

#include "wayland/wlr-layer-shell-unstable-v1.h"
#include "wayland/xdg-shell-client-protocol.h"

#define LEFT ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT
#define RIGHT ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT
#define TOP ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP
#define BOTTOM ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM

struct draw_state {
	/* display */
	struct wl_display *wl_display;
	//struct wl_registry *wl_registry;
	struct wl_compositor *wl_compositor;
	struct wl_shm *wl_shm;
	struct wl_shm_pool *wl_shm_pool;
	struct wl_buffer *wl_buffer;
	struct zwlr_layer_shell_v1 *wl_layer_shell;
	struct zwlr_layer_surface_v1 *wl_layer_surface;
	struct wl_output *wl_output;
	struct wl_surface *wl_surface;

	/* cairo and pixman */
	cairo_surface_t *cairo_surface;
	cairo_t *cairo;
	
	/* input */
	struct wl_seat *wl_seat;
	struct wl_pointer *wl_pointer;
	struct wl_cursor_image *wl_cursor_image;
	struct wl_cursor_theme *wl_cursor_theme;
	struct wl_surface *wl_cursor_surface, *wl_input_surface;
};

void dispatch(void);
void init_draw(const char *text);
void quit_draw(void);

#endif
