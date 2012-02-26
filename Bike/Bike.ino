
/*
 * Bike
 * 
 *  https://github.com/reefab/CyclingPusher
 *
 *  CyclingPusher is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  CyclingPusher is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with CyclingPusher.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Created on: 25.02.2012
 *      Author: reefab
 */

#include <LiquidCrystal.h>
#include <SPI.h>         
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Time.h>
#include <HTTPClient.h>
// MAC address
static uint8_t mac[6] = { 
  0x90, 0xA2, 0xDA, 0x05, 0x00, 0x43 };
// For NTP init
unsigned int localPort = 8888;      // local port to listen for UDP packets
IPAddress timeServer(192, 43, 244, 18); // time.nist.gov NTP server
const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets 

// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;
EthernetClient client;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 6, 5, 4, 9, 8);
// This needs to be pin 2 to use interrupt 0
int reedPin = 2;
int buttonPin = 3;
unsigned int rotationCount = 0;
unsigned int updateCount = 0;
// Distance per 'wheel' turn
float meterPerTurn = 6.66;
// Number of 'wheel' turns needed to recalculate speed/total distance 
int interval = 5;
// Minimal number of millisecond between reed switch changes to prevent bounce
int reedRes = 50;
// Timeout in seconds, if the reed switch is not activated during this time, pause everything 
int timeOut = 15;
// Distance in meters
unsigned int totalDistance = 0;
float currentSpeed = 0;
boolean screen = true;
boolean done = false;
unsigned long lastReedPress = millis();
unsigned long lastButtonPress = millis();
unsigned long lastUpdate = millis();
unsigned long currentTime = millis();
unsigned long startTime = millis();
unsigned long totalTime = 0;
unsigned long time_elasped = 0;
unsigned long timestamp = 0;
unsigned long effectiveTime = 0;
//unsigned long lastTimeUpdate = 0;
// For display during init and sending to HealthGraph
String startTimeStr;
byte serverIp[] = {
  74,50,63,142};
char apiServer[] = "api.runkeeper.com";
char apiUri[] = "/fitnessActivities";
char accessToken[] = "Bearer XXX";
String json_start = "{\"type\": \"Cycling\",\"start_time\": \"";
String json_middle_1 = "\",\"notes\": \"Arduino powered stationary bike\",\"total_distance\": ";
String json_middle_2 = ",\"duration\": ";
String json_end = "}";
boolean lastConnected = false;


void setup() {
  pinMode(reedPin, INPUT);
  pinMode(buttonPin, INPUT);
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);

  // Reed switch handling by interrupt
  attachInterrupt(0, turnCounter, FALLING);

  attachInterrupt(1, changeScreen, FALLING);
  Serial.begin(9600);

  // start Ethernet and UDP
  if (Ethernet.begin(mac) == 0) {
    lcd.print("Failed to configure");
    lcd.setCursor(0, 1);
    lcd.print("Ethernet using DHCP");
    delay(5000);
  }
  else {
    Udp.begin(localPort);
    timestamp = getTimeStamp();
    setTime(timestamp);
    startTimeStr = getTimeString();
    displayInitScreen();
  }

  lcd.clear();
}

void loop() {
  // Activity finished & api push
  if (done == true && !client.connected()) {
    delay(1000);
    lcd.clear();
    String data = json_start;
    data += startTimeStr;
    data += json_middle_1;
    data += totalDistance;
    data += json_middle_2;
    data += (int) (effectiveTime / 1000);
    data += json_end;
    unsigned int bufSize = data.length() +1;
    char apiData[bufSize];
    data.toCharArray(apiData, bufSize);

    HTTPClient client(apiServer, serverIp);
    http_client_parameter apiHeaders[] = {
      { 
        "Authorization", accessToken      }
      ,
      { 
        "Content-Type", "application/vnd.com.runkeeper.NewFitnessActivity+json"      }
      ,
      {
        NULL, NULL      }
    };
    lcd.print("Uploading result");
    delay(500);
    FILE* result = client.postURI(apiUri, NULL, apiData, apiHeaders);
    int returnCode = client.getLastReturnCode();
    lcd.clear();
    if (result!=NULL) {
      client.closeStream(result);  // this is very important -- be sure to close the STREAM
    } 
    else {
      lcd.print("failed to connect");
    }
    if (returnCode==201) {
      lcd.print("Data uploaded");
      lcd.setCursor(0, 1);
      lcd.print("Session Created");
      // Hang on success
      for(;;);
    } 
    else {
      lcd.print("ERROR: Server returned ");
      lcd.setCursor(0, 1);
      lcd.print(returnCode);
    }
    // Hang if finished
    if (lastConnected) for(;;);
  } 
  else {
    // Activity in progres
    unsigned long enterLoop = millis();
    // Update turns and distance
    int nbRotation = rotationCount - updateCount;
    if (nbRotation >= interval)
    {
      currentTime = millis();
      time_elasped = currentTime - lastUpdate;
      float distance = nbRotation * meterPerTurn;
      currentSpeed = ((float) distance / (float) time_elasped) * 3600;  
      totalDistance = rotationCount * meterPerTurn;
      updateCount = rotationCount;
      lastUpdate = currentTime;
    }
    // Update time
    if(millis() - lastReedPress < (1000 * timeOut))
    {
      // Display
      lcd.setCursor(0,0);
      if (screen == true) {
        displayCurrentScreen();
      } 
      else {
        displayGlobalScreen();
      }
      delay(500);
      unsigned long exitLoop = millis();
      effectiveTime = effectiveTime + (exitLoop - enterLoop);
      // Automatic activity pause
    } 
    else {
      // Deactivate the "Change Screen Button"
      detachInterrupt(1);
      // Activate the finish function instead at button press
      attachInterrupt(1, finishActivity, FALLING);
      displayPauseScreen();
      detachInterrupt(1);
      attachInterrupt(1, changeScreen, FALLING);
    }
  }
  lastConnected = client.connected();
}

void turnCounter() {
  if (millis() - lastReedPress > reedRes)
  {
    rotationCount++;
  }
  lastReedPress = millis();
}

void changeScreen() {
  if (millis() - lastButtonPress > 200)
  {
    screen = !screen;
    lcd.clear();
  }
  lastButtonPress = millis();
}

float getAverageSpeed() {
  totalTime = getTotalTime();
  return ((float) totalDistance / (float) totalTime) * 3600;
}  

unsigned long getTotalTime() {
  return effectiveTime;
}

void displayCurrentScreen() {
  lcd.print("Speed: ");
  lcd.print(currentSpeed);
  lcd.print("km/h");
  lcd.setCursor(0, 1);
  lcd.print("Distance: ");
  if (totalDistance < 1000) {
    lcd.print(totalDistance);
    lcd.print("m");
  } 
  else {
    lcd.print((float) totalDistance / 1000);
    lcd.print("km");
  }
}

void displayGlobalScreen() {
  // FIXME: minutes display > 60
  lcd.print("Avg: ");
  lcd.print(getAverageSpeed());
  lcd.print("km/h");
  lcd.setCursor(0, 1);
  totalTime = getTotalTime();
  int hours = totalTime / 3600000;
  int minutes = totalTime / 60000;
  long seconds = (totalTime % 60000);
  seconds = (long) seconds / 1000;
  String time = "Time: "
    + String(hours)
    + prettyDigits(minutes) 
      + prettyDigits(seconds);
  lcd.print(time);
}

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
  delay(1000);  
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

String getTimeString() {
  String str_time;
  str_time += String(dayShortStr(weekday())) + ", " 
    + String(day()) + " " 
    + String(monthShortStr(month())) + " " 
    + String(year()) 
    + " "
      + String(hour())
      + prettyDigits(minute())
        + prettyDigits(second());
  return str_time; 
}

void displayPauseScreen() {
  lcd.clear();
  lcd.print("Activity paused.");
  delay(1000);
  lcd.clear();
  lcd.print("Press button ");
  lcd.setCursor(0, 1);
  lcd.print("to finish");
  delay(1000);
}

void displayInitScreen() {
  lcd.print("IP: ");
  lcd.print(Ethernet.localIP());
  lcd.setCursor(0, 1);
  for (int i = 0;i < 16;i++) lcd.print(startTimeStr.charAt(i));
  delay(2500); 
  lcd.setCursor(0, 1);
  for (int i = 1;i < 32;i++) lcd.print(" ");
  lcd.setCursor(0, 1);
  for (int i = 16;i < startTimeStr.length();i++) lcd.print(startTimeStr.charAt(i));
  delay(2500);
}

void finishActivity() {
  lcd.clear();
  lcd.print("Activity"); 
  lcd.setCursor(0, 1);
  lcd.print("finished");
  done = true;
}

