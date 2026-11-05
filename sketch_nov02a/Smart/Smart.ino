#include <WiFi.h>
#include <ESP32Servo.h>

// Wi-Fi設定
const char* ssid = "TP-Link_7CC4";
const char* password = "61725157";

// TCPサーバー
WiFiServer server(5000);
WiFiClient client;

// サーボ設定
Servo myServo;

const int SERVO_PIN = D1;

// サーボ角度
int currentAngle = 180;

// 最初は安全のため可動範囲を制限
const int MIN_ANGLE = 0;
const int MAX_ANGLE = 180;//最大値、最小値、サーボの角度（初期値）を調整して直す。他はたぶん大丈夫。
// 受信データ
String receiveData = "";


// サーボ制御関数
void moveServo(int angle) {

  // 可動範囲制限
  angle = constrain(angle, MIN_ANGLE, MAX_ANGLE);

  myServo.write(angle);

  currentAngle = angle;

  Serial.print("Servo Angle: ");
  Serial.println(currentAngle);
}


// コマンド処理
String processCommand(String command) {

  command.trim();

  // OPEN
  if (command == "CLOSE") {

    moveServo(MAX_ANGLE);

    return "しめた";
  }

  // CLOSE
  else if (command == "OPEN") {

    moveServo(MIN_ANGLE);

    return "あけた";
  }

  // STATUS
  else if (command == "STATUS") {

    return "ANGLE:" + String(currentAngle);
  }

  // ANGLE指定
  else if (command.startsWith("ANGLE:")) {

    int angle = command.substring(6).toInt();

    if (angle < MIN_ANGLE || angle > MAX_ANGLE) {
      return "ERROR: ANGLE OUT OF RANGE";
    }

    moveServo(angle);

    return "ANGLE OK:" + String(currentAngle);
  }

  return "UNKNOWN COMMAND";
}


void setup() {

  Serial.begin(115200);
  delay(1000);

  // サーボ初期化
  myServo.setPeriodHertz(50);
  myServo.attach(SERVO_PIN, 500, 2500);

  myServo.write(currentAngle);

  Serial.println("Servo Ready");

  // Wi-Fi接続
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);

  Serial.println("WiFi Connecting...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected!");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // TCPサーバー開始
  server.begin();

  Serial.println("TCP Server Started");
  Serial.println("Port: 5000");
}


void loop() {

  // PCからの接続確認
  if (!client || !client.connected()) {

    client = server.available();

    if (client) {
      Serial.println("PC Connected!");
      receiveData = "";
    }
  }

  // PCからのデータ受信
  if (client && client.connected()) {

    while (client.available()) {

      char c = client.read();

      if (c == '\n') {

        receiveData.trim();

        Serial.print("PC -> ESP32: ");
        Serial.println(receiveData);

        // コマンド実行
        String response = processCommand(receiveData);

        // PCへ返信
        client.println(response);

        Serial.print("ESP32 -> PC: ");
        Serial.println(response);

        receiveData = "";

      } else if (receiveData.length() < 64) {

        receiveData += c;
      }
    }
  }

  // Wi-Fi切断確認
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Disconnected");
  }
}
