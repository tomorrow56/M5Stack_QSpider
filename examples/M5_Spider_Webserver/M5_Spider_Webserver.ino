#include <M5Stack.h>
#include "M5StackUpdater.h"
#include "ip5306.h"

#include "duke32.h"
#include <WiFiClientSecure.h>

#include <Adafruit_PWMServoDriver.h>

#include "clappyavator.h"

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
WiFiClientSecure client;
IPAddress ipadr;

/*
 * LINE Notify account information
 */
const char* host = "notify-api.line.me";
// M5Stack IP Notify
const char* token = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

String header;

int pos1 = 0;
int pos2 = 0;

/********************************
* for Servo control
*********************************/
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x72);

const uint8_t motorInterval = 30;  //[ms]

// Published values for SG90 servos; 0-180 deg
int minUs = 500;
int maxUs = 2400;

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

#define CENT_FR0 86 + OFST   // knees UP ++
#define CENT_FL0 77 - OFST   // knees UP --
#define CENT_RR0 90 - OFST   // knees UP --
#define CENT_RL0 97 + OFST   // knees UP ++

#define CENT_FR1 80   // leg forward ++
#define CENT_FL1 95   // leg forward --
#define CENT_RR1 95   // leg backword ++
#define CENT_RL1 82   // leg backword --

#define CENT_TILT 105

#define ROTATEIN  40
#define ROTATEOUT  150

int stp = 3;
int smooth = 5;

int stepInterval = 50;
int stepIntervalTurn = 100;

boolean moveFlag = false;

//String Face = "/GALLERY/KumaMCN.jpg";

Avator *avator;
uint32_t count = 0;

void setup() {
  // M5Stack::begin(LCDEnable, SDEnable, SerialEnable, I2CEnable);
  M5.begin(true, true, true, true);
  M5.Power.begin();
  M5.Power.setVinMaxCurrent(CURRENT_100MA);

  M5.Speaker.begin();
  M5.Speaker.mute();
  M5.Speaker.end();

  if(digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }

  Serial.println("M5Stack Q-Spider sample");

/*
 * Servo setup
 */
  pwm.begin();
  pwm.setPWMFreq(60);  // Analog servos run at 60 Hz updates
  delay(10);

  // Wifi and OTA setup
  uint64_t Chipid = GetChipid();
  sprintf(WiFiAPname, "%s%04X", WiFiAPPrefix, (uint16_t)Chipid);

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println((String)WiFiAPname);
  M5.Lcd.println("192.168.4.1");
  #ifdef usesoftAP
    M5.Lcd.setTextColor(CYAN, BLACK);
    M5.Lcd.println("softAP mode");
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
    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.println("connected(^^)");
    M5.Lcd.println((String)WiFi.SSID());
    M5.Lcd.printf("%3d.%3d.%3d.%3d", ipadr[0], ipadr[1], ipadr[2], ipadr[3]);
    String message = "M5Q-Spider Connected!\r\nSSID: " +
                     (String)WiFi.SSID() + "\r\nIP adrs: " +
                     (String)ipadr[0] + "." +
                     (String)ipadr[1] + "."+
                     (String)ipadr[2] + "."+
                     (String)ipadr[3];
    LINE_Notify(message);
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

    if (MDNS.begin("esp32")) {
      Serial.println("MDNS responder started");
    }
  #endif

  Serial.println("Ready");

  server.begin();

  avator = new Avator();
  M5.Lcd.fillScreen(RED);
//  M5.Lcd.drawJpgFile(SD, Face.c_str(), 0, 0, 320, 240);
  avator->init();

/*
 * Demo action
 */
//  servoDemo();
  servoHome();
  if(getChargeEnable()){
    adjMode();
  }
}

void loop(){
  M5.update();

  if(millis() % 200 == 0){
    avatorUpdate();
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
  String htmlCmd = "";

  if (client) {                             // If a new client connects,
    #ifdef DEBUG
      Serial.println("New Client.");          // print a message out in the serial port
    #endif
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()){             // loop while the client's connected
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
              walkFoward(5);
              //delay(motorInterval);
              //walkStop();
            }
            if(header.indexOf("GET /?MOTOR=BACK")>=0) {
              walkBackward(5);
              //delay(motorInterval);
              //walkStop();
            }
            if(header.indexOf("GET /?MOTOR=LEFT")>=0) {
              // ACT1
              servoHome();
              delay(500);
              walkStop();
            }
            if(header.indexOf("GET /?MOTOR=RIGHT")>=0) {
              //ACT2
              servoDemo();
              //motor_turnright();
            }
            if(header.indexOf("GET /?MOTOR=TURN_L")>=0) {
              walkLeft(5);
              //delay(motorInterval);
              //walkStop();
            }
            if(header.indexOf("GET /?MOTOR=TURN_R")>=0) {
              walkRight(5);
              //delay(motorInterval);
              //walkStop();
            }
            if(header.indexOf("GET /?MOTOR=SPIN_L")>=0) {
              turnLeft(6);
              //delay(motorInterval);
              //walkStop();
            }
            if(header.indexOf("GET /?MOTOR=SPIN_R")>=0) {
              turnRight(6);
              //delay(motorInterval);
              //walkStop();
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
void walkFoward(int steps){
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
    stepFoward();
  }

  servoLegFront(45);
  servoLegRear(45);
  servoKneesAll(70);
}

void stepFoward(){
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
  int i = ang;
  moveServo(FR1, i);
  moveServo(FL1, i);
}

// Rear leg angle
void servoLegRear(int ang){
  int i = ang;
  moveServo(RR1, i);
  moveServo(RL1, i);
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
  pulselength /= 60;   // 60 Hz
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
void setServoAngle(uint8_t n, uint8_t ang){
  double pulse_us = map(ang, 0, 180, minUs, maxUs); //map angle(0～180) to pulse_us
  setServoPulse(n, pulse_us);
}

/*************************************************** 
 status of charging
 **************************************************/
void adjMode(){
    delay(300);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setCursor(50, 0);
    M5.Lcd.print("HOME POSITION");
    servoHome();

    while(1){
      M5.update();
      M5.Lcd.setCursor(10, 150);
      if(getChargeEnable()){
        switch(getChargeMode()){
          case 1:
            M5.Lcd.print("Full Charge          ");
            break;
          case 2:
            M5.Lcd.print("Charging(CV mode)    ");
            break;
          case 3:
            M5.Lcd.print("Charging(CC mode)    ");
            break;
          default:
            M5.Lcd.print("Unknown mode         ");
            break;
        }
      }else{
        M5.Lcd.print("                     ");
      }

      uint8_t Level =  M5.Power.getBatteryLevel();
      if(Level > 100){
        Level = 100;
      }else if(Level < 0){
        Level = 0;
      }
      M5.Lcd.progressBar(30, 200, 260, 20, Level);
      M5.Lcd.drawRect(29, 199, 262, 22, WHITE);

      if(M5.BtnB.wasPressed()){
        M5.Lcd.fillScreen(RED);
//        M5.Lcd.drawJpgFile(SD, Face.c_str(), 0, 0, 320, 240);
        avator->init();
        count = 0;
        break;
      }
      delay(100);
    }
}

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

/*
 * Send message to Line Notify
 */
void LINE_Notify(String message){
  if(!client.connect(host, 443)){
    Serial.println("Connection failed!");
    return;
  }else{
    Serial.println("Connected to " + String(host));
    String query = String("message=") + message;
    String request = String("") +
                 "POST /api/notify HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Authorization: Bearer " + token + "\r\n" +
                 "Content-Length: " + String(query.length()) +  "\r\n" + 
                 "Content-Type: application/x-www-form-urlencoded\r\n\r\n" +
                  query + "\r\n";
    client.print(request);
    client.println("Connection: close");
    client.println();

    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("headers received");
        break;
      }
    }
    // if there are incoming bytes available
    // from the server, read them and print them:
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
    }

    client.stop();
    Serial.println("closing connection");
    Serial.println("");
  }
}

void avatorUpdate(){
  if(count == 0){
    avator->openMouth(50);
  }

  if (count % 500 == 497){
    avator->openEye(false);
  }
  if (count % 500 == 0){
    avator->openEye(true);
    count = random(500);
  }
  count++;
}
