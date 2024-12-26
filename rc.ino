#include <Wire.h>
#include "LiquidCrystal_I2C.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

WiFiClient client;

byte customChar1[8] = {
  0b00100,
  0b01110,
  0b11111,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100
};

byte customChar2[8] = {
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b11111,
  0b01110,
  0b00100
};


int readXAxis() {
  digitalWrite(D7, HIGH);
  digitalWrite(D8, LOW);
  delay(10);
  return analogRead(A0);
}

int readYAxis() {
  digitalWrite(D7, LOW);
  digitalWrite(D8, HIGH);
  delay(10);
  return analogRead(A0);
}

int initialX = 0;
int initialY = 0;
int motor1_b = 154;
int motor2_b = 154;
int motor3_b = 154;
String lcdMessage;
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

int stop_time = 0;

String padRight(String content, int width) {
  int len = content.length();
  for (int i = 0; i < (width - len); i++)
    content += " ";
  return content;
}

void sendControlCommand(int motor1, int motor2, int motor3) {
  if ((motor1_b!=motor1 or motor2_b != motor2 or motor3!= motor3_b) ){
    motor1_b=motor1;
    motor2_b=motor2;
    motor3_b=motor3;
    HTTPClient http;
    
    // 使用 GET 請求，將參數附加到 URL 中
    String url = "http://192.168.4.1/control?motor1=" + String(motor1*-1) +
                 "&motor2=" + String(motor2) + "&motor3=" + String(motor3);
    
    http.begin(client, url); // 開始 GET 請求
    int httpCode = http.GET(); // 發送 GET 請求並接收回應狀態碼
    
    if (httpCode > 0) {
      Serial.println("HTTP GET Response code: " + String(httpCode));
    } else {
      Serial.println("HTTP GET Request failed");
    }
    
    http.end(); // 結束 HTTP 請求 
  }
}
 
void setup() {
  Serial.begin(115200);  // 設置序列通訊波特率為 115200

  lcd.begin(16, 2);
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);
  pinMode(A0, INPUT);
  pinMode(D4, INPUT);
  pinMode(D5, INPUT_PULLUP);
  pinMode(D6, INPUT_PULLUP);

  lcd.createChar(1, customChar1); // create a new custom character (index 1)
  lcd.createChar(2, customChar2); // create a new custom character (index 2)

  lcd.setCursor(0, 0);
  lcd.print(padRight(String("Connecting"), 16));
  lcd.setCursor(0, 1);
  lcd.print(padRight(String("To The Car"), 16));
  Serial.println("Serial communication initialized!");
  WiFi.begin("DongRcCar", "12345678");

  while (WiFi.status() != WL_CONNECTED) {
    lcdMessage = "Connecting";
    for (int count = 0; count < 3; count++) {
      if(digitalRead(D4)== LOW){
         break;
      }
      lcdMessage += String(".");
      lcd.setCursor(0, 0);
      lcd.print(padRight(String(lcdMessage), 16));
      delay(500);
    }
    if(digitalRead(D4)== LOW){
         break; 
    }
  }

  lcd.setCursor(0, 0);
  lcd.print(padRight(String("Connected"), 16));
  lcd.setCursor(0, 1);
  lcd.print(padRight(String("Successfully"), 16));
  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print(padRight(String("Calibrating"), 16));
  lcd.setCursor(0, 1);
  lcd.print(padRight(String("Joystick"), 16));
  delay(1000);

  initialX = readXAxis();
  initialY = readYAxis();
}

void loop() {
  int currentX = readXAxis();
  int currentY = readYAxis();

  int motor1 = 0;
  int motor2 = 0;
  int motor3 = 0;
  
  if ((abs(currentX - initialX) > abs(currentY - initialY)) and (digitalRead(D4)== HIGH)) {
    if ((currentX > initialX) and (abs(currentX - initialX) > abs(initialX) * 0.13)) {
      motor1 = 255;
      motor2 = -255;
      lcd.setCursor(0, 0);
      lcd.print(padRight("Turning Left", 16));
    } else if ((currentX < initialX) and (abs(currentX - initialX) > abs(initialX) * 0.5)) {
      motor1 = -255;
      motor2 = 255;
      lcd.setCursor(0, 0);
      lcd.print(padRight("Turning Right", 16));
    }
  }
  if ((abs(currentX - initialX) < abs(currentY - initialY)) and (digitalRead(D4)== HIGH)) {
    if ((currentY > initialY) and (abs(currentY - initialY) > abs(initialY) * 0.13)) {
      motor1 = -255;
      motor2 = -255;
      lcd.setCursor(0, 0);
      lcd.print(padRight("Moving Backward", 16));
    } else if ((currentY < initialY) and (abs(currentY - initialY) > abs(initialY) * 0.5)){
      motor1 = 255;
      motor2 = 255;
      lcd.setCursor(0, 0);
      lcd.print(padRight("Moving Forward", 16));
    }
  }
  if (motor1==0 and motor2==0){
    lcd.setCursor(0, 0);
    lcd.print(padRight("Stop", 16));
  }
  if (digitalRead(D5)== LOW){
    motor3 = 255;
    lcd.setCursor(15, 0);
    lcd.write((byte)1);  // print the custom char 1 at (4, 0)
  }else if (digitalRead(D6)== LOW){
    motor3 = -255;
    lcd.setCursor(15, 0);
    lcd.write((byte)2);  // print the custom char 2 at (6, 0)
  }else{
    motor3 = 0;
  }

  if (digitalRead(D4)== LOW){
    stop_time++;  
  }
  if (stop_time==30){
    lcd.begin(16, 2);
    lcd.createChar(1, customChar1); // create a new custom character (index 1)
     lcd.createChar(2, customChar2); // create a new custom character (index 2)
    lcd.setCursor(0, 0);
    lcd.print(padRight(String("Recalibrate"), 16));
    lcd.setCursor(0, 1);
    lcd.print(padRight(String("Joystick"), 16));
    delay(1000);
    initialX = readXAxis();
    initialY = readYAxis();
    stop_time=0;
  }

  sendControlCommand(motor1,motor2 , motor3);

  lcd.setCursor(0, 1);
  lcd.print(padRight(String("X:" + String(currentX) + " Y:" + String(currentY)), 16));

  
  delay(100);
}
