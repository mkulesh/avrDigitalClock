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

#include "Screens.h"

#include <stdio.h>
#include <string.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

/************************************************************************
 * Common functions and constants
 ************************************************************************/

#define CHAR_SETTINGS 0b11111100
#define CHAR_ALARM 0b10010011
#define CHAR_DEGREE 0b11110010
#define CHAR_DCF 0b00010101

static const char * emptyString = "      ";
static const char * dayNames[7] = { "So", "Mo", "Di", "Mi", "Do", "Fr", "Sa" };

unsigned char cicleIncrement(unsigned char val, int s, unsigned char min, unsigned char max)
{
    unsigned char ret = val;
    if (s > 0 && ret == max)
    {
        ret = min;
    }
    else if (s < 0 && ret == min)
    {
        ret = max;
    }
    else
    {
        ret += s;
    }
    return ret;
}

/************************************************************************
 * Class HomeScreen
 ************************************************************************/
void HomeScreen::fillLine(int line, const DisplayDataProvider * dataProvider, char * dest)
{
    const AvrPlusPlus::tm & dayTime = dataProvider->getDayTime();
    if (line == 0)
    {
        sprintf(dest, "%c %02d.%02d.%04d %2s", (dataProvider->isDcfTimeAvailable() ? CHAR_DCF : ' '), dayTime.tm_mday,
                dayTime.tm_mon + 1, dayTime.tm_year + 2000, dayNames[dayTime.tm_wday]);
    }
    else
    {
        sprintf(dest, "%c %02d:%02d:%02d %2d%cC", (dataProvider->isAlarmActive() ? CHAR_ALARM : ' '), dayTime.tm_hour,
                dayTime.tm_min, dayTime.tm_sec, (int) (dataProvider->getTemperature()),
                CHAR_DEGREE);
    }
}

/************************************************************************
 * Class TimeSetting
 ************************************************************************/
const unsigned char TimeSetting::elementPos[6][3] = { 
        { 1, 0, 2 }, // TS_DAY
        { 4, 0, 2 }, // TS_MONTH
        { 7, 0, 4 }, // TS_YEAR
        { 1, 1, 2 }, // TS_HOUR
        { 4, 1, 2 }, // TS_MIN
        { 7, 1, 2 }  // TS_SEC
};

void TimeSetting::setNext()
{
    activeElement = (activeElement == TS_SEC) ? TS_DAY : (Element) ((int) activeElement + 1);
}

void TimeSetting::fillLine(int line, const DisplayDataProvider * dataProvider, char * dest)
{
    if (line == 0)
    {
        sprintf(dest, "%c%02d.%02d.%04d  %2s", CHAR_SETTINGS, dataProvider->getDayTime().tm_mday,
                dataProvider->getDayTime().tm_mon + 1, dataProvider->getDayTime().tm_year + 2000,
                dayNames[dataProvider->getDayTime().tm_wday]);
    }
    else
    {
        sprintf(dest, " %02d:%02d:%02d", dataProvider->getDayTime().tm_hour, dataProvider->getDayTime().tm_min,
                dataProvider->getDayTime().tm_sec);
    }
    if (!dataProvider->isActiveElementVisible() && elementPos[activeElement][1] == line)
    {
        strncpy(&dest[elementPos[activeElement][0]], emptyString, elementPos[activeElement][2]);
    }
}

void TimeSetting::modifyValue(AvrPlusPlus::tm & dayTime, int s)
{
    switch (activeElement)
    {
    case TimeSetting::TS_DAY:
        dayTime.tm_mday += s;
        break;
    case TimeSetting::TS_MONTH:
        if (s < 0 && dayTime.tm_mon == AvrPlusPlus::JANUARY)
        {
            dayTime.tm_mon = AvrPlusPlus::DECEMBER;
            dayTime.tm_year--;
        }
        else if (s > 0 && dayTime.tm_mon == AvrPlusPlus::DECEMBER)
        {
            dayTime.tm_mon = AvrPlusPlus::JANUARY;
            ++dayTime.tm_year;
        }
        else
        {
            dayTime.tm_mon += s;
        }
        break;
    case TimeSetting::TS_YEAR:
        dayTime.tm_year += s;
        break;
    case TimeSetting::TS_HOUR:
        dayTime.tm_hour += s;
        break;
    case TimeSetting::TS_MIN:
        dayTime.tm_min += s;
        break;
    case TimeSetting::TS_SEC:
        dayTime.tm_sec += s;
        break;
    }
}

/************************************************************************
 * Class BrightnessSetting
 ************************************************************************/
BrightnessSetting::Data EEMEM brightnessDataEE = {false, 100};

const unsigned char BrightnessSetting::elementPos[2][3] = { 
    { 1, 1, 4 }, // BS_MODE
    { 6, 1, 3 }  // BS_MAN_VALUE
};

BrightnessSetting::BrightnessSetting() :
        activeElement(BS_MODE)
{
    eeprom_busy_wait();
    eeprom_read_block(&data, &brightnessDataEE, sizeof(BrightnessSetting::Data));
}

void BrightnessSetting::setFirst()
{
    activeElement = BS_MODE;
}

void BrightnessSetting::setNext()
{
    if (data.isManual)
    {
        activeElement = (activeElement == BS_MODE) ? BS_MAN_VALUE : BS_MODE;
    }
    else
    {
        activeElement = BS_MODE;
    }
}

void BrightnessSetting::fillLine(int line, const DisplayDataProvider * dataProvider, char * dest)
{
    if (line == 0)
    {
        sprintf(dest, "%cBeleuchtung:", CHAR_SETTINGS);
    }
    else
    {
        if (data.isManual)
        {
            sprintf(dest, " MAN: %3d%%", data.manValue);
        }
        else
        {
            sprintf(dest, " AUTO      ");
        }
        if (!dataProvider->isActiveElementVisible())
        {
            strncpy(&dest[elementPos[activeElement][0]], emptyString, elementPos[activeElement][2]);
        }
    }
}

void BrightnessSetting::modifyValue(int s)
{
    if (activeElement == BS_MODE)
    {
        data.isManual = !data.isManual;
    }
    else
    {
        data.manValue = cicleIncrement(data.manValue, s, 0, 100);
    }
    cli();
    eeprom_busy_wait();
    eeprom_write_block(&data, &brightnessDataEE, sizeof(BrightnessSetting::Data));
    sei();
}

/************************************************************************
 * Class AlarmSetting
 ************************************************************************/
const unsigned char AlarmSetting::elementPos[10][3] = { 
        { 5, 0, 3 }, // AS_ACTIVE
        { 9, 0, 2 }, // AS_HOUR
        { 12, 0, 2 }, // AS_MIN
        { 1, 1, 1 }, // AS_DAY1
        { 3, 1, 1 }, // AS_DAY2
        { 5, 1, 1 }, // AS_DAY3
        { 7, 1, 1 }, // AS_DAY4
        { 9, 1, 1 }, // AS_DAY5
        { 11, 1, 1 }, // AS_DAY6
        { 13, 1, 1 }  // AS_DAY7
};
const char * AlarmSetting::activeString[2] = { "AUS", " AN" };
AlarmSetting::Data EEMEM alarmDataEE1 = {true, 6, 50,
    {false, true, true, true, true, true, false}};
AlarmSetting::Data EEMEM alarmDataEE2 = {true, 7, 30,
    {false, true, true, true, true, true, false}};
AlarmSetting::Data EEMEM alarmDataEE3 = {false, 9, 10,
    {false, true, true, true, true, true, false}};

AlarmSetting::AlarmSetting(unsigned char _number) :
        activeElement(AS_ACTIVE), 
        number(_number)
{
    eeprom_busy_wait();
    switch (number)
    {
    case 1:
        eeprom_read_block(&data, &alarmDataEE1, sizeof(AlarmSetting::Data));
        break;
    case 2:
        eeprom_read_block(&data, &alarmDataEE2, sizeof(AlarmSetting::Data));
        break;
    case 3:
        eeprom_read_block(&data, &alarmDataEE3, sizeof(AlarmSetting::Data));
        break;
    }
}

void AlarmSetting::setFirst()
{
    activeElement = AS_ACTIVE;
}

void AlarmSetting::setNext()
{
    activeElement = (activeElement == AS_DAY7) ? AS_ACTIVE : (Element) ((int) activeElement + 1);
}

void AlarmSetting::fillLine(int line, const DisplayDataProvider * dataProvider, char * dest)
{
    if (line == 0)
    {
        sprintf(dest, "%cW%d: %3s %02d:%02d",
        CHAR_SETTINGS, number, activeString[data.isActive], data.hour, data.min);
    }
    else
    {
        sprintf(dest, " S M D M D F S");
        for (unsigned char i = 0; i < 7; ++i)
        {
            if (!data.days[i])
            {
                dest[elementPos[i + 3][0]] = '.';
            }
        }
    }
    if (!dataProvider->isActiveElementVisible() && elementPos[activeElement][1] == line)
    {
        strncpy(&dest[elementPos[activeElement][0]], emptyString, elementPos[activeElement][2]);
    }
}

void AlarmSetting::modifyValue(int s)
{
    switch (activeElement)
    {
    case AS_ACTIVE:
        data.isActive = !data.isActive;
        break;
    case AS_HOUR:
        data.hour = cicleIncrement(data.hour, s, 0, 23);
        break;
    case AS_MIN:
        data.min = cicleIncrement(data.min, s, 0, 59);
        break;
    case AS_DAY1:
    case AS_DAY2:
    case AS_DAY3:
    case AS_DAY4:
    case AS_DAY5:
    case AS_DAY6:
    case AS_DAY7:
        data.days[activeElement - 3] = !data.days[activeElement - 3];
        break;
    }
    cli();
    eeprom_busy_wait();
    switch (number)
    {
    case 1:
        eeprom_write_block(&data, &alarmDataEE1, sizeof(AlarmSetting::Data));
        break;
    case 2:
        eeprom_write_block(&data, &alarmDataEE2, sizeof(AlarmSetting::Data));
        break;
    case 3:
        eeprom_write_block(&data, &alarmDataEE3, sizeof(AlarmSetting::Data));
        break;
    }
    sei();
}

bool AlarmSetting::isOccured(const AvrPlusPlus::tm & dayTime) const
{
    return data.isActive && data.days[dayTime.tm_wday] && data.hour == dayTime.tm_hour && data.min == dayTime.tm_min;
}
