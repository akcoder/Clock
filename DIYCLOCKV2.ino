#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include "digits.h"

#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval
#include <coredecls.h>                  // settimeofday_cb()

// for testing purpose:
extern "C" int clock_gettime(clockid_t unused, struct timespec *tp);

static char ssid[] = "Morphis (N)";
static char pass[] = "MorphisFamily";

int hours = 0;
int mins = 0;

#define TZ              -8       // (utc+) TZ in hours
#define DST_MN          60       // use 60mn for summer time in some countries

#define TZ_MN           ((TZ)*60)
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)

#define LED_PIN 5 /* d1 on the node mcu board */
#define NUMPIXELS MINUTE2 + DIGIT_SIZE

unsigned int localPort = 2390;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

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
  pixels.begin();
  clearDisplay();

  Serial.begin(115200);
  Serial.println();
  testDisplay();

  //Indicate which step we are on...
  drawDigit(MINUTE2, 255, 0, 0, 1);
  setupWiFi();

  drawDigit(MINUTE2, 255, 0, 0, 2);
  configTime(TZ_SEC, DST_SEC, "pool.ntp.org");

  drawDigit(MINUTE2, 255, 0, 0, 3);
  settimeofday_cb(time_is_set);
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
    Serial.println();
    printTm("localtime", localtime(&now));
    Serial.println();
    printTm("gmtime   ", gmtime(&now));
    Serial.println();
    Serial.println();

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

    
    Serial.printf("tm %02d:%02d:%02d\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
    Serial.printf("++ %d%d:%d%d:%d%d\n",
      hours / 10, hours - ((hours / 10) * 10),
      mins / 10, mins - ((mins / 10) * 10),
      tm->tm_sec / 10, tm->tm_sec - ((tm->tm_sec / 10) * 10));

    //Don't display the leading zero for hours
    if (hours / 10 == 0) {
      drawDigit(HOUR1, 0, 0, 0, 0);
    } else {
      drawDigit(HOUR1, 255, 0, 0, hours / 10);
    }

    drawDigit(HOUR2, 255, 0, 0, hours - ((hours / 10) * 10));
    pixels.setPixelColor(DOT1, pixels.Color(255, 0, 0));
    pixels.setPixelColor(DOT2, pixels.Color(255, 0, 0));
    drawDigit(MINUTE1, 255, 0, 0, mins / 10);
    drawDigit(MINUTE2, 255, 0, 0, mins - ((mins / 10) * 10));
  }

  // simple drifting loop
  delay(100);

  // while (true) {
  //   for (int i = 0; i < 9999; ++i) {
  //     drawDigit(HOUR1, 255, 0, 0, 1000 % i );
  //     drawDigit(HOUR2, 255, 0, 0, 100 % i);
  //     drawDigit(MINUTE1, 255, 0, 0, 10 % i);
  //     drawDigit(MINUTE2, 255, 0, 0, i % 1 );
      
  //     delay(50);
  //   }
  // }
}

void drawDigit(int offset, int r, int g, int b, int n) {
  int8 digit = digits[n];

  uint32_t on = pixels.Color(r, g, b);
  uint32_t off = pixels.Color(0, 0, 0);

  for (int i = 0; i < 8; ++i) { // 7 bits
    for (int p = 0; p < SEGMENT_SIZE; ++p) {
      bool isSet = (1 << i) & digit;
      pixels.setPixelColor((i * SEGMENT_SIZE) + p + offset, isSet ? on : off);
    }
  }

  pixels.show();
}

void clearDisplay() {
  drawDigit(HOUR1, 0, 0, 0, 0);
  drawDigit(HOUR2, 0, 0, 0, 0);
  pixels.setPixelColor(DOT1, 0);
  pixels.setPixelColor(DOT2, 0);
  drawDigit(MINUTE1, 0, 0, 0, 0);
  drawDigit(MINUTE2, 0, 0, 0, 0);
}

void testDisplay() {
  Serial.println(F("Running self test"));
  for (int i = 0; i < 10; ++i) {
    drawDigit(HOUR1, 255, 0, 0, i);
    drawDigit(HOUR2, 255, 0, 0, i);

    uint32_t dotColor = i % 2 ? pixels.Color(0, 0, 0) : pixels.Color(255, 0, 0);
    pixels.setPixelColor(DOT1, dotColor);
    pixels.setPixelColor(DOT2, dotColor);

    drawDigit(MINUTE1, 255, 0, 0, i);
    drawDigit(MINUTE2, 255, 0, 0, i);
    delay(500);
  }

  clearDisplay();
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
  Serial.print(F("IP address: "));
  Serial.println(WiFi.localIP());
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