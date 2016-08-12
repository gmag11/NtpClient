/*
 Name:		NtpClientLib
 Created:	21/12/2015 16:26:34
 Author:	Germán Martín (gmag11@gmail.com)
 Maintainer:Germán Martín (gmag11@gmail.com)
 Editor:	http://www.visualmicro.com

 Library to get system sync from a NTP server
*/

#ifndef _NtpClientLib_h
#define _NtpClientLib_h

//#define DEBUG_NTPCLIENT //Uncomment this to enable debug messages over serial port

#include <TimeLib.h>

#define NETWORK_ESP8266         (1)
#define NETWORK_W5100           (2)
#define NETWORK_ENC28J60        (3)

#ifdef ARDUINO_ARCH_ESP8266
#include "ESPNTPClient.h"
#elif defined (ARDUINO_ARCH_AVR)
#include "AvrNTPClient.h"
#else
#error "Incorrect platform. Only ARDUINO and ESP8266 MCUs are valid."
#endif

#endif // _NtpClientLib_h

