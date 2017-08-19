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

#include "Led.h"

#include <util/delay.h>
#include <avr/cpufunc.h>

namespace AvrPlusPlus
{
namespace Devices
{

Led::Led(Name name, unsigned char pinNr, ConnectionType _connectionType, bool isTurned /*= false*/) :
        IOPin(name, pinNr, OUTPUT), 
        connectionType(_connectionType)
{
    if (isTurned)
    {
        turnOn();
    }
}

void Led::turnOn()
{
    (connectionType == Led::ANODE) ? setHigh() : setLow();
}

void Led::turnOff()
{
    (connectionType == Led::ANODE) ? setLow() : setHigh();
}

bool Led::isTurned() const
{
    return (connectionType == Led::ANODE) ? getBit() : !getBit();
}

void Led::toggle()
{
    if (isTurned())
    {
        turnOff();
    }
    else
    {
        turnOn();
    }
}

void Led::pulse(unsigned int count, unsigned int msOn, unsigned int msOff)
{
    unsigned d;
    for (unsigned int i = 0; i < count; ++i)
    {
        turnOn();
        d = msOn;
        while (d--)
        {
            _delay_us(1000);  // one millisecond
        }
        turnOff();
        d = msOff;
        while (d--)
        {
            _delay_us(1000);  // one millisecond
        }
    }
}

} // end of namespace Devices
} // end of namespace AvrPlusPlus
