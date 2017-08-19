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

#ifndef LCD_DOGM162_H_
#define LCD_DOGM162_H_

#include "../AvrPlusPlus.h"

namespace AvrPlusPlus {
namespace Devices {
	
/** 
 * @brief Driver for the DOGM162 LCD series by Electonic Assembly with ST7036 controller.
 *        This driver uses 4 bit connection method.
 */
class Lcd_DOGM162
{
protected:

	IOPin pinE, pinRW, pinRS;
	IOPin pinD4, pinD5, pinD6, pinD7;

	void busyCheck (void);
	void transmitBits (char data);
	void writeData (bool isData, char data);
	void writeDataWithoutCheck (bool isData, char data);

public:

	/** 
	 * @brief Default constructor: Initialize LCD module
	 */ 
    Lcd_DOGM162 (const unsigned char * controlPins, const unsigned char * dataPins);
	
	/** 
	 * @brief Clear display, go to first char in first line
	 */ 
	inline void clear (void) { writeData(false, 0x01); };

	/** 
	 * @brief Go to position
	 */ 
	void gotoXY (char x, char y);
	
	/** 
	 * @brief Write single character to display
	 */ 
	inline void putChar (char c) { writeData(true, c); };
	
	/** 
	 * @brief Write string to display
	 */ 
	void putString (char x, char y, const char *);
};


/** 
 * @brief Driver for the DOGM162 LCD series by Electonic Assembly with ST7036 controller.
 *        This driver uses SPI connection method.
 */
class Lcd_DOGM162_SPI : public SpiDevice
{
private:
	
	IOPin pinRs;
	void writeData (bool isData, char data);
	
public:
	
	/** 
	 * @brief Default constructor: Initialize LCD module
	 */ 
	Lcd_DOGM162_SPI(IOPort::Name spiPortName, unsigned char pinMosiNr, unsigned char pinSckNr, IOPort::Name devicePortName, unsigned char pinCsNr, unsigned char pinRsNr);
	
	/** 
	 * @brief Clear display, go to first char in first line
	 */ 
	inline void clear (void) { writeData(false, 0x01); };

	/** 
	 * @brief Go to position
	 */ 
	void gotoXY (char x, char y);
	
	/** 
	 * @brief Write single character to display
	 */ 
	inline void putChar (char c) { writeData(true, c); };
	
	/** 
	 * @brief Write string to display
	 */ 
	void putString (char x, char y, const char *);
};

} // end of namespace Devices
} // end of namespace AvrPlusPlus

#endif
