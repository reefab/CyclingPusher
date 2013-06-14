// vim: filetype=arduino
// NTP
const unsigned int localPort = 8888;      // local port to listen for UDP packets
const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
// For NTP init
//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServer(192, 168, 1, 150); // Or a local server
// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;

    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    Udp.beginPacket(address, 123); //NTP requests are to port 123
    Udp.write(packetBuffer,NTP_PACKET_SIZE);
    Udp.endPacket();
}

unsigned long getTimeStamp() {
    sendNTPpacket(timeServer); // send an NTP packet to a time server
    // wait to see if a reply is available
    delay(1500);
    if ( Udp.parsePacket() ) {
        // We've received a packet, read the data from it
        Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer
        //the timestamp starts at byte 40 of the received packet and is four bytes,
        // or two words, long. First, esxtract the two words:
        unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
        unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
        // combine the four bytes (two words) into a long integer
        // this is NTP time (seconds since Jan 1 1900):
        unsigned long secsSince1900 = highWord << 16 | lowWord;
        // now convert NTP time into Unix timestamp:
        const unsigned long seventyYears = 2208988800UL;
        unsigned long epoch = secsSince1900 - seventyYears;
        return epoch;
    }
}

String prettyDigits(int digits){
    // utility function for digital clock display: prints preceding colon and leading 0
    String output = ":";
    if(digits < 10)
        output += '0';
    output += digits;
    return output;
}

String elaspedTime(unsigned long time){
    time = time / 1000UL;
    int hours = numberOfHours(time);
    int minutes = numberOfMinutes(time);
    int seconds = numberOfSeconds(time);

    String timeStr = " ";
    // digital clock display
    timeStr = String(hours)
        + prettyDigits(minutes)
        + prettyDigits(seconds);
    return timeStr;
}

String getTimeString() {
    String str_time;
    str_time += String(dayShortStr(weekday()))
        + String(", ")
        + String(day())
        + String(" ")
        + String(monthShortStr(month()))
        + String(" ")
        + String(year())
        + String(" ")
        + String(hour())
        + prettyDigits(minute())
        + prettyDigits(second());
    return str_time;
}

void setStartTime() {
    Udp.begin(localPort);
    setTime(getTimeStamp());
}
