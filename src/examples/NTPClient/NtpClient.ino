/*
 Name:		NtpClient.ino
 Created:	21/12/2015 16:26:34
 Author:	gmag1
 Editor:	http://www.visualmicro.com
*/

//#define EXT_WIFI_CONFIG_H

//#include <WiFiUdp.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <NtpClientLib.h>
#ifdef EXT_WIFI_CONFIG_H
#include "WifiConfig.h" // Wifi configuration (SSID + PASSWD) in an extenal .h file
#endif // EXT_WIFI_CONFIG_H

#ifndef EXT_WIFI_CONFIG_H
#define YOUR_WIFI_SSID "YOUR_WIFI_SSID"
#define YOUR_WIFI_PASSWD "YOUR_WIFI_PASSWD"
#endif // !EXT_WIFI_CONFIG_H

struct strConfig {
	String ssid;
	String password;
} config;

int i = 0;
ntpClient *ntp;


// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);
	config.ssid = YOUR_WIFI_SSID; // Your SSID
	config.password = YOUR_WIFI_PASSWD; //Your WiFi Password
	WiFi.mode(WIFI_STA);
	WiFi.begin(config.ssid.c_str(), config.password.c_str());
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
