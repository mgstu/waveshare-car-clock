WAVESHARE CAR CLOCK v8.0 - PROJECT LEAPER
==========================================

V8 replaces the unreliable transform-scaled digital font with a large native
seven-segment HH:MM display drawn from LVGL objects.

Changes in v8.0:
- Large, crisp HH:MM digital clock without any font transform/zoom
- White seven-segment digits with Jaguar-red colon
- Keeps the Jaguar-red outer ring
- Keeps normal unscaled weekday and date labels
- Keeps the analogue face, Jaguar splash, settings and tap switching
- Retains the V5 foreground/redraw switching safeguard

Reason:
- V7 proved the digital screen and RTC were working correctly
- The digital labels only disappeared when transform scaling was applied
- V8 avoids the problematic font-transform path completely

Open CarClock_Modern.ino in Arduino IDE and upload as before.
