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

#ifndef DAC_MCP4901_H_
#define DAC_MCP4901_H_

#include "../AvrPlusPlus.h"

namespace AvrPlusPlus {
namespace Devices {

class Dac_MCP4901 : public SpiDevice
{
public:
	Dac_MCP4901 (Name spiPortName, unsigned char pinMosiNr, unsigned char pinSckNr, Name devicePortName, unsigned char pinCsNr);
	void putValue (unsigned char percent);
	void inline setOutputGain (bool flag) { outputGain = flag; };
	
private:
	bool outputGain;
};


}
}

#endif
