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

        String ssid();
        void ssid(String);

        String passphrase();
        void passphrase(String);
    private:
        uint8_t _red;
        uint8_t _green;
        uint8_t _blue;
        String _ssid;
        String _passphrase;
};

struct __attribute__ ((packed)) Config {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    char ssid[32];
    char passphrase[64];
};

#endif //PARAMETERS_H
