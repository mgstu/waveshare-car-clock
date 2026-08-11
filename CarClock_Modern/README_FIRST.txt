WAVESHARE CAR CLOCK v6.0 - PROJECT LEAPER
==========================================

Open CarClock_Modern.ino in Arduino IDE and upload exactly as with v4.0.

Changes in v6.0:
- Restores the digital HH:MM zoom from 1800 to the last known-good value of 900
- Retains the V5 face foreground/redraw fix
- Retains all v4.0 Project Leaper styling and behaviour

Why:
- LVGL transform zoom uses 256 as 100%
- The V4/V5 HH:MM value of 1800 is roughly 7x scaling
- On the target display that could leave the digital face blank while the Jaguar-red ring still rendered

v4.0 features retained:
- Large digital time for in-car visibility
- No year, seconds, battery text, or on-screen touch instructions
- Uppercase weekday and short date
- Jaguar-red outer ring and accents
- Cleaner analogue face with 12, 3, 6 and 9
- Red second hand
- Jaguar startup logo retained
- Tap toggles digital/analogue
- Long press opens clock settings

This build uses the LVGL 8-compatible calls required by the Waveshare example.
