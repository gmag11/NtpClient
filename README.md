# NtpClient
NTP client libary to sync ESP8266 time via WiFi

This is my trial to develop a NTP library to be able to get time from NTP with my ESP8266s.

Using the library is fairly easy. You only need to create an instance giving host name. You may use default values by using contructor without parameters. Then you have add getTime() as a sync provider.

This library makes use of [Time library]. I had to
[Time library]: http://www.pjrc.com/teensy/td_libs_Time.html

ntpClient class is implemented following singleton structure so you don't have to run constructor but getInstance instead. Thanks to Mikael Patel for his recommendation to do it so.

This is a preliminary version. I'd like to do sync provider registering in a begin() member funcion but I've been not able to do it until now. I need a static function for that so I am not sure if I can do it from begin function or I have to make the registration in my main code.

ntpClient.ino is an example of how I'd like my libary to work. It shows time every second as soon it is synchronized.
