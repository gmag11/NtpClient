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

#define MINS_PER_HOUR 60

int NTPClient::setTimeZone ( int16_t  timeZoneOffset,     // full offset from GMT 0 in minutes 
			      char * timeZoneName,
			      char * timeZoneDSTName,    // NULL if disable
			      int16_t  timeZoneDSTOffset,  // full offset from GMT 0 in minutes  for DST
			      uint8_t dstStartMonth,     // start of Summer time if enabled  Month 1 - 12, 0 disabled dst
			      uint8_t dstStartWeek,      // start of Summer time if enabled Week 1 - 5: (5 means last)
			      uint8_t dstStartDay,       // start of Summer time if enabled Day 0- 6  (0- Sun)
			     // if startDay == 7, then sum of (StartMonth+StartWeek) is the day of the year DST starts
			      uint16_t dstStartMin,     // start of Summer time if enabled in minutes
			      uint8_t dstEndMonth,       // end of Summer time if enabled  Month 1 - 12
			      uint8_t dstEndWeek,        // end of Summer time if enabled Week 1 - 5: (5 means last)
			      uint8_t dstEndDay,         // end of Summer time if enabled Day 0-6  (0- Sun)
			     // if EndDay == 7, then sum of (EndMonth+EndWeek) is the day of the year DST ends
			      uint16_t dstEndMin) {       // end of Summer time if enabled in minutes

  if ( timeZoneOffset < -12 * MINS_PER_HOUR  || timeZoneOffset > 14*MINS_PER_HOUR ) return 1;
  if ( timeZoneDSTName != NULL ) {
    if ( timeZoneDSTOffset < -12 * MINS_PER_HOUR  || timeZoneDSTOffset > 14*MINS_PER_HOUR ) return 2;
    if ( (dstStartMonth  < 1  || dstStartMonth > 12 ) && dstStartDay != 7   ) return 3;
    if ( (dstStartWeek  < 1  || dstStartWeek > 5  ) && dstStartDay != 7 ) return 4;
    if ( dstStartDay  < 0  || dstStartDay > 7  ) return 5;
    if ( dstStartMin  > 24*MINS_PER_HOUR-1  ) return 6;
    if ( (dstEndMonth  < 1  || dstEndMonth > 12) && dstEndDay != 7  ) return 7;
    if ( (dstEndWeek  < 1  || dstEndWeek > 5) && dstEndDay != 7  ) return 8;
    if ( dstEndDay  < 0  || dstEndDay > 7  ) return 9;
    if ( dstEndMin  > 24*MINS_PER_HOUR-1  ) return 10;
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

bool sendNTPpacket (const char* address, UDP *udp) {
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

int16_t NTPClient::getOffset () {
  
  if ( _useDST == true ) {
    return _tzDSTOffset;
  }
  return _tzOffset;
}

time_t NTPClient::getTime () {
    //DNSClient dns;
    //WiFiUDP *udpClient = new WiFiUDP(*udp);
    IPAddress timeServerIP; //NTP server IP address
    char ntpPacketBuffer[NTP_PACKET_SIZE]; //Buffer to store response message


    DEBUGLOG ("Starting UDP\n");
    udp->begin (DEFAULT_NTP_PORT);
    //DEBUGLOG ("UDP port: %d\n",udp->localPort());
    while (udp->parsePacket () > 0); // discard any previously received packets
                                    /*dns.begin(WiFi.dnsServerIP());
                                    uint8_t dnsResult = dns.getHostByName(NTP.getNtpServerName().c_str(), timeServerIP);
                                    DEBUGLOG(F("NTP Server hostname: "));
                                    DEBUGLOGCR(NTP.getNtpServerName());
                                    DEBUGLOG(F("NTP Server IP address: "));
                                    DEBUGLOGCR(timeServerIP);
                                    DEBUGLOG(F("Result code: "));
                                    DEBUGLOG(dnsResult);
                                    DEBUGLOG(" ");
                                    DEBUGLOGCR(F("-- IP Connected. Waiting for sync"));
                                    DEBUGLOGCR(F("-- Transmit NTP Request"));*/

                                    //if (dnsResult == 1) { //If DNS lookup resulted ok
    sendNTPpacket (getNtpServerName ().c_str (), udp);
    uint32_t beginWait = millis ();
    while (millis () - beginWait < ntpTimeout ) {
        int size = udp->parsePacket ();
        if (size >= NTP_PACKET_SIZE) {
            DEBUGLOG ("-- Receive NTP Response\n");
            udp->read (ntpPacketBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
            time_t timeValue = decodeNtpMessage (ntpPacketBuffer);
            setSyncInterval (getLongInterval ());
            if (!_firstSync) {
                //    if (timeStatus () == timeSet)
                _firstSync = timeValue;
            }
            //getFirstSync (); // Set firstSync value if not set before
            DEBUGLOG ("Sync frequency set low\n");
            udp->stop ();
            setLastNTPSync (timeValue);
            DEBUGLOG ("Successful NTP sync at %s", getTimeDateString (getLastNTPSync ()).c_str ());

            if (onSyncEvent)
                onSyncEvent (timeSyncd);
            return timeValue;
        }
#ifdef ARDUINO_ARCH_ESP8266
        ESP.wdtFeed ();
#endif
    }
    DEBUGLOG ("-- No NTP Response :-(\n");
    udp->stop ();
    setSyncInterval (getShortInterval ()); // Retry connection more often
    if (onSyncEvent)
        onSyncEvent (noResponse);
    return 0; // return 0 if unable to get the time
}

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
#elif NETWORK_TYPE == NETWORK_ESP8266 || NETWORK_TYPE == NETWORK_WIFI101 || NETWORK_TYPE == NETWORK_ESP32
bool NTPClient::begin (String ntpServerName, int8_t timeZone, bool daylight, int8_t minutes, WiFiUDP* udp_conn) {
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
#else
        udp = new WiFiUDP ();
#endif

    //_timeZone = timeZone;
    setDayLight (daylight);
    _lastSyncd = 0;

    if (!setInterval (DEFAULT_NTP_SHORTINTERVAL, DEFAULT_NTP_INTERVAL)) {
        DEBUGLOG ("Time sync not started\r\n");
        return false;
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
    DEBUGLOG ("--Set daylight saving %s\n", daylight ? "ON" : "OFF");
    setTime (getTime ());
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

time_t getDstDate ( int dst_year, uint8_t dst_month, uint8_t dst_day, uint8_t dst_week, uint16_t dst_min ) {
   tmElements_t base;
   time_t base_month;;
   uint8_t wday; // first_day_of_month
    char buffer[100];

   base.Second = 0;
   base.Minute = dst_min % MINS_PER_HOUR;
   base.Hour = dst_min/MINS_PER_HOUR;
   base.Day = 1;
   base.Month = dst_month;
   base.Year = CalendarYrToTm(dst_year);
   if ( dst_day != 7 ) {
     if ( dst_week == 5 ) { // last
       base.Month += 1; // next month
     }
     base_month = makeTime(base);
     if ( dst_week == 5 ) { // last
       base_month -= SECS_PER_DAY; // now last day of month
     }
     wday = dayOfWeek(base_month);
     if ( dst_week < 5 ) {
       base_month +=    (SECS_PER_DAY)*((dst_week-1)*7+((dst_day+7-wday)%7));
     } else {
       base_month -=    (SECS_PER_DAY)*(((wday+7-dst_day)%7));
     }
   } else {
     base.Month = 1;
     base_month = makeTime(base);
     base_month += (SECS_PER_DAY)*(dst_week+dst_month);
   }
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
    uint8_t s_month;
    uint8_t s_week;
    uint8_t s_day;
    uint8_t e_month;
    uint8_t e_week;
    uint8_t e_day;
    uint8_t dst_hour;
    tmElements_t cur_tm;
    time_t cur;
    time_t end_dst;
    time_t start_dst;

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

time_t NTPClient::decodeNtpMessage (char *messageBuffer) {
    unsigned long secsSince1900;
    // convert four bytes starting at location 40 to a long integer
    secsSince1900 = (unsigned long)messageBuffer[40] << 24;
    secsSince1900 |= (unsigned long)messageBuffer[41] << 16;
    secsSince1900 |= (unsigned long)messageBuffer[42] << 8;
    secsSince1900 |= (unsigned long)messageBuffer[43];

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
