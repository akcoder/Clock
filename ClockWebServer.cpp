#include "ClockWebServer.h"

ClockWebServer::ClockWebServer(const char *username, const char *password,
                               uint16_t port, Parameters *params,
                               RtcDS3231<TwoWire> *rtc) {
  _server = new AsyncWebServer(port);
  _params = params;
  _rtc = rtc;
  _username = username;
  _password = password;
  // strcpy(reinterpret_cast<char*>
}

void ClockWebServer::start() {
  Serial.println(F("ClockWebServer::start"));
  _server->on("/", HTTP_GET, std::bind(&ClockWebServer::index, this, std::placeholders::_1));
  _server->on("/info", HTTP_GET, std::bind(&ClockWebServer::info, this, std::placeholders::_1));
  _server->on("/params", HTTP_GET, std::bind(&ClockWebServer::showParams, this, std::placeholders::_1));
  _server->on("/params", HTTP_POST, std::bind(&ClockWebServer::storeParams, this, std::placeholders::_1));
  _server->on("/reboot", HTTP_GET, std::bind(&ClockWebServer::reboot, this, std::placeholders::_1));
  _server->on("/style.css", HTTP_GET, std::bind(&ClockWebServer::stylesheet, this, std::placeholders::_1));
  _server->on("/scan.json", HTTP_GET, std::bind(&ClockWebServer::scan, this, std::placeholders::_1));
  _server->onNotFound(std::bind(&ClockWebServer::notFound, this, std::placeholders::_1));

  _server->begin();
}

void ClockWebServer::info(AsyncWebServerRequest * request) {
  Serial.println(F("ClockWebServer::info"));
  char body[512];

  String coreVersion = ESP.getCoreVersion();
  coreVersion.replace('_', '.');
  snprintf_P(body, sizeof(body),
             PSTR("<p>Compiled: %s %s"
                  "<p>Heap: %d</p>"
                  "<p>Core version: v%s</p>"
                  "<p>SDK version: %s</p>"
                  "<p>CPU freq: %d MHz</p>"
                  "<p>Reset reason: %s</p>"
                  "<p>Sketch size: %d</p>"
                  "<p>Free sketch size: %d</p>"
                  "<a href=\"/\">Home</a>"),
             __DATE__, __TIME__, ESP.getFreeHeap(), coreVersion.c_str(),
             ESP.getSdkVersion(), ESP.getCpuFreqMHz(),
             ESP.getResetReason().c_str(), ESP.getSketchSize(),
             ESP.getFreeSketchSpace());

  String page = FPSTR(HTML_HEAD);
  page.replace("{v}", "Info");
  page += FPSTR(HTML_STYLE);
  page += FPSTR(HTML_HEAD_END);
  page += String(F("<h1>System Info</h1>"));
  page += body;
  page += FPSTR(HTML_END);

  request->send(200, "text/html", page);
}

void ClockWebServer::notFound(AsyncWebServerRequest * request) {
  Serial.printf_P(PSTR("ClockWebServer::notFound %s\n"), request->url().c_str());
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

void ClockWebServer::reboot(AsyncWebServerRequest *request) {
  if (!request->authenticate(_username, _password)) {
    return request->requestAuthentication();
  }

  request->redirect("/");

  _ticker.once(1, []() {
    Serial.println(F("Rebooting"));
    ESP.restart();
  });
}

void ClockWebServer::showParams(AsyncWebServerRequest *request) {
  Serial.println(F("ClockWebServer::showParams"));

  if (!request->authenticate(_username, _password)) {
    return request->requestAuthentication();
  }

  char body[500];

  snprintf_P(body, sizeof(body),
             PSTR("<p><input type=\"color\" name=\"color\" "
                  "value=\"#%02X%02X%02X\" /></p>"
                  "<p><input type=\"text\" name=\"ssid\" value=\"%s\" "
                  "placeholder=\"ssid\" /></p>"
                  "<p><input type=\"text\" name=\"passphrase\" value=\"%s\" "
                  "placeholder=\"passphrase\" /></p>"),
             _params->red(), _params->green(), _params->blue(),
             _params->ssid().c_str(), _params->passphrase().c_str());

  String page = FPSTR(HTML_HEAD);
  page.replace("{v}", "Parameters");
  page += FPSTR(HTML_STYLE);
  page += FPSTR(HTML_HEAD_END);
  page += String(F("<h1>Parameters</h1>"));
  page += String(F("<form method=\"POST\" action=\"params\">"));
  page += body;
  page += String(F("<input type=\"submit\" value=\"Update\">"
    "<input type=\"submit\" name=\"store\" value=\"Store &amp; Update\">"
    "</form><p><a href=\"/\">Home</a></p>"));
  page += FPSTR(HTML_END);

  request->send(200, "text/html", page);
}

void ClockWebServer::storeParams(AsyncWebServerRequest *request) {
  Serial.println(F("ClockWebServer::storeParams"));
  if (!request->authenticate(_username, _password)) {
    return request->requestAuthentication();
  }

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
  Serial.println(F("Redirected"));
}

void ClockWebServer::index(AsyncWebServerRequest * request) {
  Serial.println(F("ClockWebServer::index"));
  char temp[400];
  int up_sec = millis() / 1000;
  int up_min = up_sec / 60;
  int up_hr = up_min / 60;

  RtcDateTime now = _rtc->GetDateTime();

  int hours = now.Hour();

  if (hours > 12) {
    hours -= 12;
  }
  if (hours == 0) {
    hours = 12;
  }

  char body[200];

  snprintf_P(body, sizeof(body),
           PSTR("<p>Current Date: %04d-%02d-%02d</p>"
           "<p>Current Time: %d:%02d:%02d %s</p>"
           "<p>Uptime: %d:%02d:%02d</p>"),
           now.Year(), now.Month(), now.Day(),
           hours, now.Minute(), now.Second(), now.Hour() < 12 ? "am" : "pm",
           up_hr, up_min % 60, up_sec % 60);

  String page = FPSTR(HTML_HEAD);
  page.replace("{v}", "Clock");
  page += FPSTR(HTML_STYLE);
  page += FPSTR(HTML_HEAD_END);
  page += String(F("<h1>NTP Clock!</h1>"));
  page += String(body);
  page += String(F("<p><a href=\"params\">Change Parameters</a></p>"
    "<p><a href=\"reboot\">Reboot</a></p>"));
  page += String(F("<p><a href=\"info\">System Info</a></p>"));
  page += FPSTR(HTML_END);

  request->send(200, "text/html", page);
}

void ClockWebServer::stylesheet(AsyncWebServerRequest *request) {
  Serial.println(F("ClockWebServer::stylesheet"));

  auto response = request->beginResponse(200, "text/css", FPSTR(HTML_STYLESHEET));
  response->addHeader("Cache-Control", "max-age=86400");
  request->send(response);
}

/** scan.json */
void ClockWebServer::scan(AsyncWebServerRequest *request) {
   if (!request->authenticate(_username, _password)) {
    return request->requestAuthentication();
  }

 int n = WiFi.scanNetworks(false, true);
  if (n == 0) {
    Serial.println(F("No networks found"));
    request->send(204);
    return;
  }

  //sort networks
  uint16_t indices[n];

  getSortedAPList(indices);
  
  StaticJsonDocument<1024> doc;
  JsonArray root = doc.to<JsonArray>();

  //display networks in page
  for (int i = 0; i < n; i++) {
    if (indices[i] == -1) {
      continue; // skip dups
    }
    Serial.printf_P(PSTR("ssid: %s, rssi: %d\n"), WiFi.SSID(indices[i]).c_str(), WiFi.RSSI(indices[i]));

    int quality = getRSSIasQuality(WiFi.RSSI(indices[i]));

    if (_minimumQuality == -1 || _minimumQuality < quality) {
        JsonObject obj;
        obj["ssid"] = WiFi.SSID(indices[i]).c_str();
        obj["rssi"] = quality;
        obj["enc"] = WiFi.encryptionType(indices[i]) != ENC_TYPE_NONE;

        root.add(obj);
    }
  }

  String output;
  serializeJson(doc, output);

  request->send(200, F("application/json"), output);
}

int ClockWebServer::getRSSIasQuality(int rssi) {
  int quality = 0;

  if (rssi <= -100) {
    quality = 0;
  } else if (rssi >= -50) {
    quality = 100;
  } else {
    quality = 2 * (rssi + 100);
  }

  return quality;
}

void ClockWebServer::getSortedAPList(uint16_t indices[]) {
  int16_t n = sizeof(indices);

  for (int i = 0; i < n; i++) {
    indices[i] = i;
  }

  // RSSI SORT
  // old sort
  for (int i = 0; i < n; i++) {
    for (int j = i + 1; j < n; j++) {
      if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
        std::swap(indices[i], indices[j]);
      }
    }
  }

  //Remove duplicate ssids
  String cssid;
  for (int i = 0; i < n; i++) {
    if (indices[i] == -1) {
      continue;
    }

    cssid = WiFi.SSID(indices[i]);

    for (int j = i + 1; j < n; j++) {
      if (cssid == WiFi.SSID(indices[j])) {
        Serial.printf_P(PSTR("DUP AP: %s\n"), WiFi.SSID(indices[j]).c_str());
        indices[j] = -1; // set dup aps to index -1
      }
    }
  }
}
