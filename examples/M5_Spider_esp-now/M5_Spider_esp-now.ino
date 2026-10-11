#include <M5Unified.h>

#include <NESpad.h>
#include <esp_now.h>
#include <WiFi.h>

#include <Adafruit_PWMServoDriver.h>
#include "avatar.h"

#define DEBUG

/********************
 * NESPad parameters
 ********************/
#define nesStrobe 22
#define nesClock 19
#define nesData 23
NESpad nintendo = NESpad(nesStrobe,nesClock,nesData);
byte state = 0;

/********************
 * ESPNow settings
 ********************/
esp_now_peer_info_t slave;
int modeNES = 1;  //0:Multicast, 1:M5SpiderW, 2:M5SpiderB, other: Multicast
esp_err_t result;
boolean sending = false;

// command from Master
#define rNES_STOP 0
#define rNES_A 1
#define rNES_B 2
#define rNES_SELECT 3
#define rNES_START 4
#define rNES_UP 5
#define rNES_DOWN 6
#define rNES_LEFT 7
#define rNES_RIGHT 8

/********************************
* for Servo control
*********************************/
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x72);

// Published values for SG90 servos; 0-180 deg
const int minUs = 500;
const int maxUs = 2400;

// Servo assign to each channel
#define FR0 0   // knees
#define FL0 2
#define RR0 6
#define RL0 8

#define FR1 1   // leg
#define FL1 3
#define RR1 7
#define RL1 9

#define TILT 4

// center position calibration(0-180 deg)
#define OFST 20

uint8_t CENT_FR0 = 90 + OFST;   // knees UP ++
uint8_t CENT_FL0 = 90 - OFST;   // knees UP --
uint8_t CENT_RR0 = 90 - OFST;   // knees UP --
uint8_t CENT_RL0 = 90 + OFST;   // knees UP ++

uint8_t CENT_FR1 = 90;   // leg forward ++
uint8_t CENT_FL1 = 90;   // leg forward --
uint8_t CENT_RR1 = 90;   // leg backword ++
uint8_t CENT_RL1 = 90;   // leg backword --

uint8_t CENT_TILT = 90; // up --

const int stp = 3;
const int smooth = 5;

const int stepInterval = 50;
const int stepIntervalTurn = 100;

boolean moveFlag = false;
boolean walkFwdFlag = false;
boolean walkBckFlag = false;
boolean walkLeftFlag = false;
boolean walkRightFlag = false;
boolean spinLeftFlag = false;
boolean spinRightFlag = false;

//String Face = "/GALLERY/KumaMCN.jpg";

Avatar *avatar;
TaskHandle_t avatarTaskHandle = NULL;

/************************************
 * Update Avatar Task
 ************************************/
void avatarUpdate(void* arg) {
  int cnt = 0;
  while (1) {
    if(cnt == 0){
      //avatar->openMouth(15);
    }
    if (cnt % 100 == 97){
      avatar->openEye(false);
    }
    if (cnt % 100 == 0){
      avatar->openEye(true);
      cnt = random(100);
    }
    cnt++;
    vTaskDelay(50);
  }
}

void setMoveFlag(boolean &targetFlag) {
  boolean commandChanged = !targetFlag;

  walkFwdFlag = false;
  walkBckFlag = false;
  walkLeftFlag = false;
  walkRightFlag = false;
  spinLeftFlag = false;
  spinRightFlag = false;

  if(commandChanged == true){
    moveFlag = false;
  }
  targetFlag = true;
}

// 受信コールバック
void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int data_len) {
  const uint8_t *mac_addr = recv_info->src_addr;
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.printf("Last Packet Recv from: %s\n", macStr);
  Serial.printf("Last Packet Recv Data(%d): ", data_len);
  for ( int i = 0 ; i < data_len ; i++ ) {
    Serial.print(data[i]);
    Serial.print(" ");
  }
  Serial.println("");

// 送信元でフィルタ(ESP-NOW送信機のMACアドレスに合わせて修正)
  if((String)macStr == "4C:75:25:C6:F8:0C" || (String)macStr == "D8:A0:1D:5B:D1:AC"){
    if(data[0] == rNES_A){
      Serial.println("NES_A");
      setMoveFlag(spinRightFlag);
      #ifdef DEBUG
        Serial.println("Spin Right");
      #endif
    }else if(data[0] == rNES_B){
      Serial.println("NES_B");
      setMoveFlag(spinLeftFlag);
      #ifdef DEBUG
        Serial.println("Spin Left");
      #endif
    }else if(data[0] == rNES_SELECT){
      Serial.println("NES_SELECT");
    }else if(data[0] == rNES_START){
      Serial.println("NES_START");
    }else if(data[0] == rNES_UP){
      Serial.println("NES_UP");
      setMoveFlag(walkFwdFlag);
      #ifdef DEBUG
        Serial.println("Walk Forward");
      #endif
    }else if(data[0] == rNES_DOWN){
      Serial.println("NES_DOWN");
      setMoveFlag(walkBckFlag);
      #ifdef DEBUG
        Serial.println("Walk Backward");
      #endif
    }else if(data[0] == rNES_LEFT){
      Serial.println("NES_LEFT");
      setMoveFlag(walkLeftFlag);
      #ifdef DEBUG
        Serial.println("Walk Left");
      #endif
    }else if(data[0] == rNES_RIGHT){
      Serial.println("NES_RIGHT");
      setMoveFlag(walkRightFlag);
      #ifdef DEBUG
        Serial.println("Walk Right");
      #endif
    }else if(data[0] == rNES_STOP){
      Serial.println("NES_STOP");
      walkStop();
    }else{
      Serial.println("Unknown");
    }
  }
}

/************************************
 * Arduino block
 ************************************/
void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  // Mute speaker
  M5.Speaker.setVolume(0);
  M5.Speaker.end();

  Serial.println("M5Stack Q-Spider esp-now");
  Serial.printf("NES mode: %d\n", modeNES);

/*
 * Servo setup
 */
 //modeNES 1:M5SpiderW, 2:M5SpiderB
  if(modeNES == 1){
    CENT_FR0 = 90 + OFST;   // knees UP ++
    CENT_FL0 = 90 - OFST;   // knees UP --
    CENT_RR0 = 90 - OFST;   // knees UP --
    CENT_RL0 = 90 + OFST;   // knees UP ++
    CENT_FR1 = 90;   // leg forward ++
    CENT_FL1 = 90;   // leg forward --
    CENT_RR1 = 90;   // leg backword ++
    CENT_RL1 = 90;   // leg backword --
    CENT_TILT = 100; // up --
  }else if(modeNES == 2){
    CENT_FR0 = 85 + OFST;   // knees UP ++
    CENT_FL0 = 85 - OFST;   // knees UP --
    CENT_RR0 = 95 - OFST;   // knees UP --
    CENT_RL0 = 90 + OFST;   // knees UP ++
    CENT_FR1 = 85;   // leg forward ++
    CENT_FL1 = 90;   // leg forward --
    CENT_RR1 = 90;   // leg backword ++
    CENT_RL1 = 95;   // leg backword --
    CENT_TILT = 110; // up --
  }

  pwm.begin();
  pwm.setPWMFreq(50);  // Analog servos run at 50 Hz updates
  delay(10);

  Serial.println("Ready");

  avatar = new Avatar();
  M5.Display.fillScreen(SECONDARY_COLOR);
  avatar->init();

/*
 * Demo action
 */
//  servoDemo();
  servoHome();

  walkStop();

  // ESP-NOW初期化
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  } else {
    Serial.println("ESPNow Init Failed");
    ESP.restart();
  }

// ESP-NOWコールバック登録
  esp_now_register_recv_cb(OnDataRecv);

  // xTaskCreatePinnedToCore(タスクの関数名,"タスク名",スタックメモリサイズ,NULL,タスク優先順位,タスクハンドルポインタ,Core ID);
  // xTaskCreatePinnedToCore(func,"name",Stuck size,NULL,priority,Task pointer,Core ID)
  // Core ID: 0 or 1 or tskNO_AFFINITY
  xTaskCreatePinnedToCore(avatarUpdate, "Task0", 4096, NULL, 1, &avatarTaskHandle, 0);
}

void loop(){
  M5.update();

  if(walkFwdFlag == true){
    if(moveFlag == false){
      walkFwdInit();
      moveFlag = true;
    }
    stepForward();
  }

  if(walkBckFlag == true){
    if(moveFlag == false){
      walkBckInit();
      moveFlag = true;
    }
    stepBackward();
  }

  if(walkLeftFlag == true){
    if(moveFlag == false){
      walkLeftInit();
      moveFlag = true;
    }
    stepLeft();
  }

  if(walkRightFlag == true){
    if(moveFlag == false){
      walkRightInit();
      moveFlag = true;
    }
    stepRight();
  }

  if(spinLeftFlag == true){
    if(moveFlag == false){
      spinLeftInit();
      moveFlag = true;
    }
    stepSpinLeft();
  }

  if(spinRightFlag == true){
    if(moveFlag == false){
      spinRightInit();
      moveFlag = true;
    }
    stepSpinRight();
  }

  if(M5.BtnB.wasPressed()){
    walkStop();
    adjMode();
  }

}

/************************************
 * Demo Action
 ************************************/
// Servo Demo
void servoDemo(){
  servoHome();
  delay(500);

  servoLegOpen(45);
  delay(500);
  servoStandUp(70);
  delay(500);
  servoLegClose(45);
  delay(500);
  servoLayDown(70);
  delay(500);

  walkStop();
  delay(500);

  servoTilt(10);
  delay(500);
  servoTilt(0);
}

// Home position
void servoHome(){
  moveServo(TILT, 0);

  moveServo(FR1, 0);
  moveServo(FL1, 0);
  moveServo(RR1, 0);
  moveServo(RL1, 0);

  moveServo(FR0, 0);
  moveServo(FL0, 0);
  moveServo(RR0, 0);
  moveServo(RL0, 0);
}

/************************************
 * Stop
 ************************************/
void walkStop(){
  walkFwdFlag = false;
  walkBckFlag = false;
  walkLeftFlag = false;
  walkRightFlag = false;
  spinLeftFlag = false;
  spinRightFlag = false;

  moveFlag = false;
  walkEnd();

  #ifdef DEBUG
    Serial.println("Walk Stop");
  #endif
}

// END position
void walkEnd(){
  delay(stepInterval);
  servoLegFront(45);
  servoLegRear(45);
  servoKneesAll(70);
}

/************************************
 * Walk
 ************************************/
// Walk Forward
void walkFwdInit(){
  servoLegFront(45);
  servoLegRear(45);
  servoKneesAll(70);
  delay(stepInterval);

  // initial position for walk Forward
  moveServo(RL0, -50);
  moveServo(RL1, 0);
  delay(stepInterval);
  moveServo(RL0, -70);
}

void stepForward(){
  moveServo(FL0, -50);
  moveServo(RR0, -50);
  delay(stepInterval);

  moveServo(FL1, 45);
  delay(stepInterval);

  moveServo(FR1, 0);
  moveServo(RL1, 45);
  delay(stepInterval);

  moveServo(RR1, 0);
  delay(stepInterval);

  moveServo(FL0, -70);
  moveServo(RR0, -70);
  delay(stepInterval);

  moveServo(FR0, -50);
  moveServo(RL0, -50);
  delay(stepInterval);

  moveServo(FR1, 45);
  delay(stepInterval);

  moveServo(FL1, 0);
  moveServo(RR1, 45);
  delay(stepInterval);

  moveServo(RL1, 0);
  delay(stepInterval);

  moveServo(FR0, -70);
  moveServo(RL0, -70);
  delay(stepInterval);
}

// Walk Backward
void walkBckInit(){
  servoLegFront(45);
  servoLegRear(45);
  servoKneesAll(70);
  delay(stepInterval);

  // initial position for walk Back
  moveServo(FR0, -50);
  moveServo(FR1, 0);
  delay(stepInterval);
  moveServo(FR0, -70);
}

void stepBackward(){
  moveServo(FL0, -50);
  moveServo(RR0, -50);
  delay(stepInterval);

  moveServo(RR1, 45);
  delay(stepInterval);

  moveServo(FR1, 45);
  moveServo(RL1, 0);
  delay(stepInterval);

  moveServo(FL1, 0);
  delay(stepInterval);

  moveServo(FL0, -70);
  moveServo(RR0, -70);
  delay(stepInterval);

  moveServo(FR0, -50);
  moveServo(RL0, -50);
  delay(stepInterval);

  moveServo(RL1, 45);
  delay(stepInterval);

  moveServo(FL1, 45);
  moveServo(RR1, 0);
  delay(stepInterval);

  moveServo(FR1, 0);
  delay(stepInterval);

  moveServo(FR0, -70);
  moveServo(RL0, -70);
  delay(stepInterval);
}

// Walk Left
void walkLeftInit(){
  servoLegFront(45);
  servoLegRear(45);
  servoKneesAll(70);
  delay(stepInterval);

  // initial position for walk Left
  moveServo(FR0, -50);
  moveServo(FR1, 90);
  delay(stepInterval);
  moveServo(FR0, -70);
}

void stepLeft(){
  moveServo(FL0, -50);
  moveServo(RR0, -50);
  delay(stepInterval);

  moveServo(FL1, 45);
  delay(stepInterval);

  moveServo(FR1, 45);
  moveServo(RL1, 90);
  delay(stepInterval);

  moveServo(RR1, 90);
  delay(stepInterval);

  moveServo(FL0, -70);
  moveServo(RR0, -70);
  delay(stepInterval);

  moveServo(FR0, -50);
  moveServo(RL0, -50);
  delay(stepInterval);

  moveServo(RL1, 45);
  delay(stepInterval);

  moveServo(FL1, 90);
  moveServo(RR1, 45);
  delay(stepInterval);

  moveServo(FR1, 90);
  delay(stepInterval);

  moveServo(FR0, -70);
  moveServo(RL0, -70);
  delay(stepInterval);
}

// Walk Right
void walkRightInit(){
  servoLegFront(45);
  servoLegRear(45);
  servoKneesAll(70);
  delay(stepInterval);

  // initial position for walk Right
  moveServo(RL0, -50);
  moveServo(RL1, 90);
  delay(stepInterval);
  moveServo(RL0, -70);
}

void stepRight(){
  moveServo(FL0, -50);
  moveServo(RR0, -50);
  delay(stepInterval);

  moveServo(RR1, 45);
  delay(stepInterval);

  moveServo(RL1, 45);
  moveServo(FR1, 90);
  delay(stepInterval);

  moveServo(FL1, 90);
  delay(stepInterval);

  moveServo(FL0, -70);
  moveServo(RR0, -70);
  delay(stepInterval);

  moveServo(FR0, -50);
  moveServo(RL0, -50);
  delay(stepInterval);

  moveServo(FR1, 45);
  delay(stepInterval);

  moveServo(FL1, 45);
  moveServo(RR1, 90);
  delay(stepInterval);

  moveServo(RL1, 90);
  delay(stepInterval);

  moveServo(FR0, -70);
  moveServo(RL0, -70);
  delay(stepInterval);
}

/************************************
 * Spin
 ************************************/
// Spin Left
void spinLeftInit(){
  servoLegFront(45);
  servoLegRear(45);
  servoKneesAll(70);
  delay(stepIntervalTurn);
}

void stepSpinLeft(){
  moveServo(FR0, -50);
  moveServo(RL0, -50);
  moveServo(FR1, 90);
  moveServo(RL1, 90);
  moveServo(FL1, 90);
  moveServo(RR1, 90);
  delay(stepIntervalTurn);

  moveServo(FR0, -70);
  moveServo(RL0, -70);
  delay(stepIntervalTurn);

  moveServo(FL0, -50);
  moveServo(RR0, -50);
  moveServo(FL1, 45);
  moveServo(RR1, 45);
  moveServo(FR1, 45);
  moveServo(RL1, 45);
  delay(stepIntervalTurn);

  moveServo(FL0, -70);
  moveServo(RR0, -70);
  delay(stepIntervalTurn);
}

// Spin Right
void spinRightInit(){
  servoLegFront(45);
  servoLegRear(45);
  servoKneesAll(70);
  delay(stepIntervalTurn);
}

void stepSpinRight(){
  moveServo(FL0, -50);
  moveServo(RR0, -50);
  moveServo(FR1, 90);
  moveServo(RL1, 90);
  moveServo(FL1, 90);
  moveServo(RR1, 90);
  delay(stepIntervalTurn);

  moveServo(FL0, -70);
  moveServo(RR0, -70);
  delay(stepIntervalTurn);

  moveServo(FR0, -50);
  moveServo(RL0, -50);
  moveServo(FL1, 45);
  moveServo(RR1, 45);
  moveServo(FR1, 45);
  moveServo(RL1, 45);
  delay(stepIntervalTurn);

  moveServo(FR0, -70);
  moveServo(RL0, -70);
  delay(stepIntervalTurn);
}

/************************************
 * fixed action
 ************************************/
// all knees angle
void servoKneesAll(int ang){
  moveServo(FR0, -ang);
  moveServo(FL0, -ang);
  moveServo(RR0, -ang);
  moveServo(RL0, -ang);
}

// front leg angle
void servoLegFront(int ang){
  moveServo(FR1, ang);
  moveServo(FL1, ang);
}

// Rear leg angle
void servoLegRear(int ang){
  moveServo(RR1, ang);
  moveServo(RL1, ang);
}

// Stand Up
void servoStandUp(int ang){
  int i = 0;
  while(i < ang){
    servoKneesAll(i);
    i = i + stp;
    delay(smooth);
  }
}

// Lay Down
void servoLayDown(int ang){
  // lay down
  int i = ang;
  while(i > 0){
    servoKneesAll(i);
    i = i - stp;
    delay(smooth);
  }
}

// Leg Open
void servoLegOpen(int ang){
  int i = 0;
  while(i < ang){
    servoLegFront(i);
    servoLegRear(i);
    i = i + stp;
    delay(smooth);
  }
}

// Leg Close
void servoLegClose(int ang){
  int i = ang;
  // leg close
  while(i > 0){
    servoLegFront(i);
    servoLegRear(i);
    i = i - stp;
    delay(smooth);
  }
}

// Tilt
void servoTilt(int ang){
  moveServo(TILT, ang);
}

/************************************
 * Servo position by angle
 ************************************/
void moveServo(int n, int deltaAngle){
  if(n == TILT){
     setServoAngle(n, CENT_TILT - deltaAngle);
  }else if(n == FR1){
     setServoAngle(n, CENT_FR1 + deltaAngle);
  }else if(n == FL1){
     setServoAngle(n, CENT_FL1 - deltaAngle);
  }else if(n == RR1){
     setServoAngle(n, CENT_RR1 - deltaAngle);
  }else if(n == RL1){
     setServoAngle(n, CENT_RL1 + deltaAngle);
  }else if(n == FR0){
     setServoAngle(n, CENT_FR0 + deltaAngle);
  }else if(n == FL0){
     setServoAngle(n, CENT_FL0 - deltaAngle);
  }else if(n == RR0){
     setServoAngle(n, CENT_RR0 - deltaAngle);
  }else if(n == RL0){
     setServoAngle(n, CENT_RL0 + deltaAngle);
  }
}

/*************************************************** 
 you can use this function if you'd like to set the pulse length in microsecond
***************************************************/
void setServoPulse(uint8_t n, double pulse_us){
  double pulselength;
  
  pulselength = 1000000;   // 1,000,000 us per second
  pulselength /= 50;   // 50 Hz
//  Serial.print(pulselength); Serial.println(" us per period"); 
  pulselength /= 4096;  // 12 bits of resolution
//  Serial.print(pulselength); Serial.println(" us per bit"); 
  pulse_us /= pulselength;
//  Serial.println(pulse_us);

  pwm.setPWM(n, 0, pulse_us);
}

/*************************************************** 
 you can use this function if you'd like to set the angle in degree
***************************************************/
void setServoAngle(uint8_t n, int ang){
  if (ang < 0)   { ang = 0; }
  if (ang > 180) { ang = 180; }
  double pulse_us = map(ang, 0, 180, minUs, maxUs); //map angle(0～180) to pulse_us
  setServoPulse(n, pulse_us);
}

/*************************************************** 
 status of charging
 **************************************************/
void adjMode(){
    vTaskSuspend(avatarTaskHandle);

    delay(300);
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(WHITE, BLACK);
    M5.Display.setCursor(50, 0);
    M5.Display.print("HOME POSITION");
    M5.Display.setCursor(50, 50);
    M5.Display.printf("NES mode: %d\n", modeNES);
    servoHome();

    #ifdef DEBUG
      Serial.println("Charging...");
    #endif

    while(1){
      M5.update();

      if(M5.BtnB.wasPressed()){
        M5.Display.fillScreen(SECONDARY_COLOR);
        vTaskResume(avatarTaskHandle);
        avatar->init();
        break;
      }
      delay(100);
   }
}
