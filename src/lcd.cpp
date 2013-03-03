#include "Arduino.h"
#include "lcd.h"

#include "LiquidCrystal.h"

LiquidCrystal lcdlib(4, 5, 6, 7, 8, 9);

Lcd::Lcd(int blPin) {
  // LCD Init
  lcdlib.begin(16, 2);
  lcdlib.clear();

  _blPin = blPin;
  pinMode(_blPin, OUTPUT);
  switchBacklight(true);
}

void Lcd::clear() {
    lcdlib.clear();
}

void Lcd::setFirstLine(String str) {
    firstLine = restrictIn16Chars(str);
}

void Lcd::setSecondLine(String str) {
    secondLine = restrictIn16Chars(str);
}

void Lcd::switchBacklight(boolean bl) {
    if (bl) {
        digitalWrite(_blPin, HIGH);
        backlight = true;
    } else {
        digitalWrite(_blPin, LOW);
        backlight = false;
    }
}

String Lcd::restrictIn16Chars(String line)
{
    if (line.length() > 16) {
        return line.substring(0, 17);
    } else {
        return line;
    }
}
