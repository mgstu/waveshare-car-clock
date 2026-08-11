WAVESHARE CAR CLOCK v7.0 - PROJECT LEAPER DIAGNOSTIC
=====================================================

Purpose of v7.0:
- Diagnose the missing digital text while the red digital ring still renders.
- Removes ALL LVGL transform/zoom operations from the three digital labels only.
- Digital time, weekday and date use plain native-size LVGL labels.
- Keeps analogue face, Jaguar red ring, touch switching, splash and settings unchanged.

Expected result:
- Digital text will be smaller than desired, but it should be plainly visible.
- If visible, the fault is confirmed to be the transformed/scaled digital labels, and the next build can use real larger LVGL fonts instead of transform zoom.
