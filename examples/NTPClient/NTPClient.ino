/*
 Name:		NtpClient.ino
 Created:	21/12/2015 16:26:34
 Author:	gmag1
 Editor:	http://www.visualmicro.com
*/

#include <TimeLib.h>

#include <TimeLib.h>
#include "WifiConfig.h"
#include "NtpClientLib.h"
#include <ESP8266WiFi.h>

#ifndef WIFI_CONFIG_H
#define YOUR_WIFI_SSID "YOUR_WIFI_SSID"
#define YOUR_WIFI_PASSWD "YOUR_WIFI_PASSWD"
#endif // !WIFI_CONFIG_H

void setup()
{
	Serial.begin(115200);
	WiFi.mode(WIFI_STA);
	WiFi.begin(YOUR_WIFI_SSID, YOUR_WIFI_PASSWD);
	NTP.begin("es.pool.ntp.org", 1, true);
}

void loop()
{
	static int i = 0;
	static int last = 0;

	//if (timeStatus() == timeSet) {
	if ((millis() - last) > 5000) {
		//Serial.println(millis() - last);
		last = millis();
		Serial.printf("%d %s. WiFi is %s. Uptime: %s since %s                 \r\n",
			i,
			NTP.getTimeDateString().c_str(),
			WiFi.isConnected() ? "connected" : "not connected",
			NTP.getUptimeString().c_str(),
			NTP.getTimeDateString(NTP.getFirstSync()).c_str());
		i++;
	}

}
