/*
 Name:		NtpClientLib.cpp
 Created:	21/12/2015 16:26:34
 Author:	gmag11@gmail.com
 Editor:	http://www.visualmicro.com
*/

#include "NtpClientLib.h"

time_t ntpClient::getNtpTime() {
	if (WiFi.status() == WL_CONNECTED) {
#ifdef DEBUG
		Serial.println("Starting UDP");
#endif
		_udp.begin(_udpPort);
#ifdef DEBUG
		Serial.print("Local port: ");
		Serial.println(_udp.localPort());
#endif
		while (_udp.parsePacket() > 0); // discard any previously received packets
		Serial.print("NTP Server hostname: ");
		uint8_t dnsResult = WiFi.hostByName(_ntpServerName, _timeServerIP);
		Serial.print(dnsResult);
		Serial.print(" ");
		Serial.println(_timeServerIP);
		Serial.println("-- Wifi Connected. Waiting for sync");
#ifdef DEBUG
		Serial.println("-- Transmit NTP Request");
#endif
		if (dnsResult != 0) {
			sendNTPpacket(_timeServerIP);
			uint32_t beginWait = millis();
			while (millis() - beginWait < 1500) {
				int size = _udp.parsePacket();
				if (size >= NTP_PACKET_SIZE) {
#ifdef DEBUG
					Serial.println("-- Receive NTP Response");
#endif
					_udp.read(_ntpPacketBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
					time_t timeValue = decodeNtpMessage(_ntpPacketBuffer);
					setSyncInterval(/*NTP_SYNC_PERIOD*/86000);
					Serial.println("Sync Period set low");
					_udp.stop();

					return timeValue;
				}
			}
			Serial.println("-- No NTP Response :-(");
			_udp.stop();

			return 0; // return 0 if unable to get the time 
		}
		else {
			Serial.println("-- Invalid address :-((");
			_udp.stop();

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

ntpClient::ntpClient(int udpPort, String ntpServerName) {
	_udpPort = udpPort;
	memset(_ntpServerName, 0, NTP_SERVER_NAME_SIZE);
	memset(_ntpPacketBuffer, 0, NTP_PACKET_SIZE);
	//_ntpServerName = ntpServerName.c_str();
	ntpServerName.toCharArray(_ntpServerName, NTP_SERVER_NAME_SIZE);
	Serial.print("ntpClient instance created: ");
	Serial.println(_ntpServerName);
	_interval = DEFAULT_NTP_INTERVAL;
	_timeZone = DEFAULT_NTP_TIMEZONE;
}

boolean ntpClient::begin() {
	setSyncProvider(ntpClient::getNtpTime); //NOT WORKING, FAIL TO COMPILE
	//setSyncInterval(_interval); //TODO
}

boolean ntpClient::stop() {
	setSyncProvider((time_t)0);
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
	return secsSince1900 - SEVENTY_YEARS + _timeZone * SECS_PER_HOUR;
}

void ntpClient::nullSyncProvider() {

}

/*time_t ntpClient::_getNtpTime() {
return this->getNtpTime();
}*/

/*String ntpClient::getTimeString() {
String timeStr = "";
timeStr += String(hour());
timeStr += ":";
timeStr += printDigits(minute());
timeStr += ":";
timeStr += printDigits(second());
timeStr += " ";
timeStr += printDigits(day());
timeStr += "/";
timeStr += printDigits(month());
timeStr += "/";
timeStr += String(year());

return timeStr;
}*/

String ntpClient::printDigits(int digits) {
	// utility for digital clock display: prints preceding colon and leading 0
	String digStr = "";

	if (digits < 10)
		digStr += '0';
	digStr += String(digits);

	return digStr;
}


boolean ntpClient::setUdpPort(int port)
{
	if (port > 0 || port < 65535) {
		_udpPort = port;
		return true;
	}
	else
		return false;
}

int ntpClient::getUdpPort()
{
	return _udpPort;
}

boolean ntpClient::setInterval(int interval)
{
	if (interval > 15) {
		if (_interval != interval) {
			_interval = interval;
			setSyncInterval(_interval);
		}
		return true;
	}
	else
		return false;
}

int ntpClient::getInterval()
{
	return _interval;
}

boolean ntpClient::setNtpServerName(String ntpServerName) {
	memset(_ntpServerName, 0, NTP_SERVER_NAME_SIZE);
	ntpServerName.toCharArray(_ntpServerName, NTP_SERVER_NAME_SIZE);
	return true;
}

String ntpClient::getNtpServerName()
{
	return String(_ntpServerName);
}

boolean ntpClient::setTimeZone(int timeZone)
{
	if (timeZone >= -13 || timeZone <= 13) {
		_timeZone = timeZone;
		return true;
	}
	else
		return false;
}

int ntpClient::getTimeZone()
{
	return _timeZone;
}

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

