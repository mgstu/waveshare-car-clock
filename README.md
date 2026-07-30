# Waveshare Car Clock

Modern offline touchscreen car clock for the **Waveshare ESP32-S3 Touch LCD 2.1** with its 480 × 480 round display and onboard **PCF85063 RTC**.

## Current stable build

**v2.2 — tested and working on the target hardware**

The working application is in:

```text
CarClock_Modern/
```

Open this sketch in Arduino IDE:

```text
CarClock_Modern/CarClock_Modern.ino
```

Arduino requires the sketch folder and main `.ino` filename to match.

## Features

- Modern dark dashboard display
- Large 24-hour digital clock
- Analogue clock face
- Tap anywhere to switch digital ↔ analogue
- Full-screen touch layer on both clock faces
- Long press to open time settings
- Onboard PCF85063 RTC support
- Battery-voltage display
- Fully offline operation — no Wi-Fi required
- LVGL 9.5 compatible
- Uses `LV_FONT_DEFAULT`, avoiding disabled Montserrat font errors

## Controls

- **Quick tap anywhere:** change clock face
- **Long press:** open time setting controls
- **SAVE:** write the selected time to the RTC
- **CANCEL:** return without changing the RTC

## Hardware

- Waveshare ESP32-S3 Touch LCD 2.1
- 480 × 480 capacitive-touch round LCD
- PCF85063 RTC
- RTC backup battery

For vehicle installation, use a protected, regulated automotive **12 V to 5 V USB supply**. Do not connect the ESP32 board directly to the vehicle's 12 V electrical system.

## Arduino setup

Use the board package, LVGL configuration and libraries from the official Waveshare `LVGL_Arduino` example that successfully runs on the display.

Recommended board selection:

```text
Waveshare ESP32-S3-Touch-LCD-2.1
```

Keep all Waveshare `.cpp` and `.h` driver files in the same `CarClock_Modern` sketch folder.

## v2.2 touch fix

Earlier builds could switch from digital to analogue but then appear stuck because analogue clock objects intercepted touch events.

v2.2 places a transparent clickable object over the entire 480 × 480 face. Clock hands, labels and markers can no longer block taps, so switching works reliably in both directions.

## Status

Confirmed working on Stu's physical Waveshare display on 30 July 2026.
