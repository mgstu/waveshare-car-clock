# Waveshare Car Clock

Jaguar-themed offline touchscreen car clock for the **Waveshare ESP32-S3 Touch LCD 2.1** with 480 × 480 round display and onboard **PCF85063 RTC**.

## Current stable build

**v8.0 — Project Leaper — tested and working on the target hardware**

The working application is in:

```text
CarClock_Modern/
```

Open:

```text
CarClock_Modern/CarClock_Modern.ino
```

## V8 digital display fix

Earlier Project Leaper builds used LVGL `transform_zoom` to enlarge the digital clock labels. On the target Waveshare display this could cause all of the digital text to disappear while the digital face itself — including the Jaguar-red outer ring — continued to render.

V7 confirmed that the screen, RTC updates and face switching were healthy when the labels were left at native size.

V8 removes transform scaling from the main digital time completely and draws **HH:MM as large seven-segment graphics using native LVGL objects**. This produces a large, crisp digital display without relying on enlarged font rendering.

V8 has been confirmed working on the physical display, including switching **digital → analogue → digital**.

## Features

- Large 24-hour seven-segment digital clock for in-car visibility
- White digital digits with Jaguar-red colon
- Jaguar-red outer ring and accents
- Weekday and date display
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

- **v8.0 — Project Leaper** — tested working baseline; large seven-segment HH:MM display replaces unreliable transform-scaled digital fonts
- **v7.0** — diagnostic build; removed all digital label scaling and proved the digital screen, RTC and switching were working
- **v6.0** — reduced the main HH:MM transform zoom; did not fully resolve the blank digital labels
- **v5.0** — added explicit LVGL foreground/redraw handling for face switching
- **v4.0 — Project Leaper** — Jaguar styling, large digital display, Jaguar splash/logo and cleaner analogue face
- **v2.2** — earlier tested digital/analogue UI and touch-layer fix

## Known-good baseline

As of **11 August 2026**, v8.0 is the version confirmed working on Stu's physical Waveshare ESP32-S3 Touch LCD 2.1. Future changes should be based on v8.0 rather than the earlier transform-scaled digital builds.
