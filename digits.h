#ifndef DIGITS_H
#define DIGITS_H

#include <cstdint>

#define SEGMENT_SIZE 3
#define DIGIT_SIZE (7 * SEGMENT_SIZE)

#define HOUR1 0
#define HOUR2 DIGIT_SIZE
#define DOT1 (HOUR2 + DIGIT_SIZE)
#define DOT2 (DOT1 + 1)
#define MINUTE1 (DOT2 + 1)
#define MINUTE2 MINUTE1 + DIGIT_SIZE
#define NUMPIXELS MINUTE2 + DIGIT_SIZE

/*
Display bit mask
 3
4 2
 1
5 7
 6
*/

// Bit mapping
const uint8_t ZERO  = 0b01111110;
const uint8_t ONE   = 0b01000010;
const uint8_t TWO   = 0b00110111;
const uint8_t THREE = 0b01100111;
const uint8_t FOUR  = 0b01001011;
const uint8_t FIVE  = 0b01101101;
const uint8_t SIX   = 0b01111101;
const uint8_t SEVEN = 0b01000110;
const uint8_t EIGHT = 0b01111111;
const uint8_t NINE  = 0b01101111;

const uint8_t digits[] = { ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE };
#endif //DIGITS_H