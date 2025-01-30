# RTC_DS1307_Library

The **RTC_DS1307_Library** allows easy interaction with the DS1307 Real-Time Clock (RTC) module, offering advanced functions for managing time and additional configurations such as square wave output and NV-RAM.

## What's New in v1.1.1

This release includes significant improvements and new features:

- **Added support for 12-hour and 24-hour time formats.** You can now switch between these formats using the `SET_FORMAT` command.
- **Added two Python tools in the `tools/` directory:**
  - `rtc_sync.py`: Synchronizes the RTC with the system time (supports UTC and local time).
  - `local_test.py`: Detects the system timezone and displays UTC and local times.
- **Updated documentation.** The repository now includes three versions of the DS1307 datasheet (`Dallas`, `Maxim`, and `Spanish`).

## Features

- Configuration and retrieval of time in Unix timestamp format.
- Support for both **12-hour and 24-hour formats** with AM/PM indication.
- Reading and writing to the DS1307's battery-backed RAM.
- Control of square wave output with configurable frequencies (1Hz, 4kHz, 8kHz, and 32kHz).
- Support for synchronization with the [TimeLib](https://github.com/PaulStoffregen/Time) library.
- Compatible with **Arduino, ESP32, and ESP8266**.

## Installation

### **Option 1: Arduino Library Manager (Recommended)**

The library is available directly in the **Arduino Library Manager**.

1. Open the **Arduino IDE**.
2. Go to `Sketch -> Include Library -> Manage Libraries...`.
3. Search for **RTC_DS1307_Library**.
4. Click **Install**.

### **Option 2: Manual Installation**

1. Download this repository as a ZIP file.
2. Open the Arduino IDE.
3. Go to `Sketch -> Include Library -> Add .ZIP Library...`.
4. Select the downloaded ZIP file.
5. The library is now ready to use.

## Basic Example

This minimal example demonstrates how to initialize the DS1307 module and retrieve the current time:

```cpp
#include <Wire.h>
#include <DS1307Lib.h>

void setup() {
  Serial.begin(9600);
  Wire.begin();
  RTC.begin();
}

void loop() {
  Serial.println(RTC.get());
  delay(1000);
}
```

## New Commands

- **SET_FORMAT**: Switch between 12-hour and 24-hour formats.
  ```
  SET_FORMAT 12  // Sets the RTC to 12-hour mode
  SET_FORMAT 24  // Sets the RTC to 24-hour mode
  ```
- **Python Tools for Synchronization**
  - `rtc_sync.py COM5 utc`  Syncs the RTC with system UTC time.
  - `rtc_sync.py COM5 local`  Syncs the RTC with system local time.

## License

This library is distributed under the GNU Lesser General Public License (LGPL) version 3. See the `COPYING.LESSER.txt` file for more details.

## Additional Resources

- [DS1307 Datasheet - Dallas](docs/DS1307_Dallas.pdf)
- [DS1307 Datasheet - Maxim](docs/DS1307_Maxim.pdf)
- [DS1307 Datasheet (Spanish)](docs/DS1307_Dallas_es.pdf)
- Included examples:
  - `RTC_ClockOut.ino`: Configuring the square wave output.
  - `RTC_Ram.ino`: Reading and writing to the RTC's RAM.
  - `RTC_Time.ino`: Configuring and reading time.
  - `RTC_Full.ino`: Complete example with all features.

