#include <SoftwareSerial.h> //블루투스 통신 라이브러리
#include <DHT.h> // 온습도센서 DHT11을 쓰기 위한 라이브러리

#define DHTPIN 5 //5번핀에 연결
#define DHTTYPE DHT11 //센서 타입 지정
DHT dht(DHTPIN, DHTTYPE); //변수 지정값 


//각각의 핀 번호를 변수로 지정
const int INA = 8;
const int INB = 9;
const int bluetoothTx = 2;
const int bluetoothRx = 3;
const int trigPin = 6;
const int echoPin = 7;
const int buzzerPin = 10;
const int ledRed = 11;
const int ledGreen = 12;

int time=0;
bool fanState = false;

SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);
//블루투스 모듈을 변수 지정 소프트웨어 시리얼로 설정(TX=2,RX=3)

unsigned long timerStart = 0;
bool timerActive = false;
bool ultrasonicEnabled = false;
bool tempControlEnabled = false;
//타이머,초음파 제어, 온도 제어 상태 저장 변수

void setup() {
  pinMode(INA, OUTPUT);
  pinMode(INB, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  //각 핀들을 입/출력 모드로 설정

  Serial.begin(9600);
  bluetooth.begin(9600);
  dht.begin();
  //시리얼 통신, 블루투스 통신,DHT11 센서도 초기화

  fanOff();  // 초기 상태는 OFF
  Serial.println("System Ready");
}

void loop() {
  if (bluetooth.available()) {
    char cmd = bluetooth.read(); //블루투스 들어온 명령 읽음
    Serial.print("Received command: ");
    Serial.println(cmd);

    if (cmd == '1') {
      fanOn(); //펜 켜짐
      timerActive = false; //타이머 활동 X
      ultrasonicEnabled = false; //초음파 활동 X
      tempControlEnabled = false; //온도 
    } else if (cmd == '2') {
      fanOff(); //펜 꺼짐
      timerActive = false;
      ultrasonicEnabled = false;
      tempControlEnabled = false;
      Serial.println("all OFF");
    } else if (cmd == '3') {
      String timeStr = "";
      // 블루투스 버퍼에 남아있는 숫자들 읽기 (예: "5000")
      while (bluetooth.available()) {
        char c = bluetooth.read();
        if (isDigit(c)) {
          timeStr += c;
        }
        delay(5); // 블루투스 통신 안정화용 짧은 지연
      }
      time = timeStr.toInt(); // 문자열을 정수로 변환하여 ms 단위 시간 저장
      Serial.print("Received time (ms): ");
      Serial.println(time);
      fanOn(); // 펜 켜기
      timerStart = millis();
      timerActive = true;
      ultrasonicEnabled = false;
      tempControlEnabled = false;
    } else if (cmd == '4') {
      ultrasonicEnabled = true; //초음파 활동
      timerActive = false;
      tempControlEnabled = false;
      Serial.println("Ultrasonic Control ENABLED");
    } else if (cmd == '5') {
      tempControlEnabled = true; //온도 켜짐
      ultrasonicEnabled = false;
      timerActive = false;
      Serial.println("Temperature Control ENABLED");
    }
  }
	// 타이머 제어 코드
  if (timerActive && millis() - timerStart >= (unsigned long)time) {
    fanOff();
    timerActive = false;
    Serial.println("Timer expired: Fan OFF");
  }

	//초음파 코드
  if (ultrasonicEnabled) {
    long duration, distance;
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance = duration * 0.034 / 2;
    Serial.print("Distance: ");
    Serial.println(distance);
		//거리코드 200이하일때 켜지고있음
    if (distance <= 200) { 
      fanOn();
      Serial.println("Fan ON (by ultrasonic)");
    } else {
      fanOff();
      Serial.println("Fan OFF (by ultrasonic)");
    }
  }
	//온도 제어 코드
  if (tempControlEnabled) {
    float temp = dht.readTemperature();
    Serial.print("Temp: ");
    Serial.println(temp);
    //20 이상 켜짐
    if (temp > 20.0) {
      fanOn();
      Serial.println("Fan ON (by temp)");
    } else {
      fanOff();
      Serial.println("Fan OFF (by temp)");
    }
  }

  delay(500);
}


void fanOn() {
  if (!fanState) {  // 꺼진 상태였다면
    digitalWrite(INA, LOW);
    digitalWrite(INB, HIGH);
    digitalWrite(ledGreen, HIGH);
    digitalWrite(ledRed, LOW);
    tone(buzzerPin, 1000, 200); // 삐 - 켜질 때만 울림
    fanState = true;
  }
}


void fanOff() {
  if (fanState) {  // 켜진 상태였다면
    digitalWrite(INA, LOW);
    digitalWrite(INB, LOW);
    digitalWrite(ledGreen, LOW);
    digitalWrite(ledRed, HIGH);
    tone(buzzerPin, 500, 200); // 삐 - 꺼질 때만 울림
    fanState = false;
  }
}

 
 
 