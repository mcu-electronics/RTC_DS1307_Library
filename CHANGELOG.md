# CHANGELOG - RTC_DS1307_Library

This file documents the most important changes in each version of the **RTC_DS1307_Library**.

---

## [1.1.1] - 2025-01-30
### Improvements and New Features
- Support for 12h/24h: You can now switch between time formats using the `SET_FORMAT` command.
- Added two Python tools in `tools/`:
  - `rtc_sync.py`: Synchronizes the RTC with the system time (UTC or Local).
  - `local_test.py`: Detects the system timezone and displays UTC and local time.
- Added datasheet:
  - `DS1307_Maxim.pdf`

### Bug Fixes
- Fixed an issue with setting the year field when updating the date.

---

## [1.0.0] - 2025-01-27
### Initial Release
- Compatibility with **Arduino, ESP32, and ESP8266**.
- Basic functionality:
  - Read and write time on the DS1307.
  - Configure and read internal RAM memory.
  - Control the clock output (`SQW`).
  - Support for synchronization with `TimeLib`.

---

Versioning Format:  
This changelog follows the [SemVer](https://semver.org/lang/en/) scheme with **MAJOR.MINOR.PATCH** versions:
- `MAJOR`: Incompatible changes with previous versions.
- `MINOR`: New backward-compatible features.
- `PATCH`: Bug fixes or minor improvements.