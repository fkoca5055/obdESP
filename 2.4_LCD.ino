//Build date 26 November 2019

/**************************************************************************
  ILI9341     --->     ESP8266
  VCC         --->     5V
  GND         --->     GND
  CS          --->     D2 try D8
  RESET       --->     D0
  A0(DC)      --->     D1
  MOSI(SDA)   --->     D7
  SCK(CLK)    --->     D5
  LED         --->     3.3V

  BRIGHTNESS  --->     D6
  NEOPIXEL    --->     D3 (IF PROBLEM OCCUR WITH D3, TRY D4)

 **************************************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include "Adafruit_ILI9341.h"
#include <SPI.h>

#define TFT_CS        D2 //D8
#define TFT_RST       D0
#define TFT_DC        D1
#define CYAN     0x07FF
#define MAGENTA  0xF81F
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

#define LED D6
#include "images.h"

char rxData[128];
char rxIndex = 0;
int rpm, LDR, LDRMAX, LDRMIN, EGT, turboRAW, reading, COOLANT, x;
byte INTEMP, CACT, SPEED;
float BATTERY, turboPRESS, turboMAX;
unsigned long time_now = 0;
unsigned long period = 300; //300ms
unsigned long time_now2 = 0;
unsigned long period2 = 10000; //10sn
unsigned long time_now3 = 0;
unsigned long period3 = 31000; //30sn
unsigned long time_now4 = 0;
unsigned long period4 = 2000; //2sn
unsigned long time_now5 = 0;
unsigned long period5 = 13000; //120000 2dk
unsigned long delayTime = 0; //50 çalışıyor
//const int buttonPin = D6;

char iBuf[5] = {};
char eBuf[5] = {};
char cBuf[5] = {};
char oBuf[5] = {};
char ldrBuf[5] = {};

void setup() {
  //Serial.flush();
  pinMode(A0, INPUT);
  tft.begin();
  tft.setRotation(1);
  pinMode(LED, OUTPUT);
  analogWrite(LED, 0);
  tft.fillScreen(ILI9341_WHITE);
  int h = 230, w = 275, row, col, buffidx = 0;
  for (row = 0; row < h; row++) {
    for (col = 35; col < w; col++) {
      tft.drawPixel(col, row, pgm_read_word(vw_logo + buffidx));
      buffidx++;
    }
  }

  tft.setCursor(250, 230);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
  tft.print("2.4_ab_LCD");

  for (int i = 1; i < 1025; i++) {
    analogWrite(LED, i);
    delay(1);
  }

  //Serial.flush();
  ODB_init();
  tft.fillScreen(ILI9341_BLACK);
  tft.drawLine(106, 0, 106, 165, MAGENTA); //DİKEY 1
  tft.drawLine(212, 0, 212, 165, MAGENTA); //DİKEY 2
  tft.drawLine(0, 55, 320, 55, MAGENTA);   //YATAY 1
  tft.drawLine(0, 110, 320, 110, MAGENTA); //YATAY 2
  tft.drawLine(0, 165, 320, 165, MAGENTA); //YATAY 3
  
  getBATT();
}

void loop()
{
  /*reading = digitalRead(buttonPin);
    tft.setCursor(55, 60);
    tft.setTextColor(ILI9341_RED,ILI9341_BLACK);
    tft.setTextSize(1);
    tft.println(reading); */
  getRPM();
  getCACT();
  getEGT();
  getTURBOPRESS();
  //tft.drawRect(160, 180, 10, 20, ILI9341_GREEN);
  //tft.fillRect(160, 0, 40, 50, ILI9341_GREEN);
  if (millis() > time_now2 + period2) {
    getINTEMP();
    getCOOLANT();
    LDR = analogRead(A0);
    if (LDR > 768) {
      analogWrite(LED, 1024);
      x = 5;
    }
    if ((LDR > 512) && (LDR <= 768)) {
      analogWrite(LED, 768);
      x = 4;
    }
    if ((LDR > 384) && (LDR <= 512)) {
      analogWrite(LED, 512);
      x = 3;
    }
    if ((LDR > 256) && (LDR <= 384)) {
      analogWrite(LED, 246);
      x = 2;
    }
    if ((LDR > 128) && (LDR <= 256)) {
      analogWrite(LED, 123);
      x = 1;
    }
    if (LDR <= 128) {
      analogWrite(LED, 20);
      x = 0;
    }
    tft.setCursor(220, 145);
    tft.setTextColor(ILI9341_MAGENTA, ILI9341_BLACK);
    tft.setTextSize(2);
    tft.print("BL lvl:");
    tft.print(x);
    time_now2 = millis();
  }

  if (millis() > time_now3 + period3) {
    if (COOLANT <= 0) {
      ODB_init();
      tft.fillScreen(ILI9341_BLACK);
      tft.drawLine(106, 0, 106, 165, MAGENTA); //DİKEY 1
      tft.drawLine(212, 0, 212, 165, MAGENTA); //DİKEY 2
      tft.drawLine(0, 55, 320, 55, MAGENTA);   //YATAY 1
      tft.drawLine(0, 110, 320, 110, MAGENTA); //YATAY 2
      tft.drawLine(0, 165, 320, 165, MAGENTA); //YATAY 3
    }
    time_now3 = millis();
  }

  if (millis() > time_now4 + period4) {
    getBATT();
    time_now4 = millis();
  }

  if (millis() > time_now5 + period5) {
    getSMC();
    time_now5 = millis();
  }


}



void ODB_init(void)
{

  Serial.flush();
  Serial.begin(38400);
  delay(0); //1000
  Serial.print("ATZ\r");
  delay(500);  //2000
  OBD_read();
  Serial.print("ATE0\r");
  delay(500); //1000
  OBD_read();
  Serial.print("ATSP6\r");
  delay(500); //1000
  OBD_read();
  Serial.print("ATSH7E0\r");
  delay(700);
  OBD_read();
  //Serial.flush();
}


void getRPM() {
  Serial.flush();
  Serial.print("010C\r");
  delay(delayTime);
  OBD_read();
  rpm = ((strtol(&rxData[6], 0, 16) * 256) + strtol(&rxData[9], 0, 16)) / 4;
  graphRPM();
}

void getCOOLANT()
{
  Serial.flush();
  Serial.print("0105\r");
  delay(delayTime);
  OBD_read();
  COOLANT = strtol(&rxData[6], 0, 16) - 40;
  if (COOLANT < 50) {
    tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
    tft.drawCircle(75, 25, 3, ILI9341_CYAN);
  }
  else {
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    tft.drawCircle(75, 25, 3, ILI9341_GREEN);
  }
  tft.setCursor(4, 0);
  tft.setTextSize(2);
  tft.print("COOLANT");
  tft.setTextSize(3);
  tft.setCursor(10, 25);
  snprintf(oBuf, sizeof(oBuf), "%3d", COOLANT);
  tft.println(oBuf);
  tft.setCursor(80, 25);
  tft.println("c");

}

void getINTEMP()
{
  Serial.flush();
  Serial.print("010F\r");
  delay(delayTime);
  OBD_read();
  INTEMP = strtol(&rxData[6], 0, 16) - 40;
  tft.setCursor(230, 0);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.print("INTAKE");
  tft.setTextSize(3);
  tft.setCursor(225, 25);
  snprintf(iBuf, sizeof(iBuf), "%3d", INTEMP);
  tft.println(iBuf);
  tft.setCursor(290, 25);
  tft.println("c");
  tft.drawCircle(285, 25, 3, ILI9341_GREEN);
}

void getCACT()
{
  Serial.flush();
  Serial.print("0177\r");
  delay(delayTime);
  OBD_read();
  CACT = strtol(&rxData[9], 0, 16) - 40;
  tft.setCursor(140, 60);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.print("CACT");
  tft.setTextSize(3);
  tft.setCursor(125, 85);
  snprintf(cBuf, sizeof(cBuf), "%3d", CACT);
  tft.println(cBuf);
  //tft.println(CACT)+tft.print((char)247)+tft.print("C");
  tft.setCursor(190, 85);
  tft.println("c");
  tft.drawCircle(185, 85, 3, ILI9341_GREEN);
}

void getBATT() {


  Serial.flush();
  Serial.print("0142\r");
  delay(delayTime);
  OBD_read();
  BATTERY = ((strtol(&rxData[6], 0, 16) * 256) + strtol(&rxData[9], 0, 16)); //((A*256)+B)/1000
  BATTERY = BATTERY / 1000;
  if (BATTERY < 12) {
    tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
  }
  else {
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  }
  tft.setCursor(5, 60);
  tft.setTextSize(2);
  tft.print("BATTERY");
  tft.setTextSize(3);
  tft.setCursor(0, 85);
  tft.print(BATTERY, 1);
  tft.setCursor(80, 85);
  tft.println("v");
}

//SOOT MASS CALC
void getSMC() {

  Serial.flush();
  Serial.print("0100\r");
  delay(200);
  OBD_read();
  Serial.print("22114F\r");
  delay(200);
  OBD_read();
  
  int SMC = ((strtol(&rxData[9], 0, 16) * 256) + strtol(&rxData[12], 0, 16)); //((A*256)+B)/100

  if ((SMC > 200) && (SMC < 2500)) {
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setCursor(30, 115);
    tft.print("SMC");
    tft.setTextSize(3);
    tft.setCursor(10, 140);
    float SMCF = SMC;
    SMCF = SMCF / 100;
    tft.print(SMCF, 1);

    int DPFL = map(SMC, 500, 2350, 0, 100);
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setCursor(140, 115);
    tft.print("DPFL");
    tft.setTextSize(3);
    tft.setCursor(130, 140); //125
    tft.print(DPFL);
    tft.print("%");
  }
}

void getEGT()
{
  Serial.flush();
  Serial.print("0178\r");
  delay(100);
  OBD_read();
  EGT = (((strtol(&rxData[30], 0, 16) * 256) + strtol(&rxData[33], 0, 16)) / 10) - 40;
  if (EGT < 350) {
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    tft.drawCircle(185, 25, 3, ILI9341_GREEN);
    tft.fillCircle(190, 7, 7, ILI9341_BLACK);
  }
  else if ((EGT >= 350) && (EGT < 450)) {
    tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
    tft.drawCircle(185, 25, 3, ILI9341_YELLOW);
    tft.fillCircle(190, 7, 7, ILI9341_YELLOW);
  }
  else if (EGT >= 450) {
    tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
    tft.drawCircle(185, 25, 3, ILI9341_RED);
    tft.fillCircle(190, 7, 7, ILI9341_RED);
  }
  tft.setCursor(140, 0);
  tft.setTextSize(2);
  tft.print("EGT");
  tft.setTextSize(3);
  tft.setCursor(120, 25);
  snprintf(eBuf, sizeof(eBuf), "%3d", EGT);
  tft.println(eBuf);
  tft.setCursor(190, 25);
  tft.println("c");

}

int getSPEED(void)
{
  Serial.flush();
  Serial.print("010D\r");
  delay(delayTime);
  OBD_read();
  return strtol(&rxData[6], 0, 16);

}

int getINTAKEPRESS(void)
{
  Serial.flush();
  Serial.print("010B\r");
  delay(delayTime);
  OBD_read();
  return strtol(&rxData[6], 0, 16);
}

int getBAROPRESS(void)
{
  Serial.flush();
  Serial.print("0133\r");
  delay(delayTime);
  OBD_read();
  return strtol(&rxData[6], 0, 16);
}

void getTURBOPRESS() {
  turboRAW = (getINTAKEPRESS() - getBAROPRESS());

  turboPRESS = turboRAW;
  turboPRESS = (turboPRESS * 0.01);
  graphTURBO();
  if (turboRAW < 139) {
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  }
  else {
    tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
  }
  tft.setCursor(230, 60);
  tft.setTextSize(2);
  tft.print("BOOST");

  if (turboRAW <= 0) {
    tft.setTextSize(2);
  }
  else {
    tft.setTextSize(3);
  }

  tft.setCursor(220, 85);
  tft.print(turboPRESS, 2);
  tft.setTextSize(1);
  tft.setCursor(300, 100);
  tft.print("bar");

  if (turboPRESS > turboMAX) {
    (turboMAX = turboPRESS);
  }
  tft.setCursor(220, 115);
  tft.setTextSize(2);
  tft.print("Max:");
  tft.print(turboMAX, 2);

}

void graphTURBO() {
  if (turboRAW > 9) {
    tft.fillCircle(20, 220, 10, ILI9341_GREEN);
  }
  else {
    tft.fillCircle(20, 220, 10, ILI9341_BLACK);
  }
  if (turboRAW > 19) {
    tft.fillCircle(50, 220, 10, ILI9341_GREEN);
  }
  else {
    tft.fillCircle(50, 220, 10, ILI9341_BLACK);
  }
  if (turboRAW > 39) {
    tft.fillCircle(80, 220, 10, ILI9341_GREEN);
  }
  else {
    tft.fillCircle(80, 220, 10, ILI9341_BLACK);
  }
  if (turboRAW > 59) {
    tft.fillCircle(110, 220, 10, ILI9341_GREEN);
  }
  else {
    tft.fillCircle(110, 220, 10, ILI9341_BLACK);
  }
  if (turboRAW > 79) {
    tft.fillCircle(140, 220, 10, ILI9341_GREEN);
  }
  else {
    tft.fillCircle(140, 220, 10, ILI9341_BLACK);
  }
  if (turboRAW > 99) {
    tft.fillCircle(170, 220, 10, ILI9341_GREEN);
  }
  else {
    tft.fillCircle(170, 220, 10, ILI9341_BLACK);
  }
  if (turboRAW > 119) {
    tft.fillCircle(200, 220, 10, ILI9341_RED);
  }
  else {
    tft.fillCircle(200, 220, 10, ILI9341_BLACK);
  }
  if (turboRAW > 139) {
    tft.fillCircle(230, 220, 10, ILI9341_RED);
  }
  else {
    tft.fillCircle(230, 220, 10, ILI9341_BLACK);
  }
  tft.setCursor(255, 210);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.print("turbo");
}

void graphRPM() {
  if ((rpm > 1000) && (rpm < 2600)) {
    tft.fillRect(10, 170, 20, 25, ILI9341_GREEN);
  }
  else {
    tft.fillRect(10, 170, 20, 25, ILI9341_BLACK);
  }
  if ((rpm > 1400) && (rpm < 2600)) {
    tft.fillRect(40, 170, 20, 25, ILI9341_GREEN);
  }
  else {
    tft.fillRect(40, 170, 20, 25, ILI9341_BLACK);
  }
  if ((rpm > 1600) && (rpm < 2600)) {
    tft.fillRect(70, 170, 20, 25, ILI9341_GREEN);
  }
  else {
    tft.fillRect(70, 170, 20, 25, ILI9341_BLACK);
  }
  if ((rpm > 1800) && (rpm < 2600)) {
    tft.fillRect(100, 170, 20, 25, ILI9341_GREEN);
  }
  else {
    tft.fillRect(100, 170, 20, 25, ILI9341_BLACK);
  }
  if ((rpm > 2100) && (rpm < 2600)) {
    tft.fillRect(130, 170, 20, 25, ILI9341_GREEN);
  }
  else {
    tft.fillRect(130, 170, 20, 25, ILI9341_BLACK);
  }
  if ((rpm > 2300) && (rpm < 2600)) {
    tft.fillRect(160, 170, 20, 25, ILI9341_YELLOW);
  }
  else {
    tft.fillRect(160, 170, 20, 25, ILI9341_BLACK);
  }
  if ((rpm > 2400) && (rpm < 2600)) {
    tft.fillRect(190, 170, 20, 25, ILI9341_YELLOW);
  }
  else {
    tft.fillRect(190, 170, 20, 25, ILI9341_BLACK);
  }
  if ((rpm > 2500) && (rpm < 2600)) {
    tft.fillRect(220, 170, 20, 25, ILI9341_RED);

  }
  else {
    tft.fillRect(220, 170, 20, 25, ILI9341_BLACK);
  }
  tft.setCursor(255, 170);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.print("rpm");

  if (rpm >= 2600) {        //2600 RPM
    blue_flash();
  }
}

void OBD_read(void)
{
  char c;
  do {
    if (Serial.available() > 0)
    {
      c = Serial.read();
      //lcd.print(c);
      if ((c != '>') && (c != '\r') && (c != '\n')) //Keep these out of our buffer
      {
        rxData[rxIndex++] = c; //Add whatever we receive to the buffer
      }
    }
  } while (c != '>'); //The ELM327 ends its response with this char so when we get it we exit out.
  rxData[rxIndex++] = '\0';//Converts the array into a string
  rxIndex = 0; //Set this to 0 so next time we call the read we get a "clean buffer"
}


void blue_flash() {
  if (millis() > time_now + period) {
    time_now = millis();
    tft.fillRect(10, 170, 20, 25, ILI9341_BLUE);
    tft.fillRect(40, 170, 20, 25, ILI9341_BLUE);
    tft.fillRect(70, 170, 20, 25, ILI9341_BLUE);
    tft.fillRect(100, 170, 20, 25, ILI9341_BLUE);
    tft.fillRect(130, 170, 20, 25, ILI9341_BLUE);
    tft.fillRect(160, 170, 20, 25, ILI9341_BLUE);
    tft.fillRect(190, 170, 20, 25, ILI9341_BLUE);
    tft.fillRect(210, 170, 20, 25, ILI9341_BLUE);
    delay(100);
    tft.fillRect(10, 170, 20, 25, ILI9341_BLACK);
    tft.fillRect(40, 170, 20, 25, ILI9341_BLACK);
    tft.fillRect(70, 170, 20, 25, ILI9341_BLACK);
    tft.fillRect(100, 170, 20, 25, ILI9341_BLACK);
    tft.fillRect(130, 170, 20, 25, ILI9341_BLACK);
    tft.fillRect(160, 170, 20, 25, ILI9341_BLACK);
    tft.fillRect(190, 170, 20, 25, ILI9341_BLACK);
    tft.fillRect(210, 170, 20, 25, ILI9341_BLACK);
    delay(100);
  }
}
