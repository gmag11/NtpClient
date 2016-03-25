#NtpClientLib

##Introduction
There are some NTP client examples around. You can see some examples, like [this].

In a device like ESP8266 or any Internet connected Arduino having NTP synchronization is well convenient. Using that example you can add NTP client to your projects but I was looking for a quicker way to add it, using something like NTPClient class.

So I decided to do my own NTP client libary to sync ESP8266 time via WiFi using Arduino IDE.
##Description
This is a NTP library to be able to get time from NTP server with my ESP8266s. Initial support for regular Arduino is available. Please test it and inform via GitHub.

Using the library is fairly easy. You only need to create an instance giving host name. You may use default values by using `getInstance()` without parameters. Then you can start synchronization using `begin()` funcion. Some parameters can be adjusted: server, sync frequency, time offset.

You don't need anything more. Time update is managed inside library so, after `begin()` no more calls to library are needed.

Update frequency is higher (every some seconds) until 1st successful sync is achieved. Since then, yout own (or default) adjusted period applies. There is a way to adjust this first period if needed.
##Example

```Arduino
    #include <TimeLib.h>
    
    #ifdef ARDUINO_ARCH_ESP8266
    #include <ESP8266WiFi.h>
    #endif // ARDUINO_ARCH_ESP8266
    
    #include <NtpClientLib.h>
    #define YOUR_WIFI_SSID "YOUR_WIFI_SSID"
    #define YOUR_WIFI_PASSWD "YOUR_WIFI_PASSWD"
        
    int i = 0;
    ntpClient *ntp;
        
    void setup() {
    	Serial.begin(115200);
    	WiFi.mode(WIFI_STA);
    	WiFi.begin(YOUR_WIFI_SSID, YOUR_WIFI_PASSWD);
    	//ntp = ntpClient::getInstance();
    	ntp = ntpClient::getInstance("es.pool.ntp.org", 1); // Spain
    	//ntp = ntpClient::getInstance("us.pool.ntp.org", -5); // New York
    	ntp->setInterval(15, 1800); // OPTIONAL. Set sync interval
    	ntp->begin(); //Starts time synchronization
    }
    
    void loop() {
    	Serial.print(i);
    	Serial.print(" ");
    	Serial.println(ntp->getTimeString());
    	delay(1000);
    	i++;
    }
```

##Performance
Don't expect atomic-clock-like precission. This library does not take network delay into account neither uses all NTP mechanisms available to improve accuracy. It is in the range of 1 to 2 seconds. Enough for most projects.
##Dependencies
This library makes use of [Time] library. You need to add it to use NTPClientLib
[Time]: https://github.com/PaulStoffregen/Time.git
_________________________________________________________
ntpClient class is implemented following singleton structure so you don't have to run constructor but getInstance() instead. Thanks to [Mikael Patel] for his recommendation to do it so. See [this message].
[this message]: http://arduino.stackexchange.com/questions/18825/ntp-client-library-set-sync-provider-pointing-to-public-class-function-possibl
[Mikael Patel]: http://arduino.stackexchange.com/users/13432/mikael-patel

ntpClient.ino is an example of how this library works. It shows time every second as soon it gets synchronized.
[this]: https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi/examples/NTPClient
