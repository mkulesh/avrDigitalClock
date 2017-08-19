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

#ifndef DIGITALCLOCK_H_
#define DIGITALCLOCK_H_

#include "AvrPlusPlus/AvrPlusPlus.h"
#include "AvrPlusPlus/Devices/Led.h"
#include "AvrPlusPlus/Devices/Ssd.h"
#include "AvrPlusPlus/Devices/Lcd_DOGM162.h"
#include "AvrPlusPlus/Devices/Dac_MCP4901.h"
#include "AvrPlusPlus/Devices/Button.h"
#include "AvrPlusPlus/Devices/PiezoAlarm.h"
#include "AvrPlusPlus/Devices/Dcf77.h"
#include "Screens.h"

// #define UART_DEBUG 0

using namespace AvrPlusPlus;

class DigitalClock : public DisplayDataProvider, public Devices::Dcf77Handler
{
private:

	volatile RealTimeClock * rtc;

    // Chip Select for MCU SPI
	IOPin mcuCS;
	
	// LEDs for seconds
	Devices::Led ledSec1, ledSec2;
	
	// common SCLR line for seven segment display: hide display content
	Devices::Led ssdLine;

	// Seven segment display with 4 digits
	Devices::Ssd_74HC595_SPI ssd;
	
	// DAC for display brightness
	Devices::Dac_MCP4901 displayBrightness;
	
	// LCD DOGM186
	Devices::Lcd_DOGM162_SPI lcd;
	
	// Buttons
	Devices::Button bMode, bActiveElement, bPlus, bMinus;

	// Piezo element
	Devices::PiezoAlarm piezoAlarm;

	// Periodical events
	PeriodicalEvent ledToggle, secToggle, activeElementToggle, returnToHome;
	
	// Seconds correction
	PeriodicalEvent secondsCorrection;
	
	// Light and temperature sensors
	AnalogToDigitConverter adc;
	
	// Radio-controlled clock
	Devices::Dcf77 dcfSignal;
	Devices::Led dcfBitReceived, dcfBitFailed, dcfPower;
	class DcfData
	{
	public:
		volatile duration_sec dcfUpdatePeriod;
		volatile bool dcfTimeReceived;
		volatile time_t lastReceivedTime;
		
	private:
		volatile int min, hour, day, month, year;
		
	public:
		DcfData ();
		void invalidateTime ();
		void onTimeReceived (int _min, int _hour, int _day, int _month, int _year);
	};
	DcfData dcfData;

	// Flag defining that the active element shall be displayed
	volatile bool activeElementVisible;

	// Available screens
	enum ScreenType
	{
		SCR_HOME = 0,           // home screen
		SCR_TIME_SETTING = 1,   // time setting screen
		SCR_BRIGHTNESS = 2,     // brightness setting screen
		SCR_ALARM1 = 3,         // alarm setting screen
		SCR_ALARM2 = 4,         // alarm setting screen
		SCR_ALARM3 = 5          // alarm setting screen
	};
	static const unsigned char screensNumber = 6;
	Screen * screens[screensNumber];
	
	// Layouts of screens
	HomeScreen homeScreen;
	TimeSetting timeSetting;
	BrightnessSetting brightnessSetting;
	AlarmSetting alarmSetting1, alarmSetting2, alarmSetting3;

	// Alarms
	static const unsigned char alarmsNumber = 3;
	const AlarmSetting * alarms[alarmsNumber];
	
	// Active screen	
	volatile ScreenType activeScreen;
	
	// current temperature
	static const unsigned char temperatureTrials = 10;
	volatile float temperatureArr[temperatureTrials];
	volatile float temperature;
	volatile unsigned char temperatureTrial;

	// temporary attributes
	char lcdString[16];
	tm dayTime;

	// UART used for logging
	#ifdef UART_DEBUG
	Usart uart;
	#endif
	
public:
	
	DigitalClock (RealTimeClock * _rtc);
	inline const AvrPlusPlus::tm & getDayTime () const { return dayTime; };
	inline bool isActiveElementVisible () const { return activeElementVisible; };
	inline bool isDcfTimeAvailable() const { return dcfData.lastReceivedTime != INFINITY_SEC; };
	inline float getTemperature () const { return temperature; };
	inline void onComparatorInterrupt () { dcfSignal.onInterrupt(); };
	void init ();
	void resetEvents ();
	void periodic ();
	void correctSeconds ();
	void setHomeScreen ();
	void updateBrightness ();
	void updateLcd (bool changeActiveElement);
	void updateSsd ();
	void modifyActiveElement (int s);
	bool isAlarmActive () const;
	void measureTemperature ();
	void dcfActivate(bool flag);
	virtual void onDcfLog (const char * str);
	virtual void onTimeReceived (int min, int hour, int day, int month, int year);
	virtual void onBitReceived ();
	virtual void onBitFailed ();
};

#endif /* DIGITALCLOCK_H_ */
