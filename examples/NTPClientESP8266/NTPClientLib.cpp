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

#ifdef ARDUINO_ARCH_ESP8266

#include "NtpClientLib.h"

#define DBG_PORT Serial

#ifdef DEBUG_NTPCLIENT
#define DEBUGLOG(...) DBG_PORT.printf(__VA_ARGS__)
#else
#define DEBUGLOG(...)
#endif


NTPClient NTP;

NTPClient::NTPClient()
{
    udp = new WiFiUDP ();
}

bool NTPClient::setNtpServerName(String ntpServerName, int idx)
{
    char * name = (char *)malloc ((ntpServerName.length () + 1) * sizeof (char));
    if (!name)
        return false;
    ntpServerName.toCharArray (name, ntpServerName.length () + 1);
    DEBUGLOG ("NTP server set to %s\n", name);
    free (_ntpServerName);
    _ntpServerName = name;
    return true;
}

bool NTPClient::setNtpServerName (char* ntpServerName, int idx) {
    char *name = ntpServerName;
    if (name == NULL)
        return false;
    DEBUGLOG ("NTP server set to %s\n", name);
    free (_ntpServerName);
    _ntpServerName = name;
    return true;
}

String NTPClient::getNtpServerName(int idx)
{
	if ((idx >= 0) && (idx <= 0)) {
        return String (_ntpServerName);
    }
	return "";
}

char* NTPClient::getNtpServerNamePtr (int idx) {
    if ((idx >= 0) && (idx <= 0)) {
        return _ntpServerName;
    }
    return "";
}

bool NTPClient::setTimeZone(int timeZone)
{
	if ((timeZone >= -11) && (timeZone <= 13)) {
        // Temporarily set time to new time zone, before trying to synchronize
        int8_t timeDiff = timeZone - _timeZone;
        _timeZone = timeZone;
        setTime(now() + timeDiff * 3600);

        setTime (getTime ());
        DEBUGLOG("NTP time zone set to: %d\r\n", timeZone);
	    return true;
	}
	return false;
}

boolean sendNTPpacket (IPAddress &address, UDP *udp) {
    char ntpPacketBuffer[NTP_PACKET_SIZE]; //Buffer to store request message

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
    udp->beginPacket (address, 123); //NTP requests are to port 123
    udp->write(ntpPacketBuffer, NTP_PACKET_SIZE);
    udp->endPacket ();
    return true;
}

time_t NTPClient::getTime () {
    //DNSClient dns;
    //WiFiUDP *udpClient = new WiFiUDP(*udp);
    IPAddress timeServerIP; //NTP server IP address
    char ntpPacketBuffer[NTP_PACKET_SIZE]; //Buffer to store response message


    DEBUGLOG ("Starting UDP");
    udp->begin (DEFAULT_NTP_PORT);
    DEBUGLOG ("Remote port: %d\n",udp->remotePort());
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
    sendNTPpacket (NTP.getNtpServerName ().c_str (), udp);
    uint32_t beginWait = millis ();
    while (millis () - beginWait < 1500) {
        int size = udp->parsePacket ();
        if (size >= NTP_PACKET_SIZE) {
            DEBUGLOG ("-- Receive NTP Response\n");
            udp->read (ntpPacketBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
            time_t timeValue = NTP.decodeNtpMessage (ntpPacketBuffer);
            setSyncInterval (NTP.getLongInterval ());
            NTP.getFirstSync (); // Set firstSync value if not set before
            DEBUGLOG ("Sync frequency set low\n");
            udp->stop ();
            NTP.setLastNTPSync (timeValue);
            DEBUGLOG ("Succeccful NTP sync at %s", NTP.getTimeDateString (NTP.getLastNTPSync ()).c_str());

            if (onSyncEvent)
                onSyncEvent (timeSyncd);
            return timeValue;
        }
    }
    DEBUGLOGCR (F ("-- No NTP Response :-("));
    udp.stop ();
    setSyncInterval (NTP.getShortInterval ()); // Retry connection more often
    if (onSyncEvent)
        onSyncEvent (noResponse);
    return 0; // return 0 if unable to get the time 
}

int NTPClient::getTimeZone()
{
    return _timeZone;
}

/*void NTPClient::setLastNTPSync(time_t moment) {
	_lastSyncd = moment;
}*/

time_t NTPClient::s_getTime() {
	return NTP.getTime();
}


#endif // ARDUINO_ARCH_ESP8266