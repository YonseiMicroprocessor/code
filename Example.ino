// MCube 10핀 헤더 전용 통합 스케치 (초기 표시 수정)
#include <Arduino.h>

// === 핀 매핑 ===
// J1 D8–D13: GREEN LED[0..5]
const int greenLEDs[6]   = {8, 9, 10, 11, 12, 13};
// J2 D0–D7: RED LED[0..1] (다이아, 하트)
const int redLEDs[2]     = {0, 1};
// J3 D14–D21: extraGreen[0..1], buzzerPin (D17)
const int extraGreen[2]  = {14, 15};
const int buzzerPin      = 17;
// J4 D22–D29: Matrix COL
const int colPins[8]     = {22,23,24,25,26,27,28,29};
// J13 D30–D37: Matrix ROW
const int rowPins[8]     = {30,31,32,33,34,35,36,37};
// J14 D38–D45: 7-Segment segments A~G, DP
const int segmentPins[8] = {38,39,40,41,42,43,44,45};
// J26 D46–D47: 7-Segment digit select S0 (left), S1 (right)
const int digitPins[2]   = {46,47};
// J25 A15: PB1 버튼
const int buttonPin      = A15;

// === 게임 상태 ===
int cardCount    = 42;
int reverseCount = 6;
int horsePos[4]  = {0,0,0,0};
bool reverseMode = false;

enum Suit { DIAMOND, HEART, SPADE, CLOVER };
Suit deck[48], revCards[6];
int deckIdx = 0;

// 프로토타입
void initDeck(), shuffleDeck(), pickReverse();
void moveHorse(Suit s,int d), updateMatrix();
void showCardLED(Suit s), show7Seg();
void show2(int dig,int v), playTune();

void setup(){
  Serial.begin(9600);
  // LED
  for(int i=0;i<6;i++) pinMode(greenLEDs[i], OUTPUT);
  for(int i=0;i<2;i++) pinMode(redLEDs[i],   OUTPUT);
  for(int i=0;i<2;i++) pinMode(extraGreen[i],OUTPUT);
  // Buzzer & 버튼
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  // Matrix
  for(int i=0;i<8;i++){
    pinMode(colPins[i], OUTPUT);
    pinMode(rowPins[i], OUTPUT);
  }
  // 7-Segment
  for(int i=0;i<8;i++) pinMode(segmentPins[i], OUTPUT);
  for(int i=0;i<2;i++) pinMode(digitPins[i],   OUTPUT);

  // 덱 초기화
  initDeck(); shuffleDeck(); pickReverse();

  // 초기 표시
  // 7-Segment: left=6, right=42
  show7Seg();
  // Matrix: 말 초기 위치 (row 7, cols 0,2,4,6)
  updateMatrix();
}

void loop(){
  // 매 루프 7-Segment 갱신
  show7Seg();
  // 버튼 눌림 감지
  if(digitalRead(buttonPin)==LOW){
    delay(200);
    if(reverseMode && reverseCount>0){
      Suit r = revCards[6-reverseCount];
      moveHorse(r, -1);
      reverseCount--;
    } else if(cardCount>0){
      Suit s = deck[deckIdx++];
      cardCount--;
      moveHorse(s, +1);
      showCardLED(s);
      // 리버스 모드 전환 조건
      bool all=true; for(int i=0;i<4;i++) if(horsePos[i]==0) all=false;
      if(all) reverseMode=true;
    }
    // 상태 갱신 후 다시 표시
    show7Seg();
    // 종료 체크
    for(int i=0;i<4;i++) if(horsePos[i]>=7){ playTune(); while(1); }
  }
}

void initDeck(){
  int idx=0;
  for(int i=0;i<13;i++) deck[idx++]=DIAMOND;
  for(int i=0;i<13;i++) deck[idx++]=HEART;
  for(int i=0;i<13;i++) deck[idx++]=SPADE;
  for(int i=0;i<13;i++) deck[idx++]=CLOVER;
}
void shuffleDeck(){
  for(int i=0;i<100;i++){
    int a=random(48),b=random(48);
    Suit t=deck[a]; deck[a]=deck[b]; deck[b]=t;
  }
}
void pickReverse(){
  for(int i=0;i<6;i++) revCards[i]=deck[i];
  deckIdx=6;
}

void moveHorse(Suit s,int d){
  int &p=horsePos[s];
  if(d>0 && p<7) p++;
  if(d<0 && p>0) p--;
  updateMatrix();
}
void updateMatrix(){
  for(int r=0;r<8;r++) digitalWrite(rowPins[r],HIGH);
  for(int c=0;c<8;c++) digitalWrite(colPins[c],LOW);
  for(int i=0;i<4;i++){
    int r=7-horsePos[i], c=i*2;
    digitalWrite(colPins[c],HIGH);
    digitalWrite(rowPins[r],LOW);
  }
}

void showCardLED(Suit s){
  for(int i=0;i<6;i++) digitalWrite(greenLEDs[i],LOW);
  for(int i=0;i<2;i++) digitalWrite(redLEDs[i],LOW);
  for(int i=0;i<2;i++) digitalWrite(extraGreen[i],LOW);
  switch(s){
    case DIAMOND: digitalWrite(redLEDs[0], HIGH); break;
    case HEART:   digitalWrite(redLEDs[1], HIGH); break;
    case SPADE:   digitalWrite(greenLEDs[4], HIGH); break;
    case CLOVER:  digitalWrite(greenLEDs[5], HIGH); break;
  }
}

void show7Seg(){
  show2(digitPins[0], reverseCount);
  show2(digitPins[1], cardCount);
}
void show2(int dig,int val){
  static const byte M[10]={
    B00111111,B00000110,B01011011,B01001111,B01100110,
    B01101101,B01111101,B00000111,B01111111,B01101111
  };
  int t=val/10, o=val%10;
  for(int i=0;i<8;i++) digitalWrite(segmentPins[i], bitRead(M[t],i));
  digitalWrite(dig,HIGH); delay(5); digitalWrite(dig,LOW);
  for(int i=0;i<8;i++) digitalWrite(segmentPins[i], bitRead(M[o],i));
  digitalWrite(dig,HIGH); delay(5); digitalWrite(dig,LOW);
}

void playTune(){
  int notes[]={262,294,330,349,392,440,494,523};
  for(int i=0;i<8;i++){
    tone(buzzerPin,notes[i],200);
    delay(250);
  }
  noTone(buzzerPin);
}
