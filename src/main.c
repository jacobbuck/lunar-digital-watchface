#include <pebble.h>

const int SPRITE_SIZE = 48;

static Window *s_window;
static GBitmap *s_numbers_bitmap;
static Layer *s_canvas_layer;
static GSize s_sprite_size;

static void draw_number(GContext *ctx, GPoint origin, int number) {
	// Create temporary GBitmap of digit we want to display
	struct GBitmap *temp_bitmap = gbitmap_create_as_sub_bitmap(s_numbers_bitmap, (GRect){
		.origin = GPoint(number * SPRITE_SIZE, SPRITE_SIZE),
		.size = s_sprite_size
	});

	// Draw digit GBitmap to canvas
	graphics_draw_bitmap_in_rect(ctx, temp_bitmap, (GRect){ .origin = origin, .size = s_sprite_size });

	// Destroy temporary GBitmap
	gbitmap_destroy(temp_bitmap);
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
	// Get a tm structure
	time_t now = time(NULL);
	struct tm *tick_time = localtime(&now);

	// Get the hours and minutes
	int hour = tick_time->tm_hour;
	int min = tick_time->tm_min;

	// Convert hours to 12 hours if the user prefers
	if (clock_is_24h_style() == false && hour > 12) {
		hour -= 12;
	}

	// Only draw first digit of hours if not 0
	if ((hour / 10) > 0) {
		draw_number(ctx, GPoint(16, 28), (int) (hour / 10));
	}

	// Draw second digit of hours
	draw_number(ctx, GPoint(80, 28), (int) (hour % 10));

	// Draw minute digits
	draw_number(ctx, GPoint(16, 92), (int) (min / 10));
	draw_number(ctx, GPoint(80, 92), (int) (min % 10));
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	layer_mark_dirty(window_get_root_layer(s_window));
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect window_bounds = layer_get_bounds(window_layer);

	// Set background color of window
	window_set_background_color(window, GColorBlack);

	// Create canvas layer and add to window
	s_canvas_layer = layer_create(window_bounds);
	layer_set_update_proc(s_canvas_layer, canvas_update_proc);
	layer_add_child(window_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
	// Destroy canvas layer
	layer_destroy(s_canvas_layer);
}

static void init() {
	// Creat sprite GSize for later use
	s_sprite_size = GSize(SPRITE_SIZE, SPRITE_SIZE);

	// Create main window element
	s_window = window_create();

	// Create numbers spritesheet GBitmap
	s_numbers_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NUMBERS);

	// Set handlers to manage the elements inside the Window
	window_set_window_handlers(s_window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload
	});

	// Show the window on the watch, with animated=true
	window_stack_push(s_window, true);

	// Register with TickTimerService
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
	// Destroy window
	window_destroy(s_window);

	// Destroy numbers spritesheet GBitmap
	gbitmap_destroy(s_numbers_bitmap);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}