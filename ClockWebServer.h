#ifndef CLOCK_WEBSERVER_H
#define CLOCK_WEBSERVER_H

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Ticker.h>

#include "Parameters.h"

class ClockWebServer {
    public:
        ClockWebServer(uint16_t port, Parameters *params);

        void start();
    private:
        Parameters *_params;
        AsyncWebServer *_server;
        Ticker _ticker;
        uint8_t _minimumQuality = 8;

        void index(AsyncWebServerRequest * request);
        void info(AsyncWebServerRequest * request);
        void scan(AsyncWebServerRequest *request);
        void showParams(AsyncWebServerRequest * request);
        void storeParams(AsyncWebServerRequest * request);
        void reboot(AsyncWebServerRequest * request);
        void notFound(AsyncWebServerRequest * request);
        void stylesheet(AsyncWebServerRequest * request);

        int getRSSIasQuality(int rssi);
        void getSortedAPList(uint16_t indices[]);};

const char HTML_HEAD[] PROGMEM       = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\"content=\"width=device-width,initial-scale=1,user-scalable=no\"/><title>{v}</title>";
const char HTML_STYLE[] PROGMEM      = "<link rel=\"stylesheet\" href=\"style.css\" type=\"text/css\">";
const char HTML_HEAD_END[] PROGMEM   = "</head><body><div style=\"text-align:left;display:inline-block;min-width:260px;\">";
const char HTML_END[] PROGMEM        = "</div></body></html>";
const char HTML_STYLESHEET[] PROGMEM = "body {background-color: white; font-family: Arial, Helvetica, Sans-Serif; color: black;} "
              "a {color: red; text-decoration: none;} "
              "a:hover {text-decoration: underline}";
#endif //CLOCK_WEBSERVER_H
