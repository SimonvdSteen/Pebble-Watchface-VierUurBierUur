#include <pebble.h>
#include "container.h"
	
Window *window;
TextLayer *text_date_layer;
TextLayer *text_countdown_layer;
TextLayer *text_time_layer;
TextLayer *text_beer_layer;
TextLayer *text_hours_layer;
TextLayer *text_white_layer;
TextLayer *text_drinkup_layer;
Layer *line_layer;
const char *time_format_12 = "%02I:%M";
const char *time_format_24 = "%02H:%M";
const char beer_text[] = "Tijd voor bier!";
const char beer_text_remaining[] = "Bijna biertijd...";
const char drink_up[] = "4 uur, Bier uur!";
const int beer_oclock = 16;
static char time_text[] = "00:00";
static char hour_remaining_text[] = "00";

void line_layer_update_callback(Layer *layer, GContext* ctx) {
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	if (!tick_time) {
		time_t now = time(NULL);
		tick_time = localtime(&now);
	}
	
	strftime(time_text, sizeof(time_text), clock_is_24h_style() ?
	time_format_24 : time_format_12, tick_time);
	text_layer_set_text(text_time_layer, time_text);
	text_layer_set_text(text_beer_layer, beer_text);
	
	int hour_remaining = tick_time->tm_hour;
	int minute_current = tick_time->tm_min;
	int hour_stop = 4;
	int hour_wake = 7;
	if (hour_remaining >= hour_stop && hour_remaining < beer_oclock) {
		int hour_current_remaining = beer_oclock - hour_remaining;
		if(hour_remaining >= hour_stop && hour_current_remaining < hour_wake){
			text_layer_set_text(text_hours_layer, beer_text_remaining);
			text_layer_set_font(text_hours_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
		
			snprintf(hour_remaining_text, sizeof(hour_remaining_text), "%d", hour_current_remaining);
			text_layer_set_text(text_countdown_layer, hour_remaining_text);
		}else{
			text_layer_set_text(text_countdown_layer, "Zzzzzz!");
		}
		layer_set_hidden((Layer *)text_drinkup_layer, true);
	} else {
		text_layer_set_text(text_drinkup_layer, drink_up);
		layer_set_hidden((Layer *)text_hours_layer, false);
	}
	
	//Vibrate on Beer 'o Clock
	if(hour_remaining == beer_oclock && minute_current == 00){
		vibes_long_pulse();
	}
}

void handle_deinit(void) {
	tick_timer_service_unsubscribe();
}

void handle_init(void) {
	window = window_create();
	window_stack_push(window, true /* Animated */);
	window_set_background_color(window, GColorBlack);
	Layer *window_layer = window_get_root_layer(window);
	text_time_layer = text_layer_create(GRect(0, 15, 144, 55));
	text_layer_set_text_alignment(text_time_layer, GTextAlignmentCenter);
	text_layer_set_text_color(text_time_layer, GColorWhite);
	text_layer_set_background_color(text_time_layer, GColorClear);
	text_layer_set_font(text_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	layer_add_child(window_layer, text_layer_get_layer(text_time_layer));
	text_white_layer = text_layer_create(GRect(0, 80, 144, 100));
	text_layer_set_text_color(text_white_layer, GColorBlack);
	text_layer_set_background_color(text_white_layer, GColorWhite);
	text_layer_set_font(text_white_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	layer_add_child(window_layer, text_layer_get_layer(text_white_layer));
	text_beer_layer = text_layer_create(GRect(0, 80, 144, 30));
	text_layer_set_text_alignment(text_beer_layer, GTextAlignmentCenter);
	text_layer_set_text_color(text_beer_layer, GColorBlack);
	text_layer_set_background_color(text_beer_layer, GColorClear);
	text_layer_set_font(text_beer_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	layer_add_child(window_layer, text_layer_get_layer(text_beer_layer));
	text_drinkup_layer = text_layer_create(GRect(25, 103, 90, 70));
	text_layer_set_text_alignment(text_drinkup_layer, GTextAlignmentCenter);
	text_layer_set_text_color(text_drinkup_layer, GColorBlack);
	text_layer_set_background_color(text_drinkup_layer, GColorClear);
	text_layer_set_font(text_drinkup_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	layer_add_child(window_layer, text_layer_get_layer(text_drinkup_layer));
	text_countdown_layer = text_layer_create(GRect(0, 107, 144, 35));
	text_layer_set_text_alignment(text_countdown_layer, GTextAlignmentCenter);
	text_layer_set_text_color(text_countdown_layer, GColorBlack);
	text_layer_set_background_color(text_countdown_layer, GColorClear);
	text_layer_set_font(text_countdown_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
	layer_add_child(window_layer, text_layer_get_layer(text_countdown_layer));
	text_hours_layer = text_layer_create(GRect(0, 135, 144, 30));
	text_layer_set_text_alignment(text_hours_layer, GTextAlignmentCenter);
	text_layer_set_text_color(text_hours_layer, GColorBlack);
	text_layer_set_background_color(text_hours_layer, GColorClear);
	text_layer_set_font(text_hours_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
	layer_add_child(window_layer, text_layer_get_layer(text_hours_layer));
	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
	handle_minute_tick(NULL, MINUTE_UNIT);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}