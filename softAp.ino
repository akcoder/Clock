#include <Arduino.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h>

/**
 * Maybe switch over to https://github.com/tzapu/WiFiManager
 * It handles getting the Wi-Fi creds, and works in captive portal mode
 */
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

const char *softAP_ssid = "Clock-AP";
const char *softAP_password = "Remington";

const byte DNS_PORT = 53;
DNSServer dnsServer;

void startSoftAP() {
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(softAP_ssid, softAP_password);

  delay(500); // Without delay I've seen the IP address blank
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);
}
