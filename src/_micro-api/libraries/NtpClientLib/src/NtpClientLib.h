/*
 Name:		NtpClientLib.h
 Created:	21/12/2015 16:26:34
 Author:	gmag11@gmail.com
 Editor:	http://www.visualmicro.com
*/

#ifndef _NtpClientLib_h
#define _NtpClientLib_h

#define DEBUG

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
	ntpClient(const ntpClient& s) {};

	static ntpClient* Instance();
	

	virtual ~ntpClient() {};

	static boolean begin() { //Starts NTP Sync
		if (s_client != NULL) {
			setSyncProvider(s_client->getNtpTime); //NOT WORKING, FAIL TO COMPILE
			//setSyncInterval(_interval); //TODO
			return true;
		} else 
			return false;
	}
	
	static boolean stop() { //Stops NTP Sync
		setSyncProvider(NULL);
		return true;
	}

	time_t getNtpTime2(); //Starts a NTP time request to server. Returns a time in UNIX time format

	//String getTimeString();

	boolean	setUdpPort(int port);
	int		getUdpPort();

	boolean setInterval(int interval);
	int		getInterval();

	boolean	setNtpServerName(String ntpServerName);
	String getNtpServerName();

	boolean setTimeZone(int timeZone);
	int getTimeZone();

	static time_t getNtpTime() {
		return (s_client->getNtpTime()); //NOT WORKING. s_client NOT DEFINED IN THIS SCOPE
	}

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

	static ntpClient* s_client;

	static void DestroyNtpClient() {
		if (s_client != NULL)
			delete s_client;
	}
};

#endif

