#ifndef lcdwrapper_h
#define lcdwrapper_h

#include "Arduino.h"
#include "LiquidCrystal.h"

class LcdWrapper
{
    public:
        LcdWrapper(int LCDRS, int LCDE, int LCDD4, int LCDD5, int LCDD6, int LCDD7, int blPin);
        void begin();
        void setFirstLine(String str);
        void infoMessage(String str);
        void errorMessage(String str);
        void setSecondLine(String str);
        void switchBacklight(boolean bl);
        void clear();

    private:
        LiquidCrystal* lcdlib;
        int _LCDRS;
        int _LCDE;
        int _LCDD4;
        int _LCDD5;
        int _LCDD6;
        int _LCDD7;
        int _blPin;
        unsigned long displayUntil;
        String firstLine;
        String secondLine;
        String _message;
        String restrictIn16Chars(String str);
        void message(String str, int delay, bool error);
        volatile boolean backlight;
};

#endif
