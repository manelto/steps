/*
 * pebble steps
 * @author MTG
 */

#include <pebble.h>
#include <run.h>
  
 
// App-specific data
Window *window; // All apps must have at least one window
TextLayer *time_layer; 
TextLayer *day_layer; 
TextLayer *battery_layer;
TextLayer *connection_layer;

//parte accelerometer
TextLayer *text_layer_1, *text_layer_2;
char tap_text[11] = " Golpe:   ";
                     
void accel_handler(AccelData *data, uint32_t num_samples)
{
  // data is an array of num_samples elements.
  // num_samples was set when calling accel_data_service_subscribe.
  static char buffer[]= "XYZ: 9999 / 9999 / 9999";
  snprintf(buffer, sizeof("XYZ: 9999 / 9999 / 9999"), "XYZ: %d / %d / %d", data[0].x,data[0].y,data[0].z);
  text_layer_set_text(text_layer_1, buffer);
}

void tap_handler(AccelAxisType axis, int32_t direction)
{
  // Build a short message one character at a time to cover all possible taps.
 
  if (direction > 0)
  {
    tap_text[8] = '+';
  } else {
    tap_text[8] = '-';
  }
 
  if (axis == ACCEL_AXIS_X)
  {
    tap_text[9] = 'X';
  } else if (axis == ACCEL_AXIS_Y)
  {
    tap_text[9] = 'Y';
  } else if (axis == ACCEL_AXIS_Z)
  {
    tap_text[9] = 'Z';
  }
 
  // The last byte must be zero to indicate end of string.
  tap_text[10] = 0;
 
  text_layer_set_text(text_layer_2, tap_text);
}
//--------------------


static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100% cargado";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "cargando...");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%% cargado", charge_state.charge_percent);
  }
  text_layer_set_text(battery_layer, battery_text);
}

// Called once per second
static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {
  static char time_text[] = "00:00:00"; // Needs to be static because it's used by the system later.
  static char day_text[] = "- 00 / 00 / 00 -"; // Needs to be static because it's used by the system later.

  strftime(time_text, sizeof(time_text), "%T", tick_time);
  text_layer_set_text(time_layer, time_text);
  
  strftime(day_text, sizeof(day_text), " %d / %m / %y ", tick_time);
  text_layer_set_text(day_layer, day_text);

  handle_battery(battery_state_service_peek());
}

static void handle_bluetooth(bool connected) {
  text_layer_set_text(connection_layer, connected ? "conectado" : "desconectado");
}


 
void inicio(void)
{
  // Create our app's base window
  window = window_create();
  window_stack_push(window, true);
  window_set_background_color(window, GColorBlack);

  Layer *root_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(root_layer);
  
  
  //parte accelerometer
  Layer *window_layer = window_get_root_layer(window);
  
  text_layer_1 = text_layer_create(GRect(15, 0, 110, 18));
  text_layer_2 = text_layer_create(GRect(15, 20, 110, 18));
    
  layer_add_child(window_layer, text_layer_get_layer(text_layer_1));
  layer_add_child(window_layer, text_layer_get_layer(text_layer_2));
 
  accel_data_service_subscribe(1, accel_handler);
  accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
 
  accel_tap_service_subscribe(tap_handler);
  //----------------------
  

  // Init the text layer used to show the time
  time_layer = text_layer_create(GRect(0, 40, frame.size.w /* width */, 35/* height */));
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  
  day_layer = text_layer_create(GRect(0, 75, frame.size.w /* width */, 30/* height */));
  text_layer_set_text_color(day_layer, GColorWhite);
  text_layer_set_background_color(day_layer, GColorClear);
  text_layer_set_font(day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text_alignment(day_layer, GTextAlignmentCenter);

  connection_layer = text_layer_create(GRect(0, 98, /* width */ frame.size.w, 24 /* height */));
  text_layer_set_text_color(connection_layer, GColorWhite);
  text_layer_set_background_color(connection_layer, GColorClear);
  text_layer_set_font(connection_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(connection_layer, GTextAlignmentCenter);
  handle_bluetooth(bluetooth_connection_service_peek());

  battery_layer = text_layer_create(GRect(0, 120, /* width */ frame.size.w, 34 /* height */));
  text_layer_set_text_color(battery_layer, GColorWhite);
  text_layer_set_background_color(battery_layer, GColorClear);
  text_layer_set_font(battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(battery_layer, GTextAlignmentCenter);
  text_layer_set_text(battery_layer, "100% cargado");

  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_second_tick(current_time, SECOND_UNIT);

  tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);
  battery_state_service_subscribe(&handle_battery);
  bluetooth_connection_service_subscribe(&handle_bluetooth);

  layer_add_child(root_layer, text_layer_get_layer(time_layer));
  layer_add_child(root_layer, text_layer_get_layer(day_layer));
  layer_add_child(root_layer, text_layer_get_layer(connection_layer));
  layer_add_child(root_layer, text_layer_get_layer(battery_layer));
}
 
void cierre(void)
{
  //parte accelerometer
  accel_data_service_unsubscribe();
  accel_tap_service_unsubscribe();
 
  text_layer_destroy(text_layer_2);
  text_layer_destroy(text_layer_1);
  //---------------------
  
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  text_layer_destroy(time_layer);
  text_layer_destroy(connection_layer);
  text_layer_destroy(battery_layer);
  window_destroy(window);
}
