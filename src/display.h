#include <stdlib.h>

#define numberOfSecondLine 3
#define pause_str "--===Paused===--"
#define speed_str "Speed: "
#define distance_str "Distance: "
#define time_str "Time:  "
#define kmh_str "km/h"

String firstLine;
String secondLine;
unsigned long lastSecondLineChange = 0;
int currentSecondLine = 0;
float avgSpeed = 0;

String floatToString(float float_val)
{
    char number[6];
    if ((int) float_val > 99) {
        return dtostrf(float_val, 5, 1, number);
    } else {
        return dtostrf(float_val, 5, 2, number);
    }
}

String restrictIn16Chars(String line)
{
    if (line.length() > 16) {
        return line.substring(0, 17);
    } else {
        return line;
    }
}

String getCurrentSpeedLine()
{
    String line = String(speed_str);
    line += floatToString(currentSpeed);
    line += kmh_str;
    return line;
}

String getDistanceLine()
{
    String line = String(distance_str);
    if (totalDistance < 1000) {
        line += totalDistance;
        line += "m       ";
    } else {
        line += floatToString((float) totalDistance / 1000U);
        line += "km      ";
    }
    return line;
}

float getAverageSpeed() {
    if (effectiveTime == 0) {
        return 0;
    } else {
        return (float) ((float) totalDistance / effectiveTime) * 3600;
    }
}

String getTimeLine() {
    String time = String(time_str + elaspedTime(effectiveTime));
    time += " ";
    return time;
}

String getAvgSpeedLine() {
    String line = String("Avg:   ");
    line += floatToString(avgSpeed);
    line += kmh_str;
    return line;
}

void displayInfo() {
    if (paused) {
        firstLine = pause_str;
    } else {
        firstLine = getCurrentSpeedLine();
    }
    lcd.clear();
    lcd.print(restrictIn16Chars(firstLine));
    if ( (millis() - lastSecondLineChange) > ((unsigned long) changeSecondLine * 1000UL)) {
        currentSecondLine = (currentSecondLine + 1) % numberOfSecondLine;
        lastSecondLineChange = millis();
        avgSpeed = getAverageSpeed();
    }
    lcd.setCursor(0, 1);
    switch (currentSecondLine) {
        case 0:
            secondLine = getDistanceLine();
            break;
        case 1:
            secondLine = getAvgSpeedLine();
            break;
        case 2:
            secondLine = getTimeLine();
            break;
    }
    lcd.print(restrictIn16Chars(secondLine));
}

// void displayInitScreen() {
//     String startTimeStr = getTimeString();
//     lcd.clear();
//     lcd.print("IP: ");
//     lcd.print(Ethernet.localIP());
//     lcd.setCursor(0, 1);
//     for (int i = 0;i < 16;i++) lcd.print(startTimeStr.charAt(i));
//     delay(2500);
//     lcd.setCursor(0, 1);
//     for (int i = 1;i < 32;i++) lcd.print(" ");
//     lcd.setCursor(3, 1);
//     for (int i = 16;i < startTimeStr.length();i++) lcd.print(startTimeStr.charAt(i));
//     delay(2500);
// }

void switchBacklight(boolean bl) {
    if (bl) {
        digitalWrite(ledblPin, HIGH);
        backlight = true;
    } else {
        digitalWrite(ledblPin, LOW);
        backlight = false;
    }
}
