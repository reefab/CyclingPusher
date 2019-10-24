// vim: set ft=arduino:

#include <TFT_eSPI.h>
#include <SPI.h>

#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Time.h>

#include <config.h>
#include <display.h>
#include <Task.h>
#include <MD_CirQueue.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

TFT_eSPI tft = TFT_eSPI();

// Global Vars
boolean paused = false;
volatile unsigned int nbRevolution = 0;
volatile unsigned long lastHallActivation = 0;
unsigned long currentTime = 0;
unsigned long startTime = 0;
unsigned long timeElasped = 0;
int rpm = 0;
const float meterPerTurn = METER_PER_TURN;
boolean done = false;
boolean uploaded = false;
// For display during init and sending via API
String startTimeStr;
TaskManager taskManager;

const unin8_t QUEUE_SIZE = 10;
MD_CirQueue Queue(QUEUE_SIZE, sizeof(unint32_t));

// utility function for digital clock display: prints preceding colon and leading 0
String prettyDigits(int digits) {
    String output = ":";
    if(digits < 10)
        output += '0';
    output += digits;
    return output;
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

void reset(boolean startNew=false) {
  Serial.println("Session Reset");
  nbRevolution = 0;
  done = false;
  uploaded = false;
  lastHallActivation = millis();
  currentTime = 0;
  startTime = 0;
  timeElasped = 0;
  rpm = 0;
  if (startNew) startTimeStr = getTimeString();
  paused = false;
}

// Start a new session if requested
void startNewSession() {
    Serial.println("Activity started");
    reset(true);
}

unsigned int totalDistance () {
    return nbRevolution * meterPerTurn;
}

void turnCounter() {
    unsigned long delta = millis() - lastHallActivation;
    if (delta > (unsigned long) HALL_RES) nbRevolution++;
    lastHallActivation = millis();
}

boolean isSessionValid() {
    return ((totalDistance > (unsigned int) MIN_DISTANCE) &&
            (effectiveTime > ((unsigned long) MIN_TIME * 1000)));
}

/* void updateData(uint32_t deltaTime) { */
/*     enterLoop = millis(); */
/*     delay(100); */
/*     // Activity finished & api push */
/*     if (done == true) { // && !client.connected() && !uploaded) { */
/*         Serial.println(" Activity ended "); */
/*         /1* Lcd.switchBacklight(true); *1/ */
/*         /1* saveProgress(startTimeStr, totalDistance, effectiveTime); *1/ */
/*         /1* delay(500); *1/ */
/*         /1* Serial.println(" Activity saved "); *1/ */
/*         // Try to upload the saved result */
/*         /1* uploaded = uploadResult(startTimeStr, totalDistance, effectiveTime); *1/ */
/*         /1* if(uploaded) { *1/ */
/*         /1*     resetRequested=true; *1/ */
/*         /1*     eraseProgress(); *1/ */
/*         /1* } *1/ */
/*         /1* delay(5000); *1/ */
/*         /1* Lcd.switchBacklight(false); *1/ */
/*     } else { */
/*         // Activity in progres */
/*         // Update turns and distance */
/*         nbRotation = rotationCount - updateCount; */
/*         if (nbRotation >= INTERVAL) */
/*         { */
/*             currentTime = millis(); */
/*             timeElasped = currentTime - lastUpdate; */
/*             float distance = nbRotation * meterPerTurn; */
/*             currentSpeed = ((float) distance / (float) timeElasped) * 3600; */
/*             totalDistance = rotationCount * meterPerTurn; */
/*             updateCount = rotationCount; */
/*             lastUpdate = currentTime; */
/*         } */
/*         // Update time */
/*         if((millis() - lastHallActivation < ((unsigned long) 1000 * TIMEOUT)) && (rotationCount > 0)) */
/*         { */
/*             exitLoop = millis(); */
/*             effectiveTime = effectiveTime + (exitLoop - enterLoop); */
/*         } else if(((millis() - lastHallActivation) < ((unsigned long) 1000 * MAX_TIME)) && (!paused)) { */
/*             // Automatic activity pause */
/*             Serial.println("Autopausing."); */
/*             paused = true; */
/*             currentSpeed = 0; */
/*             // Display sleep if needed */
/*             /1* if ((( (millis() - lastHallActivation) > ((unsigned long) 1000 * displaySleep)))) *1/ */
/*             /1* { *1/ */
/*             /1*     Lcd.switchBacklight(false); *1/ */
/*             /1* } *1/ */
/*         } else { */
/*             // upload session if there is pertinent data, otherwise just reset */
/*             if (isSessionValid()) { */
/*                 done = true; */
/*             } else if(rotationCount > 0) { */
/*                 Serial.println("Discarding data."); */
/*                 delay(1000); */
/*                 reset(); */
/*                 /1* eraseProgress(); *1/ */
/*             } */
/*         } */
/*         // Save session data regularly */
/*         /1* if(isSessionValid() && ((millis() - lastSave) > ((unsigned long) 1000 * saveInterval))) { *1/ */
/*         /1*     saveProgress(startTimeStr, totalDistance, effectiveTime); *1/ */
/*         /1*     lastSave = millis(); *1/ */
/*         /1* } *1/ */
/*         // Beep if needed */
/*         /1* if(totalDistance >= (beepCount * beepInterval)) { *1/ */
/*         /1*     beepCount++; *1/ */
/*         /1*     tone(A0, 2349, 250); *1/ */
/*         /1* } *1/ */
/*     } */
/* } */

void updateDisplay(uint32_t deltaTime) {
    displayInfo();
}

void updateData(uint32_t deltaTime) {
    
}

FunctionTask taskUpdateData(updateData, MsToTaskTime(100));
FunctionTask taskUpdateDisplay(updateDisplay, MsToTaskTime(1000));

void setup(void) {
    // Diag led
    pinMode(BUILTIN_LED, OUTPUT);
    // Hall sensor
    pinMode(HALL_PIN, INPUT);

    Serial.begin(9600);
    // initialize the LCD
    tft.init();
    tft.setRotation(TFT_ROTATION);
    tft.fillScreen(TFT_BLACK);

    // Start Wifi
    WiFiManager wifiManager;
    wifiManager.autoConnect("CyclingPusher");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        tft.print(".");
    }

    tft.setTextColor(TFT_WHITE);
    tft.println("Connected to Wifi");
    tft.print("IP: ");
    tft.setTextColor(TFT_BLUE);
    tft.println(WiFi.localIP());

    // Get time via NTP
    uint32_t timestamp;
    bool updateResult = false;
    do {
        delay(100);
        timeClient.begin();
        updateResult = timeClient.update();
        timestamp = timeClient.getEpochTime();
    } while (!updateResult);
    setTime(timestamp);

    tft.setTextColor(TFT_WHITE);
    tft.print("Time: ");
    tft.setTextColor(TFT_BLUE);
    tft.println(getTimeString());
    Serial.print("Time: ");
    Serial.println(getTimeString());

    // Getting ready to start recording activity
    lastHallActivation = millis();
    attachInterrupt(HALL_PIN, turnCounter, FALLING);
    turnCounter();

    // Enable recurring tasks
    taskManager.StartTask(&taskUpdateData);
    taskManager.StartTask(&taskUpdateDisplay);

    // Display meter
    analogMeter();
}

void loop() {
    taskManager.Loop();
}
