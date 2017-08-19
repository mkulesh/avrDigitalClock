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

#include "AvrPlusPlus.h"

#include <stdlib.h>
#include <avr/interrupt.h>

/************************************************************************
 * Common methods
 ************************************************************************/
void __cxa_pure_virtual(void)
{
    // We might want to write some diagnostics to uart in this case
    exit(0);
}

void __cxa_deleted_virtual(void)
{
    // We might want to write some diagnostics to uart in this case
    exit(0);
}

namespace AvrPlusPlus
{

/************************************************************************
 * Class System
 ************************************************************************/
float System::voltage = 0.0;

void System::setClockDivisionFactor(System::Prescale prescale)
{
    CLKPR = (1 << CLKPCE); // enable a change to CLKPR
    CLKPR = prescale; // set the necessary factor
}

void System::disableJTAG()
{
#ifdef JTD
    unsigned char tmp = (1 << JTD);
    MCUCR = tmp; // Disable JTAG
    MCUCR = tmp;// Disable JTAG
#endif
}

/************************************************************************
 * Class IOPort
 ************************************************************************/
IOPort::IOPort(Name name)
{
    bool initialized = false;
    switch (name)
    {
    case A:
#ifdef PORTA
        port = &PORTA; pins = &PINA; ddr = &DDRA; initialized = true;
#endif
        break;

    case B:
#ifdef PORTB
        port = &PORTB; pins = &PINB; ddr = &DDRB; initialized = true;
#endif
        break;

    case C:
#ifdef PORTC
        port = &PORTC; pins = &PINC; ddr = &DDRC; initialized = true;
#endif
        break;

    case D:
#ifdef PORTD
        port = &PORTD; pins = &PIND; ddr = &DDRD; initialized = true;
#endif
        break;
    }
    if (!initialized)
    {
        abort();
    }
}

void IOPort::setDirection(Direction direction)
{
    *ddr = (direction == INPUT) ? 0xFF : 0;
}

void IOPort::putChar(unsigned char value)
{
    *port = value;
}

/************************************************************************
 * Class IOPin
 ************************************************************************/
IOPin::IOPin(Name name, unsigned char pinNr, Direction direction) : IOPort(name)
{
    pin = pinNr;
    setDirection(direction);
}

void IOPin::setDirection(Direction direction)
{
    if (direction == INPUT)
    {
        *ddr &= ~(1 << pin);
    }
    else
    {
        *ddr |= (1 << pin);
    }
}

void IOPin::setHigh()
{
    *port |= (1 << pin);
}

void IOPin::setLow()
{
    *port &= ~(1 << pin);
}

void IOPin::putBit(bool value)
{
    if (value)
    {
        *port |= (1 << pin);
    }
    else
    {
        *port &= ~(1 << pin);
    }
}

bool IOPin::getBit() const
{
    return *port & (1 << pin);
}

bool IOPin::readInput() const
{
    return *pins & (1 << pin);
}

/************************************************************************
 * Class RealTimeClock
 ************************************************************************/
RealTimeClock::RealTimeClock() :
        syncMs1(0), syncMs2(0), errorMs(0), timeMillisec(0), timeSec(0)
{
    // empty
}

void RealTimeClock::setupTimer1()
{
    // 16-bit timer T1 is clocked from MCU generator. Comparator interrupts it every milliseconds.
    // Disable the Timer/Counter1 interrupts
    TIMSK1 = 0;
    // TCCR1A: Timer/Counter1 Control Register A:
    // Normal port operation, OCnA/OCnB disconnected: COM1A1/COM1B1 COM1A0/COM1B0 zero,
    // Timer/Counter Mode of Operation normal: WGM11, WGM10 zero
    TCCR1A = (0 << COM1A1) | (0 << COM1B1) | (0 << COM1A0) | (0 << COM1B0) | (0 << WGM11) | (0 << WGM10);
    // TCCR1B: Timer/Counter1 Control Register B:
    // Waveform Generation: WGM13 zero, WGM12 = 1: Clear Timer on Compare match (CTC)
    // Clock Select: clkI/O/1 (No prescaling): CS10 = 1;
    TCCR1B = (0 << WGM13) | (1 << WGM12) | (0 << CS12) | (0 << CS11) | (1 << CS10);
    // TCCR1C: Timer/Counter1 Control Register C:
    // No Force Output Compare:
    TCCR1C = (0 << FOC1A) | (0 << FOC1B);
    // Set initial value
    TCNT1 = 0;
    // Output Compare Register 1A: 1000 (1 ms) depending on current pre-scale
    switch (CLKPR | (0 << CLKPCE))
    {
    case System::PRE_1:
        OCR1A = F_CRYSTAL * 1000;
        break;
    case System::PRE_2:
        OCR1A = F_CRYSTAL * 500;
        break;
    case System::PRE_4:
        OCR1A = F_CRYSTAL * 250;
        break;
    case System::PRE_8:
        OCR1A = F_CRYSTAL * 125;
        break;
    }
    // Clear the Timer Interrupt Flags
    TIFR1 = (1 << OCF1A);
    // TIMSK1: Timer/Counter1 Interrupt Mask Register
    // Enable Output Compare A Match Interrupt: OCIE1A = 1
    TIMSK1 = (1 << OCIE1A);
}

void RealTimeClock::setupTimer2()
{
    // 8-bit timer T2 is used asynchronously from 32kHz quartz with overflow interrupt.
    // Disable the Timer/Counter2 interrupts
    TIMSK2 = 0;
    // Select clock source by setting AS2 as appropriate
    ASSR = (1 << AS2);
    // Set initial value
    TCNT2 = 0;
    // TCCR2B: Timer/Counter2 Control Register B:
    // Set prescaller 128: interrupt every second
    TCCR2B |= (1 << CS22) | (1 << CS20);
    // Wait for TCN2UB, OCR2xUB, and TCR2xUB
    while (ASSR & ((1 << TCN2UB) | (1 << TCR2BUB)));
    // Clear the Timer Interrupt Flags
    TIFR2 = (1 << TOV2);
    // Enable interrupts
    TIMSK2 = (1 << TOIE2);
}

void RealTimeClock::startClock()
{
    setupTimer1();
    setupTimer2();
    sei();
};

void RealTimeClock::onInterrupCompareMatch()
{
    if (syncMs1 < 999)
    {
        ++timeMillisec;
        ++syncMs1;
    }
    ++syncMs2;
}

void RealTimeClock::onInterrupOverflow()
{
    ++syncMs2;
    errorMs = syncMs2 - 1000;
    ++timeSec;
    timeMillisec = (time_ms) timeSec * 1000L;
    syncMs1 = syncMs2 = 0;
};

void RealTimeClock::setTime(time_t sec)
{
    cli();
    timeSec = sec;
    timeMillisec = (duration_ms) sec * 1000L;
    sei();
}

/************************************************************************
 * Class SpiDevice
 ************************************************************************/
SpiDevice::SpiDevice(Name spiPortName, unsigned char pinMosiNr, unsigned char pinSckNr, Name devicePortName, unsigned char pinCsNr) :
        IOPin(devicePortName, pinCsNr, OUTPUT),
        pinMosi(spiPortName, pinMosiNr, OUTPUT),
        pinSck(spiPortName, pinSckNr, OUTPUT)
{
    setHigh();
    SPCR |= (1 << SPE) | (1 << MSTR);
}

void SpiDevice::putChar(char data)
{
    // Start transmission
    SPDR = data;
    // Wait for transmission complete
    while (!(SPSR & (1 << SPIF)));
}

void SpiDevice::putInt(int data)
{
    putChar(highByte(data));
    putChar(lowByte(data));
}

/************************************************************************
 * Class Usart
 ************************************************************************/
Usart::Usart(unsigned long baudrate)
{
    // Set baud rate
    UBRR0L = (F_CPU / baudrate / 8 - 1);
    UBRR0H = (F_CPU / baudrate / 8 - 1) >> 8;
    // Double the USART Transmission Speed
    UCSR0A = (1 << U2X0);
    // Transmitter Enable
    UCSR0B = (1 << TXEN0);
    // Stop Bit Select: 1; Character Size: 8
    UCSR0C = (1 << USBS0) | (1 << UCSZ01) | (1 << UCSZ00);
}

void Usart::putChar(char data)
{
    // Wait for empty transmit buffer
    while (!(UCSR0A & (1 << UDRE0)));
    // Put data into buffer, sends the data
    UDR0 = data;
}

void Usart::putString(const char * str)
{
    while (*str != '\0')
    {
        putChar(*str++);
    }
}

/************************************************************************
 * Class AnalogToDigitConverter
 ************************************************************************/
void AnalogToDigitConverter::init(float _vRef, Division division)
{
    vRef = _vRef;
    // REFSn: AREF, Internal Vref turned off
    // ADLAR: AVCC
    ADMUX = (0 << REFS0) | (0 << REFS1) | (0 << ADLAR) | (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0);
    // ADEN: ADC Enable
    // ADATE: ADC Auto Trigger Disable
    unsigned char div = ((unsigned char) division) & 0b00000111;
    ADCSRA = (1 << ADEN) | (0 << ADATE) | div;
}

unsigned int AnalogToDigitConverter::getInteger(unsigned char channel) const
{
    // select the corresponding channel 0~7
    channel &= 0b00000111;
    ADMUX = (ADMUX & 0xF8) | channel;
    ADCSRA |= (1 << ADSC);
    while ((ADCSRA & (1 << ADIF)) == 0);
    ADCSRA |= (1 << ADIF);
    unsigned int res = ADCW;
    return res;
}

float AnalogToDigitConverter::getVoltage(unsigned char channel) const
{
    return (vRef * (float) getInteger(channel)) / 1024.0;
}

/************************************************************************
 * Class PeriodicalEvent
 ************************************************************************/
PeriodicalEvent::PeriodicalEvent(const RealTimeClock * _rtc, time_ms _delay, long _maxOccurrence /* = -1*/) :
        rtc(_rtc), lastEventTime(0), delay(_delay), maxOccurrence(_maxOccurrence), occurred(0)
{
    // empty	
}

void PeriodicalEvent::resetTime()
{
    lastEventTime = rtc->timeMillisec;
    occurred = 0;
};

bool PeriodicalEvent::isOccured()
{
    if (maxOccurrence > 0 && occurred >= maxOccurrence)
    {
        return false;
    }
    if (rtc->timeMillisec >= lastEventTime + delay)
    {
        lastEventTime = rtc->timeMillisec;
        if (maxOccurrence > 0)
        {
            ++occurred;
        }
        return true;
    }
    return false;
}

/************************************************************************
 * Class AnalogComparator
 ************************************************************************/
AnalogComparator::AnalogComparator(IOPort::Name pinAin0Name, unsigned char pinAin0Nr, IOPort::Name pinAin1Name, unsigned char pinAin1Nr) :
        pinAin0(pinAin0Name, pinAin0Nr, IOPort::INPUT), pinAin1(pinAin1Name, pinAin1Nr, IOPort::INPUT)
{
    pinAin0.setPullUp(false);
    pinAin1.setPullUp(false);
    turnOff();
}

int AnalogComparator::getValue() const
{
    return ACSR & (1 << ACO);
}

void AnalogComparator::onInterrupt()
{
    // empty	
}

void AnalogComparator::turnOn()
{
    ACSR = (0 << ACD) | (1 << ACBG) | (1 << ACIE);
}

void AnalogComparator::turnOff()
{
    ACSR = (1 << ACD);
}

/************************************************************************
 * Class FastPwmT0
 ************************************************************************/
FastPwmT0::FastPwmT0()
{
    // empty
}

void FastPwmT0::start(bool channelA, bool channelB)
{
    // TCCR0A: Timer/Counter Control Register A:
    // set none-inverting fast PWM mode to given channels
    TCCR0A = 0;
    if (channelA)
    {
        OC0A_DDR |= (1 << OC0A_BIT);
        TCCR0A |= (1 << COM0A1);
        OCR0A = 0;
    }
    if (channelB)
    {
        OC0A_DDR |= (1 << OC0B_BIT);
        TCCR0A |= (1 << COM0B1) | (0 << COM0B0);
        OCR0B = 0;
    }
    TCCR0A |= (1 << WGM01) | (1 << WGM00);

    // TCCR0B: Timer/Counter Control Register B:
    // set no prescaling
    TCCR0B |= (1 << CS00);
}

void FastPwmT0::stop()
{
    TCCR0A = 0;
    TCCR0B = 0;
}

}
