/*
 Name:		NtpClient.ino
 Created:	21/12/2015 16:26:34
 Author:	gmag1
 Editor:	http://www.visualmicro.com
*/


//#include <WiFiUdp.h>
#include <Time2.h>
#include <ESP8266WiFi.h>
#include "NtpClientLib.h"

struct strConfig {
	String ssid;
	String password;
} config;

ntpClient *ntp;

time_t getTimeProvider() {
	if (ntp != NULL) {
		return ntp->getTime();
	} else {
		Serial.println("ntp instance null");
		return 0;
	}
}

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);
	config.ssid = "Virus_Detected!!!"; // Your SSID
	config.password = "LaJunglaSigloXX1@."; //Your WiFi Password
	WiFi.begin(config.ssid.c_str(), config.password.c_str());
	ntp = ntpClient::getInstance("es.pool.ntp.org");
	setSyncProvider(getTimeProvider);
	setSyncInterval(10);
}

void showTime() {
	if (timeStatus() != timeNotSet)
		Serial.println(ntp->getTimeString());
	else
		Serial.println("Time not set");
	//showtime_flag = false;
}

// the loop function runs over and over again until power down or reset
void loop() {
	showTime();
	delay(1000);
}
