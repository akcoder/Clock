#ifndef CLOCK_WEBSERVER_H
#define CLOCK_WEBSERVER_H

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <RtcDS3231.h>
#include <Ticker.h>
#include <Wire.h>

#include "Parameters.h"

class ClockWebServer {
    public:
      ClockWebServer(const char *username, const char *password, uint16_t port,
                     Parameters *params, RtcDS3231<TwoWire> *rtc);

      void start();
    protected:
        Parameters *_params;
        AsyncWebServer *_server;
        RtcDS3231<TwoWire> *_rtc;

        Ticker _ticker;
        uint8_t _minimumQuality = 8;
        const char * _username;
        const char * _password;

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
const char HTML_HEAD_END[] PROGMEM   = "</head><body>";
const char HTML_END[] PROGMEM        = "</body></html>";
const char HTML_STYLESHEET[] PROGMEM = "body {background-color: white; font-family: Arial, Helvetica, Sans-Serif; color: black;} "
              "h1 {margin: 0.25em 0 } "
              "a {color: red; text-decoration: none} "
              "a:hover {text-decoration: underline} ";
#endif //CLOCK_WEBSERVER_H
