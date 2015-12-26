/*
 Name:		NtpClientLib.h
 Created:	21/12/2015 16:26:34
 Author:	gmag11@gmail.com
 Editor:	http://www.visualmicro.com
*/

#ifndef _NtpClientLib_h
#define _NtpClientLib_h

//#define DEBUG //Uncomment this to enable debug messages over serial port

#define NTP_TIME_SYNC // Comment this to enable experimental Web Time synchronization via HTTP
					  // This method is far less accurate and is not recommended

#ifndef NTP_TIME_SYNC
#define WEB_TIME_SYNC // Web time sync method. Not reliable!!!
#endif //NTP_TIME_SYNC

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <Time2.h> // Renamed from Time.h due to incompatibility with ESP8266 Arduino environment.
#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#ifdef NTP_TIME_SYNC
#include <WiFiUdp.h>
#endif // NTP_TIME_SYNC
#endif // ARDUINO_ARCH_ESP8266

//#include <Udp.h>

#define DEFAULT_NTP_SERVER "pool.ntp.org" // Default international NTP server. I recommend you to select a closer server to get better accuracy
#define DEFAULT_NTP_PORT 123 // Default local udp port. Select a different one if neccesary (usually not needed)
#define DEFAULT_NTP_INTERVAL 1800 // Default sync interval 30 minutes 
#define DEFAULT_NTP_SHORTINTERVAL 15 // Sync interval when sync has not been achieved. 15 seconds
#define DEFAULT_NTP_TIMEZONE 0 // Select your local time offset. 0 if UTC time has to be used

#define NTP_SERVER_NAME_SIZE 60 // Max server name length

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
	static ntpClient* getInstance(String ntpServerName = DEFAULT_NTP_SERVER, int timeOffset = DEFAULT_NTP_TIMEZONE);

	//virtual ~ntpClient() {};

	/**
	* Starts time synchronization.
	* @param[out] true if everything went ok.
	*/
	boolean begin();

	/**
	* Stops time synchronization.
	* @param[out] True if everything went ok.
	*/
	boolean stop();

	/**
	* Starts a NTP time request to server. Returns a time in UNIX time format. Normally only called from library.
	* Kept in public section to allow direct NTP request.
	* @param[out] Time in UNIX time format.
	*/
	static time_t getTime();
	
	/**
	* Convert current time to a String.
	* @param[out] String constructed from current time.
	*/
	String getTimeString();

	//boolean	setUdpPort(int port);
	//int		getUdpPort();

	/**
	* Changes sync period.
	* @param[in] New interval in seconds.
	* @param[out] True if everything went ok.
	*/
	boolean setInterval(int interval);
	
	/**
	* Changes sync period in sync'd and not sync'd status.
	* @param[in] New interval while time is not first adjusted yet, in seconds.
	* @param[in] New interval for normal operation, in seconds.
	* @param[out] True if everything went ok.
	*/
	boolean setInterval(int shortInterval, int longInterval);

	/**
	* Gets sync period.
	* @param[out] Interval for normal operation, in seconds.
	*/
	int		getInterval();
	
	/**
	* Changes sync period not sync'd status.
	* @param[out] Interval while time is not first adjusted yet, in seconds.
	*/
	int		getShortInterval();

	/**
	* Gets sync period.
	* @param[out] Interval for normal operation in seconds.
	*/
	int		getLongInterval() { return getInterval(); }

	/**
	* Sets NTP server name.
	* @param[in] New NTP server name.
	* @param[out] True if everything went ok.
	*/
	boolean	setNtpServerName(String ntpServerName);
	
	/**
	* Gets NTP server name
	* @param[out] NTP server name.
	*/
	String getNtpServerName();

	/**
	* Sets timezone.
	* @param[in] New time offset in hours (-12 <= timeZone <= +12).
	* @param[out] True if everything went ok.
	*/
	boolean setTimeZone(int timeZone);
	
	/**
	* Gets timezone.
	* @param[out] Time offset in hours (plus or minus).
	*/
	int getTimeZone();

protected:
	
	/**
	* Helper function to add leading 0 to hour, minutes or seconds if < 10.
	* @param[in] Digit to evaluate the need of leading 0.
	* @param[out] Result digit with leading 0 if needed.
	*/
	String printDigits(int digits);

	int _timeZone; //Local time zone. Added to NTP time
	static bool instanceFlag; //Flag to control that instance has been created
	static ntpClient *s_client; //pointer to this instance
	int _shortInterval; //Interval to set periodic time sync until first synchronization.
	int _longInterval; //Interval to set periodic time sync

	/**
	* Construct NTP client to given server name.
	* @param[in] NTP server name as String.
	* @param[in] Time offset from UTC.
	*/
	ntpClient(String ntpServerName, int timeOffset);
	
	//static void DestroyNtpClient();

private:

#ifdef NTP_TIME_SYNC
	int _udpPort; //UDP port number to send request from
#endif // NTP_TIME_SYNC
	char _ntpServerName[NTP_SERVER_NAME_SIZE]; //NTP server name
	IPAddress _timeServerIP; //NTP server IP address
#ifdef WEB_TIME_SYNC
	WiFiClient _webClient; // HTTP client
#endif //WEB_TIME_SYNC

#ifdef NTP_TIME_SYNC
	WiFiUDP _udp; //UDP port object
#endif // NTP_TIME_SYNC

	byte _ntpPacketBuffer[NTP_PACKET_SIZE]; //Buffer to store request and response messages
	
	/**
	* Sends NTP request packet to given IP address.
	* @param[in] NTP server's IP address.
	* @param[out] True if everything went ok.
	*/
	boolean sendNTPpacket(IPAddress &address);

	/**
	* Decode NTP response contained in buffer.
	* @param[in] Pointer to message buffer.
	* @param[out] Decoded time from message, 0 if error ocurred.
	*/
	time_t decodeNtpMessage(byte *messageBuffer);

};

#endif // _NtpClientLib_h

