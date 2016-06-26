#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_real_time;
static TextLayer *s_time_line1;
static TextLayer *s_time_line1_shaddow;
static TextLayer *s_time_line2;
static TextLayer *s_time_line2_shaddow;
static TextLayer *s_time_line3;
static TextLayer *s_date_line1;
static TextLayer *s_date_line2;
static TextLayer *s_date_line3;
static TextLayer *s_date_line4;

static TextLayer *s_accel_line1;

static int s_battery_level;

  static GColor8 bgColor; 
  static GColor8 textColor;
  static GColor8 shadowColor;
    
static GColor8 bgColorDateCircle; 
  static GColor8 textColorDateCircle;  
  static GColor8 colorShortHand;  
  static GColor8 colorLongHand;  
  static GColor8 colorSecondHand;  
  static GColor8 colorTimePoints;  


int16_t accel_y =0;
int16_t y_border = 750; 
int akt_tick;
 

static Layer *window_layer;
static Layer *s_canvas_layer;
static Layer *s_hand_layer;
static bool showFuzzy = false;

static GBitmap *s_bitmap_bt;
static BitmapLayer *s_bitmap_layer;
static bool btConnected;



static bool showFuzzyFix = false;


/////////////////////////////////////////////////////////////////////////////////////
/////                       Daten für die Uhrzeit 

static char * getMinute() {
  
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  int min = tick_time->tm_min;
  
  if (min >= 0 && min <=2) return "";
  else if (min >= 3 && min <=7) return "kurz";
  else if (min >= 8 && min <=12) return "zehn";  
  else if (min >= 13 && min <=17) return "viertel";  
  else if (min >= 18 && min <=22) return "zehn";  
  else if (min >= 23 && min <=27) return "kurz";  
  else if (min >= 28 && min <=32) return "";  
  else if (min >= 33 && min <=37) return "kurz";  
  else if (min >= 38 && min <=42) return "zehn";  
  else if (min >= 43 && min <=47) return "viertel"; 
  else if (min >= 48 && min <=52) return "zehn";  
  else if (min >= 53 && min <=57) return "kurz";  
  else return "";  
}
 
static char * getBeforeAfter() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  int min = tick_time->tm_min;  
  
  if (min >= 0 && min <=2) return "";
  else if (min >= 3 && min <=17) return "nach";
  else if (min >= 18 && min <=27) return "vor halb";  
  else if (min >= 28 && min <=32) return "halb";  
  else if (min >= 33 && min <=42) return "nach halb";  
  else if (min >= 43 && min <=57) return "vor";  
  else return "";  
  
}

static char * getHour() {
 time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  int min = tick_time->tm_min;  
  int hour = tick_time->tm_hour;  
  

  
  if (min >= 18 && min <=59 && hour < 23) hour++;
  else if (min >= 18 && min <=59 && hour == 23) hour = 0;
    
  switch (hour) {
    case 0: return "zwölf"; break;
    case 1: return "eins"; break;
    case 2: return "zwei"; break;
    case 3: return "drei"; break;
    case 4: return "vier"; break;
    case 5: return "fünf"; break;
    case 6: return "sechs"; break;
    case 7: return "sieben"; break;
    case 8: return "acht"; break;
    case 9: return "neun"; break;
    case 10: return "zehn"; break;
    case 11: return "elf"; break;
    case 12: return "zwölf"; break;
    case 13: return "eins"; break;
    case 14: return "zwei"; break;
    case 15: return "drei"; break;
    case 16: return "vier"; break;
    case 17: return "fünf"; break;
    case 18: return "sechs"; break;
    case 19: return "sieben"; break;
    case 20: return "acht"; break;
    case 21: return "neun"; break;
    case 22: return "zehn"; break;
    case 23: return "elf"; break;
 }
    
  return "";
}


static char * getDay() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  int wday = tick_time->tm_wday;  
 
  switch (wday) {
    case 0:return "Sonntag,"; break;
    case 1:return "Montag,"; break;
    case 2:return "Dienstag,"; break;
    case 3:return "Mittwoch,"; break;
    case 4:return "Donnerstag,"; break;
    case 5:return "Freitag,"; break;
    case 6:return "Samstag,"; break;
  }
  
  return "";
}    
  
static char * getDate() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  int mday = tick_time->tm_mday; 
  int mon = tick_time->tm_mon; 
 
  static char s_buffer[50];  
  
  if (  !showFuzzy) { 
    snprintf(s_buffer, 20, "%u.%u.", mday,mon+1);
  } else {
    
  switch (mon) {
    case 0:snprintf(s_buffer, 20, "%u. Jan", mday); break;
    case 1:snprintf(s_buffer, 20, "%u. Feb", mday); break;
    case 2:snprintf(s_buffer, 20, "%u. Mär", mday); break;
    case 3:snprintf(s_buffer, 20, "%u. Apr", mday); break;
    case 4:snprintf(s_buffer, 20, "%u. Mai", mday); break;
    case 5:snprintf(s_buffer, 20, "%u. Jun", mday); break;
    case 6:snprintf(s_buffer, 20, "%u. Jul", mday); break;
    case 7:snprintf(s_buffer, 20, "%u. Aug", mday); break;
    case 8:snprintf(s_buffer, 20, "%u. Sep", mday); break;
    case 9:snprintf(s_buffer, 20, "%u. Okt", mday); break;
    case 10:snprintf(s_buffer, 20, "%u. Nov", mday); break;
    case 11:snprintf(s_buffer, 20, "%u. Dez", mday); break;
  }
  }
  return s_buffer;
}   



static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;
}

static void app_connection_handler(bool connected) {
  if (connected) {
    vibes_short_pulse();
  } else {
    vibes_double_pulse();
  }
  btConnected = connected;
  
  if (!showFuzzy) {
    layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layer), btConnected );
  }
}

/////////////////////////////////////////////////////////////////////////////////////
/////      Anzeige der Uhrzeit



static GPoint getHandPoint(GPoint center,double deg,int len) {
  
 

  GPoint handPoint = {
    .x = (int16_t)(sin_lookup(DEG_TO_TRIGANGLE(deg)) * (int32_t)len / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(DEG_TO_TRIGANGLE(deg)) * (int32_t)len / TRIG_MAX_RATIO) + center.y,
  };
  return handPoint;
}


static void drawBattery( GContext *ctx,GPoint center){
  graphics_context_set_stroke_color(ctx, GColorBlack);
  // Body
  GRect batPart;
  batPart.size.w=10;
  batPart.size.h=16;
  batPart.origin.x=center.x/2;
  batPart.origin.y=center.y-12;      
  graphics_draw_round_rect(ctx, batPart, 2);
  // Nipple
  batPart.size.w=6;
  batPart.size.h=3;
  batPart.origin.x=center.x/2+2;
  batPart.origin.y=center.y-14;      
  graphics_draw_rect(ctx, batPart);
  // Fill
  batPart.size.w=8;
  batPart.size.h=s_battery_level*14/100;
  batPart.origin.x=center.x/2+1;
  batPart.origin.y=center.y-11+(14-(s_battery_level*14/100));
  if (s_battery_level > 20) {
     graphics_context_set_fill_color(ctx, GColorGreen);  
   
  } else {
     graphics_context_set_fill_color(ctx, GColorRed);  
   
  }
  graphics_fill_rect(ctx, batPart, 0, GCornersAll);  
}

static void hand_update_proc(Layer *layer, GContext *ctx) {
 

  GRect bounds = layer_get_bounds(window_layer);
 // GRect frame = grect_inset(bounds, GEdgeInsets(1));
  GPoint center = grect_center_point(&bounds);
 
  
   if ( !showFuzzy) {  
  
   time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  double lhStart = 360*((tick_time->tm_hour % 12)*60+tick_time->tm_min)/(12*60)+1;

  double lmStart = tick_time->tm_min*360/60+1; 

  double lsStart =tick_time->tm_sec*360/60+1;

  //static char s_buffer[30];
  //snprintf(s_buffer, sizeof(s_buffer), " h: %d, m: %d, s: %d",  (int)lhStart, (int)lmStart, (int)lsStart);
  //text_layer_set_text(s_accel_line1, s_buffer);
 
   
     
  graphics_context_set_stroke_color(ctx, colorShortHand);
  graphics_context_set_stroke_width(ctx, 4);
  graphics_draw_line(ctx,center,getHandPoint(center, lhStart, 60));
  
  graphics_context_set_stroke_color(ctx, colorLongHand);
  graphics_draw_line(ctx,center,getHandPoint(center, lmStart, 80));
  
  graphics_context_set_stroke_color(ctx, colorSecondHand);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx,center,getHandPoint(center, lsStart, 80));

   }
  
}
static void canvas_update_proc(Layer *layer, GContext *ctx) {
  // Custom drawing happens here!

  GRect bounds = layer_get_bounds(window_layer);
 // GRect frame = grect_inset(bounds, GEdgeInsets(1));
  GPoint center = grect_center_point(&bounds);
 
  
   if ( !showFuzzy) {  
  
 


  //static char s_buffer[30];
  //snprintf(s_buffer, sizeof(s_buffer), " h: %d, m: %d, s: %d",  (int)lhStart, (int)lmStart, (int)lsStart);
  //text_layer_set_text(s_accel_line1, s_buffer);
 
   GRect iconWindow;
  iconWindow.size.w=35;
  iconWindow.size.h=26;
  iconWindow.origin.x=center.x/2-5;
  iconWindow.origin.y=center.y-18;   
  graphics_context_set_fill_color(ctx, GColorWhite);  
  graphics_fill_rect(ctx, iconWindow, 2, GCornersAll);
  
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  iconWindow.size.w=33;
  iconWindow.size.h=24;
  iconWindow.origin.x=center.x/2-4;
  iconWindow.origin.y=center.y-17;      
  graphics_draw_round_rect(ctx, iconWindow, 2);         
     
     
  drawBattery(ctx,center);
     
  
  
     
  for (int i=0;i<12;i++) {  
     double point = 360*i/12+1;
     graphics_context_set_stroke_color(ctx, colorTimePoints);
     graphics_context_set_stroke_width(ctx, 2);
     graphics_draw_circle(ctx, getHandPoint(center, point, 80), 2);
  }
     
for (int i=0;i<60;i++) {  
     double point = 360*i/60+1;
     graphics_draw_pixel(ctx, getHandPoint(center, point, 80));
  }     
   

      
    
  
  
   } else {
     graphics_context_set_stroke_color(ctx, bgColorDateCircle);
     graphics_context_set_stroke_width(ctx, 1);
     graphics_context_set_fill_color(ctx, bgColorDateCircle);
     GPoint point = center;
     point.y = point.y*4-45;
     graphics_fill_circle(ctx, point , center.x*2);
    
   }
  
  
}



static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
 
   if ( !showFuzzy ) {  
  text_layer_set_text(s_real_time, "");
  
  text_layer_set_text(s_time_line1, "");
  text_layer_set_text(s_time_line1_shaddow, "");
  text_layer_set_text(s_time_line2, "");
  text_layer_set_text(s_time_line2_shaddow, "");
  text_layer_set_text(s_time_line3, "");
  text_layer_set_text(s_date_line1, "");
  text_layer_set_text(s_date_line2, "");
  text_layer_set_text(s_date_line3, getDay());
  text_layer_set_text(s_date_line4, getDate());
   
  
   } else {
  text_layer_set_text(s_real_time, s_buffer);
  
  text_layer_set_text(s_time_line1, getMinute());
     text_layer_set_text(s_time_line1_shaddow, getMinute());
  
  if (strcmp(getBeforeAfter(), "") == 0) {
     text_layer_set_font(s_time_line2, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
     
     text_layer_set_text(s_time_line2, getHour());
     text_layer_set_text(s_time_line2_shaddow, "");
    text_layer_set_text(s_time_line3, "");
  } else {
    
 text_layer_set_font(s_time_line2, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
   text_layer_set_text(s_time_line2, getBeforeAfter());
  text_layer_set_text(s_time_line2_shaddow, getBeforeAfter());
  text_layer_set_text(s_time_line3, getHour());
    
  }
     

     
   text_layer_set_text(s_date_line1, getDay());
   text_layer_set_text(s_date_line2, getDate());     
   text_layer_set_text(s_date_line3, "");     
   text_layer_set_text(s_date_line4, "");     
  
 
   }


 

}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
 update_time();
}



/////////////////////////////////////////////////////////////////////////////////////
/////                      Bewegungsdaten

static void accel_data_handler(AccelData *data, uint32_t num_samples) {
  
  // Read sample 0's x, y, and z values
  int16_t x = data[0].x;
  int16_t y = data[0].y;
  int16_t z = data[0].z;

  // Determine if the sample occured during vibration, and when it occured
  bool did_vibrate = data[0].did_vibrate;
 // uint64_t timestamp = data[0].timestamp;

  if(!did_vibrate) {
    static char s_buffer[30];
    snprintf(s_buffer, sizeof(s_buffer), " x: %d, y: %d, z: %d",  x, y, z);
    //text_layer_set_text(s_accel_line1, s_buffer);
    accel_y = y;
 
  if (!showFuzzyFix) {
     showFuzzy = accel_y >= -y_border; 
  }

    
    
 if ( !showFuzzy) {
    
    if (akt_tick != SECOND_UNIT) {
      tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
      akt_tick = SECOND_UNIT;  
       update_time();
       layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layer), btConnected);
    }
  
  }  else {
     if (akt_tick != MINUTE_UNIT) {
      tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
      akt_tick = MINUTE_UNIT;    
      update_time();
      layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layer), true);

 
    }
  }
      
    
    
  } else {
    // Discard with a warning
    APP_LOG(APP_LOG_LEVEL_WARNING, "Vibration occured during collection");
  }
}



static void accelTapHandler(AccelAxisType axis, int32_t direction) {
  char achse[10];
  switch (axis) {
   case ACCEL_AXIS_X:  snprintf(achse,10,"X %ld",direction); break;
   case ACCEL_AXIS_Y:  snprintf(achse,10,"Y %ld",direction); break;
   case ACCEL_AXIS_Z:  snprintf(achse,10,"Z %ld",direction); break;
  }
  

  
  APP_LOG(APP_LOG_LEVEL_INFO, "TAP: %s",achse );
}


/////////////////////////////////////////////////////////////////////////////////////
/////      Initialisierung / Deinitialisierung


static void main_window_load(Window *window) {
  
  bgColor = GColorBlack;
  textColor  = GColorWhite;
  shadowColor = GColorDarkGray;
  bgColorDateCircle  = GColorFromHEX(0x77AAFF);
  textColorDateCircle  = GColorBlack;
  colorShortHand = GColorWhite;
  colorLongHand = GColorWhite;
  colorSecondHand = GColorRed;
  colorTimePoints = GColorLightGray;

  
  
  window_set_background_color(window,bgColor);
  
 // Get information about the Window
  window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  s_canvas_layer = layer_create(bounds);
   s_hand_layer = layer_create(bounds);
  GPoint center = grect_center_point(&bounds);

  // Add Bluetooth Icon
s_bitmap_bt = gbitmap_create_with_resource(RESOURCE_ID_IMG_BLUETOOTH);
GRect btRect;
btRect.size.w = 16;
btRect.size.h = 16;
btRect.origin.x = center.x/2+12;
btRect.origin.y = center.y-14;
  



  
  // Create the TextLayer with specific bounds
  s_real_time = text_layer_create(
               GRect(0, 3, bounds.size.w, 50));
  
  s_time_line1_shaddow = text_layer_create(
               GRect(1, 17, bounds.size.w, 50));
  s_time_line1 = text_layer_create(
               GRect(0, 16, bounds.size.w, 50));

  s_time_line2_shaddow = text_layer_create(
               GRect(1, 51, bounds.size.w, 50)); 
  s_time_line2 = text_layer_create(
               GRect(0, 50, bounds.size.w, 50));  
  s_time_line3 = text_layer_create(
               GRect(0, 84, bounds.size.w, 50));
  
  s_date_line1 = text_layer_create(
               GRect(0, 136, bounds.size.w, 50));  
  s_date_line2 = text_layer_create(
               GRect(0, 154, bounds.size.w, 50));  

  
  s_date_line3 = text_layer_create(
               GRect(0, 118, bounds.size.w, 50));
  s_date_line4 = text_layer_create(
               GRect(0, 136, bounds.size.w, 50));
  
  s_accel_line1 = text_layer_create(
               GRect(0, 128, bounds.size.w, 50));
   
 
  layer_add_child(window_get_root_layer(window), s_canvas_layer);

  s_bitmap_layer = bitmap_layer_create(btRect);
  
  
  
  bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpSet);

  bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap_bt); 
  
  layer_add_child(window_get_root_layer(window), 
                                      bitmap_layer_get_layer(s_bitmap_layer));
  
  layer_add_child(window_get_root_layer(window), s_hand_layer);
  
  layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layer),true);
  
  text_layer_set_background_color(s_real_time, GColorClear);
  text_layer_set_text_color(s_real_time, textColor);
  text_layer_set_font(s_real_time, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_real_time, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_real_time));  
  
  text_layer_set_background_color(s_time_line1_shaddow, GColorClear);
  text_layer_set_text_color(s_time_line1_shaddow, shadowColor);
  text_layer_set_font(s_time_line1_shaddow, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  text_layer_set_text_alignment(s_time_line1_shaddow, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_line1_shaddow));
  
  text_layer_set_background_color(s_time_line1, GColorClear);
  text_layer_set_text_color(s_time_line1, textColor);
  text_layer_set_font(s_time_line1, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  text_layer_set_text_alignment(s_time_line1, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_line1));  
  
  text_layer_set_background_color(s_time_line2_shaddow, GColorClear);
  text_layer_set_text_color(s_time_line2_shaddow, shadowColor);
  text_layer_set_font(s_time_line2_shaddow, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  text_layer_set_text_alignment(s_time_line2_shaddow, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_line2_shaddow));
  
  text_layer_set_background_color(s_time_line2, GColorClear);
  text_layer_set_text_color(s_time_line2, textColor);
  text_layer_set_font(s_time_line2, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  text_layer_set_text_alignment(s_time_line2, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_line2));
  
  text_layer_set_background_color(s_time_line3, GColorClear);
  text_layer_set_text_color(s_time_line3, textColor);
  text_layer_set_font(s_time_line3, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_line3, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_line3));
 
  text_layer_set_background_color(s_date_line1, GColorClear);
  text_layer_set_text_color(s_date_line1, textColorDateCircle);
  text_layer_set_font(s_date_line1, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_date_line1, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_date_line1));

  text_layer_set_background_color(s_date_line2, GColorClear);
  text_layer_set_text_color(s_date_line2, textColorDateCircle);
  text_layer_set_font(s_date_line2, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_date_line2, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_date_line2));

  text_layer_set_background_color(s_date_line3, GColorClear);
  text_layer_set_text_color(s_date_line3, textColor);
  text_layer_set_font(s_date_line3, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_date_line3, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_date_line3));

  text_layer_set_background_color(s_date_line4, GColorClear);
  text_layer_set_text_color(s_date_line4, textColor);
  text_layer_set_font(s_date_line4, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_date_line4, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_date_line4));


  text_layer_set_background_color(s_accel_line1, GColorClear);
  text_layer_set_text_color(s_accel_line1, textColor);
  text_layer_set_font(s_accel_line1, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_accel_line1, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_accel_line1));
  
  // Assign the custom drawing procedure
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_set_update_proc(s_hand_layer, hand_update_proc);
  



  // Subscribe to tap events
  accel_tap_service_subscribe(accelTapHandler);
  
  
  
  
  uint32_t num_samples = 10;  // Number of samples per batch/callback

  // Subscribe to batched data events
  accel_data_service_subscribe(num_samples, accel_data_handler);
  
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = app_connection_handler,
    .pebblekit_connection_handler = NULL
   });
  btConnected = bluetooth_connection_service_peek();
  
  
  // Register for battery level updates
  battery_state_service_subscribe(battery_callback);
  BatteryChargeState bstate;
  bstate = battery_state_service_peek();
  
  s_battery_level = bstate.charge_percent;
  
  
  update_time();
}






static void main_window_unload(Window *window) {
 // Destroy TextLayer
  text_layer_destroy(s_real_time);
  text_layer_destroy(s_time_line1);
  text_layer_destroy(s_time_line1_shaddow);
  text_layer_destroy(s_time_line2);
  text_layer_destroy(s_time_line2_shaddow);
  text_layer_destroy(s_time_line3);
  text_layer_destroy(s_date_line1);
  text_layer_destroy(s_date_line2);
  text_layer_destroy(s_date_line3);
  text_layer_destroy(s_date_line4);
  text_layer_destroy(s_accel_line1);
  
  gbitmap_destroy(s_bitmap_bt);
  bitmap_layer_destroy(s_bitmap_layer);

  
  
  // Unsubscribe from tap events
  accel_data_service_unsubscribe();
  accel_tap_service_unsubscribe();
  connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  
  
  
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
  
  // Register with TickTimerService
  akt_tick = SECOND_UNIT;  
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  

  
  // Load the image data
  s_bitmap_bt = gbitmap_create_with_resource(RESOURCE_ID_IMG_BLUETOOTH);
  
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

