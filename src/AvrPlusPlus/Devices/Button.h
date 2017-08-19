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

#ifndef BUTTON_H_
#define BUTTON_H_

#include "../AvrPlusPlus.h"

namespace AvrPlusPlus
{
namespace Devices
{

/** 
 * @brief Class describing a button connected to a pin
 */
class Button: IOPin
{
private:

    volatile const RealTimeClock * rtc;
    volatile time_ms pressDelay, longPressDelay, pressTime;
    volatile long occurred;

public:

    Button(Name name, unsigned char pinNr, const RealTimeClock * _rtc, time_ms _pressDelay = 50,
            time_ms _longPressDelay = 1500);
    void resetTime();
    bool isPressed();
    bool isLongPressed();
    inline void setProcessed()
    {
        ++occurred;
    };
};

} // end of namespace Devices
} // end of namespace AvrPlusPlus

#endif
