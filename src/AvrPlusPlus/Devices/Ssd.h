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

#ifndef SSD_H_
#define SSD_H_

#include "../AvrPlusPlus.h"

namespace AvrPlusPlus {
namespace Devices {

/** 
 * @brief Class that describes a seven segment digit
 */
class Ssd
{
public:

	class SegmentsMask
	{
	public:
		unsigned char top;
		unsigned char rightTop;
		unsigned char rightBottom;
		unsigned char bottom;
		unsigned char leftBottom;
		unsigned char leftTop;
		unsigned char center;
		unsigned char dot;
		SegmentsMask ();
	};
	
protected:

	SegmentsMask sm;
	
public:
	
	Ssd (): sm()
	{
		// empty
	};

	inline void setSegmentsMask (SegmentsMask sm)
	{
		this->sm = sm;
	};
	
	char getBits (char c, bool dot = false) const;
};


/** 
 * @brief Class that describes a seven segment display connected to a port (8 bit)
 */
class Ssd_8bit : public Ssd
{
protected:

	IOPort port;
	
public:

	Ssd_8bit (IOPort::Name name): port(name)
	{
		port.setDirection(IOPort::OUTPUT);
	};

	inline void putChar (signed char c, bool dot = false)
	{
		port.putChar(getBits(c, dot));
	};
};


/** 
 * @brief Class that describes a seven segment display connected to a shift register IC.
 *        The shift register IC is connected to three given pins of a given port.
 */
class Ssd_74HC595_3bit : public Ssd
{
protected:

    static const int maxSegments = 5;
	volatile char segData [maxSegments];
	
	IOPin pinSer, pinSck, pinRck;
	void putChar (char bits);
	
public:

	Ssd_74HC595_3bit (IOPort::Name name, const unsigned char * controlPins);
	void putString (const char * str, int segNumbers, bool dot = false);
};

/** 
 * @brief Class that describes a seven segment display connected to a shift register IC.
 *        The shift register IC is connected via the SPI interface.
 */
class Ssd_74HC595_SPI : public Ssd
{
protected:

    static const int maxSegments = 5;
    volatile char segData [maxSegments];
	SpiDevice spi;
		
public:

	Ssd_74HC595_SPI (IOPort::Name spiPortName, unsigned char pinMosiNr, unsigned char pinSckNr, IOPort::Name devicePortName, unsigned char pinCsNr);
	void putString (const char * str, int segNumbers, bool dot = false);
};


} // end of namespace Devices
} // end of namespace AvrPlusPlus

#endif
