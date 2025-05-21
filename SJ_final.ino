// 포커 경마 – Matrix LED + 버튼 + 시리얼 모니터 통합 (승자 판정 추가)
#include <Arduino.h>

// === 핀 매핑 ===
// 전류 소스: D30~D37 (Column)
const int matrixSourcePins[8] = {30,31,32,33,34,35,36,37};
// 전류 싱크: D22~D29 (Row)
const int matrixSinkPins[8]   = {22,23,24,25,26,27,28,29};
// 입력 버튼: J25→A15
const int buttonPin = A15;

// === 디바운스 ===
const unsigned long debounceDelay = 200;
unsigned long lastDebounceTime = 0;
bool prevButtonState = HIGH;

// === 게임 파라미터 ===
const int horseCount = 4;         // 말 4마리
const int trackLength = 7;        // 0~7칸
bool gameOver = false;            // 게임 종료 플래그

// 카드 덱 관리
enum Suit {DIAMOND, HEART, SPADE, CLOVER};
enum CardType {NORMAL, REVERSE};
struct Card { Suit suit; CardType type; };

Card deck[48];                   // 42 일반 + 6 리버스
int deckSize = 0;
int remainingNormal = 0;         // 일반 카드 남은 수
int remainingReverse = 0;        // 리버스 카드 남은 수

// 말 위치 및 LED 상태
int horseCols[horseCount];       // 각 말의 현재 컬럼(0~7)
bool dotState[8][8];             // 매트릭스 LED 온/오프 맵

// 이모지 및 이름 맵
static const char* suitNames[] = {"Diamond","Heart","Spade","Clover"};
static const char* suitIcons[] = {"♦","♥","♠","♣"};

// 함수 선언
void initPins();
void initGame();
void initDeck();
Card drawCard();
void advanceHorse(Card card);
void drawMatrix();
void printStatus(Card card);
void printWinner(int idx);

void setup() {
  Serial.begin(9600);
  Serial.println("--- Poker Horse Race Initialized ---");
  Serial.println("Button: Draw card and advance/reverse horse");

  randomSeed(analogRead(A0));
  pinMode(buttonPin, INPUT_PULLUP);
  initPins(); initGame(); initDeck();
  printStatus({DIAMOND,NORMAL}); // 초기 상태 출력
}

void loop() {
  drawMatrix();
  if (gameOver) return;

  bool curr = digitalRead(buttonPin);
  if (curr==LOW && prevButtonState==HIGH
      && millis()-lastDebounceTime>debounceDelay) {
    if (deckSize>0) {
      Card card = drawCard();
      advanceHorse(card);
      printStatus(card);
      // 승자 발생 시 출력
      for (int i=0; i<horseCount; i++) {
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
  for (int i=0;i<8;i++){
    pinMode(matrixSinkPins[i], OUTPUT);
    digitalWrite(matrixSinkPins[i], HIGH);
    pinMode(matrixSourcePins[i], OUTPUT);
    digitalWrite(matrixSourcePins[i], LOW);
  }
}

// 게임 초기화
void initGame() {
  memset(dotState, 0, sizeof(dotState));
  for (int i=0;i<horseCount;i++) {
    horseCols[i]=0;
    dotState[i*2][0]=true;
  }
  for (int c=1;c<=6;c++) dotState[7][c]=true;
  gameOver = false;
}

// 덱 초기화
void initDeck() {
  Card temp[52]; int idx=0;
  for (int s=0;s<4;s++) for (int k=0;k<13;k++)
    temp[idx++]={static_cast<Suit>(s), NORMAL};
  bool mark[52] = {};
  mark[0]=mark[13]=mark[26]=mark[39]=true; // 말 카드 제외
  int reserved=0;
  while(reserved<6) {
    int r=random(52);
    if (!mark[r]) { mark[r]=true; temp[r].type=REVERSE; reserved++; }
  }
  deckSize=0; remainingNormal=0; remainingReverse=0;
  for (int i=0;i<52;i++) {
    if (!mark[i] || temp[i].type==REVERSE) {
      deck[deckSize++] = temp[i];
      if (temp[i].type==NORMAL) remainingNormal++; else remainingReverse++;
    }
  }
}

// 카드 뽑기
Card drawCard() {
  int r = random(deckSize);
  Card card = deck[r];
  deck[r] = deck[--deckSize];
  if (card.type==NORMAL) remainingNormal--; else remainingReverse--;
  return card;
}

// 말 전진/후진
void advanceHorse(Card card) {
  int i = card.suit;
  dotState[i*2][horseCols[i]] = false;
  if (card.type==NORMAL) {
    if (horseCols[i]<trackLength) horseCols[i]++;
  } else {
    if (horseCols[i]>0) horseCols[i]--;
  }
  dotState[i*2][horseCols[i]] = true;
  if (card.type==NORMAL) {
    int minc=7;
    for (int j=0;j<horseCount;j++) if (horseCols[j]<minc) minc=horseCols[j];
    if (minc>=1 && minc<=6) dotState[7][minc]=false;
  }
}

// 매트릭스 스캔
void drawMatrix() {
  for (int r=0;r<8;r++){
    digitalWrite(matrixSinkPins[r], LOW);
    for (int c=0;c<8;c++) digitalWrite(matrixSourcePins[c], dotState[r][c]?HIGH:LOW);
    delay(2);
    digitalWrite(matrixSinkPins[r], HIGH);
    for (int c=0;c<8;c++) digitalWrite(matrixSourcePins[c], LOW);
  }
}

// 상태 출력 (이모지 포함)
void printStatus(Card card) {
  Serial.print("Drawn: ");
  Serial.print(suitIcons[card.suit]); Serial.print(" ");
  Serial.print(suitNames[card.suit]); Serial.print(" (");
  Serial.print(card.type==NORMAL?"Normal":"Reverse"); Serial.println(")");

  Serial.print("Positions: ");
  for (int i=0;i<horseCount;i++) {
    Serial.print(suitIcons[i]); Serial.print(":"); Serial.print(horseCols[i]);
    if (i<horseCount-1) Serial.print(", ");
  }
  Serial.println();

  Serial.print("Remaining - ");
  Serial.print("🂠("); Serial.print(remainingNormal); Serial.print(") ");
  Serial.print("↺("); Serial.print(remainingReverse); Serial.println(")");

  Serial.println("----------------------");
}

// 우승 말 출력
void printWinner(int idx) {
  Serial.print("Winner: ");
  Serial.print(suitIcons[idx]); Serial.print(" ");
  Serial.println(suitNames[idx]);
  Serial.println("=== Game Over ===");
}
