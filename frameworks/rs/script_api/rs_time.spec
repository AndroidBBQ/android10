#
# Copyright (C) 2015 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

header:
summary: Time Functions and Types
description:
 The functions below can be used to tell the current clock time and the current
 system up time.  It is not recommended to call these functions inside of a kernel.
end:

type: rs_time_t
size: 32
simple: int
summary: Seconds since January 1, 1970
description:
 Calendar time interpreted as seconds elapsed since the Epoch (00:00:00 on
 January 1, 1970, Coordinated Universal Time (UTC)).
end:

type: rs_time_t
size: 64
simple: long
end:

type: rs_tm
struct:
field: int tm_sec, "Seconds after the minute. This ranges from 0 to 59, but possibly up to 60 for leap seconds."
field: int tm_min, "Minutes after the hour. This ranges from 0 to 59."
field: int tm_hour, "Hours past midnight. This ranges from 0 to 23."
field: int tm_mday, "Day of the month. This ranges from 1 to 31."
field: int tm_mon, "Months since January. This ranges from 0 to 11."
field: int tm_year, "Years since 1900."
field: int tm_wday, "Days since Sunday. This ranges from 0 to 6."
field: int tm_yday, "Days since January 1. This ranges from 0 to 365."
field: int tm_isdst, "Flag to indicate whether daylight saving time is in effect. The value is positive if it is in effect, zero if it is not, and negative if the information is not available."
summary: Date and time structure
description:
 Data structure for broken-down time components.
end:

function: rsGetDt
ret: float, "Time in seconds."
summary: Elapsed time since last call
description:
 Returns the time in seconds since this function was last called in this script.
test: none
end:

function: rsLocaltime
ret: rs_tm*, "Pointer to the output local time, i.e. the same value as the parameter local."
arg: rs_tm* local, "Pointer to time structure where the local time will be stored."
arg: const rs_time_t* timer, "Input time as a number of seconds since January 1, 1970."
summary: Convert to local time
description:
 Converts the time specified by timer into a @rs_tm structure that provides year, month,
 hour, etc.  This value is stored at *local.

 This functions returns the same pointer that is passed as first argument.  If the
 local parameter is NULL, this function does nothing and returns NULL.
test: none
end:

function: rsTime
ret: rs_time_t, "Seconds since the Epoch, -1 if there's an error."
arg: rs_time_t* timer, "Location to also store the returned calendar time."
summary: Seconds since January 1, 1970
description:
 Returns the number of seconds since the Epoch (00:00:00 UTC, January 1, 1970).

 If timer is non-NULL, the result is also stored in the memory pointed to by
 this variable.
test: none
end:

function: rsUptimeMillis
ret: int64_t, "Uptime in milliseconds."
summary: System uptime in milliseconds
description:
 Returns the current system clock (uptime) in milliseconds.
test: none
end:

function: rsUptimeNanos
ret: int64_t, "Uptime in nanoseconds."
summary: System uptime in nanoseconds
description:
 Returns the current system clock (uptime) in nanoseconds.

 The granularity of the values return by this call may be much larger than a nanosecond.
test: none
end:
