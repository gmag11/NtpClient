/*
 Name:		NtpClient.ino
 Created:	20/08/2016
 Author:	gmag11@gmail.com
 Editor:	http://www.visualmicro.com
*/

#include <TimeLib.h>
#include "WifiConfig.h"
#include <NtpClientLib.h>
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
	NTP.onNTPSyncEvent([](NTPSyncEvent_t error) {
		if (error) {
			Serial.print("Time Sync error: ");
			if (error == noResponse)
				Serial.println("NTP server not reachable");
			else if (error == invalidAddress)
				Serial.println("Invalid NTP server address");
		}
		else {
			Serial.print("Got NTP time: ");
			Serial.println(NTP.getTimeDateString(NTP.getLastNTPSync()));
		}
	});
	NTP.begin("es.pool.ntp.org", 1, true);
	NTP.setInterval(63);
}

void loop()
{
	static int i = 0;
	static int last = 0;

	if ((millis() - last) > 5100) {
		//Serial.println(millis() - last);
		last = millis();
		Serial.print(i); Serial.print(" ");
		Serial.print(NTP.getTimeDateString()); Serial.print(". ");
		Serial.print("WiFi is ");
		Serial.print(WiFi.isConnected() ? "connected" : "not connected"); Serial.print(". ");
		Serial.print("Uptime: ");
		Serial.print(NTP.getUptimeString()); Serial.print(" since ");
		Serial.println(NTP.getTimeDateString(NTP.getFirstSync()).c_str());

		i++;
	}
	delay(0);
}
