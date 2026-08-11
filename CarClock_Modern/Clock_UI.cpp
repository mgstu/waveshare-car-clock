#include "Clock_UI.h"
#include "Jaguar_Logo.h"
#include <math.h>
#include <stdio.h>

namespace {

constexpr int SCREEN_SIZE = 480;
constexpr int CLOCK_CX = 240;
constexpr int CLOCK_CY = 240;
constexpr float PI_F = 3.14159265358979323846f;

enum ClockFace { FACE_DIGITAL, FACE_ANALOGUE };

enum SettingAction {
  HOUR_DOWN = 1,
  HOUR_UP,
  MINUTE_DOWN,
  MINUTE_UP,
  DAY_DOWN,
  DAY_UP,
  MONTH_DOWN,
  MONTH_UP,
  YEAR_DOWN,
  YEAR_UP,
  SAVE_SETTINGS,
  CANCEL_SETTINGS
};

ClockFace current_face = FACE_DIGITAL;
bool long_press_seen = false;

lv_obj_t *digital_screen = nullptr;
lv_obj_t *analogue_screen = nullptr;
lv_obj_t *settings_panel = nullptr;
lv_obj_t *splash_screen = nullptr;

lv_obj_t *time_label = nullptr;
lv_obj_t *day_label = nullptr;
lv_obj_t *date_label = nullptr;

lv_obj_t *analogue_time_label = nullptr;
lv_obj_t *hour_hand = nullptr;
lv_obj_t *minute_hand = nullptr;
lv_obj_t *second_hand = nullptr;
lv_point_t hour_points[2];
lv_point_t minute_points[2];
lv_point_t second_points[2];

lv_obj_t *set_hour_label = nullptr;
lv_obj_t *set_minute_label = nullptr;
lv_obj_t *set_day_label = nullptr;
lv_obj_t *set_month_label = nullptr;
lv_obj_t *set_year_label = nullptr;

int edit_hour = 0;
int edit_minute = 0;
int edit_day = 1;
int edit_month = 1;
int edit_year = 2026;

const char *weekday_names[] = {
  "SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY",
  "THURSDAY", "FRIDAY", "SATURDAY"
};

const char *month_names[] = {
  "January", "February", "March", "April", "May", "June",
  "July", "August", "September", "October", "November", "December"
};

const char *month_short[] = {
  "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
  "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
};

bool is_leap_year(int year) {
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int days_in_month(int month, int year) {
  static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (month == 2 && is_leap_year(year)) return 29;
  return days[month - 1];
}

int calculate_weekday(int year, int month, int day) {
  static const int offsets[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  if (month < 3) year -= 1;
  return (year + year / 4 - year / 100 + year / 400 + offsets[month - 1] + day) % 7;
}

bool valid_datetime(const datetime_t &dt) {
  if (dt.hour >= 24 || dt.minute >= 60 || dt.second >= 60 ||
      dt.month < 1 || dt.month > 12 || dt.year < 1970 || dt.year > 2069 ||
      dt.dotw > 6 || dt.day < 1) return false;
  return dt.day <= days_in_month(dt.month, dt.year);
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

void clamp_edit_day() {
  const int maximum = days_in_month(edit_month, edit_year);
  if (edit_day > maximum) edit_day = maximum;
  if (edit_day < 1) edit_day = 1;
}

void update_settings_labels() {
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%02d", edit_hour);
  lv_label_set_text(set_hour_label, buffer);
  snprintf(buffer, sizeof(buffer), "%02d", edit_minute);
  lv_label_set_text(set_minute_label, buffer);
  snprintf(buffer, sizeof(buffer), "%02d", edit_day);
  lv_label_set_text(set_day_label, buffer);
  lv_label_set_text(set_month_label, month_short[edit_month - 1]);
  snprintf(buffer, sizeof(buffer), "%04d", edit_year);
  lv_label_set_text(set_year_label, buffer);
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
    case HOUR_DOWN: edit_hour = (edit_hour + 23) % 24; break;
    case HOUR_UP: edit_hour = (edit_hour + 1) % 24; break;
    case MINUTE_DOWN: edit_minute = (edit_minute + 59) % 60; break;
    case MINUTE_UP: edit_minute = (edit_minute + 1) % 60; break;
    case DAY_DOWN:
      edit_day--;
      if (edit_day < 1) edit_day = days_in_month(edit_month, edit_year);
      break;
    case DAY_UP:
      edit_day++;
      if (edit_day > days_in_month(edit_month, edit_year)) edit_day = 1;
      break;
    case MONTH_DOWN:
      edit_month--;
      if (edit_month < 1) edit_month = 12;
      clamp_edit_day();
      break;
    case MONTH_UP:
      edit_month++;
      if (edit_month > 12) edit_month = 1;
      clamp_edit_day();
      break;
    case YEAR_DOWN:
      edit_year--;
      if (edit_year < 1970) edit_year = 2069;
      clamp_edit_day();
      break;
    case YEAR_UP:
      edit_year++;
      if (edit_year > 2069) edit_year = 1970;
      clamp_edit_day();
      break;
    case SAVE_SETTINGS: {
      datetime_t updated = {};
      updated.year = edit_year;
      updated.month = edit_month;
      updated.day = edit_day;
      updated.dotw = calculate_weekday(edit_year, edit_month, edit_day);
      updated.hour = edit_hour;
      updated.minute = edit_minute;
      updated.second = 0;
      PCF85063_Set_All(updated);
      datetime = updated;
      close_settings();
      return;
    }
    case CANCEL_SETTINGS:
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
  lv_obj_set_style_radius(button, 14, 0);
  lv_obj_set_style_bg_color(button, lv_color_hex(0x1C2733), 0);
  lv_obj_set_style_bg_color(button, lv_color_hex(0x30465B), LV_STATE_PRESSED);
  lv_obj_add_event_cb(button, settings_button_event, LV_EVENT_CLICKED,
                      reinterpret_cast<void *>(action));

  lv_obj_t *label = lv_label_create(button);
  lv_label_set_text(label, text);
  style_label(label, 0xFFFFFF);
  lv_obj_center(label);
  return button;
}

void make_setting_row(lv_obj_t *parent, const char *name, lv_obj_t **value_label,
                      int y, intptr_t down_action, intptr_t up_action) {
  lv_obj_t *name_label = lv_label_create(parent);
  lv_label_set_text(name_label, name);
  style_label(name_label, 0x718496);
  lv_obj_set_pos(name_label, 45, y + 13);

  make_button(parent, "-", 145, y, 48, 42, down_action);

  *value_label = lv_label_create(parent);
  style_label(*value_label, 0xFFFFFF);
  lv_obj_set_width(*value_label, 95);
  lv_obj_set_style_text_align(*value_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_pos(*value_label, 198, y + 13);

  make_button(parent, "+", 297, y, 48, 42, up_action);
}

void open_settings() {
  if (settings_panel != nullptr) return;

  datetime_t now;
  PCF85063_Read_Time(&now);
  if (!valid_datetime(now)) {
    now = {2026, 1, 1, 4, 12, 0, 0};
  }

  edit_hour = now.hour;
  edit_minute = now.minute;
  edit_day = now.day;
  edit_month = now.month;
  edit_year = now.year;

  settings_panel = lv_obj_create(lv_scr_act());
  lv_obj_set_size(settings_panel, 410, 410);
  lv_obj_center(settings_panel);
  lv_obj_set_style_radius(settings_panel, 46, 0);
  lv_obj_set_style_bg_color(settings_panel, lv_color_hex(0x0D141C), 0);
  lv_obj_set_style_bg_opa(settings_panel, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(settings_panel, 2, 0);
  lv_obj_set_style_border_color(settings_panel, lv_color_hex(0xD71920), 0);
  lv_obj_set_style_pad_all(settings_panel, 0, 0);
  lv_obj_clear_flag(settings_panel, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *title = lv_label_create(settings_panel);
  lv_label_set_text(title, "SET CLOCK");
  style_label(title, 0xE32A32);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 18);

  make_setting_row(settings_panel, "HOUR", &set_hour_label, 58, HOUR_DOWN, HOUR_UP);
  make_setting_row(settings_panel, "MIN", &set_minute_label, 108, MINUTE_DOWN, MINUTE_UP);
  make_setting_row(settings_panel, "DAY", &set_day_label, 158, DAY_DOWN, DAY_UP);
  make_setting_row(settings_panel, "MONTH", &set_month_label, 208, MONTH_DOWN, MONTH_UP);
  make_setting_row(settings_panel, "YEAR", &set_year_label, 258, YEAR_DOWN, YEAR_UP);

  make_button(settings_panel, "SAVE", 75, 330, 115, 48, SAVE_SETTINGS);
  make_button(settings_panel, "CANCEL", 220, 330, 115, 48, CANCEL_SETTINGS);

  update_settings_labels();
}

void splash_fade_done(lv_anim_t *) {
  if (splash_screen != nullptr) {
    lv_obj_del(splash_screen);
    splash_screen = nullptr;
  }
}

void start_splash_fade(lv_timer_t *timer) {
  lv_timer_del(timer);
  if (splash_screen == nullptr) return;

  lv_anim_t animation;
  lv_anim_init(&animation);
  lv_anim_set_var(&animation, splash_screen);
  lv_anim_set_values(&animation, LV_OPA_COVER, LV_OPA_TRANSP);
  lv_anim_set_time(&animation, 650);
  lv_anim_set_exec_cb(&animation, [](void *obj, int32_t value) {
    lv_obj_set_style_opa(static_cast<lv_obj_t *>(obj), value, 0);
  });
  lv_anim_set_ready_cb(&animation, splash_fade_done);
  lv_anim_start(&animation);
}

void create_startup_splash(lv_obj_t *parent) {
  splash_screen = lv_obj_create(parent);
  lv_obj_set_size(splash_screen, SCREEN_SIZE, SCREEN_SIZE);
  lv_obj_set_pos(splash_screen, 0, 0);
  set_screen_background(splash_screen);
  lv_obj_set_style_opa(splash_screen, LV_OPA_TRANSP, 0);
  lv_obj_move_foreground(splash_screen);

  lv_obj_t *logo = lv_img_create(splash_screen);
  lv_img_set_src(logo, &jaguar_logo);
  lv_obj_center(logo);

  lv_obj_t *caption = lv_label_create(splash_screen);
  lv_label_set_text(caption, "JAGUAR");
  style_label(caption, 0xD71920);
  lv_obj_align(caption, LV_ALIGN_BOTTOM_MID, 0, -34);

  lv_anim_t animation;
  lv_anim_init(&animation);
  lv_anim_set_var(&animation, splash_screen);
  lv_anim_set_values(&animation, LV_OPA_TRANSP, LV_OPA_COVER);
  lv_anim_set_time(&animation, 700);
  lv_anim_set_exec_cb(&animation, [](void *obj, int32_t value) {
    lv_obj_set_style_opa(static_cast<lv_obj_t *>(obj), value, 0);
  });
  lv_anim_start(&animation);

  lv_timer_create(start_splash_fade, 2400, nullptr);
}

void show_face(ClockFace face) {
  current_face = face;

  if (face == FACE_DIGITAL) {
    lv_obj_add_flag(analogue_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(digital_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(digital_screen);
    lv_obj_invalidate(digital_screen);
  } else {
    lv_obj_add_flag(digital_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(analogue_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(analogue_screen);
    lv_obj_invalidate(analogue_screen);
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

  lv_obj_t *ring = lv_obj_create(digital_screen);
  lv_obj_set_size(ring, 438, 438);
  lv_obj_center(ring);
  lv_obj_set_style_radius(ring, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(ring, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(ring, 2, 0);
  lv_obj_set_style_border_color(ring, lv_color_hex(0x8E1118), 0);
  lv_obj_clear_flag(ring, LV_OBJ_FLAG_SCROLLABLE);

  time_label = lv_label_create(digital_screen);
  lv_label_set_text(time_label, "00:00");
  style_label(time_label, 0xFFFFFF);
  // V6: 1800 (~7x) caused the digital face to fail to render reliably.
  // 900 is the last known-good clock scale on this hardware.
  lv_obj_set_style_transform_zoom(time_label, 900, 0);
  lv_obj_set_style_transform_pivot_x(time_label, 35, 0);
  lv_obj_set_style_transform_pivot_y(time_label, 7, 0);
  lv_obj_align(time_label, LV_ALIGN_CENTER, 0, -52);

  day_label = lv_label_create(digital_screen);
  lv_label_set_text(day_label, "THURSDAY");
  style_label(day_label, 0xD71920);
  lv_obj_set_style_transform_zoom(day_label, 610, 0);
  lv_obj_set_style_transform_pivot_x(day_label, 32, 0);
  lv_obj_set_style_transform_pivot_y(day_label, 7, 0);
  lv_obj_align(day_label, LV_ALIGN_CENTER, 0, 73);

  date_label = lv_label_create(digital_screen);
  lv_label_set_text(date_label, "30 JULY");
  style_label(date_label, 0xB8C0C8);
  lv_obj_set_style_transform_zoom(date_label, 500, 0);
  lv_obj_set_style_transform_pivot_x(date_label, 28, 0);
  lv_obj_set_style_transform_pivot_y(date_label, 7, 0);
  lv_obj_align(date_label, LV_ALIGN_CENTER, 0, 126);

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
                            lv_color_hex(hour % 3 == 0 ? 0xE32A32 : 0x657584), 0);
  lv_obj_set_pos(marker, x - marker_size / 2, y - marker_size / 2);
  lv_obj_clear_flag(marker, LV_OBJ_FLAG_SCROLLABLE);
}

void add_dial_number(lv_obj_t *parent, const char *text, lv_align_t align,
                     int x_offset, int y_offset) {
  lv_obj_t *label = lv_label_create(parent);
  lv_label_set_text(label, text);
  style_label(label, 0xD9DEE3);
  lv_obj_set_style_transform_zoom(label, 430, 0);
  lv_obj_align(label, align, x_offset, y_offset);
}

void create_analogue_screen(lv_obj_t *parent) {
  analogue_screen = lv_obj_create(parent);
  lv_obj_set_size(analogue_screen, SCREEN_SIZE, SCREEN_SIZE);
  lv_obj_set_pos(analogue_screen, 0, 0);
  set_screen_background(analogue_screen);

  lv_obj_t *rim = lv_obj_create(analogue_screen);
  lv_obj_set_size(rim, 430, 430);
  lv_obj_center(rim);
  lv_obj_set_style_radius(rim, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(rim, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(rim, 2, 0);
  lv_obj_set_style_border_color(rim, lv_color_hex(0x8E1118), 0);
  lv_obj_clear_flag(rim, LV_OBJ_FLAG_SCROLLABLE);

  for (int hour = 0; hour < 12; ++hour) add_hour_marker(analogue_screen, hour);

  add_dial_number(analogue_screen, "12", LV_ALIGN_TOP_MID, 0, 38);
  add_dial_number(analogue_screen, "3", LV_ALIGN_RIGHT_MID, -42, 0);
  add_dial_number(analogue_screen, "6", LV_ALIGN_BOTTOM_MID, 0, -39);
  add_dial_number(analogue_screen, "9", LV_ALIGN_LEFT_MID, 42, 0);

  hour_hand = lv_line_create(analogue_screen);
  lv_obj_set_style_line_width(hour_hand, 11, 0);
  lv_obj_set_style_line_rounded(hour_hand, true, 0);
  lv_obj_set_style_line_color(hour_hand, lv_color_hex(0xFFFFFF), 0);

  minute_hand = lv_line_create(analogue_screen);
  lv_obj_set_style_line_width(minute_hand, 6, 0);
  lv_obj_set_style_line_rounded(minute_hand, true, 0);
  lv_obj_set_style_line_color(minute_hand, lv_color_hex(0xC5CCD2), 0);

  second_hand = lv_line_create(analogue_screen);
  lv_obj_set_style_line_width(second_hand, 3, 0);
  lv_obj_set_style_line_rounded(second_hand, true, 0);
  lv_obj_set_style_line_color(second_hand, lv_color_hex(0xD71920), 0);

  lv_obj_t *hub = lv_obj_create(analogue_screen);
  lv_obj_set_size(hub, 22, 22);
  lv_obj_set_pos(hub, CLOCK_CX - 11, CLOCK_CY - 11);
  lv_obj_set_style_radius(hub, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_border_width(hub, 0, 0);
  lv_obj_set_style_bg_color(hub, lv_color_hex(0xD71920), 0);
  lv_obj_clear_flag(hub, LV_OBJ_FLAG_SCROLLABLE);

  analogue_time_label = lv_label_create(analogue_screen);
  lv_label_set_text(analogue_time_label, "00:00");
  style_label(analogue_time_label, 0xAAB2BA);
  lv_obj_set_style_transform_zoom(analogue_time_label, 360, 0);
  lv_obj_align(analogue_time_label, LV_ALIGN_CENTER, 0, 76);

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

  lv_label_set_text(day_label, weekday_names[now.dotw]);
  snprintf(buffer, sizeof(buffer), "%u %s", now.day,
           month_short[now.month - 1]);
  lv_label_set_text(date_label, buffer);

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
  create_startup_splash(lv_scr_act());

  update_clock(nullptr);
  lv_timer_create(update_clock, 250, nullptr);
}
