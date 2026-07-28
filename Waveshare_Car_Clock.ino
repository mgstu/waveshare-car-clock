/*
  Car Clock for Waveshare ESP32-S3-Touch-LCD-2.1
  ------------------------------------------------
  Features:
    - Fully offline
    - PCF85063 RTC
    - Tap screen: digital / analogue
    - Long press: settings
    - 24-hour display
    - Remembers selected clock face
    - RTC backup battery retains time

  IMPORTANT:
  Keep these Waveshare driver files in the same Arduino sketch folder:
    Display_ST7701.cpp / .h
    I2C_Driver.cpp / .h
    TCA9554PWR.cpp / .h
    Touch_CST820.cpp / .h

  Required Arduino library:
    Arduino_GFX
*/

#include <Arduino.h>
#include <Wire.h>
#include <Preferences.h>
#include <Arduino_GFX_Library.h>

#include "TCA9554PWR.h"
#include "Display_ST7701.h"
#include "Touch_CST820.h"

#define SCREEN_W 480
#define SCREEN_H 480

#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREY    0x8410
#define DARKGREY 0x4208
#define GREEN   0x07E0

#define PCF85063_ADDR 0x51

struct ClockTime {
  int second;
  int minute;
  int hour;
  int day;
  int weekday;
  int month;
  int year;
};

enum ScreenMode {
  MODE_DIGITAL = 0,
  MODE_ANALOGUE = 1,
  MODE_SETTINGS = 2
};

Preferences preferences;
ScreenMode screenMode = MODE_DIGITAL;
ScreenMode previousClockMode = MODE_DIGITAL;

ClockTime nowTime;
ClockTime editTime;

uint32_t lastClockDraw = 0;
uint32_t lastTouchRead = 0;
bool fullRedraw = true;

class Arduino_ST7701 : public Arduino_GFX {
public:
  Arduino_ST7701(Arduino_ESP32RGBPanel *panel,
                 int16_t w = SCREEN_W,
                 int16_t h = SCREEN_H)
      : Arduino_GFX(w, h), _panel(panel) {}

  bool begin(int32_t speed = GFX_NOT_DEFINED) override {
    _panel->begin();
    LCD_Init();
    return true;
  }

protected:
  void writePixelPreclipped(int16_t x, int16_t y, uint16_t color) override {
    esp_lcd_panel_draw_bitmap(panel_handle, x, y, x + 1, y + 1, &color);
  }

  void writeFastVLine(int16_t x, int16_t y, int16_t h,
                      uint16_t color) override {
    if (h <= 0) return;
    uint16_t *line = (uint16_t *)malloc(h * sizeof(uint16_t));
    if (!line) return;
    for (int i = 0; i < h; i++) line[i] = color;
    esp_lcd_panel_draw_bitmap(panel_handle, x, y, x + 1, y + h, line);
    free(line);
  }

  void writeFastHLine(int16_t x, int16_t y, int16_t w,
                      uint16_t color) override {
    if (w <= 0) return;
    uint16_t *line = (uint16_t *)malloc(w * sizeof(uint16_t));
    if (!line) return;
    for (int i = 0; i < w; i++) line[i] = color;
    esp_lcd_panel_draw_bitmap(panel_handle, x, y, x + w, y + 1, line);
    free(line);
  }

  void writeFillRectPreclipped(int16_t x, int16_t y, int16_t w, int16_t h,
                               uint16_t color) override {
    if (w <= 0 || h <= 0) return;

    const int chunkRows = 12;
    uint16_t *buffer =
        (uint16_t *)malloc(w * chunkRows * sizeof(uint16_t));
    if (!buffer) return;

    for (int i = 0; i < w * chunkRows; i++) buffer[i] = color;

    for (int row = 0; row < h; row += chunkRows) {
      int rows = min(chunkRows, h - row);
      esp_lcd_panel_draw_bitmap(panel_handle, x, y + row,
                                x + w, y + row + rows, buffer);
    }
    free(buffer);
  }

private:
  Arduino_ESP32RGBPanel *_panel;
};

Arduino_ESP32RGBPanel *rgbPanel = new Arduino_ESP32RGBPanel(
    ESP_PANEL_LCD_PIN_NUM_RGB_DATA0,
    ESP_PANEL_LCD_PIN_NUM_RGB_DATA1,
    ESP_PANEL_LCD_PIN_NUM_RGB_DATA2,
    ESP_PANEL_LCD_PIN_NUM_RGB_DATA3,
    ESP_PANEL_LCD_PIN_NUM_RGB_DATA4,
    ESP_PANEL_LCD_PIN_NUM_RGB_DATA5,
    ESP_PANEL_LCD_PIN_NUM_RGB_DATA6,
    ESP_PANEL_LCD_PIN_NUM_RGB_DATA7,
    ESP_PANEL_LCD_PIN_NUM_RGB_DATA8,
    ESP_PANEL_LCD_PIN_NUM_RGB_DATA9,
    ESP_PANEL_LCD_PIN_NUM_RGB_DATA10,
    ESP_PANEL_LCD_PIN_NUM_RGB_DATA11,
    ESP_PANEL_LCD_PIN_NUM_RGB_DATA12,
    ESP_PANEL_LCD_PIN_NUM_RGB_DATA13,
    ESP_PANEL_LCD_PIN_NUM_RGB_DATA14,
    ESP_PANEL_LCD_PIN_NUM_RGB_DATA15,
    ESP_PANEL_LCD_PIN_NUM_RGB_PCLK,
    ESP_PANEL_LCD_PIN_NUM_RGB_VSYNC,
    ESP_PANEL_LCD_PIN_NUM_RGB_HSYNC,
    ESP_PANEL_LCD_PIN_NUM_RGB_DE,
    SCREEN_W,
    SCREEN_H,
    16,
    10,
    8,
    50,
    3,
    8,
    8,
    8
);

Arduino_ST7701 *gfx = new Arduino_ST7701(rgbPanel);

uint8_t decToBcd(int value) {
  return ((value / 10) << 4) | (value % 10);
}

int bcdToDec(uint8_t value) {
  return ((value >> 4) * 10) + (value & 0x0F);
}

bool rtcRead(ClockTime &t) {
  Wire.beginTransmission(PCF85063_ADDR);
  Wire.write(0x04);
  if (Wire.endTransmission(false) != 0) return false;

  if (Wire.requestFrom(PCF85063_ADDR, 7) != 7) return false;

  uint8_t seconds = Wire.read();
  uint8_t minutes = Wire.read();
  uint8_t hours   = Wire.read();
  uint8_t days    = Wire.read();
  uint8_t weekday = Wire.read();
  uint8_t months  = Wire.read();
  uint8_t years   = Wire.read();

  bool oscillatorStopped = seconds & 0x80;

  t.second  = bcdToDec(seconds & 0x7F);
  t.minute  = bcdToDec(minutes & 0x7F);
  t.hour    = bcdToDec(hours & 0x3F);
  t.day     = bcdToDec(days & 0x3F);
  t.weekday = bcdToDec(weekday & 0x07);
  t.month   = bcdToDec(months & 0x1F);
  t.year    = 2000 + bcdToDec(years);

  bool sensible =
      t.second >= 0 && t.second <= 59 &&
      t.minute >= 0 && t.minute <= 59 &&
      t.hour >= 0 && t.hour <= 23 &&
      t.day >= 1 && t.day <= 31 &&
      t.month >= 1 && t.month <= 12 &&
      t.year >= 2024 && t.year <= 2099;

  return sensible && !oscillatorStopped;
}

bool rtcWrite(const ClockTime &t) {
  Wire.beginTransmission(PCF85063_ADDR);
  Wire.write(0x04);
  Wire.write(decToBcd(t.second) & 0x7F);
  Wire.write(decToBcd(t.minute) & 0x7F);
  Wire.write(decToBcd(t.hour) & 0x3F);
  Wire.write(decToBcd(t.day) & 0x3F);
  Wire.write(decToBcd(t.weekday) & 0x07);
  Wire.write(decToBcd(t.month) & 0x1F);
  Wire.write(decToBcd(t.year - 2000));
  return Wire.endTransmission() == 0;
}

int daysInMonth(int month, int year) {
  static const int days[] =
      {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  if (month == 2) {
    bool leap = (year % 4 == 0 && year % 100 != 0) ||
                (year % 400 == 0);
    return leap ? 29 : 28;
  }
  return days[month - 1];
}

int calculateWeekday(int day, int month, int year) {
  if (month < 3) {
    month += 12;
    year--;
  }

  int k = year % 100;
  int j = year / 100;
  int h = (day + (13 * (month + 1)) / 5 + k + k / 4 +
           j / 4 + 5 * j) % 7;

  return (h + 6) % 7;
}

void normaliseEditTime() {
  editTime.month = constrain(editTime.month, 1, 12);
  editTime.year = constrain(editTime.year, 2024, 2099);
  editTime.day =
      constrain(editTime.day, 1, daysInMonth(editTime.month, editTime.year));
  editTime.hour = (editTime.hour + 24) % 24;
  editTime.minute = (editTime.minute + 60) % 60;
  editTime.second = 0;
  editTime.weekday =
      calculateWeekday(editTime.day, editTime.month, editTime.year);
}

const char *weekdayName(int weekday) {
  static const char *names[] =
      {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
  if (weekday < 0 || weekday > 6) return "---";
  return names[weekday];
}

void centeredText(const String &text, int y, int size,
                  uint16_t foreground = WHITE,
                  uint16_t background = BLACK) {
  gfx->setTextSize(size);
  gfx->setTextColor(foreground, background);

  int16_t x1, y1;
  uint16_t w, h;
  gfx->getTextBounds(text, 0, y, &x1, &y1, &w, &h);
  gfx->setCursor((SCREEN_W - w) / 2, y);
  gfx->print(text);
}

void drawDigital() {
  gfx->fillScreen(BLACK);

  char timeBuffer[6];
  snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d",
           nowTime.hour, nowTime.minute);

  char secondsBuffer[3];
  snprintf(secondsBuffer, sizeof(secondsBuffer), "%02d", nowTime.second);

  char dateBuffer[32];
  snprintf(dateBuffer, sizeof(dateBuffer), "%s  %02d/%02d/%04d",
           weekdayName(nowTime.weekday),
           nowTime.day, nowTime.month, nowTime.year);

  centeredText(timeBuffer, 165, 8, WHITE, BLACK);
  centeredText(secondsBuffer, 275, 3, RED, BLACK);
  centeredText(dateBuffer, 340, 2, GREY, BLACK);

  gfx->drawCircle(240, 240, 228, DARKGREY);
  gfx->drawCircle(240, 240, 229, DARKGREY);
}

void drawAnalogue() {
  gfx->fillScreen(BLACK);

  const int cx = 240;
  const int cy = 240;
  const int radius = 220;

  gfx->drawCircle(cx, cy, radius, GREY);
  gfx->drawCircle(cx, cy, radius - 1, GREY);

  for (int i = 0; i < 60; i++) {
    float angle = (i * 6.0 - 90.0) * DEG_TO_RAD;
    int outerX = cx + cos(angle) * 205;
    int outerY = cy + sin(angle) * 205;
    int innerRadius = (i % 5 == 0) ? 180 : 192;
    int innerX = cx + cos(angle) * innerRadius;
    int innerY = cy + sin(angle) * innerRadius;

    gfx->drawLine(innerX, innerY, outerX, outerY,
                  i % 5 == 0 ? WHITE : DARKGREY);
  }

  float secondAngle = (nowTime.second * 6.0 - 90.0) * DEG_TO_RAD;
  float minuteAngle =
      ((nowTime.minute + nowTime.second / 60.0) * 6.0 - 90.0) * DEG_TO_RAD;
  float hourAngle =
      (((nowTime.hour % 12) + nowTime.minute / 60.0) * 30.0 - 90.0) *
      DEG_TO_RAD;

  gfx->drawLine(cx, cy,
                cx + cos(hourAngle) * 105,
                cy + sin(hourAngle) * 105,
                WHITE);
  gfx->drawLine(cx + 1, cy,
                cx + 1 + cos(hourAngle) * 105,
                cy + sin(hourAngle) * 105,
                WHITE);

  gfx->drawLine(cx, cy,
                cx + cos(minuteAngle) * 155,
                cy + sin(minuteAngle) * 155,
                WHITE);

  gfx->drawLine(cx, cy,
                cx + cos(secondAngle) * 175,
                cy + sin(secondAngle) * 175,
                RED);

  gfx->fillCircle(cx, cy, 7, RED);

  char digitalBuffer[6];
  snprintf(digitalBuffer, sizeof(digitalBuffer), "%02d:%02d",
           nowTime.hour, nowTime.minute);
  centeredText(digitalBuffer, 325, 3, WHITE, BLACK);

  char dateBuffer[20];
  snprintf(dateBuffer, sizeof(dateBuffer), "%s %02d/%02d",
           weekdayName(nowTime.weekday), nowTime.day, nowTime.month);
  centeredText(dateBuffer, 370, 2, GREY, BLACK);
}

void drawButton(int x, int y, int w, int h,
                const String &label, uint16_t border = GREY) {
  gfx->drawRoundRect(x, y, w, h, 10, border);
  gfx->setTextSize(2);
  gfx->setTextColor(WHITE, BLACK);

  int16_t x1, y1;
  uint16_t tw, th;
  gfx->getTextBounds(label, 0, 0, &x1, &y1, &tw, &th);
  gfx->setCursor(x + (w - tw) / 2, y + (h - th) / 2);
  gfx->print(label);
}

void drawSettings() {
  gfx->fillScreen(BLACK);
  centeredText("SET CLOCK", 55, 3, WHITE, BLACK);

  char value[32];

  drawButton(50, 125, 80, 55, "-");
  drawButton(350, 125, 80, 55, "+");
  snprintf(value, sizeof(value), "TIME  %02d:%02d",
           editTime.hour, editTime.minute);
  centeredText(value, 142, 3, WHITE, BLACK);

  drawButton(50, 205, 80, 55, "-");
  drawButton(350, 205, 80, 55, "+");
  snprintf(value, sizeof(value), "DATE  %02d/%02d",
           editTime.day, editTime.month);
  centeredText(value, 222, 3, WHITE, BLACK);

  drawButton(50, 285, 80, 55, "-");
  drawButton(350, 285, 80, 55, "+");
  snprintf(value, sizeof(value), "YEAR  %04d", editTime.year);
  centeredText(value, 302, 3, WHITE, BLACK);

  drawButton(95, 375, 125, 60, "CANCEL");
  drawButton(260, 375, 125, 60, "SAVE", GREEN);

  centeredText("Tap centre time/date to select field", 455, 1, GREY, BLACK);
}

int selectedSettingField = 0;

void drawSelectedFieldHint() {
  static const char *fieldNames[] =
      {"HOUR", "MINUTE", "DAY", "MONTH", "YEAR"};

  gfx->fillRect(150, 92, 180, 20, BLACK);
  centeredText(String("ADJUST ") + fieldNames[selectedSettingField],
               95, 1, RED, BLACK);
}

void renderCurrentScreen() {
  if (screenMode == MODE_DIGITAL) drawDigital();
  else if (screenMode == MODE_ANALOGUE) drawAnalogue();
  else {
    drawSettings();
    drawSelectedFieldHint();
  }

  fullRedraw = false;
}

void IRAM_ATTR touchISR() {
  Touch_CST820_ISR();
}

bool pointIn(int x, int y, int bx, int by, int bw, int bh) {
  return x >= bx && x <= bx + bw && y >= by && y <= by + bh;
}

void saveClockMode() {
  preferences.putUChar("clockMode", (uint8_t)screenMode);
}

void enterSettings() {
  if (!rtcRead(editTime)) {
    editTime = {0, 0, 12, 1, 1, 1, 2026};
  }

  previousClockMode = screenMode;
  screenMode = MODE_SETTINGS;
  selectedSettingField = 0;
  fullRedraw = true;
}

void adjustSelectedField(int direction) {
  switch (selectedSettingField) {
    case 0: editTime.hour += direction; break;
    case 1: editTime.minute += direction; break;
    case 2: editTime.day += direction; break;
    case 3: editTime.month += direction; break;
    case 4: editTime.year += direction; break;
  }
  normaliseEditTime();
  fullRedraw = true;
}

void handleSettingsTouch(int x, int y) {
  if (pointIn(x, y, 50, 125, 80, 55)) {
    adjustSelectedField(-1);
    return;
  }
  if (pointIn(x, y, 350, 125, 80, 55)) {
    adjustSelectedField(1);
    return;
  }

  if (pointIn(x, y, 130, 120, 220, 70)) {
    selectedSettingField =
        (selectedSettingField == 0) ? 1 : 0;
    fullRedraw = true;
    return;
  }

  if (pointIn(x, y, 50, 205, 80, 55)) {
    adjustSelectedField(-1);
    return;
  }
  if (pointIn(x, y, 350, 205, 80, 55)) {
    adjustSelectedField(1);
    return;
  }

  if (pointIn(x, y, 130, 200, 220, 70)) {
    if (selectedSettingField == 2) selectedSettingField = 3;
    else selectedSettingField = 2;
    fullRedraw = true;
    return;
  }

  if (pointIn(x, y, 50, 285, 80, 55)) {
    selectedSettingField = 4;
    adjustSelectedField(-1);
    return;
  }
  if (pointIn(x, y, 350, 285, 80, 55)) {
    selectedSettingField = 4;
    adjustSelectedField(1);
    return;
  }

  if (pointIn(x, y, 95, 375, 125, 60)) {
    screenMode = previousClockMode;
    fullRedraw = true;
    return;
  }

  if (pointIn(x, y, 260, 375, 125, 60)) {
    normaliseEditTime();
    if (rtcWrite(editTime)) {
      nowTime = editTime;
      screenMode = previousClockMode;
      fullRedraw = true;
    }
    return;
  }
}

void handleTouch() {
  if (millis() - lastTouchRead < 60) return;
  lastTouchRead = millis();

  if (!Touch_interrupts) return;

  Touch_interrupts = false;
  Touch_Read_Data();

  if (touch_data.points == 0 && touch_data.gesture == NONE) return;

  int x = constrain((int)touch_data.x, 0, SCREEN_W - 1);
  int y = constrain((int)touch_data.y, 0, SCREEN_H - 1);

  if (screenMode == MODE_SETTINGS) {
    handleSettingsTouch(x, y);
    return;
  }

  if (touch_data.gesture == LONG_PRESS) {
    enterSettings();
    return;
  }

  if (touch_data.gesture == SINGLE_CLICK ||
      touch_data.gesture == DOUBLE_CLICK ||
      touch_data.points > 0) {
    screenMode =
        (screenMode == MODE_DIGITAL) ? MODE_ANALOGUE : MODE_DIGITAL;
    saveClockMode();
    fullRedraw = true;
  }
}

bool initialiseHardware() {
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  delay(100);

  TCA9554PWR_Init(0x00);
  delay(20);

  Set_EXIO(EXIO_PIN8, Low);
  delay(20);

  if (!static_cast<Arduino_GFX *>(gfx)->begin()) {
    Serial.println("Display initialisation failed");
    return false;
  }

  Backlight_Init();
  Set_Backlight(60);

  if (!Touch_Init()) {
    Serial.println("Touch initialisation failed");
    return false;
  }

  pinMode(CST820_INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(CST820_INT_PIN),
                  touchISR, FALLING);

  return true;
}

void setup() {
  Serial.begin(115200);
  delay(300);

  preferences.begin("carclock", false);
  uint8_t storedMode = preferences.getUChar("clockMode", MODE_DIGITAL);
  screenMode = storedMode == MODE_ANALOGUE
                   ? MODE_ANALOGUE
                   : MODE_DIGITAL;

  if (!initialiseHardware()) {
    while (true) {
      Serial.println("Hardware initialisation failed");
      delay(1000);
    }
  }

  gfx->setRotation(0);

  if (!rtcRead(nowTime)) {
    nowTime = {0, 0, 12, 1, 1, 1, 2026};
    rtcWrite(nowTime);
    enterSettings();
  }

  fullRedraw = true;
}

void loop() {
  handleTouch();

  if (screenMode != MODE_SETTINGS &&
      millis() - lastClockDraw >= 1000) {
    lastClockDraw = millis();
    if (rtcRead(nowTime)) {
      fullRedraw = true;
    }
  }

  if (fullRedraw) {
    renderCurrentScreen();
  }

  delay(5);
}
