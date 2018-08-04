#include "ClockWebServer.h";

#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval

ClockWebServer::ClockWebServer(uint16_t port, Parameters *params) {
  _server = new AsyncWebServer(port);
  _params = params;
}

void ClockWebServer::start() {
  Serial.println("ClockWebServer::start");
  _server->on("/", HTTP_GET, std::bind(&ClockWebServer::index, this, std::placeholders::_1));
  _server->on("/info", HTTP_GET, std::bind(&ClockWebServer::info, this, std::placeholders::_1));
  _server->on("/params", HTTP_GET, std::bind(&ClockWebServer::showParams, this, std::placeholders::_1));
  _server->on("/params", HTTP_POST, std::bind(&ClockWebServer::storeParams, this, std::placeholders::_1));
  _server->on("/reboot", HTTP_GET, std::bind(&ClockWebServer::reboot, this, std::placeholders::_1));
  _server->on("/style.css", HTTP_GET, std::bind(&ClockWebServer::stylesheet, this, std::placeholders::_1));
  _server->onNotFound(std::bind(&ClockWebServer::notFound, this, std::placeholders::_1));

  _server->begin();
}

void ClockWebServer::info(AsyncWebServerRequest * request) {
  Serial.println("ClockWebServer::info");
  char result[1024];

  snprintf(result, sizeof(result),
  "<html><head>\
  <link rel=\"stylesheet\" href=\"style.css\" type=\"text/css\"> \
  </head><body>\
  <h1>System Info</h1>\
  <p>Heap: %d</p>\
  <p>Core version: %s</p>\
  <p>SDK version: %s</p>\
  <p>CPU freq: %d MHz</p>\
  <p>Reset reason: %s</p>\
  <p>Sketch size: %d</p>\
  <p>Free sketch size: %d</p>\
  <a href=\"/\">Home</a>\
  </body></html>", ESP.getFreeHeap(), ESP.getCoreVersion().c_str(), ESP.getSdkVersion(),
    ESP.getCpuFreqMHz(), ESP.getResetReason().c_str(), ESP.getSketchSize(), ESP.getFreeSketchSpace());

  request->send(200, "text/html", result);
}

void ClockWebServer::notFound(AsyncWebServerRequest * request) {
  Serial.printf("ClockWebServer::notFound %s\n", request->url().c_str());
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += request->url();
  message += "\nMethod: ";
  message += request->methodToString();
  message += "\nArguments: ";
  message += request->args();
  message += "\n";

  for (uint8_t i = 0; i < request->args(); i++) {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }

  request->send(404, "text/plain", message);
}

void ClockWebServer::reboot(AsyncWebServerRequest * request) {
  request->redirect("/");

  _ticker.once(1, []() {
    Serial.println("Rebooting");
    ESP.restart();
  });
}

void ClockWebServer::showParams(AsyncWebServerRequest *request) {
  Serial.println("ClockWebServer::showParams");
  char result[800];
  
  snprintf(result, sizeof(result),
  "<html><head>\
  <link rel=\"stylesheet\" href=\"style.css\" type=\"text/css\"> \
  </head><body>\
  <h1>Clock colors</h1>\
  <form method=\"POST\" action=\"params\">\
  <p><input type=\"color\" name=\"color\" value=\"#%02X%02X%02X\" /></p>\
  <p><input type=\"text\" name=\"ssid\" value=\"%s\" placeholder=\"ssid\" /></p>\
  <p><input type=\"text\" name=\"passphrase\" value=\"%s\" placeholder=\"passphrase\" /></p>\
  <input type=\"submit\" value=\"Update\">\ <input type=\"submit\" name=\"store\" value=\"Store &amp; Update\">\
  </form>\
  <a href=\"/\">Home</a>\
  </body></html>", _params->red(), _params->green(), _params->blue(), _params->ssid().c_str(), _params->passphrase().c_str());

  request->send(200, "text/html", result);
}

void ClockWebServer::storeParams(AsyncWebServerRequest *request) {
  Serial.println("ClockWebServer::storeParams");

  String ssid = request->arg("ssid");
  String passphrase = request->arg("passphrase");
  String color = request->arg("color");
  color.setCharAt(0, ' '); //Erase the leading "#"

  uint8_t red, green, blue;

  /* Convert the provided value to a decimal long */
  long result = strtol(color.c_str(), nullptr, HEX);

  blue = result;
  green = result >> 8;
  red = result >> 16;

  _params->blue(result);
  _params->green(result >> 8);
  _params->red(result >> 16);

  int ssidIsDifferent = _params->ssid() != ssid;
  int pwIsDifferent = _params->passphrase() != passphrase;

  if (ssidIsDifferent != 0 || pwIsDifferent != 0) {
    _params->ssid(ssid);
    _params->passphrase(passphrase);
    _params->write();
    delay(1000);
  } else if (request->hasArg("store")) {
    _params->write();
  }

  request->redirect("/params");
  Serial.println("Redirected");
}

void ClockWebServer::index(AsyncWebServerRequest * request) {
  Serial.println("ClockWebServer::index");
  char temp[400];
  int up_sec = millis() / 1000;
  int up_min = up_sec / 60;
  int up_hr = up_min / 60;

  time_t now = time(nullptr);

  tm *tm = localtime(&now);
  int hours = tm->tm_hour;
  int mins = -1;

  if (!tm->tm_isdst) {
    --hours;
  }
  if (hours > 12) {
    hours -= 12;
  }
  if (hours == 0) {
    hours = 12;
  }
  snprintf(temp, sizeof(temp),
           "<html>\
  <head>\
    <title>Clock</title>\
    <link rel=\"stylesheet\" href=\"style.css\" type=\"text/css\"> \
  </head>\
  <body>\
    <h1>NTP Clock!</h1>\
    <p>Current Time: %d:%02d:%02d %s</p>\
    <p>Current Time: %d:%02d:%02d</p>\
    <p>Uptime: %d:%02d:%02d</p>\
    <p><a href=\"params\">Change Parameters</a></p>\
    <p><a href=\"reboot\">Reboot</a></p>\
    <p><a href=\"info\">System Info</a></p>\
  </body>\
</html>",
           hours, tm->tm_min, tm->tm_sec, tm->tm_hour < 12 ? "am" : "pm",
           tm->tm_hour, tm->tm_min, tm->tm_sec,
           up_hr, up_min % 60, up_sec % 60);
  request->send(200, "text/html", temp);
}

void ClockWebServer::stylesheet(AsyncWebServerRequest *request) {
  Serial.println("ClockWebServer::stylesheet");
  auto body = "body { background-color: white; font-family: Arial, Helvetica, Sans-Serif; color: black; } "
              "a { color: red; text-decoration: none; } "
              "a:hover {text-decoration: underline}";

  auto response = request->beginResponse(200, "text/css", body);
  response->addHeader("Cache-Control", "max-age=86400");
  request->send(response);
}
