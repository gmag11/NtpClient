/*
 Name:		NtpClientLib
 Created:	21/12/2015 16:26:34
 Author:	Germán Martín (gmag11@gmail.com)
 Maintainer:Germán Martín (gmag11@gmail.com)
 Editor:	http://www.visualmicro.com

 Library to get system sync from a NTP server
*/

#ifndef _NtpClientLib_h
#define _NtpClientLib_h

#define DEBUG_NTPCLIENT //Uncomment this to enable debug messages over serial port

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

//#include <Time.h> // Do not use Time.h due to incompatibility with ESP8266 Arduino environment. 
					// See https://github.com/esp8266/Arduino/issues/1203
					// Use TimeLib.h instead
#include <TimeLib.h>

//#include <ESP8266WiFi.h>

//#include <WiFiUdp.h>
extern "C" {
#include "user_interface.h"
#include "sntp.h"
}

#define DEFAULT_NTP_SERVER "pool.ntp.org" // Default international NTP server. I recommend you to select a closer server to get better accuracy
#define DEFAULT_NTP_PORT 123 // Default local udp port. Select a different one if neccesary (usually not needed)
#define DEFAULT_NTP_INTERVAL 1800 // Default sync interval 30 minutes 
#define DEFAULT_NTP_SHORTINTERVAL 15 // Sync interval when sync has not been achieved. 15 seconds
#define DEFAULT_NTP_TIMEZONE 0 // Select your local time offset. 0 if UTC time has to be used

#define NTP_SERVER_NAME_SIZE 60 // Max server name length

class ntpClient {
	
public:
	
	/**
	* Starts time synchronization.
	* @param[in] NTP server name.
	* @param[in] Time offset from UTC.
	* @param[in] DayLight saving hour on.
	* @param[out] true if everything went ok.
	*/
	boolean begin(String ntpServerName = DEFAULT_NTP_SERVER, int timeOffset = DEFAULT_NTP_TIMEZONE, boolean daylight = false);

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
	* TODO: Add internationalization support
	*/
	String getTimeStr();
	
	/**
	* Convert a time in UNIX format to a String representing time.
	* @param[out] String constructed from current time.
	* @param[in] time_t object to convert to extract time.
	* TODO: Add internationalization support
	*/
	String getTimeStr(time_t moment);

	/**
	* Convert current date to a String.
	* @param[out] String constructed from current date.
	* TODO: Add internationalization support
	*/
	String getDateStr();

	/**
	* Convert a time in UNIX format to a String representing its date.
	* @param[out] String constructed from current date.
	* @param[in] time_t object to convert to extract date.
	* TODO: Add internationalization support
	*/
	String getDateStr(time_t moment);

	/**
	* Convert current time and date to a String.
	* @param[out] String constructed from current time.
	* TODO: Add internationalization support
	*/
	String getTimeString();

	/**
	* Convert current time and date to a String.
	* @param[out] String constructed from current time.
	* @param[in] time_t object to convert to String.
	* TODO: Add internationalization support
	*/
	String getTimeString(time_t moment);

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
	* Get daylight time saving option.
	* @param[out] true is daylight time savings apply.
	*/
	boolean getDayLight();


	/**
	* Gets NTP server name
	* @param[out] NTP server name.
	*/
	String getNtpServerName();

		/**
	* Gets timezone.
	* @param[out] Time offset in hours (plus or minus).
	*/
	int getTimeZone();

	/**
	* Gets last successful sync time in UNIX format.
	* @param[out] Last successful sync time. 0 equals never.
	*/

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
	* Sets timezone.
	* @param[in] New time offset in hours (-12 <= timeZone <= +12).
	* @param[out] True if everything went ok.
	*/
	boolean setTimeZone(int timeZone);

	/**
	* Sets NTP server name.
	* @param[in] New NTP server name.
	* @param[out] True if everything went ok.
	*/
	boolean	setNtpServerName(String ntpServerName);

	/**
	* Set daylight time saving option.
	* @param[in] true is daylight time savings apply.
	*/
	void setDayLight(boolean daylight);

	time_t getLastNTPSync();

protected:
	
	/**
	* Helper function to add leading 0 to hour, minutes or seconds if < 10.
	* @param[in] Digit to evaluate the need of leading 0.
	* @param[out] Result digit with leading 0 if needed.
	*/
	String printDigits(int digits);

	//int _timeZone; //Local time zone. Added to NTP time
	boolean _daylight; //Does this time zone have daylight saving?
	static boolean instanceFlag; //Flag to control that instance has been created
	//static ntpClient *s_client; //pointer to this instance
	int _shortInterval; //Interval to set periodic time sync until first synchronization.
	int _longInterval; //Interval to set periodic time sync
	time_t _lastSyncd; //Stored time of last successful sync

	boolean summertime(int year, byte month, byte day, byte hour, byte tzHours);

	/**
	* Construct NTP client to given server name.
	* @param[in] NTP server name as String.
	* @param[in] Time offset from UTC.
	*/
	ntpClient();
	
	//static void DestroyNtpClient();

private:

	//char _ntpServerName[NTP_SERVER_NAME_SIZE]; //NTP server name
	//IPAddress _timeServerIP; //NTP server IP address

};

ntpClient NTPClient;

#ifndef ARDUINO_ARCH_ESP8266
//#error “This library only supports ESP8266 based boards.”
#endif //ARDUINO_ARCH_ESP8266
#endif // _NtpClientLib_h

