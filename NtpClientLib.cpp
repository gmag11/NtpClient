/*
 Name:		NtpClientLib.cpp
 Created:	21/12/2015 16:26:34
 Author:	gmag11@gmail.com
 Editor:	http://www.visualmicro.com
*/

#include "NtpClientLib.h"

boolean ntpClient::instanceFlag = false;
ntpClient *ntpClient::s_client = NULL;

extern ntpClient client;

ntpClient *ntpClient::getInstance(String ntpServerName, int timeOffset, boolean daylight) {
	if (!instanceFlag) {
		s_client = new ntpClient(ntpServerName,timeOffset,daylight);
		//atexit(&DestroyNtpClient);
		instanceFlag = true;
		return s_client;
	} else {
		s_client->setNtpServerName(ntpServerName);
		s_client->setTimeZone(timeOffset);
		s_client->setDayLight(daylight);
		//s_client->setUdpPort(udpPort);
		return s_client;
	}
}

/*void ntpClient::DestroyNtpClient() {
	if (s_client != NULL) delete s_client;
}*/

#ifdef NTP_TIME_SYNC
#if NETWORK_TYPE == NETWORK_ESP8266
time_t ntpClient::getTime() {
	ntpClient *client = s_client;

	if (WiFi.status() == WL_CONNECTED) {
#ifdef DEBUG
		Serial.println("Starting UDP");
#endif
		s_client->_udp.begin(DEFAULT_NTP_PORT);
#ifdef DEBUG
		Serial.print("Local port: ");
		Serial.println(client->_udp.localPort());
#endif
		while (client->_udp.parsePacket() > 0); // discard any previously received packets
		uint8_t dnsResult = WiFi.hostByName(client->_ntpServerName, client->_timeServerIP);
#ifdef DEBUG
		Serial.print("NTP Server hostname: ");
		Serial.println(client->_ntpServerName);
		Serial.print("NTP Server IP address: ");
		Serial.println(client->_timeServerIP);
		Serial.print("Result code: ");
		Serial.print(dnsResult);
		Serial.print(" ");
		Serial.println("-- Wifi Connected. Waiting for sync");
		Serial.println("-- Transmit NTP Request");
#endif //DEDUG
		if (dnsResult == 1) { //If DNS lookup resulted ok
			client->sendNTPpacket(client->_timeServerIP);
			uint32_t beginWait = millis();
			while (millis() - beginWait < 1500) {
				int size = client->_udp.parsePacket();
				if (size >= NTP_PACKET_SIZE) {
#ifdef DEBUG
					Serial.println("-- Receive NTP Response");
#endif
					client->_udp.read(client->_ntpPacketBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
					time_t timeValue = client->decodeNtpMessage(client->_ntpPacketBuffer);
					setSyncInterval(client->_longInterval);
#ifdef DEBUG
					Serial.println("Sync frequency set low");
#endif // DEBUG
					client->_udp.stop();
					client->_lastSyncd = timeValue;
#ifdef DEBUG
					Serial.printf("Succeccful NTP sync at %s", client->getTimeString(client->_lastSyncd));
#endif // DEBUG
					return timeValue;
				}
			}
#ifdef DEBUG
			Serial.println("-- No NTP Response :-(");
#endif //DEBUG
			client->_udp.stop();

			return 0; // return 0 if unable to get the time 
		}
		else {
#ifdef DEBUG
			Serial.println("-- Invalid address :-((");
#endif //DEBUG
			client->_udp.stop();

			return 0; // return 0 if unable to get the time 
		}
	}
	else {
#ifdef DEBUG
		Serial.println("-- NTP Error. Not connected");
#endif // DEBUG
		return 0;
	}
}
#endif //NETWORK_TYPE == ESP8266

#if NETWORK_TYPE == NETWORK_W5100
time_t ntpClient::getTime() {
	ntpClient *client = s_client;
	DNSClient dns;

#ifdef DEBUG
	Serial.println("Starting UDP");
#endif
	s_client->_udp.begin(DEFAULT_NTP_PORT);
#ifdef DEBUG
	Serial.print("Local port: ");
	Serial.println(client->_udp.localPort());
#endif
	while (client->_udp.parsePacket() > 0); // discard any previously received packets
	dns.begin(Ethernet.dnsServerIP());
	uint8_t dnsResult = dns.getHostByName(client->_ntpServerName, client->_timeServerIP);
#ifdef DEBUG
	Serial.print("NTP Server hostname: ");
	Serial.println(client->_ntpServerName);
	Serial.print("NTP Server IP address: ");
	Serial.println(client->_timeServerIP);
	Serial.print("Result code: ");
	Serial.print(dnsResult);
	Serial.print(" ");
	Serial.println("-- Wifi Connected. Waiting for sync");
	Serial.println("-- Transmit NTP Request");
#endif //DEDUG
	if (dnsResult == 1) { //If DNS lookup resulted ok
		client->sendNTPpacket(client->_timeServerIP);
		uint32_t beginWait = millis();
		while (millis() - beginWait < 1500) {
			int size = client->_udp.parsePacket();
			if (size >= NTP_PACKET_SIZE) {
#ifdef DEBUG
				Serial.println("-- Receive NTP Response");
#endif
				client->_udp.read(client->_ntpPacketBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
				time_t timeValue = client->decodeNtpMessage(client->_ntpPacketBuffer);
				setSyncInterval(client->_longInterval);
#ifdef DEBUG
				Serial.println("Sync frequency set low");
#endif // DEBUG
				client->_udp.stop();
				client->_lastSyncd = timeValue;
#ifdef DEBUG
				Serial.printf("Succeccful NTP sync at %s", client->getTimeString(client->_lastSyncd));
#endif // DEBUG
				return timeValue;
			}
		}
#ifdef DEBUG
		Serial.println("-- No NTP Response :-(");
#endif //DEBUG
		client->_udp.stop();

		return 0; // return 0 if unable to get the time 
	}
	else {
#ifdef DEBUG
		Serial.println("-- Invalid address :-((");
#endif //DEBUG
		client->_udp.stop();

		return 0; // return 0 if unable to get the time 
	}
}

#endif //NETWORK_TYPE == W5100

#endif // NTP_TIME_SYNC

#ifdef WEB_TIME_SYNC
time_t ntpClient::getTime() {
	ntpClient *client = s_client;
	boolean error = false;

	TimeElements time;
	//WiFiClient webClient;

	// Just choose any reasonably busy web server, the load is really low
	Serial.print("Conectando "); Serial.println(client->_ntpServerName);
	if (client->_webClient.connect(client->_ntpServerName, 80))
	{
		// Make an HTTP 1.1 request which is missing a Host: header
		// compliant servers are required to answer with an error that includes
		// a Date: header.
		Serial.println("Conectado");
		//PROGMEM const char HTTP_Request[]  = "GET / HTTP/1.1 \r\n\r\n";
		client->_webClient.print(FPSTR(HTTP_Request));
		//client.print(F("GET / HTTP/1.1 \r\n\r\n"));

		char buf[5];			// temporary buffer for characters
		client->_webClient.setTimeout(5000);
		if (client->_webClient.find((char *)"\r\nDate: ") // look for Date: header
			&& client->_webClient.readBytes(buf, 5) == 5) // discard
		{
			time.Day = client->_webClient.parseInt();	   // day
			Serial.print("Day: "); Serial.println(time.Day);
			client->_webClient.readBytes(buf, 1);	   // discard
			client->_webClient.readBytes(buf, 3);	   // month
			time.Month = 1;
			Serial.print("Month: "); Serial.println(buf[0,3]);
			time.Year = (int)(client->_webClient.parseInt() - 1970); 	   // year
			Serial.print("Year: "); Serial.println(time.Year + 1970);
			time.Hour = client->_webClient.parseInt();   // hour
			time.Hour+=client->_timeZone; //Adjust time zone
			Serial.print("Hour: "); Serial.println(time.Hour);
			time.Minute = client->_webClient.parseInt(); // minute
			Serial.print("Minute: "); Serial.println(time.Minute);
			time.Second = client->_webClient.parseInt(); // second
			Serial.print("Second: "); Serial.println(time.Second);

			//	  int daysInPrevMonths;
			switch (buf[0])
			{
			case 'F': time.Month = 2; break; // Feb
			case 'S': time.Month = 9; break; // Sep
			case 'O': time.Month = 10; break; // Oct
			case 'N': time.Month = 11; break; // Nov
			case 'D': time.Month = 12; break; // Dec
			default:
				if (buf[0] == 'J' && buf[1] == 'a')
					time.Month = 1;		// Jan
				else if (buf[0] == 'A' && buf[1] == 'p')
					time.Month = 4;		// Apr
				else switch (buf[2])
				{
				case 'r': time.Month = 3; break; // Mar
				case 'y': time.Month = 5; break; // May
				case 'n': time.Month = 6; break; // Jun
				case 'l': time.Month = 7; break; // Jul
				default: // add a default label here to avoid compiler warning
				case 'g': time.Month = 8; break; // Aug
				}
			}

		}
		else {
			Serial.println("Error datos");
			error = true;
		}

	}
	else {
		Serial.println("Error");
		error = true;
	}
	delay(10);
	client->_webClient.flush();
	client->_webClient.stop();
	if (!error)
		return makeTime(time);
	else
		return (0);
}
#endif // WEB_TIME_SYNC


ntpClient::ntpClient(String ntpServerName, int timeOffset, boolean daylight) {
	_udpPort = DEFAULT_NTP_PORT;
	memset(_ntpServerName, 0, NTP_SERVER_NAME_SIZE); //Initialize ntp server name char[]
	memset(_ntpPacketBuffer, 0, NTP_PACKET_SIZE); //Initialize packet buffer[]
	ntpServerName.toCharArray(_ntpServerName, NTP_SERVER_NAME_SIZE);
#ifdef DEBUG
	Serial.print("ntpClient instance created: ");
	Serial.println(_ntpServerName);
#endif // DEBUG
	_shortInterval = DEFAULT_NTP_SHORTINTERVAL;
	_longInterval = DEFAULT_NTP_INTERVAL;
	if (timeOffset >= -12 && timeOffset <= 12)
		_timeZone = timeOffset;
	else
		_timeZone = 0;
	_daylight = daylight;
	_lastSyncd = 0;
	s_client = this;
}

/*time_t ntpClient::getTimeProvider() {
	return (s_client->getTime()); //NOT WORKING. s_client NOT DEFINED IN THIS SCOPE
}*/

boolean ntpClient::begin() {
	setSyncProvider(getTime);
	setSyncInterval(_shortInterval);
#ifdef DEBUG
	Serial.println("Time sync started");
#endif // DEBUG
	return true;
}

boolean ntpClient::stop() {
	setSyncProvider(NULL);
#ifdef DEBUG
	Serial.println("Time sync disabled");
#endif // DEBUG

	return true;
}

time_t ntpClient::decodeNtpMessage(byte *messageBuffer) {
	unsigned long secsSince1900;
	// convert four bytes starting at location 40 to a long integer
	secsSince1900 = (unsigned long)messageBuffer[40] << 24;
	secsSince1900 |= (unsigned long)messageBuffer[41] << 16;
	secsSince1900 |= (unsigned long)messageBuffer[42] << 8;
	secsSince1900 |= (unsigned long)messageBuffer[43];

#define SEVENTY_YEARS 2208988800UL
	time_t timeTemp = secsSince1900 - SEVENTY_YEARS + _timeZone * SECS_PER_HOUR;

	if (summertime(year(timeTemp), month(timeTemp), day(timeTemp), hour(timeTemp), _timeZone)) {
		timeTemp += SECS_PER_HOUR;
#ifdef DEBUG
		Serial.println("Summer Time");
	}
	else {
		Serial.println("Winter Time");
#endif // DEBUG
	}

	return timeTemp;
}

/*void ntpClient::nullSyncProvider() {

}*/

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


/*boolean ntpClient::setUdpPort(int port)
{
	if (port > 0 || port < 65535) {
		_udpPort = port;
		return true;
	}
	else
		return false;
}*/

/*int ntpClient::getUdpPort()
{
	return _udpPort;
}*/

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
	return _timeZone;
}

/*int ntpClient::getLongInterval()
{
	return _longInterval;
}*/


String ntpClient::getNtpServerName()
{
	return String(_ntpServerName);
}

boolean ntpClient::setNtpServerName(String ntpServerName) {
	memset(_ntpServerName, 0, NTP_SERVER_NAME_SIZE);
	ntpServerName.toCharArray(_ntpServerName, NTP_SERVER_NAME_SIZE);
#ifdef DEBUG
	Serial.println("NTP server set to " + ntpServerName);
#endif // DEBUG

	return true;
}

boolean ntpClient::setInterval(int interval)
{
	if (interval >= 10) {
		if (_longInterval != interval) {
			_longInterval = interval;
#ifdef DEBUG
			Serial.println("Sync interval set to " + interval);
#endif // DEBUG
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
#ifdef DEBUG
		Serial.print("Short sync interval set to "); Serial.println(shortInterval);
		Serial.print("Long sync interval set to "); Serial.println(longInterval);
#endif // DEBUG
		return true;
	}
	else
		return false;
}


boolean ntpClient::setTimeZone(int timeZone)
{
	if (timeZone >= -13 || timeZone <= 13) {
		_timeZone = timeZone;
#ifdef DEBUG
		Serial.println("Time zone set to " + _timeZone);
#endif // DEBUG

		return true;
	}
	else
		return false;
}

void ntpClient::setDayLight(boolean daylight)
{
	this->_daylight = daylight;
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


#ifdef NTP_TIME_SYNC
// send an NTP request to the time server at the given address
boolean ntpClient::sendNTPpacket(IPAddress &address) {
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
#endif // NTP_TIME_SYNC

time_t ntpClient::getLastNTPSync() {
	return _lastSyncd;
}

