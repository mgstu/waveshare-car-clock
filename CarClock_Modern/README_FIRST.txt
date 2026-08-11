WAVESHARE CAR CLOCK v5.0 - PROJECT LEAPER
==========================================

Open CarClock_Modern.ino in Arduino IDE and upload exactly as with v4.0.

Changes in v5.0:
- Fixes analogue -> digital short-tap blank-screen issue
- Selected face is explicitly brought to the LVGL foreground
- Selected face is invalidated to force a clean redraw
- Retains all v4.0 Project Leaper styling and behaviour

v4.0 features retained:
- Much larger digital time for in-car visibility
- No year, seconds, battery text, or on-screen touch instructions
- Uppercase weekday and short date (for example: THURSDAY / 30 JUL)
- Jaguar-red outer ring and accents
- Cleaner analogue face with 12, 3, 6 and 9
- Red second hand
- Jaguar startup logo retained
- Tap toggles digital/analogue
- Long press opens clock settings

This build uses the LVGL 8-compatible calls required by the Waveshare example.
