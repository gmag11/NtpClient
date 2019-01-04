/*
Copyright 2016 German Martin (gmag11@gmail.com). All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met :

1. Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list
of conditions and the following disclaimer in the documentation and / or other materials
provided with the distribution.

THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.IN NO EVENT SHALL <COPYRIGHT HOLDER> OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of German Martin
*/
//
//
//

#include "NtpClientLib.h"

#define DBG_PORT Serial

#ifdef DEBUG_NTPCLIENT
#define DEBUGLOG(...) DBG_PORT.printf(__VA_ARGS__)
#else
#define DEBUGLOG(...)
#endif

#define MINS_PER_HOUR 60

NTPClient::NTPClient () {
}

bool NTPClient::setNtpServerName (String ntpServerName) {
    char * name = (char *)malloc ((ntpServerName.length () + 1) * sizeof (char));
    if (!name)
        return false;
    ntpServerName.toCharArray (name, ntpServerName.length () + 1);
    DEBUGLOG ("NTP server set to %s\n", name);
    free (_ntpServerName);
    _ntpServerName = name;
    return true;
}

bool NTPClient::setNtpServerName (char* ntpServerName) {
    char *name = ntpServerName;
    if (name == NULL)
        return false;
    DEBUGLOG ("NTP server set to %s\n", name);
    free (_ntpServerName);
    _ntpServerName = name;
    return true;
}

String NTPClient::getNtpServerName () {
    return String (_ntpServerName);
}

char* NTPClient::getNtpServerNamePtr () {
    return _ntpServerName;
}

int NTPClient::setTimeZone ( int16_t  timeZoneOffset,     // full offset from GMT 0 in minutes 
			      char * timeZoneName,
			      char * timeZoneDSTName,    // NULL if disable
			      int16_t  timeZoneDSTOffset,  // full offset from GMT 0 in minutes  for DST
			      uint8_t dstStartMonth,     // start of Summer time if enabled  Month 1 - 12, 0 disabled dst
			      uint8_t dstStartWeek,      // start of Summer time if enabled Week 1 - 5: (5 means last)
			      uint8_t dstStartDay,       // start of Summer time if enabled Day 0- 6  (0- Sun)
			     // if startDay == 7, then sum of (StartMonth+StartWeek) is the day of the year DST starts
			      int16_t dstStartMin,     // start of Summer time if enabled in minutes
			      uint8_t dstEndMonth,       // end of Summer time if enabled  Month 1 - 12
			      uint8_t dstEndWeek,        // end of Summer time if enabled Week 1 - 5: (5 means last)
			      uint8_t dstEndDay,         // end of Summer time if enabled Day 0-6  (0- Sun)
			     // if EndDay == 7, then sum of (EndMonth+EndWeek) is the day of the year DST ends
			      int16_t dstEndMin) {       // end of Summer time if enabled in minutes

  if ( timeZoneOffset < -12 * MINS_PER_HOUR  || timeZoneOffset > 14*MINS_PER_HOUR ) return 1;
  if ( timeZoneDSTName != NULL ) {
    if ( timeZoneDSTOffset < -12 * MINS_PER_HOUR  || timeZoneDSTOffset > 14*MINS_PER_HOUR ) return 2;
    if ( (dstStartMonth  < 1  || dstStartMonth > 12 ) && dstStartDay < 7   ) return 3;
    if ( (dstStartWeek  < 1  || dstStartWeek > 5  ) && dstStartDay < 7 ) return 4;
    if ( dstStartDay  < 0  || dstStartDay > 8  ) return 5;
    if ( dstStartMin  > 167*MINS_PER_HOUR-1  ) return 6;
    if ( (dstEndMonth  < 1  || dstEndMonth > 12) && dstEndDay < 7  ) return 7;
    if ( (dstEndWeek  < 1  || dstEndWeek > 5) && dstEndDay < 7  ) return 8;
    if ( dstEndDay  < 0  || dstEndDay > 8  ) return 9;
    if ( dstEndMin  > 167*MINS_PER_HOUR-1  ) return 10;
  }
  int16_t currOffset = (_useDST) ? _tzDSTOffset : _tzOffset;
  _tzName = timeZoneName;
  _tzDSTName = timeZoneDSTName;
  _tzDSTOffset = timeZoneDSTOffset;
  _tzOffset = timeZoneOffset;
  _tzDSTOffset = timeZoneDSTOffset;
  _dstStartMonth = dstStartMonth;
  _dstStartWeek  = dstStartWeek;
  _dstStartDay   = dstStartDay;
  _dstStartMin   = dstStartMin;
  _dstEndMonth = dstEndMonth;
  _dstEndWeek  = dstEndWeek;
  _dstEndDay   = dstEndDay;
  _dstEndMin   = dstEndMin;
  _useDST = false;
  setTime (now () + (_tzOffset - currOffset) * SECS_PER_MIN);
  DEBUGLOG ("NTP time zone set to: %d:02d\r\n", _tzOffset < 0 ? "-" : "", abs(_tzOffset)/MINS_PER_HOUR, abs(_tzOffset) % MINS_PER_HOUR);
  if (_tzDSTName != NULL ) {
    if (isSummerTimePeriod ( now() ) ) {
      setTime (now () + (_tzDSTOffset - _tzOffset) * SECS_PER_MIN);
      _useDST = true;
      DEBUGLOG ("NTP adjust for DST, time zone set to: %s%d:02d\r\n",
		_tzDSTOffset < 0 ? "-" : "", abs(_tzDSTOffset)/MINS_PER_HOUR, abs(_tzDSTOffset) % MINS_PER_HOUR);
    }
  }
  return 0;
  
}

bool NTPClient::setTimeZone (int8_t timeZone, int8_t minutes) {

  int16_t totalOffset = timeZone * MINS_PER_HOUR + minutes;
  return setTimeZone( totalOffset, "?" ) == 0;
}

int16_t NTPClient::getOffset () {

    if (_useDST == true) {
        return _tzDSTOffset;
    }
    return _tzOffset;
}

#if NETWORK_TYPE == NETWORK_W5100 || NETWORK_TYPE == NETWORK_WIFI101
boolean sendNTPpacket (IPAddress address, UDP *udp) {
    uint8_t ntpPacketBuffer[NTP_PACKET_SIZE]; //Buffer to store request message

                                           // set all bytes in the buffer to 0
    memset (ntpPacketBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    ntpPacketBuffer[0] = 0b11100011;   // LI, Version, Mode
    ntpPacketBuffer[1] = 0;     // Stratum, or type of clock
    ntpPacketBuffer[2] = 6;     // Polling Interval
    ntpPacketBuffer[3] = 0xEC;  // Peer Clock Precision
                                // 8 bytes of zero for Root Delay & Root Dispersion
    ntpPacketBuffer[12] = 49;
    ntpPacketBuffer[13] = 0x4E;
    ntpPacketBuffer[14] = 49;
    ntpPacketBuffer[15] = 52;
    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    udp->beginPacket (address, DEFAULT_NTP_PORT); //NTP requests are to port 123
    udp->write (ntpPacketBuffer, NTP_PACKET_SIZE);
    udp->endPacket ();
    return true;
}

time_t NTPClient::getTime () {
    IPAddress timeServerIP; //NTP server IP address
    uint8_t ntpPacketBuffer[NTP_PACKET_SIZE]; //Buffer to store response message


    DEBUGLOG ("Starting UDP\n");
    udp->begin (DEFAULT_NTP_PORT);
    //DEBUGLOG ("UDP port: %d\n",udp->localPort());
    while (udp->parsePacket () > 0); // discard any previously received packets
#if NETWORK_TYPE == NETWORK_W5100
    DNSClient dns;
    dns.begin (Ethernet.dnsServerIP ());
    int8_t dnsResult = dns.getHostByName (getNtpServerName ().c_str (), timeServerIP);
    if (dnsResult <= 0) {
        if (onSyncEvent)
            onSyncEvent (invalidAddress);
        return 0; // return 0 if unable to get the time
    }
#else
    WiFi.hostByName (getNtpServerName ().c_str (), timeServerIP);
#endif
    DEBUGLOG ("NTP Server IP: %s\r\n", timeServerIP.toString ().c_str ());
    sendNTPpacket (timeServerIP, udp);
    uint32_t beginWait = millis ();
    while (millis () - beginWait < ntpTimeout) {
        int size = udp->parsePacket ();
        if (size >= NTP_PACKET_SIZE) {
            DEBUGLOG ("-- Receive NTP Response\n");
            udp->read (ntpPacketBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
            time_t timeValue = decodeNtpMessage (ntpPacketBuffer);
            if (timeValue != 0) {
                setSyncInterval (getLongInterval ());
                if (!_firstSync) {
                    //    if (timeStatus () == timeSet)
                    _firstSync = timeValue;
                }
                //getFirstSync (); // Set firstSync value if not set before
                DEBUGLOG ("Sync frequency set low\n");
                udp->stop ();
                setLastNTPSync (timeValue);
                DEBUGLOG ("Successful NTP sync at %s\n", getTimeDateString (getLastNTPSync ()).c_str ());

                if (onSyncEvent)
                    onSyncEvent (timeSyncd);
                return timeValue;
            } else {
                DEBUGLOG ("-- No valid NTP data :-(\n");
                udp->stop ();
                setSyncInterval (getShortInterval ()); // Retry connection more often
                if (onSyncEvent)
                    onSyncEvent (noResponse);
                return 0; // return 0 if unable to get the time
            }
        }
#ifdef ARDUINO_ARCH_ESP8266
        ESP.wdtFeed ();
        yield ();
#endif
    }
    DEBUGLOG ("-- No NTP Response :-(\n");
    udp->stop ();
    if (timeStatus () != timeSet) {
        setSyncInterval (getShortInterval ()); // Retry connection more often if sync is needed and we get no response
    }
    if (onSyncEvent)
        onSyncEvent (noResponse);
    return 0; // return 0 if unable to get the time
}
#elif NETWORK_TYPE == NETWORK_ESP8266 || NETWORK_TYPE == NETWORK_ESP32
time_t NTPClient::getTime () {
    IPAddress timeServerIP; //NTP server IP address
                            //char ntpPacketBuffer[NTP_PACKET_SIZE]; //Buffer to store response message
    DEBUGLOG ("Starting UDP\n");
    int error = WiFi.hostByName (getNtpServerName ().c_str (), timeServerIP);
    if (error) {
        DEBUGLOG ("Starting UDP. IP: %s\n", timeServerIP.toString ().c_str ());
        if (udp->connect (timeServerIP, DEFAULT_NTP_PORT)) {
            udp->onPacket (std::bind (&NTPClient::processPacket, this, _1));
            DEBUGLOG ("Sending UDP packet\n");
            if (sendNTPpacket (udp)) {
                DEBUGLOG ("NTP request sent\n");
                status = requested;
                responseTimer.once_ms (ntpTimeout, &NTPClient::s_processRequestTimeout, static_cast<void*>(this));
                /*timer1_attachInterrupt (s_processRequestTimeout);
                timer1_enable (TIM_DIV256, TIM_EDGE, TIM_SINGLE);
                timer1_write ((uint32_t)(312.5*ntpTimeout));*/
                if (onSyncEvent)
                    onSyncEvent (requestSent);
                return 0;
            } else {
                DEBUGLOG ("NTP request error\n");
                if (onSyncEvent)
                    onSyncEvent (errorSending);
                return 0;
            }
        } else {
            if (onSyncEvent)
                onSyncEvent (noResponse);
            return 0; // return 0 if unable to get the time
        }
    } else {
        DEBUGLOG ("HostByName error %d\n", error);
        if (onSyncEvent)
            onSyncEvent (invalidAddress);
        return 0; // return 0 if unable to get the time
    }

}

void dumpNTPPacket (byte *data, size_t length) {
    //byte *data = packet.data ();
    //size_t length = packet.length ();

    for (size_t i = 0; i < length; i++) {
        DEBUGLOG ("%02X ", data[i]);
        if ((i + 1) % 16 == 0) {
            DEBUGLOG ("\n");
        } else if ((i + 1) % 4 == 0) {
            DEBUGLOG ("| ");
        }
    }
}

boolean NTPClient::sendNTPpacket (AsyncUDP *udp) {
    AsyncUDPMessage ntpPacket = AsyncUDPMessage ();

    uint8_t ntpPacketBuffer[NTP_PACKET_SIZE]; //Buffer to store request message
                                              // set all bytes in the buffer to 0
    memset (ntpPacketBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    ntpPacketBuffer[0] = 0b11100011;   // LI, Version, Mode
    ntpPacketBuffer[1] = 0;     // Stratum, or type of clock
    ntpPacketBuffer[2] = 6;     // Polling Interval
    ntpPacketBuffer[3] = 0xEC;  // Peer Clock Precision
                                // 8 bytes of zero for Root Delay & Root Dispersion
    ntpPacketBuffer[12] = 49;
    ntpPacketBuffer[13] = 0x4E;
    ntpPacketBuffer[14] = 49;
    ntpPacketBuffer[15] = 52;
    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    ntpPacket.write (ntpPacketBuffer, NTP_PACKET_SIZE);
    if (udp->send (ntpPacket)) {
        DEBUGLOG ("\n");
        dumpNTPPacket (ntpPacket.data (), ntpPacket.length ());
        DEBUGLOG ("\nUDP packet sent\n");
        return true;
    } else {
        return false;
    }
}

void NTPClient::processPacket (AsyncUDPPacket packet) {
    uint8_t *ntpPacketBuffer;
    int size;

    if (status == requested) {
        size = packet.length ();
        if (size >= NTP_PACKET_SIZE) {
            //timer1_disable ();
            responseTimer.detach ();
            ntpPacketBuffer = packet.data ();
            time_t timeValue = decodeNtpMessage (ntpPacketBuffer);
            setTime (timeValue);
            status = syncd;
            setSyncInterval (getLongInterval ());
            if (!_firstSync) {
                //    if (timeStatus () == timeSet)
                _firstSync = timeValue;
            }
            setLastNTPSync (timeValue);
            DEBUGLOG ("Sync frequency set low\n");
            DEBUGLOG ("Successful NTP sync at %s\n", getTimeDateString (getLastNTPSync ()).c_str ());

            if (onSyncEvent)
                onSyncEvent (timeSyncd);
        } else {
            DEBUGLOG ("Response Error\n");
            status = unsyncd;
            if (onSyncEvent)
                onSyncEvent (responseError);
        }

    } else {
        DEBUGLOG ("Unrequested response\n");
    }

    DEBUGLOG ("UDP packet received\n");
    DEBUGLOG ("UDP Packet Type: %s, From: %s:%d, To: %s:%d, Length: %u, Data:\n\n",
        packet.isBroadcast () ? "Broadcast" : packet.isMulticast () ? "Multicast" : "Unicast",
        packet.remoteIP ().toString ().c_str (),
        packet.remotePort (),
        packet.localIP ().toString ().c_str (),
        packet.localPort (),
        packet.length ());
    //reply to the client
    dumpNTPPacket (packet.data (), packet.length ());
    DEBUGLOG ("\n");
}

void ICACHE_RAM_ATTR NTPClient::processRequestTimeout () {
    status = unsyncd;
    //timer1_disable ();
    responseTimer.detach ();
    DEBUGLOG ("NTP response Timeout\n");
    if (onSyncEvent)
        onSyncEvent (noResponse);
}

void ICACHE_RAM_ATTR NTPClient::s_processRequestTimeout (void* arg) {
    NTPClient* self = reinterpret_cast<NTPClient*>(arg);
    self->processRequestTimeout ();
}
#endif

int8_t NTPClient::getTimeZone () {
    return _tzOffset/MINS_PER_HOUR;
}

int8_t NTPClient::getTimeZoneMinutes () {
    return _tzOffset%MINS_PER_HOUR;
}

/*void NTPClient::setLastNTPSync(time_t moment) {
    _lastSyncd = moment;
}*/

time_t NTPClient::s_getTime () {
    return NTP.getTime ();
}

#if NETWORK_TYPE == NETWORK_W5100
bool NTPClient::begin (String ntpServerName, int8_t timeZone, bool daylight, int8_t minutes, EthernetUDP* udp_conn) {
#elif NETWORK_TYPE == NETWORK_WIFI101
bool NTPClient::begin (String ntpServerName, int8_t timeZone, bool daylight, int8_t minutes, WiFiUDP* udp_conn) {
#elif NETWORK_TYPE == NETWORK_ESP8266 || NETWORK_TYPE == NETWORK_ESP32
bool NTPClient::begin (String ntpServerName, int8_t timeZone, bool daylight, int8_t minutes, AsyncUDP* udp_conn) {
#endif
    if (!setNtpServerName (ntpServerName)) {
        DEBUGLOG ("Time sync not started\r\n");
        return false;
    }
    if (!setTimeZone (timeZone, minutes)) {
        DEBUGLOG ("Time sync not started\r\n");
        return false;
    }
    if (udp_conn)
        udp = udp_conn;
    else
#if NETWORK_TYPE == NETWORK_W5100
        udp = new EthernetUDP ();
#elif NETWORK_TYPE == NETWORK_WIFI101
        udp = new WiFiUDP ();
#else
        udp = new AsyncUDP ();
#endif

    //_timeZone = timeZone;
    setDayLight (daylight);
    _lastSyncd = 0;

    if (_shortInterval == 0 && _longInterval == 0) {
        if (!setInterval (DEFAULT_NTP_SHORTINTERVAL, DEFAULT_NTP_INTERVAL)) {
            DEBUGLOG ("Time sync not started\r\n");
            return false;
        }
    }
    DEBUGLOG ("Time sync started\r\n");

    setSyncInterval (getShortInterval ());
    setSyncProvider (s_getTime);

    return true;
}

bool NTPClient::stop () {
    setSyncProvider (NULL);
    DEBUGLOG ("Time sync disabled\n");

    return true;
}

bool NTPClient::setInterval (int interval) {
    if (interval >= 10) {
        if (_longInterval != interval) {
            _longInterval = interval;
            DEBUGLOG ("Sync interval set to %d\n", interval);
            if (timeStatus () == timeSet)
                setSyncInterval (interval);
        }
        return true;
    } else
        return false;
}

bool NTPClient::setInterval (int shortInterval, int longInterval) {
    if (shortInterval >= 10 && longInterval >= 10) {
        _shortInterval = shortInterval;
        _longInterval = longInterval;
        if (timeStatus () != timeSet) {
            setSyncInterval (shortInterval);
        } else {
            setSyncInterval (longInterval);
        }
        DEBUGLOG ("Short sync interval set to %d\n", shortInterval);
        DEBUGLOG ("Long sync interval set to %d\n", longInterval);
        return true;
    } else
        return false;
}

int NTPClient::getInterval () {
    return _longInterval;
}

int NTPClient::getShortInterval () {
    return _shortInterval;
}

void NTPClient::setDayLight (bool daylight) {
    // Do the maths to change current time, but only if we are not yet sync'ed,
    // we don't want to trigger the UDP query with the now() below
    if (_lastSyncd > 0) {
        if ((_daylight != daylight) && isSummerTimePeriod (now ())) {
            if (daylight) {
                setTime (now () + SECS_PER_HOUR);
            } else {
                setTime (now () - SECS_PER_HOUR);
            }
        }
    }

    _daylight = daylight;
    DEBUGLOG ("--Set daylight saving %s\n", daylight ? "ON" : "OFF");

}

bool NTPClient::getDayLight () {
  return (_tzDSTName != NULL)? true : false ;
}

String NTPClient::getTimeStr (time_t moment) {
    char timeStr[15];
    if ( _tzName == NULL ) {
      sprintf (timeStr, "%02d:%02d:%02d", hour (moment), minute (moment), second (moment));
    } else {
      sprintf (timeStr, "%02d:%02d:%02d %s", hour (moment), minute (moment), second (moment), (_useDST) ? _tzDSTName : _tzName );
    }

    return timeStr;
}

String NTPClient::getDateStr (time_t moment) {
    char dateStr[12];
    sprintf (dateStr, "%02d/%02d/%4d", day (moment), month (moment), year (moment));

    return dateStr;
}

String NTPClient::getTimeDateString (time_t moment) {
    return getTimeStr (moment) + " " + getDateStr (moment);
}

String NTPClient::getDateTimeString (time_t moment) {
    return getDateStr (moment) + " " + getTimeStr (moment);
}

time_t NTPClient::getLastNTPSync () {
    return _lastSyncd;
}

void NTPClient::onNTPSyncEvent (onSyncEvent_t handler) {
    onSyncEvent = handler;
}

time_t NTPClient::getUptime () {
    _uptime = _uptime + (millis () - _uptime);
    return _uptime / 1000;
}

String NTPClient::getUptimeString () {
    uint16_t days;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;

    time_t uptime = getUptime ();

    seconds = uptime % SECS_PER_MIN;
    uptime -= seconds;
    minutes = (uptime % SECS_PER_HOUR) / SECS_PER_MIN;
    uptime -= minutes * SECS_PER_MIN;
    hours = (uptime % SECS_PER_DAY) / SECS_PER_HOUR;
    uptime -= hours * SECS_PER_HOUR;
    days = uptime / SECS_PER_DAY;

    char uptimeStr[20];
    sprintf (uptimeStr, "%4u days %02d:%02d:%02d", days, hours, minutes, seconds);

    return uptimeStr;
}

time_t NTPClient::getLastBootTime () {
    if (timeStatus () == timeSet) {
        return (now () - getUptime ());
    }
    return 0;
}

time_t NTPClient::getFirstSync () {
    /*if (!_firstSync) {
        if (timeStatus () == timeSet) {
            _firstSync = now () - getUptime ();
        }
    }*/
    return _firstSync;
}

time_t getDstDate ( int dst_year, uint8_t dst_month, uint8_t dst_day, uint8_t dst_week, int16_t dst_min ) {
   tmElements_t base;
   time_t base_month;;
   uint8_t wday; // first_day_of_month
    char buffer[100];

   base.Second = 0;
   base.Minute = 0;
   base.Hour = 0;
   // base.Minute = dst_min % MINS_PER_HOUR;
   // base.Hour = dst_min/MINS_PER_HOUR;
   base.Day = 1;
   base.Month = dst_month;
   base.Year = CalendarYrToTm(dst_year);
   if ( dst_day < 7 ) {
     if ( dst_week == 5 ) { // last
       base.Month += 1; // next month
     }
     base_month = makeTime(base);
     if ( dst_week == 5 ) { // last
       base_month -= SECS_PER_DAY; // now last day of month
     }
     wday = dayOfWeek(base_month)-1; // need 0 - 7 range
     if ( dst_week < 5 ) {
       base_month +=    (SECS_PER_DAY)*((dst_week-1)*7+((dst_day+7-wday)%7));
     } else {
       base_month -=    (SECS_PER_DAY)*(((wday+7-dst_day)%7));
     }
   } else if ( dst_day == 8 ) { 
     // includes leap day range 0 - 365
     base.Month = 1;
     base_month = makeTime(base);
     base_month += (SECS_PER_DAY)*(dst_week+dst_month);
   } else if ( (dst_week+dst_month) <= (31+28) ){
     // excludes leap day range 1 - 365
     // befor Mar 1
     base.Month = 1;
     base_month = makeTime(base);
     base_month += (SECS_PER_DAY)*(dst_week+dst_month-1);
   } else {
     base.Month = 3;
     // excludes leap day range 1 - 365
     base_month = makeTime(base);
     base_month += (SECS_PER_DAY)*(dst_week+dst_month-(31+28)-1);
   }
   base_month += dst_min*(SECS_PER_MIN);
  return base_month;
  
 }

bool NTPClient::isSummerTimePeriod (time_t moment)
// input parameters: time to be tested
{
    time_t end_dst;
    time_t start_dst;
    int n_year;

    n_year = year(moment);

    if ( _tzDSTName == NULL ) return false; // No DST
    start_dst = getDstDate ( n_year, _dstStartMonth, _dstStartDay, _dstStartWeek, _dstStartMin);
    end_dst = getDstDate ( n_year, _dstEndMonth, _dstEndDay, _dstEndWeek, _dstEndMin);
    return true;

    if (start_dst < end_dst) { // Northern
      return ((moment >= start_dst)  && (moment < end_dst))  ? true : false;
    } else { // Southern
      return ((moment > end_dst)  && (moment <= start_dst))  ? false : true;
    }
}

bool NTPClient::summertime (int n_year, byte n_month, byte n_day, byte n_hour)
// input parameters: "normal time" for year, month, day, hour
{
    //uint8_t s_month;
    //uint8_t s_week;
    //uint8_t s_day;
    //uint8_t e_month;
    //uint8_t e_week;
    //uint8_t e_day;
    //uint8_t dst_hour;
    tmElements_t cur_tm;
    time_t cur;
    //time_t end_dst;
    //time_t start_dst;

    if ( _tzDSTName == NULL ) return false; // No DST

    cur_tm.Second = 0;
    cur_tm.Minute = 1;
    cur_tm.Hour  = (uint8_t)n_hour;
    cur_tm.Day   = (uint8_t)n_day;
    cur_tm.Month = (uint8_t)n_month;
    cur_tm.Year  = (uint8_t)CalendarYrToTm(n_year);
    cur = makeTime(cur_tm);
    return isSummerTimePeriod (cur);
}

time_t NTPClient::getDstStart () {
  return getDstDate ( year(), _dstStartMonth, _dstStartDay, _dstStartWeek, _dstStartMin);

}

time_t NTPClient::getDstEnd () {
  return getDstDate ( year(), _dstEndMonth, _dstEndDay, _dstEndWeek, _dstEndMin);
}


void NTPClient::setLastNTPSync (time_t moment) {
    _lastSyncd = moment;
}

uint16_t NTPClient::getNTPTimeout () {
    return ntpTimeout;
}

boolean NTPClient::setNTPTimeout (uint16_t milliseconds) {

    if (milliseconds >= MIN_NTP_TIMEOUT) {
        ntpTimeout = milliseconds;
        DEBUGLOG ("Set NTP timeout to %u ms\n", milliseconds);
        return true;
    }
    DEBUGLOG ("NTP timeout should be higher than %u ms. You've tried to set %u ms\n", MIN_NTP_TIMEOUT, milliseconds);
    return false;

}

time_t NTPClient::decodeNtpMessage (uint8_t *messageBuffer) {
    unsigned long secsSince1900;
    // convert four bytes starting at location 40 to a long integer
    secsSince1900 = (unsigned long)messageBuffer[40] << 24;
    secsSince1900 |= (unsigned long)messageBuffer[41] << 16;
    secsSince1900 |= (unsigned long)messageBuffer[42] << 8;
    secsSince1900 |= (unsigned long)messageBuffer[43];

    DEBUGLOG ("Secs: %u \n", secsSince1900);

    if (secsSince1900 == 0) {
        DEBUGLOG ("--Timestamp is Zero\n");
        return 0;
    }
#define SEVENTY_YEARS 2208988800UL
    time_t timeTemp = secsSince1900 - SEVENTY_YEARS;
    timeTemp += (_tzOffset) *  SECS_PER_MIN;

    _useDST = false;

    if (_tzDSTName != NULL ) {
        if (summertime (year (timeTemp), month (timeTemp), day (timeTemp), hour (timeTemp) )) {
	  timeTemp += (_tzDSTOffset - _tzOffset) *  SECS_PER_MIN;
	  _useDST = true;
            DEBUGLOG ("Summer Time\n");
        } else {
            DEBUGLOG ("Winter Time\n");
        }
    } else {
        DEBUGLOG ("No daylight\n");
    }
    return timeTemp;
}

NTPClient NTP;
