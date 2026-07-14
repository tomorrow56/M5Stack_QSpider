#include <M5Unified.h>

#include "wifi_setup.h"
#include <Adafruit_PWMServoDriver.h>

#include "avatar.h"

//#define usesoftAP

#ifndef usesoftAP
  #define useOTA
#endif

#define DEBUG

/********************************
* for Wifi Connection
*********************************/
#include "WebPage.h"
char OTAHostPrefix[]= "ESP32OTA-";
char WiFiAPPrefix[] = "ESP32-";

char WiFiAPname[256];
char OTAHostname[256];

const char passwd[] = "esp32";

WiFiServer server(80);
IPAddress ipadr;

String header;

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

#define CENT_FR0 (86 + OFST)   // knees UP ++
#define CENT_FL0 (77 - OFST)   // knees UP --
#define CENT_RR0 (90 - OFST)   // knees UP --
#define CENT_RL0 (97 + OFST)   // knees UP ++

#define CENT_FR1 80   // leg forward ++
#define CENT_FL1 95   // leg forward --
#define CENT_RR1 95   // leg backword ++
#define CENT_RL1 82   // leg backword --

#define CENT_TILT 105

const int stp = 3;
const int smooth = 5;

const int stepInterval = 50;
const int stepIntervalTurn = 100;

//String Face = "/GALLERY/KumaMCN.jpg";

Avatar *avatar;
uint32_t count = 0;
uint32_t lastAvatarMs = 0;

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  // Mute speaker
  M5.Speaker.setVolume(0);
  M5.Speaker.end();

  Serial.println("M5Stack Q-Spider Webserver example");

/*
 * Servo setup
 */
  pwm.begin();
  pwm.setPWMFreq(50);  // Analog servos run at 50 Hz
  delay(10);

  // Wifi and OTA setup
  uint64_t Chipid = GetChipid();
  sprintf(WiFiAPname, "%s%04X", WiFiAPPrefix, (uint16_t)Chipid);

  M5.Display.fillScreen(BLACK);
  M5.Display.setTextColor(WHITE, BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(0, 0);
  M5.Display.println((String)WiFiAPname);
  M5.Display.println("192.168.4.1");
  #ifdef usesoftAP
    M5.Display.setTextColor(CYAN, BLACK);
    M5.Display.println("softAP mode");
    WiFi.softAP(WiFiAPname, passwd);
    delay(100);
    IPAddress ip(192,168,4,1);
    IPAddress subnet(255,255,255,0);
    WiFi.softAPConfig(ip, ip, subnet);
    Serial.print("AP name: ");
    Serial.println(WiFiAPname);
  #else
    WiFiMgrSetup(WiFiAPname);
    ipadr = WiFi.localIP();
    M5.Display.setTextColor(GREEN, BLACK);
    M5.Display.println("connected(^^)");
    M5.Display.println((String)WiFi.SSID());
    M5.Display.printf("%3d.%3d.%3d.%3d", ipadr[0], ipadr[1], ipadr[2], ipadr[3]);
  #endif

  delay(1000);

  #ifdef useOTA
    sprintf(OTAHostname, "%s%04X", OTAHostPrefix, (uint16_t)Chipid);
    OTASetup(OTAHostname);
  #endif

  Serial.print("IP address: ");
  #ifdef usesoftAP
    Serial.println(ip);
  #else
    Serial.println(WiFi.localIP());
  #endif

  Serial.println("Ready");

  server.begin();

  avatar = new Avatar();
  M5.Display.fillScreen(SECONDARY_COLOR);
  avatar->init();

/*
 * Demo action
 */
//  servoDemo();
  servoHome();
}

void loop(){
  M5.update();

  uint32_t now = millis();
  if(now - lastAvatarMs >= 200){
    lastAvatarMs = now;
    avatarUpdate();
  }

  #ifdef DEBUG
    if(M5.BtnA.wasPressed()){
      i2c_scan();
    }
  #endif

  if(M5.BtnB.wasPressed()){
    adjMode();
  }

  #ifdef useOTA
    ArduinoOTA.handle();
  #endif

  WiFiClient client = server.available();   // Listen for incoming clients

  int lineNum;

  if (client) {                             // If a new client connects,
    header = "";                            // Clear header at start of each request
    #ifdef DEBUG
      Serial.println("New Client.");          // print a message out in the serial port
    #endif
    String currentLine = "";                // make a String to hold incoming data from the client
    const uint32_t CLIENT_TIMEOUT_MS = 2000;
    uint32_t clientConnectedAt = millis();
    while (client.connected()){             // loop while the client's connected
      if (millis() - clientConnectedAt > CLIENT_TIMEOUT_MS) {
        #ifdef DEBUG
          Serial.println("Client timeout.");
        #endif
        break;
      }
      if (client.available()){              // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        #ifdef DEBUG
          Serial.write(c);                    // print it out the serial monitor
        #endif
        header += c;
        if (c == '\n'){                     // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0){
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Display the web page of WebPage.h
            lineNum = 0;
            do{
              client.println(HTTP_Page[lineNum]);
              if(HTTP_Page[lineNum].indexOf("</html>")>= 0){
                break;
              }
              lineNum++;
            }while(true);

            // Get Motor value
            if(header.indexOf("GET /?MOTOR=STOP")>=0) {
              walkStop();
            }
            if(header.indexOf("GET /?MOTOR=FWD")>=0) {
              walkForward(5);
            }
            if(header.indexOf("GET /?MOTOR=BACK")>=0) {
              walkBackward(5);
            }
            if(header.indexOf("GET /?MOTOR=ACT1")>=0) {
              servoHome();
              delay(500);
              walkStop();
            }
            if(header.indexOf("GET /?MOTOR=ACT2")>=0) {
              servoDemo();
            }
            if(header.indexOf("GET /?MOTOR=LEFT")>=0) {
              walkLeft(5);
            }
            if(header.indexOf("GET /?MOTOR=RIGHT")>=0) {
              walkRight(5);
            }
            if(header.indexOf("GET /?MOTOR=SPIN_L")>=0) {
              turnLeft(6);
            }
            if(header.indexOf("GET /?MOTOR=SPIN_R")>=0) {
              turnRight(6);
            }

            client.println();
            // Break out of the while loop
            break;
          }else{
            currentLine = "";
          }
        }else if(c != '\r') {
          currentLine += c;
        }
      }
    }
    // Clear the header variable
    header = "";
    client.stop();
    #ifdef DEBUG
      Serial.println("Client disconnected.");
      Serial.println("");
    #endif
  }
}

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

  servoTilt(15);
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

// STOP position
void walkStop(){
  servoLegOpen(45);
  delay(500);
  servoStandUp(70);
}

// Walk Forward
void walkForward(int steps){
  servoLegFront(45);
  servoLegRear(45);
  servoKneesAll(70);
  delay(stepInterval);

  // initial position for walk Forward
  moveServo(RL0, -50);
  moveServo(RL1, 0);
  delay(stepInterval);
  moveServo(RL0, -70);

  for(int i = 0; i < steps; i++){
    stepForward();
  }

  servoLegFront(45);
  servoLegRear(45);
  servoKneesAll(70);
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
void walkBackward(int steps){
  servoLegFront(45);
  servoLegRear(45);
  servoKneesAll(70);
  delay(stepInterval);

  // initial position for walk Back
  moveServo(FR0, -50);
  moveServo(FR1, 0);
  delay(stepInterval);
  moveServo(FR0, -70);

  for(int i = 0; i < steps; i++){
    stepBackward();
  }

  servoLegFront(45);
  servoLegRear(45);
  servoKneesAll(70);
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

// Walk Right
void walkRight(int steps){
  servoLegFront(45);
  servoLegRear(45);
  servoKneesAll(70);
  delay(stepInterval);

  // initial position for walk Right
  moveServo(RL0, -50);
  moveServo(RL1, 90);
  delay(stepInterval);
  moveServo(RL0, -70);

  for(int i = 0; i < steps; i++){
    stepRight();
  }

  servoLegFront(45);
  servoLegRear(45);
  servoKneesAll(70);
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

// Walk Left
void walkLeft(int steps){
  servoLegFront(45);
  servoLegRear(45);
  servoKneesAll(70);
  delay(stepInterval);

  // initial position for walk Left
  moveServo(FR0, -50);
  moveServo(FR1, 90);
  delay(stepInterval);
  moveServo(FR0, -70);

  for(int i = 0; i < steps; i++){
    stepLeft();
  }

  servoLegFront(45);
  servoLegRear(45);
  servoKneesAll(70);
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

// Turn Right
void turnRight(int steps){
  servoLegFront(45);
  servoLegRear(45);
  servoKneesAll(70);
  delay(stepIntervalTurn);

  for(int i = 0; i < steps; i++){
    stepTurnRight();
  }

  servoLegFront(45);
  servoLegRear(45);
  servoKneesAll(70);
}

void stepTurnRight(){
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

// Turn Left
void turnLeft(int steps){
  servoLegFront(45);
  servoLegRear(45);
  servoKneesAll(70);
  delay(stepIntervalTurn);

  for(int i = 0; i < steps; i++){
    stepTurnLeft();
  }

  servoLegFront(45);
  servoLegRear(45);
  servoKneesAll(70);
}

void stepTurnLeft(){
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

// all knees angle
void servoKneesAll(int ang){
  int i = (-1 * ang);
  moveServo(FR0, i);
  moveServo(FL0, i);
  moveServo(RR0, i);
  moveServo(RL0, i);
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
  if (ang < 0){
    ang = 0;
  }
  if (ang > 180){
    ang = 180;
  }
  double pulse_us = map(ang, 0, 180, minUs, maxUs); //map angle(0～180) to pulse_us
  setServoPulse(n, pulse_us);
}

/*************************************************** 
 status of charging
 **************************************************/
void adjMode(){
    delay(300);
    M5.Display.fillScreen(BLACK);

    // Title
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(WHITE, BLACK);
    M5.Display.setCursor(50, 0);
    M5.Display.print("HOME POSITION");

    // SSID / IP address
    M5.Display.setTextSize(2);
    M5.Display.setCursor(0, 50);
  #ifdef usesoftAP
    M5.Display.setTextColor(CYAN, BLACK);
    M5.Display.print("AP: ");
    M5.Display.println(WiFiAPname);
    M5.Display.setTextColor(GREEN, BLACK);
    M5.Display.setCursor(0, 80);
    M5.Display.print("IP: 192.168.4.1");
  #else
    M5.Display.setTextColor(CYAN, BLACK);
    M5.Display.print("SSID: ");
    M5.Display.println(WiFi.SSID());
    M5.Display.setTextColor(GREEN, BLACK);
    M5.Display.setCursor(0, 80);
    M5.Display.printf("IP: %d.%d.%d.%d",
      ipadr[0], ipadr[1], ipadr[2], ipadr[3]);
  #endif

    servoHome();

    while(1){
      M5.update();
      if(M5.BtnB.wasPressed()){
        M5.Display.fillScreen(SECONDARY_COLOR);
        avatar->init();
        count = 0;
        break;
      }
      delay(100);
    }
}

void avatarUpdate(){
  if(count == 0){
    //avatar->openMouth(50);
  }

  if (count % 100 == 97){
    avatar->openEye(false);
  }
  if (count % 100 == 0){
    avatar->openEye(true);
    count = random(100);
  }
  count++;
}
