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

#ifndef PIEZOALARM_H_
#define PIEZOALARM_H_

#include "../AvrPlusPlus.h"

namespace AvrPlusPlus {
namespace Devices {

/** 
 * @brief Class implementing non-blocking alarm sound on piezo element
 */
class PiezoAlarm : public IOPin
{
private:
	enum State
	{
		OFF,
	    ON1,
		PAUSE1,
	    ON2,
	    PAUSE2,
	};
    volatile const RealTimeClock * rtc;
	volatile State state;
	volatile time_ms startTime, stateTime;
	volatile unsigned char maxNumber, number;
	
	static const duration_ms onDuratin = 75L;
	static const duration_ms pause1Duratin = 100L;
	static const duration_ms pause2Duratin = 300L;
public:
	PiezoAlarm (Name name, unsigned char pinNr, const RealTimeClock * _rtc);
	void resetTime ();
	void start (unsigned char _maxNumber);
	void periodic ();
	bool finish ();
};

}
}

#endif
