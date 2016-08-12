/*
 Name:		NtpClientLib.cpp
 Created:	21/12/2015 16:26:34
 Author:	gmag11@gmail.com
 Editor:	http://www.visualmicro.com
*/

#ifdef ARDUINO_ARCH_AVR

#include "AvrNTPClient.h"

#define DBG_PORT Serial

AvrNTPClient NTP;

/*ntpClient *ntpClient::getInstance(String ntpServerName, int timeOffset, boolean daylight) {
	if (!instanceFlag) {
		s_client = new ntpClient(ntpServerName,timeOffset,daylight);
		
		instanceFlag = true;
		return s_client;
	} else {
		s_client->setNtpServerName(ntpServerName);
		s_client->setTimeZone(timeOffset);
		s_client->setDayLight(daylight);
		
		return s_client;
	}
}*/

#if NETWORK_TYPE == NETWORK_W5100
/**
* Starts a NTP time request to server. Returns a time in UNIX time format. Normally only called from library.
* @param[out] Time in UNIX time format. Seconds since 1st january 1970.
*/
time_t getTime() {
	ntpClient *client = s_client;
	DNSClient dns;

#ifdef DEBUG_NTPCLIENT
	Serial.println("Starting UDP");
#endif // DEBUG_NTPCLIENT
	s_client->_udp.begin(DEFAULT_NTP_PORT);
#ifdef DEBUG_NTPCLIENT
	Serial.print("Local port: ");
	Serial.println(client->_udp.localPort());
#endif // DEBUG_NTPCLIENT
	while (client->_udp.parsePacket() > 0); // discard any previously received packets
	dns.begin(Ethernet.dnsServerIP());
	uint8_t dnsResult = dns.getHostByName(client->_ntpServerName, client->_timeServerIP);
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
	if (dnsResult == 1) { //If DNS lookup resulted ok
		client->sendNTPpacket(client->_timeServerIP);
		uint32_t beginWait = millis();
		while (millis() - beginWait < 1500) {
			int size = client->_udp.parsePacket();
			if (size >= NTP_PACKET_SIZE) {
#ifdef DEBUG_NTPCLIENT
				Serial.println("-- Receive NTP Response");
#endif // DEBUG_NTPCLIENT
				client->_udp.read(client->_ntpPacketBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
				time_t timeValue = client->decodeNtpMessage(client->_ntpPacketBuffer);
				setSyncInterval(client->_longInterval);
#ifdef DEBUG_NTPCLIENT
				Serial.println("Sync frequency set low");
#endif // DEBUG_NTPCLIENT
				client->_udp.stop();
				client->_lastSyncd = timeValue;
#ifdef DEBUG_NTPCLIENT
				Serial.println("Succeccful NTP sync at ");
				Serial.println(client->getTimeString(client->_lastSyncd));
#endif // DEBUG_NTPCLIENT
				return timeValue;
			}
		}
#ifdef DEBUG_NTPCLIENT
		Serial.println("-- No NTP Response :-(");
#endif // DEBUG_NTPCLIENT
		client->_udp.stop();
		setSyncInterval(client->_shortInterval); // Retry connection more often
		return 0; // return 0 if unable to get the time 
	}
	else {
#ifdef DEBUG_NTPCLIENT
		Serial.println("-- Invalid address :-((");
#endif // DEBUG_NTPCLIENT
		client->_udp.stop();

		return 0; // return 0 if unable to get the time 
	}
}

#endif //NETWORK_TYPE == W5100


AvrNTPClient::AvrNTPClient() {
	/*_udpPort = DEFAULT_NTP_PORT;
	memset(_ntpServerName, 0, NTP_SERVER_NAME_SIZE); //Initialize ntp server name char[]
	memset(_ntpPacketBuffer, 0, NTP_PACKET_SIZE); //Initialize packet buffer[]
	ntpServerName.toCharArray(_ntpServerName, NTP_SERVER_NAME_SIZE);
#ifdef DEBUG_NTPCLIENT
	Serial.print("ntpClient instance created: ");
	Serial.println(_ntpServerName);
#endif // DEBUG_NTPCLIENT
	_shortInterval = DEFAULT_NTP_SHORTINTERVAL;
	_longInterval = DEFAULT_NTP_INTERVAL;
	if (timeOffset >= -12 && timeOffset <= 12)
		_timeZone = timeOffset;
	else
		_timeZone = 0;
	_daylight = daylight;
	_lastSyncd = 0;
	s_client = this;*/
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
	_lastSyncd = 0;

	if (!setInterval(DEFAULT_NTP_SHORTINTERVAL, DEFAULT_NTP_INTERVAL)) {
		return false;
}
#ifdef DEBUG_NTPCLIENT
	DBG_PORT.println("Time sync started");
#endif // DEBUG_NTPCLIENT

	setSyncInterval(getShortInterval());
	setSyncProvider(getTime);

	return true;
}

boolean AvrNTPClient::stop() {
	setSyncProvider(NULL);
#ifdef DEBUG_NTPCLIENT
	Serial.println("Time sync disabled");
#endif // DEBUG_NTPCLIENT

	return true;
}

time_t AvrNTPClient::decodeNtpMessage(byte *messageBuffer) {
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
#ifdef DEBUG_NTPCLIENT
			Serial.println("Summer Time");
#endif // DEBUG_NTPCLIENT		
		}
#ifdef DEBUG_NTPCLIENT
		else {
			Serial.println("Winter Time");
		}
#endif // DEBUG_NTPCLIENT		
	}
#ifdef DEBUG_NTPCLIENT

	else {
		Serial.println("No daylight");

	}
#endif // DEBUG_NTPCLIENT		
	return timeTemp;
}

/*void ntpClient::nullSyncProvider() {

}*/

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
	else return "Time not set";
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
	else return "Date not set";
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
		return "Time not set";
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
#ifdef DEBUG_NTPCLIENT
	Serial.printf("NTP server set to %s" + name);
#endif // DEBUG_NTPCLIENT
	free(_ntpServerName);
	_ntpServerName = name;
	return true;
}

boolean AvrNTPClient::setInterval(int interval)
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

boolean AvrNTPClient::setInterval(int shortInterval, int longInterval) {
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


boolean AvrNTPClient::setTimeZone(int timeZone)
{
	if (timeZone >= -11 || timeZone <= 13) {
		_timeZone = timeZone;
#ifdef DEBUG_NTPCLIENT
		Serial.println("Time zone set to " + _timeZone);
#endif // DEBUG_NTPCLIENT

		return true;
	}
	else
		return false;
}

void AvrNTPClient::setDayLight(boolean daylight)
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

// send an NTP request to the time server at the given address
boolean AvrNTPClient::sendNTPpacket(IPAddress &address) {
	// set all bytes in the buffer to 0
	memset(_ntpPacketBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	_ntpPacketBuffer[0] = 0b11100011;   // LI, Version, Mode
	_ntpPacketBuffer[1] = 0;     // Stratum, or type of clock
	_ntpPacketBuffer[2] = 6;     // Polling Interval
	_ntpPacketBuffer[3] = 0xEC;  // Peer Clock Precision
								 // 8 bytes of zero for Root Delay & Root Dispersion
	_ntpPacketBuffer[12] = 49;
	_ntpPacketBuffer[13] = 0x4E;
	_ntpPacketBuffer[14] = 49;
	_ntpPacketBuffer[15] = 52;
	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp: 
	_udp.beginPacket(address, 123); //NTP requests are to port 123
	_udp.write(_ntpPacketBuffer, NTP_PACKET_SIZE);
	_udp.endPacket();
	return true;
}

time_t AvrNTPClient::getLastNTPSync() {
	return _lastSyncd;
}

#endif // ARDUINO_ARCH_AVR