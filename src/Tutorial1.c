#include <pebble.h>

#define KEY_NAME      0
#define KEY_DISTANCE  1
#define KEY_DATE      2
#define KEY_CURRENT   3
#define KEY_AVERAGE   4

static Window *s_main_window;

static GFont s_time_font;

// This is a scroll layer
static ScrollLayer *s_scroll_layer;

// We also use a text layer to scroll in the scroll layer
static TextLayer *s_text_layer;

// Lorum ipsum to have something to scroll
static char s_scroll_text[10000] = "Loading...";

// Setup the scroll layer on window load
// We do this here in order to be able to get the max used text size
static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  GRect max_text_bounds = GRect(0, 0, bounds.size.w, 2000);

  // Initialize the scroll layer
  s_scroll_layer = scroll_layer_create(bounds);

  // This binds the scroll layer to the window so that up and down map to scrolling
  // You may use scroll_layer_set_callbacks to add or override interactivity
  scroll_layer_set_click_config_onto_window(s_scroll_layer, window);

  // Initialize the text layer
  s_text_layer = text_layer_create(max_text_bounds);
  text_layer_set_text(s_text_layer, s_scroll_text);

  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_20));

  // Apply to TextLayer
  text_layer_set_font(s_text_layer, s_time_font);

  // Trim text layer and scroll content to fit text box
  GSize max_size = text_layer_get_content_size(s_text_layer);
  text_layer_set_size(s_text_layer, max_size);
  scroll_layer_set_content_size(s_scroll_layer, GSize(bounds.size.w, bounds.size.h));

  // Add the layers for display
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_text_layer));

  layer_add_child(window_layer, scroll_layer_get_layer(s_scroll_layer));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
  scroll_layer_destroy(s_scroll_layer);
  fonts_unload_custom_font(s_time_font);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char name_buffer[32];
  static char distance_buffer[32];
  static char date_buffer[32];
  static char current_buffer[32];
  static char average_buffer[32];

  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_NAME:
      snprintf(name_buffer, sizeof(name_buffer), "%s", t->value->cstring);
      break;
    case KEY_DISTANCE:
      snprintf(distance_buffer, sizeof(distance_buffer), "%s", t->value->cstring);
      break;
    case KEY_DATE:
      snprintf(date_buffer, sizeof(date_buffer), "%s", t->value->cstring);
      break;
    case KEY_CURRENT:
      snprintf(current_buffer, sizeof(current_buffer), "%s", t->value->cstring);
      break;
    case KEY_AVERAGE:
      snprintf(average_buffer, sizeof(average_buffer), "%s", t->value->cstring);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }

  // Assemble full string and display
  snprintf(s_scroll_text, 10000, "Cur: %sµSv\nAve: %sµSv\nAddr: %s\nDist: %skm", current_buffer, average_buffer, name_buffer, distance_buffer);
  text_layer_set_text(s_text_layer, s_scroll_text);
  GSize content_size = text_layer_get_content_size(s_text_layer);
  text_layer_set_size(s_text_layer, content_size);
  scroll_layer_set_content_size(s_scroll_layer, GSize(148, content_size.h+4));
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
