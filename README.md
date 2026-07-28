# Waveshare Car Clock

Offline touchscreen car clock for the **Waveshare ESP32-S3-Touch-LCD-2.1**.

## Features

- 24-hour digital clock
- Analogue clock face
- Tap the display to switch between digital and analogue modes
- Long-press to open the settings screen
- Set hour, minute, date, month and year using the touchscreen
- Uses the onboard **PCF85063 RTC**
- RTC backup battery retains the time when vehicle power is removed
- Remembers the last selected clock face
- No Wi-Fi, Bluetooth or network connection required

## Hardware

- Waveshare ESP32-S3-Touch-LCD-2.1
- RTC backup battery fitted
- USB-C cable for programming
- For vehicle installation: a protected, regulated automotive 12 V to 5 V USB supply

> Do not connect the ESP32 board directly to the vehicle's 12 V electrical system.

## Software required

- Arduino IDE 2.x
- Espressif ESP32 board package 3.0.0 or newer
- GFX Library for Arduino by Moon On Our Nation
- Waveshare's demonstration package for the ESP32-S3-Touch-LCD-2.1

## 1. Install ESP32 support in Arduino IDE

Open Arduino IDE and go to:

`File > Preferences`

Add this URL to **Additional boards manager URLs**:

```text
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

Then open:

`Tools > Board > Boards Manager`

Search for **esp32** and install **esp32 by Espressif Systems**.

Use version **3.0.0 or newer**.

## 2. Install Arduino_GFX

Open:

`Sketch > Include Library > Manage Libraries`

Search for:

```text
GFX Library for Arduino
```

Install the library by **Moon On Our Nation**.

## 3. Download the Waveshare driver files

Download the official demonstration archive from the Waveshare wiki for the ESP32-S3-Touch-LCD-2.1.

From the Arduino example, copy these files into the same folder as `Waveshare_Car_Clock.ino`:

```text
Display_ST7701.cpp
Display_ST7701.h
I2C_Driver.cpp
I2C_Driver.h
TCA9554PWR.cpp
TCA9554PWR.h
Touch_CST820.cpp
Touch_CST820.h
```

The Arduino project folder must look like this:

```text
Waveshare_Car_Clock/
├── Waveshare_Car_Clock.ino
├── Display_ST7701.cpp
├── Display_ST7701.h
├── I2C_Driver.cpp
├── I2C_Driver.h
├── TCA9554PWR.cpp
├── TCA9554PWR.h
├── Touch_CST820.cpp
└── Touch_CST820.h
```

The Waveshare files are not included in this repository because they should be taken from the current official demonstration package for the board.

## 4. Download this project

Either download the repository as a ZIP from GitHub, or clone it:

```bash
git clone https://github.com/mgstu/waveshare-car-clock.git
```

Open the cloned folder and double-click:

```text
Waveshare_Car_Clock.ino
```

If Arduino IDE asks to move the sketch into a folder named `Waveshare_Car_Clock`, allow it to do so. Then copy the eight Waveshare driver files into that folder.

## 5. Arduino board settings

Select:

```text
Tools > Board > esp32 > ESP32S3 Dev Module
```

Recommended settings:

```text
USB CDC On Boot: Enabled
CPU Frequency: 240MHz
Flash Size: 16MB
Partition Scheme: Huge APP
PSRAM: OPI PSRAM
Upload Speed: 921600
```

If uploading is unreliable, reduce the upload speed to `460800` or `115200`.

## 6. Verify the code

Press the **Verify** button in Arduino IDE.

Common errors:

### Arduino_GFX_Library.h not found

Install **GFX Library for Arduino** from Arduino Library Manager.

### Display_ST7701.h not found

Copy the Waveshare driver files into the same sketch folder as the `.ino` file.

### ledcAttach was not declared

Upgrade the Espressif ESP32 board package to version 3.0.0 or newer.

### PSRAM or memory errors

Confirm that **PSRAM** is set to **OPI PSRAM**.

## 7. Upload to the board

Connect the display board using USB-C, select the correct COM port, and click **Upload**.

If the board will not enter upload mode:

1. Hold **BOOT**.
2. Press and release **RESET**.
3. Release **BOOT**.
4. Start the upload again.

## First startup

If the RTC does not contain a sensible date and time, the settings screen opens automatically.

### Normal clock controls

- **Tap:** switch between digital and analogue clock faces
- **Long-press:** open settings

### Settings controls

- Tap the centre of the **TIME** row to select hour or minute
- Use the TIME `-` and `+` buttons to change the selected value
- Tap the centre of the **DATE** row to select day or month
- Use the DATE `-` and `+` buttons to change the selected value
- Use the YEAR `-` and `+` buttons to change the year
- Tap **SAVE** to write the new time to the RTC
- Tap **CANCEL** to leave without saving

Seconds reset to zero when the clock is saved.

## GMT and BST

The first version stores local UK time directly in the RTC. It does not yet change automatically between GMT and BST.

Adjust the hour manually using the touchscreen when the clocks change. Automatic UK daylight-saving support is planned for a later version.

## Vehicle installation notes

Use a good-quality automotive USB adapter or a protected 12 V to 5 V converter. Vehicle electrical systems can produce voltage spikes, especially during engine starting and load switching.

Recommended installation behaviour:

- Power the display from an ignition-switched supply
- Leave the RTC backup battery connected
- Mount the display where it does not obstruct the driver's view
- Set brightness low enough to avoid glare at night
- Do not operate the settings screen while driving

## Current status

This is the initial hardware-test release. It has not yet been physically compiled and tested on the target board, so small adjustments may be needed for the exact version of Waveshare's driver files.

Planned improvements include:

- Automatic GMT/BST handling
- Day and night brightness settings
- Better automotive clock themes
- Optional startup logo
- Improved touch debouncing
- Reduced screen redraw flicker

## Repository

Created for Stu's Waveshare car clock project.
