#include<stdlib.h>

#define numberOfSecondLine 3

String firstLine;
String secondLine;
unsigned long lastSecondLineChange = 0;
int currentSecondLine = 0;

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
  String line = "Speed: ";
  line += floatToString(currentSpeed);
  line += "km/h";
 return line; 
}

String getDistanceLine()
{
  String line = "Distance: ";
  if (totalDistance < 1000) {
    line += totalDistance;
    line += "m       ";
  } else {
    line += floatToString((float) totalDistance / 1000);
    line += "km      ";
  }
  return line;
}

float getAverageSpeed() {
  if (effectiveTime == 0) {
    return 0;
  } else {
    return ((float) totalDistance / (float) effectiveTime) * 3600;
  }
}  

String getTimeLine() {
  int hours = effectiveTime / 3600000;
  int minutes = effectiveTime / 60000;
  if (minutes > 59) minutes = minutes % 60;
  long seconds = (effectiveTime % 60000);
  seconds = (long) seconds / 1000;
  String time = "Time: "
    + String(hours)
    + prettyDigits(minutes) 
      + prettyDigits(seconds);
  time += "   ";
  return time;
}

String getAvgSpeedLine() {
  String line ="Avg:   ";
  line += floatToString(getAverageSpeed());
  line += "km/h";
  return line;
}

void displayInfo() {
  if (paused) {
    firstLine = "--===Paused===--";
  } else {
    firstLine = getCurrentSpeedLine();
  }
  lcd.setCursor(0, 0);
  lcd.print(firstLine);
  if ( (millis() - lastSecondLineChange) > ((unsigned long) changeSecondLine * 1000)) {
    currentSecondLine = (currentSecondLine + 1) % numberOfSecondLine;
    lastSecondLineChange = millis();
  }
  lcd.setCursor(0, 1);
  switch (currentSecondLine) {
    case 0:
      secondLine = getTimeLine();
      break;
    case 1:
      secondLine = getDistanceLine();
      break;
     case 2:
       secondLine = getAvgSpeedLine();
       break;
  }
  lcd.print(secondLine);
}

void displayInitScreen() {
  lcd.clear();
  lcd.print("IP: ");
  lcd.print(Ethernet.localIP());
  lcd.setCursor(0, 1);
  for (int i = 0;i < 16;i++) lcd.print(startTimeStr.charAt(i));
  delay(2500); 
  lcd.setCursor(0, 1);
  for (int i = 1;i < 32;i++) lcd.print(" ");
  lcd.setCursor(4, 1);
  for (int i = 16;i < startTimeStr.length();i++) lcd.print(startTimeStr.charAt(i));
  delay(2500);
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

