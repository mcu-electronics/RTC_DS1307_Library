"""
=======================================================
local_test.py - Local Time Detection Utility
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
This script detects the system's local timezone and displays both
the current UTC time and the local time using the appropriate timezone.

Usage:
------
Simply run the script:

    python local_test.py

The output will display:
- The detected local timezone.
- The current UTC time.
- The local time converted using the detected timezone.

"""

from datetime import datetime
from tzlocal import get_localzone
import pytz

# Automatically detect the system's local timezone
local_tz = get_localzone()

# Get current UTC time
now_utc = datetime.now(pytz.utc)

# Get local time with the detected timezone
now_local = datetime.now(pytz.timezone(str(local_tz)))

# Display results
print(f"🕒 Detected Timezone      : {local_tz}")
print(f"🌍 UTC Time in Python    : {now_utc.strftime('%Y/%m/%d %H:%M:%S %Z%z')}")
print(f"🏠 Local Time in Python  : {now_local.strftime('%Y/%m/%d %H:%M:%S %Z%z')}")
