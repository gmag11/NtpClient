/*
 Name:		NtpClientLib.cpp
 Created:	21/12/2015 16:26:34
 Author:	gmag11@gmail.com
 Editor:	http://www.visualmicro.com
*/

#include "NtpClientLib.h"

extern ntpClient NTPClient;

time_t ntpClient::getTime() {

	if (WiFi.status() == WL_CONNECTED) {
#ifdef DEBUG_NTPCLIENT
		Serial.print("NTP Server hostname: ");
		Serial.println(client->_ntpServerName);
		Serial.print("NTP Server IP address: ");
		Serial.println(client->_timeServerIP);
		Serial.print("Result code: ");
		Serial.print(dnsResult);
		Serial.print(" ");
		Serial.println("-- Wifi Connected. Waiting for sync");
		Serial.println("-- Transmit NTP Request");
#endif // DEBUG_NTPCLIENT
		time_t timeValue = sntp_get_current_timestamp();
		if (timeValue > 0) {
			setSyncInterval(client->_longInterval);
#ifdef DEBUG_NTPCLIENT
			Serial.println("Sync frequency set low");
#endif // DEBUG_NTPCLIENT
			client->_lastSyncd = timeValue;
#ifdef DEBUG_NTPCLIENT
			Serial.println("Succeccful NTP sync at ");
			Serial.println(client->getTimeString(client->_lastSyncd));
#endif // DEBUG_NTPCLIENT
			if (_daylight) {
				if (summertime())
			}
			return timeValue;
		} else {
#ifdef DEBUG_NTPCLIENT
			Serial.println("-- NTP Error :-(");
#endif // DEBUG_NTPCLIENT
			setSyncInterval(client->_shortInterval); // Retry connection more often
			return 0; // return 0 if unable to get the time 
		}
	} else {
#ifdef DEBUG_NTPCLIENT
		Serial.println("-- NTP Error. Not connected");
#endif // DEBUG_NTPCLIENT
		return 0;
	}
}

ntpClient::ntpClient() {
	_udpPort = DEFAULT_NTP_PORT;
	_shortInterval = DEFAULT_NTP_SHORTINTERVAL;
	_longInterval = DEFAULT_NTP_INTERVAL;
}

/*time_t ntpClient::getTimeProvider() {
	return (s_client->getTime()); //NOT WORKING. s_client NOT DEFINED IN THIS SCOPE
}*/

boolean ntpClient::begin(String ntpServerName, int timeOffset, boolean daylight) {
	if (ntpServerName!="") {
		setNtpServerName(ntpServerName);
	}	
	setTimeZone(timeOffset);
	setDayLight(daylight);
	sntp_init();
	setSyncProvider(getTime);
	setSyncInterval(_shortInterval);
#ifdef DEBUG_NTPCLIENT
	Serial.println("Time sync started");
#endif // DEBUG_NTPCLIENT
	return true;
}

boolean ntpClient::stop() {
	sntp_stop();
	setSyncProvider(NULL);
#ifdef DEBUG_NTPCLIENT
	Serial.println("Time sync disabled");
#endif // DEBUG_NTPCLIENT

	return true;
}

String ntpClient::getTimeStr(time_t moment) {
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

String ntpClient::getTimeStr() {
	return this->getTimeStr(now());
}

String ntpClient::getDateStr(time_t moment) {
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

String ntpClient::getDateStr() {
	return this->getDateStr(now());
}

String ntpClient::getTimeString(time_t moment) {
	if ((timeStatus() != timeNotSet) || (moment != 0)) {
		String timeStr = "";
		timeStr += this->getTimeStr(moment);
		timeStr += " ";
		timeStr += this->getDateStr(moment);

		return timeStr;
	} else {
		return "Time not set";
	}
}

String ntpClient::getTimeString() {
	return this->getTimeString(now());
}

String ntpClient::printDigits(int digits) {
	// utility for digital clock display: prints preceding colon and leading 0
	String digStr = "";

	if (digits < 10)
		digStr += '0';
	digStr += String(digits);

	return digStr;
}


int ntpClient::getInterval()
{
	return _longInterval;
}

int ntpClient::getShortInterval()
{
	return _shortInterval;
}

boolean ntpClient::getDayLight()
{
	return this->_daylight;
}

int ntpClient::getTimeZone()
{
	return sntp_get_timezone();
}

/*int ntpClient::getLongInterval()
{
	return _longInterval;
}*/


String ntpClient::getNtpServerName()
{
	return String(sntp_getservername(0));
}

boolean ntpClient::setNtpServerName(String ntpServerName) {
	sntp_setservername(0, ntpServerName.c_str());
#ifdef DEBUG_NTPCLIENT
	Serial.println("NTP server set to " + ntpServerName);
#endif // DEBUG_NTPCLIENT

	return true;
}

boolean ntpClient::setInterval(int interval)
{
	if (interval >= 10) {
		if (_longInterval != interval) {
			_longInterval = interval;
#ifdef DEBUG_NTPCLIENT
			Serial.println("Sync interval set to " + interval);
#endif // DEBUG_NTPCLIENT
			setSyncInterval(interval);
		}
		return true;
	}
	else
		return false;
}

boolean ntpClient::setInterval(int shortInterval, int longInterval) {
	if (shortInterval >= 10 && _longInterval >= 10) {
		_shortInterval = shortInterval;
		_longInterval = longInterval;
		if (timeStatus() == timeNotSet) {
			setSyncInterval(shortInterval);
		}
		else {
			setSyncInterval(longInterval);
		}
#ifdef DEBUG_NTPCLIENT
		Serial.print("Short sync interval set to "); Serial.println(shortInterval);
		Serial.print("Long sync interval set to "); Serial.println(longInterval);
#endif // DEBUG_NTPCLIENT
		return true;
	}
	else
		return false;
}

boolean ntpClient::setTimeZone(int timeZone)
{
	if (timeZone >= -11 || timeZone <= 13) {
		sntp_stop();
		if (sntp_set_timezone(timeZone))
			sntp_init();
		else
			return false;
#ifdef DEBUG_NTPCLIENT
		Serial.println("Time zone set to " + _timeZone);
#endif // DEBUG_NTPCLIENT

		return true;
	}
	else
		return false;
}


void ntpClient::setDayLight(boolean daylight)
{
	_daylight = daylight;
#ifdef DEBUG_NTPCLIENT
	Serial.print("--Set daylight saving to ");
	Serial.println(daylight);

#endif // DEBUG_NTPCLIENT
}

//
// Summertime calculates the daylight saving for a given date.
//
boolean ntpClient::summertime(int year, byte month, byte day, byte hour, byte tzHours)
// input parameters: "normal time" for year, month, day, hour and tzHours (0=UTC, 1=MEZ)
{
	if (month<3 || month>10) return false; // keine Sommerzeit in Jan, Feb, Nov, Dez
	if (month>3 && month<10) return true; // Sommerzeit in Apr, Mai, Jun, Jul, Aug, Sep
	if (month == 3 && (hour + 24 * day) >= (1 + tzHours + 24 * (31 - (5 * year / 4 + 4) % 7)) || month == 10 && (hour + 24 * day)<(1 + tzHours + 24 * (31 - (5 * year / 4 + 1) % 7)))
		return true;
	else
		return false;
}

time_t ntpClient::getLastNTPSync() {
	return _lastSyncd;
}

