#ifndef h_IP5306_h
  #define h_IP5306_h

// ================ Power IC IP5306 ===================
  #define IP5306_ADDR (117) // 0x75
  #define IP5306_REG_SYS_CTL0 (0x00)
  #define IP5306_REG_SYS_CTL1 (0x01)
  #define IP5306_REG_SYS_CTL2 (0x02)
  #define IP5306_REG_READ0 (0x70)
  #define IP5306_REG_READ1 (0x71)
  #define IP5306_REG_READ3 (0x78)
  #define IP5306_REG_CHG_CTL0 (0x20)
  #define IP5306_REG_CHG_CTL1 (0x21)
  #define IP5306_REG_CHG_DIG  (0x24)

//- SYS_CTL0
  #define BOOST_ENABLE_BIT (0x20)
  #define CHARGE_OUT_BIT (0x10)
  #define BOOT_ON_LOAD_BIT (0x04)
  #define BOOST_OUT_BIT (0x02)
  #define BOOST_BUTTON_EN_BIT (0x01)

//- SYS_CTL1
  #define BOOST_SET_BIT (0x80)
  #define WLED_SET_BIT (0x40)
  #define SHORT_BOOST_BIT (0x20)
  #define VIN_ENABLE_BIT (0x04)
  #define BATLOW_SHUTDOWN_BIT (0x01)

//- SYS_CTL2
  #define SHUTDOWNTIME_MASK (0x0c)
  #define SHUTDOWNTIME_64S (0x0c)
  #define SHUTDOWNTIME_32S (0x04)
  #define SHUTDOWNTIME_16S (0x08)
  #define SHUTDOWNTIME_8S  (0x00)

//- REG_READ0
  #define CHARGE_ENABLE_BIT (0x08)

//- REG_READ1
  #define CHARGE_MODE_CV (0x60)
  #define CHARGE_MODE_CC (0x40)
  #define CHARGE_FULL_BIT (0x08)

//- REG_READ2
  #define LIGHT_LOAD_BIT (0x20)
  #define LOWPOWER_SHUTDOWN_BIT (0x01)

//- CHG_DIG
  #define CURRENT_100MA  (0x01 << 0)
  #define CURRENT_200MA  (0x01 << 1)
  #define CURRENT_400MA  (0x01 << 2)
  #define CURRENT_800MA  (0x01 << 3)
  #define CURRENT_1600MA  (0x01 << 4)

//- CHG_CTL2[3:2]
  #define BAT_4_2V      (0x00)
  #define BAT_4_3V      (0x01)
  #define BAT_4_3_5V    (0x02)
  #define BAT_4_4V      (0x03)

//- CHG_CTL3
  #define CHG_CC_BIT    (0x20)

  uint8_t i2cdata;

#endif

/*
  //I2C sample
  M5.I2C.readByte(IP5306_ADDR, 0x21, &i2cdata);
  M5.I2C.writeByte(IP5306_ADDR, 0x21, (i2cdata & 0x3f));

  POWER();
  bool canControl();
  void begin();
  
  // -- control for power
  bool setKeepLightLoad(bool en) __attribute__((deprecated));
  bool setPowerBoostKeepOn(bool en);
  bool setAutoBootOnLoad(bool en);
  bool setLowPowerShutdown(bool en) __attribute__((deprecated));    
  bool setLowPowerShutdownTime(ShutdownTime time);
  bool setPowerBoostOnOff(bool en);
  bool setPowerBoostSet(bool en);
  bool setPowerVin(bool en);
  bool setPowerWLEDSet(bool en);
  bool setPowerBtnEn(bool en);

  // -- control for battery
  bool setVinMaxCurrent(uint8_t cur);
  bool setChargeVolt(uint8_t volt);
  
  bool setCharge(bool en);
  bool isChargeFull();
  bool isCharging();
  int8_t getBatteryLevel();
  bool batteryMode(bool en);
  
  // -- configuration for wakeup
  void setWakeupButton(uint8_t button);
  
  // -- get resson for startup
  bool isResetbyWatchdog();
  bool isResetbyDeepsleep();
  bool isResetbySoftware();
  bool isResetbyPowerSW();
  
  // -- sleep
  void deepSleep(uint64_t time_in_us = 0);
  void lightSleep(uint64_t time_in_us = 0);
  
  // -- power off
  void powerOFF();
  
  // -- software reset
  void reset();
  
private:
  uint8_t _wakeupPin;
*/
