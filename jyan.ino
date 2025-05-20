#include <Arduino.h>

// 슈트 정의
enum Suit { DIAMOND, HEART, SPADE, CLOVER };

// 4x4 LED 매트릭스 핀 (D2 ~ D17 사용 가정)
const int ledPins[4][4] = {
  {0, 1, 2, 3},
  {4, 5, 6, 7},
  {14, 15, 16, 17},
  {18, 19, 20, 21}
};

// 버튼 핀
const int buttonPin = A0;
Suit currentSuit = DIAMOND;

// 디바운스 처리
unsigned long lastPress = 0;
const unsigned long debounceDelay = 200;

void setup() {
  // LED 핀 출력 설정
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      pinMode(ledPins[r][c], OUTPUT);
    }
  }

  // 버튼 핀 설정
  pinMode(buttonPin, INPUT_PULLUP);

  // 초기 슈트 표시
  showCardLED(currentSuit);
}

void loop() {
  if (digitalRead(buttonPin) == LOW) {
    if (millis() - lastPress > debounceDelay) {
      currentSuit = static_cast<Suit>((currentSuit + 1) % 4);
      showCardLED(currentSuit);
      lastPress = millis();
    }
  }
}

// 슈트에 맞는 4개 LED 점등
void showCardLED(Suit s) {
  // 모든 LED 끄기
  for (int r = 0; r < 4; r++)
    for (int c = 0; c < 4; c++)
      digitalWrite(ledPins[r][c], LOW);

  // 슈트에 따라 2x2 정사각형 점등
  int baseR = (s / 2) * 2;  // 0 or 2
  int baseC = (s % 2) * 2;  // 0 or 2
  for (int r = 0; r < 2; r++) {
    for (int c = 0; c < 2; c++) {
      digitalWrite(ledPins[baseR + r][baseC + c], HIGH);
    }
  }
}
