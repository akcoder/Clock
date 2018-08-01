#include "ClockDisplay.h"
#include "digits.h"

#include <Arduino.h>
#include <cstdint>

ClockDisplay::ClockDisplay(Adafruit_NeoPixel &pixels) {
  _pixels = pixels;
}

void ClockDisplay::drawDigit(int offset, int r, int g, int b, int n) {
  drawDigit(offset, r, g, b, n, false);
}

void ClockDisplay::drawDigit(int offset, int r, int g, int b, int n, bool update) {
  int8 digit = digits[n];

  uint32_t on = _pixels.Color(r, g, b);
  uint32_t off = _pixels.Color(0, 0, 0);

  for (int i = 0; i < 8; ++i) { // 7 bits
    for (int p = 0; p < SEGMENT_SIZE; ++p) {
      bool isSet = (1 << i) & digit;
      _pixels.setPixelColor((i * SEGMENT_SIZE) + p + offset, isSet ? on : off);
    }
  }

  if (update) {
    _pixels.show();
  }
}

void ClockDisplay::turnOffDigit(int offset, bool update) {
  uint32_t off = _pixels.Color(0, 0, 0);

  for (int i = 0; i < 8; ++i) { // 7 bits
    for (int p = 0; p < SEGMENT_SIZE; ++p) {
      _pixels.setPixelColor((i * SEGMENT_SIZE) + p + offset, off);
    }
  }

  if (update) {
    _pixels.show();
  }
}

int ClockDisplay::nthDigit(int place, int number) {
  while (place--) {
    number /= 10;
  }

  return number % 10;
}

void ClockDisplay::drawNumber(int r, int g, int b, int number) {
  // const uint8_t places[] = { HOUR1, HOUR2, MINUTE1, MINUTE2 };

  // int place = 0;
  // int factor = 1;

  // while (factor > 1) {
  //   factor = factor / 10;


  //   printf(“% d “, num / factor);

  //   num = num % factor;
  // }

  int thousands = nthDigit(3, number);
  int hundreds = nthDigit(2, number);
  int tens = nthDigit(1, number);
  int ones = nthDigit(0, number);

  thousands > 0 ?
    drawDigit(HOUR1, r, g, b, thousands) :
    turnOffDigit(HOUR1, false);

  drawDigit(HOUR2, r, g, b, hundreds);
  drawDigit(MINUTE1, r, g, b, tens);
  drawDigit(MINUTE2, r, g, b, ones, true);
}

void ClockDisplay::begin() {
    _pixels.begin();
}

void ClockDisplay::clear() {
  for(int8_t i = 0; i < NUMPIXELS; ++i) {
    _pixels.setPixelColor(i, 0);
  }
  _pixels.show();
}

void ClockDisplay::drawDots(int r, int g, int b) {
  drawDots(r, g, b, false);
}

void ClockDisplay::drawDots(int r, int g, int b, bool update) {
  _pixels.setPixelColor(DOT1, _pixels.Color(r, g, b));
  _pixels.setPixelColor(DOT2, _pixels.Color(r, g, b));

  if (update) {
    _pixels.show();
  }
}

void ClockDisplay::test() {
  Serial.println(F("Running self test"));
  for (int i = 0; i < 10; ++i) {
    drawDigit(HOUR1, 255, 0, 0, i);
    drawDigit(HOUR2, 255, 0, 0, i);

    uint32_t dotColor = i % 2 ? _pixels.Color(0, 0, 0) : _pixels.Color(255, 0, 0);
    _pixels.setPixelColor(DOT1, dotColor);
    _pixels.setPixelColor(DOT2, dotColor);

    drawDigit(MINUTE1, 255, 0, 0, i);
    drawDigit(MINUTE2, 255, 0, 0, i, true);
    delay(400);
  }

  clear();
}