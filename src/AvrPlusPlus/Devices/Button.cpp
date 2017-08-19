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

#include "Button.h"

namespace AvrPlusPlus {
namespace Devices {

Button::Button (Name name, unsigned char pinNr, const RealTimeClock * _rtc, time_ms _pressDelay/* = 30*/, time_ms _longPressDelay/* = 1000*/):
	IOPin(name, pinNr, INPUT),
	rtc(_rtc),
	pressDelay(_pressDelay),
	longPressDelay(_longPressDelay),
	pressTime(INFINITY_TIME),
	occurred(0)
{
	setPullUp(true);
}

void Button::resetTime ()
{
	pressTime = rtc->timeMillisec;
};

bool Button::isPressed ()
{
	bool val = !readInput();
	if (!val)
	{
		pressTime = INFINITY_TIME;
		occurred = 0;
		return false;
	}
	if (occurred > 0)
	{
		return false;
	}
	if (pressTime == INFINITY_TIME)
	{
		pressTime = rtc->timeMillisec;
	}
	return (rtc->timeMillisec >= pressTime && rtc->timeMillisec - pressTime >= pressDelay);
}

bool Button::isLongPressed ()
{
	if (pressTime == INFINITY_TIME)
	{
		return false;
	}
	if (occurred < 2)
	{
		return (rtc->timeMillisec > pressTime && rtc->timeMillisec - pressTime >= longPressDelay);
	}
	return (rtc->timeMillisec > pressTime && rtc->timeMillisec - pressTime >= longPressDelay/6);
}
	
}
}
