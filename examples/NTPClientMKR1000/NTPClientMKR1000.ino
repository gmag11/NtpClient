/*
 Name:		NtpClientAvr.ino
 Created:	20/08/2016
 Author:	gmag11@gmail.com
 Editor:	http://www.visualmicro.com
*/

#include <WiFiClient.h>
#include <SPI.h>
#include <WiFiUdp.h>
#include <WiFi101.h>
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <Dns.h>
#include <Dhcp.h>

#ifndef WIFI_CONFIG_H
#define YOUR_WIFI_SSID "YOUR_WIFI_SSID"
#define YOUR_WIFI_PASSWD "YOUR_WIFI_PASSWD"
#endif // !WIFI_CONFIG_H
//WiFiClient client;

void setup()
{
	Serial.begin(115200);
	WiFi.begin(YOUR_WIFI_SSID, YOUR_WIFI_PASSWD);
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print('.');
		delay(500);
	}
	Serial.println();
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
	NTP.begin("pool.ntp.org", 1, true);
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
		Serial.print("Uptime: ");
		Serial.print(NTP.getUptimeString()); Serial.print(" since ");
		Serial.println(NTP.getTimeDateString(NTP.getFirstSync()).c_str());

		i++;
	}
	delay(0);
}
