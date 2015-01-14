/*
 *  WiFiTest.ino
 *
 *  Written January 2015
 *  Scott Piette (Piette Technologies, LTD)
 *  Copyright (c) 2015 Piette Technologies, LTD/Users/scottpiette/Dropbox/git/WiFiTest/WiFiTest.cpp
 *
 *  Demonstrates inconsistent behavior of WiFi.ready()
 *
 *  License:  GPL v3 (http://www.gnu.org/licenses/gpl.html)
 *
 *  Application:  DNSTEST.ino
 *
 *  This application is designed to test the network settings on the Spark
 *
 *  Options are -
 *          [1] Start Network w/ WiFi.on / WiFi.connect /Spark.connect
 *          [2] Stop Network w/ Spark.disconnect / WiFi.disconnect / WiFi.off
 *          [2] Stop Network w/ Spark.disconnect / WiFi.off
 *
 */

#include <application.h>

SYSTEM_MODE(MANUAL)

bool bWiFiReady;

char _strMacAddress[18];
char *MacAddress2Str(byte *_mac)
{
	sprintf(_strMacAddress, "%02x:%02x:%02x:%02x:%02x:%02x",
	        _mac[5], _mac[4], _mac[3], _mac[2], _mac[1], _mac[0]);
	return _strMacAddress;
}

void printDNS() {
    IPAddress dnshost(ip_config.aucDNSServer[3], ip_config.aucDNSServer[2], ip_config.aucDNSServer[1], ip_config.aucDNSServer[0]);
    Serial.print("DNS IP      : ");
    Serial.println(dnshost);
}

#if 0
/*
 *  Helper that takes an IP address and turns it into a 32 bit int
 */
uint32_t IP2U32(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
  uint32_t ip = a;
  ip <<= 8;
  ip |= b;
  ip <<= 8;
  ip |= c;
  ip <<= 8;
  ip |= d;

  return ip;
}
#endif

void dumpIP() {
    Serial.println();
    Serial.print("SSID        : ");
    Serial.println(WiFi.SSID());
    Serial.print("RSSI        : ");
    if (bWiFiReady)
        Serial.println(WiFi.RSSI());
    else
        Serial.println();
    Serial.print("Local IP    : ");
    Serial.println(WiFi.localIP());
    Serial.print("Mac         : ");
    byte mac[6];
    WiFi.macAddress(mac);
    Serial.println(MacAddress2Str(mac));
    Serial.print("Subnet mask : ");
    Serial.println(WiFi.subnetMask());
    Serial.print("Gateway IP  : ");
    Serial.println(WiFi.gatewayIP());
    printDNS();
}

void setup() {
    Serial.begin(9600);
    while(!Serial.available()) {
        Serial.println("Press any key to begin");
        delay(1000);
    }
    Serial.read();  // Clear any key
    Serial.println("\r\nWiFi Test App v1.0");
    Serial.println("Written by Scott Piette January 12, 2015");
}

#define MAX_CONNECT_RETRY 10
#define MAX_WIFI_RETRY 100
void loop() {

    byte _ans;
    int _ret;
    int _timeout = 0;
    dumpIP();
    Serial.println("\r\nEnter");
    if (!bWiFiReady)
	Serial.print("   [1] Start Network w/ WiFi.on / WiFi.connect / Spark.connect\r\n");
    else {
        Serial.print("   [2] Stop Network w/ Spark.disconnect / WiFi.disconnect / WiFi.off\r\n");
        Serial.print("   [3] Stop Network w/ Spark.disconnect / WiFi.off\r\n");
    }
    Serial.print("   --> ");
    while(!Serial.available()) { if (WiFi.ready()) Spark.process(); };
    _ans = Serial.read();
    switch (_ans) {
        case '1':
            if (!bWiFiReady) {
                Serial.print("Start Network\r\n\r\nCC3000 -");
                WiFi.on();
                Serial.print(" on\r\n");
                WiFi.connect();
                Serial.print("WiFi +");
                _timeout = 0;
                while (_timeout < MAX_WIFI_RETRY && !WiFi.ready()) { Serial.print("+"); _timeout++; delay(100);}
                if (_timeout == MAX_WIFI_RETRY) {
                    Serial.print("Failed to connect to WiFi router - credentials ");
                    Serial.println(WiFi.hasCredentials() ? "known" : "unknown");
                    WiFi.off();
                }
                else {
                    Serial.print(" connected\r\nCloud +");
                    Spark.connect();
                    _timeout = 0;
                    while (_timeout < MAX_WIFI_RETRY && !Spark.connected()) { Serial.print("+"); _timeout++; delay(100);}
                    if (_timeout == MAX_WIFI_RETRY) {
                	Serial.println("Failed to connect to Spark Cloud.");
                    }
                    else {
                	Serial.println(" connected");
                	bWiFiReady = true;
                    }
                }
            }
            else
        	Serial.println("WiFi is already enabled");
            break;
        case '2':
        case '3':
            if (bWiFiReady) {
                Serial.print("Stop Network\r\n\r\nCloud -");
                Spark.disconnect();
                _timeout = 0;
                while (_timeout < MAX_WIFI_RETRY && Spark.connected()) { Serial.print("-"); _timeout++; delay(100);}
                if (_timeout == MAX_WIFI_RETRY) {
                    Serial.println("Failed to disconnect from cloud");
                }
                else
                  Serial.println(" disconnected");

                if (_ans == '2') {
                    WiFi.disconnect();
                    Serial.print("WiFi -");
                    _timeout = 0;
                    while (_timeout < MAX_WIFI_RETRY && WiFi.ready()) { Serial.print("-"); _timeout++; delay(100); }
                    while (ip_config.aucDNSServer[3] != 0 || ip_config.aucDNSServer[2] != 0)
                      { Serial.print("+"); delay(100); }
                    Serial.println(" disconnected");
                }

                Serial.print("CC3000 -");
                WiFi.off();
                while (WiFi.ready()) { Serial.print("-"); Spark.process(); delay(100); }
                while (ip_config.aucDNSServer[3] != 0 || ip_config.aucDNSServer[2] != 0)
                    { Serial.print("+"); delay(100); }
                Serial.println(" off");
                bWiFiReady = false;
            }
            else
                Serial.println("Please turn on network");
            break;
        default:
            Serial.println("Invalid selection.");
            break;
    }
}
