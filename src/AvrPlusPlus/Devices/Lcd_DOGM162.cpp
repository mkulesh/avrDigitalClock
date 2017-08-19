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

#include "Lcd_DOGM162.h"

#include <util/delay.h>
#include <avr/cpufunc.h>

namespace AvrPlusPlus
{
namespace Devices
{

/************************************************************************
 * Lcd_DOGM162
 ************************************************************************/
Lcd_DOGM162::Lcd_DOGM162(const unsigned char * controlPins, const unsigned char * dataPins) :
        pinE((IOPort::Name) controlPins[0], controlPins[1], IOPort::OUTPUT), 
        pinRW((IOPort::Name) controlPins[2], controlPins[3], IOPort::OUTPUT), 
        pinRS((IOPort::Name) controlPins[4], controlPins[5], IOPort::OUTPUT), 
        pinD4((IOPort::Name) dataPins[0], dataPins[1], IOPort::OUTPUT), 
        pinD5((IOPort::Name) dataPins[2], dataPins[3], IOPort::OUTPUT), 
        pinD6((IOPort::Name) dataPins[4], dataPins[5], IOPort::OUTPUT), 
        pinD7((IOPort::Name) dataPins[6], dataPins[7], IOPort::OUTPUT)
{
    pinRS.setLow();

    // Function Set #1: DL=1
    _delay_ms(200);
    transmitBits(0x30);

    // Function Set #2: DL=1
    _delay_ms(5);
    transmitBits(0x30);

    // Function Set #3: DL=1
    _delay_us(30);
    transmitBits(0x30);

    // Function Set #4: DL=0
    _delay_us(30);
    transmitBits(0x20);

    // Function Set #5: DL=0, N=1, DH=0, IS2=0, IS1=1
    writeData(false, 0x29);

    // Bias Set: BS=0, FX=0
    writeData(false, 0b00011100);

    // Power/ICON/Contrast: Icon=1, Bon=1, C5=1, C4=1
    writeData(false, 0b01011111);

    // Contrast Set: C3=1, C2=C1=C0=0
    writeData(false, 0b01110100);

    // Follower Ctrl: Fon=1, Rab2=0, Rab1=1, Rab0=0
    writeData(false, 0b01101010);

    // DISPLAY ON: D=1, C=0, B=0
    writeData(false, 0x0C);

    // CLEAR DISPLAY
    writeData(false, 0x01);

    // Entry mode set: I/D=1, S=0
    writeData(false, 0x06);
}

void Lcd_DOGM162::gotoXY(char x, char y)
{
    writeData(false, 0x80 | ((y * 0x40) + x));
}

void Lcd_DOGM162::putString(char x, char y, const char * str)
{
    gotoXY(x, y);
    while (*str != '\0')
    {
        putChar(*str++);
    }
}

void Lcd_DOGM162::busyCheck(void)
{
    // Configure LCD data ports as inputs
    pinD4.setDirection(IOPort::INPUT);
    pinD5.setDirection(IOPort::INPUT);
    pinD6.setDirection(IOPort::INPUT);
    pinD7.setDirection(IOPort::INPUT);

    // Set RW high to read busy flag
    pinRW.setHigh();

    // Set RS low to read from control registers
    pinRS.setLow();

    // Loop while LCD is busy
    bool busy;
    do
    {
        // Get upper 4 bits, D7 is busy flag
        pinE.setHigh();
        _NOP();
        busy = pinD7.readInput();
        pinE.setLow();

        // Get lower 4 bits
        pinE.setHigh();
        _NOP();
        pinE.setLow();
    }
    while (busy);

    // Set RW low to write to LCD again
    pinRW.setLow();

    // Configure LCD data ports as outputs
    pinD4.setDirection(IOPort::OUTPUT);
    pinD5.setDirection(IOPort::OUTPUT);
    pinD6.setDirection(IOPort::OUTPUT);
    pinD7.setDirection(IOPort::OUTPUT);
}

void Lcd_DOGM162::transmitBits(char data)
{
    pinRW.setLow();
    pinE.setHigh();
    pinD7.putBit(data & 0x80);
    pinD6.putBit(data & 0x40);
    pinD5.putBit(data & 0x20);
    pinD4.putBit(data & 0x10);
    pinE.setLow();
}

void Lcd_DOGM162::writeData(bool isData, char data)
{
    busyCheck();
    writeDataWithoutCheck(isData, data);
}

void Lcd_DOGM162::writeDataWithoutCheck(bool isData, char data)
{
    pinRS.putBit(isData);
    transmitBits(data);          // write upper 4 bit to LCD
    transmitBits(data << 4);     // write lower 4 bit to LCD
}

/************************************************************************
 * Class Lcd_DOGM162_SPI
 ************************************************************************/
Lcd_DOGM162_SPI::Lcd_DOGM162_SPI(IOPort::Name spiPortName, unsigned char pinMosiNr, unsigned char pinSckNr,
        IOPort::Name devicePortName, unsigned char pinCsNr, unsigned char pinRsNr) :
        SpiDevice(spiPortName, pinMosiNr, pinSckNr, devicePortName, pinCsNr), 
        pinRs(devicePortName, pinRsNr, IOPort::OUTPUT)
{
    _delay_ms(80);

    // Function Set ; 8 Bit; 2Zeilen, Istr.Tab 1
    writeData(false, 0x29);

    // Bias Set: BS=0, FX=0
    writeData(false, 0b00011100);

    // Power/ICON/Contrast: Icon=1, Bon=1, C5=1, C4=1
    writeData(false, 0b01011111);

    // Contrast Set: C3=1, C2=C1=C0=0
    writeData(false, 0b01110100);

    // Follower Ctrl: Fon=1, Rab2=0, Rab1=1, Rab0=0
    writeData(false, 0b01101010);

    // DISPLAY ON: D=1, C=0, B=0
    writeData(false, 0x0C);

    // CLEAR DISPLAY
    writeData(false, 0x01);

    // Entry mode set: I/D=1, S=0
    writeData(false, 0x06);
}

void Lcd_DOGM162_SPI::gotoXY(char x, char y)
{
    writeData(false, 0x80 | ((y * 0x40) + x));
}

void Lcd_DOGM162_SPI::putString(char x, char y, const char * str)
{
    gotoXY(x, y);
    while (*str != '\0')
    {
        putChar(*str++);
    }
}

void Lcd_DOGM162_SPI::writeData(bool isData, char data)
{
    pinRs.putBit(isData);
    startTransfer();
    SpiDevice::putChar(data);
    finishTransfer();
}

} // end of namespace Devices
} // end of namespace AvrPlusPlus
