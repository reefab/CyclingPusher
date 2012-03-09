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
  int hours = effectiveTime / 3600000;
  int minutes = effectiveTime / 60000;
  long seconds = (effectiveTime % 60000);
  seconds = (long) seconds / 1000;
  String time = "Time: "
    + String(hours)
    + prettyDigits(minutes) 
      + prettyDigits(seconds);
  lcd.print(time);
}

void displayPauseScreen() {
  lcd.clear();
  lcd.print("Activity paused.");
  lcd.setCursor(0, 1);
  lcd.print("Upload result?");
  delay(1000);
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
  lcd.setCursor(0, 1);
  for (int i = 16;i < startTimeStr.length();i++) lcd.print(startTimeStr.charAt(i));
  delay(2500);
}


