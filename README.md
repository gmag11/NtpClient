# NtpClient
NTP client libary to sync ESP8266 time via WiFi using Arduino IDE.

This is a NTP library to be able to get time from NTP server with my ESP8266s. It does not support regular Arduino yet. Feel free to adapt it.

Using the library is fairly easy. You only need to create an instance giving host name. You may use default values by using getInstance() without parameters. Then you neet to start synchronization using begin() funcion. Some parameters can be adjusted: server, sync frequency, time offset.

This library makes use of [Time] library. I had to rename it to Time2.h due to a file named time.h already existing in Arduino ESP8266 environment. 
[Time]: http://www.pjrc.com/teensy/td_libs_Time.html

ntpClient class is implemented following singleton structure so you don't have to run constructor but getInstance() instead. Thanks to Mikael Patel for his recommendation to do it so. See [this message].
[this message]: http://arduino.stackexchange.com/questions/18825/ntp-client-library-set-sync-provider-pointing-to-public-class-function-possibl

~~This is a preliminary version. I'd like to do sync provider registering in a begin() member funcion but I've been not able to do it until now. I need a static function for that so I am not sure if I can do it from begin function or I have to make the registration in my main code.~~

ntpClient.ino is an example of how this library works. It shows time every second as soon it is synchronized.
