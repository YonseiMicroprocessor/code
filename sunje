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

bool prevButtonState = HIGH;

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 200;

// 움직이는 LED들의 위치를 저장할 배열 (ROW 0,2,4,6)
int ledPositions[4] = {0, 0, 0, 0};  // 초기 위치는 모두 0열
int activeLedCount = 4;  // 현재 활성화된 LED 개수

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
  // COL 0, ROW 0,2,4,6에 불 켜기 (움직이는 LED)
  dotState[0][0] = true;
  dotState[2][0] = true;
  dotState[4][0] = true;
  dotState[6][0] = true;

  // ROW 7, COL 1~7에 불 켜기
  for (int c = 1; c <= 6; c++) {
    dotState[7][c] = true;
  }
}

void loop() {
    drawMatrix();
    bool currState = digitalRead(buttonPin);
    
    // 버튼이 눌렸을 때
    if (currState == LOW && prevButtonState == HIGH) {
        if ((millis() - lastDebounceTime) > debounceDelay) {
            // 무작위로 하나의 LED만 선택
            int randomLed = random(activeLedCount);
            int row = randomLed * 2;  // 0,2,4,6 중 하나
            
            // 현재 위치의 LED를 끄고
            dotState[row][ledPositions[randomLed]] = false;
            
            // 오른쪽으로 한 칸 이동 (COL 증가)
            if (ledPositions[randomLed] < 7) {  // COL 7이 아닐 때만 이동
                ledPositions[randomLed]++;
                dotState[row][ledPositions[randomLed]] = true;
                
                // 가장 뒤쳐진 LED의 위치 확인
                int minCol = 7;
                for (int i = 0; i < activeLedCount; i++) {
                    if (ledPositions[i] < minCol) {
                        minCol = ledPositions[i];
                    }
                }
                
                // 가장 뒤쳐진 LED가 COL 6~1에 도달하면 ROW 7의 해당 COL LED 끄기
                if (minCol >= 1 && minCol <= 6) {
                    dotState[7][minCol] = false;
                }
            }
            
            lastDebounceTime = millis();
        }
    }
    prevButtonState = currState;
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
