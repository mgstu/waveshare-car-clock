#include "Clock_UI.h"
#include <math.h>
#include <stdio.h>

namespace {

constexpr int SCREEN_SIZE = 480;
constexpr int CLOCK_CX = 240;
constexpr int CLOCK_CY = 240;
constexpr float PI_F = 3.14159265358979323846f;

enum ClockFace {
  FACE_DIGITAL,
  FACE_ANALOGUE
};

ClockFace current_face = FACE_DIGITAL;
bool long_press_seen = false;

lv_obj_t *digital_screen = nullptr;
lv_obj_t *analogue_screen = nullptr;
lv_obj_t *settings_panel = nullptr;

lv_obj_t *time_label = nullptr;
lv_obj_t *seconds_label = nullptr;
lv_obj_t *day_label = nullptr;
lv_obj_t *date_label = nullptr;
lv_obj_t *battery_label = nullptr;

lv_obj_t *analogue_time_label = nullptr;
lv_obj_t *hour_hand = nullptr;
lv_obj_t *minute_hand = nullptr;
lv_obj_t *second_hand = nullptr;
lv_point_t hour_points[2];
lv_point_t minute_points[2];
lv_point_t second_points[2];

lv_obj_t *set_hour_label = nullptr;
lv_obj_t *set_minute_label = nullptr;
int edit_hour = 0;
int edit_minute = 0;

const char *weekday_names[] = {
  "Sunday", "Monday", "Tuesday", "Wednesday",
  "Thursday", "Friday", "Saturday"
};

const char *month_names[] = {
  "January", "February", "March", "April", "May", "June",
  "July", "August", "September", "October", "November", "December"
};

bool valid_datetime(const datetime_t &dt) {
  return dt.hour < 24 && dt.minute < 60 && dt.second < 60 &&
         dt.month >= 1 && dt.month <= 12 && dt.day >= 1 && dt.day <= 31 &&
         dt.dotw <= 6;
}

void set_screen_background(lv_obj_t *obj) {
  lv_obj_set_style_bg_color(obj, lv_color_hex(0x05070A), 0);
  lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(obj, 0, 0);
  lv_obj_set_style_radius(obj, LV_RADIUS_CIRCLE, 0);
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

void style_label(lv_obj_t *label, uint32_t colour) {
  lv_obj_set_style_text_color(label, lv_color_hex(colour), 0);
  lv_obj_set_style_text_font(label, LV_FONT_DEFAULT, 0);
}

void draw_hand(lv_obj_t *line, lv_point_t *points, float angle_deg, int length) {
  const float radians = (angle_deg - 90.0f) * PI_F / 180.0f;
  points[0].x = CLOCK_CX;
  points[0].y = CLOCK_CY;
  points[1].x = CLOCK_CX + static_cast<int>(cosf(radians) * length);
  points[1].y = CLOCK_CY + static_cast<int>(sinf(radians) * length);
  lv_line_set_points(line, points, 2);
}

void update_settings_labels() {
  char buffer[8];
  snprintf(buffer, sizeof(buffer), "%02d", edit_hour);
  lv_label_set_text(set_hour_label, buffer);
  snprintf(buffer, sizeof(buffer), "%02d", edit_minute);
  lv_label_set_text(set_minute_label, buffer);
}

void close_settings() {
  if (settings_panel != nullptr) {
    lv_obj_del(settings_panel);
    settings_panel = nullptr;
  }
}

void settings_button_event(lv_event_t *event) {
  const intptr_t action = reinterpret_cast<intptr_t>(lv_event_get_user_data(event));

  switch (action) {
    case 1: edit_hour = (edit_hour + 1) % 24; break;
    case 2: edit_hour = (edit_hour + 23) % 24; break;
    case 3: edit_minute = (edit_minute + 1) % 60; break;
    case 4: edit_minute = (edit_minute + 59) % 60; break;
    case 5: {
      datetime_t updated;
      PCF85063_Read_Time(&updated);
      updated.hour = edit_hour;
      updated.minute = edit_minute;
      updated.second = 0;
      PCF85063_Set_Time(updated);
      datetime = updated;
      close_settings();
      return;
    }
    case 6:
      close_settings();
      return;
  }

  update_settings_labels();
}

lv_obj_t *make_button(lv_obj_t *parent, const char *text, int x, int y,
                      int width, int height, intptr_t action) {
  lv_obj_t *button = lv_btn_create(parent);
  lv_obj_set_size(button, width, height);
  lv_obj_set_pos(button, x, y);
  lv_obj_set_style_radius(button, 18, 0);
  lv_obj_set_style_bg_color(button, lv_color_hex(0x1C2733), 0);
  lv_obj_set_style_bg_color(button, lv_color_hex(0x30465B), LV_STATE_PRESSED);
  lv_obj_add_event_cb(button, settings_button_event, LV_EVENT_CLICKED,
                      reinterpret_cast<void *>(action));

  lv_obj_t *label = lv_label_create(button);
  lv_label_set_text(label, text);
  lv_obj_set_style_text_font(label, LV_FONT_DEFAULT, 0);
  lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
  lv_obj_center(label);
  return button;
}

void open_settings() {
  if (settings_panel != nullptr) return;

  datetime_t now;
  PCF85063_Read_Time(&now);
  edit_hour = now.hour < 24 ? now.hour : 0;
  edit_minute = now.minute < 60 ? now.minute : 0;

  settings_panel = lv_obj_create(lv_scr_act());
  lv_obj_set_size(settings_panel, 420, 340);
  lv_obj_center(settings_panel);
  lv_obj_set_style_radius(settings_panel, 36, 0);
  lv_obj_set_style_bg_color(settings_panel, lv_color_hex(0x0D141C), 0);
  lv_obj_set_style_bg_opa(settings_panel, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(settings_panel, 2, 0);
  lv_obj_set_style_border_color(settings_panel, lv_color_hex(0x2B91D0), 0);
  lv_obj_clear_flag(settings_panel, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *title = lv_label_create(settings_panel);
  lv_label_set_text(title, "SET TIME");
  style_label(title, 0x5BC0FF);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

  set_hour_label = lv_label_create(settings_panel);
  style_label(set_hour_label, 0xFFFFFF);
  lv_obj_set_style_transform_zoom(set_hour_label, 420, 0);
  lv_obj_set_pos(set_hour_label, 92, 132);

  lv_obj_t *colon = lv_label_create(settings_panel);
  lv_label_set_text(colon, ":");
  style_label(colon, 0x5BC0FF);
  lv_obj_set_style_transform_zoom(colon, 420, 0);
  lv_obj_set_pos(colon, 195, 132);

  set_minute_label = lv_label_create(settings_panel);
  style_label(set_minute_label, 0xFFFFFF);
  lv_obj_set_style_transform_zoom(set_minute_label, 420, 0);
  lv_obj_set_pos(set_minute_label, 245, 132);

  make_button(settings_panel, "+", 65, 70, 100, 52, 1);
  make_button(settings_panel, "-", 65, 210, 100, 52, 2);
  make_button(settings_panel, "+", 255, 70, 100, 52, 3);
  make_button(settings_panel, "-", 255, 210, 100, 52, 4);
  make_button(settings_panel, "SAVE", 75, 280, 120, 48, 5);
  make_button(settings_panel, "CANCEL", 225, 280, 120, 48, 6);

  update_settings_labels();
}

void show_face(ClockFace face) {
  current_face = face;
  if (face == FACE_DIGITAL) {
    lv_obj_clear_flag(digital_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(analogue_screen, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(digital_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(analogue_screen, LV_OBJ_FLAG_HIDDEN);
  }
}

void screen_event(lv_event_t *event) {
  const lv_event_code_t code = lv_event_get_code(event);

  if (code == LV_EVENT_PRESSED) {
    long_press_seen = false;
  } else if (code == LV_EVENT_LONG_PRESSED) {
    long_press_seen = true;
    open_settings();
  } else if (code == LV_EVENT_CLICKED) {
    if (!long_press_seen && settings_panel == nullptr) {
      show_face(current_face == FACE_DIGITAL ? FACE_ANALOGUE : FACE_DIGITAL);
    }
    long_press_seen = false;
  }
}

void add_full_screen_touch_layer(lv_obj_t *parent) {
  // Keep one transparent, clickable object above every clock-face element.
  // This prevents clock hands, labels and markers intercepting the tap.
  lv_obj_t *touch_layer = lv_obj_create(parent);
  lv_obj_set_size(touch_layer, SCREEN_SIZE, SCREEN_SIZE);
  lv_obj_set_pos(touch_layer, 0, 0);
  lv_obj_set_style_bg_opa(touch_layer, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(touch_layer, 0, 0);
  lv_obj_set_style_pad_all(touch_layer, 0, 0);
  lv_obj_set_style_radius(touch_layer, 0, 0);
  lv_obj_clear_flag(touch_layer, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(touch_layer, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(touch_layer, screen_event, LV_EVENT_ALL, nullptr);
}

void create_digital_screen(lv_obj_t *parent) {
  digital_screen = lv_obj_create(parent);
  lv_obj_set_size(digital_screen, SCREEN_SIZE, SCREEN_SIZE);
  lv_obj_set_pos(digital_screen, 0, 0);
  set_screen_background(digital_screen);

  lv_obj_t *accent = lv_obj_create(digital_screen);
  lv_obj_set_size(accent, 220, 5);
  lv_obj_align(accent, LV_ALIGN_TOP_MID, 0, 72);
  lv_obj_set_style_radius(accent, 3, 0);
  lv_obj_set_style_border_width(accent, 0, 0);
  lv_obj_set_style_bg_color(accent, lv_color_hex(0x2B91D0), 0);

  time_label = lv_label_create(digital_screen);
  lv_label_set_text(time_label, "00:00");
  style_label(time_label, 0xFFFFFF);
  lv_obj_set_style_transform_zoom(time_label, 900, 0);
  lv_obj_align(time_label, LV_ALIGN_CENTER, 0, -60);

  seconds_label = lv_label_create(digital_screen);
  lv_label_set_text(seconds_label, ":00");
  style_label(seconds_label, 0x5BC0FF);
  lv_obj_align(seconds_label, LV_ALIGN_CENTER, 118, -15);

  day_label = lv_label_create(digital_screen);
  lv_label_set_text(day_label, "Thursday");
  style_label(day_label, 0x5BC0FF);
  lv_obj_align(day_label, LV_ALIGN_CENTER, 0, 48);

  date_label = lv_label_create(digital_screen);
  lv_label_set_text(date_label, "30 July 2026");
  lv_obj_set_style_text_font(date_label, LV_FONT_DEFAULT, 0);
  lv_obj_set_style_text_color(date_label, lv_color_hex(0xBAC6D1), 0);
  lv_obj_align(date_label, LV_ALIGN_CENTER, 0, 88);

  battery_label = lv_label_create(digital_screen);
  lv_label_set_text(battery_label, "BAT --.--V");
  lv_obj_set_style_text_font(battery_label, LV_FONT_DEFAULT, 0);
  lv_obj_set_style_text_color(battery_label, lv_color_hex(0x708090), 0);
  lv_obj_align(battery_label, LV_ALIGN_BOTTOM_MID, 0, -56);

  lv_obj_t *hint = lv_label_create(digital_screen);
  lv_label_set_text(hint, "TAP TO CHANGE  |  HOLD TO SET");
  lv_obj_set_style_text_font(hint, LV_FONT_DEFAULT, 0);
  lv_obj_set_style_text_color(hint, lv_color_hex(0x506070), 0);
  lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -28);

  add_full_screen_touch_layer(digital_screen);
}

void add_hour_marker(lv_obj_t *parent, int hour) {
  const float angle = (hour * 30.0f - 90.0f) * PI_F / 180.0f;
  const int x = CLOCK_CX + static_cast<int>(cosf(angle) * 176.0f);
  const int y = CLOCK_CY + static_cast<int>(sinf(angle) * 176.0f);

  lv_obj_t *marker = lv_obj_create(parent);
  const int marker_size = (hour % 3 == 0) ? 12 : 7;
  lv_obj_set_size(marker, marker_size, marker_size);
  lv_obj_set_style_radius(marker, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_border_width(marker, 0, 0);
  lv_obj_set_style_bg_color(marker,
                            lv_color_hex(hour % 3 == 0 ? 0x5BC0FF : 0x657584), 0);
  lv_obj_set_pos(marker, x - marker_size / 2, y - marker_size / 2);
  lv_obj_clear_flag(marker, LV_OBJ_FLAG_SCROLLABLE);
}

void create_analogue_screen(lv_obj_t *parent) {
  analogue_screen = lv_obj_create(parent);
  lv_obj_set_size(analogue_screen, SCREEN_SIZE, SCREEN_SIZE);
  lv_obj_set_pos(analogue_screen, 0, 0);
  set_screen_background(analogue_screen);

  lv_obj_t *rim = lv_obj_create(analogue_screen);
  lv_obj_set_size(rim, 410, 410);
  lv_obj_center(rim);
  lv_obj_set_style_radius(rim, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(rim, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(rim, 3, 0);
  lv_obj_set_style_border_color(rim, lv_color_hex(0x263847), 0);
  lv_obj_clear_flag(rim, LV_OBJ_FLAG_SCROLLABLE);

  for (int hour = 0; hour < 12; ++hour) add_hour_marker(analogue_screen, hour);

  hour_hand = lv_line_create(analogue_screen);
  lv_obj_set_style_line_width(hour_hand, 12, 0);
  lv_obj_set_style_line_rounded(hour_hand, true, 0);
  lv_obj_set_style_line_color(hour_hand, lv_color_hex(0xFFFFFF), 0);

  minute_hand = lv_line_create(analogue_screen);
  lv_obj_set_style_line_width(minute_hand, 7, 0);
  lv_obj_set_style_line_rounded(minute_hand, true, 0);
  lv_obj_set_style_line_color(minute_hand, lv_color_hex(0xB8C5D0), 0);

  second_hand = lv_line_create(analogue_screen);
  lv_obj_set_style_line_width(second_hand, 3, 0);
  lv_obj_set_style_line_rounded(second_hand, true, 0);
  lv_obj_set_style_line_color(second_hand, lv_color_hex(0x2B91D0), 0);

  lv_obj_t *hub = lv_obj_create(analogue_screen);
  lv_obj_set_size(hub, 24, 24);
  lv_obj_set_pos(hub, CLOCK_CX - 12, CLOCK_CY - 12);
  lv_obj_set_style_radius(hub, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_border_width(hub, 0, 0);
  lv_obj_set_style_bg_color(hub, lv_color_hex(0x2B91D0), 0);
  lv_obj_clear_flag(hub, LV_OBJ_FLAG_SCROLLABLE);

  analogue_time_label = lv_label_create(analogue_screen);
  lv_label_set_text(analogue_time_label, "00:00");
  lv_obj_set_style_text_font(analogue_time_label, LV_FONT_DEFAULT, 0);
  lv_obj_set_style_text_color(analogue_time_label, lv_color_hex(0x5BC0FF), 0);
  lv_obj_align(analogue_time_label, LV_ALIGN_BOTTOM_MID, 0, -47);

  lv_obj_t *hint = lv_label_create(analogue_screen);
  lv_label_set_text(hint, "TAP TO CHANGE  |  HOLD TO SET");
  lv_obj_set_style_text_font(hint, LV_FONT_DEFAULT, 0);
  lv_obj_set_style_text_color(hint, lv_color_hex(0x506070), 0);
  lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -24);

  add_full_screen_touch_layer(analogue_screen);
}

void update_clock(lv_timer_t *) {
  datetime_t now;
  PCF85063_Read_Time(&now);
  if (!valid_datetime(now)) return;
  datetime = now;

  char buffer[64];
  snprintf(buffer, sizeof(buffer), "%02u:%02u", now.hour, now.minute);
  lv_label_set_text(time_label, buffer);
  lv_label_set_text(analogue_time_label, buffer);

  snprintf(buffer, sizeof(buffer), ":%02u", now.second);
  lv_label_set_text(seconds_label, buffer);

  lv_label_set_text(day_label, weekday_names[now.dotw]);
  snprintf(buffer, sizeof(buffer), "%u %s %u", now.day,
           month_names[now.month - 1], now.year);
  lv_label_set_text(date_label, buffer);

  snprintf(buffer, sizeof(buffer), "BAT %.2fV", BAT_analogVolts);
  lv_label_set_text(battery_label, buffer);

  const float second_angle = now.second * 6.0f;
  const float minute_angle = now.minute * 6.0f + now.second * 0.1f;
  const float hour_angle = (now.hour % 12) * 30.0f + now.minute * 0.5f;
  draw_hand(hour_hand, hour_points, hour_angle, 108);
  draw_hand(minute_hand, minute_points, minute_angle, 155);
  draw_hand(second_hand, second_points, second_angle, 170);
}

}  // namespace

void Clock_UI_Init(void) {
  lv_obj_clean(lv_scr_act());
  set_screen_background(lv_scr_act());
  lv_obj_set_size(lv_scr_act(), SCREEN_SIZE, SCREEN_SIZE);

  create_digital_screen(lv_scr_act());
  create_analogue_screen(lv_scr_act());
  show_face(FACE_DIGITAL);

  update_clock(nullptr);
  lv_timer_create(update_clock, 250, nullptr);
}
