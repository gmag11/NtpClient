/*
 Name:		NtpClientLib.h
 Created:	21/12/2015 16:26:34
 Author:	gmag11@gmail.com
 Editor:	http://www.visualmicro.com
*/

#ifndef _NtpClientLib_h
#define _NtpClientLib_h

//#define DEBUG //Uncomment this to enable debug messages over serial port

#define NTP_TIME_SYNC

#ifndef NTP_TIME_SYNC
#define WEB_TIME_SYNC
#endif

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <Time2.h>
#include <ESP8266WiFi.h>
#ifdef NTP_TIME_SYNC
#include <WiFiUdp.h>
#endif // NTP_TIME_SYNC

//#include <Udp.h>

#define DEFAULT_NTP_SERVER "pool.ntp.org"
#define DEFAULT_NTP_PORT 123
#define DEFAULT_NTP_INTERVAL 60
#define DEFAULT_NTP_SHORTINTERVAL 15
#define DEFAULT_NTP_TIMEZONE 1

#define NTP_SERVER_NAME_SIZE 60

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message

#ifdef WEB_TIME_SYNC
PROGMEM const char HTTP_Request[] = "GET / HTTP/1.1 \r\n\r\n";
#endif // WEB_TIME_SYNC

class ntpClient {
	
public:
	
	/**
	* Gets ntpClient instance changing parameters if it already exists or creates it if does not.
	* @param[in] NTP server name.
	* @param[in] Time offset from UTC.
	* @param[out] Pointer to ntpClient instance
	*/
	static ntpClient* getInstance(String ntpServerName, int timeOffset);

	//virtual ~ntpClient() {};

	boolean begin(); //Starts NTP Sync

	boolean stop(); //Stops NTP Sync

	/**
	* Starts a NTP time request to server. Returns a time in UNIX time format
	* @param[out] Time in UNIX time format.
	*/
	static time_t getTime(); //Starts a NTP time request to server. Returns a time in UNIX time format
	
	/**
	* Convert current time to a String,
	* @param[out] String constructed from current time.
	*/
	String getTimeString();

	//boolean	setUdpPort(int port);
	//int		getUdpPort();

	boolean setInterval(int interval);
	boolean setInterval(int shortInterval, int longInterval);
	int		getInterval();
	int		getShortInterval();
	int		getLongInterval();

	boolean	setNtpServerName(String ntpServerName);
	String getNtpServerName();

	boolean setTimeZone(int timeZone);
	int getTimeZone();

protected:
	
	//void nullSyncProvider();
	//static time_t getTimeProvider();
	String printDigits(int digits);
	int _timeZone; //Local time zone. Added to NTP time
	static bool instanceFlag; //Flag to control that instance has been created
	static ntpClient *s_client; //pointer to this instance
	int _shortInterval; //Interval to set periodic time sync
	int _longInterval; //Interval to set periodic time sync

	/**
	* Construct NTP client to given server name.
	* @param[in] NTP server name as String.
	* @param[in] Time offset from UTC.
	*/
	ntpClient(String ntpServerName, int timeOffset);
	
	//static void DestroyNtpClient();
	//time_t getTime(); 

private:

	int _udpPort; //UDP port number to send request from
	char _ntpServerName[NTP_SERVER_NAME_SIZE]; //NTP server name
	IPAddress _timeServerIP; //NTP server IP address
#ifdef WEB_TIME_SYNC
	WiFiClient _webClient;
#endif //WEB_TIME_SYNC

#ifdef NTP_TIME_SYNC
	WiFiUDP _udp; //UDP port object  
#endif // NTP_TIME_SYNC

	byte _ntpPacketBuffer[NTP_PACKET_SIZE]; //Bffer to store request and response messages
	

	
	boolean sendNTPpacket(IPAddress &address);
	time_t decodeNtpMessage(byte *messageBuffer);

};

#endif

