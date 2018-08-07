#ifndef SOFTAP_H
#define SOFTAP_H

#include "Parameters.h"
#include "ClockDisplay.h"
#include "digits.h"

#include <ArduinoJson.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

const byte DNS_PORT = 53;

class SoftAP {
    public:
        SoftAP(const char * ssid, const char * passphrase, Parameters *params, ClockDisplay * display);
        void init();
        void run();

    private:
        void startAp();

        ClockDisplay * _display;
        Parameters * _params;
        DNSServer * _dnsServer;
        uint8_t _minimumQuality = 8;

        const char * _ssid;
        const char * _passphrase;
};

#endif //SOFTAP_H