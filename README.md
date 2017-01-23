#NtpClientLib

##Introduction
There are some NTP client examples around. You can see some examples, like [this].

In a device like ESP8266 or any Internet connected Arduino having NTP synchronization is well convenient. Using that example you can add NTP client to your projects but I was looking for a quicker way to add it, using something like NTPClient class.

So I decided to do my own NTP client libary to sync ESP8266 time via WiFi using Arduino IDE. It can also be used on any ethernet or WiFi connected Arduino, although as I do not have any WiFi enabled arduino code is not tested. Testers are welcome.

##Description
This is a NTP library to be able to get time from NTP server with my ESP8266s. Initial support for regular Arduino is available. Please test it and inform via GitHub.

Using the library is fairly easy. A NTP object is created inside library. You may use default values by using `NTP.begin()` without parameters. After that, synchronization is done regularly without user intervention. Some parameters can be adjusted: server, sync frequency, time offset.

You don't need anything more. Time update is managed inside library so, after `NTP.begin()` no more calls to library are needed.

Update frequency is higher (every 15 seconds as default) until 1st successful sync is achieved. Since then, your own (or default 1800 seconds) adjusted period applies. There is a way to adjust both period if needed.

In order to reduce scketch size, ESP8266 version makes use of internal Espressif SDK routines that already implement SNTP protocol.

This library includes an uptime log too. It counts number of seconds since scketch is started. It can be checked calling `NTP.getUptime()` or `NTP.getUptimeString()` for a human readable string.

Every time that local time is adjuste a `ntpEvent` is thrown. You can attach a function to it using `NTP.onNTPSyncEvent()`. Indeed, this event is thrown just before time is sent to [Time] Libary. Bacause of that, you should try not to make time consuming tasks inside event handler. Although it is taken into account inside library, it would add some offset to calculated time.

Called funtion format must be like `void eventHandler(NTPSyncEvent_t event)`.

ESP8266 example uses a simple function to turn a flag on, so actual event handling code is run inside main loop.

##Example for Arduino

```Arduino
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <SPI.h>
#include <EthernetUdp.h>
#include <Ethernet.h>
#include <Dns.h>
#include <Dhcp.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
	0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};

EthernetClient client;

void setup()
{
	Serial.begin(115200);
	if (Ethernet.begin(mac) == 0) {
		Serial.println("Failed to configure Ethernet using DHCP");
		// no point in carrying on, so do nothing forevermore:
		for (;;)
			;
	}
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
		Serial.print("Uptime: ");
		Serial.print(NTP.getUptimeString()); Serial.print(" since ");
		Serial.println(NTP.getTimeDateString(NTP.getFirstSync()).c_str());

		i++;
	}
	delay(0);
	Ethernet.maintain(); // Check DHCP for renewal
}
```

##Example for ESP8266

```Arduino
#include <TimeLib.h>
#include "WifiConfig.h"
#include <NtpClientLib.h>
#include <ESP8266WiFi.h>

#ifndef WIFI_CONFIG_H
#define YOUR_WIFI_SSID "YOUR_WIFI_SSID"
#define YOUR_WIFI_PASSWD "YOUR_WIFI_PASSWD"
#endif // !WIFI_CONFIG_H

#define ONBOARDLED 2 // Built in LED on ESP-12/ESP-07

// Start NTP only after IP network is connected
void onSTAGotIP(WiFiEventStationModeGotIP ipInfo) {
	Serial.printf("Got IP: %s\r\n", ipInfo.ip.toString().c_str());
	NTP.begin("pool.ntp.org", 1, true);
	NTP.setInterval(63);
	digitalWrite(ONBOARDLED, LOW); // Turn on LED
}

// Manage network disconnection
void onSTADisconnected(WiFiEventStationModeDisconnected event_info) {
	Serial.printf("Disconnected from SSID: %s\n", event_info.ssid.c_str());
	Serial.printf("Reason: %d\n", event_info.reason);
	digitalWrite(ONBOARDLED, HIGH); // Turn off LED
	//NTP.stop(); // NTP sync can be disabled to avoid sync errors
}

void processSyncEvent(NTPSyncEvent_t ntpEvent) {
	if (ntpEvent) {
		Serial.print("Time Sync error: ");
		if (ntpEvent == noResponse)
			Serial.println("NTP server not reachable");
		else if (ntpEvent == invalidAddress)
			Serial.println("Invalid NTP server address");
	}
	else {
		Serial.print("Got NTP time: ");
		Serial.println(NTP.getTimeDateString(NTP.getLastNTPSync()));
	}
}

boolean syncEventTriggered = false; // True if a time even has been triggered
NTPSyncEvent_t ntpEvent; // Last triggered event

void setup()
{
	static WiFiEventHandler e1, e2;

	Serial.begin(115200);
	WiFi.mode(WIFI_STA);
	WiFi.begin(YOUR_WIFI_SSID, YOUR_WIFI_PASSWD);

	pinMode(ONBOARDLED, OUTPUT); // Onboard LED
	digitalWrite(ONBOARDLED, HIGH); // Switch off LED

	NTP.onNTPSyncEvent([](NTPSyncEvent_t event) {
		ntpEvent = event;
		syncEventTriggered = true;
	});

	// Deprecated
	/*WiFi.onEvent([](WiFiEvent_t e) {
		Serial.printf("Event wifi -----> %d\n", e);
	});*/

	e1 = WiFi.onStationModeGotIP(onSTAGotIP);// As soon WiFi is connected, start NTP Client
	e2 = WiFi.onStationModeDisconnected(onSTADisconnected);

}

void loop()
{
	static int i = 0;
	static int last = 0;

	if (syncEventTriggered) {
		processSyncEvent(ntpEvent);
		syncEventTriggered = false;
	}

	if ((millis() - last) > 5100) {
		//Serial.println(millis() - last);
		last = millis();
		Serial.print(i); Serial.print(" ");
		Serial.print(NTP.getTimeDateString()); Serial.print(" ");
		Serial.print(NTP.isSummerTime() ? "Summer Time. " : "Winter Time. ");
		Serial.print("WiFi is ");
		Serial.print(WiFi.isConnected() ? "connected" : "not connected"); Serial.print(". ");
		Serial.print("Uptime: ");
		Serial.print(NTP.getUptimeString()); Serial.print(" since ");
		Serial.println(NTP.getTimeDateString(NTP.getFirstSync()).c_str());

		i++;
	}
	delay(0);
}
```

##Performance
Don't expect atomic-clock-like precission. This library does not take network delay into account neither uses all NTP mechanisms available to improve accuracy. It is in the range of 1 to 2 seconds. Enough for most projects.

Next major update will add network delay compensation. Due to limited Time Library precission of 1 second, it probably will not affect overall accuracy.

##Dependencies
This library makes use of [Time] library. You need to add it to use NTPClientLib
[Time]: https://github.com/PaulStoffregen/Time.git
_________________________________________________________

ntpClient.ino is an example of how this library works. It shows current time and uptime every 5 seconds as soon it gets synchronized.
[this]: https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi/examples/NTPClient
