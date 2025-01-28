/*
 * DS1307Lib.cpp - Library implementation for the DS1307 Real-Time Clock (RTC)
 * 
 * Based on the DS1307RTC library by Michael Margolis.
 * 
 * Copyright (c) 2025 Alejandro Meza
 * Website: http://mcuelectronica.com.ar
 * Contact: mcu.electronica@gmail.com
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, visit <https://www.gnu.org/licenses/>.
 * 
 * Created: January 27, 2025
 */

#ifndef DS1307LIB_H
#define DS1307LIB_H

#include <TimeLib.h>

// Class DS1307Clock for interacting with the DS1307 RTC module.
// Provides methods to read and write time, manage the RTC state,
// and access specific device registers.
class DS1307Clock {
  public:
    // Constructor of the DS1307Clock class.
    DS1307Clock();

    // Retrieves the current time from the RTC as a Unix timestamp.
    // Returns the current time in seconds since 1970.
    static time_t get();

    // Sets the current time on the RTC using a Unix timestamp.
    // Returns true if the configuration was successful, false in case of an error.
    static bool set(time_t t);

    // Reads the time from the RTC and stores it in a tmElements_t structure.
    // Returns true if the read was successful, false if an error occurred.
    static bool readTime(tmElements_t &tm);

    // Writes the time to the RTC using a tmElements_t structure.
    // Returns true if the write was successful, false in case of an error.
    static bool writeTime(tmElements_t &tm);

    // Verifies if the DS1307 RTC chip is present.
    // Returns true if the chip is connected, false otherwise.
    static bool isChipPresent() { return present; }

    // Checks if the RTC is running.
    // Returns 1 if the RTC is running, 0 if not.
    static unsigned char isRunning();

    // Configures the DS1307 control register based on internal variables.
    static void configureClockOut();

    // Synchronizes the internal variables with the DS1307 control register.
    static void updateClockOut();

    // State and mode variables.
    static bool present;             // Indicates if the RTC chip is present on the I2C bus.
    static bool running;             // Indicates if the clock is running.
    static bool mode12_24;           // Hour mode: true = 12-hour, false = 24-hour.
    static bool am_pm;               // Indicates AM or PM if in 12-hour mode.
    // Control register variables.
    static bool clockout_en;         // Indicates if the clock output is enabled.
    static bool default_out_state;   // Default state of the SQW/OUT pin.
    static uint8_t clockout_divider; // ClockOut divider.

    // Reads a byte from a specific register of the DS1307 RTC.
    // Returns the value read from the register or 0xFF if an error occurs.
    static uint8_t readReg(uint8_t regAddress);

    // Writes a byte to a specific register of the DS1307 RTC.
    static void writeReg(uint8_t regAddress, uint8_t value);

  private:
    // Private function to decode the hour register, 12/24-hour flags, and AM/PM.
    static uint8_t DS1307Clock::decodeHourRegister(uint8_t hourReg);
    // Private function to encode the hour register with 12/24-hour flags and AM/PM.
    static uint8_t DS1307Clock::encodeHourRegister(uint8_t hour);
    // Converts a decimal number to BCD format.
    static uint8_t dToBcd(uint8_t num);
    // Converts a BCD number to decimal format.
    static uint8_t bcdToD(uint8_t num);
};

#ifdef RTC
#undef RTC // workaround for Arduino Due, which defines "RTC"...
#endif

// Global instance of the DS1307Clock class for easy use.
extern DS1307Clock RTC;

#endif
