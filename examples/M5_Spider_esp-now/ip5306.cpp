#include "ip5306.h"
#include <M5Stack.h>

boolean getChargeEnable(){
  uint8_t data;
  M5.I2C.readByte(IP5306_ADDR, IP5306_REG_READ0, &data);
  return((data & CHARGE_ENABLE_BIT) == 0x08);
}

uint8_t getChargeMode(){
  uint8_t data;
  M5.I2C.readByte(IP5306_ADDR, IP5306_REG_READ1, &data);
  if((data & CHARGE_FULL_BIT)){
    return 1;
  }else if((data & 0xE0) == CHARGE_MODE_CV){
    return 2;
  }else if((data & 0xE0) == CHARGE_MODE_CC){
    return 3;
  }else{
    return 0;
  }
}
