/*
 * 심박 센서 모듈 테스트 (Pulse Sensor / Heart Rate Sensor)
 * 보드: NUCLEO-F103RB (STM32F103RBT6)
 * 환경: Arduino IDE with STM32duino
 * 
 * 핀 연결:
 *   - VCC: 3.3V
 *   - GND: GND
 *   - Signal: PA1 (A1)
 */

#define PULSE_PIN PA1        // 심박 센서 아날로그 핀
#define LED_PIN LED_BUILTIN  // 내장 LED (PA5)

// 심박수 측정 변수
int pulseValue = 0;
int threshold = 550;         // 심박 감지 임계값 (센서에 따라 조정 필요)
bool pulseDetected = false;
unsigned long lastBeatTime = 0;
unsigned long beatInterval = 0;
int bpm = 0;

// 이동 평균 필터
const int FILTER_SIZE = 10;
int readings[FILTER_SIZE];
int readIndex = 0;
int total = 0;
int average = 0;

// BPM 계산용
const int BPM_BUFFER_SIZE = 5;
int bpmBuffer[BPM_BUFFER_SIZE];
int bpmIndex = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  
  pinMode(PULSE_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // 필터 배열 초기화
  for (int i = 0; i < FILTER_SIZE; i++) {
    readings[i] = 0;
  }
  for (int i = 0; i < BPM_BUFFER_SIZE; i++) {
    bpmBuffer[i] = 0;
  }
  
  Serial.println("================================");
  Serial.println("심박 센서 모듈 테스트");
  Serial.println("NUCLEO-F103RB");
  Serial.println("================================");
  Serial.println("손가락을 센서에 가볍게 올려주세요.");
  Serial.println("너무 세게 누르지 마세요.");
  Serial.println();
  
  delay(1000);
}

void loop() {
  // 아날로그 값 읽기
  pulseValue = analogRead(PULSE_PIN);
  
  // 이동 평균 필터 적용
  total = total - readings[readIndex];
  readings[readIndex] = pulseValue;
  total = total + readings[readIndex];
  readIndex = (readIndex + 1) % FILTER_SIZE;
  average = total / FILTER_SIZE;
  
  // 심박 감지
  if (average > threshold && !pulseDetected) {
    pulseDetected = true;
    digitalWrite(LED_PIN, HIGH);
    
    unsigned long currentTime = millis();
    beatInterval = currentTime - lastBeatTime;
    lastBeatTime = currentTime;
    
    // BPM 계산 (60000ms / 박동 간격)
    if (beatInterval > 300 && beatInterval < 2000) {  // 유효 범위: 30~200 BPM
      int currentBPM = 60000 / beatInterval;
      
      // BPM 버퍼에 저장
      bpmBuffer[bpmIndex] = currentBPM;
      bpmIndex = (bpmIndex + 1) % BPM_BUFFER_SIZE;
      
      // 평균 BPM 계산
      int validCount = 0;
      int bpmSum = 0;
      for (int i = 0; i < BPM_BUFFER_SIZE; i++) {
        if (bpmBuffer[i] > 0) {
          bpmSum += bpmBuffer[i];
          validCount++;
        }
      }
      if (validCount > 0) {
        bpm = bpmSum / validCount;
      }
      
      Serial.print("♥ 심박 감지! | Raw: ");
      Serial.print(pulseValue);
      Serial.print(" | Filtered: ");
      Serial.print(average);
      Serial.print(" | BPM: ");
      Serial.println(bpm);
    }
  }
  
  if (average < threshold - 50) {  // 히스테리시스
    pulseDetected = false;
    digitalWrite(LED_PIN, LOW);
  }
  
  // 시리얼 플로터용 데이터 출력 (주석 해제하여 사용)
  // Serial.print(pulseValue);
  // Serial.print(",");
  // Serial.print(average);
  // Serial.print(",");
  // Serial.println(threshold);
  
  delay(10);  // 약 100Hz 샘플링
}

// 동적 임계값 조정 함수 (필요시 사용)
void adjustThreshold() {
  int minVal = 4095;
  int maxVal = 0;
  
  Serial.println("임계값 보정 중... 5초간 손가락을 올려주세요.");
  
  for (int i = 0; i < 500; i++) {
    int val = analogRead(PULSE_PIN);
    if (val < minVal) minVal = val;
    if (val > maxVal) maxVal = val;
    delay(10);
  }
  
  threshold = (minVal + maxVal) / 2;
  Serial.print("새 임계값: ");
  Serial.println(threshold);
}
