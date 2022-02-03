#include <BleConnectionStatus.h>
#include <BleMouse.h>
#include <M5StickC.h>
#include <Wire.h>
#include "images.h"

#define JOY_ADDR 0x38

TFT_eSprite canvas=TFT_eSprite(&M5.lcd);

BleMouse bleMouse;
//BleMouse.moveの引数がsingned charのため
signed char mouse_x;
signed char mouse_y;
int mouse_senseX=20,mouse_senseY=30; //マウス感度
float mouse_min=400;  //水平判定しきい値（これを超えるとカーソルが動き出す）
float accX,accY,accZ; //加速度センサ（重力の方向）の変数
int8_t joyX,joyY,joyB;//ジョイスティックの入力を受け付ける変数

bool lock=false,pLockButton=false;  //ロックボタンの処理

//起動時1回だけ行われる処理
void setup() {
  M5.begin();
  M5.Lcd.fillScreen(BLACK); //背景を黒く塗りつぶす
  M5.MPU6886.Init();        //加速度センサの初期化
  bleMouse.begin();         //BLEマウスの初期化
  M5.Lcd.setRotation(1);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0,10);
  M5.Lcd.setTextColor(0x1C7F);
  M5.Lcd.println(" Bluetooth");
  M5.Lcd.setTextColor(0xFC40);
  M5.Lcd.println(" Connecting");
  //Bluetoothの受付開始
  while(!bleMouse.isConnected()){
    delay(100);
  }
  M5.Lcd.setRotation(0);

  //ジョイスティックとの接続開始
  Wire.begin(0, 26, 100000);

  //省電力設定開始
  M5.Axp.ScreenBreath(10);
  M5.Lcd.setSwapBytes(true);
  canvas.createSprite(M5.Lcd.width(),M5.Lcd.height());
  canvas.setSwapBytes(false);
}

//以降ここをループ
void loop() {
  //加速度センサー情報取得
  M5.MPU6886.getAccelData(&accX,&accY,&accZ);
  //ジョイスティックからの情報取得開始
  Wire.beginTransmission(JOY_ADDR);
  Wire.write(0x02);
  Wire.endTransmission();
  Wire.requestFrom(JOY_ADDR,3);
  if(Wire.available()){
    joyX=Wire.read();
    joyY=Wire.read();
    joyB=Wire.read();
  }

  //マウスカーソルの移動
  mouse_x=0;
  mouse_y=0;

  //本体の傾きによる移動をロック
  if(!lock){
    if(abs(accX*1000)>mouse_min){
      mouse_x=-accX*mouse_senseX;
    }
    if(abs(accY*1000)>mouse_min){
      mouse_y=accY*mouse_senseY;
    }
  }

  //ジョイスティックによるカーソル移動
  if(abs(joyX)>=5)mouse_x+=map(joyX,-110,110,-10,10);
  if(abs(joyY)>=5)mouse_y+=map(joyY,-110,110,10,-10);
  bleMouse.move(mouse_x,mouse_y);

  M5.update();
  //マウスボタン情報の送信
  //マウスのボタンを押すときと離すとき，それぞれのタイミングで送信する必要がある
  pressB=0;
  releaseB=0;
  if(joyB==0){
    pressB|=MOUSE_LEFT;
  }else{
    releaseB|=MOUSE_LEFT;
  }
  if(M5.BtnA.isPressed()){
    pressB|=MOUSE_RIGHT;
  }else{
    releaseB|=MOUSE_RIGHT;
  }
  if(pressB!=0)bleMouse.press(pressB);
  if(releaseB!=0)bleMouse.release(releaseB);
  //ロックボタンの処理
  if(M5.BtnB.isPressed() && !pLockButton){
    lock=!lock;
  }
  pLockButton=M5.BtnB.isPressed();

  //バッテリ情報の取得
  float vbat=M5.Axp.GetBatVoltage();
  float cbat=M5.Axp.GetBatCurrent();

  //画面表示の処理
  M5.Lcd.startWrite();
  canvas.fillRect(0,0,80,160,BLACK);
  canvas.drawRect(10,10,60,60,WHITE);
  canvas.fillRect(40,40,2,2,WHITE);
  canvas.fillRect(40+mouse_x-1,40+mouse_y-1,4,4,0xFC40);

  if(lock){
    canvas.pushImage(73,76,LockWidth,LockHeight,Lock);
  }else{
    canvas.pushImage(73,76,UnlockWidth,UnlockHeight,Unlock);
  }
  if(bleMouse.isPressed(MOUSE_LEFT)){
    canvas.pushImage(26,80,LeftClickedWidth,LeftClickedHeight,LeftClicked);
  }else{
    canvas.pushImage(26,80,LeftButtonWidth,LeftButtonHeight,LeftButton);
  }
  if(bleMouse.isPressed(MOUSE_RIGHT)){
    canvas.pushImage(42,80,RightClickedWidth,RightClickedHeight,RightClicked);
  }else{
    canvas.pushImage(42,80,RightButtonWidth,RightButtonHeight,RightButton);
  }
  canvas.pushImage(26,106,MouseBottomWidth,MouseBottomHeight,MouseBottom);

  canvas.pushImage(9,138,BatteryWidth,BatteryHeight,Battery);
  if(cbat>=0){
    canvas.pushImage(0,142,PlugLeftWidth,PlugLeftHeight,PlugLeft);
    canvas.pushImage(76,142,PlugRightWidth,PlugRightHeight,PlugRight);
  }

  //バッテリ残量のバー表示
  canvas.fillRect(12,141,(int16_t)constrain(map(vbat*100.0,310,390,0,57),0,57),10,WHITE);

  //画面出力
  canvas.pushSprite(0,0);
  M5.Lcd.endWrite();

  //気持ちだけ処理を止める
  delay(10);
}
