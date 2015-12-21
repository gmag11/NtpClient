/*
 Name:		NtpClientLib.h
 Created:	21/12/2015 16:26:34
 Author:	gmag11@gmail.com
 Editor:	http://www.visualmicro.com
*/

#ifndef _NtpClientLib_h
#define _NtpClientLib_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <Time2.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>

//#include <Udp.h>

#define DEFAULT_NTP_SERVER "pool.ntp.org"
#define DEFAULT_NTP_PORT 123
#define DEFAULT_NTP_INTERVAL 600
#define DEFAULT_NTP_TIMEZONE 1

#define NTP_SERVER_NAME_SIZE 60

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message


class ntpClient {
	
public:
	ntpClient() {
		ntpClient(DEFAULT_NTP_PORT, DEFAULT_NTP_SERVER);
	}

	ntpClient(int udpPort) {
		ntpClient(udpPort, DEFAULT_NTP_SERVER);
	}

	ntpClient(String ntpServerName) {
		ntpClient(DEFAULT_NTP_PORT, ntpServerName);
	}

	ntpClient(int udpPort, String ntpServerName);

	//~ntpClient ();

	boolean begin(); //Starts NTP Sync

	boolean stop(); //Stops NTP Sync

	time_t getNtpTime(); //Starts a NTP time request to server. Returns a time in UNIX time format

	//String getTimeString();

	boolean	setUdpPort(int port);
	int		getUdpPort();

	boolean setInterval(int interval);
	int		getInterval();

	boolean	setNtpServerName(String ntpServerName);
	String getNtpServerName();

	boolean setTimeZone(int timeZone);
	int getTimeZone();

protected:
	
	boolean sendNTPpacket(IPAddress &address);
	time_t decodeNtpMessage(byte *messageBuffer);
	void nullSyncProvider();
	//time_t _getNtpTime();
	String printDigits(int digits);

private:

	int _udpPort; //UDP port number to send request from
	char _ntpServerName[NTP_SERVER_NAME_SIZE]; //NTP server name
	IPAddress _timeServerIP; //NTP server IP address

	int _timeZone; //Local time zone. Added to NTP time
	WiFiUDP _udp; //UDP port object
	byte _ntpPacketBuffer[NTP_PACKET_SIZE]; //Bffer to store request and response messages
	int _interval; //Interval to set periodic time sync


};

#endif

