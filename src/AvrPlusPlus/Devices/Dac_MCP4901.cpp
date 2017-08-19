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

#include "Dac_MCP4901.h"

namespace AvrPlusPlus
{
namespace Devices
{

Dac_MCP4901::Dac_MCP4901(Name spiPortName, unsigned char pinMosiNr, unsigned char pinSckNr, Name devicePortName, unsigned char pinCsNr) :
        SpiDevice(spiPortName, pinMosiNr, pinSckNr, devicePortName, pinCsNr), 
        outputGain(false)
{
    // empty
}

void Dac_MCP4901::putValue(unsigned char percent)
{
    unsigned int packet = 0;
    if (percent == 0)
    {
        packet |= 0 << 12;          //De-active mode operation
    }
    else
    {
        unsigned char volt_digits = ((int) 0xFF * (int) percent) / 100;
        packet = volt_digits << 4;  //shift voltage setting digits
        packet |= 1 << 12;          //Active mode operation
        packet |= !outputGain << 13; //Set output gain
    }
    startTransfer();
    putInt(packet);
    finishTransfer();
}

}
}
