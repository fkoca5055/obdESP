//Build date 27 October 2019

/**************************************************************************
ST7735      --->     ESP8266
VCC         --->     5V
GND         --->     GND
CS          --->     D2
RESET       --->     D0
A0(DC)      --->     D1
MOSI(SDA)   --->     D7
SCK(CLK)    --->     D5
LED         --->     3.3V

BUTTON      --->     D6
NEOPIXEL    --->     D3 (IF PROBLEM OCCUR WITH D3, TRY D4)

 **************************************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>


  #define TFT_CS        5   //D1
  #define TFT_RST       4   //D2
  #define TFT_DC        2   //D4
  #define CYAN     0x07FF
  #define MAGENTA  0xF81F
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

#include <Adafruit_NeoPixel.h>
#define PIN            12
#define NUMPIXELS      16
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#include "images.h"

char rxData[128];
char rxIndex=0;
int rpm=0, LDR,LDRMAX,LDRMIN, EGT, turboRAW;
byte INTEMP, CACT, SPEED, COOLANT;
float BATTERY, turboPRESS, turboMAX;
unsigned long time_now = 0;
unsigned long period = 300; //300ms
unsigned long time_now2 = 0;
unsigned long period2 = 10000; //10sn
unsigned long time_now3 = 0;
unsigned long period3 = 30000; //30sn
unsigned long delayTime = 0; //50 çalışıyor


void setup() 
{
   Serial.flush();
   pixels.begin(); // This initializes the NeoPixel library.
   pixels.setBrightness(1);
   pixels.show();   
   tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab
   tft.setRotation(3);
   tft.fillScreen(ST77XX_WHITE);
   
   int h = 127,w = 127, row, col, buffidx=0;
  for (row=0; row<h; row++) { // For each scanline...
    for (col=0; col<w; col++) { // For each pixel...
      //To read from Flash Memory, pgm_read_XXX is required.
      //Since image is stored as uint16_t, pgm_read_word is used as it uses 16bit address
      tft.drawPixel(col, row, pgm_read_word(vw_logo + buffidx));
      buffidx++;
    } // end pixel
  }
   //delay(2000);
   Serial.flush();
   pinMode(A0, INPUT);
   ODB_init();
   tft.fillScreen(ST77XX_BLACK);
   
}

void loop() 
{
       /*reading = digitalRead(buttonPin);
        tft.setCursor(58, 60);
        tft.setTextColor(ST77XX_RED,ST77XX_BLACK);
        tft.setTextSize(2);
        tft.println(reading);  */  
   

      getCACT();
      getEGT();
      getTURBOPRESS();
      //SPEED = getSPEED();
      
      
      
     tft.drawLine(105, 0, 105, 128, MAGENTA);
     tft.drawLine(54, 0, 54, 128, MAGENTA);
     tft.drawLine(0, 43, 160, 43, MAGENTA);
     tft.drawLine(0, 86, 105, 86, MAGENTA);
     
     if(millis() > time_now2 + period2){
        getINTEMP();
        getCOOLANT();
        getBATT();
        time_now2 = millis();
        LDR = analogRead(A0);   // read the input on analog pin 0 LDR
        LDR = map(LDR, 1, 1023, 1, 50);
        pixels.setBrightness(LDR);
       }

      if(millis() > time_now3 + period3){
        if (COOLANT<=0){
            ODB_init();
          }
        time_now3 = millis();
        }
       
   rpmLED();
      
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
  Serial.flush();  
}


int getRPM(void)
{
  Serial.flush();
  Serial.print("010C\r");
  delay(delayTime);
  OBD_read();

  return ((strtol(&rxData[6],0,16)*256)+strtol(&rxData[9],0,16))/4;
  
}

void getCOOLANT()
{
  Serial.flush();
  Serial.print("0105\r");
  delay(delayTime);
  OBD_read();
  COOLANT = strtol(&rxData[6],0,16)-40;
       if (COOLANT<50){
       tft.setTextColor(ST77XX_BLUE,ST77XX_BLACK);
       }
       else{
       tft.setTextColor(ST77XX_GREEN,ST77XX_BLACK);
       }
       tft.setCursor(4, 0);       
       tft.setTextSize(1);
       tft.println("COOLANT");
       tft.setTextSize(2);
       tft.setCursor(10, 15);
       tft.println(COOLANT);
       //tft.setCursor(30, 17);
       //tft.println("C");
}

void getINTEMP()
{
  Serial.flush();
  Serial.print("010F\r");
  delay(delayTime);
  OBD_read();
  INTEMP = strtol(&rxData[6],0,16)-40;
       tft.setCursor(120, 0);
       tft.setTextColor(ST77XX_GREEN,ST77XX_BLACK);
       tft.setTextSize(1);
       tft.println("INTAKE");
       tft.setTextSize(2);
       tft.setCursor(125, 15);
       tft.println(INTEMP);

}

void getCACT()
{
  Serial.flush();
  Serial.print("0177\r");
  delay(delayTime);
  OBD_read();
  CACT = strtol(&rxData[9],0,16)-40;
       tft.setCursor(65, 50);
       tft.setTextColor(ST77XX_GREEN,ST77XX_BLACK);
       tft.setTextSize(1);
       tft.println("CACT");
       tft.setTextSize(2);
       tft.setCursor(60, 65);
       tft.println(CACT);
       //tft.println(CACT)+tft.print((char)247)+tft.print("C");
       
       
       //tft.setCursor(30, 17);
       //tft.println("C");

}

void getBATT(){
  

  Serial.flush();
  Serial.print("0142\r");
  delay(delayTime);
  OBD_read();
  BATTERY = ((strtol(&rxData[6],0,16)*256)+strtol(&rxData[9],0,16));  //((A*256)+B)/1000
  BATTERY = BATTERY / 1000;
       if (BATTERY<12){
       tft.setTextColor(ST77XX_BLACK,ST77XX_YELLOW);
       }
       else {
       tft.setTextColor(ST77XX_GREEN,ST77XX_BLACK);
       }
       tft.setCursor(5, 50);
       tft.setTextSize(1);
       tft.println("BATTERY");
       tft.setTextSize(2);
       tft.setCursor(0, 65);
       tft.println(BATTERY,1);
       //tft.setCursor(70, 17);
       //tft.println("C");
  

}
void getEGT()
{
  Serial.flush();
  Serial.print("0178\r");
  delay(100);
  OBD_read();
  EGT = (((strtol(&rxData[30],0,16)*256)+strtol(&rxData[33],0,16))/10)-40;
       if (EGT<350){
       tft.setTextColor(ST77XX_GREEN,ST77XX_BLACK);
       }
       else if((EGT>350)&&(EGT<450)){
       tft.setTextColor(ST77XX_BLACK,ST77XX_YELLOW);
       }
       else {
       tft.setTextColor(ST77XX_BLACK,ST77XX_RED);
       }
       tft.setCursor(70, 0);
       tft.setTextSize(1);
       tft.println("EGT");
       tft.setTextSize(2);
       tft.setCursor(60, 15);
       tft.println(EGT);
       //tft.setCursor(70, 17);
       //tft.println("C");

}

int getSPEED(void)
{
  Serial.flush();
  Serial.print("010D\r");
  delay(delayTime);
  OBD_read();
  return strtol(&rxData[6],0,16);
  
}

int getINTAKEPRESS(void)
{
  Serial.flush();
  Serial.print("010B\r");
  delay(delayTime);
  OBD_read();
  return strtol(&rxData[6],0,16);
}

int getBAROPRESS(void)
{
  Serial.flush();
  Serial.print("0133\r");
  delay(delayTime);
  OBD_read();
  return strtol(&rxData[6],0,16);
}

void getTURBOPRESS(){
  turboRAW = (getINTAKEPRESS()-getBAROPRESS());
  
  turboPRESS = turboRAW;
  turboPRESS = (turboPRESS * 0.01);
       if (turboRAW<139){
       tft.setTextColor(ST77XX_GREEN,ST77XX_BLACK);
       }
       else{
       tft.setTextColor(ST77XX_RED,ST77XX_BLACK);
       }
       tft.setCursor(120, 50);
       tft.setTextSize(1);
       tft.println("BOOST");
       tft.setTextSize(2);
       tft.setCursor(110, 65);
       tft.println(turboPRESS,2);
  if (turboPRESS>turboMAX) {
    (turboMAX=turboPRESS);
   }
       tft.setCursor(120, 90);
       tft.setTextSize(1);
       tft.println("PEAK");
       tft.setTextSize(2);
       tft.setCursor(110, 105);
       tft.println(turboMAX,2);
 
}



void OBD_read(void)
{
  char c;
  do{
    if(Serial.available() > 0)
    {
      c = Serial.read();
      //lcd.print(c);
      if((c!= '>') && (c!='\r') && (c!='\n')) //Keep these out of our buffer
      {
        rxData[rxIndex++] = c; //Add whatever we receive to the buffer
      }  
     }     
  }while(c != '>'); //The ELM327 ends its response with this char so when we get it we exit out.
  rxData[rxIndex++] = '\0';//Converts the array into a string
  rxIndex=0; //Set this to 0 so next time we call the read we get a "clean buffer"
}

void rpmLED(){
  
//////////////////NEO STICK DESIGN///////////////////////////////////////////////////
//7-6-5-4-3-2-1-0              //15-14-13-12-11-10-9-8

     pixels.begin();
     rpm = getRPM();
if ((rpm > 1000) && (rpm<2600))
  {
    pixels.setPixelColor(7, pixels.Color(0,150,0));
    pixels.setPixelColor(8, pixels.Color(0,150,0));
  }
  else{
    pixels.setPixelColor(7, pixels.Color(0,0,0));
    pixels.setPixelColor(8, pixels.Color(0,0,0));
  }
  if ((rpm > 1400) && (rpm<2600))
  {
    pixels.setPixelColor(6, pixels.Color(0,150,0));
    pixels.setPixelColor(9, pixels.Color(0,150,0));
  }
  else{
    pixels.setPixelColor(6, pixels.Color(0,0,0));
    pixels.setPixelColor(9, pixels.Color(0,0,0));
  }
  if ((rpm > 1600) && (rpm<2600))
  {
    pixels.setPixelColor(5, pixels.Color(0,150,0));
    pixels.setPixelColor(10, pixels.Color(0,150,0));
  }
  else{
    pixels.setPixelColor(5, pixels.Color(0,0,0));
    pixels.setPixelColor(10, pixels.Color(0,0,0));
  }
  if ((rpm > 1800) && (rpm<2600))
  {
    pixels.setPixelColor(4, pixels.Color(0,150,0));
    pixels.setPixelColor(11, pixels.Color(0,150,0));
  }
  else{
    pixels.setPixelColor(4, pixels.Color(0,0,0));
    pixels.setPixelColor(11, pixels.Color(0,0,0));
  }
  if ((rpm > 2100) && (rpm<2600))  //5.led yeşil yanacak 
  {
    pixels.setPixelColor(3, pixels.Color(0,150,0));
    pixels.setPixelColor(12, pixels.Color(0,150,0));
  }
  else{
    pixels.setPixelColor(3, pixels.Color(0,0,0));
    pixels.setPixelColor(12, pixels.Color(0,0,0));
  }
  if ((rpm > 2300) && (rpm<2600))
  {
    pixels.setPixelColor(2, pixels.Color(150,150,0));
    pixels.setPixelColor(13, pixels.Color(150,150,0));
  }
  else{
    pixels.setPixelColor(2, pixels.Color(0,0,0));
    pixels.setPixelColor(13, pixels.Color(0,0,0));
  }
  if ((rpm > 2400) && (rpm<2600))
  {
    pixels.setPixelColor(1, pixels.Color(150,150,0));
    pixels.setPixelColor(14, pixels.Color(150,150,0));
  }
  else{
    pixels.setPixelColor(1, pixels.Color(0,0,0));
    pixels.setPixelColor(14, pixels.Color(0,0,0));
  }
  if ((rpm > 2500) && (rpm<2600))
  {
    pixels.setPixelColor(0, pixels.Color(150,0,0));
    pixels.setPixelColor(15, pixels.Color(150,0,0));
  }
  else
  {
    pixels.setPixelColor(0, pixels.Color(0,0,0));
    pixels.setPixelColor(15, pixels.Color(0,0,0));
  }
  
  if (rpm > 2600) //2600 RPM
  {
  blue_flash();  
  }
  
  pixels.show();
}

void blue_flash(){
 if(millis() > time_now + period){
        time_now = millis();
        for(int i=0;i<NUMPIXELS;i++)
       {    
          pixels.setPixelColor(i, pixels.Color(0,0,150));         
       }
       pixels.show();
       delay(100);
        for(int i=0;i<NUMPIXELS;i++)
       {    
          pixels.setPixelColor(i, pixels.Color(0,0,0));         
       }
        pixels.show();
       delay(100); 
    } 
}
