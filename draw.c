/* See LICENSE file for copyright and license details. */

#define _POSIX_C_SOURCE 200809L /* for stpcpy */
#include <sys/mman.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "draw.h"
#include "util.h"

/* globals */
static struct draw_state draw_state;
static uint32_t output = UINT32_MAX;
static int cur_x = -1, cur_y = -1;
static int button = 0;
static uint32_t layer = ZWLR_LAYER_SHELL_V1_LAYER_TOP;
static int keyboard_interactive = 0;
static void *shm_data = NULL;

/* tmpfile creation */
int
create_tmpfile(off_t size)
{
	static const char template[] = "/mayflower-shared-XXXXXX";
	const char *path;
	char *name;
	int fd;
	int ret;

	path = getenv("XDG_RUNTIME_DIR");
	if (!path) {
		errno = ENOENT;
		return -1;
	}

	name = malloc(strlen(path) + sizeof(template));
	if (!name)
		return -1;

	strcpy(stpcpy(name, path), template);

	fd = mkstemp(name);
	if (fd >= 0)
		unlink(name);
	free(name);

	if (fd < 0)
		return -1;

	ret = posix_fallocate(fd, 0, size);
	if (ret != 0) {
		close(fd);
		errno = ret;
		return -1;
	}
	
	return fd;
}

/* listeners and handles */

/* layer_shell listener */
static void
layer_surface_configure(UNUSED void *data, struct zwlr_layer_surface_v1 *surface, uint32_t serial, UNUSED uint32_t w, UNUSED uint32_t h)
{
	zwlr_layer_surface_v1_ack_configure(surface, serial);
	wl_surface_commit(draw_state.wl_surface);
}

static void
layer_surface_closed(UNUSED void *data, struct zwlr_layer_surface_v1 *surface)
{
	zwlr_layer_surface_v1_destroy(surface);
	wl_surface_destroy(draw_state.wl_surface);
}

struct zwlr_layer_surface_v1_listener layer_surface_listener = {
	.configure = layer_surface_configure,
	.closed = layer_surface_closed,
};

/* pointer listeners and events */
static void
wl_pointer_enter(UNUSED void *data, UNUSED struct wl_pointer *wl_pointer, UNUSED uint32_t serial, struct wl_surface *surface, UNUSED wl_fixed_t surface_x, UNUSED wl_fixed_t surface_y)
{
	draw_state.wl_input_surface = surface;
}

static void
wl_pointer_leave(UNUSED void *data, UNUSED struct wl_pointer *wl_pointer, UNUSED uint32_t serial, UNUSED struct wl_surface *surface)
{
	cur_x = cur_y = -1;
	button = 0;
}	

static void
wl_pointer_motion(UNUSED void *data, UNUSED struct wl_pointer *wl_pointer, UNUSED uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
	cur_x = wl_fixed_to_int(surface_x);
	cur_y = wl_fixed_to_int(surface_y);
}

static void
wl_pointer_button(UNUSED void *data, UNUSED struct wl_pointer *wl_pointer, UNUSED uint32_t serial, UNUSED uint32_t time, uint32_t button, uint32_t state)
{
	if (draw_state.wl_input_surface == draw_state.wl_surface) {
		if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
			if (button == ACTION_BUTTON)
				raise(SIGUSR2);
			else if (button == DISMISS_BUTTON)
				raise(SIGUSR1);
		}
	}
}

struct wl_pointer_listener pointer_listener = {
	.enter = wl_pointer_enter,
	.leave = wl_pointer_leave,
	.motion = wl_pointer_motion,
	.button = wl_pointer_button,
};

/* seat listener */
static void
seat_handle_capabilities(UNUSED void *data, struct wl_seat *wl_seat, enum wl_seat_capability caps)
{
	if ((caps & WL_SEAT_CAPABILITY_POINTER)) {
		draw_state.wl_pointer = wl_seat_get_pointer(wl_seat);
		wl_pointer_add_listener(draw_state.wl_pointer, &pointer_listener, NULL);
	}
}

const struct wl_seat_listener seat_listener = {
	.capabilities = seat_handle_capabilities,
};


/* registry */
static void
registry_global(UNUSED void *data, struct wl_registry *registry, uint32_t name, const char *interface, UNUSED uint32_t version)
{
	if (strcmp(interface, wl_compositor_interface.name) == 0) {
		draw_state.wl_compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 1);
	} else if (strcmp(interface, wl_shm_interface.name) == 0) {
		draw_state.wl_shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
	} else if (strcmp(interface, "wl_output") == 0) {
		if (output != UINT32_MAX) {
			if (!draw_state.wl_output)
				draw_state.wl_output = wl_registry_bind(registry, name, &wl_output_interface, 1);
			else
				--output;
		}
	} else if (strcmp(interface, wl_seat_interface.name) == 0) {
		draw_state.wl_seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
		wl_seat_add_listener(draw_state.wl_seat, &seat_listener, NULL);
	} else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
		draw_state.wl_layer_shell = wl_registry_bind(registry, name, &zwlr_layer_shell_v1_interface, 1);
	}
}

static void
registry_handle_remove(UNUSED void *data, UNUSED struct wl_registry *registry, UNUSED uint32_t name)
{
	/* empty */
}

static const struct wl_registry_listener registry_listener = {
	.global = registry_global,
	.global_remove = registry_handle_remove,
};

/* draw_state intialization and main loop for drawing and input */
static struct wl_buffer *
create_buffer(const char *text, int height) {
	int stride = width * 4;
	int size = stride * height;

	int fd = create_tmpfile(size);
	if (fd < 0) {
		die("failed to create temporary file");
	}

	shm_data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (shm_data == MAP_FAILED) {
		close(fd);
		die("mmap failed");
	}
	
	struct wl_shm_pool *pool = wl_shm_create_pool(draw_state.wl_shm, fd, size);
	struct wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);
	wl_shm_pool_destroy(pool);

	draw_state.cairo_surface = cairo_image_surface_create_for_data(shm_data, CAIRO_FORMAT_ARGB32, width, height, stride);
	draw_state.cairo = cairo_create(draw_state.cairo_surface);
	
	/*cairo_scale(draw_state.cairo, width, height);*/
	cairo_rectangle(draw_state.cairo, 0.0, 0.0, 1.0, 1.0);
	/*cairo_clip(draw_state.cairo);*/
	cairo_set_source_rgba(draw_state.cairo, bgr, bgg, bgb, background_alpha);
	cairo_paint(draw_state.cairo);

	cairo_set_line_width(draw_state.cairo, border_size);
	cairo_set_source_rgba(draw_state.cairo, brr, brg, brb, border_alpha);
	cairo_rectangle(draw_state.cairo, 0, 0, width, height);
	cairo_stroke(draw_state.cairo);
	cairo_fill(draw_state.cairo);

	cairo_select_font_face(draw_state.cairo, font, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(draw_state.cairo, font_size);

	cairo_set_source_rgba(draw_state.cairo, fr, fg, fb, font_alpha);
	/*
	 * TODO: make this based on number of lines, for each line added
	 * divide the height by 2
	 */
	cairo_move_to(draw_state.cairo, font_size, height - font_size/2 - border_size);
	cairo_show_text(draw_state.cairo, text);
	cairo_fill(draw_state.cairo);

	return buffer;
}

static int
get_height(const char *text)
{
	cairo_t *cairo;
	cairo_surface_t *temp_surface;
	temp_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, 1);
	cairo = cairo_create(temp_surface);
	cairo_text_extents_t text_extents;
	cairo_select_font_face(cairo, font, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cairo, font_size);
	cairo_text_extents(cairo, text, &text_extents);
	double line_width = text_extents.width + font_size * 2;
	if (line_width > width) {
		return font_size * 2 * 2;
	}

	return font_size * 2;		
}

void
dispatch(void)
{
	wl_display_prepare_read(draw_state.wl_display);
	/*wl_display_dispatch_pending(draw_state.wl_display);*/
	wl_display_flush(draw_state.wl_display);
	wl_display_read_events(draw_state.wl_display);
	wl_display_dispatch_pending(draw_state.wl_display);
        wl_display_flush(draw_state.wl_display);
        /*wl_display_dispatch(draw_state.wl_display);*/
}

void
init_draw(const char *text)
{
	/* some variables for setting up our layer_surface */
	int height = get_height(text);
	int exclusive_zone = 0;
	const char *namespace = "mayflower";
	if (exclusive_zone_on == 1) {
		exclusive_zone = height;
	}

	/* connect to wayland display */
	draw_state.wl_display = wl_display_connect(NULL);
	if (!draw_state.wl_display) {
		die("failed to connect to Wayland display");
	}

	/* set up our registry for getting events */
	struct wl_registry *registry = wl_display_get_registry(draw_state.wl_display);
	wl_registry_add_listener(registry, &registry_listener, NULL);
	wl_display_roundtrip(draw_state.wl_display);
	
	if (!draw_state.wl_compositor) {
		die("no compositor available");
	}

	if (!draw_state.wl_shm) {
		die("shared memory buffer not available");
	}

	if (!draw_state.wl_layer_shell) {
		die("layer shell not available");
	}

	/* cursor image for being able to see the cursor over our notifcations */
	draw_state.wl_cursor_theme = wl_cursor_theme_load(NULL, 16, draw_state.wl_shm);
	draw_state.wl_cursor_surface = wl_compositor_create_surface(draw_state.wl_compositor);
	
	/* buffer */
	draw_state.wl_buffer = create_buffer(text, height);

	draw_state.wl_surface = wl_compositor_create_surface(draw_state.wl_compositor);
	draw_state.wl_layer_surface = zwlr_layer_shell_v1_get_layer_surface(draw_state.wl_layer_shell, draw_state.wl_surface, draw_state.wl_output, layer, namespace);

	/* configure how layer surface acts */
	zwlr_layer_surface_v1_set_size(draw_state.wl_layer_surface, width, height);
	zwlr_layer_surface_v1_set_anchor(draw_state.wl_layer_surface, anchor);
	zwlr_layer_surface_v1_set_exclusive_zone(draw_state.wl_layer_surface, exclusive_zone);
	zwlr_layer_surface_v1_set_margin(draw_state.wl_layer_surface, margin_top, margin_right, margin_bottom, margin_left);
	zwlr_layer_surface_v1_set_keyboard_interactivity(draw_state.wl_layer_surface, keyboard_interactive);
	zwlr_layer_surface_v1_add_listener(draw_state.wl_layer_surface, &layer_surface_listener, draw_state.wl_layer_surface);

	wl_surface_commit(draw_state.wl_surface);
	wl_display_roundtrip(draw_state.wl_display);

	wl_surface_attach(draw_state.wl_surface, draw_state.wl_buffer, 0, 0);
	wl_surface_commit(draw_state.wl_surface);
	wl_display_roundtrip(draw_state.wl_display);
}

void
quit_draw(void)
{
	wl_surface_attach(draw_state.wl_surface, NULL, 0, 0);
	wl_surface_commit(draw_state.wl_surface);
	wl_surface_destroy(draw_state.wl_surface);
	wl_display_disconnect(draw_state.wl_display);
}
