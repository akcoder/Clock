#include "SoftAP.h";
#include "ClockWebServer.h";

SoftAP::SoftAP(const char *ssid, const char *passphrase, Parameters *params,
               ClockDisplay * display) {
  _display = display;
  _params = params;
  _ssid = ssid;
  _passphrase = passphrase;
}

void SoftAP::init() {
  _display->setBits(MINUTE1, 0, 255, 0, 0b01011111);       // A
  _display->setBits(MINUTE2, 0, 255, 0, 0b00011111, true); // P

  startAp();

  /* Setup the DNS server redirecting all the domains to the apIP */
  _dnsServer = new DNSServer();
  _dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
  _dnsServer->start(DNS_PORT, "*", WiFi.softAPIP());
}

void SoftAP::run() {
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

  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
}