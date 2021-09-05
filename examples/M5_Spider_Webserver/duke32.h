#ifndef _duke32_h_
#define _duke32_h_

/********************************
* for Wifi Connection
*********************************/
#include <WiFi.h>
// https://github.com/zhouhan0126/DNSServer---esp32
#include <DNSServer.h>
// https://github.com/zhouhan0126/WebServer-esp32
#include <WebServer.h>
#include <ESPmDNS.h>
// https://github.com/zhouhan0126/WIFIMANAGER-ESP32
#include "WiFiManager.h"

#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <Wire.h>

uint64_t GetChipid();
void WiFiMgrSetup(char WiFiAPname[]);
void OTASetup(char OTAHostname[]);

// --------------------------------------
// i2c_scanner
// --------------------------------------
void i2c_scan();

#endif
