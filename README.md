# RTC_DS1307_Library

The **RTC_DS1307_Library** allows easy interaction with the DS1307 Real-Time Clock (RTC) module, offering advanced functions for managing time and additional configurations such as square wave output and NV-RAM.

## Features
- Configuration and retrieval of time in Unix timestamp format.
- Reading and writing to the DS1307's battery-backed RAM.
- Control of square wave output with configurable frequencies (1Hz, 4kHz, 8kHz, and 32kHz).
- Support for synchronization with the [TimeLib](https://github.com/PaulStoffregen/Time) library.
- Compatible with Arduino, ESP32, and ESP8266.

## Installation
1. Download this repository as a ZIP file.
2. Open the Arduino IDE.
3. Go to `Sketch -> Include Library -> Add .ZIP Library...`.
4. Select the downloaded ZIP file.
5. The library is now ready to use.

## Basic Example
This example demonstrates how to initialize the DS1307 module, configure it with the current time, and continuously read the time. It is ideal for verifying the basic functionality of the RTC and establishing a foundation for more advanced projects.

```cpp
#include <Wire.h>
#include <DS1307Lib.h>
#include <TimeLib.h>

void setup() {
  Serial.begin(9600);
  Wire.begin();

  if (!RTC.begin()) {
    Serial.println("DS1307 not detected");
    while (1);
  }

  // Configure the RTC with the current time
  RTC.set(now());
}

void loop() {
  // Read and display the current time
  time_t currentTime = RTC.get();
  Serial.print("Current time: ");
  Serial.println(currentTime);
  delay(1000);
}
```

## Contributions
If you encounter an issue or have ideas to improve this library:
1. Create an **issue** in the repository.
2. You can also submit a **pull request** with your suggestions.

## License
This library is distributed under the GNU Lesser General Public License (LGPL) version 3. See the `COPYING.LESSER.txt` file for more details.

## Additional Resources
- [DS1307 Datasheet](docs/DS1307_datasheet.pdf)
- Included examples:
  - `RTC_ClockOut.ino`: Configuring the square wave output.
  - `RTC_Ram.ino`: Reading and writing to the RTC's RAM.
  - `RTC_Time.ino`: Configuring and reading time.