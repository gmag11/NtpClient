# NtpClient
NTP client libary to sync ESP8266 time via WiFi using Arduino IDE.

This is a NTP library to be able to get time from NTP server with my ESP8266s. It does not support regular Arduino yet. Feel free to adapt it. Anyway I'll try to.

Using the library is fairly easy. You only need to create an instance giving host name. You may use default values by using getInstance() without parameters. Then you can start synchronization using begin() funcion. Some parameters can be adjusted: server, sync frequency, time offset.

Update frequency is higher (every some seconds) until 1st successful sync is achieved. Since then, yout own (or default) adjusted period applies. There is a way to adjust this first period if needed.

Don't expect atomic-clock-like precission. This library does not take network delay into account neither uses all NTP mechanisms available to improve accuracy. It is in the range of 1 to 2 seconds. Enough for most projects.

This library makes use of [Time] library. 
[Time]: https://github.com/PaulStoffregen/Time.git

ntpClient class is implemented following singleton structure so you don't have to run constructor but getInstance() instead. Thanks to [Mikael Patel] for his recommendation to do it so. See [this message].
[this message]: http://arduino.stackexchange.com/questions/18825/ntp-client-library-set-sync-provider-pointing-to-public-class-function-possibl
[Mikael Patel]: http://arduino.stackexchange.com/users/13432/mikael-patel

ntpClient.ino is an example of how this library works. It shows time every second as soon it gets synchronized.
