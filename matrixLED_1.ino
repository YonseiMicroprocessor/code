// 포커 경마 - Matrix LED 초기 세팅
#include <Arduino.h>

// === 핀 매핑 ===
// ROW = D30~D37 (R1~R8)
const int colPins[8] = {30, 31, 32, 33, 34, 35, 36, 37};
// COL = D22~D29 (C1~C8)
const int rowPins[8] = {22, 23, 24, 25, 26, 27, 28, 29};

const int buttonPin = A15; // PB1 버튼 (J25 → A15)

// 현재 LED 상태를 저장할 2차원 배열
bool dotState[8][8] = {false};

void setup() {
  // 핀 설정
  for (int i = 0; i < 8; i++) {
    pinMode(colPins[i], OUTPUT);
    pinMode(rowPins[i], OUTPUT);
    digitalWrite(colPins[i], LOW);     // COL 기본 LOW
    digitalWrite(rowPins[i], HIGH);    // ROW 기본 HIGH (꺼짐 상태)
  }

  pinMode(buttonPin, INPUT_PULLUP);

    // 초기 점등 상태 세팅
  // ROW 0~5, COL 7에 불 켜기
  for (int r = 1; r <= 6; r++) {
    dotState[r][7] = true;
  }

  // ROW 7, COL 0,2,4,6 켜기
  dotState[7][0] = true;
  dotState[7][2] = true;
  dotState[7][4] = true;
  dotState[7][6] = true;
}

void loop() {
  drawMatrix();

    // 버튼(PB1) 누르면 점 하나 왼쪽으로 이동
  if (digitalRead(buttonPin) == LOW) {
    delay(200); // 디바운스용

    // 오른쪽 열에 불 켜진 점 위치 모으기
    int activeRows[8];
    int count = 0;
    for (int r = 0; r < 8; r++) {
      if (dotState[r][7]) {
        activeRows[count++] = r;
      }
    }

    if (count > 0) {
      int pick = random(count);   // 무작위 선택
      int r = activeRows[pick];

      dotState[r][7] = false;     // 기존 점 끄고
      dotState[r][6] = true;      // 왼쪽 한 칸 이동
    }
  }
}

void drawMatrix() {
  for (int r = 0; r < 8; r++) {
    digitalWrite(rowPins[r], LOW); // 해당 행 선택

    for (int c = 0; c < 8; c++) {
      digitalWrite(colPins[c], dotState[r][c] ? HIGH : LOW);
    }

    delay(2); // 짧게 점등
    digitalWrite(rowPins[r], HIGH); // 끄기

    for (int c = 0; c < 8; c++) {
      digitalWrite(colPins[c], LOW); // COL OFF
    }
  }
}
