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
/*
 Name:		NtpClientLib.cpp
 Created:	21/12/2015 16:26:34
 Author:	gmag11@gmail.com
 Editor:	http://www.visualmicro.com
*/

#include "NtpClientLib.h"

#if defined ARDUINO_ARCH_AVR || defined ARDUINO_ARCH_SAMD || defined ARDUINO_ARCH_ARC32

#define DBG_PORT Serial

#ifdef DEBUG_NTPCLIENT
#define DEBUGLOG(A) DBG_PORT.print(A)
#define DEBUGLOGCR(A) DBG_PORT.println(A)
#else
#define DEBUGLOG(A)
#define DEBUGLOGCR(A)
#endif


NTPClient NTP;

#if NETWORK_TYPE == NETWORK_W5100
// send an NTP request to the time server at the given address
boolean sendNTPpacket(IPAddress &address, EthernetUDP udp) {
	char ntpPacketBuffer[NTP_PACKET_SIZE]; //Buffer to store request message

	// set all bytes in the buffer to 0
	memset(ntpPacketBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	ntpPacketBuffer[0] = 0b11100011;   // LI, Version, Mode
	ntpPacketBuffer[1] = 0;     // Stratum, or type of clock
	ntpPacketBuffer[2] = 6;     // Polling Interval
	ntpPacketBuffer[3] = 0xEC;  // Peer Clock Precision
								// 8 bytes of zero for Root Delay & Root Dispersion
	ntpPacketBuffer[12] = 49;
	ntpPacketBuffer[13] = 0x4E;
	ntpPacketBuffer[14] = 49;
	ntpPacketBuffer[15] = 52;
	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp: 
	udp.beginPacket(address, 123); //NTP requests are to port 123
	udp.write(ntpPacketBuffer, NTP_PACKET_SIZE);
	udp.endPacket();
	return true;
}

/**
* Starts a NTP time request to server. Returns a time in UNIX time format. Normally only called from library.
* @param[out] Time in UNIX time format. Seconds since 1st january 1970.
*/
time_t getTime() {
	DNSClient dns;
	EthernetUDP udp;
	IPAddress timeServerIP; //NTP server IP address
	char ntpPacketBuffer[NTP_PACKET_SIZE]; //Buffer to store response message


	DEBUGLOGCR(F("Starting UDP"));
	udp.begin(DEFAULT_NTP_PORT);
	DEBUGLOG(F("Remote port: "));
	DEBUGLOGCR(udp.remotePort());
	while (udp.parsePacket() > 0); // discard any previously received packets
	dns.begin(Ethernet.dnsServerIP());
	uint8_t dnsResult = dns.getHostByName(NTP.getNtpServerName().c_str(), timeServerIP);
	DEBUGLOG(F("NTP Server hostname: "));
	DEBUGLOGCR(NTP.getNtpServerName());
	DEBUGLOG(F("NTP Server IP address: "));
	DEBUGLOGCR(timeServerIP);
	DEBUGLOG(F("Result code: "));
	DEBUGLOG(dnsResult);
	DEBUGLOG(" ");
	DEBUGLOGCR(F("-- IP Connected. Waiting for sync"));
	DEBUGLOGCR(F("-- Transmit NTP Request"));

	if (dnsResult == 1) { //If DNS lookup resulted ok
		sendNTPpacket(timeServerIP,udp);
		uint32_t beginWait = millis();
		while (millis() - beginWait < 1500) {
			int size = udp.parsePacket();
			if (size >= NTP_PACKET_SIZE) {
				DEBUGLOGCR(F("-- Receive NTP Response"));
				udp.read(ntpPacketBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
				time_t timeValue = NTP.decodeNtpMessage(ntpPacketBuffer);
				setSyncInterval(NTP.getLongInterval());
				NTP.getFirstSync(); // Set firstSync value if not set before
				DEBUGLOGCR(F("Sync frequency set low"));
				udp.stop();
				NTP.setLastNTPSync(timeValue);
				DEBUGLOG(F("Succeccful NTP sync at "));
				DEBUGLOGCR(NTP.getTimeDateString(NTP.getLastNTPSync()));

				return timeValue;
			}
		}
		DEBUGLOGCR(F("-- No NTP Response :-("));
		udp.stop();
		setSyncInterval(NTP.getShortInterval()); // Retry connection more often
		return 0; // return 0 if unable to get the time 
	}
	else {
		DEBUGLOGCR(F("-- Invalid address :-(("));
		udp.stop();
		return 0; // return 0 if unable to get the time 
	}
}
#elif NETWORK_TYPE == NETWORK_WIFI101
// send an NTP request to the time server at the given address
boolean sendNTPpacket(const char* address, WiFiUDP udp) {
	char ntpPacketBuffer[NTP_PACKET_SIZE]; //Buffer to store request message

										   // set all bytes in the buffer to 0
	memset(ntpPacketBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	ntpPacketBuffer[0] = 0b11100011;   // LI, Version, Mode
	ntpPacketBuffer[1] = 0;     // Stratum, or type of clock
	ntpPacketBuffer[2] = 6;     // Polling Interval
	ntpPacketBuffer[3] = 0xEC;  // Peer Clock Precision
								// 8 bytes of zero for Root Delay & Root Dispersion
	ntpPacketBuffer[12] = 49;
	ntpPacketBuffer[13] = 0x4E;
	ntpPacketBuffer[14] = 49;
	ntpPacketBuffer[15] = 52;
	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp: 
	udp.beginPacket(address, 123); //NTP requests are to port 123
	udp.write(ntpPacketBuffer, NTP_PACKET_SIZE);
	udp.endPacket();
	return true;
}

/**
* Starts a NTP time request to server. Returns a time in UNIX time format. Normally only called from library.
* @param[out] Time in UNIX time format. Seconds since 1st january 1970.
*/
time_t getTime() {
	DNSClient dns;
	WiFiUDP udp;
	IPAddress timeServerIP; //NTP server IP address
	char ntpPacketBuffer[NTP_PACKET_SIZE]; //Buffer to store response message


	DEBUGLOGCR(F("Starting UDP"));
	udp.begin(DEFAULT_NTP_PORT);
	DEBUGLOG(F("Remote port: "));
	DEBUGLOGCR(udp.remotePort());
	while (udp.parsePacket() > 0); // discard any previously received packets
	/*dns.begin(WiFi.dnsServerIP());
	uint8_t dnsResult = dns.getHostByName(NTP.getNtpServerName().c_str(), timeServerIP);
	DEBUGLOG(F("NTP Server hostname: "));
	DEBUGLOGCR(NTP.getNtpServerName());
	DEBUGLOG(F("NTP Server IP address: "));
	DEBUGLOGCR(timeServerIP);
	DEBUGLOG(F("Result code: "));
	DEBUGLOG(dnsResult);
	DEBUGLOG(" ");
	DEBUGLOGCR(F("-- IP Connected. Waiting for sync"));
	DEBUGLOGCR(F("-- Transmit NTP Request"));*/

	//if (dnsResult == 1) { //If DNS lookup resulted ok
	sendNTPpacket(NTP.getNtpServerName().c_str(), udp);
	uint32_t beginWait = millis();
	while (millis() - beginWait < 1500) {
		int size = udp.parsePacket();
		if (size >= NTP_PACKET_SIZE) {
			DEBUGLOGCR(F("-- Receive NTP Response"));
			udp.read(ntpPacketBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
			time_t timeValue = NTP.decodeNtpMessage(ntpPacketBuffer);
			setSyncInterval(NTP.getLongInterval());
			NTP.getFirstSync(); // Set firstSync value if not set before
			DEBUGLOGCR(F("Sync frequency set low"));
			udp.stop();
			NTP.setLastNTPSync(timeValue);
			DEBUGLOG(F("Succeccful NTP sync at "));
			DEBUGLOGCR(NTP.getTimeDateString(NTP.getLastNTPSync()));

			return timeValue;
		}
	}
	DEBUGLOGCR(F("-- No NTP Response :-("));
	udp.stop();
	setSyncInterval(NTP.getShortInterval()); // Retry connection more often
	return 0; // return 0 if unable to get the time 
}

#endif //NETWORK_TYPE

NTPClient::NTPClient() {
}

bool NTPClient::begin(String ntpServerName, int timeOffset, bool daylight) {
	if (!setNtpServerName(ntpServerName)) {
		return false;
	}
	if (!setTimeZone(timeOffset)) {
		return false;
	}
	//sntp_init();
	setDayLight(daylight);

	if (!setInterval(DEFAULT_NTP_SHORTINTERVAL, DEFAULT_NTP_INTERVAL)) {
		return false;
}
	DEBUGLOGCR(F("Time sync started"));

	setSyncInterval(getShortInterval());
	setSyncProvider(getTime);

	return true;
}

boolean NTPClient::stop() {
	setSyncProvider(NULL);
	DEBUGLOGCR(F("Time sync disabled"));

	return true;
}

time_t NTPClient::decodeNtpMessage(char *messageBuffer) {
	unsigned long secsSince1900;
	// convert four bytes starting at location 40 to a long integer
	secsSince1900 = (unsigned long)messageBuffer[40] << 24;
	secsSince1900 |= (unsigned long)messageBuffer[41] << 16;
	secsSince1900 |= (unsigned long)messageBuffer[42] << 8;
	secsSince1900 |= (unsigned long)messageBuffer[43];

#define SEVENTY_YEARS 2208988800UL
	time_t timeTemp = secsSince1900 - SEVENTY_YEARS + _timeZone * SECS_PER_HOUR;

	if (_daylight) {
		if (summertime(year(timeTemp), month(timeTemp), day(timeTemp), hour(timeTemp), _timeZone)) {
			timeTemp += SECS_PER_HOUR;
			DEBUGLOGCR(F("Summer Time"));
		}
		else {
			DEBUGLOGCR(F("Winter Time"));
		}
	}
	else {
		DEBUGLOGCR(F("No daylight"));
	}
	return timeTemp;
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
	else return F("Time not set");
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
	else return F("Date not set");
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
	} else {
		return F("Time not set");
	}
}

String NTPClient::getTimeDateString() {
	return getTimeDateString(now());
}

String NTPClient::printDigits(int digits) {
	// utility for digital clock display: prints preceding colon and leading 0
	String digStr = "";

	if (digits < 10)
		digStr += '0';
	digStr += String(digits);

	return digStr;
}

int NTPClient::getInterval()
{
	return _longInterval;
}

int NTPClient::getShortInterval()
{
	return _shortInterval;
}

boolean NTPClient::getDayLight()
{
	return this->_daylight;
}

int NTPClient::getTimeZone()
{
	return _timeZone;
}

String NTPClient::getNtpServerName()
{
	return String(_ntpServerName);
}

boolean NTPClient::setNtpServerName(String ntpServerName) {
	char * name = (char *)malloc((ntpServerName.length() + 1) * sizeof(char));
	ntpServerName.toCharArray(name, ntpServerName.length() + 1);
	DEBUGLOG(F("NTP server set to "));
	DEBUGLOGCR(name);
	free(_ntpServerName);
	_ntpServerName = name;
	return true;
}

boolean NTPClient::setInterval(int interval)
{
	if (interval >= 10) {
		if (_longInterval != interval) {
			_longInterval = interval;
			DEBUGLOG(F("Sync interval set to "));
			DEBUGLOGCR(interval);
			if(timeStatus() != timeSet)
				setSyncInterval(interval);
		}
		return true;
	}
	else
		return false;
}

boolean NTPClient::setInterval(int shortInterval, int longInterval) {
	if (shortInterval >= 10 && _longInterval >= 10) {
		_shortInterval = shortInterval;
		_longInterval = longInterval;
		if (timeStatus() != timeSet) {
			setSyncInterval(shortInterval);
		}
		else {
			setSyncInterval(longInterval);
		}
		DEBUGLOG(F("Short sync interval set to ")); DEBUGLOGCR(shortInterval);
		DEBUGLOG(F("Long sync interval set to ")); DEBUGLOGCR(longInterval);
		return true;
	}
	else
		return false;
}


boolean NTPClient::setTimeZone(int timeZone)
{
	if (timeZone >= -11 || timeZone <= 13) {
		_timeZone = timeZone;
		DEBUGLOGCR(F("Time zone set to "));
		DEBUGLOGCR(_timeZone);
		return true;
	}
	else
		return false;
}

void NTPClient::setDayLight(boolean daylight)
{
	_daylight = daylight;
	DEBUGLOG(F("--Set daylight saving to "));
	DEBUGLOGCR(daylight);
}

//
// Summertime calculates the daylight saving for a given date.
//
boolean NTPClient::summertime(int year, byte month, byte day, byte hour, byte tzHours)
// input parameters: "normal time" for year, month, day, hour and tzHours (0=UTC, 1=MEZ)
{
	if (month<3 || month>10) return false; // keine Sommerzeit in Jan, Feb, Nov, Dez
	if (month>3 && month<10) return true; // Sommerzeit in Apr, Mai, Jun, Jul, Aug, Sep
	if (month == 3 && (hour + 24 * day) >= (1 + tzHours + 24 * (31 - (5 * year / 4 + 4) % 7)) || month == 10 && (hour + 24 * day)<(1 + tzHours + 24 * (31 - (5 * year / 4 + 1) % 7)))
		return true;
	else
		return false;
}

time_t NTPClient::getLastNTPSync() {
	return _lastSyncd;
}

void NTPClient::setLastNTPSync(time_t moment) {
	_lastSyncd = moment;
}

time_t NTPClient::getLastBootTime() {
	if (timeStatus() == timeSet) {
		return (now() - getUptime());
	}
	return 0;
}

time_t NTPClient::getUptime()
{
	_uptime = _uptime + (millis() - _uptime); // Add time since last getUptime call
	return _uptime / 1000;
}

String NTPClient::getUptimeString() {
	unsigned int days;
	unsigned char hours;
	unsigned char minutes;
	unsigned char seconds;

	long uptime = getUptime();

	seconds = uptime % SECS_PER_MIN;
	uptime -= seconds;
	minutes = (uptime % SECS_PER_HOUR) / SECS_PER_MIN;
	uptime -= minutes * SECS_PER_MIN;
	hours = (uptime % SECS_PER_DAY) / SECS_PER_HOUR;
	uptime -= hours * SECS_PER_HOUR;
	days = uptime / SECS_PER_DAY;

	String uptimeStr = "";
	char buffer[20];
	sprintf(buffer, "%4d days %02d:%02d:%02d", days, hours, minutes, seconds);
	uptimeStr += buffer;

	return uptimeStr;
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

void NTPClient::onNTPSyncEvent(onSyncEvent_t handler) {
	onSyncEvent = handler;
}

boolean NTPClient::isSummerTimePeriod(time_t moment) {
	return summertime(year(), month(), day(), hour(), getTimeZone());
}

#endif // ARDUINO_ARCH_AVR