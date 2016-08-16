/*
 Name:		NtpClient.ino
 Created:	21/12/2015 16:26:34
 Author:	gmag1
 Editor:	http://www.visualmicro.com
*/

#include <TimeLib.h>
//#include "WifiConfig.h"
#include <NtpClientLib.h>

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#include <ESPNTPClient.h>
#elif defined ARDUINO_ARCH_AVR
#include <SPI.h>
#include <Dns.h>
#include <Dhcp.h>

#if NETWORK_TYPE == NETWORK_W5100
#include <EthernetUdp.h>
#include <Ethernet.h>
// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
	0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};
//EthernetClient client;

#elif NETWORK_TYPE == NETWORK_WIFI101
#include <WiFi101.h>
#include <WiFiUdp.h>

//WiFiClient client;
#endif


#endif

#ifndef WIFI_CONFIG_H
#define YOUR_WIFI_SSID "YOUR_WIFI_SSID"
#define YOUR_WIFI_PASSWD "YOUR_WIFI_PASSWD"
#endif // !WIFI_CONFIG_H

void setup()
{
	Serial.begin(115200);
#if NETWORK_TYPE == NETWORK_ESP8266
	WiFi.mode(WIFI_STA);
	WiFi.begin(YOUR_WIFI_SSID, YOUR_WIFI_PASSWD);
#elif NETWORK_TYPE == NETWORK_W5100
	if (Ethernet.begin(mac) == 0) {
		Serial.println("Failed to configure Ethernet using DHCP");
		// no point in carrying on, so do nothing forevermore:
		for (;;)
			;
	}
#elif NETWORK_TYPE == NETWORK_WIFI101
	if (WiFi.status() == WL_NO_SHIELD) {
		Serial.println("WiFi shield not present");
		// don't continue:
		while (true);
	}
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print("Attempting to connect to SSID: ");
		Serial.println(YOUR_WIFI_SSID);
		// Connect to WPA/WPA2 network. Change this line if using open or WEP network:
		WiFi.begin(YOUR_WIFI_SSID, YOUR_WIFI_PASSWD);
		// wait 10 seconds for connection:
		delay(10000);
	}

#endif // NETWORK_TYPE
	NTP.begin("es.pool.ntp.org", 1, true);
}

void loop()
{
	static int i = 0;
	static int last = 0;

	if ((millis() - last) > 5000) {
		//Serial.println(millis() - last);
		last = millis();
		Serial.print(i); Serial.print(" ");
		Serial.print(NTP.getTimeDateString()); Serial.print(". ");
#if NETWORK_TYPE == NETWORK_ESP8266
		Serial.print("WiFi is ");
		Serial.print(WiFi.isConnected() ? "connected" : "not connected"); Serial.print(". ");
#elif NETWORK_TYPE == NETWORK_WIFI101
		Serial.print("WiFi is ");
		Serial.print(WiFi.status() == WL_CONNECTED ? "connected" : "not connected"); Serial.print(". ");
#endif
		Serial.print("Uptime: ");
		Serial.print(NTP.getUptimeString()); Serial.print(" since ");
		Serial.println(NTP.getTimeDateString(NTP.getFirstSync()).c_str());

		i++;
	}
#if NETWORK_TYPE == NETWORK_W5100
	Ethernet.maintain(); // Check DHCP for renewal
#endif // NETWORK_W5100
}
