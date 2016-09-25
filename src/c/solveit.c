#include <pebble.h>
#define STR_MAX_LEN 12
#define STEP_LAYER_HEIGHT 15
#define DATE_LAYER_HEIGHT 15

static Window *window;
static TextLayer *hour_layer;
static TextLayer *hour_label_layer;
static TextLayer *min_layer;
static TextLayer *min_label_layer;
static TextLayer *step_layer;
static TextLayer *day_layer;
static char min_text[STR_MAX_LEN];
static char hour_text[STR_MAX_LEN];
static char step_text[STR_MAX_LEN];
static char day_text[STR_MAX_LEN];
static TextLayer *month_layer;
static char month_text[STR_MAX_LEN];
static GFont eq_font;
static GFont clear_font;
static GFont label_font;
static GFont step_font;
static GFont date_font;
static GColor eq_bg;
static GColor eq_text;
static GColor clear_bg;
static GColor clear_text;
static int shake; // 0 = nothing, 1 = solve, 2 = new expression
static bool clear = false;
static bool hide_labels;
static bool show_steps;
static bool show_day;
static bool show_month;
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
static const HealthMetric metric = HealthMetricStepCount;

static void create_equation(int num, char *eq) {
  int i = 0;

  int all = add + subtract + multiply + divide + square + root;
  int op = rand() % all;
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "Num: %d, Op: %d (add %d, sub %d, div %d, mul %d, sq %d, root %d)", num, op, add, subtract, divide, multiply, square, root);
  if (root && (op >= all - root)) {
    // don't show roots of very large numbers (steps)
    if ((num > 0) && (num <= 100)) {
      snprintf(eq, STR_MAX_LEN, "√%d", num*num);
      return;
    }
  }
  if (square && (op >= all - (root + square))) {
    for (i=2; i<7; i++) {
      // square (num can be square rooted exactly)
      if (num == i*i) {
        snprintf(eq, STR_MAX_LEN, "%d²", i);
        return;
      }
    }
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "Tried to square %d, failed", num);
  }
  if (multiply && (op >= add + subtract + divide)) {
    for (i=2; i<8; i++) {
      if ((num != i) && (num % i == 0))  {
        snprintf(eq, STR_MAX_LEN, "%d×%d", (int) num/i, i);
        return;
      }
    }
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "Tried to multiply %d, failed", num);
  }
  if (divide && (op >= add + subtract)) {
    i = (rand() % 5) + 2;
    snprintf(eq, STR_MAX_LEN, "%d÷%d", num*i, i);
    return;
  }
  if (subtract && (op >= add)) {
    i = (rand() % 22) + 1;
    snprintf(eq, STR_MAX_LEN, "%d−%d", num+i, i);
    return;
  }
  if (add) {
    if (num > 2) {
      i = (rand() % num - 2) + 2;
      snprintf(eq, STR_MAX_LEN, "%d+%d", num-i, i);
      return;
    }
  }
  // still here? (e.g. multiply chosen but not possible) - let's retry
  create_equation(num, eq);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "No suitable op, retrying");
}

static void update_time(struct tm* t) {
  int step_count = 0;
  if (show_steps) {
    time_t start = time_start_of_today();
    time_t end = time(NULL);
    // Check the metric has data available for today
    HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, start, end);
    if(mask & HealthServiceAccessibilityMaskAvailable) {
      step_count = (int) health_service_sum_today(metric);
      // Data is available!
      APP_LOG(APP_LOG_LEVEL_INFO, "Steps today: %d", step_count);
    } else {
      // No data recorded yet today
      APP_LOG(APP_LOG_LEVEL_ERROR, "Data unavailable!");
    }
  }

  if (clear) {
    text_layer_set_font(min_layer, clear_font);
    text_layer_set_font(hour_layer, clear_font);
    snprintf(hour_text, sizeof(hour_text), "%d", t->tm_hour);
    snprintf(min_text, sizeof(min_text), "%02d", t->tm_min);
    snprintf(day_text, sizeof(day_text), "%d", t->tm_mday);
    snprintf(month_text, sizeof(min_text), "%d", t->tm_mon + 1);
    if (show_steps && (step_count > 0)) {
      snprintf(step_text, STR_MAX_LEN, "%d", step_count);
    }
  }
  else {
    text_layer_set_font(min_layer, eq_font);
    text_layer_set_font(hour_layer, eq_font);
    create_equation(t->tm_hour, hour_text);
    create_equation(t->tm_min, min_text);
    create_equation(t->tm_mday, day_text);
    create_equation(t->tm_mon + 1, month_text);
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "Got %s and %s from %d and %d", hour_text, min_text, t->tm_hour, t->tm_min);
    if (show_steps && (step_count > 0)) {
      create_equation(step_count, step_text);
    }
  }
  window_set_background_color(window, clear ? clear_bg : eq_bg);
  text_layer_set_text_color(hour_layer, clear ? clear_text : eq_text);
  text_layer_set_text_color(min_layer, clear ? clear_text : eq_text);
  text_layer_set_text_color(step_layer, clear ? clear_text : eq_text);
  text_layer_set_text_color(day_layer, clear ? clear_text : eq_text);
  text_layer_set_text_color(month_layer, clear ? clear_text : eq_text);

  layer_set_hidden(text_layer_get_layer(hour_label_layer), (clear || hide_labels) ? true : false);
  layer_set_hidden(text_layer_get_layer(min_label_layer), (clear || hide_labels) ? true : false);
  layer_set_hidden(text_layer_get_layer(step_layer), !show_steps);
  layer_set_hidden(text_layer_get_layer(month_layer), PBL_IF_ROUND_ELSE(true, !show_month));
  layer_set_hidden(text_layer_get_layer(day_layer), !show_day);

  text_layer_set_text(min_layer, min_text);
  text_layer_set_text(hour_layer, hour_text);
  text_layer_set_text(step_layer, step_text);
  text_layer_set_text(day_layer, day_text);
  text_layer_set_text(month_layer, month_text);

}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  if (shake == 0) {
    // do nothing
    return;
  }
  if (shake == 1) {
    // toggle between expression and solution
    clear = !clear;
  }
  // if shake == 2, just create a new expression
  time_t tm = time(NULL);
  struct tm *tms;
  tms = localtime(&tm);
  update_time(tms);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
}

void in_received_handler(DictionaryIterator *received, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Got message from phone!");

  Tuple *sht = dict_find(received, MESSAGE_KEY_SHAKE);
  // shake = atoi(sht->value->int8);
  shake = atoi(sht->value->cstring);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Set shake value to: %d", shake);
  accel_tap_service_unsubscribe();
  if (shake) {
    accel_tap_service_subscribe(tap_handler);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Subscribed to accel tap service");
  }
  else {
    clear = false;
  }

  Tuple *lt = dict_find(received, MESSAGE_KEY_LABELS);
  hide_labels = lt->value->int8 ? false : true; // invert!
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Set hide_labels to: %d", hide_labels);

  Tuple *ht = dict_find(received, MESSAGE_KEY_STEPS);
  show_steps = ht->value->int8 ? true : false;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Set show_steps to: %d", show_steps);

  Tuple *dayt = dict_find(received, MESSAGE_KEY_DAY);
  show_day = dayt->value->int8 ? true : false;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Set show_day to: %d", show_day);

  Tuple *montht = dict_find(received, MESSAGE_KEY_MONTH);
  show_month = montht->value->int8 ? true : false;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Set show_month to: %d", show_month);

  Tuple *at = dict_find(received, MESSAGE_KEY_ADD);
  // add = at->value->int16;
  add = atoi(at->value->cstring);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Configured add to: %d", add);

  Tuple *st = dict_find(received, MESSAGE_KEY_SUBTRACT);
  // subtract = st->value->int16;
  subtract = atoi(st->value->cstring);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Configured subtract to: %d", subtract);

  Tuple *mt = dict_find(received, MESSAGE_KEY_MULTIPLY);
  // multiply = mt->value->int16;
  multiply = atoi(mt->value->cstring);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Configured multiply to: %d", multiply);

  Tuple *dt = dict_find(received, MESSAGE_KEY_DIVIDE);
  // divide = dt->value->int16;
  divide = atoi(dt->value->cstring);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Configured divide to: %d", divide);

  Tuple *sqt = dict_find(received, MESSAGE_KEY_SQUARE);
  // square = sqt->value->int16;
  square = atoi(sqt->value->cstring);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Configured square to: %d", square);

  Tuple *rt = dict_find(received, MESSAGE_KEY_ROOT);
  // root = rt->value->int16;
  root = atoi(rt->value->cstring);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Configured root to: %d", root);

  time_t tm = time(NULL);
  struct tm *tms;
  tms = localtime(&tm);
  update_time(tms);
}

void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Message from phone dropped: %d", reason);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_unobstructed_bounds(window_layer);
  eq_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_48));
  clear_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_64));
  label_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_14));
  step_font = label_font;
  date_font = step_font;

  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  // app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  app_message_open(128, 128);

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

  hour_layer = text_layer_create((GRect) { .origin = { 0, bounds.size.h/2-72 }, .size = { bounds.size.w, 65 } });
  text_layer_set_font(hour_layer, eq_font);
  text_layer_set_text(hour_layer, hour_text);
  text_layer_set_background_color(hour_layer, GColorClear);
  text_layer_set_text_alignment(hour_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(hour_layer));

  hour_label_layer = text_layer_create((GRect) { .origin = { 0, bounds.size.h/2-22 }, .size = { bounds.size.w, 15 } });
  text_layer_set_font(hour_label_layer, label_font);
  text_layer_set_text(hour_label_layer, "hours");
  text_layer_set_background_color(hour_label_layer, GColorClear);
  text_layer_set_text_color(hour_label_layer, eq_text);
  text_layer_set_text_alignment(hour_label_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(hour_label_layer));

  min_layer = text_layer_create((GRect) { .origin = { 0, bounds.size.h/2-7 }, .size = { bounds.size.w, 65 } });
  text_layer_set_font(min_layer, eq_font);
  text_layer_set_text(min_layer, min_text);
  text_layer_set_background_color(min_layer, GColorClear);
  text_layer_set_text_alignment(min_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(min_layer));

  min_label_layer = text_layer_create((GRect) { .origin = { 0, bounds.size.h/2+43 }, .size = { bounds.size.w, 15 } });
  text_layer_set_font(min_label_layer, label_font);
  text_layer_set_text(min_label_layer, "minutes");
  text_layer_set_background_color(min_label_layer, GColorClear);
  text_layer_set_text_color(min_label_layer, eq_text);
  text_layer_set_text_alignment(min_label_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(min_label_layer));

  layer_set_hidden(text_layer_get_layer(hour_label_layer), hide_labels ? true : false);
  layer_set_hidden(text_layer_get_layer(min_label_layer), hide_labels ? true : false);

  step_layer = text_layer_create((GRect) { .origin = { 0, 1 }, .size = { bounds.size.w, STEP_LAYER_HEIGHT } });
  text_layer_set_font(step_layer, step_font);
  text_layer_set_text(step_layer, step_text);
  text_layer_set_text_color(step_layer, eq_text);
  text_layer_set_background_color(step_layer, GColorClear);
  text_layer_set_text_alignment(step_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(step_layer));

  day_layer = text_layer_create((GRect) { .origin = { PBL_IF_ROUND_ELSE(0, bounds.size.w/2), bounds.size.h-1-DATE_LAYER_HEIGHT }, .size = { PBL_IF_ROUND_ELSE(bounds.size.w, bounds.size.w/2), DATE_LAYER_HEIGHT } });
  text_layer_set_font(day_layer, date_font);
  text_layer_set_text(day_layer, day_text);
  text_layer_set_text_color(day_layer, eq_text);
  text_layer_set_background_color(day_layer, GColorClear);
  text_layer_set_text_alignment(day_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(day_layer));

  month_layer = text_layer_create((GRect) { .origin = { 0, bounds.size.h-1-DATE_LAYER_HEIGHT }, .size = { bounds.size.w/2, DATE_LAYER_HEIGHT } });
  text_layer_set_font(month_layer, date_font);
  text_layer_set_text(month_layer, month_text);
  text_layer_set_text_color(month_layer, eq_text);
  text_layer_set_background_color(month_layer, GColorClear);
  text_layer_set_text_alignment(month_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(month_layer));

}

static void window_unload(Window *window) {
  text_layer_destroy(step_layer);
  text_layer_destroy(day_layer);
  text_layer_destroy(month_layer);
  text_layer_destroy(min_label_layer);
  text_layer_destroy(min_layer);
  text_layer_destroy(hour_label_layer);
  text_layer_destroy(hour_layer);
}

static void prv_unobstructed_did_change(GRect bounds, void *context) {
  layer_set_frame(text_layer_get_layer(hour_layer), (GRect) { .origin = { 0, bounds.size.h/2-62 }, .size = { bounds.size.w, 65 } });
  layer_set_frame(text_layer_get_layer(hour_label_layer), (GRect) { .origin = { 0, bounds.size.h/2-12 }, .size = { bounds.size.w, 15 } });
  layer_set_frame(text_layer_get_layer(min_layer), (GRect) { .origin = { 0, bounds.size.h/2-8 }, .size = { bounds.size.w, 65 } });
  layer_set_frame(text_layer_get_layer(min_label_layer), (GRect) { .origin = { 0, bounds.size.h/2+42 }, .size = { bounds.size.w, 15 } });
  int step_bottom = bounds.size.h/2-62;
  APP_LOG(APP_LOG_LEVEL_INFO, "Step bottom is %d", step_bottom);
  if (step_bottom > STEP_LAYER_HEIGHT) {
    layer_set_frame(text_layer_get_layer(step_layer), (GRect) { .origin = { 0, 1 }, .size = { bounds.size.w, STEP_LAYER_HEIGHT } });
    layer_set_hidden(text_layer_get_layer(step_layer), false);
  }
  else {
    APP_LOG(APP_LOG_LEVEL_INFO, "No room for step count, missing %d px", step_bottom-STEP_LAYER_HEIGHT);
    layer_set_hidden(text_layer_get_layer(step_layer), true);
  }

  int date_top = bounds.size.h/2+57;
  APP_LOG(APP_LOG_LEVEL_INFO, "Date top is %d", date_top);
  if (date_top + DATE_LAYER_HEIGHT < bounds.size.h) {
    layer_set_frame(text_layer_get_layer(day_layer), (GRect) { .origin = { PBL_IF_ROUND_ELSE(0, bounds.size.w/2), date_top }, .size = { bounds.size.w, DATE_LAYER_HEIGHT } });
    layer_set_hidden(text_layer_get_layer(day_layer), false);
    layer_set_hidden(text_layer_get_layer(month_layer), false);
  }
  else {
    APP_LOG(APP_LOG_LEVEL_INFO, "No room for day and month, missing %d px", date_top + DATE_LAYER_HEIGHT - bounds.size.h);
    layer_set_hidden(text_layer_get_layer(day_layer), true);
    layer_set_hidden(text_layer_get_layer(month_layer), true);
  }
}

static void init(void) {
  shake = persist_exists(MESSAGE_KEY_SHAKE) ? persist_read_int(MESSAGE_KEY_SHAKE) : 1;
  hide_labels = persist_exists(MESSAGE_KEY_LABELS) ? persist_read_bool(MESSAGE_KEY_LABELS) : false;
  show_steps = persist_exists(MESSAGE_KEY_STEPS) ? persist_read_bool(MESSAGE_KEY_STEPS) : false;
  show_day = persist_exists(MESSAGE_KEY_DAY) ? persist_read_bool(MESSAGE_KEY_DAY) : false;
  show_month = persist_exists(MESSAGE_KEY_MONTH) ? persist_read_bool(MESSAGE_KEY_MONTH) : false;
  add = persist_exists(MESSAGE_KEY_ADD) ? persist_read_int(MESSAGE_KEY_ADD) : REGULARLY;
  subtract = persist_exists(MESSAGE_KEY_SUBTRACT) ? persist_read_int(MESSAGE_KEY_SUBTRACT) : REGULARLY;
  multiply = persist_exists(MESSAGE_KEY_MULTIPLY) ? persist_read_int(MESSAGE_KEY_MULTIPLY) : OFTEN;
  divide = persist_exists(MESSAGE_KEY_DIVIDE) ? persist_read_int(MESSAGE_KEY_DIVIDE) : RARELY;
  square = persist_exists(MESSAGE_KEY_SQUARE) ? persist_read_int(MESSAGE_KEY_SQUARE) : OFTEN;
  root = persist_exists(MESSAGE_KEY_ROOT) ? persist_read_int(MESSAGE_KEY_ROOT) : RARELY;
  accel_tap_service_subscribe(tap_handler);
  tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
  UnobstructedAreaHandlers handlers = {
    .will_change = prv_unobstructed_did_change,
  };
  unobstructed_area_service_subscribe(handlers, NULL);
}

static void deinit(void) {
  persist_write_int(MESSAGE_KEY_SHAKE, shake);
  persist_write_bool(MESSAGE_KEY_LABELS, hide_labels);
  persist_write_bool(MESSAGE_KEY_STEPS, show_steps);
  persist_write_bool(MESSAGE_KEY_DAY, show_day);
  persist_write_bool(MESSAGE_KEY_MONTH, show_month);
  persist_write_int(MESSAGE_KEY_ADD, add);
  persist_write_int(MESSAGE_KEY_SUBTRACT, subtract);
  persist_write_int(MESSAGE_KEY_MULTIPLY, multiply);
  persist_write_int(MESSAGE_KEY_DIVIDE, divide);
  persist_write_int(MESSAGE_KEY_SQUARE, square);
  persist_write_int(MESSAGE_KEY_ROOT, root);
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
