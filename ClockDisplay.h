#ifndef CLOCKDISPLAY_H
#define CLOCKDISPLAY_H

#include <Adafruit_NeoPixel.h>

class ClockDisplay {
    public:
        ClockDisplay(Adafruit_NeoPixel &pixels);

        void drawNumber(uint16_t r, uint16_t g, uint16_t b, int number);
        void turnOffDigit(uint16_t offset, bool update = false);
        void setPixel(uint16_t offset, uint16_t r, uint16_t g, uint16_t b);
        void setBits(uint16_t offset, uint16_t r, uint16_t g, uint16_t b, uint8_t mask, bool update = false);
        void drawDigit(uint16_t offset, uint16_t r, uint16_t g, uint16_t b, int n, bool update = false);
        void drawDots(uint16_t r, uint16_t g, uint16_t b, bool update = false);
        void clear();
        void begin();
        void test();
    private:
        int nthDigit(int place, int number);
        Adafruit_NeoPixel _pixels;
};

#endif