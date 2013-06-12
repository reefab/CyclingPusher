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

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Time.h>
#include <IPAddress.h>
#include <EEPROM.h>

EthernetClient client;

#include "config.h"
#include "ntp.h"
#include "persistent.h"

// This needs to be pin 2 to use interrupt 0
#define reedPin 2
// Number of 'wheel' turns needed to recalculate speed/total distance
#define interval 5
// Minimal number of millisecond between reed switch changes to prevent bounce
#define reedRes 100
// Timeout in seconds, if the reed switch is not activated during this time, pause everything
#define timeOut 15
// Time in seconds before the backlight of the LCD is switched off if there is no activity
#define displaySleep 60
// Minimum distance for a valid activity (in meters)
#define minDistance 500
// least amount of effective time for a valid activity (in secs)
#define minTime 60
// Amount of inactive time before either automatic upload or discarding of current session
#define maxTime 60
// Change the data displayed on the second line of the lcd every X seconds
#define changeSecondLine 3
// Save session data every X seconds
#define saveInterval 120
// Beep shortly every X meters
#define beepInterval 5000

// Global Vars
unsigned int nbRotation = 0;
unsigned int updateCount = 0;
unsigned int totalDistance = 0;
unsigned int lastBeep = 0;
unsigned int beepCount = 0;
unsigned long lastUpdate = 0;
unsigned long currentTime = 0;
unsigned long startTime = 0;
unsigned long enterLoop = 0;
unsigned long exitLoop = 0;
unsigned long time_elasped = 0;
unsigned long effectiveTime = 0;
unsigned long lastSave = 0;
volatile boolean paused = false;
volatile boolean resetRequested = false;
volatile boolean start = false;
volatile unsigned long lastReedPress = 0;
volatile unsigned int rotationCount = 0;
// For display during init and sending via API
String startTimeStr;
float currentSpeed = 0;
boolean done = false;
boolean uploaded = false;

#define lcdblPin 3 // LCD backlight
#include "lcdwrapper.h"
LcdWrapper Lcd(4, 5, 6, 7, 8, 9, lcdblPin);

#include "display.h"
#include "runkeeper.h"

void setup() {
  delay( 50 );   // allow some time (50 ms) after powerup and sketch start, for the Wiznet W5100 Reset IC to release and come out of reset.
  Serial.begin(9600);
  delay(5000);
  Lcd.begin();

  pinMode(reedPin, INPUT);

  pinMode(lcdblPin, OUTPUT);
  Lcd.switchBacklight(true);
  Lcd.infoMessage("  Starting up.");

  // start Ethernet
  Ethernet.begin(mac, ip, gateway, dns_server);
  delay(5000);
  Lcd.setSecondLine("Getting Time...");
  // get time
  setStartTime();
  delay(1000);
  // Retry if unable to get time from NTP
  while(year() == 1970 or getTimeString().length() <= 10) {
    delay(50000);
    Lcd.errorMessage("Retrying ...");
    setStartTime();
  }
  // Upload saved session if present
  if (savePresent()) {
      Lcd.infoMessage("Prev. act. found");
      Lcd.setSecondLine("Uploading");
      delay(1000);
      uploaded = uploadResult(getSavedStartTimeStr(), getSavedDistance(), getSavedTime());
      if(uploaded) {
        resetRequested=true;
        eraseProgress();
        Lcd.infoMessage("Saved data uploaded");
        delay(1000);
      }
  }
  Lcd.clear();

  // Reed switch handling by interrupt
  lastReedPress = millis();
  attachInterrupt(0, turnCounter, RISING);

  // Beeper init
  pinMode(A0, OUTPUT);
  digitalWrite(A0, HIGH);
}

void reset(boolean startNew=false)
{
  rotationCount = 0;
  updateCount = 0;
  totalDistance = 0;
  currentSpeed = 0;
  done = false;
  uploaded = false;
  lastReedPress = millis();
  lastUpdate = 0;
  currentTime = 0;
  startTime = 0;
  time_elasped = 0;
  effectiveTime = 0;
  if (startNew) {
    startTimeStr = getTimeString();
    start = false;
  }
  paused = false;
}

void loop() {
  enterLoop = millis();
  delay(100);
  // Activity finished & api push
  if (done == true && !client.connected() && !uploaded) {
    Lcd.switchBacklight(true);
    Lcd.infoMessage(" Activity ended ");
    saveProgress(startTimeStr, totalDistance, effectiveTime);
    delay(500);
    Lcd.infoMessage(" Activity saved ");
    // Try to upload the saved result
    uploaded = uploadResult(startTimeStr, totalDistance, effectiveTime);
    if(uploaded) {
      resetRequested=true;
      eraseProgress();
    }
    delay(5000);
    Lcd.switchBacklight(false);
  } else {
    // Activity in progres
    // Start a new session if requested
    if (resetRequested) {
      delay(250);
      reset(start);
      if (start) {
        Lcd.infoMessage("Activity started");
      }
      delay(250);
      resetRequested = false;
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
    } else if((millis() - lastReedPress) < ((unsigned long) 1000 * maxTime)) {
      // Automatic activity pause
      paused = true;
      currentSpeed = 0;
      // Display sleep if needed
      if ((( (millis() - lastReedPress) > ((unsigned long) 1000 * displaySleep))))
      {
        Lcd.switchBacklight(false);
      }
    } else {
      // upload session if there is pertinent data, otherwise just reset
      if (isSessionValid()) {
        done = true;
      } else if(rotationCount > 0) {
        Lcd.errorMessage("Discarding data.");
        delay(1000);
        resetRequested = true;
        eraseProgress();
      }
    }
    // Save session data regularly
    if(isSessionValid() && ((millis() - lastSave) > ((unsigned long) 1000 * saveInterval))) {
          saveProgress(startTimeStr, totalDistance, effectiveTime);
          lastSave = millis();
    }
    // Beep if needed
    if(totalDistance >= (beepCount * beepInterval)) {
      beepCount++;
      tone(A0, 2349, 250);
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
    Lcd.switchBacklight(true);
    if (paused) paused = false;

    rotationCount++;
  }
  lastReedPress = millis();
}

boolean isSessionValid() {
  return ((totalDistance > (unsigned int) minDistance) && (effectiveTime > ((unsigned long) minTime * 1000)));
}
