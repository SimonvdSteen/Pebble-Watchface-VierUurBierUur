#include <pebble.h>
#include "container.h"
	
Window *window;
static GBitmap *image_icon_pebble;
static BitmapLayer *image_icon_pebble_layer;
TextLayer *text_charge_layer;
TextLayer *text_week_layer;
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
const char beer_text_base[] = "Tijd voor Bier";
static char beer_text[] = "Tijd voor Bier?";
const char beer_text_remaining[] = "wachten op Bier!";
const char drink_up[] = "4 uur, Bier uur!";
const char drink_on[] = "Oost west, bier lest";
const int beer_oclock = 16;
static char time_text[] = "00:00";
static char hour_remaining_text[] = "00";
static char hour_remaining_text_show[] = "00 uur";
static char s_battery_buffer[] = "000%";
static char s_week_number[] = "WK00";

//Check Pebble Battery
static void battery_handler(BatteryChargeState charge_state) {
	if (charge_state.is_charging) {
		text_layer_set_font(text_charge_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
		snprintf(s_battery_buffer, sizeof(s_battery_buffer), "~");
	} else {
		snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%%", charge_state.charge_percent);
	}
	text_layer_set_text(text_charge_layer, s_battery_buffer);
}

void line_layer_update_callback(Layer *layer, GContext* ctx) {
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	if (!tick_time) {
		time_t now = time(NULL);
		tick_time = localtime(&now);
	}
	
	strftime(time_text, sizeof(time_text), clock_is_24h_style() ? time_format_24 : time_format_12, tick_time);
	text_layer_set_text(text_time_layer, time_text);
	
	strftime(s_week_number, sizeof(s_week_number), "WK%V", tick_time);
	
	int hour_remaining = tick_time->tm_hour;
	int minute_current = tick_time->tm_min;
	int hour_stop = 4;
	int hour_wake = 7;
	//Calculate remaining time
	int hour_current_remaining = beer_oclock - hour_remaining;
	if (hour_remaining >= hour_stop && hour_remaining < beer_oclock) {
		//Add Questionmark
		snprintf(beer_text, sizeof(beer_text), "%s?", beer_text_base);
		
		if(hour_remaining >= hour_stop && hour_remaining > hour_wake){
			text_layer_set_text(text_hours_layer, beer_text_remaining);
			text_layer_set_font(text_hours_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
		
			if(hour_current_remaining > 1){				
				snprintf(hour_remaining_text_show, 7, "%d uur", hour_current_remaining);
			}else{
				int  minute_remaining =  60-minute_current;
				snprintf(hour_remaining_text_show, 7, "%d min", minute_remaining);
			}
			text_layer_set_text(text_countdown_layer, hour_remaining_text_show);
		}else{
			text_layer_set_text(text_countdown_layer, "Zzzzzz!");
		}
		layer_set_hidden((Layer *)text_countdown_layer, false);
		layer_set_hidden((Layer *)text_drinkup_layer, true);
	} else {
		//Add Exclamationmark
		snprintf(beer_text, sizeof(beer_text), "%s!", beer_text_base);
		
		//Beer hour
		if(hour_current_remaining == 0){	
			text_layer_set_text(text_drinkup_layer, drink_up);
		}else{
			text_layer_set_font(text_drinkup_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
			text_layer_set_text(text_drinkup_layer, drink_on);	
		}
		
		layer_set_hidden((Layer *)text_hours_layer, true);
		layer_set_hidden((Layer *)text_countdown_layer, true);
		layer_set_hidden((Layer *)text_drinkup_layer, false);
	}
	
	//Vibrate on Beer 'o Clock
	if(hour_remaining == beer_oclock && minute_current == 00){
		vibes_long_pulse();
	}
	
	text_layer_set_text(text_beer_layer, beer_text);
}

void handle_deinit(void) {
	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	gbitmap_destroy(image_icon_pebble);
	bitmap_layer_destroy(image_icon_pebble_layer);
}

void handle_init(void) {	
	window = window_create();
	window_stack_push(window, true /* Animated */);
	window_set_background_color(window, GColorBlack);
	Layer *window_layer = window_get_root_layer(window);
	text_time_layer = text_layer_create(GRect(0, 18, 144, 144));
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
	text_beer_layer = text_layer_create(GRect(0, 77, 144, 30));
	text_layer_set_text_alignment(text_beer_layer, GTextAlignmentCenter);
	text_layer_set_text_color(text_beer_layer, GColorBlack);
	text_layer_set_background_color(text_beer_layer, GColorClear);
	text_layer_set_font(text_beer_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	layer_add_child(window_layer, text_layer_get_layer(text_beer_layer));
	text_drinkup_layer = text_layer_create(GRect(25, 105, 90, 70));
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
	
	//Handle Week Number
	text_week_layer = text_layer_create(GRect(5, 0, 40, 20));
	text_layer_set_text_alignment(text_week_layer, GTextAlignmentLeft);
	text_layer_set_text_color(text_week_layer, GColorWhite);
	text_layer_set_background_color(text_week_layer, GColorClear);
	text_layer_set_font(text_week_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	layer_add_child(window_layer, text_layer_get_layer(text_week_layer));
	text_layer_set_text(text_week_layer, s_week_number);
	
	//Add Icon next to Battery
	image_icon_pebble = gbitmap_create_with_resource(RESOURCE_ID_ICON_PEBBLE);
	image_icon_pebble_layer = bitmap_layer_create(GRect(94, 1, 14, 17));
	bitmap_layer_set_bitmap(image_icon_pebble_layer, image_icon_pebble);
	layer_add_child(window_layer, bitmap_layer_get_layer(image_icon_pebble_layer));
	
	//Handle Battery
	text_charge_layer = text_layer_create(GRect(99, 0, 40, 20));
	text_layer_set_text_alignment(text_charge_layer, GTextAlignmentRight);
	text_layer_set_text_color(text_charge_layer, GColorWhite);
	text_layer_set_background_color(text_charge_layer, GColorClear);
	text_layer_set_font(text_charge_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	layer_add_child(window_layer, text_layer_get_layer(text_charge_layer));
	text_layer_set_text(text_charge_layer, s_battery_buffer);
	battery_state_service_subscribe(battery_handler);
	battery_handler(battery_state_service_peek());
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}