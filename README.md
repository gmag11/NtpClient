# NtpClient
NTP client libary to sync ESP8266 time via WiFi

This is my trial to develop a NTP library to be able to get time from NTP with my ESP8266s.

Using the library is fairly easy. You only need to create an instance giving host and port. You may use default values by using contructor without parameters. Then you can control NTP sync using begin() and stop() functions.

NOTICE that this version is not working yet. I have to find a way to register ntpClient::getNtpTime() function as a syncProvider. I need a static function for that so I am not sure if I can do it from begin function or I have to make the registration in my main code.

ntpClient.ino is an example of how I'd like my libary to work. It is neither working.
