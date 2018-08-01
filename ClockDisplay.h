#ifndef CLOCKDISPLAY_H
#define CLOCKDISPLAY_H

#include <Adafruit_NeoPixel.h>

class ClockDisplay {
    public:
        ClockDisplay(Adafruit_NeoPixel &pixels);

        void drawNumber(int r, int g, int b, int number);
        void turnOffDigit(int offset, bool update);
        void drawDigit(int offset, int r, int g, int b, int n);
        void drawDigit(int offset, int r, int g, int b, int n, bool update);
        void drawDots(int r, int g, int b);
        void drawDots(int r, int g, int b, bool update);
        void clear();
        void begin();
        void test();
    private:
        int nthDigit(int place, int number);
        Adafruit_NeoPixel _pixels;
};

#endif