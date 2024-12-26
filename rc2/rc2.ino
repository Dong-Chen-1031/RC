// 車子的程式: 
#include <WiFi.h>
#include <WebServer.h>

// 熱點名稱與密碼
const char* ssid = "DongRcCar";
const char* password = "12345678";

// HTTP 伺服器埠號
WebServer server(80);

// 馬達控制引腳（根據 L9110S 的接線）
const int motor1_IA = 18;
const int motor1_IB = 19;
const int motor2_IA = 16;
const int motor2_IB = 17;
const int motor3_IA = 21;
const int motor3_IB = 15;

// 馬達控制函數
void controlMotor(int IA, int IB, int speed) {
  speed = constrain(speed, -255, 255);

  if (speed > 0) {
    digitalWrite(IA, HIGH);
    digitalWrite(IB, LOW);
  } else if (speed < 0) {
    digitalWrite(IA, LOW);
    digitalWrite(IB, HIGH);
  } else {
    digitalWrite(IA, LOW);
    digitalWrite(IB, LOW);
  }
}

// 馬達控制函數
void controlMotor3(int IA, int IB, int speed) {
  speed = constrain(speed, -255, 255);

  if (speed > 0) {
    digitalWrite(IA, HIGH);
    digitalWrite(IB, LOW);
  } else if (speed < 0) {
    digitalWrite(IA, HIGH);
    digitalWrite(IB, HIGH);
  } else {
    digitalWrite(IA, LOW);
    digitalWrite(IB, LOW);
  }
}

// 處理 /control 請求的函數
void handleControl() {
  if (server.hasArg("motor1") && server.hasArg("motor2") && server.hasArg("motor3")) {
    int motor1_speed = server.arg("motor1").toInt();
    int motor2_speed = server.arg("motor2").toInt();
    int motor3_speed = server.arg("motor3").toInt();

    controlMotor(motor1_IA, motor1_IB, motor1_speed);
    controlMotor(motor2_IA, motor2_IB, motor2_speed);
    controlMotor3(motor3_IA, motor3_IB, motor3_speed);

    server.send(200, "text/plain", "Motors updated");
  } else {
    server.send(400, "text/plain", "Invalid parameters");
  }
}

void setup() {
  // 初始化序列通訊
  Serial.begin(115200);

  // 啟動熱點模式
  WiFi.softAP(ssid, password);
  Serial.println("Wi-Fi 熱點已啟動，IP 地址為: " + WiFi.softAPIP().toString());

  // 初始化馬達控制引腳
  pinMode(motor1_IA, OUTPUT);
  pinMode(motor1_IB, OUTPUT);
  pinMode(motor2_IA, OUTPUT);
  pinMode(motor2_IB, OUTPUT);
  pinMode(motor3_IA, OUTPUT);
  pinMode(motor3_IB, OUTPUT);
  digitalWrite(motor1_IA, LOW);
  digitalWrite(motor1_IB, LOW);
  digitalWrite(motor2_IA, LOW);
  digitalWrite(motor2_IB, LOW);
  digitalWrite(motor3_IA, LOW);
  digitalWrite(motor3_IB, LOW);

  // 設置 HTTP 伺服器路由
  server.on("/control", handleControl);

  // 啟動 HTTP 伺服器
  server.begin();
  Serial.println("HTTP 伺服器已啟動!");
}

void loop() {
  // 處理 HTTP 請求
  server.handleClient();
}
