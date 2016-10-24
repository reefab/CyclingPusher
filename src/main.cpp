// vim: set ft=arduino:
#include "Arduino.h"

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Time.h>

#include <config.h>
#include <Fonts/FreeSansBold18pt7b.h>
#define FONT_NAME FreeSansBold18pt7b
#include <display.h>
#include <Task.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

// Global Vars
unsigned int nbRotation = 0;
unsigned int updateCount = 0;
unsigned int totalDistance = 0;
unsigned long lastUpdate = 0;
unsigned long currentTime = 0;
unsigned long startTime = 0;
unsigned long enterLoop = 0;
unsigned long exitLoop = 0;
unsigned long timeElasped = 0;
unsigned long effectiveTime = 0;
/* unsigned long lastSave = 0; */
volatile boolean paused = false;
volatile unsigned long lastHallActivation = 0;
volatile unsigned int rotationCount = 0;
const float meterPerTurn = METER_PER_TURN;
float currentSpeed = 0;
boolean done = false;
boolean uploaded = false;
// For display during init and sending via API
String startTimeStr;
TaskManager taskManager;

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
  Serial.print("Session Reset");
  rotationCount = 0;
  updateCount = 0;
  totalDistance = 0;
  currentSpeed = 0;
  done = false;
  uploaded = false;
  lastHallActivation = millis();
  lastUpdate = 0;
  currentTime = 0;
  startTime = 0;
  timeElasped = 0;
  effectiveTime = 0;
  if (startNew) startTimeStr = getTimeString();
  paused = false;
}

// Start a new session if requested
void startNewSession() {
    Serial.println("Activity started");
    reset(true);
}

void turnCounter() {
    unsigned long delta = millis() - lastHallActivation;
    Serial.print("Hall activation: ");
    Serial.println(delta);
    if (delta > (unsigned long) HALL_RES) {
        // Start a new session at first pedal turn
        if (rotationCount == 0) {
            Serial.println("Should start new session");
            startNewSession();
        }
        rotationCount++;
    }
    lastHallActivation = millis();
}

boolean isSessionValid() {
    return ((totalDistance > (unsigned int) MIN_DISTANCE) &&
            (effectiveTime > ((unsigned long) MIN_TIME * 1000)));
}

void updateData(uint32_t deltaTime) {
    enterLoop = millis();
    delay(100);
    // Activity finished & api push
    if (done == true) { // && !client.connected() && !uploaded) {
        Serial.println(" Activity ended ");
        /* Lcd.switchBacklight(true); */
        /* saveProgress(startTimeStr, totalDistance, effectiveTime); */
        /* delay(500); */
        /* Serial.println(" Activity saved "); */
        // Try to upload the saved result
        /* uploaded = uploadResult(startTimeStr, totalDistance, effectiveTime); */
        /* if(uploaded) { */
        /*     resetRequested=true; */
        /*     eraseProgress(); */
        /* } */
        /* delay(5000); */
        /* Lcd.switchBacklight(false); */
    } else {
        // Activity in progres
        // Update turns and distance
        nbRotation = rotationCount - updateCount;
        if (nbRotation >= INTERVAL)
        {
            currentTime = millis();
            timeElasped = currentTime - lastUpdate;
            float distance = nbRotation * meterPerTurn;
            currentSpeed = ((float) distance / (float) timeElasped) * 3600;
            totalDistance = rotationCount * meterPerTurn;
            updateCount = rotationCount;
            lastUpdate = currentTime;
        }
        // Update time
        if((millis() - lastHallActivation < ((unsigned long) 1000 * TIMEOUT)) && (rotationCount > 0))
        {
            exitLoop = millis();
            effectiveTime = effectiveTime + (exitLoop - enterLoop);
        } else if(((millis() - lastHallActivation) < ((unsigned long) 1000 * MAX_TIME)) && (!paused)) {
            // Automatic activity pause
            Serial.println("Autopausing.");
            paused = true;
            currentSpeed = 0;
            // Display sleep if needed
            /* if ((( (millis() - lastHallActivation) > ((unsigned long) 1000 * displaySleep)))) */
            /* { */
            /*     Lcd.switchBacklight(false); */
            /* } */
        } else {
            // upload session if there is pertinent data, otherwise just reset
            if (isSessionValid()) {
                done = true;
            } else if(rotationCount > 0) {
                Serial.println("Discarding data.");
                delay(1000);
                reset();
                /* eraseProgress(); */
            }
        }
        // Save session data regularly
        /* if(isSessionValid() && ((millis() - lastSave) > ((unsigned long) 1000 * saveInterval))) { */
        /*     saveProgress(startTimeStr, totalDistance, effectiveTime); */
        /*     lastSave = millis(); */
        /* } */
        // Beep if needed
        /* if(totalDistance >= (beepCount * beepInterval)) { */
        /*     beepCount++; */
        /*     tone(A0, 2349, 250); */
        /* } */
    }
}

void updateDisplay(uint32_t deltaTime) {
    displayInfo();
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
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(TFT_ROTATION);
    tft.fillScreen(ST7735_BLACK);
    tft.setTextWrap(true);
    tft.fillScreen(ST7735_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(1);

    // Start Wifi
    WiFiManager wifiManager;
    wifiManager.autoConnect("CyclingPusher");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        tft.print(".");
    }

    tft.setTextColor(ST7735_WHITE);
    tft.println("Connected to Wifi");
    tft.print("IP: ");
    tft.setTextColor(ST7735_BLUE);
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

    tft.setTextColor(ST7735_WHITE);
    tft.print("Time: ");
    tft.setTextColor(ST7735_BLUE);
    tft.println(getTimeString());
    Serial.print("Time: ");
    Serial.println(getTimeString());

    // Getting ready to start recording activity
    lastHallActivation = millis();
    attachInterrupt(HALL_PIN, turnCounter, RISING);
    turnCounter();

    // Clear display
    tft.fillScreen(ST7735_WHITE);
    tft.setFont(&FONT_NAME);

    // Enable recurring tasks
    taskManager.StartTask(&taskUpdateData);
    taskManager.StartTask(&taskUpdateDisplay);
}

void loop() {
    taskManager.Loop();
}
