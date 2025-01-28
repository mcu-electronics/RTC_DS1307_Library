/*
 * RTC_ClockOut.ino - Example to configure the DS1307 clock output and check RTC presence.
 *
 * Based on the original work by Michael Margolis and expanded as part of the DS1307Lib library
 * by Alejandro Meza.
 *
 * This example demonstrates:
 * - SET_CLOCKOUT: Command to configure the DS1307's clock out pin.
 * - CHECK_RTC:    Command to verify if the DS1307 chip is present on the I2C bus.
 *
 * Notes:
 * - In addition to the standard square-wave configurations (1 Hz, 4 kHz, 8 kHz, 32 kHz),
 *   the DS1307's clock out pin can also be forced HIGH or LOW, effectively using it
 *   as a simple GPIO (digital output).
 * - This sketch follows the same structure and style as other examples in this library.
 *
 * Dependencies:
 * - DS1307Lib (for DS1307 RTC handling and clock output configuration)
 * - TimeLib   (for time manipulation, though not heavily used in this example)
 * - Wire      (for I2C communication)
 *
 * Author: Alejandro Meza
 * Website: http://mcuelectronica.com.ar
 * Contact: mcu.electronica@gmail.com
 *
 * License:
 * This example is distributed under the terms of the GNU Lesser General Public License (LGPL)
 * as published by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version. See <https://www.gnu.org/licenses/>.
 *
 * Created: January 27, 2025
 */


// ---------------------- Includes ----------------------
#include <TimeLib.h>    // Library for handling time on Arduino
#include <Wire.h>       // Library for I2C communication
#include <DS1307Lib.h>  // DS1307 RTC library

// ---------------------- Global configuration ----------------------
unsigned long lastCommandTime = 0;
const unsigned long commandInterval = 1000; // Interval of 1 second

// ---------------------- Function declarations ----------------------
void processSetClockOut(String params);
void processCheckRTC(String params);

// ---------------------- Command structure ----------------------
struct Command {
  const char* name;            // Command name
  void (*function)(String);    // Pointer to the function that executes the command
};

// ---------------------- Command table ----------------------
Command commandTable[] = {
  {"SET_CLOCKOUT", processSetClockOut},
  {"CHECK_RTC",    processCheckRTC}
};

const int commandCount = sizeof(commandTable) / sizeof(Command);

// ---------------------- setup() ----------------------
void setup() {
  Serial.begin(9600);
  while (!Serial);   // Wait for serial port to be ready (needed on boards like Leonardo)

  // Set A3 pin as input
  pinMode(A3, INPUT);
  // Set LED_BUILTIN as output
  pinMode(LED_BUILTIN, OUTPUT);

  // Optionally synchronize with RTC; here we mostly focus on clock out usage.
  setSyncProvider(RTC.get);
}

// ---------------------- loop() ----------------------
void loop() {
  // Check if there is any serial data available
  if (Serial.available()) {
    // Read incoming message until newline
    String receivedMessage = Serial.readStringUntil('\n');
    receivedMessage.trim();  // Remove whitespace and control characters

    // Extract command and parameters
    int spaceIndex = receivedMessage.indexOf(' ');
    String command = (spaceIndex == -1) ? receivedMessage : receivedMessage.substring(0, spaceIndex);
    String params  = (spaceIndex == -1) ? "" : receivedMessage.substring(spaceIndex + 1);

    // Search and execute the command in the table
    bool commandFound = false;
    for (int i = 0; i < commandCount; i++) {
      if (command.equalsIgnoreCase(commandTable[i].name)) {
        commandTable[i].function(params);
        commandFound = true;
        break;
      }
    }

    if (!commandFound) {
      Serial.println("Invalid Command");
    }

    lastCommandTime = millis(); // Update the last command time
  }

  // Read A3 pin state and set LED_BUILTIN accordingly
  int stateA3 = digitalRead(A3);
  digitalWrite(LED_BUILTIN, stateA3);

  // Periodic tasks (if needed)
  if (millis() - lastCommandTime >= commandInterval) {
    lastCommandTime = millis();
    // Add any additional periodic tasks here if necessary
  }
}

// ---------------------- Helper functions ----------------------

/**
 * SET_CLOCKOUT - Configures the DS1307's clock out pin.
 * Possible parameters:
 *   - "ON":  Enable square wave output.
 *   - "OFF": Disable square wave (pin goes to default level).
 *   - "H":   Disable square wave, force pin to HIGH (acts like a GPIO output).
 *   - "L":   Disable square wave, force pin to LOW  (acts like a GPIO output).
 *   - "1", "2", "3", "4": Select the clock out divider (1 Hz, 4 kHz, 8 kHz, 32 kHz).
 * 
 * Example usage:
 *   SET_CLOCKOUT ON
 *   SET_CLOCKOUT OFF
 *   SET_CLOCKOUT H
 *   SET_CLOCKOUT L
 *   SET_CLOCKOUT 4
 */
void processSetClockOut(String params) {
  // Synchronize internal variables with the RTC register
  RTC.updateClockOut();

  // ON: Enable clock out (square wave)
  if (params.equalsIgnoreCase("ON")) {
    RTC.clockout_en = true;
    RTC.configureClockOut();
    Serial.println("OK");

  // OFF: Disable clock out (pin goes to default state)
  } else if (params.equalsIgnoreCase("OFF")) {
    RTC.clockout_en = false;
    RTC.configureClockOut();
    Serial.println("OK");

  // H: Pin forced to HIGH (no square wave), use pin as a digital output
  } else if (params.equalsIgnoreCase("H")) {
    RTC.clockout_en = false;
    RTC.default_out_state = true;
    RTC.configureClockOut();
    Serial.println("OK");

  // L: Pin forced to LOW (no square wave), use pin as a digital output
  } else if (params.equalsIgnoreCase("L")) {
    RTC.clockout_en = false;
    RTC.default_out_state = false;
    RTC.configureClockOut();
    Serial.println("OK");

  // Numeric divider selection
  } else {
    int divider = params.toInt();
    // DS1307 supports four divider settings (0..3), typically mapped to 1 Hz, 4 kHz, 8 kHz, 32 kHz.
    // Here we interpret param "1" to "4" as (divider - 1) internally.
    if (divider >= 1 && divider <= 4) {
      RTC.clockout_divider = divider - 1;
      RTC.configureClockOut();
      Serial.println("OK");
    } else {
      Serial.println("ERROR");
    }
  }
}

/**
 * CHECK_RTC - Checks if the DS1307 chip is present on the I2C bus.
 * Prints "OK" if present, otherwise "ERROR".
 */
void processCheckRTC(String params) {
  if (RTC.isChipPresent()) {
    Serial.println("OK");
  } else {
    Serial.println("ERROR");
  }
}
