#include <stdlib.h>

#define numberOfSecondLine 3
#define pause_str "--===Paused===--"
#define speed_str "Speed: "
#define distance_str "Distance: "
#define time_str "Time:  "
#define kmh_str "km/h"

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
        Lcd.setFirstLine(pause_str);
    } else {
        Lcd.setFirstLine(getCurrentSpeedLine());
    }
    if ( (millis() - lastSecondLineChange) > ((unsigned long) changeSecondLine * 1000UL)) {
        currentSecondLine = (currentSecondLine + 1) % numberOfSecondLine;
        lastSecondLineChange = millis();
        avgSpeed = getAverageSpeed();
    }
    switch (currentSecondLine) {
        case 0:
            Lcd.setSecondLine(getDistanceLine());
            break;
        case 1:
            Lcd.setSecondLine(getAvgSpeedLine());
            break;
        case 2:
            Lcd.setSecondLine(getTimeLine());
            break;
    }
}
