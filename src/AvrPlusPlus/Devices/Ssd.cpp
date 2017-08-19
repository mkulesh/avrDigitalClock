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

#include "Ssd.h"
#include <avr/interrupt.h>

namespace AvrPlusPlus
{
namespace Devices
{

/************************************************************************
 * Class Ssd
 ************************************************************************/
Ssd::SegmentsMask::SegmentsMask()
{
    top = 0;
    rightTop = 1;
    rightBottom = 2;
    bottom = 3;
    leftBottom = 4;
    leftTop = 5;
    center = 6;
    dot = 7;
}

char Ssd::getBits(char c, bool dot /*= false*/) const
{
    char bits = 0;
    switch (c)
    {
    case '.':
        bits |= (1 << sm.dot);
        break;
    case '-':
        bits |= (1 << sm.center);
        break;
    case 0:
    case '0':
        bits = (1 << sm.top) | (1 << sm.rightTop) | (1 << sm.rightBottom) | (1 << sm.bottom) | (1 << sm.leftBottom)
                | (1 << sm.leftTop);
        break;
    case 1:
    case '1':
        bits = (1 << sm.rightTop) | (1 << sm.rightBottom);
        break;
    case 2:
    case '2':
        bits = (1 << sm.top) | (1 << sm.rightTop) | (1 << sm.center) | (1 << sm.leftBottom) | (1 << sm.bottom);
        break;
    case 3:
    case '3':
        bits = (1 << sm.top) | (1 << sm.rightTop) | (1 << sm.center) | (1 << sm.rightBottom) | (1 << sm.bottom);
        break;
    case 4:
    case '4':
        bits = (1 << sm.leftTop) | (1 << sm.center) | (1 << sm.rightTop) | (1 << sm.rightBottom);
        break;
    case 5:
    case '5':
        bits = (1 << sm.top) | (1 << sm.leftTop) | (1 << sm.center) | (1 << sm.rightBottom) | (1 << sm.bottom);
        break;
    case 6:
    case '6':
        bits = (1 << sm.top) | (1 << sm.leftTop) | (1 << sm.center) | (1 << sm.rightBottom) | (1 << sm.bottom)
                | (1 << sm.leftBottom);
        break;
    case 7:
    case '7':
        bits = (1 << sm.top) | (1 << sm.rightTop) | (1 << sm.rightBottom);
        break;
    case 8:
    case '8':
        bits = (1 << sm.top) | (1 << sm.rightTop) | (1 << sm.center) | (1 << sm.rightBottom) | (1 << sm.bottom)
                | (1 << sm.leftBottom) | (1 << sm.leftTop);
        break;
    case 9:
    case '9':
        bits = (1 << sm.top) | (1 << sm.rightTop) | (1 << sm.center) | (1 << sm.rightBottom) | (1 << sm.bottom)
                | (1 << sm.leftTop);
        break;
    default:
        break;
    }
    if (dot)
    {
        bits |= (1 << sm.dot);
    }
    return bits;
}

/************************************************************************
 * Class SsdOn74HC595
 ************************************************************************/
Ssd_74HC595_3bit::Ssd_74HC595_3bit(IOPort::Name name, const unsigned char * controlPins) :
        pinSer(name, controlPins[0], IOPort::OUTPUT), 
        pinSck(name, controlPins[1], IOPort::OUTPUT), 
        pinRck(name, controlPins[2], IOPort::OUTPUT)
{
    pinRck.setHigh();
}

void Ssd_74HC595_3bit::putChar(char bits)
{
    for (int i = 7; i >= 0; --i)
    {
        pinSer.putBit(bits & (1 << i));
        pinSck.setLow();
        pinSck.setHigh();
        pinSck.setLow();
        pinSer.setLow();
    }
}

void Ssd_74HC595_3bit::putString(const char * str, int segNumbers, bool dot /*= false*/)
{
    if (segNumbers >= maxSegments)
    {
        return;
    }
    for (int i = 0; i < segNumbers; ++i)
    {
        segData[i] = getBits(str[i], false);
    }

    cli();
    pinRck.setLow();
    for (int i = segNumbers - 1; i >= 0; --i)
    {
        putChar(segData[i]);
    }
    pinRck.setHigh();
    sei();
}

/************************************************************************
 * Class SsdOnSpi
 ************************************************************************/
Ssd_74HC595_SPI::Ssd_74HC595_SPI(IOPort::Name spiPortName, unsigned char pinMosiNr, unsigned char pinSckNr,
        IOPort::Name devicePortName, unsigned char pinCsNr) :
        spi(spiPortName, pinMosiNr, pinSckNr, devicePortName, pinCsNr)
{
    // empty
}

void Ssd_74HC595_SPI::putString(const char * str, int segNumbers, bool dot /*= false*/)
{
    if (segNumbers >= maxSegments)
    {
        return;
    }
    for (int i = 0; i < segNumbers; ++i)
    {
        segData[i] = getBits(str[i], false);
    }
    spi.startTransfer();
    for (int i = segNumbers - 1; i >= 0; --i)
    {
        spi.putChar(segData[i]);
    }
    spi.finishTransfer();
}

}
}
