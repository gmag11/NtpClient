/*
 Name:		NtpClient.ino
 Created:	21/12/2015 16:26:34
 Author:	gmag1
 Editor:	http://www.visualmicro.com
*/



//#include <WiFiUdp.h>

#include <TimeLib.h>

#ifdef ARDUINO_ARCH_ESP8266
//#define EXT_WIFI_CONFIG_H //Uncomment to enable WiFi credentials external header file storage.
// Used to not publish your own wifi keys in main code
/* WifiConfig.h example
#pragma once
#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#define YOUR_WIFI_SSID "YOUR_WIFI_SSID"
#define YOUR_WIFI_PASSWD "YOUR_WIFI_PASSWD"

#endif //WIFI_CONFIG_H
*/
#include <ESP8266WiFi.h>

#ifndef EXT_WIFI_CONFIG_H
#define YOUR_WIFI_SSID "YOUR_WIFI_SSID"
#define YOUR_WIFI_PASSWD "YOUR_WIFI_PASSWD"
#endif // !EXT_WIFI_CONFIG_H

struct strConfig {
	String ssid;
	String password;
} config;

#ifdef EXT_WIFI_CONFIG_H
#include "WifiConfig.h" // Wifi configuration (SSID + PASSWD) in an extenal .h file
#endif // EXT_WIFI_CONFIG_H

#elif defined (ARDUINO_ARCH_AVR)
#include <EthernetUdp.h>
#include <Ethernet.h>
#include <Dhcp.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
	0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
#endif // ARDUINO_ARCH_ESP8266

#include <NtpClientLib.h>



int i = 0;
ntpClient *ntp;


// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);
#ifdef ARDUINO_ARCH_ESP8266
	config.ssid = YOUR_WIFI_SSID; // Your SSID
	config.password = YOUR_WIFI_PASSWD; //Your WiFi Password
	WiFi.mode(WIFI_STA);
	WiFi.begin(config.ssid.c_str(), config.password.c_str());
#elif defined(ARDUINO_ARCH_AVR)
	if (Ethernet.begin(mac) == 0) {
		Serial.println("Failed to configure Ethernet using DHCP");
		// no point in carrying on, so do nothing forevermore:
		for (;;)
			;
	}
#endif // ARDUINO_ARCH_ESP8266
	
	//ntp = ntpClient::getInstance();
	ntp = ntpClient::getInstance("es.pool.ntp.org", 1); // Spain
	//ntp = ntpClient::getInstance("us.pool.ntp.org", -5); // New York
	ntp->setInterval(15, 1800); // OPTIONAL. Set sync interval
	ntp->begin(); //Starts time synchronization
}


// the loop function runs over and over again until power down or reset
void loop() {
	Serial.print(i);
	Serial.print(" ");
	Serial.println(ntp->getTimeString());
	delay(1000);
	i++;
}
