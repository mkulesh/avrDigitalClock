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

#ifndef LED_H_
#define LED_H_

#include "../AvrPlusPlus.h"

namespace AvrPlusPlus
{
namespace Devices
{

/** 
 * @brief Class describing a LED element connecting to a pin
 */
class Led: public IOPin
{
public:

    enum ConnectionType
    {
        ANODE = 0, CATHODE = 1
    };

protected:

    volatile ConnectionType connectionType;

public:

    Led(Name name, unsigned char pinNr, ConnectionType _connectionType, bool isTurned = false);
    void turnOn();
    void turnOff();
    bool isTurned() const;
    void toggle();
    void pulse(unsigned int count, unsigned int msOn, unsigned int msOff);
};

} // end of namespace Devices
} // end of namespace AvrPlusPlus

#endif
