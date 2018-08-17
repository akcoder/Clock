#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "digits.h"
#include "ClockDisplay.h"
#include "ClockWebServer.h"
#include "SoftAP.h"
#include "Parameters.h"
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval
#include <coredecls.h>                  // settimeofday_cb()
#include <Wire.h>
#include <RtcDS3231.h>

// for testing purpose:
extern "C" int clock_gettime(clockid_t unused, struct timespec *tp);

int hours = 0;
int mins = -1;

#define TZ              -8       // (utc+) TZ in hours
#define DST_MN          60       // use 60mn for summer time in some countries

#define TZ_MN           ((TZ)*60)
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)

#define LED_PIN 12      /* D6 */
#define HEARTBEAT_PIN 2 /* D4 */
#define SDA_PIN 4       /* D2 */
#define SCL_PIN 5       /* D1 */

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
ClockDisplay *display = new ClockDisplay(pixels);
Parameters *params;
ClockWebServer *webServer;

RtcDS3231<TwoWire> rtc(Wire);

timeval tv;
timespec tp;
RtcDateTime rtcNow;

timeval cbtime;			// time set in callback
bool cbtime_set = false;

void time_is_set(void) {
  gettimeofday(&cbtime, NULL);

  cbtime_set = true;
  Serial.println(F("-- settimeofday() was called --"));

  updateRtcFromNtp();
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  params = new Parameters();
  params->load();

  webServer = new ClockWebServer(httpAuthUsername, httpAuthPassword, 80, params, &rtc);

  Serial.println(F("Testing display"));
  display->begin();
  display->clear();

  display->test();

  // Indicate which step we are on...
  display->setBits(HOUR1, 255, 255, 0, 0b01111000);    // Left half of W
  display->setBits(HOUR2, 255, 255, 0, 0b01110010);    // Right half of W
  display->setBits(MINUTE1, 255, 255, 0, 0b00011101);  // F
  display->turnOffDigit(MINUTE2, true); 

  bool wifiStarted = setupWiFi();
  if (!wifiStarted) {
    SoftAP softAp = SoftAP(softApSsid, softApPassword, params, display);
    softAp.init();
    startWebServerAndShowIp();
    softAp.run(); //This never exits by design....
  }

  startWebServerAndShowIp();

  rtc.Begin();

  display->drawDigit(MINUTE2, 255, 255, 0, 2, true);
  configTime(TZ_SEC, DST_SEC, "pool.ntp.org");

  display->drawDigit(MINUTE2, 255, 255, 0, 3, true);
  settimeofday_cb(time_is_set);

  //Waiting for RTC to update... Updated in time_is_set callback...
  display->setBits(HOUR1, 255, 255, 0, 0b01111000); // Left half of W
  display->setBits(HOUR2, 255, 255, 0, 0b01110010); // Right half of W
  display->setBits(MINUTE1, 255, 255, 0, 0b01100110); // Left half of T
  display->setBits(MINUTE2, 255, 255, 0, 0b00011100, true); // Right half of T

  //Don't force the display to wait on a time sync at startup if we have a valid time
  if (rtc.IsDateTimeValid()) {
    cbtime_set = true;
  }

  pinMode(HEARTBEAT_PIN, OUTPUT);
}

void loop() {
  bool even = false;

  static uint8_t prevSecond = -1;

  // localtime / gmtime every second change
  rtcNow = rtc.GetDateTime();

  //Update the clock from NTP at 2:01:00 am
  if (rtcNow.Hour() == 2 && rtcNow.Minute() == 1 && rtcNow.Second() == 0) {
    settimeofday_cb(time_is_set);
  }

  if (cbtime_set && prevSecond != rtcNow.Second()) {
    even = rtcNow.Second() % 2;
    prevSecond = rtcNow.Second();

    digitalWrite(HEARTBEAT_PIN, even);

    if (rtcNow.Second() == 30) {
      showTemp();
    } else {
      displayTime(rtcNow);
    }
  }

  delay(250);
}

bool setupWiFi() {
  if (params->ssid().length() == 0) {
    return false;
  }

  Serial.printf_P(PSTR("Connecting to '%s', '%s'"), params->ssid().c_str(), params->passphrase().c_str());
  WiFi.begin(params->ssid().c_str(), params->passphrase().c_str());

  int count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);

    display->setPixel(DOT1, 0, 0, count % 2? 255 : 0);
    Serial.print(".");

    ++count;
    if (count > 30) {
      Serial.println(F("failed!"));
      WiFi.disconnect();
      return false;
    }
  }

  Serial.println(F("\nWiFi connected"));
  Serial.printf_P(PSTR("IP address: %s, Channel: %d\n"), WiFi.localIP().toString().c_str(), WiFi.channel());

  if (MDNS.begin("web-clock")) {
    MDNS.addService("http", "tcp", 80);
    Serial.println(F("mDNS responder started"));
  } else {
    Serial.println(F("Error setting up MDNS responder!"));
  }

  return true;
}

#define PTM(w) \
  Serial.print(":" #w "="); \
  Serial.print(tm->tm_##w);

void printTm(const char* what, const tm* tm) {
  Serial.print(what);
  PTM(isdst); PTM(yday); PTM(wday);
  PTM(year);  PTM(mon);  PTM(mday);
  PTM(hour);  PTM(min);  PTM(sec);
}

void showIpOnDisplay() {
  IPAddress ip = WiFi.status() == WL_CONNECTED ?
    WiFi.localIP(): WiFi.softAPIP();

  Serial.printf_P(PSTR("Displaying ip: %s"), ip.toString().c_str());

  for (int8_t i = 0; i < 4; ++i) {
    display->drawNumber(0, 0, 255, ip[i]);
    delay(2000);
  }
  Serial.println(F(" - Done"));
}

void startWebServerAndShowIp() {
  webServer->start();
  showIpOnDisplay();
  display->clear();
}

void updateRtcFromNtp() {
  gettimeofday(&tv, nullptr);
  clock_gettime(0, &tp);
  time_t now = time(nullptr);

  tm *tm = localtime(&now);
  hours = tm->tm_hour;
  mins = tm->tm_min;

  if (!tm->tm_isdst) {
    --hours;
  }

  Serial.printf_P(PSTR("tm %04d-%02d-%02d %02d:%02d:%02d\n"), 1900 + tm->tm_year,
                  tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
  Serial.printf_P(PSTR("++ %04d-%02d-%02d %02d:%02d:%02d\n"), 1900 + tm->tm_year,
                  tm->tm_mon, tm->tm_mday, hours, mins, tm->tm_sec);
  printTm("localtime", localtime(&now));
  Serial.println();

  RtcDateTime dtTime = RtcDateTime(1900 + tm->tm_year, tm->tm_mon, tm->tm_mday, hours,
                                   tm->tm_min, tm->tm_sec);

  Serial.printf_P(PSTR("Year: %d, Mon: %d, Day: %d, %02d:%02d:%02d\n"),
    dtTime.Year(), dtTime.Month(), dtTime.Day(), hours, dtTime.Minute(), dtTime.Second());

  rtc.SetDateTime(dtTime);

  if (!rtc.GetIsRunning()) {
    rtc.SetIsRunning(true);

    Serial.printf_P(PSTR("RTC was not actively running, starting now. Now running? %s\n"),
      rtc.GetIsRunning() ? "Yes" : "No");
  }

  rtc.Enable32kHzPin(false);
  rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
}

void printDateTime(const RtcDateTime &dt) {
  char datestring[30];

  snprintf_P(datestring, sizeof(datestring),
             PSTR("D: %04u-%02u-%02u T: %02u:%02u:%02u\n"), dt.Year(),
             dt.Month(), dt.Day(), dt.Hour(), dt.Minute(), dt.Second());
  Serial.print(datestring);
}


void displayTime(RtcDateTime &rtcNow) {
  bool even = rtcNow.Second() % 2;
  // Serial.println();
  // printTm("localtime", localtime(&now));
  // Serial.println();
  // printTm("gmtime   ", gmtime(&now));
  // Serial.println();
  // Serial.println();

  hours = rtcNow.Hour();
  mins = rtcNow.Minute();

  if (hours > 12) {
    hours -= 12;
  }
  if (hours == 0) {
    hours = 12;  
  }

  //Serial.printf_P(PSTR("RTC %02d:%02d:%02d\n"), rtcNow.Hour(), rtcNow.Minute(), rtcNow.Second());

  //Don't display the leading zero for hours
  if (hours / 10 == 0) {
    display->turnOffDigit(HOUR1);
  } else {
    display->drawDigit(HOUR1, params->red(), params->green(), params->blue(), hours / 10);
  }

  display->drawDigit(HOUR2, params->red(), params->green(), params->blue(), hours - ((hours / 10) * 10));
  display->drawDots(even ? params->red() : 0, even ? params->green() : 0, even ? params->blue() : 0);
  display->drawDigit(MINUTE1, params->red(), params->green(), params->blue(), mins / 10);
  display->drawDigit(MINUTE2, params->red(), params->green(), params->blue(), mins - ((mins / 10) * 10), true);
}

void showTemp() {
  RtcTemperature temp = rtc.GetTemperature();
  int f = (int)round(temp.AsFloatDegF());

  display->drawNumber(f >= 80 ? 255 : 0, (f < 80 && f >= 60) ? 255 : 0, f < 60 ? 255 : 0, f);
  delay(2000);
}
