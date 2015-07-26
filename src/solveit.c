#include <pebble.h>

static Window *window;
static TextLayer *hour_layer;
static TextLayer *hour_label_layer;
static TextLayer *min_layer;
static TextLayer *min_label_layer;
static char min_text[] = "Just a minute";
static char hour_text[8];
static GFont eq_font;
static GFont clear_font;
static GFont label_font;
static GColor eq_bg;
static GColor eq_text;
static GColor clear_bg;
static GColor clear_text;
static bool clear = false;
static bool hide_labels = false;
enum Interval {
  NEVER= 0,
  RARELY = 1,
  REGULARLY = 3,
  OFTEN = 5
};
static int add = REGULARLY;
static int subtract = REGULARLY;
static int multiply = OFTEN;
static int divide = RARELY;
static int square = OFTEN;
static int root = RARELY;
enum MessageKey {
  SHAKE = 0,
  LABELS = 1,
  ADD = 2,
  SUBTRACT = 3,
  MULTIPLY = 4,
  DIVIDE = 5,
  SQUARE = 6,
  ROOT = 7
};

static void create_equation(int num, char *eq) {
  int i = 0;
  
  int all = add + subtract + multiply + divide + square + root;
  int op = rand() % all;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Num: %d, Op: %d (add %d, sub %d, div %d, mul %d, sq %d, root %d)", num, op, add, subtract, divide, multiply, square, root);
  if (root && (op >= all - root)) {
    if (num > 0) {
      snprintf(eq, 20, "√%d", num*num);
      return;
    }
  }
  if (square && (op >= all - (root + square))) {
    for (i=2; i<7; i++) {
      // square (num can be square rooted exactly)
      if (num == i*i) {
        snprintf(eq, 20, "%d²", i);
        return;
      }
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Tried to square %d, failed", num);
  }
  if (multiply && (op >= add + subtract + divide)) {
    for (i=2; i<8; i++) {
      if ((num != i) && (num % i == 0))  {
        snprintf(eq, 20, "%d×%d", (int) num/i, i);
        return;
      }
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Tried to multiply %d, failed", num);
  }
  if (divide && (op >= add + subtract)) {
    i = (rand() % 5) + 2;
    snprintf(eq, 20, "%d÷%d", num*i, i);
    return;
  }
  if (subtract && (op >= add)) {
    i = (rand() % 22) + 1;
    snprintf(eq, 20, "%d−%d", num+i, i);
    return;
  }
  if (add) {
    if (num > 2) {
      i = (rand() % num - 2) + 2;
      snprintf(eq, 20, "%d+%d", num-i, i);
      return;
    }
  }
  // still here? (e.g. multiply chosen but not possible) - let's retry
  create_equation(num, eq);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "No suitable op, retrying");
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

  layer_set_hidden(text_layer_get_layer(hour_label_layer), (clear || hide_labels) ? true : false);
  layer_set_hidden(text_layer_get_layer(min_label_layer), (clear || hide_labels) ? true : false);

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

void in_received_handler(DictionaryIterator *received, void *context) {
  Tuple *sht = dict_find(received, SHAKE);
  int shake = sht->value->int8;
  accel_tap_service_unsubscribe();
  if (shake) {
    accel_tap_service_subscribe(tap_handler);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Subscribed to accel tap service: %d", shake);
  }
  else {
    clear = false;
  }

  Tuple *lt = dict_find(received, LABELS);
  hide_labels = lt->value->int8 ? false : true;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Set hide_labels to: %d", hide_labels);

  Tuple *at = dict_find(received, ADD);
  add = at->value->int16;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Configured add to: %d", add);

  Tuple *st = dict_find(received, SUBTRACT);
  subtract = st->value->int16;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Configured subtract to: %d", subtract);

  Tuple *mt = dict_find(received, MULTIPLY);
  multiply = mt->value->int16;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Configured multiply to: %d", multiply);

  Tuple *dt = dict_find(received, DIVIDE);
  divide = dt->value->int16;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Configured divide to: %d", divide);

  Tuple *sqt = dict_find(received, SQUARE);
  square = sqt->value->int16;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Configured square to: %d", square);

  Tuple *rt = dict_find(received, ROOT);
  root = rt->value->int16;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Configured root to: %d", root);

}

void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Message from phone dropped: %d", reason);
}


static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  eq_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_48));
  clear_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_64));
  label_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_14));

  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

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

  hour_layer = text_layer_create((GRect) { .origin = { 0, 10 }, .size = { bounds.size.w, 65 } });
  text_layer_set_font(hour_layer, eq_font);
  text_layer_set_text(hour_layer, hour_text);
  text_layer_set_background_color(hour_layer, GColorClear);
  text_layer_set_text_alignment(hour_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(hour_layer));

  hour_label_layer = text_layer_create((GRect) { .origin = { 0, 60 }, .size = { bounds.size.w, 15 } });
  text_layer_set_font(hour_label_layer, label_font);
  text_layer_set_text(hour_label_layer, "hours");
  text_layer_set_background_color(hour_label_layer, GColorClear);
  text_layer_set_text_color(hour_label_layer, eq_text);
  text_layer_set_text_alignment(hour_label_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(hour_label_layer));

  min_layer = text_layer_create((GRect) { .origin = { 0, 75 }, .size = { bounds.size.w, 65 } });
  text_layer_set_font(min_layer, eq_font);
  text_layer_set_text(min_layer, min_text);
  text_layer_set_background_color(min_layer, GColorClear);
  text_layer_set_text_alignment(min_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(min_layer));

  min_label_layer = text_layer_create((GRect) { .origin = { 0, 125 }, .size = { bounds.size.w, 15 } });
  text_layer_set_font(min_label_layer, label_font);
  text_layer_set_text(min_label_layer, "minutes");
  text_layer_set_background_color(min_label_layer, GColorClear);
  text_layer_set_text_color(min_label_layer, eq_text);
  text_layer_set_text_alignment(min_label_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(min_label_layer));

}

static void window_unload(Window *window) {
  text_layer_destroy(min_label_layer);
  text_layer_destroy(min_layer);
  text_layer_destroy(hour_label_layer);
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
