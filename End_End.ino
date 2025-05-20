// 포커 경마 - Matrix LED 초기 세팅 및 게임 로직 개선
#include <Arduino.h>

// === 매트릭스 핀 매핑 ===
// 전류 소스: D30~D37 (Column)
const int matrixSourcePins[8] = {30, 31, 32, 33, 34, 35, 36, 37};
// 전류 싱크: D22~D29 (Row)
const int matrixSinkPins[8]   = {22, 23, 24, 25, 26, 27, 28, 29};

// 입력 버튼 (풀업)
const int buttonPin = A15;  // J25 → A15

// 디바운스 설정
const unsigned long debounceDelay = 200;
unsigned long lastDebounceTime = 0;
bool prevButtonState = HIGH;

// 게임 상태
const int horseCount = 4;            // 말 개수
int horseCols[horseCount];           // 각 말의 현재 열 위치
bool trackRow[8];                    // 바닥 트랙(7행) LED 상태
bool gameOver = false;

// 함수 선언
void initPins();
void initGame();
void drawMatrix();
void advanceHorse();
void announceWinner(int idx);

void setup() {
  // 랜덤 시드 설정
  randomSeed(analogRead(A0));

  initPins();
  pinMode(buttonPin, INPUT_PULLUP);

  initGame();
}

void loop() {
  drawMatrix();
  if (!gameOver) {
    bool currState = digitalRead(buttonPin);
    if (currState == LOW && prevButtonState == HIGH
        && millis() - lastDebounceTime > debounceDelay) {
      advanceHorse();
      lastDebounceTime = millis();
    }
    prevButtonState = currState;
  }
}

// 핀을 OUTPUT으로 초기화하고 꺼진 상태로 설정
void initPins() {
  for (int i = 0; i < 8; i++) {
    pinMode(matrixSinkPins[i], OUTPUT);
    pinMode(matrixSourcePins[i], OUTPUT);
    digitalWrite(matrixSinkPins[i], HIGH);    // Row off
    digitalWrite(matrixSourcePins[i], LOW);   // Col off
  }
}

// 게임 초기화: 말 위치, 바닥 트랙 세팅
void initGame() {
  // 말 초기 위치(모두 0열)
  for (int i = 0; i < horseCount; i++) {
    horseCols[i] = 0;
  }
  
  // 트랙(7행) 위치 1~6열 켜기
  for (int c = 0; c < 8; c++) {
    trackRow[c] = (c >= 1 && c <= 6);
  }
  
  gameOver = false;
}

// 랜덤 한 말 전진 및 트랙 LED 소거, 우승 검사
void advanceHorse() {
  int idx = random(horseCount);      // 0~3 중 선택
  int row = idx * 2;                 // 0,2,4,6행

  // 이전 위치 소등
  trackRow[ horseCols[idx] ];        // (트랙은 따로 관리)
  
  // 전진
  if (horseCols[idx] < 7) {
    horseCols[idx]++;
  }

  // 바닥 트랙 LED 소거
  int minCol = 7;
  for (int i = 0; i < horseCount; i++) {
    if (horseCols[i] < minCol) minCol = horseCols[i];
  }
  if (minCol >= 1 && minCol <= 6) {
    trackRow[minCol] = false;
  }

  // 우승 조건: 마지막 열 도달
  if (horseCols[idx] == 7) {
    announceWinner(idx);
    gameOver = true;
  }
}

// 승자 LED 깜빡임 표시
void announceWinner(int idx) {
  int row = idx * 2;
  for (int k = 0; k < 6; k++) {
    // 깜빡임
    delay(200);
    // 모든 LED 끔
    for (int r = 0; r < 8; r++) {
      digitalWrite(matrixSinkPins[r], HIGH);
    }
    delay(200);
  }
}

// 매트릭스 스캔 및 점등
void drawMatrix() {
  for (int r = 0; r < 8; r++) {
    digitalWrite(matrixSinkPins[r], LOW);  // 해당 행 on
    for (int c = 0; c < 8; c++) {
      bool state = false;
      // 말 위치 표시
      for (int i = 0; i < horseCount; i++) {
        if (r == i*2 && c == horseCols[i]) state = true;
      }
      // 바닥 트랙 표시
      if (r == 7 && trackRow[c]) state = true;
      digitalWrite(matrixSourcePins[c], state ? HIGH : LOW);
    }
    delay(2);
    digitalWrite(matrixSinkPins[r], HIGH);// 해당 행 off
    // 컬럼 초기화
    for (int c = 0; c < 8; c++) {
      digitalWrite(matrixSourcePins[c], LOW);
    }
  }
}
