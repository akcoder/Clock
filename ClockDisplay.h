#ifndef CLOCKDISPLAY_H
#define CLOCKDISPLAY_H

#include <Adafruit_NeoPixel.h>

class ClockDisplay {
    public:
        ClockDisplay(Adafruit_NeoPixel &pixels);

        void drawNumber(int r, int g, int b, int number);
        void turnOffDigit(int offset, bool update = false);
        void setBits(int offset, int r, int g, int b, uint8_t mask);
        void drawDigit(int offset, int r, int g, int b, int n, bool update = false);
        void drawDots(int r, int g, int b, bool update = false);
        void clear();
        void begin();
        void test();
    private:
        int nthDigit(int place, int number);
        Adafruit_NeoPixel _pixels;
};

#endif