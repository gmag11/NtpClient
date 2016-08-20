/*
 Name:		NtpClientLib.cpp
 Created:	21/12/2015 16:26:34
 Author:	gmag11@gmail.com
 Editor:	http://www.visualmicro.com
*/

#ifdef ARDUINO_ARCH_AVR

#include <AvrNTPClient.h>

#define DBG_PORT Serial

#ifdef DEBUG_NTPCLIENT
#define DEBUGLOG(A) DBG_PORT.print(A)
#define DEBUGLOGCR(A) DBG_PORT.println(A)
#else
#define DEBUGLOG(A)
#define DEBUGLOGCR(A)
#endif


AvrNTPClient NTP;

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

#endif //NETWORK_TYPE == W5100

AvrNTPClient::AvrNTPClient() {
}

boolean AvrNTPClient::begin(String ntpServerName = DEFAULT_NTP_SERVER, int timeOffset = DEFAULT_NTP_TIMEZONE, boolean daylight = false) {
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

boolean AvrNTPClient::stop() {
	setSyncProvider(NULL);
	DEBUGLOGCR(F("Time sync disabled"));

	return true;
}

time_t AvrNTPClient::decodeNtpMessage(char *messageBuffer) {
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

String AvrNTPClient::getTimeStr(time_t moment) {
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

String AvrNTPClient::getTimeStr() {
	return getTimeStr(now());
}

String AvrNTPClient::getDateStr(time_t moment) {
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

String AvrNTPClient::getDateStr() {
	return getDateStr(now());
}

String AvrNTPClient::getTimeDateString(time_t moment) {
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

String AvrNTPClient::getTimeDateString() {
	return getTimeDateString(now());
}

String AvrNTPClient::printDigits(int digits) {
	// utility for digital clock display: prints preceding colon and leading 0
	String digStr = "";

	if (digits < 10)
		digStr += '0';
	digStr += String(digits);

	return digStr;
}

int AvrNTPClient::getInterval()
{
	return _longInterval;
}

int AvrNTPClient::getShortInterval()
{
	return _shortInterval;
}

boolean AvrNTPClient::getDayLight()
{
	return this->_daylight;
}

int AvrNTPClient::getTimeZone()
{
	return _timeZone;
}

String AvrNTPClient::getNtpServerName()
{
	return String(_ntpServerName);
}

boolean AvrNTPClient::setNtpServerName(String ntpServerName) {
	char * name = (char *)malloc((ntpServerName.length() + 1) * sizeof(char));
	ntpServerName.toCharArray(name, ntpServerName.length() + 1);
	DEBUGLOG(F("NTP server set to "));
	DEBUGLOGCR(name);
	free(_ntpServerName);
	_ntpServerName = name;
	return true;
}

boolean AvrNTPClient::setInterval(int interval)
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

boolean AvrNTPClient::setInterval(int shortInterval, int longInterval) {
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


boolean AvrNTPClient::setTimeZone(int timeZone)
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

void AvrNTPClient::setDayLight(boolean daylight)
{
	_daylight = daylight;
	DEBUGLOG(F("--Set daylight saving to "));
	DEBUGLOGCR(daylight);
}

//
// Summertime calculates the daylight saving for a given date.
//
boolean AvrNTPClient::summertime(int year, byte month, byte day, byte hour, byte tzHours)
// input parameters: "normal time" for year, month, day, hour and tzHours (0=UTC, 1=MEZ)
{
	if (month<3 || month>10) return false; // keine Sommerzeit in Jan, Feb, Nov, Dez
	if (month>3 && month<10) return true; // Sommerzeit in Apr, Mai, Jun, Jul, Aug, Sep
	if (month == 3 && (hour + 24 * day) >= (1 + tzHours + 24 * (31 - (5 * year / 4 + 4) % 7)) || month == 10 && (hour + 24 * day)<(1 + tzHours + 24 * (31 - (5 * year / 4 + 1) % 7)))
		return true;
	else
		return false;
}

time_t AvrNTPClient::getLastNTPSync() {
	return _lastSyncd;
}

void AvrNTPClient::setLastNTPSync(time_t moment) {
	_lastSyncd = moment;
}

time_t AvrNTPClient::getUptime()
{
	_uptime = _uptime + (millis() - _uptime); // Add time since last getUptime call
	return _uptime / 1000;
}

String AvrNTPClient::getUptimeString() {
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

time_t AvrNTPClient::getFirstSync()
{
	if (!_firstSync) {
		if (timeStatus() == timeSet) {
			_firstSync = now() - getUptime();
		}
	}

	return _firstSync;
}

#endif // ARDUINO_ARCH_AVR