#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <stdint.h>
#include <Arduino.h>
#include <EEPROM.h>

class Parameters {
    public:
        void load();
        void write();

        uint8_t red();
        void red(uint8_t);

        uint8_t green();
        void green(uint8_t);

        uint8_t blue();
        void blue(uint8_t);
    
    private:
        uint8_t _red = -2;
        uint8_t _green = -2;
        uint8_t _blue = -2;
};

#endif //PARAMETERS_H
