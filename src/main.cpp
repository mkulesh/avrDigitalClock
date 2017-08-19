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

#include "DigitalClock.h"
#include <avr/interrupt.h>
#include <avr/cpufunc.h>

// Real time machine
RealTimeClock rtc;
DigitalClock * clockPtr;

int main(void)
{
	System::disableJTAG();
	System::setClockDivisionFactor(System::PRE_1);
	System::setVoltage(3.32);
	
	DigitalClock dc(&rtc);
	clockPtr = &dc;
	dc.init();
	rtc.startClock();
	
    do
    {
		dc.periodic();
    }
	while(1);
    return 0;
}

// overflow interrupt
ISR(TIMER2_OVF_vect)
{
	rtc.onInterrupOverflow();
}

// compare output interrupt
ISR(TIMER1_COMPA_vect)
{
	rtc.onInterrupCompareMatch();
}

// analog comparator interrupt
ISR(ANALOG_COMP_vect)
{
	clockPtr->onComparatorInterrupt();
}
