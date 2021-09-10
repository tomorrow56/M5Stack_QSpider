#include "duke32.h"

/********************************
* for Wifi Connection
*********************************/
// Add Feature String
void AddFeatureString(String &S, const String F){
  if (S.length() != 0) S.concat(", ");
  S.concat(F);
}

uint64_t GetChipid(){
  // Chip Info
  const char* ModelStrings[] PROGMEM = {"", "ESP32"};
  uint64_t chipid;
  // Get Chip Information
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  Serial.printf("Model: %s\r\n", ModelStrings[chip_info.model]);

  // Features
  String Features = "";
  if (chip_info.features & CHIP_FEATURE_EMB_FLASH) AddFeatureString(Features, "Embedded Flash");
  if (chip_info.features & CHIP_FEATURE_WIFI_BGN ) AddFeatureString(Features, "Wifi-BGN"      );
  if (chip_info.features & CHIP_FEATURE_BLE      ) AddFeatureString(Features, "BLE"           );
  if (chip_info.features & CHIP_FEATURE_BT       ) AddFeatureString(Features, "Bluetooth"     );
  Serial.println("Features: " + Features);
  // Cores
  Serial.printf("Cores: %d\r\n", chip_info.cores);
  // Revision
  Serial.printf("Revision: %d\r\n", chip_info.revision);
  // MAC Address
  String MACString = "";
  chipid = ESP.getEfuseMac();
  for (int i=0; i<6; i++) {
    if (i > 0) MACString.concat(":");
    uint8_t Octet = chipid >> (i * 8);
    if (Octet < 16) MACString.concat("0");
    MACString.concat(String(Octet, HEX));
  }
  Serial.println("MAC Address: " + MACString);
  // Flash Size
  uint32_t FlashSize = ESP.getFlashChipSize();
  String ValueString = "";
  do {
    ValueString = String(FlashSize % 1000) + ValueString;
    FlashSize /= 1000;
    if (FlashSize > 0)
      ValueString = "," + ValueString;
  } while (FlashSize > 0);
  Serial.println("Flash Size: " + ValueString);
  // ChipID
  chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
  Serial.printf("Chip ID = %04X",(uint16_t)(chipid>>32));//print High 2 bytes
  Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.
  return chipid;
}

void WiFiMgrSetup(char WiFiAPname[]){
  //WiFiManager
  Serial.printf("AP: %s\n", WiFiAPname);
  Serial.println("IP: 192.168.4.1");

  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  // WiFiManager̃
  wifiManager.setDebugOutput(false);

  //set custom ip for portal
  //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //or use this for auto generated name ESP + ChipID
  wifiManager.autoConnect(WiFiAPname);
  //wifiManager.autoConnect("NoseRadi32AP");

  //if you get here you have connected to the WiFi
  Serial.println("connected(^^)");
  Serial.println(WiFi.SSID());
  Serial.println(WiFi.localIP());
}

void OTASetup(char OTAHostname[]){
  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  // ArduinoOTA.setHostname("myesp32");
  ArduinoOTA.setHostname(OTAHostname);
  Serial.printf("OTA Host: %s\n", OTAHostname);

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

// --------------------------------------
// i2c_scanner
// --------------------------------------

void i2c_scan(){
  Serial.println("\nI2C Scanner");
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16){
        Serial.print("0");
      }
      Serial.print(address,HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error==4) 
    {
      Serial.print("Unknow error at address 0x");
      if (address<16){
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0){
    Serial.println("No I2C devices found\n");
  }else{
    Serial.println("done\n");
  }
}
