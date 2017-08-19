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

#include "Dcf77.h"

#include <stdio.h>

namespace AvrPlusPlus {
namespace Devices {

Dcf77::Dcf77 (volatile RealTimeClock * aClock, IOPort::Name pinAin0Name, unsigned char pinAin0Nr, IOPort::Name pinAin1Name, unsigned char pinAin1Nr):
	AnalogComparator(pinAin0Name, pinAin0Nr, pinAin1Name, pinAin1Nr),
	clock(aClock),
	currBit(-1),
	streaming(false),
	lastInterruptTime(clock->timeMillisec)
{
	// empty
}

void Dcf77::setHandler(Dcf77Handler * aHandler)
{
	handler = aHandler;
}

void Dcf77::onInterrupt ()
{
	unsigned int dur = (unsigned int)(clock->timeMillisec - lastInterruptTime);
	lastInterruptTime = clock->timeMillisec;
	int val = getValue();
	// Duration pattern
	static const unsigned int ZERO_START = 50;
	static const unsigned int ZERO_END = 149;
	static const unsigned int ONE_START = 150;
	static const unsigned int ONE_END = 300;
	static const unsigned int DATA_END = 1500;
	// Decode duration into bits array
	if (dur > DATA_END)
	{
		// data set end
		if (streaming)
		{
			if (currBit == BITS_NUMBER - 1)
			{
				sprintf(outString, "Received valid bits set -> decode time\n");
				handler->onDcfLog(outString);
				decodeTime();
			}
			else
			{
				sprintf(outString, "Error: invalid bits number = %d\n", currBit);
				handler->onDcfLog(outString);
			}
			currBit = -1;
			streaming = false;
		}
		else
		{
			sprintf(outString, "Start receiving\n");
			handler->onDcfLog(outString);
			currBit = 0;
			streaming = true;
		}
	}
	else if (val == 0 && streaming)
	{
		if (dur >= ZERO_START && dur <= ZERO_END)
		{
			if (currBit < BITS_NUMBER)
			{
				handler->onBitReceived();
				bits[currBit] = 0;
				currBit++;
			}
		}
		else if (dur >= ONE_START && dur <= ONE_END)
		{
			if (currBit < BITS_NUMBER)
			{
				handler->onBitReceived();
				bits[currBit] = 1;
				currBit++;
			}
		}
		else if (dur > 30)
		{
			// error: wait new data set
			currBit = -1;										 
			streaming = false;
			sprintf(outString, "Error: invalid duration = %d\n", dur);
			handler->onBitFailed();
			handler->onDcfLog(outString);
		}
	}
	else if (val == 0)
	{
		if ((dur >= ZERO_START && dur <= ZERO_END) || (dur >= ONE_START && dur <= ONE_END))
		{
			handler->onBitFailed();
			handler->onBitReceived();
		}
		else if (dur > 30)
		{
			handler->onBitFailed();
		}
		sprintf(outString, ">> %d\n", dur);
		handler->onDcfLog(outString);
	}
}
	
void Dcf77::turnOn ()
{
	currBit = -1;
	streaming = false;
	AnalogComparator::turnOn();
}
	
bool Dcf77::decodeTime ()
{
	// see https://de.wikipedia.org/wiki/DCF77
	// Header [0-19]            |T|Min [21]|H[29]  |Data[36-58]
	// 0|Reserved[1-14]|A|C|Z |C|1|       C|      C|
	// 0|01011110001010|0|0|10|0|1|00011011|0100010|10100100101100101010001
	// 0|00101110101110|0|0|10|0|1|10000001|1100011|10100100101100101010001
	// 0|10101010010111|0|0|10|0|1|01000001|1100011|10100100101100101010001
	// 0|00000010001010|0|0|10|0|1|10100001|1100011|10100100101100101010001
	
	// minute
	bool valid = true;
	int min = -1;
	{
		int sum = bits[21] + bits[22] + bits[23] + bits[24] + bits[25] + bits[26] + bits[27];
		int checkBit = bits[28];
		if (sum % 2 == checkBit)
		{
			min = bits[21] + 2*bits[22] + 4*bits[23] + 8*bits[24] + 10*bits[25] + 20*bits[26] + 40*bits[27];
		}
		else
		{
			sprintf(outString, "Error: invalid check bit for minutes: sum = %d, check bit = %d\n", sum, checkBit);
			handler->onDcfLog(outString);
			valid = false;
		}
	}
		
	// hour
	int hour = -1;
	{
		int sum = bits[29] + bits[30] + bits[31] + bits[32] + bits[33] + bits[34];
		int checkBit = bits[35];
		if (sum % 2 == checkBit)
		{
			hour = bits[29] + 2*bits[30] + 4*bits[31] + 8*bits[32] + 10*bits[33] + 20*bits[34];
		}
		else
		{
			sprintf(outString, "Error: invalid check bit for hour: sum = %d, check bit = %d\n", sum, checkBit);
			handler->onDcfLog(outString);
			valid = false;
		}
	}

	// date
	int day = -1, month = -1, year = -1;
	{
		int sum = 0;
		for (int i = 36; i < 57; ++i)
		{
			sum += bits[i];
		}
		int checkBit = bits[58];
		if (sum % 2 == checkBit)
		{
			day = bits[36] + 2*bits[37] + 4*bits[38] + 8*bits[39] + 10*bits[40] + 20*bits[41];
			month = bits[45] + 2*bits[46] + 4*bits[47] + 8*bits[48] + 10*bits[49];
			year = bits[50] + 2*bits[51] + 4*bits[52] + 8*bits[53] + 10*bits[54] + 20*bits[55] + 40*bits[56] + 80*bits[57];
		}
		else
		{
			sprintf(outString, "Error: invalid check bit for date: sum = %d, check bit = %d\n", sum, checkBit);
			handler->onDcfLog(outString);
			valid = false;
		}
	}
		
	sprintf(outString, "Date and time: %02d.%02d.%04d %02d:%02d\n", day, month, year, hour, min);
	handler->onDcfLog(outString);
	if (valid)
	{
		dayTime.tm_sec = 0;
		dayTime.tm_min = min;
		dayTime.tm_hour = hour;
		dayTime.tm_mday = day;
		dayTime.tm_mon = month - 1;
		dayTime.tm_year = year;
		handler->onTimeReceived(dayTime.tm_min, dayTime.tm_hour, dayTime.tm_mday, dayTime.tm_mon, dayTime.tm_year);
	}
	return valid;
}

}
}
