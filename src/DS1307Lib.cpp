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

#include <Wire.h>
#include "DS1307Lib.h"

#define DS1307_ADDRESS 0x68 // I2C address of the RTC

DS1307Clock::DS1307Clock() {
    Wire.begin();
    present = false;
}

// Public functions

time_t DS1307Clock::get() {
    tmElements_t tm;
    if (!readTime(tm)) return 0;
    return makeTime(tm);
}

bool DS1307Clock::set(time_t t) {
    tmElements_t tm;
    breakTime(t, tm);
    return writeTime(tm);
}

bool DS1307Clock::readTime(tmElements_t &tm) {
    uint8_t sec;
    Wire.beginTransmission(DS1307_ADDRESS);
    Wire.write((uint8_t)0x00);

    if (Wire.endTransmission() != 0) {
        present = false;
        return false;
    }

    present = true;
	Wire.requestFrom((uint8_t)DS1307_ADDRESS, (uint8_t)7); // Read 7 bytes

    if (Wire.available() < 7) return false;

    sec = Wire.read();
    tm.Second = bcdToD(sec & 0x7F);
    tm.Minute = bcdToD(Wire.read());
    uint8_t hourReg = Wire.read();
    tm.Hour = decodeHourRegister(hourReg); // Decoding hour register, 12/24-hour flags, and AM/PM
    tm.Wday = bcdToD(Wire.read());
    tm.Day = bcdToD(Wire.read());
    tm.Month = bcdToD(Wire.read());
    tm.Year = y2kYearToTm(bcdToD(Wire.read()));

    if (sec & 0x80) return false;

    return true;
}

bool DS1307Clock::writeTime(tmElements_t &tm) {
    // Stop the clock (CH bit activated)
    Wire.beginTransmission(DS1307_ADDRESS);
    Wire.write((uint8_t)0x00); // Point to the seconds register
    Wire.write((uint8_t)0x80); // Activate CH bit to stop the clock

    // Convert and write main registers in BCD format
    Wire.write(dToBcd(tm.Minute));
    Wire.write(encodeHourRegister(tm.Hour)); // Use encodeHourRegister to write the hour register
    Wire.write(dToBcd(tm.Wday));  // Day of the week
    Wire.write(dToBcd(tm.Day));   // Day of the month
    Wire.write(dToBcd(tm.Month)); // Month
    Wire.write(tmYearToY2k(tm.Year)); // Year

    // Check if the transmission failed
    if (Wire.endTransmission() != 0) {
        present = false;
        return false;
    }

    // Restart the clock by writing seconds (CH bit deactivated)
    Wire.beginTransmission(DS1307_ADDRESS);
    Wire.write((uint8_t)0x00); // Point to the seconds register
    Wire.write(dToBcd(tm.Second)); // Write seconds with CH bit deactivated

    if (Wire.endTransmission() != 0) {
        present = false;
        return false;
    }

    present = true;
    return true;
}

unsigned char DS1307Clock::isRunning() {
    uint8_t secondsRegister = readReg(0x00);
    return !(secondsRegister & 0x80);
}

void DS1307Clock::configureClockOut() {
    uint8_t reg = 0;
    reg |= default_out_state ? 0x80 : 0x00;
    reg |= clockout_en ? 0x10 : 0x00;
    reg |= (clockout_divider & 0x03);
    writeReg(0x07, reg);
}

void DS1307Clock::updateClockOut() {
    uint8_t reg = readReg(0x07);
    default_out_state = reg & 0x80;
    clockout_en = reg & 0x10;
    clockout_divider = reg & 0x03;
}

uint8_t DS1307Clock::readReg(uint8_t regAddress) {
    Wire.beginTransmission(DS1307_ADDRESS);
    Wire.write(regAddress);
    Wire.endTransmission();

    Wire.requestFrom((uint8_t)DS1307_ADDRESS, (uint8_t)1);
    return Wire.available() ? Wire.read() : 0xFF;
}

void DS1307Clock::writeReg(uint8_t regAddress, uint8_t value) {
    Wire.beginTransmission(DS1307_ADDRESS);
    Wire.write(regAddress);
    Wire.write(value);
    Wire.endTransmission();
}

// Private functions
uint8_t DS1307Clock::dToBcd(uint8_t num) {
    return ((num / 10) << 4) | (num % 10); // Tens in the upper 4 bits, units in the lower 4 bits
}

uint8_t DS1307Clock::bcdToD(uint8_t num) {
    return ((num >> 4) * 10) + (num & 0x0F); // Tens from the upper 4 bits, units from the lower 4 bits
}

// Private function to decode the hour register, 12/24-hour flags, and AM/PM
uint8_t DS1307Clock::decodeHourRegister(uint8_t hourReg) {
    uint8_t hour;

    // Determine 12/24-hour format and set mode12_24
    if (hourReg & 0x40) { // 12-hour format (bit 6 is active)
        mode12_24 = true; // It is 12-hour format

        // Extract bits 4-0 (hour in 12-hour format)
        hour = bcdToD(hourReg & 0x1F);

        // Check AM/PM indicator (bit 5)
        if (hourReg & 0x20) {
            am_pm = true; // PM
            hour += 12;   // Convert to 24-hour format
        } else {
            am_pm = false; // AM
        }

        // Adjust midnight
        if (hour == 24) hour = 0;

    } else { // 24-hour format
        mode12_24 = false; // It is 24-hour format
        am_pm = false;     // AM/PM not applicable in 24-hour format

        // Extract bits 5-0 (hour in 24-hour format)
        hour = bcdToD(hourReg & 0x3F);
    }

    return hour;
}

// Private function to encode the hour register with 12/24-hour flags and AM/PM
uint8_t DS1307Clock::encodeHourRegister(uint8_t hour) {
    uint8_t hourReg;

    if (mode12_24) { // 12-hour format
        hourReg = dToBcd(hour % 12); // Convert to 12-hour format (0-11)
        if (hour == 0 || hour == 12) {
            hourReg = dToBcd(12); // Adjust midnight or noon
        }
        hourReg |= 0x40; // Activate bit 6 for 12-hour format
        if (am_pm) {
            hourReg |= 0x20; // Activate bit 5 for PM
        }
    } else { // 24-hour format
        hourReg = dToBcd(hour); // Convert to 24-hour format (0-23)
    }

    return hourReg;
}

// Static variable initialization
bool DS1307Clock::present = false;
bool DS1307Clock::running = false;
bool DS1307Clock::mode12_24 = false;
bool DS1307Clock::am_pm = false;
bool DS1307Clock::clockout_en = false;
bool DS1307Clock::default_out_state = false;
uint8_t DS1307Clock::clockout_divider = 0;

DS1307Clock RTC = DS1307Clock();
