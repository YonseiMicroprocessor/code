#include <Arduino.h>

// === 핀 매핑 ===
// ROW = D30~D37 (R1~R8)
const int colPins[8] = {30, 31, 32, 33, 34, 35, 36, 37};
// COL = D22~D29 (C1~C8)
const int rowPins[8] = {22, 23, 24, 25, 26, 27, 28, 29};

const int buttonPin = A15; // PB1 버튼 (J25 → A15)

// 현재 LED 상태를 저장할 2차원 배열
bool dotState[8][8] = {false};

void drawMatrix();

void setup() {
  // 핀 설정
  for (int i = 0; i < 8; i++) {
    pinMode(rowPins[i], OUTPUT);
    pinMode(colPins[i], OUTPUT);
    digitalWrite(rowPins[i], LOW);     // COL 기본 LOW
    digitalWrite(colPins[i], HIGH);    // ROW 기본 HIGH (꺼짐 상태)
  }

  pinMode(buttonPin, INPUT_PULLUP);

  // 초기 점등 상태 세팅
  // ROW 1~6, COL 7에 불 켜기
  for (int r = 1; r <= 6; r++) {
    dotState[r][7] = true;
  }

  // ROW 0,2,4,6 COL 7 켜기
  dotState[0][7] = true;
  dotState[2][7] = true;
  dotState[4][7] = true;
  dotState[6][7] = true;
}

void loop() {
  drawMatrix();

  // 버튼 누르면 점 하나 왼쪽으로 이동
  if (digitalRead(buttonPin) == LOW) {
    delay(200); // 디바운스

    // 이동 가능한 점 찾기 (col > 0인 점만)
    int activeRows[64];
    int activeCols[64];
    int count = 0;

    for (int r = 0; r < 8; r++) {
      for (int c = 1; c < 8; c++) { // col = 0이면 더 못 움직이므로 제외
        if (dotState[r][c]) {
          activeRows[count] = r;
          activeCols[count] = c;
          count++;
        }
      }
    }

    // 점이 있으면 무작위 하나 선택해서 왼쪽으로 이동
    if (count > 0) {
      int pick = random(count);
      int r = activeRows[pick];
      int c = activeCols[pick];

      dotState[r][c] = false;
      dotState[r][c - 1] = true;
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
