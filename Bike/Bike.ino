
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
 *      Author: reefab@demenzia.net
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
// Led backlight
#define ledblPin 3
// Number of 'wheel' turns needed to recalculate speed/total distance 
#define interval 5
// Minimal number of millisecond between reed switch changes to prevent bounce
#define reedRes 50
// Timeout in seconds, if the reed switch is not activated during this time, pause everything 
#define timeOut 15
// Time in seconds before the backlight of the LCD is switched off if there is no activity
#define displaySleep 120
// Minimum distance for a valid activity (in meters)
#define minDistance 500
// least amount of effective time for a valid activity (in secs)
#define minTime 60
// Amount of inactive time before either automatic upload or discarding of current session
#define maxTime 300
// Change the data displayed on the second line of the lcd every X seconds
#define changeSecondLine 5

// Global Vars
int nbRotation = 0;
unsigned int rotationCount = 0;
unsigned int updateCount = 0;
// Distance in meters
unsigned int totalDistance = 0;
float currentSpeed = 0;

boolean done = false;
boolean uploaded = false;
boolean backlight = false;
boolean paused = false;
boolean resetRequested = false;
boolean start = false;
unsigned long lastReedPress = 0;
unsigned long lastUpdate = 0;
unsigned long currentTime = 0;
unsigned long startTime = 0;
unsigned long enterLoop = 0;
unsigned long exitLoop = 0;
unsigned long time_elasped = 0;
unsigned long effectiveTime = 0;
// For display during init and sending via API
String startTimeStr;

#include "display.h"

void setup() {
  pinMode(reedPin, INPUT);
  pinMode(ledblPin, OUTPUT);

  // LCD Init 
  lcd.begin(16, 2);
  switchBacklight(true);
  lcd.print("  Starting up.  ");
  lcd.setCursor(0, 1);
  lcd.print("Getting IP...");

  // Reed switch handling by interrupt
  attachInterrupt(0, turnCounter, FALLING);

  // start Ethernet
  if (Ethernet.begin(mac) == 0) {
    lcd.clear();
    lcd.print("Failed to configure");
    lcd.setCursor(0, 1);
    lcd.print("Ethernet using DHCP");
    delay(5000);
  }
  else {
    startTimeStr = setStartTime();
    displayInitScreen();
    // So that the startTime will be reinitialized at the actual
    // start of the session
    startTimeStr = "";
  }
  lcd.clear();
  lastReedPress = millis();
}

void reset(boolean startNew=false)
{
  // Global Vars
  rotationCount = 0;
  updateCount = 0;
  // Distance in meters
  totalDistance = 0;
  currentSpeed = 0;
  done = false;
  uploaded = false;
  lastReedPress = 0;
  lastUpdate = 0;
  currentTime = 0;
  startTime = 0;
  time_elasped = 0;
  effectiveTime = 0;
  if (startNew) startTimeStr = setStartTime();
}

void loop() {
  enterLoop = millis();
  delay(450);
  // Activity finished & api push
  if (done == true && !client.connected() && !uploaded) {
    if (!backlight) switchBacklight(true);   
    lcd.setCursor(0, 0);
    lcd.print(" Activity ended "); 
    delay(1000);
    lcd.clear();
    uploaded = uploadResult(startTimeStr, totalDistance, effectiveTime);
    if(uploaded) resetRequested=true;
    delay(5000);
    switchBacklight(false);
  } 
  else {
    // Activity in progres
    // Start a new session if requested
    if (resetRequested) {
      reset(start);
      resetRequested = false;
      if (start) {
        lcd.setCursor(0, 0);
        lcd.print("Activity started");
        start = false;
        delay(1000);
      }
    }
    // Update turns and distance
    nbRotation = rotationCount - updateCount;
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
    if((millis() - lastReedPress < ((unsigned long) 1000 * timeOut)) && (rotationCount > 0))
    {
      exitLoop = millis();
      effectiveTime = effectiveTime + (exitLoop - enterLoop);
    } 
    else if((millis() - lastReedPress) < ((unsigned long) 1000 * maxTime)) {
      // Automatic activity pause
      paused = true;
      currentSpeed = 0;
      // Display sleep if needed
      if ((( (millis() - lastReedPress) > ((unsigned long) 1000 * displaySleep))) && backlight)
      {
        switchBacklight(false);
      }
    }
    else {
      // upload session if there is pertinent data, otherwise just reset
      if (isSessionValid()) {
        done = true;
      } 
      else if(rotationCount > 0) {
        if (!backlight) switchBacklight(true);   
        lcd.clear();
        lcd.print("Discarding data.");
        delay(5000);
        resetRequested = true;
      }
    }
    displayInfo();
  }
}

void turnCounter() {
  if (millis() - lastReedPress > reedRes)
  {
    // Start a new session at first pedal turn
    if (rotationCount == 0) {
      resetRequested = true;
      start = true;
    }
    if (!backlight) switchBacklight(true);   
    if (paused) paused = false;

    rotationCount++;
  }
  lastReedPress = millis();
}

boolean isSessionValid()
{
  return ((totalDistance > minDistance) && (effectiveTime > minTime));
}



