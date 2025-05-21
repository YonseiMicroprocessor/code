// 포커 경마 – Matrix LED + 버튼 + 시리얼 모니터 + 부저
#include <Arduino.h>

// === 핀 매핑 ===
const int matrixSourcePins[8] = {30,31,32,33,34,35,36,37};
const int matrixSinkPins[8]   = {22,23,24,25,26,27,28,29};
const int buttonPin = A15;      // 풀업 버튼
const int buzzerPin = 14;       // 부저 + → D12, - → GND

// === 디바운스 ===
const unsigned long debounceDelay = 200;
unsigned long lastDebounceTime = 0;
bool prevButtonState = HIGH;

// === 게임 파라미터 ===
const int horseCount = 4;
const int trackLength = 7;
bool gameOver = false;

// 카드 덱
enum Suit {DIAMOND, HEART, SPADE, CLOVER};
enum CardType {NORMAL, REVERSE};
struct Card { Suit suit; CardType type; };
Card deck[48];
int deckSize, remainingNormal, remainingReverse;

// 말 위치 & LED 맵
int horseCols[horseCount];
bool dotState[8][8];

// 이모지 맵
static const char* suitNames[] = {"Diamond","Heart","Spade","Clover"};
static const char* suitIcons[] = {"♦","♥","♠","♣"};

// 함수 원형
void initPins(), initGame(), initDeck();
Card drawCard();
void advanceHorse(Card card), drawMatrix();
void printStatus(Card card), printWinner(int idx);

void setup() {
  Serial.begin(9600);
  Serial.println("--- Poker Horse Race Initialized ---");
  randomSeed(analogRead(A0));

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);  // 부저 핀 설정

  initPins();
  initGame();
  initDeck();
  printStatus({DIAMOND, NORMAL});
}

void loop() {
  drawMatrix();
  if (gameOver) return;

  bool curr = digitalRead(buttonPin);
  if (curr==LOW && prevButtonState==HIGH
      && millis()-lastDebounceTime>debounceDelay) {
    if (deckSize>0) {
      Card card = drawCard();
      advanceHorse(card);       // 말 이동 + 효과음
      printStatus(card);
      // 승자 판정
      for (int i = 0; i < horseCount; i++) {
        if (horseCols[i] == trackLength) {
          printWinner(i);
          gameOver = true;
          break;
        }
      }
    } else {
      Serial.println("Deck empty. Game over.");
      gameOver = true;
    }
    lastDebounceTime = millis();
  }
  prevButtonState = curr;
}

// 핀 초기화
void initPins() {
  for (int i = 0; i < 8; i++) {
    pinMode(matrixSinkPins[i], OUTPUT);
    digitalWrite(matrixSinkPins[i], HIGH);
    pinMode(matrixSourcePins[i], OUTPUT);
    digitalWrite(matrixSourcePins[i], LOW);
  }
}

// 게임 초기화
void initGame() {
  memset(dotState, 0, sizeof(dotState));
  for (int i = 0; i < horseCount; i++) {
    horseCols[i] = 0;
    dotState[i*2][0] = true;
  }
  for (int c = 1; c <= 6; c++) dotState[7][c] = true;
}

// 덱 초기화 (48장 + 6 리버스)
void initDeck() {
  Card temp[52]; int idx = 0;
  for (int s = 0; s < 4; s++)
    for (int k = 0; k < 13; k++)
      temp[idx++] = { (Suit)s, NORMAL };

  bool mark[52] = {};
  mark[0] = mark[13] = mark[26] = mark[39] = true;  // 말 카드 제외
  int reserved = 0;
  while (reserved < 6) {
    int r = random(52);
    if (!mark[r]) { mark[r] = true; temp[r].type = REVERSE; reserved++; }
  }

  deckSize = remainingNormal = remainingReverse = 0;
  for (int i = 0; i < 52; i++) {
    if (!mark[i] || temp[i].type == REVERSE) {
      deck[deckSize++] = temp[i];
      if (temp[i].type == NORMAL) remainingNormal++;
      else remainingReverse++;
    }
  }
}

// 카드 뽑기
Card drawCard() {
  int r = random(deckSize);
  Card card = deck[r];
  deck[r] = deck[--deckSize];
  if (card.type == NORMAL) remainingNormal--;
  else remainingReverse--;
  return card;
}

// 말 전진/후진 + 효과음
void advanceHorse(Card card) {
  int i = card.suit;
  dotState[i*2][horseCols[i]] = false;

  if (card.type == NORMAL) {
    if (horseCols[i] < trackLength) horseCols[i]++;
    tone(buzzerPin, 1000, 100);  // 전진 삑 소리
  } else {
    if (horseCols[i] > 0) horseCols[i]--;
    tone(buzzerPin, 500, 100);   // 후진 낮은 톤
  }

  dotState[i*2][horseCols[i]] = true;
  // 트랙 소거
  if (card.type == NORMAL) {
    int minc = 7;
    for (int j = 0; j < horseCount; j++)
      if (horseCols[j] < minc) minc = horseCols[j];
    if (minc >= 1 && minc <= 6) dotState[7][minc] = false;
  }
}

// 매트릭스 스캔
void drawMatrix() {
  for (int r = 0; r < 8; r++) {
    digitalWrite(matrixSinkPins[r], LOW);
    for (int c = 0; c < 8; c++)
      digitalWrite(matrixSourcePins[c],
                   dotState[r][c] ? HIGH : LOW);
    delay(2);
    digitalWrite(matrixSinkPins[r], HIGH);
    for (int c = 0; c < 8; c++)
      digitalWrite(matrixSourcePins[c], LOW);
  }
}

// 상태 출력
void printStatus(Card card) {
  Serial.print("Drawn: ");
  Serial.print(suitIcons[card.suit]); Serial.print(" ");
  Serial.print(suitNames[card.suit]); Serial.print(" (");
  Serial.print(card.type==NORMAL?"Normal":"Reverse");
  Serial.println(")");

  Serial.print("Positions: ");
  for (int i = 0; i < horseCount; i++) {
    Serial.print(suitIcons[i]); Serial.print(":");
    Serial.print(horseCols[i]);
    if (i < horseCount-1) Serial.print(", ");
  }
  Serial.println();

  Serial.print("Remaining - ");
  Serial.print("🂠("); Serial.print(remainingNormal); Serial.print(") ");
  Serial.print("↺("); Serial.print(remainingReverse); Serial.println(")");
  Serial.println("----------------------");
}

// 승자 출력 및 축하 멜로디
void printWinner(int idx) {
  Serial.print("Winner: ");
  Serial.print(suitIcons[idx]); Serial.print(" ");
  Serial.println(suitNames[idx]);
  Serial.println("=== Game Over ===");
  
  // 축하 멜로디: 도(523Hz), 미(659Hz), 솔(784Hz)
  tone(buzzerPin, 523, 200); delay(250);
  tone(buzzerPin, 659, 200); delay(250);
  tone(buzzerPin, 784, 400); delay(450);
  noTone(buzzerPin);
}
