/*
 * RTC_Time.ino - Example to set, get, and synchronize time using the DS1307 RTC.
 *
 * Based on the original work by Michael Margolis and expanded as part of the DS1307Lib library
 * by Alejandro Meza.
 *
 * This example demonstrates:
 * - SET_TIME:  Command to set the RTC time in human-readable format (YYYY/MM/DD HH:MM:SS).
 * - SET_UNIX:  Command to set the RTC time using a Unix timestamp.
 * - GET_TIME:  Command to retrieve the current RTC time in human-readable format.
 * - GET_UNIX:  Command to retrieve the current RTC time as a Unix timestamp.
 * - SET_FORMAT: Command to switch between 12-hour and 24-hour formats.
 * - CHECK_RTC: Command to verify if the DS1307 chip is present on the I2C bus.
 *
 * Notes:
 * - This example uses the DS1307Lib library to communicate with the RTC and manage time.
 * - Time synchronization is provided via the TimeLib library.
 * - This example follows a command-based structure for flexibility and modularity.
 *
 * Dependencies:
 * - DS1307Lib (for DS1307 RTC handling)
 * - TimeLib   (for time manipulation)
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

#include <TimeLib.h>    // Library for handling time on Arduino
#include <Wire.h>       // Library for I2C communication
#include <DS1307Lib.h>  // DS1307 RTC library, returns time as time_t (seconds since 1970)

// ---------------------- Global configuration ----------------------
unsigned long lastCommandTime = 0;
const unsigned long commandInterval = 1000; // Interval of 1 second

// ---------------------- Command structure ----------------------
struct Command {
  const char* name;            // Command name
  void (*function)(String);    // Pointer to the function that executes the command
};

// ---------------------- Function declarations ----------------------
void processSetTime(String params);
void processSetTimeUnix(String params);
void sendCurrentTime(String params = "");
void sendUnixTimestamp(String params);
void processSetHourFormat(String params); 
void processCheckRTC(String params);


// ---------------------- Command table ----------------------
// Includes only time-related commands plus CHECK_RTC
Command commandTable[] = {
  {"SET_TIME",  processSetTime},
  {"SET_UNIX",  processSetTimeUnix},
  {"GET_TIME",  sendCurrentTime},
  {"GET_UNIX",  sendUnixTimestamp},
  {"SET_FORMAT",  processSetHourFormat},
  {"CHECK_RTC", processCheckRTC}
};

const int commandCount = sizeof(commandTable) / sizeof(Command);

// ---------------------- setup() ----------------------
void setup() {
  Serial.begin(9600);
  while (!Serial);   // Wait for serial port to be ready (needed on boards like Leonardo)

  // Set A3 pin as input for SQWOUT pin
  pinMode(A3, INPUT);
  // Set LED_BUILTIN as output
  pinMode(LED_BUILTIN, OUTPUT);

  RTC.begin();
  if (timeStatus() != timeSet) {
      Serial.println("Unable to sync with the RTC");
  }
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
    // Add any additional periodic tasks here
  }
}

// ---------------------- Helper functions ----------------------

/**
 * SET_TIME - Sets time in human-readable format: "YYYY/MM/DD HH:MM:SS"
 */
void processSetTime(String timeString) {
  int year, month, day, hour, minute, second;
  // Parse the incoming string
  if (sscanf(timeString.c_str(), "%d/%d/%d %d:%d:%d",
             &year, &month, &day, &hour, &minute, &second) == 6) {
    setTime(hour, minute, second, day, month, year);    // Set local time on Arduino
    RTC.set(now()); // Set the RTC with the entered time
    sendCurrentTime();    // Print the newly set time
  } else {
    Serial.println("Invalid time format");
  }
}

/**
 * SET_UNIX - Sets time based on a Unix timestamp
 */
void processSetTimeUnix(String unixString) {
  unsigned long unixTime = unixString.toInt(); // Convert the string to a long integer
  if (unixTime > 0) {
    setTime(unixTime); // Set the system time
    RTC.set(now());    // Set the RTC with the entered time
    sendUnixTimestamp(""); // Display the configured timestamp
  } else {
    Serial.println("Invalid Unix timestamp"); // Invalid timestamp
  }
}


void printDigits(int digits) {
    Serial.print(":");
    if (digits < 10) Serial.print('0');
    Serial.print(digits);
}

/**
 * GET_TIME - Prints the current time in "YYYY/MM/DD HH:MM:SS"
 */
void sendCurrentTime(String params) {
    RTC.readHourFormat(); // Update format configuration

    Serial.print(year());
    Serial.print("/");
    Serial.print(month());
    Serial.print("/");
    Serial.print(day());
    Serial.print(" ");

    int h = hour(); // Get the hour in 24h format

    if (RTC.is12HourFormat) {
        RTC.isPMFlag = isPM(h); // Determine if it's AM or PM
        int hour12 = (h == 0) ? 12 : (h > 12 ? h - 12 : h); // Convert 0->12 AM, 13->1 PM, etc.

        Serial.print(hour12);
        printDigits(minute());
        printDigits(second());
        Serial.println(RTC.isPMFlag ? " PM" : " AM");
    } else {
        Serial.print(h);
        printDigits(minute());
        printDigits(second());
        Serial.println();
    }
}



/**
 * GET_UNIX - Prints the current time as a Unix timestamp
 */
void sendUnixTimestamp(String params) {
  time_t currentTime = RTC.get();
  if (currentTime != 0) {
    Serial.println(currentTime);
  } else {
    Serial.println("Unable to read time from RTC");
  }
}


/**
 * SET_FORMAT - Set hour format (12/24)
 * Usage: "SET_FORMAT 12" or "SET_FORMAT 24"
 */
void processSetHourFormat(String params) {
    if (params.equals("12") || params.equals("24")) {
        RTC.readHourFormat();
        RTC.is12HourFormat = params.equals("12");
        RTC.writeHourFormat();
        sendCurrentTime();    // Print the newly set time
    } else {
        Serial.println("ERROR");
    }
}


/**
 * CHECK_RTC - Checks if the DS1307 is present
 */
void processCheckRTC(String params) {
  if (RTC.isChipPresent()) {
    Serial.println("OK");
  } else {
    Serial.println("ERROR");
  }
}



