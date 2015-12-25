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

int i = 0;
ntpClient *ntp;


// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);
	config.ssid = "AndroidAP9802"; // Your SSID
	config.password = "lanocheloca08."; //Your WiFi Password
	WiFi.mode(WIFI_STA);
	WiFi.begin(config.ssid.c_str(), config.password.c_str());
	ntp = ntpClient::getInstance("es.pool.ntp.org",1);
	//ntp = ntpClient::getInstance("www.google.es", 1);
	ntp->begin(); //Starts time synchronization
	//ntp->setInterval(10,60);
	//setSyncProvider(ntp->getTime);
	//setSyncInterval(10);
}

// the loop function runs over and over again until power down or reset
void loop() {
	Serial.print(i);
	Serial.print(" ");
	Serial.println(ntp->getTimeString());
	delay(1000);
	i++;
	/*if (i == 60) {
		ntp->setInterval(15);
	}
	if (i == 120) {
		ntp->setNtpServerName("www.google.com");
		ntp->setInterval(20);
	}*/
}
