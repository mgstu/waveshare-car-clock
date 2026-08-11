# Waveshare Car Clock

Jaguar-themed offline touchscreen car clock for the **Waveshare ESP32-S3 Touch LCD 2.1** with 480 × 480 round display and onboard **PCF85063 RTC**.

## Current build

**v5.0 — Project Leaper**

The working application is in:

```text
CarClock_Modern/
```

Open:

```text
CarClock_Modern/CarClock_Modern.ino
```

## V5 fix

V4 could switch from digital to analogue, but a short tap back to digital could leave the display blank.

V5 changes the face-switching routine so the selected face is explicitly brought to the LVGL foreground and invalidated for a clean redraw.

## Features

- Large 24-hour digital clock for in-car visibility
- Jaguar-red styling and accents
- Analogue clock face with 12, 3, 6 and 9
- Red second hand
- Jaguar startup splash/logo
- Quick tap toggles digital ↔ analogue
- Long press opens clock/date settings
- Onboard PCF85063 RTC support
- Fully offline operation — no Wi-Fi required
- LVGL 8-compatible implementation used by the Waveshare example

## Hardware

- Waveshare ESP32-S3 Touch LCD 2.1
- 480 × 480 capacitive-touch round LCD
- PCF85063 RTC
- RTC backup battery

For vehicle installation, use a protected, regulated automotive **12 V to 5 V USB supply**. Do not connect the ESP32 board directly to the vehicle's 12 V electrical system.

## Version history

- **v5.0** — fixes analogue → digital blank screen and establishes the uploaded V4 Project Leaper build as the source baseline
- **v4.0** — Project Leaper styling, large digital display, Jaguar splash/logo and cleaner analogue face
- **v2.2** — earlier tested digital/analogue UI and touch-layer fix
