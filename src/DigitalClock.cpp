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

#include "DigitalClock.h"

#include <stdio.h>
#include <math.h>

#define LIGHT_SENSOR_CHANNEL 3
#define TEMP_SENSOR_CHANNEL 4

/************************************************************************
 * Class DcfData
 ************************************************************************/
DigitalClock::DcfData::DcfData() :
        dcfUpdatePeriod(3600)
{
    invalidateTime();
}

void DigitalClock::DcfData::invalidateTime()
{
    dcfTimeReceived = false;
    lastReceivedTime = INFINITY_SEC;
    min = hour = day = month = year = -1;
};

void DigitalClock::DcfData::onTimeReceived(int _min, int _hour, int _day, int _month, int _year)
{
    if (hour == _hour && day == _day && month == _month && year == _year)
    {
        dcfTimeReceived = true;
    }
    min = _min;
    hour = _hour;
    day = _day;
    month = _month;
    year = _year;
}

/************************************************************************
 * Class DigitalClock
 ************************************************************************/
DigitalClock::DigitalClock(RealTimeClock * _rtc) :
        rtc(_rtc),
        mcuCS(IOPort::B, PB4, IOPort::OUTPUT),
        ledSec1(IOPort::C, PC0, Devices::Led::ANODE, true),
        ledSec2(IOPort::C, PC1, Devices::Led::ANODE, false),
        ssdLine(IOPort::D, PD2, Devices::Led::ANODE, false),
        ssd(IOPort::B, PB5, PB7, IOPort::D, PD1), 
        displayBrightness(IOPort::B, PB5, PB7, IOPort::D, PD5),
        lcd(IOPort::B, PB5, PB7, IOPort::D, PD4, PD3),
        bMode(IOPort::B, PB0, _rtc),
        bActiveElement(IOPort::B, PB1, _rtc),
        bPlus(IOPort::A, PA1, _rtc),
        bMinus(IOPort::A, PA2, _rtc),
        piezoAlarm(IOPort::C, PC2, _rtc),
        ledToggle(_rtc, 500, 1),
        secToggle(_rtc, 1000),
        activeElementToggle(_rtc, 250, 3),
        returnToHome(_rtc, 30000, 1),
        secondsCorrection(_rtc, 4870000L, 1),
        adc(),
        dcfSignal(rtc, IOPort::B, PB2, IOPort::B, PB3),
        dcfBitReceived(IOPort::C, PC5, Devices::Led::ANODE, false),
        dcfBitFailed(IOPort::C, PC4, Devices::Led::ANODE, false),
        dcfPower(IOPort::C, PC3, Devices::Led::ANODE, false),
        dcfData(),
        activeElementVisible(false),
        homeScreen(),
        timeSetting(),
        brightnessSetting(),
        alarmSetting1(1),
        alarmSetting2(2),
        alarmSetting3(3),
        activeScreen(SCR_HOME),
        temperature(0.0),
        temperatureTrial(0)
#ifdef UART_DEBUG
,uart(500000)
#endif
{
    // empty
}

void DigitalClock::init()
{
    mcuCS.setLow();
    lcd.putString(0, 0, "Hallo!");

    dcfSignal.setHandler(this);

    screens[SCR_HOME] = &homeScreen;
    screens[SCR_TIME_SETTING] = &timeSetting;
    screens[SCR_BRIGHTNESS] = &brightnessSetting;
    screens[SCR_ALARM1] = &alarmSetting1;
    screens[SCR_ALARM2] = &alarmSetting2;
    screens[SCR_ALARM3] = &alarmSetting3;

    alarms[0] = &alarmSetting1;
    alarms[1] = &alarmSetting2;
    alarms[2] = &alarmSetting3;

    Devices::Ssd::SegmentsMask sm;
    sm.top = 3;
    sm.rightTop = 5;
    sm.rightBottom = 7;
    sm.bottom = 4;
    sm.leftBottom = 1;
    sm.leftTop = 2;
    sm.center = 6;
    sm.dot = 0;
    ssd.setSegmentsMask(sm);
    ssdLine.setHigh();
    ssd.putString("0000", 4);

    displayBrightness.setOutputGain(true);

    adc.init(2.506, AnalogToDigitConverter::DIV_128);

    piezoAlarm.start(1);
}

void DigitalClock::resetEvents()
{
    bActiveElement.resetTime();
    bPlus.resetTime();
    bMinus.resetTime();
    secToggle.resetTime();
    ledToggle.resetTime();
    activeElementToggle.resetTime();
    secondsCorrection.resetTime();
    piezoAlarm.resetTime();
}

void DigitalClock::periodic()
{
    piezoAlarm.periodic();
    if (dcfData.dcfTimeReceived)
    {
        dcfActivate(false);
        dcfData.lastReceivedTime = mktime(dcfSignal.getDayTime());
        const_cast<RealTimeClock *>(rtc)->setTime(dcfData.lastReceivedTime);
        resetEvents();
        return;
    }
    if (bMode.isPressed())
    {
        bMode.setProcessed();
        returnToHome.resetTime();
        if (piezoAlarm.finish())
        {
            return;
        }
        lcd.clear();
        activeScreen =
                (activeScreen == (ScreenType) (screensNumber - 1)) ? SCR_HOME : (ScreenType) ((int) activeScreen + 1);
        screens[activeScreen]->setFirst();
        activeElementVisible = activeScreen != SCR_HOME;
        return;
    }
    if (bMode.isLongPressed())
    {
        dcfActivate(true);
        setHomeScreen();
        return;
    }
    if (bActiveElement.isPressed())
    {
        bActiveElement.setProcessed();
        returnToHome.resetTime();
        if (piezoAlarm.finish())
        {
            return;
        }
        screens[activeScreen]->setNext();
        activeElementToggle.resetTime();
        activeElementVisible = true;
        updateLcd(false);
        return;
    }
    if (bPlus.isPressed() || bPlus.isLongPressed())
    {
        bPlus.setProcessed();
        if (piezoAlarm.finish())
        {
            return;
        }
        modifyActiveElement(1);
        returnToHome.resetTime();
        bPlus.resetTime();
        return;
    }
    if (bMinus.isPressed() || bMinus.isLongPressed())
    {
        bMinus.setProcessed();
        if (piezoAlarm.finish())
        {
            return;
        }
        modifyActiveElement(-1);
        returnToHome.resetTime();
        bMinus.resetTime();
        return;
    }
    if (secToggle.isOccured())
    {
        dcfBitReceived.turnOff();
        dcfBitFailed.turnOff();
        measureTemperature();
        updateLcd(true);
        ledToggle.resetTime();
        activeElementToggle.resetTime();
        ledSec1.toggle();
        ledSec2.toggle();
        if (dayTime.tm_sec < 5)
        {
            updateSsd();
        }
        updateBrightness();
        if (secondsCorrection.isOccured())
        {
            correctSeconds();
            secondsCorrection.resetTime();
        }
        bool alarmOccured = false;
        for (unsigned char a = 0; a < alarmsNumber; ++a)
        {
            if (alarms[a]->isOccured(dayTime))
            {
                alarmOccured = true;
                break;
            }
        }
        if (alarmOccured)
        {
            piezoAlarm.start(15);
        }
        return;
    }
    if (ledToggle.isOccured())
    {
        ledSec1.toggle();
        ledSec2.toggle();
    }
    if (activeScreen != SCR_HOME)
    {
        if (activeElementToggle.isOccured())
        {
            updateLcd(true);
        }
        else if (returnToHome.isOccured())
        {
            setHomeScreen();
        }
    }
}

void DigitalClock::correctSeconds()
{
    timeSetting.setActiveElement(TimeSetting::TS_SEC);
    timeSetting.modifyValue(dayTime, -1);
    const_cast<RealTimeClock *>(rtc)->setTime(mktime(dayTime));
    resetEvents();
}

void DigitalClock::setHomeScreen()
{
    activeScreen = SCR_HOME;
    screens[activeScreen]->setFirst();
    activeElementVisible = false;
    updateLcd(false);
}

void DigitalClock::updateBrightness()
{
    if (brightnessSetting.isManual())
    {
        displayBrightness.putValue(brightnessSetting.manValue());
    }
    else
    {
        unsigned int currLight = adc.getInteger(LIGHT_SENSOR_CHANNEL) / 8;
        if (currLight > 100)
        {
            currLight = 100;
        }
        displayBrightness.putValue(100 - currLight);
    }
}

void DigitalClock::updateLcd(bool changeActiveElement)
{
    gmtime(rtc->timeSec, dayTime);
    screens[activeScreen]->fillLine(0, this, lcdString);
    lcd.putString(0, 0, lcdString);
    screens[activeScreen]->fillLine(1, this, lcdString);
    lcd.putString(0, 1, lcdString);
    if (changeActiveElement)
    {
        activeElementVisible = !activeElementVisible;
    }
}

void DigitalClock::updateSsd()
{
    sprintf(lcdString, "%02d%02d", dayTime.tm_hour, dayTime.tm_min);
    ssd.putString(lcdString, 4, false);
}

void DigitalClock::modifyActiveElement(int s)
{
    switch (activeScreen)
    {
    case SCR_HOME:
    {
        return;
    }
    case SCR_TIME_SETTING:
    {
        timeSetting.modifyValue(dayTime, s);
        const_cast<RealTimeClock *>(rtc)->setTime(mktime(dayTime));
        resetEvents();
        updateSsd();
        break;
    }
    case SCR_BRIGHTNESS:
    {
        brightnessSetting.modifyValue(s);
        updateBrightness();
        break;
    }
    case SCR_ALARM1:
        alarmSetting1.modifyValue(s);
        break;
    case SCR_ALARM2:
        alarmSetting2.modifyValue(s);
        break;
    case SCR_ALARM3:
        alarmSetting3.modifyValue(s);
        break;
    }
    activeElementVisible = true;
    updateLcd(false);
}

bool DigitalClock::isAlarmActive() const
{
    return alarmSetting1.isActive() || alarmSetting2.isActive() || alarmSetting3.isActive();
}

void DigitalClock::measureTemperature()
{
    double v = adc.getVoltage(TEMP_SENSOR_CHANNEL) * 1000;
    double t = (float) (30.0 + (10.888 - sqrt(10.888 * 10.888 + 4.0 * 0.00347 * (1777.3 - v))) / (-2.0 * 0.00347) - 1.5);
    temperatureArr[temperatureTrial] = t;
    ++temperatureTrial;
    if (temperatureTrial == temperatureTrials)
    {
        float temperatureSum = 0.0;
        for (unsigned char i = 0; i < temperatureTrials; ++i)
        {
            temperatureSum += temperatureArr[i];
        }
        temperature = temperatureSum / ((float) temperatureTrials);
        temperatureTrial = 0;
    }
}

void DigitalClock::dcfActivate(bool flag)
{
    dcfData.invalidateTime();
    if (flag)
    {
        dcfSignal.turnOn();
    }
    else
    {
        dcfSignal.turnOff();
    }
    dcfPower.putBit(flag);
}

void DigitalClock::onDcfLog(const char * str)
{
#ifdef UART_DEBUG
    uart.putString(str);
#endif
}

void DigitalClock::onTimeReceived(int min, int hour, int day, int month, int year)
{
    piezoAlarm.start(1);
    dcfData.onTimeReceived(min, hour, day, month, year);
}

void DigitalClock::onBitReceived()
{
    dcfBitReceived.turnOn();
}

void DigitalClock::onBitFailed()
{
    dcfBitFailed.turnOn();
}
