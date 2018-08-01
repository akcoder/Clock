#include "Parameters.h"

void Parameters::load() {
  Serial.println("Parameters::load");
  EEPROM.begin(512);
  EEPROM.get(0, _red);
  EEPROM.get(1, _green);
  EEPROM.get(2, _blue);
  char ok[2+1];
  EEPROM.get(3, ok);
  EEPROM.end();

  if (String(ok) != String("OK")) {
    Serial.println("EEPROM params not set, resetting");
    _red = _green = _blue = 255;

    write();
  }
  Serial.printf("\n\nred: %d, green: %d, blue: %d\n", _red, _green, _blue);

  EEPROM.end();
}

void Parameters::write() {
  Serial.println("Parameters::write");
  EEPROM.begin(512);
  EEPROM.put(0, _red);
  EEPROM.put(1, _green);
  EEPROM.put(2, _blue);

  char ok[2+1] = "OK";
  EEPROM.put(3, ok);

  EEPROM.commit();
  EEPROM.end();

  Serial.printf("\n\nred: %d, green: %d, blue: %d\n", _red, _green, _blue);
  Serial.println("Done updating EEPROM");
};

uint8_t Parameters::red() { return _red; }
void Parameters::red(uint8_t red) { _red = red; }

uint8_t Parameters::green() { return _green; }
void Parameters::green(uint8_t green) { _green = green; }

uint8_t Parameters::blue() { return _blue; }
void Parameters::blue(uint8_t blue) { _blue = blue; }
