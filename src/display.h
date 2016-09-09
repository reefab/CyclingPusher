// vim: filetype=arduino
#include <stdlib.h>

extern float currentSpeed;
extern unsigned int totalDistance;
extern unsigned long effectiveTime;
extern volatile boolean paused;
extern Adafruit_ST7735 tft;

String floatToString(float float_val)
{
    char number[6];
    if ((int) float_val > 99) {
        return dtostrf(float_val, 5, 1, number);
    } else {
        return dtostrf(float_val, 5, 2, number);
    }
}

void displayInfo() {
    if (paused) {
        tft.setCursor(10, 56);
        tft.setTextColor(ST7735_BLACK);
        tft.println("PAUSED");
    } else {
        tft.setCursor(0, 0);
        tft.setTextColor(ST7735_RED);
        tft.print(floatToString(currentSpeed));
        tft.setTextColor(ST7735_BLACK);
        tft.print("km/h");
    }
    /* if ( (millis() - lastSecondLineChange) > ((unsigned long) changeSecondLine * 1000UL)) { */
    /*     currentSecondLine = (currentSecondLine + 1) % numberOfSecondLine; */
    /*     lastSecondLineChange = millis(); */
    /*     avgSpeed = getAverageSpeed(); */
    /* } */
    /* switch (currentSecondLine) { */
    /*     case 0: */
    /*         Lcd.setSecondLine(getDistanceLine()); */
    /*         break; */
    /*     case 1: */
    /*         Lcd.setSecondLine(getAvgSpeedLine()); */
    /*         break; */
    /*     case 2: */
    /*         Lcd.setSecondLine(getTimeLine()); */
    /*         break; */
    /* } */
}
