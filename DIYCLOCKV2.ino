#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "digits.h"
#include "ClockDisplay.h"
#include "ClockWebServer.h"
#include "Parameters.h"

#include <stdio.h>
#include <string.h>
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval
#include <coredecls.h>                  // settimeofday_cb()

// for testing purpose:
extern "C" int clock_gettime(clockid_t unused, struct timespec *tp);

static char ssid[] = "Morphis (N)";
static char pass[] = "MorphisFamily";

int hours = 0;
int mins = -1;

#define TZ              -8       // (utc+) TZ in hours
#define DST_MN          60       // use 60mn for summer time in some countries

#define TZ_MN           ((TZ)*60)
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)

#define LED_PIN 5 /* d1 on the node mcu board */

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
ClockDisplay display = ClockDisplay(pixels);
Parameters params = Parameters();
ClockWebServer webServer = ClockWebServer(80, &params);

timeval tv;
timespec tp;
time_t now;

timeval cbtime;			// time set in callback
bool cbtime_set = false;

void time_is_set(void) {
  gettimeofday(&cbtime, NULL);
  cbtime_set = true;
  Serial.println(F("------------------ settimeofday() was called ------------------"));
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  params.load();

  Serial.println("Testing display");
  display.begin();
  display.clear();

  display.test();

  // Indicate which step we are on...
  display.drawDigit(MINUTE2, 0, 0, 255, 1, true);
  setupWiFi();
  showIpOnDisplay();

  display.drawDigit(MINUTE2, 0, 0, 255, 2, true);
  configTime(TZ_SEC, DST_SEC, "pool.ntp.org");

  display.drawDigit(MINUTE2, 0, 0, 255, 3, true);
  settimeofday_cb(time_is_set);

  display.drawDigit(MINUTE2, 0, 0, 255, 4, true);
  webServer.start();

  pinMode(2, OUTPUT);
}

void loop() {
  gettimeofday(&tv, nullptr);
  clock_gettime(0, &tp);
  now = time(nullptr);

  // localtime / gmtime every second change
  static time_t lastv = 0;
  if (lastv != tv.tv_sec) {
    digitalWrite(2, tv.tv_sec % 2); // Heartbeat the onboard led
    lastv = tv.tv_sec;
    // Serial.println();
    // printTm("localtime", localtime(&now));
    // Serial.println();
    // printTm("gmtime   ", gmtime(&now));
    // Serial.println();
    // Serial.println();

    tm* tm = localtime(&now);
    hours = tm->tm_hour;
    mins = tm->tm_min;

    if (hours > 12) {
      hours -= 12;
      if (!tm->tm_isdst) {
        --hours;
      }
    }
    if (hours == 0) {
      hours = 12;  
    }

    // Serial.printf("tm %02d:%02d:%02d\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
    // Serial.printf("++ %d%d:%d%d:%d%d\n",
    //   hours / 10, hours - ((hours / 10) * 10),
    //   mins / 10, mins - ((mins / 10) * 10),
    //   tm->tm_sec / 10, tm->tm_sec - ((tm->tm_sec / 10) * 10));

    //Don't display the leading zero for hours
    if (hours / 10 == 0) {
      display.turnOffDigit(HOUR1, false);
    } else {
      display.drawDigit(HOUR1, params.red(), params.green(), params.blue(), hours / 10);
    }

    display.drawDigit(HOUR2, params.red(), params.green(), params.blue(), hours - ((hours / 10) * 10));
    display.drawDots(params.red(), params.green(), params.blue());
    display.drawDigit(MINUTE1, params.red(), params.green(), params.blue(), mins / 10);
    display.drawDigit(MINUTE2, params.red(), params.green(), params.blue(), mins - ((mins / 10) * 10), true);
  }


  // simple drifting loop
  delay(100);

  // while (true) {
  //   for (int i = 0; i < 9999; ++i) {
  //     drawDigit(HOUR1, params.red(), params.green(), params.blue(), 1000 % i );
  //     drawDigit(HOUR2, params.red(), params.green(), params.blue(), 100 % i);
  //     drawDigit(MINUTE1, params.red(), params.green(), params.blue(), 10 % i);
  //     drawDigit(MINUTE2, params.red(), params.green(), params.blue(), i % 1 );
      
  //     delay(50);
  //   }
  // }
}

void setupWiFi() {
  Serial.print(F("Connecting to "));
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");

  Serial.println(F("WiFi connected"));
  Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());

  if (!MDNS.begin("web-clock")) {             // Start the mDNS responder for web-clock.local
    Serial.println(F("Error setting up MDNS responder!"));
  }
  Serial.println(F("mDNS responder started"));
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
  IPAddress ip = WiFi.localIP();
  for (int8_t i = 0; i < 4; ++i) {
    display.drawNumber(0, 0, 255, ip[i]);
    delay(2000);
  }

  Serial.println();
  Serial.println();
}
