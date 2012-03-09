
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

LiquidCrystal lcd(4, 5, 6, 7, 8, 9);
EthernetClient client;

#include "config.h"
#include "runkeeper.h"
#include "ntp.h"

// This needs to be pin 2 to use interrupt 0
#define reedPin 2
// This needs to be pin 3 to use interrupt 1
#define buttonPin 3
// Led backlight
#define ledblPin A0
// Number of 'wheel' turns needed to recalculate speed/total distance 
#define interval 5
// Minimal number of millisecond between reed switch changes to prevent bounce
#define reedRes 50
// Timeout in seconds, if the reed switch is not activated during this time, pause everything 
#define timeOut 15
// Time in seconds before the backlight of the LCD is switched off if there is no activity
#define displaySleep 60
// Minimum distance for a valid activity (in meters)
#define minDistance 500
// least amount of time for a valid activity (in secs)
#define minTime 60

// Global Vars
unsigned int rotationCount = 0;
unsigned int updateCount = 0;
// Distance in meters
unsigned int totalDistance = 0;
float currentSpeed = 0;
boolean screen = true;
boolean done = false;
boolean uploaded = false;
boolean backlight = false;
boolean paused = false;
unsigned long lastReedPress = millis();
unsigned long lastButtonPress = millis();
unsigned long lastUpdate = millis();
unsigned long currentTime = millis();
unsigned long startTime = millis();
unsigned long time_elasped = 0;
unsigned long effectiveTime = 0;
// For display during init and sending via API
String startTimeStr;

#include "display.h"

void setup() {
  pinMode(reedPin, INPUT);
  pinMode(buttonPin, INPUT);
  pinMode(ledblPin, OUTPUT);

  // LCD Init 
  lcd.begin(16, 2);
  switchBacklight(true);
  lcd.print("Starting up");
  lcd.setCursor(0, 1);
  lcd.print("Getting IP");

  // Reed switch handling by interrupt
  attachInterrupt(0, turnCounter, FALLING);
  // Action Button
  attachInterrupt(1, actionButton, FALLING);

  // start Ethernet and UDP
  if (Ethernet.begin(mac) == 0) {
    lcd.clear();
    lcd.print("Failed to configure");
    lcd.setCursor(0, 1);
    lcd.print("Ethernet using DHCP");
    delay(5000);
  }
  else {
    Udp.begin(localPort);
    setTime(getTimeStamp());
    startTimeStr = getTimeString();
    displayInitScreen();
  }
  lcd.clear();
  lastButtonPress = millis();
  lastReedPress = millis();
}

void loop() {
  // Activity finished & api push
  if (done == true && !client.connected() && !uploaded) {
    lcd.clear();
    lcd.print("Activity"); 
    lcd.setCursor(0, 1);
    lcd.print("finished");
    delay(1000);
    lcd.clear();

    if ( (totalDistance > minDistance) && (effectiveTime > minTime)) {
      uploaded = uploadResult(startTimeStr, totalDistance, effectiveTime);
    } 
    else {
      lcd.print("Need more data");
      delay(1000);
      done = false;
    }
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
      lcd.clear();
      if (screen == true) {
        displayCurrentScreen();
      } 
      else {
        displayGlobalScreen();
      }
      delay(500);
      unsigned long exitLoop = millis();
      effectiveTime = effectiveTime + (exitLoop - enterLoop);
    } 
    else {
      // Automatic activity pause
      paused = true;
      displayPauseScreen();
      // Display sleep if needed
      if ((( (millis() - lastReedPress) > (1000 * displaySleep))) && backlight)
      {
        switchBacklight(false);
      }
    }
  }
}

void turnCounter() {
  if (millis() - lastReedPress > reedRes)
  {
    rotationCount++;
  }
  lastReedPress = millis();
  if (!backlight) { 
    switchBacklight(true);   
  }
}

void actionButton() {
  if (millis() - lastButtonPress > 200)
  {
    if (paused) {
      done = true;
    } else {
      screen = !screen;
    }
  }
  lastButtonPress = millis();
  if (!backlight) { 
    switchBacklight(true);   
  }
}

float getAverageSpeed() {
  return ((float) totalDistance / (float) effectiveTime) * 3600;
}  

void switchBacklight(boolean bl)
{
  if (bl)
  {
    digitalWrite(ledblPin, HIGH);
    backlight = true;
  } 
  else {
    digitalWrite(ledblPin, LOW);
    backlight = false;
  }
}




