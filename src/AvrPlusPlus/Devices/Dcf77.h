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

#ifndef DCF77_H_
#define DCF77_H_

#include "../AvrPlusPlus.h"

namespace AvrPlusPlus {
namespace Devices {


class Dcf77Handler
{
public:
	virtual void onDcfLog (const char * str) = 0;
	virtual void onTimeReceived (int min, int hour, int day, int month, int year) = 0;
	virtual void onBitReceived () = 0;
	virtual void onBitFailed () = 0;
};


class Dcf77 : public AnalogComparator
{
private:	

	static const unsigned char BITS_NUMBER = 60;

	Dcf77Handler * handler;
	volatile RealTimeClock * clock;
	volatile int currBit;
	volatile bool streaming;
	volatile time_ms lastInterruptTime;
	tm dayTime;
	
	unsigned char bits[BITS_NUMBER];
	char outString[60];
	
	
public:

	Dcf77 (volatile RealTimeClock * aClock, IOPort::Name pinAin0Name, unsigned char pinAin0Nr, IOPort::Name pinAin1Name, unsigned char pinAin1Nr);
	void setHandler(Dcf77Handler * aHandler);
	inline tm & getDayTime () { return dayTime; };
	inline bool isStreaming() { return streaming; };
	virtual void onInterrupt ();
	virtual void turnOn ();
	
private:

	bool decodeTime ();
};


} // end of namespace Devices
} // end of namespace AvrPlusPlus

#endif
