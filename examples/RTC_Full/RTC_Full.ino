/*
 * RTC_Full.ino - Comprehensive example to demonstrate all functionalities of the DS1307 RTC.
 *
 * This example combines all the features demonstrated in the individual examples:
 * - RTC_Time: Set, get, and synchronize time using human-readable format or Unix timestamps.
 * - RTC_ClockOut: Configure and control the DS1307's clock output pin.
 * - RTC_Ram: Read and write data to the DS1307's battery-backed RAM.
 *
 * Notes:
 * - This example demonstrates how to interact with the DS1307 RTC, covering all possible use cases.
 * - A command-based structure is used for modularity and flexibility.
 *
 * Features:
 * - SET_TIME: Set the RTC time in "YYYY/MM/DD HH:MM:SS" format.
 * - SET_UNIX: Set the RTC time using a Unix timestamp.
 * - GET_TIME: Retrieve the current time in human-readable format.
 * - GET_UNIX: Retrieve the current time as a Unix timestamp.
 * - SET_FORMAT: Command to switch between 12-hour and 24-hour formats.
 * - SET_CLOCKOUT: Configure the clock output pin with various settings.
 * - READ_RAM: Read a block of bytes from the DS1307's battery-backed RAM.
 * - WRITE_RAM: Write a block of bytes to the DS1307's battery-backed RAM.
 * - CHECK_RTC: Verify if the DS1307 chip is present on the I2C bus.
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





#include <TimeLib.h> // Library for managing time in Arduino
#include <Wire.h>    // Library for I2C communication
#include <DS1307Lib.h>  // Library for the DS1307 RTC, returns time as a time_t (seconds since 1970)

unsigned long lastCommandTime = 0;
const unsigned long commandInterval = 1000; // Interval of 1 second

// Structure to associate commands with functions
struct Command {
  const char* name; // Command name
  void (*function)(String); // Pointer to the function that executes the command
};

// Function declarations
void processSetTime(String params);
void processSetTimeUnix(String params);
void sendUnixTimestamp(String params);
void sendCurrentTime(String params = "");
void processSetHourFormat(String params); 
void processSetClockOut(String params);
void processCheckRTC(String params);
void processReadRAM(String params);
void processWriteRAM(String params);

// Command table
Command commandTable[] = {
  {"SET_TIME", processSetTime},
  {"SET_UNIX", processSetTimeUnix},
  {"GET_UNIX", sendUnixTimestamp},
  {"GET_TIME", sendCurrentTime},
  {"SET_FORMAT",  processSetHourFormat},
  {"SET_CLOCKOUT", processSetClockOut},
  {"CHECK_RTC", processCheckRTC},
  {"READ_RAM", processReadRAM},
  {"WRITE_RAM", processWriteRAM}  
};

const int commandCount = sizeof(commandTable) / sizeof(Command); // Number of commands

// Initial setup
void setup() {
  Serial.begin(9600); // Set up serial communication at 9600 baud
  while (!Serial);    // Wait for the serial port to be ready (necessary for boards like Leonardo)
  // Set A3 pin as input for SQWOUT pin
  pinMode(A3, INPUT);
  // Set LED_BUILTIN as output
  pinMode(LED_BUILTIN, OUTPUT);

  RTC.begin();
  if (timeStatus() != timeSet) {
      Serial.println("Unable to sync with the RTC");
  }
}

// Main loop
void loop() {
  if (Serial.available()) {
    String receivedMessage = Serial.readStringUntil('\n'); // Read the complete message until a newline
    receivedMessage.trim(); // Remove whitespace and control characters

    // Extract the command and parameters
    int spaceIndex = receivedMessage.indexOf(' ');
    String command = (spaceIndex == -1) ? receivedMessage : receivedMessage.substring(0, spaceIndex);
    String params = (spaceIndex == -1) ? "" : receivedMessage.substring(spaceIndex + 1);

    // Search for and execute the command in the table
    bool commandFound = false;
    for (int i = 0; i < commandCount; i++) {
      if (command.equalsIgnoreCase(commandTable[i].name)) {
        commandTable[i].function(params); // Call the function associated with the command
        commandFound = true;
        break;
      }
    }

    if (!commandFound) {
      Serial.println("Invalid Command"); // Unrecognized command
    }

    lastCommandTime = millis(); // Update the time of the last command
  }

  // Read the state of pin A3
  int estadoA3 = digitalRead(A3);
  // Set the built-in LED state according to A3's state
  digitalWrite(LED_BUILTIN, estadoA3);

  // Periodic tasks or actions based on the interval
  if (millis() - lastCommandTime >= commandInterval) {
    lastCommandTime = millis();
    // Add periodic tasks here if necessary
  }
}

// Auxiliary function implementations

/**
 * SET_TIME - Sets time in human-readable format: "YYYY/MM/DD HH:MM:SS"
 */
void processSetTime(String timeString) {
  int year, month, day, hour, minute, second;
  if (sscanf(timeString.c_str(), "%d/%d/%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) == 6) {
    setTime(hour, minute, second, day, month, year); // Set the system time
    RTC.set(now()); // Set the RTC with the entered time
    sendCurrentTime(); // Display the configured time
  } else {
    Serial.println("Invalid time format"); // Invalid format
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

// Processes the SET_CLOCKOUT command
void processSetClockOut(String params) {
    RTC.updateClockOut(); // Read the current state of the control register

    if (params.equalsIgnoreCase("ON")) {
        RTC.clockout_en = true; // Enable the clock output
        RTC.configureClockOut();
        Serial.println("OK");
    } else if (params.equalsIgnoreCase("OFF")) {
        RTC.clockout_en = false; // Disable the clock output
        RTC.configureClockOut();
        Serial.println("OK");
    } else if (params.equalsIgnoreCase("H")) {
        RTC.clockout_en = false; // Disable the clock output
        RTC.default_out_state = true; // Set the default state to HIGH
        RTC.configureClockOut();
        Serial.println("OK");
    } else if (params.equalsIgnoreCase("L")) {
        RTC.clockout_en = false; // Disable the clock output
        RTC.default_out_state = false; // Set the default state to LOW
        RTC.configureClockOut();
        Serial.println("OK");
    } else {
        int divider = params.toInt(); // Convert the parameter to an integer
        if (divider >= 1 && divider <= 4) {
            RTC.clockout_divider = divider - 1; // Set the divider (0, 1, 2, 3)
            RTC.configureClockOut();
            Serial.print("OK");
        } else {
            Serial.println("ERROR"); // Invalid parameter
        }
    }
}

// Processes the CHECK_RTC command
void processCheckRTC(String params) {
  if (RTC.isChipPresent()) {
    Serial.println("OK"); // RTC present
  } else {
    Serial.println("ERROR"); // RTC not found
  }
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



// Sends the current time as a Unix timestamp
void sendUnixTimestamp(String params) {
  time_t currentTime = RTC.get(); // Get the current time from the RTC
  if (currentTime != 0) { // Verify that the RTC returned a valid value
    Serial.println(currentTime); // Send the Unix timestamp to the serial port
  } else {
    Serial.println("Unable to read time from RTC"); // Error message if the time could not be read
  }
}

// Prints numbers with two digits
void printDigits(int digits) {
  Serial.print(":"); // Add a colon separator
  if (digits < 10)
    Serial.print('0'); // Add a leading zero if less than 10
  Serial.print(digits); // Print the number
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

/*
  processReadRAM:
  - Parameters: "ADDHex lengthDecimal"
  - Example: "READ_RAM 00 10" -> reads 16 bytes starting at 0x00
*/
void processReadRAM(String params) {
    int startAddr, length;

    // Parse parameters (start address and length)
    if (sscanf(params.c_str(), "%x %d", &startAddr, &length) == 2) {
        if (startAddr >= 0x00 && startAddr <= 0x3F && length > 0 && (startAddr + length - 1) <= 0x3F) {

            Serial.print("RAM: [");

            for (int i = 0; i < length; i++) {
                uint8_t data = RTC.readReg(startAddr + i);

                Serial.print("0x");
                Serial.print(data, HEX);

                if (i < length - 1) {
                    Serial.print(", ");
                }
            }

            Serial.println("]");

        } else {
            Serial.println("ERROR: Invalid address or length");
        }
    } else {
        Serial.println("ERROR: Invalid parameters");
    }
}

/*
  processWriteRAM:
  - Parameters: "ADDHex val1Hex val2Hex ..."
  - Example: "WRITE_RAM 10 01 02 03" -> writes 0x01 at 0x10, 0x02 at 0x11, 0x03 at 0x12
*/
void processWriteRAM(String params) {
    int startAddr;
    int values[64]; // Maximum possible bytes (0x3F - 0x00 + 1) = 64
    int valueCount = 0;

    char* token = strtok((char*)params.c_str(), " ");
    if (token != NULL) {
        startAddr = strtol(token, NULL, 16);

        if (startAddr >= 0x00 && startAddr <= 0x3F) {
            token = strtok(NULL, " ");
            while (token != NULL && valueCount < (0x3F - startAddr + 1)) {
                values[valueCount++] = strtol(token, NULL, 16);
                token = strtok(NULL, " ");
            }

            if (valueCount > 0) {
                for (int i = 0; i < valueCount; i++) {
                    RTC.writeReg(startAddr + i, (uint8_t)values[i]);
                }
                Serial.println("OK");
            } else {
                Serial.println("ERROR: No values provided");
            }
        } else {
            Serial.println("ERROR: Invalid start address");
        }
    } else {
        Serial.println("ERROR: Invalid parameters");
    }
}
