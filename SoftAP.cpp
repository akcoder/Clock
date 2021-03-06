#include "SoftAP.h"
#include "ClockWebServer.h"

void SoftAP::init() {
  startAp();

  /* Setup the DNS server redirecting all the domains to the apIP */
  _dnsServer = new DNSServer();
  _dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
  _dnsServer->start(DNS_PORT, "*", WiFi.softAPIP());
}

void SoftAP::run() {
  _display->clear();
  _display->setBits(MINUTE1, 0, 255, 0, 0b01011111);       // A
  _display->setBits(MINUTE2, 0, 255, 0, 0b00011111, true); // P

  while(1) {
    _dnsServer->processNextRequest();
    yield();
  }
}

void SoftAP::startAp() {
  IPAddress apIP(192, 168, 4, 1);
  IPAddress netMask(255, 255, 255, 0);

  WiFi.softAPConfig(apIP, apIP, netMask);
  WiFi.softAP(_ssid, _passphrase);

  delay(500); // Without delay I've seen the IP address blank

  Serial.print(F("AP IP address: "));
  Serial.println(WiFi.softAPIP());
}