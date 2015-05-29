#include <pebble.h>

static Window *window;
static TextLayer *hour_layer;
static TextLayer *min_layer;
static char min_text[] = "Just a minute";
static char hour_text[8];
static GFont eq_font;
static GFont clear_font;
static GColor eq_bg;
static GColor eq_text;
static GColor clear_bg;
static GColor clear_text;
static bool clear = false;

static void create_equation(int num, char *eq) {
  int i = 0;
  for (i=2; i<7; i++) {
    // square (num can be square rooted exactly)
    if (num == i*i) {
      snprintf(eq, 20, "%d²", i);
      return;
    }
  }

  // if not square rooted or divided, use other operators  
  int op = rand() % 4;
  switch (op) {
    case 0:
      // add
      i = (rand() % 22) + 1;
      snprintf(eq, 20, "%d−%d", num+i, i);
      break;
    case 1: 
      // subtract
      i = (rand() % num-2) + 2;
      snprintf(eq, 20, "%d+%d", num-i, i);
      break;
    case 2: 
      // divide
      i = (rand() % 5) + 2;
      snprintf(eq, 20, "%d÷%d", num*i, i);
      break;
    case 3:
      // multiply (if num can be divided exactly)
      for (i=2; i<8; i++) {
        if ((num != i) && (num % i == 0))  {
          snprintf(eq, 20, "%d×%d", (int) num/i, i);
          return;
        }
      }
    default: 
      // square root
      snprintf(eq, 20, "√%d", num*num);
  }
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "Created %s based on op %d (%d, %d)", eq, op, num, i);
}

static void update_time(struct tm* t) {
  if (clear) {
    text_layer_set_font(min_layer, clear_font);
    text_layer_set_font(hour_layer, clear_font);
    snprintf(hour_text, sizeof(hour_text), "%d", t->tm_hour);
    snprintf(min_text, sizeof(min_text), "%02d", t->tm_min);
  }
  else {
    text_layer_set_font(min_layer, eq_font);
    text_layer_set_font(hour_layer, eq_font);
    create_equation(t->tm_hour, hour_text);
    create_equation(t->tm_min, min_text);
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "Got %s and %s from %d and %d", hour_text, min_text, t->tm_hour, t->tm_min);
  }
  window_set_background_color(window, clear ? clear_bg : eq_bg);
  text_layer_set_text_color(hour_layer, clear ? clear_text : eq_text);
  text_layer_set_text_color(min_layer, clear ? clear_text : eq_text);

  text_layer_set_text(min_layer, min_text);
  text_layer_set_text(hour_layer, hour_text);
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  clear = !clear;
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "Tapped, set clear to %d", (int) clear);
  time_t tm = time(NULL);
  struct tm *tms;
  tms = localtime(&tm);
  update_time(tms);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  eq_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_48));
  clear_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_64));

  // hour_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, bounds.size.h/2 } });
  hour_layer = text_layer_create((GRect) { .origin = { 0, 10 }, .size = { bounds.size.w, 65 } });
  text_layer_set_font(hour_layer, eq_font);
  text_layer_set_text(hour_layer, hour_text);
  text_layer_set_background_color(hour_layer, GColorClear);
  text_layer_set_text_alignment(hour_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(hour_layer));

  // min_layer = text_layer_create((GRect) { .origin = { 0, bounds.size.h/2 }, .size = { bounds.size.w, bounds.size.h/2 } });
  min_layer = text_layer_create((GRect) { .origin = { 0, 75 }, .size = { bounds.size.w, 65 } });
  text_layer_set_font(min_layer, eq_font);
  text_layer_set_text(min_layer, min_text);
  text_layer_set_background_color(min_layer, GColorClear);
  text_layer_set_text_alignment(min_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(min_layer));

#ifdef PBL_COLOR
  eq_bg = GColorOxfordBlue;
  eq_text = GColorWhite;
  clear_bg = GColorPastelYellow;
  clear_text = GColorBlack;
#else
  eq_bg = GColorBlack;
  eq_text = GColorWhite;
  clear_bg = GColorWhite;
  clear_text = GColorBlack;
#endif

}

static void window_unload(Window *window) {
  text_layer_destroy(min_layer);
  text_layer_destroy(hour_layer);
}

static void init(void) {
  accel_tap_service_subscribe(tap_handler);
  tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
  tick_timer_service_unsubscribe();
  accel_tap_service_unsubscribe();
}

int main(void) {
  init();
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);
  app_event_loop();
  deinit();
}
