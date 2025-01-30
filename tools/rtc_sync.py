"""
=======================================================
rtc_sync.py - RTC Synchronization Tool for DS1307
=======================================================

Copyright (c) 2025 Alejandro Meza
Website: http://mcuelectronica.com.ar
Contact: mcu.electronica@gmail.com

This script is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License
as published by the Free Software Foundation; either version 2.1
of the License, or (at your option) any later version.

This script is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this script; if not, visit <https://www.gnu.org/licenses/>.

Created: January 30, 2025

Description:
------------
This script synchronizes the UNIX timestamp of a DS1307 RTC module 
via a serial connection. It allows storing either UTC time or 
local time in the RTC and retrieving it correctly.

Usage:
------
1. Install dependencies:
   pip install pyserial pytz tzlocal

2. Run the script specifying the COM port and time type:
   - Store UTC time:    python rtc_sync.py COM5 utc
   - Store Local time:  python rtc_sync.py COM5 local

3. The RTC will be updated with the selected time, and the stored 
   time will be retrieved and displayed.

"""

import argparse
import serial
import time
from datetime import datetime, timezone
import pytz
from tzlocal import get_localzone

# Automatically detect the system's local timezone
TZ_LOCAL = get_localzone()

def get_unix_time(time_type):
    """
    Retrieves the UNIX timestamp in either UTC or Local time, depending on the selected option.

    Args:
        time_type (str): "utc" to get the UTC timestamp, "local" to get the local timestamp.

    Returns:
        int: The UNIX timestamp to be sent to the RTC.
    """
    now_local = datetime.now(pytz.timezone(str(TZ_LOCAL)))  # Get local time with correct timezone
    now_utc = datetime.now(pytz.utc)  # Get current UTC time

    if time_type == 'utc':
        unix_timestamp = int(now_utc.timestamp())  # Standard UTC UNIX timestamp
        print(f"🕒 UNIX Timestamp UTC to send to RTC: {unix_timestamp}")

    else:  # If "local" is selected
        offset_seconds = now_local.utcoffset().total_seconds()  # Get the UTC offset in seconds
        unix_timestamp = int(now_utc.timestamp()) + int(offset_seconds)  # Adjust to store real local time
        print(f"🕒 UNIX Timestamp Local to send to RTC: {unix_timestamp}")

    return unix_timestamp

def detect_rtc(ser, retries=3):
    """
    Checks if the RTC is responding correctly before attempting synchronization.

    Args:
        ser (serial.Serial): Opened serial connection to the RTC.
        retries (int, optional): Number of attempts before giving up. Defaults to 3.

    Returns:
        bool: True if the RTC responds correctly, False otherwise.
    """
    for attempt in range(retries):
        ser.write(b"CHECK_RTC\n")
        time.sleep(1.5)

        if ser.in_waiting > 0:
            response = ser.readline().decode('utf-8').strip()
            print(f"📡 Attempt {attempt + 1}: Response received -> {response}")
            if response == "OK":
                return True
        else:
            print(f"⏳ No response received on attempt {attempt + 1}. Retrying...")

    return False

def get_human_readable_time(ser, time_type):
    """
    Retrieves and displays the date and time stored in the RTC.

    Args:
        ser (serial.Serial): Opened serial connection to the RTC.
        time_type (str): "utc" if the RTC stores UTC time, "local" if it stores local time.
    """
    ser.write(b"GET_TIME\n")
    time.sleep(2)

    if ser.in_waiting > 0:
        response = ser.readline().decode('utf-8').strip()
        print(f"📅 Time stored in RTC: {response}")

        try:
            rtc_datetime = datetime.strptime(response, "%Y/%m/%d %H:%M:%S")

            if time_type == "utc":
                print(f"🕒 RTC Time: {rtc_datetime.strftime('%Y/%m/%d %H:%M:%S')} UTC")
            else:
                print(f"🕒 RTC Time: {rtc_datetime.strftime('%Y/%m/%d %H:%M:%S')} Local")

        except ValueError:
            print("⚠️  Unrecognized time format in GET_TIME.")
    else:
        print("⚠️  No response received from RTC for GET_TIME.")

def send_command(port_name, time_type):
    """
    Sends the SET_UNIX command with the correct timestamp to the RTC and then retrieves GET_TIME.

    Args:
        port_name (str): Name of the serial port (e.g., "COM5" or "/dev/ttyUSB0").
        time_type (str): "utc" to store UTC time, "local" to store local time.
    """
    try:
        with serial.Serial(port_name, baudrate=9600, timeout=3) as ser:
            print(f"🔌 Connected to port {port_name}, waiting for initialization...")
            time.sleep(2)
            ser.reset_input_buffer()
            time.sleep(1)

            if not detect_rtc(ser):
                print("⚠️  No RTC detected on the selected port.")
                return

            print("✅ RTC device successfully detected.")

            unix_time = get_unix_time(time_type)
            command = f"SET_UNIX {unix_time}\n"

            ser.write(command.encode('utf-8'))
            print(f"📤 Sent: {command.strip()}")

            time.sleep(2)
            response = ser.readline().decode('utf-8').strip()

            if response == str(unix_time):
                print("✅ RTC successfully updated. Sent and received values match.")
                get_human_readable_time(ser, time_type)  # Retrieve stored RTC time
            else:
                print(f"⚠️  Unexpected response from device: {response} (expected: {unix_time})")

    except serial.SerialException as e:
        print(f"❌ Error opening port {port_name}: {e}")
    except Exception as e:
        print(f"❌ An unexpected error occurred: {e}")

def main():
    """
    Main function to execute the script from the command line.
    Parses arguments and calls the synchronization function.
    """
    parser = argparse.ArgumentParser(description="Synchronizes UNIX time with a DS1307 RTC via the serial port.")
    parser.add_argument("port", type=str, help="COM port name (e.g., COM3 or /dev/ttyUSB0).")
    parser.add_argument("time_type", type=str, choices=["local", "utc"], help="Time type to store in RTC: local or UTC.")

    args = parser.parse_args()
    send_command(args.port, args.time_type)

if __name__ == "__main__":
    main()
