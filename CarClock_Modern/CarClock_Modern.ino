/*
 * Waveshare ESP32-S3 Touch LCD 2.1 - Offline Car Clock v6.0
 *
 * Tap the screen to switch between digital and analogue faces.
 * Hold the screen to open the time-setting panel.
 * Includes an animated Jaguar startup splash screen.
 */

#include "Wireless.h"
#include "Gyro_QMI8658.h"
#include "RTC_PCF85063.h"
#include "SD_Card.h"
#include "LVGL_Driver.h"
#include "Clock_UI.h"
#include "BAT_Driver.h"

void Driver_Loop(void *parameter)
{
  while (1)
  {
    QMI8658_Loop();
    RTC_Loop();
    BAT_Get_Volts();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void Driver_Init()
{
  Flash_test();
  BAT_Init();
  I2C_Init();
  TCA9554PWR_Init(0x00);
  Set_EXIO(EXIO_PIN8, Low);
  PCF85063_Init();
  QMI8658_Init();

  xTaskCreatePinnedToCore(
    Driver_Loop,
    "Other Driver task",
    4096,
    NULL,
    3,
    NULL,
    0
  );
}

void setup()
{
  Serial.begin(115200);

  // The clock works fully offline, so no Wi-Fi scan is started here.
  Driver_Init();
  LCD_Init();
  SD_Init();
  Lvgl_Init();
  Clock_UI_Init();
}

void loop()
{
  Lvgl_Loop();
  vTaskDelay(pdMS_TO_TICKS(5));
}
