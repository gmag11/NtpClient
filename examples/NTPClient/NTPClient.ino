/*
 Name:		NtpClient.ino
 Created:	21/12/2015 16:26:34
 Author:	gmag1
 Editor:	http://www.visualmicro.com
*/

#include <TimeLib.h>
#include "WifiConfig.h"
#include "NtpClientLib.h"

#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#elif defined (ARDUINO_ARCH_AVR)
#include <SPI.h>
#include <EthernetUdp.h>
#include <Ethernet.h>
#include <Dns.h>
#include <Dhcp.h>
#endif

#ifndef WIFI_CONFIG_H
#define YOUR_WIFI_SSID "YOUR_WIFI_SSID"
#define YOUR_WIFI_PASSWD "YOUR_WIFI_PASSWD"
#endif // !WIFI_CONFIG_H

void setup()
{
	Serial.begin(115200);
#ifdef ARDUINO_ARCH_ESP8266
	WiFi.mode(WIFI_STA);
	WiFi.begin(YOUR_WIFI_SSID, YOUR_WIFI_PASSWD);
#elif defined (ARDUINO_ARCH_AVR)
#endif
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
		Serial.print(i); Serial.print(" ");
		Serial.print(NTP.getTimeDateString()); Serial.print(". ");
#ifdef ARDUINO_ARCH_ESP8266
		Serial.print("WiFi is ");
		Serial.print(WiFi.isConnected() ? "connected" : "not connected"); Serial.print(". ");
#endif
		Serial.print("Uptime: ");
		Serial.print(NTP.getUptimeString()); Serial.print(" since ");
		Serial.println(NTP.getTimeDateString(NTP.getFirstSync()).c_str());

		i++;
	}

}
