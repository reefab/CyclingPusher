#include "Arduino.h"
#include "lcdwrapper.h"

LcdWrapper::LcdWrapper(int LCDRS, int LCDE, int LCDD4, int LCDD5, int LCDD6, int LCDD7, int blPin) {
  _blPin = blPin;
  _LCDRS = LCDRS;
  _LCDE = LCDE;
  _LCDD4 = LCDD4;
  _LCDD5 = LCDD5;
  _LCDD6 = LCDD6;
  _LCDD7 = LCDD7;
  pinMode(_blPin, OUTPUT);
  switchBacklight(true);
}

void LcdWrapper::begin() {
  lcdlib = new LiquidCrystal(_LCDRS, _LCDE, _LCDD4, _LCDD5, _LCDD6, _LCDD7);
  lcdlib->begin(16, 2);
  lcdlib->clear();
}

void LcdWrapper::clear() {
    lcdlib->clear();
}

void LcdWrapper::setFirstLine(String str) {
    firstLine = restrictIn16Chars(str);
    lcdlib->clear();
    lcdlib->print(firstLine);
}

void LcdWrapper::setSecondLine(String str) {
    secondLine = restrictIn16Chars(str);
    lcdlib->clear();
    lcdlib->print(firstLine);
    lcdlib->setCursor(0, 1);
    lcdlib->print(secondLine);
}

void LcdWrapper::switchBacklight(boolean bl) {
    if (bl) {
        digitalWrite(_blPin, HIGH);
        backlight = true;
    } else {
        digitalWrite(_blPin, LOW);
        backlight = false;
    }
}

String LcdWrapper::restrictIn16Chars(String line)
{
    if (line.length() > 16) {
        return line.substring(0, 17);
    } else {
        return line;
    }
}
