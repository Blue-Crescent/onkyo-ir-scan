#include <TimerOne.h>
#include <Arduino.h>
#include <TM1637.h>
// #include <avr/io.h>
// #include <avr/interrupt.h>

#define TX_PIN 7  // 出力ピン
#define DIO 2
#define CLK 4

#define MAX_PULSES 32 // ヘッダ2 + データ12*2 + トレーラ2 = 28

volatile uint8_t pulseDurations[MAX_PULSES];
volatile uint8_t  pulseLevels[MAX_PULSES];
volatile uint8_t  pulseCount = 0;
volatile uint8_t  pulseIndex = 0;
volatile bool     sending = false;

TM1637 led(CLK, DIO);


void enqueuePulse(uint8_t level, uint16_t dur_ms) {
  if (pulseCount >= MAX_PULSES) return;
  pulseLevels[pulseCount] = level == HIGH ? LOW : HIGH; // GPIOが反転出力
  pulseDurations[pulseCount] = dur_ms; // µsに変換
  pulseCount++;
}

void prepareHeader() {
  enqueuePulse(HIGH, 3);  // 3ms High
  enqueuePulse(LOW, 1);   // 1ms Low
}

void prepareData(uint16_t data) {
  for (int i = 11; i >= 0; i--) {
  // for (int i = 15; i >= 0; i--) {
    enqueuePulse(HIGH, 1); // 共通High 1ms
    bool bit = (data >> i) & 1;
    enqueuePulse(LOW, bit ? 2 : 1); // 1=Low2ms, 0=Low1ms
  }
}

void prepareTrailer() {
  enqueuePulse(HIGH, 1);   // 1ms High
  enqueuePulse(LOW, 20);   // 20ms Low
}

void prepareAll(uint16_t data) {
  prepareHeader();
  prepareData(data);
  prepareTrailer();
}

void pulseHandler() {
  if (!sending) return;

  digitalWrite(TX_PIN, pulseLevels[pulseIndex]);
  pulseDurations[pulseIndex]--;
  if(pulseDurations[pulseIndex] != 0) return;

  pulseIndex++;
  if (pulseIndex >= pulseCount) {
    sending = false;
    return;
  }

}

void startSend() {
  pulseIndex = 0;
  sending = true;
}

// ★ 簡単送信用関数（これを呼ぶだけで送信開始）
void sendCode(uint16_t data) {
  if (sending) return; // 送信中は無視
  pulseCount= 0;
  pulseIndex = 0;
  char text[4];
  sprintf(text, "%04x",data);
  led.display(String(text));
  prepareAll(data);
  startSend();
}

void setup() {
  Timer1.initialize(1000); // 仮初期化
  Timer1.setPeriod(1000);
  Timer1.attachInterrupt(pulseHandler);

  pinMode(TX_PIN, OUTPUT);
  led.begin();
  digitalWrite(TX_PIN, LOW);
 
  sendCode(0x7F); // Power ON
  delay(1000);
  sendCode(0x7F); // Power ON
  delay(5000);
  //digitalWrite(TX_PIN, LOW); // Output High
  //delay(5000);
  //digitalWrite(TX_PIN, HIGH); // OUtput Low
  //delay(2000);
}

void loop() {
  // 送信完了後に別のデータを送信
  if (!sending) {
    for (uint16_t code = 0x0; code <=0xFFF; code=code+0x1){
      delay(500);
      sendCode(code); // Power ON
    }
  }
  //if (!sending) {
  //  for (uint16_t count = 100; count >0; count--){
  //    sendCode(0x1A3);
  //    delay(1000);
  //  }
  //}
  // if (!sending) {
  //   for (uint16_t count = 5; count >0; count--){
  //     sendCode(0x74);
  //     delay(1000);
  //   }
  // }
  // if (!sending) {
  //   for (uint16_t count = 5; count >0; count--){
  //     sendCode(0x73);
  //     delay(1000);
  //   }
  // }
  // if (!sending) {
  //   for (uint16_t code = 0x200; code >=0x00; code--){
  //     sendCode(code);
  //     delay(1000);
  //   }
  // }
  //delay(5000);
  //sendCode(0x70); // Power ON
  //delay(5000);
  //sendCode(0x7F); // Power ON
  //delay(5000);
  //sendCode(0x73); // Power ON
  //delay(5000);
  //sendCode(0x79); // Power ON
  //delay(5000);
  //// sendCode(0x73); // Power ON
  //// delay(5000);
  //if (!sending) {
  //  for (uint16_t code = 0x73; code <=0x90; code++){
  //    sendCode(code); // Power ON
  //    delay(10000);
  //  }
  //}
  // if (!sending) {
  //   delay(2000);
  //   sendCode(0x020); // Power ON
  //   // sendCode(0x1AE); // Power OFF
  // }
  // if (!sending) {
  //   delay(2000);
  //   sendCode(0x300); // Power ON
  //   // sendCode(0x1A2); // Volume Up
  // }
  // if (!sending) {
  //   delay(2000);
  //   sendCode(0x000); // Power ON
  //   // sendCode(0x1A3); // Volume Down
  // }
  delay(2000);
}