/*
Copyright 2016 German Martin (gmag11@gmail.com). All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met :

1. Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list
of conditions and the following disclaimer in the documentation and / or other materials
provided with the distribution.

THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.IN NO EVENT SHALL <COPYRIGHT HOLDER> OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of German Martin
*/
// 
// 
// 

#ifdef ARDUINO_ARCH_ESP8266

#include "NtpClientLib.h"
#include <ESP8266WiFi.h>

#define DBG_PORT Serial

#ifdef DEBUG_NTPCLIENT
#define DEBUGLOG(...) DBG_PORT.printf(__VA_ARGS__)
#else
#define DEBUGLOG(...)
#endif


NTPClient NTP;

NTPClient::NTPClient()
{
}

bool NTPClient::setNtpServerName(String ntpServerName, int idx)
{
	char * buffer = (char *)malloc((ntpServerName.length()+1) * sizeof(char));
	if ((idx >= 0) && (idx <= 2)) {
		sntp_stop();
		ntpServerName.toCharArray(buffer, ntpServerName.length()+1);
		sntp_setservername(idx, buffer);
		DEBUGLOG("NTP server %d set to: %s \r\n", idx, buffer);
		sntp_init();
		return true;
	}
	return false;
}

String NTPClient::getNtpServerName(int idx)
{
	if ((idx >= 0) && (idx <= 2)) {
		return String(sntp_getservername(idx));
	}
	return "";
}

bool NTPClient::setTimeZone(int timeZone)
{
	//if ((timeZone >= -11) && (timeZone <= 13)) {
	sntp_stop();
	bool result = sntp_set_timezone(timeZone);
	sntp_init();
	DEBUGLOG("NTP time zone set to: %d, result: %s\r\n", timeZone, result?"OK":"error");
	return result;
	//return true;
	//}
	//return false;
}

int NTPClient::getTimeZone()
{
	return sntp_get_timezone();
}

/*void NTPClient::setLastNTPSync(time_t moment) {
	_lastSyncd = moment;
}*/

time_t NTPClient::s_getTime() {
	NTP.getTime();
}

time_t NTPClient::getTime()
{
	DEBUGLOG("-- NTP Server hostname: %s\r\n", sntp_getservername(0));
	if (WiFi.isConnected())	{
		DEBUGLOG("-- Transmit NTP Request\r\n");
		uint32 secsSince1970 = sntp_get_current_timestamp();
		NTP.getUptime();
		if (secsSince1970) {
			setSyncInterval(NTP.getInterval()); // Normal refresh frequency
			DEBUGLOG("Sync frequency set low\r\n");
			if (getDayLight()) {
				if (summertime(year(secsSince1970), month(secsSince1970), day(secsSince1970), hour(secsSince1970), getTimeZone())) {
					secsSince1970 += SECS_PER_HOUR;
					DEBUGLOG("Summer Time\r\n");
				}
				else {
					DEBUGLOG("Winter Time\r\n");
				}

			}
			else {
				DEBUGLOG("No daylight\r\n");

			}
			getFirstSync();
			_lastSyncd = secsSince1970;
			if (!_firstSync) {
				_firstSync = secsSince1970;
				DEBUGLOG("First sync! %s\r\n", getTimeDateString(getFirstSync()).c_str());
			}
			if (onSyncEvent != NULL)
				onSyncEvent(timeSyncd);     // call the handler
			DEBUGLOG("Succeccful NTP sync at %s\r\n", getTimeDateString(getLastNTPSync()).c_str());
		}
		else {
			DEBUGLOG("-- NTP error :-(\r\n");
			if (onSyncEvent != NULL)
				onSyncEvent(noResponse);     // call the handler
			setSyncInterval(getShortInterval()); // Fast refresh frequency, until successful sync
		}

		return secsSince1970;
	}
	else {
		DEBUGLOG("-- NTP Error. WiFi not connected.\r\n");
		return 0;
	}
}

bool NTPClient::begin(String ntpServerName, int timeOffset, bool daylight)
{
	if (!setNtpServerName(ntpServerName)) {
		return false;
	}
	if (!setTimeZone(timeOffset)) {
		return false;
	}
	sntp_init();
	setDayLight(daylight);
	_lastSyncd = 0;

	if (!setInterval(DEFAULT_NTP_SHORTINTERVAL, DEFAULT_NTP_INTERVAL)) {
		return false;
	}
	DEBUGLOG("Time sync started\r\n");

	setSyncInterval(getShortInterval());
	setSyncProvider(s_getTime);

	return true;
}

bool NTPClient::stop() {
	setSyncProvider(NULL);
	DEBUGLOG("Time sync disabled\r\n");
	sntp_stop();
	return true;
}

bool NTPClient::setInterval(int interval)
{
	if (interval >= 10) {
		if (_longInterval != interval) {
			_longInterval = interval;
			DEBUGLOG("Long sync interval set to %d\r\n", interval);
			if (timeStatus() == timeSet)
				setSyncInterval(interval);
		}
		return true;
	}
	DEBUGLOG("Error setting interval %d\r\n", interval);

	return false;
}

bool NTPClient::setInterval(int shortInterval, int longInterval) {
	if (shortInterval >= 5 && longInterval >= 10) {
		_shortInterval = shortInterval;
		_longInterval = longInterval;
		if (timeStatus() != timeSet) {
			setSyncInterval(shortInterval);
		}
		else {
			setSyncInterval(longInterval);
		}
		DEBUGLOG("Short sync interval set to %d\r\n", shortInterval);
		DEBUGLOG("Long sync interval set to %d\r\n", longInterval);
		return true;
	}
	DEBUGLOG("Error setting interval. Short: %d Long: %d\r\n", shortInterval, longInterval);
	return false;
}

int NTPClient::getInterval()
{
	return _longInterval;
}

int NTPClient::getShortInterval()
{
	return _shortInterval;
}

void NTPClient::setDayLight(bool daylight)
{
	_daylight = daylight;
	DEBUGLOG("--Set daylight %s\r\n", daylight? "ON" : "OFF");
}

bool NTPClient::getDayLight()
{
	return _daylight;
}

String NTPClient::getTimeStr(time_t moment) {
	if ((timeStatus() != timeNotSet) || (moment != 0)) {
		String timeStr = "";
		timeStr += printDigits(hour(moment));
		timeStr += ":";
		timeStr += printDigits(minute(moment));
		timeStr += ":";
		timeStr += printDigits(second(moment));

		return timeStr;
	}
	else return "Time not set";
}

String NTPClient::getTimeStr() {
	return getTimeStr(now());
}

String NTPClient::getDateStr(time_t moment) {
	if ((timeStatus() != timeNotSet) || (moment != 0)) {
		String timeStr = "";

		timeStr += printDigits(day(moment));
		timeStr += "/";
		timeStr += printDigits(month(moment));
		timeStr += "/";
		timeStr += String(year(moment));

		return timeStr;
	}
	else return "Date not set";
}

String NTPClient::getDateStr() {
	return getDateStr(now());
}

String NTPClient::getTimeDateString(time_t moment) {
	if ((timeStatus() != timeNotSet) || (moment != 0)) {
		String timeStr = "";
		timeStr += getTimeStr(moment);
		timeStr += " ";
		timeStr += getDateStr(moment);

		return timeStr;
	}
	else {
		return "Time not set";
	}
}

String NTPClient::getTimeDateString() {
	if (timeStatus() == timeSet) {
		return getTimeDateString(now());
	}
	return "Time not set";
}

String NTPClient::printDigits(int digits) {
	// utility for digital clock display: prints preceding colon and leading 0
	String digStr = "";

	if (digits < 10)
		digStr += '0';
	digStr += String(digits);

	return digStr;
}

bool NTPClient::summertime(int year, byte month, byte day, byte hour, byte tzHours)
// input parameters: "normal time" for year, month, day, hour and tzHours (0=UTC, 1=MEZ)
{
	if ((month<3) || (month>10)) return false; // keine Sommerzeit in Jan, Feb, Nov, Dez
	if ((month>3) && (month<10)) return true; // Sommerzeit in Apr, Mai, Jun, Jul, Aug, Sep
	if (month == 3 && (hour + 24 * day) >= (1 + tzHours + 24 * (31 - (5 * year / 4 + 4) % 7)) || month == 10 && (hour + 24 * day)<(1 + tzHours + 24 * (31 - (5 * year / 4 + 1) % 7)))
		return true;
	else
		return false;
}

time_t NTPClient::getLastBootTime() {
	if (timeStatus() == timeSet) {
		return (now() - getUptime());
	}
	return 0;
}

time_t NTPClient::getUptime()
{
	_uptime = _uptime + (millis() - _uptime);
	return _uptime / 1000;
}

time_t NTPClient::getFirstSync()
{
	if (!_firstSync) {
		if (timeStatus() == timeSet) {
			_firstSync = now() - getUptime();
		}
	}
	
	return _firstSync;
}

String NTPClient::getUptimeString() {
	uint days;
	uint8 hours;
	uint8 minutes;
	uint8 seconds;
	
	long uptime = getUptime();

	seconds = uptime % SECS_PER_MIN;
	uptime -= seconds;
	minutes = (uptime % SECS_PER_HOUR)/ SECS_PER_MIN;
	uptime -= minutes * SECS_PER_MIN;
	hours = (uptime % SECS_PER_DAY) / SECS_PER_HOUR;
	uptime -= hours * SECS_PER_HOUR;
	days = uptime / SECS_PER_DAY;

	String uptimeStr = ""; 
	char buffer[20];
	sprintf(buffer, "%d days %02d:%02d:%02d", days, hours, minutes, seconds);
	uptimeStr += buffer;

	return uptimeStr;
}

time_t NTPClient::getLastNTPSync() {
	return _lastSyncd;
}

void NTPClient::onNTPSyncEvent(onSyncEvent_t handler) {
	onSyncEvent = handler;
}

boolean NTPClient::isSummerTimePeriod(time_t moment) {
	return summertime(year(), month(), day(), hour(), getTimeZone());
}

#endif // ARDUINO_ARCH_ESP8266