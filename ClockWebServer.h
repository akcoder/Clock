#ifndef CLOCK_WEBSERVER_H
#define CLOCK_WEBSERVER_H

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "Parameters.h"

class ClockWebServer {
    public:
        ClockWebServer(uint16_t port, Parameters *params);

        void start();
    private:
        Parameters *_params;
        AsyncWebServer *_server;
        void info(AsyncWebServerRequest * request);
        void showParams(AsyncWebServerRequest * request);
        void updateParams(AsyncWebServerRequest * request);
        void index(AsyncWebServerRequest * request);
        void notFound(AsyncWebServerRequest * request);
        void stylesheet(AsyncWebServerRequest * request);
};

#endif //CLOCK_WEBSERVER_H
