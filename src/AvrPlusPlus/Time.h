/*******************************************************************************
 * avrDigitalClock - a digital clock based on ATmega644 MCU
 * *****************************************************************************
 * Copyright (C) 2014-2017 Mikhail Kulesh
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef TIME_H_
#define TIME_H_

#include <stdint.h>

namespace AvrPlusPlus
{

/**
 * @brief Time types
 */
typedef uint32_t time_t;
typedef int32_t duration_sec;
typedef uint64_t time_ms;
typedef int64_t duration_ms;

#define INFINITY_SEC __UINT32_MAX__
#define INFINITY_TIME __UINT64_MAX__

/**
 * @brief Time structure
 * Structure containing a calendar date and time broken down into its components.
 *
 * The structure contains nine members of type int (in any order), which are:
 * tm_sec      seconds after the minute - [ 0 to 59 ]
 * tm_min      minutes after the hour - [ 0 to 59 ]
 * tm_hour     hours since midnight - [ 0 to 23 ]
 * tm_mday     day of the month - [ 1 to 31 ]
 * tm_wday     days since Sunday - [ 0 to 6 ]
 * tm_mon      months since January - [ 0 to 11 ]
 * tm_year     years since 2000 - [0 to 136 ]
 * tm_yday     days since January 1 - [ 0 to 365 ]
 * tm_isdst    Daylight Saving Time flag
 */
struct tm
{
    int8_t tm_sec;
    int8_t tm_min;
    int8_t tm_hour;
    int8_t tm_mday;
    int8_t tm_wday;
    int8_t tm_mon;
    int16_t tm_year;
    int16_t tm_yday;
    int16_t tm_isdst;
};

/**
 * @brief Enumerated labels for the days of the week.
 */
enum WEEK_DAYS
{
    SUNDAY, MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY
};

/**
 * @brief Enumerated labels for the months.
 */
enum MONTHS
{
    JANUARY, FEBRUARY, MARCH, APRIL, MAY, JUNE, JULY, AUGUST, SEPTEMBER, OCTOBER, NOVEMBER, DECEMBER
};

/**
 * @brief Convert time_t to tm as UTC time.
 *
 * Uses the value pointed by timer to fill a tm structure with the values that represent the corresponding time,
 * expressed as a UTC time (i.e., the time at the GMT timezone).
 */
void gmtime(time_t timer, struct tm & timeptr);

/**
 * @brief Convert tm structure to time_t
 *
 * Returns the value of type time_t that represents the UTC time described by the tm structure pointed by
 * timeptr (which may be modified).
 *
 * The values of the members tm_wday and tm_yday of timeptr are ignored, and the values of the other
 * members are interpreted even if out of their valid ranges (see struct tm). For example, tm_mday
 * may contain values above 31, which are interpreted accordingly as the days that follow the last
 * day of the selected month.
 *
 * A call to this function automatically adjusts the values of the members of timeptr if they are
 * off-range or -in the case of tm_wday and tm_yday- if they have values that do not match the date
 * described by the other members.
 */
time_t mktime(struct tm & timeptr);

} // end of namespace AvrPlusPlus

#endif
