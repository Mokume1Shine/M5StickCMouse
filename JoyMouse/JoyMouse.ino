#include <BleConnectionStatus.h>
#include <BleMouse.h>
#include <M5StickC.h>
#include <Wire.h>

#define JOY_ADDR 0x38

BleMouse bleMouse;
//BleMouse.moveの引数がsingned charのため
signed char mouse_x;
signed char mouse_y;
int mouse_senseX=15,mouse_senseY=30; //マウス感度
float mouse_min=400;  //水平判定しきい値
float accX,accY,accZ;
int8_t joyX,joyY,joyB;

void setup() {
  M5.begin();
  M5.Lcd.setRotation(0);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  M5.MPU6886.Init();
  bleMouse.begin();
  while(!bleMouse.isConnected()){
    delay(100);
  }
//  setCpuFrequencyMhz(10);
//  M5.Axp.ScreenBreath(10);
}

void loop() {
  M5.MPU6886.getAccelData(&accX,&accY,&accZ);
  M5.update();
  mouse_x=0;
  mouse_y=0;
  if(abs(accX*1000)>mouse_min){
    mouse_x=-accX*mouse_senseX;
  }
  if(abs(accY*1000)>mouse_min){
    mouse_y=accY*mouse_senseY;
  }
  
//  Wire.beginTransmission(JOY_ADDR);
//  Wire.write(0x02);
//  Wire.endTransmission();
//  Wire.requestFrom(JOY_ADDR,3);
//  if(Wire.available()){
//    joyX=Wire.read();
//    joyY=Wire.read();
//    joyB=Wire.read();
//  }
//  mouse_x+=map(joyX,-110,110,-10,10);
//  mouse_y+=map(joyY,-110,110,-10,10);
  bleMouse.move(mouse_x,mouse_y);
  delay(33);
}
