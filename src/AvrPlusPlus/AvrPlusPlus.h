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

#ifndef AVRPLUSPLUS_H_
#define AVRPLUSPLUS_H_

#define F_CRYSTAL 8
#define F_CPU 8000000UL // 8 MHz
#include "Time.h"

#include <avr/io.h>

namespace AvrPlusPlus
{

#define lowByte(x)    ((x) & 0xFF)
#define highByte(x)   (((x)>>8) & 0xFF)

/** 
 * @brief Static class collecting helper methods for general system settings.
 */
class System
{
private:

    static float voltage;

public:

    /** 
     * @brief Enumeration collecting dufferent pre-scale (division) factors for system clock.
     */
    enum Prescale
    {
        PRE_1 = 0, // no division, MCU frequence as it is
        PRE_2 = 1, // system clock can be divided by 2
        PRE_4 = 2, // system clock can be divided by 4
        PRE_8 = 3  // system clock can be divided by 8
    };

    /** 
     * @brief Sets the clock division factor.
     *    
     * The system clock can be divided by setting the "CLKPR - Clock Prescale Register". This 
     * feature can be used to decrease the system clock frequency and the power consumption when 
     * the requirement for processing power is low.
     * 
     * @param prescale enumeration value that corresponds to the necessary division factor. 
     */
    static void setClockDivisionFactor(Prescale prescale);

    /** 
     * @brief Disable JTAG interface.
     *
     * This procedure allows to disable JTAG interface in order to use pins hosting this interface
     * (for example pins C2-C5 on Atmega644) as normal input/output pins.
     */
    static void disableJTAG();

    static inline void setVoltage(float _voltage)
    {
        voltage = _voltage;
    };
    static inline float getVoltage()
    {
        return voltage;
    };
};

/** 
 * @brief Base IO port class.
 *
 * This class contains an interface to whole IO port (8 pins) and allows to set port direction and
 * put 8 bit data into the port.
 */
class IOPort
{
protected:

    /** 
     * @brief Pointers to the port registers.
     *
     * Each port pin consists of three register bits: DDx, PORTx, and PINx. We will store here
     * pointers to these registers and use these pointers for each input/output operation.
     *
     * These pointers will be set in the construstur depending on given port name. There are no
     * interface to modity these pointers afterwards online.
     */
    volatile unsigned char * port; // the PORTx I/O address
    volatile unsigned char * pins; // the PINx I/O address
    volatile unsigned char * ddr;  // the DDRx I/O address

public:

    /** 
     * @brief Enumeration collecting port names.
     *
     * The port name will be used in the constructor in order to set-up the pointers to the port 
     * registers.
     */
    enum Name
    {
        A = 0, B = 1, C = 2, D = 3
    };

    /** 
     * @brief Enumeration collecting port directions.
     */
    enum Direction
    {
        INPUT = 0, OUTPUT = 1
    };

    /** 
     * @brief Default constructor.
     *
     * The constructor initializes the pointers to three port registers: DD(x), PORT(x), and PIN(x), 
     * where (x) corresponds to the port name parameter. 
     * 
     * @param name of the port. This is a letter corresponding to Name enumeration. If the given 
     *        port name is unknown for the target Atmega model, constructor calls abort().
     */
    IOPort(Name name);

    /** 
     * @brief Procedure sets necessary direction for whole port.
     *
     * @param direction of the port: INPUT or OUTPUT
     */
    void setDirection(Direction direction);

    /** 
     * @brief Procedure puts a byte (8 bit) into the PORT register of this port.
     * 
     * @param value
     */
    void putChar(unsigned char value);
};

/** 
 * @brief Class that describes a single port pin.
 * 
 * All AVR ports have true Read-Modify-Write functionality when used as general digital I/O ports.
 * This means that the direction of one port pin can be changed without unintentionally changing
 * the direction of any other pin.
 * The same applies when changing drive value (if configured as output) or enabling/disabling of 
 * pull-up resistors (if configured as input).
 */
class IOPin: public IOPort
{
protected:

    /** 
     * @brief Pin number in the range [0..7]. 
     */
    volatile unsigned char pin;

public:

    /** 
     * @brief Default constructor.
     *
     * The constructor initializes the given port pin as an input or output pin. 
     * 
     * @param name of the port. This is a letter corresponding to Name enumeration. If the given 
     *        port name is unknown for the target Atmega model, constructor calls abort().
     * @param pinNr pin number in the range [0..7]. Pin constants defined within the file 
     *        <avr/io.h> can be also used. 
     * @param direction of the port: INPUT or OUTPUT
     */
    IOPin(Name name, unsigned char pinNr, Direction direction);

    /** 
     * @brief Procedure sets necessary direction for this port pin.
     *
     * @param direction of the pin: INPUT or OUTPUT. The direction of one port pin can be 
     *        changed without unintentionally changing the direction of any other pin.
     */
    void setDirection(Direction direction);

    /** 
     * @brief Procedure writes logic one into the pin.
     * 
     * If PORTxn is written logic one when the pin is configured as an input pin, the pull-up
     * resistor is activated, see setPullUp method.
     * 
     * If PORTxn is written logic one when the pin is configured as an output pin, the port pin 
     * is driven high (one).
     */
    void setHigh();

    /** 
     * @brief Procedure writes logic zero into the pin.
     * 
     * If PORTxn is written logic zero the pin is configured as an input pin, the pull-up
     * resistor is de-activated, see setPullUp method.
     * 
     * If PORTxn is written logic zero when the pin is configured as an output pin, the port 
     * pin is driven low (zero). 
     */
    void setLow();

    /** 
     * @brief Procedure writes logic one or zero into the pin depending on given parameter.
     * 
     * @param value if set to true, procedure writes logic one into the pin. Else logic zero
     *        will be written.
     */
    void putBit(bool value);

    /** 
     * @brief Procedure returns the current value in the PORT register of this pin.
     *
     * Note: if the pin is configured as an input pin, this procedure returns the state of the 
     * pull-up resistor rather then the input value. For the input value, see readInput() method.
     * 
     * @return high (logic one) or low (logic zero) state of the PORT register for this pin.
     */
    bool getBit() const;

    /** 
     * @brief Procedure returns current input value (PIN register) of this pin.
     * 
     * @return high (logic one) or low (logic zero) state of the PIN register for this pin
     *         depending on the input state of this pin.
     */
    bool readInput() const;

    /** 
     * @brief If the pin is configured as an input pin, this procedure toggles the pull-up resistor.
     * 
     * @param pullUp if set to true, the pull-up resistor is activated, else it will be switched off.
     */
    inline void setPullUp(bool pullUp)
    {
        putBit(pullUp);
    };
};

/** 
 * @brief Class that implements real time clock.
 *
 * The real time clock is implemented using two timers at the same time:
 * 1) 16-bit timer T1 is used to count milliseconds, it is synchroneus with the system clock and is driven 
 *    by system takt generator.
 *    Output Compare Match Interrupt Handler ISR(TIMER1_COMPA_vect) shall be declared in the main file. 
 *    This interrupt will occures every milliseconds.
 *    The handler shall call onInterrupCompareMatch() method.
 *
 * 2) 8-bit timer T2 driven asynchronously by the external 32kHz quartz is used in order to precisely 
 *    count seconds and to correct possible error in the first timer.
 *    Overflow Interrupt Handler ISR(TIMER2_OVF_vect) shall be declared in the main file. 
 *    This interrupt will occures every seconds.
 *    The handler shall call onInterrupOverflow() method.
 *
 * Current time is represented by two variables that are automatically synchronized:
 * 1) time_ms timeMillisec is used to store milliseconds number since clock start.
 * 2) time_t timeSec is used to store seconds number since clock start.
 * Both types time_ms and time_t are declared in the file Time.h
 */
class RealTimeClock
{
protected:

    volatile unsigned int syncMs1, syncMs2;
    volatile int errorMs;
    void setupTimer1();
    void setupTimer2();

public:

    volatile time_ms timeMillisec; // current time (in milliseconds)
    volatile time_t timeSec; // current time (in seconds)

    /** 
     * @brief Default constructor.
     */
    RealTimeClock();

    /** 
     * @brief Procedure prepares and activates 16-bit timer T1 and 8-bit timer T2.
     *
     * Note: this procedure calls sei() method in order to enable interrupts.
     */
    void startClock();

    /** 
     * @brief Handler for milliseconds interrupt.
     *
     * This method shall be called from ISR(TIMER1_COMPA_vect) handler declared and implemented
     * in the main application file.
     */
    void onInterrupCompareMatch();

    /** 
     * @brief Handler for seconds interrupt.
     *
     * This method shall be called from ISR(TIMER2_OVF_vect) handler declared and implemented
     * in the main application file.
     */
    void onInterrupOverflow();

    /** 
     * @brief Procedure sets the given seconds value. 
     */
    void setTime(time_t sec);

    inline int getErrorMs() const
    {
        return errorMs;
    };
};

/** 
 * @brief Class that implements SPI device interface.
 *
 * This class provides methods aimed to send data via SPI to a device connected to the MCU SPI pins.
 * Currently, SPI masted functionality only is supported (no possibility to read any data from SPI device).
 *
 * To send data to an SPI device, three pins are necessary:
 * - MOSI pin is used to transfer serial data,
 * - SCK pin is used clock signal, 
 * - CS is the chip select pin. SPI device shall accept the serial data if this pin is driven low (zero).
 *
 * These three pins are given in the constructor of this class.
 */
class SpiDevice: public IOPin
{
private:

    // CS pin is declared as the parent class,
    // MOSI and SCK pins are declared as member variables:
    IOPin pinMosi, pinSck;

public:

    /** 
     * @brief Default constructor used to initialize an SPI device.
     *
     * @param spiPortName of the port where SPI interface is presented. This is a letter corresponding 
     *        to Name enumeration. If the given port name is unknown for the target Atmega model, 
     *        constructor calls abort().
     * @param pinMosiNr number in the range [0..7] that corresponds to the MOSI pin within SPI port. 
     * @param pinSckNr number in the range [0..7] that corresponds to the SCK pin within SPI port. 
     * @param devicePortName of the port where chip select pin of the SPI device is connected.
     * @param pinCsNr number in the range [0..7] that corresponds to the chip select pin of the SPI device.
     */
    SpiDevice(Name spiPortName, unsigned char pinMosiNr, unsigned char pinSckNr, Name devicePortName,
            unsigned char pinCsNr);

    /** 
     * @brief Procedure is called if the SPI device shall start the data reception.
     */
    inline void startTransfer()
    {
        setLow();
    };

    /** 
     * @brief Procedure transfers a byte to the device and waits until transmission is complete.
     *
     * @param data character to be sent.
     */
    void putChar(char data);

    /** 
     * @brief Procedure transfers a word (two bytes) to the device and waits until transmission is complete.
     *
     * @param data integer to be sent. The high byte is transferred first, and the low byte is transferred 
     *        after the high byte.
     */
    void putInt(int data);

    /** 
     * @brief Procedure is called if the SPI device shall finish the data reception.
     */
    inline void finishTransfer()
    {
        setHigh();
    };
};

/** 
 * @brief Class that implements UART interface.
 */
class Usart
{
public:

    /** 
     * @brief Default constructor used to initialize UART interface.
     *
     * This constructor initializes UART with 8-bit transmission mode and one stop bit.
     *
     * @param baudrate the necessary transmission speed.
     */
    Usart(unsigned long baudrate);

    /** 
     * @brief Procedure waits until previous transmission is complete and transfers a byte.
     *
     * @param data character to be sent.
     */
    void putChar(char data);

    /** 
     * @brief Procedure waits until previous transmission is complete and transfers a byte.
     *
     * @param data string to be sent.
     */
    void putString(const char * data);
};

/** 
 * @brief Class that implements analog-to-digit converter
 */
class AnalogToDigitConverter
{
private:

    volatile float vRef;

public:

    enum Division
    {
        DIV_2 = 1, DIV_4 = 2, DIV_8 = 3, DIV_16 = 4, DIV_32 = 5, DIV_64 = 6, DIV_128 = 7,
    };

    AnalogToDigitConverter()
    {
    };
    void init(float _vRef, Division division);
    unsigned int getInteger(unsigned char channel) const;
    float getVoltage(unsigned char channel) const;
};

/** 
 * @brief Class that implements a periodical event with a given delay
 */
class PeriodicalEvent
{
private:

    volatile const RealTimeClock * rtc;
    volatile time_ms lastEventTime, delay;
    volatile long maxOccurrence, occurred;

public:

    PeriodicalEvent(const RealTimeClock * _rtc, time_ms _delay, long _maxOccurrence = -1);
    void resetTime();
    bool isOccured();
};

/** 
 * @brief Class that implements analog comparator on the pins given in the constructor
 */
class AnalogComparator
{
private:

    IOPin pinAin0, pinAin1;

public:

    AnalogComparator(IOPort::Name pinAin0Name, unsigned char pinAin0Nr, IOPort::Name pinAin1Name,
            unsigned char pinAin1Nr);
    int getValue() const;
    virtual void onInterrupt();
    virtual void turnOn();
    virtual void turnOff();
};

/** 
 * @brief Class that fast PWM using timer T0
 */
class FastPwmT0
{
public:

    FastPwmT0();
    void start(bool channelA, bool channelB);
    void stop();
    inline void setChannelAValue(unsigned char val)
    {
        OCR0A = val;
    };
    inline void setChannelBValue(unsigned char val)
    {
        OCR0B = val;
    };
};

} // end of namespace AvrPlusPlus

#endif
