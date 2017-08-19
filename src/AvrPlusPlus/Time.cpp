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

#include "Time.h"

#include <stdlib.h>
#include <stdio.h>

namespace AvrPlusPlus
{
	
unsigned char is_leap_year (int year)
{
	/* year must be divisible by 4 to be a leap year */
	if (year & 3)
	{
		return 0;
	}

	/* If theres a remainder after division by 100, year is not divisible by 100 or 400 */
	div_t d = div(year, 100);
	if (d.rem)
	{
		return 1;
	}

	/* If the quotient is divisible by 4, then year is divisible by 400 */
	if ((d.quot & 3) == 0)
	{
		return 1;
	}
	return 0;
}


void gmtime (time_t timer, struct tm & timeptr)
{
    div_t result;

    /* break down timer into whole and fractional parts of 1 day */
    uint16_t days = timer / 86400UL;
    int32_t fract = timer % 86400UL;

    /*
     Extract hour, minute, and second from the fractional day
     */
    ldiv_t lresult = ldiv(fract, 60L);
    timeptr.tm_sec = lresult.rem;
    result = div((int) lresult.quot, 60);
    timeptr.tm_min = result.rem;
    timeptr.tm_hour = result.quot;

    /* Determine day of week ( the epoch was a Saturday ) */
    uint16_t n = days + SATURDAY;
    n %= 7;
    timeptr.tm_wday = n;

    /*
     * Our epoch year has the property of being at the conjunction of all three 'leap cycles',
     * 4, 100, and 400 years ( though we can ignore the 400 year cycle in this library).
     *
     * Using this property, we can easily 'map' the time stamp into the leap cycles, quickly
     * deriving the year and day of year, along with the fact of whether it is a leap year.
     */

    /* map into a 100 year cycle */
    lresult = ldiv((long) days, 36525L);
    uint16_t years = 100 * lresult.quot;

    /* map into a 4 year cycle */
    lresult = ldiv(lresult.rem, 1461L);
    years += 4 * lresult.quot;
    days = lresult.rem;
    if (years > 100)
    {
        days++;
    }

    /*
     * 'years' is now at the first year of a 4 year leap cycle, which will always be a leap year,
     * unless it is 100. 'days' is now an index into that cycle.
     */
    uint16_t leapyear = 1;
    if (years == 100)
    {
        leapyear = 0;
    }

    /* compute length, in days, of first year of this cycle */
    n = 364 + leapyear;

    /*
     * if the number of days remaining is greater than the length of the
     * first year, we make one more division.
     */
    if (days > n)
    {
        days -= leapyear;
        leapyear = 0;
        result = div(days, 365);
        years += result.quot;
        days = result.rem;
    }
    timeptr.tm_year = years;
    timeptr.tm_yday = days;

    /*
     Given the year, day of year, and leap year indicator, we can break down the
     month and day of month. If the day of year is less than 59 (or 60 if a leap year), then
     we handle the Jan/Feb month pair as an exception.
     */
    n = 59 + leapyear;
    if (days < n)
    {
        /* special case: Jan/Feb month pair */
        result = div(days, 31);
        timeptr.tm_mon = result.quot;
        timeptr.tm_mday = result.rem;
    }
    else
    {
        /*
         The remaining 10 months form a regular pattern of 31 day months alternating with 30 day
         months, with a 'phase change' between July and August (153 days after March 1).
         We proceed by mapping our position into either March-July or August-December.
         */
        days -= n;
        result = div(days, 153);
        timeptr.tm_mon = 2 + result.quot * 5;

        /* map into a 61 day pair of months */
        result = div(result.rem, 61);
        timeptr.tm_mon += result.quot * 2;

        /* map into a month */
        result = div(result.rem, 31);
        timeptr.tm_mon += result.quot;
        timeptr.tm_mday = result.rem;
    }

    /*
     Cleanup and return
     */
    timeptr.tm_isdst = 0; /* gmt is never in DST */
    timeptr.tm_mday++; /* tm_mday is 1 based */
}


time_t mktime (struct tm & timeptr)
{
    time_t ret;

    /*
     Determine elapsed whole days since the epoch to the beginning of this year. Since our epoch is
     at a conjunction of the leap cycles, we can do this rather quickly.
     */
    int n = timeptr.tm_year;
    int leaps = 0;
    if (n)
    {
        int m = n - 1;
        leaps = m / 4;
        leaps -= m / 100;
        leaps++;
    }
    uint32_t tmp = 365UL * n + leaps;

    /*
     Derive the day of year from month and day of month. We use the pattern of 31 day months
     followed by 30 day months to our advantage, but we must 'special case' Jan/Feb, and
     account for a 'phase change' between July and August (153 days after March 1).
     */
    int d = timeptr.tm_mday - 1; /* tm_mday is one based */

    /* handle Jan/Feb as a special case */
    if (timeptr.tm_mon < 2)
    {
        if (timeptr.tm_mon)
        {
            d += 31;
        }
    }
    else
    {
        n = 59 + is_leap_year(timeptr.tm_year + 2000);
        d += n;
        n = timeptr.tm_mon - MARCH;

        /* account for phase change */
        if (n > (JULY - MARCH))
        {
            d += 153;
        }
        n %= 5;

        /*
         * n is now an index into a group of alternating 31 and 30
         * day months... 61 day pairs.
         */
        int m = n / 2;
        m *= 61;
        d += m;

        /*
         * if n is odd, we are in the second half of the
         * month pair
         */
        if (n & 1)
        {
            d += 31;
        }
    }

    /* Add day of year to elapsed days, and convert to seconds */
    tmp += d;
    tmp *= 86400;
    ret = tmp;

    /* compute 'fractional' day */
    tmp = timeptr.tm_hour;
    tmp *= 3600;
    tmp += timeptr.tm_min * 60UL;
    tmp += timeptr.tm_sec;
    ret += tmp;

    gmtime(ret, timeptr);
    return ret;
}


}
