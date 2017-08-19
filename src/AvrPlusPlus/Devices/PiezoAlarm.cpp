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

#include "PiezoAlarm.h"
#include <util/delay.h>

namespace AvrPlusPlus {
namespace Devices {
	
PiezoAlarm::PiezoAlarm (Name name, unsigned char pinNr, const RealTimeClock * _rtc):
	IOPin(name, pinNr, OUTPUT),
	rtc(_rtc),
	state(OFF),
	startTime(INFINITY_TIME),
	stateTime(INFINITY_TIME),
	number(0)
{
	// empty
}

void PiezoAlarm::resetTime ()
{
	startTime = INFINITY_TIME;
}

void PiezoAlarm::start (unsigned char _maxNumber)
{
	if (state != OFF)
	{
		return;
	}
	if (startTime != INFINITY_TIME && rtc->timeMillisec < startTime + 60000)
	{
		return;
	}
	maxNumber = _maxNumber;
	state = ON1;
	startTime = rtc->timeMillisec;
	stateTime = startTime;
	number = 0;
	setHigh();
}

void PiezoAlarm::periodic ()
{
	switch (state)
	{
		case OFF: return;
		case ON1:
			if (rtc->timeMillisec > (stateTime + onDuratin))
			{
				state = PAUSE1;
				stateTime = rtc->timeMillisec;
				setLow();
			}
			break;
		case PAUSE1:
			if (rtc->timeMillisec > (stateTime + pause1Duratin))
			{
				state = ON2;
				stateTime = rtc->timeMillisec;
				setHigh();
			}
			break;
		case ON2:
			if (rtc->timeMillisec > (stateTime + onDuratin))
			{
				state = PAUSE2;
				stateTime = rtc->timeMillisec;
				setLow();
			}
			break;
		case PAUSE2:
			if (rtc->timeMillisec > (stateTime + pause2Duratin))
			{
				if (++number >= maxNumber)
				{
					finish();
				}
				else
				{
					state = ON1;
					stateTime = rtc->timeMillisec;
					setHigh();
				}
			}
			break;
	}
}

bool PiezoAlarm::finish ()
{
	bool retValue = state != OFF;
	state = OFF;
	number = 0;
	setLow();
	return retValue;
}
	
}
}
