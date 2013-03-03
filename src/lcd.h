#ifndef lcd_h
#define lcd_h

#include "Arduino.h"

class Lcd
{
    public:
        Lcd(int blPin);
        void setFirstLine(String str);
        void setSecondLine(String str);
        // void setFirstLine(__FlashStringHelper* str);
        // void setSecondLine(__FlashStringHelper* str);
        void switchBacklight(boolean bl);
        void clear();

    private:
        int _blPin;
        String firstLine;
        String secondLine;
        String restrictIn16Chars(String str);
        volatile boolean backlight;
};

#endif
