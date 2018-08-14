#include "Parameters.h"

void Parameters::load() {
  Serial.println("Parameters::load");
  char ok[2+1];
  Config config;
  EEPROM.begin(sizeof(Config) + sizeof("OK"));

  EEPROM.get(0, config);
  EEPROM.get(sizeof(Config), ok);

  EEPROM.end();

  Serial.printf("ok: {%s}\n", ok);
  if (String(ok) == String("OK")) {
    red(config.red);
    green(config.green);
    blue(config.blue);
    ssid(config.ssid);
    passphrase(config.passphrase);
  } else {
    Serial.println("EEPROM params not set, resetting");
    red(255);
    green(255);
    blue(255);
    ssid("");
    passphrase("");

    write();
  }

  Serial.printf_P(PSTR("Params: red: %d, green: %d, blue: %d\n"), red(), green(), blue());
  Serial.printf_P(PSTR("Params: ssid: %s, passphrase: %s\n"), ssid().c_str(), passphrase().c_str());
}

void Parameters::write() {
  Serial.println("Parameters::write");
  EEPROM.begin(sizeof(Config) + sizeof("OK"));

  Config config;
  config.red = red();
  config.green = green();
  config.blue = blue();
  ssid().toCharArray(config.ssid, sizeof(config.ssid));
  passphrase().toCharArray(config.passphrase, sizeof(config.passphrase));

  EEPROM.put(0, config);

  Serial.println("EEPROM.put OK");
  EEPROM.put(sizeof(Config), "OK");

  EEPROM.commit();
  EEPROM.end();

  Serial.printf("\n\nred: %d, green: %d, blue: %d\n", config.red, config.green, config.blue);
  Serial.printf("ssid: %s, passphrase: %s\n", config.ssid, config.passphrase);
  Serial.println("Done updating EEPROM");
};

uint8_t Parameters::red() { return _red; }
void Parameters::red(uint8_t red) { _red = red; }

uint8_t Parameters::green() { return _green; }
void Parameters::green(uint8_t green) { _green = green; }

uint8_t Parameters::blue() { return _blue; }
void Parameters::blue(uint8_t blue) { _blue = blue; }

String Parameters::ssid() { return _ssid; }
void Parameters::ssid(String ssid) {
  _ssid = String(ssid);
}

String Parameters::passphrase() { return _passphrase; }
void Parameters::passphrase(String passphrase) {
  _passphrase = String(passphrase);
}