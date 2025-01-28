/*
 * RTC_Ram.ino - Example to read and write data to the DS1307's battery-backed RAM.
 *
 * Based on the original work by Michael Margolis and expanded as part of the DS1307Lib library
 * by Alejandro Meza.
 *
 * This example demonstrates:
 * - READ_RAM:  Command to read a block of bytes from the DS1307's user RAM.
 * - WRITE_RAM: Command to write a block of bytes to the DS1307's user RAM.
 * - CHECK_RTC: Command to verify if the DS1307 chip is present on the I2C bus.
 *
 * Notes:
 * - The DS1307 includes 56 bytes of battery-backed RAM, located in registers 0x08 to 0x3F.
 * - This example follows the same structure and style as RTC_Time.ino and RTC_ClockOut.ino.
 *
 * Dependencies:
 * - DS1307Lib (for DS1307 RTC handling and RAM operations)
 * - TimeLib   (optional, for time data structures, not heavily used in this example)
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
#include <TimeLib.h>    // Library for handling time on Arduino (optional here)
#include <Wire.h>       // Library for I2C communication
#include <DS1307Lib.h>  // DS1307 RTC library

// ---------------------- Global configuration ----------------------
unsigned long lastCommandTime = 0;
const unsigned long commandInterval = 1000; // Interval of 1 second

// ---------------------- Function declarations ----------------------
void processReadRAM(String params);
void processWriteRAM(String params);
void processCheckRTC(String params);

// ---------------------- Command structure ----------------------
struct Command {
  const char* name;            // Command name
  void (*function)(String);    // Pointer to the function that executes the command
};

// ---------------------- Command table ----------------------
Command commandTable[] = {
  {"READ_RAM",  processReadRAM},
  {"WRITE_RAM", processWriteRAM},
  {"CHECK_RTC", processCheckRTC}
};

const int commandCount = sizeof(commandTable) / sizeof(Command);

// ---------------------- setup() ----------------------
void setup() {
  Serial.begin(9600);
  while (!Serial);   // Wait for serial port to be ready (needed on boards like Leonardo)

  // A3 is used as input to read the SQW/OUT pin of the DS1307 (clock out signal).
  pinMode(A3, INPUT);  
  // LED_BUILTIN is used here as an indicator for the clock out signal.
  // The loop() function will read A3 and mirror its state onto the LED.
  pinMode(LED_BUILTIN, OUTPUT);

  // Optional: set up TimeLib's synchronization with DS1307
  // (Not strictly required for RAM operations, but included for consistency)
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
    // Add any additional periodic tasks here
  }
}

// ---------------------- Helper functions ----------------------

/**
 * READ_RAM - Reads a block of bytes from the DS1307's battery-backed RAM.
 * Parameters format: "ADDR_HEX LENGTH_DEC"
 * Example usage: "READ_RAM 00 16" -> Reads 16 bytes starting from address 0x00
 */
void processReadRAM(String params) {
  int startAddr, length;

  // Parse parameters (start address in hex, length in decimal)
  if (sscanf(params.c_str(), "%x %d", &startAddr, &length) == 2) {
    // Validate address is in [0x00, 0x3F] and the block doesn't exceed 0x3F
    if (startAddr >= 0x00 && startAddr <= 0x3F && length > 0 && (startAddr + length - 1) <= 0x3F) {
      Serial.print("RAM: [");

      for (int i = 0; i < length; i++) {
        uint8_t dataByte = RTC.readReg(startAddr + i);

        // Print each byte in hex format
        Serial.print("0x");
        Serial.print(dataByte, HEX);

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

/**
 * WRITE_RAM - Writes a sequence of bytes to the DS1307's battery-backed RAM.
 * Parameters format: "ADDR_HEX VAL1_HEX VAL2_HEX ..."
 * Example usage: "WRITE_RAM 10 A5 5A FF" -> Writes 0xA5 at 0x10, 0x5A at 0x11, 0xFF at 0x12
 */
void processWriteRAM(String params) {
  int startAddr;
  uint8_t values[64];  // DS1307 has 56 bytes available, 64 for safety
  int valueCount = 0;

  char* tokens = strtok((char*)params.c_str(), " ");
  if (tokens != NULL) {
    // First token is the start address (in hex)
    startAddr = strtol(tokens, NULL, 16);

    // Validate address range
    if (startAddr >= 0x00 && startAddr <= 0x3F) {
      // Get the rest of the tokens as hex values
      tokens = strtok(NULL, " ");
      while (tokens != NULL && valueCount < (0x3F - startAddr + 1)) {
        values[valueCount++] = (uint8_t)strtol(tokens, NULL, 16);
        tokens = strtok(NULL, " ");
      }

      if (valueCount > 0) {
        // Write each byte to the DS1307 RAM
        for (int i = 0; i < valueCount; i++) {
          RTC.writeReg(startAddr + i, values[i]);
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
