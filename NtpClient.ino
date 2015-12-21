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

ntpClient ntp;

// the setup function runs once when you press reset or power the board
void setup() {
	config.ssid = "SSID"; // Your SSID
	config.password = "PASSWD"; //Your WiFi Password
	WiFi.begin(config.ssid.c_str(), config.password.c_str());
	ntp = ntpClient(123,"pool.ntp.org")
}

// the loop function runs over and over again until power down or reset
void loop() {
  
}
