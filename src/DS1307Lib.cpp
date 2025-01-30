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

void DS1307Clock::begin() {   // Initializes the RTC and sets TimeLib synchronization.
    setSyncProvider(RTC.get); // Calls setSyncProvider() to use the RTC as the time provider,
    //setSyncInterval(3600);  // Sincroniza cada hora (3600 segundos)
}

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
    tm.Hour = bcdToD(Wire.read() & 0x3f);//Mask = 00111111 24h Format
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
    Wire.write(dToBcd(tm.Minute));// Minutes
    Wire.write(dToBcd(tm.Hour));  // Hours
    Wire.write(dToBcd(tm.Wday));  // Day of the week
    Wire.write(dToBcd(tm.Day));   // Day of the month
    Wire.write(dToBcd(tm.Month)); // Month
    Wire.write(dToBcd(tmYearToY2k(tm.Year))); // Year

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


bool DS1307Clock::readHourFormat() {
    uint8_t hourReg = readReg(0x02); // Leer el registro de hora del DS1307

    // Verificar si está en formato de 12 horas
    is12HourFormat = (hourReg & 0x40) != 0;

    if (is12HourFormat) {
        uint8_t hour24 = bcdToD(hourReg & 0x3F); // Extraer hora en formato 24h
        isPMFlag = isPM(hour24); // Ahora no hay conflicto con TimeLib
    } else {
        isPMFlag = false; // En formato 24h, no existe AM/PM
    }
    return true;
}


bool DS1307Clock::writeHourFormat() {
    uint8_t hourReg = readReg(0x02); // Leer el registro de hora

    uint8_t hour24 = bcdToD(hourReg & 0x3F); // Convertir BCD a decimal (24h)
    uint8_t newReg = 0;

    if (is12HourFormat) {
        uint8_t hour12 = hourFormat12(hour24); // Convertir a formato 12h
        isPMFlag = isPM(hour24); // Ahora no hay conflicto con TimeLib

        newReg = 0x40; // Bit 6 en 1 = formato 12h
        if (isPMFlag) newReg |= 0x20; // Bit 5 en 1 = PM
        newReg |= dToBcd(hour12) & 0x1F; // Convertir a BCD (1-12)
    } else {
        newReg = dToBcd(hour24) & 0x3F; // Mantener formato 24h
    }

    writeReg(0x02, newReg); // Escribir la nueva configuración
    return true;
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

// Static variable initialization
bool DS1307Clock::present = false;
bool DS1307Clock::running = false;
bool DS1307Clock::is12HourFormat = false;
bool DS1307Clock::isPMFlag = false;
bool DS1307Clock::clockout_en = false;
bool DS1307Clock::default_out_state = false;
uint8_t DS1307Clock::clockout_divider = 0;

DS1307Clock RTC = DS1307Clock();
