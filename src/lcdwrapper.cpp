// vim: filetype=arduino
#include "Arduino.h"
#include "lcdwrapper.h"

LcdWrapper::LcdWrapper(int LCDRS, int LCDE, int LCDD4, int LCDD5, int LCDD6, int LCDD7, int blPin) {
  displayUntil = 0;
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
    if (displayUntil < millis()) {
        firstLine = restrictIn16Chars(str);
        lcdlib->clear();
        lcdlib->print(firstLine);
    }
}

void LcdWrapper::message(String str, int delay, bool error) {
    switchBacklight(true);
    _message = restrictIn16Chars(str);
    displayUntil = millis() + (unsigned long) delay;
    lcdlib->clear();
    lcdlib->print(_message);
    if (error) {
        lcdlib->setCursor(0, 1);
        lcdlib->print("****************");
    }
}

void LcdWrapper::infoMessage(String str) {
    message(str, 1000, false);
}

void LcdWrapper::errorMessage(String str) {
    message(str, 1000, true);
}

void LcdWrapper::setSecondLine(String str) {
    if (displayUntil < millis()) {
        lcdlib->clear();
        lcdlib->print(firstLine);
    }
    lcdlib->setCursor(0, 1);
    secondLine = restrictIn16Chars(str);
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
