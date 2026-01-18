/*
 * 조도 센서 모듈 테스트 (KY-018 Photoresistor / LDR Module)
 * 보드: NUCLEO-F103RB (STM32F103RBT6)
 * 환경: Arduino IDE with STM32duino
 * 
 * 핀 연결:
 *   - VCC: 3.3V
 *   - GND: GND
 *   - Signal: PA0 (A0)
 */

#define LDR_PIN PA0          // 조도 센서 아날로그 핀
#define LED_PIN LED_BUILTIN  // 내장 LED (PA5)

// ADC 해상도 (STM32F103: 12bit = 4095)
const int ADC_MAX = 4095;

// 조도 레벨 임계값
const int LEVEL_DARK = 500;       // 어두움
const int LEVEL_DIM = 1500;       // 약간 어두움
const int LEVEL_NORMAL = 2500;    // 보통
const int LEVEL_BRIGHT = 3500;    // 밝음
// 3500 이상: 매우 밝음

// 측정값 저장
int rawValue = 0;
int lightPercent = 0;
String lightLevel = "";

// 이동 평균 필터
const int FILTER_SIZE = 10;
int readings[FILTER_SIZE];
int readIndex = 0;
int total = 0;
int average = 0;

// 자동 밝기 조절용
int ledBrightness = 0;
bool autoLedMode = false;

// 이벤트 감지
int lastLightPercent = 0;
const int CHANGE_THRESHOLD = 10;  // 10% 이상 변화 시 이벤트
unsigned long lastEventTime = 0;

// 모드
enum Mode {
  MODE_BASIC,      // 기본 측정
  MODE_MONITOR,    // 연속 모니터링
  MODE_AUTO_LED,   // 자동 LED 밝기
  MODE_THRESHOLD   // 임계값 알림
};

Mode currentMode = MODE_BASIC;

// 임계값 모드용
int thresholdValue = 50;  // 기본 임계값 50%

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  
  pinMode(LDR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // 필터 초기화
  for (int i = 0; i < FILTER_SIZE; i++) {
    readings[i] = 0;
  }
  
  Serial.println("================================");
  Serial.println("조도 센서 모듈 테스트");
  Serial.println("NUCLEO-F103RB");
  Serial.println("================================");
  Serial.println();
  Serial.println("명령어:");
  Serial.println("  1: 기본 모드 (수동 측정)");
  Serial.println("  2: 모니터링 모드 (연속 출력)");
  Serial.println("  3: 자동 LED 모드");
  Serial.println("  4: 임계값 알림 모드");
  Serial.println("  m: 현재 조도 측정");
  Serial.println("  t: 임계값 설정 (예: t50)");
  Serial.println();
}

void loop() {
  // 시리얼 명령 처리
  if (Serial.available() > 0) {
    processSerialInput();
  }
  
  // 조도 읽기 및 필터링
  readLightSensor();
  
  // 모드별 처리
  switch (currentMode) {
    case MODE_BASIC:
      // 수동 측정 모드 - 명령 시에만 출력
      break;
      
    case MODE_MONITOR:
      monitorMode();
      break;
      
    case MODE_AUTO_LED:
      autoLedMode_func();
      break;
      
    case MODE_THRESHOLD:
      thresholdMode();
      break;
  }
  
  // 급격한 변화 감지 (모든 모드에서)
  detectSuddenChange();
  
  delay(50);
}

void readLightSensor() {
  // 원시 값 읽기
  rawValue = analogRead(LDR_PIN);
  
  // 이동 평균 필터 적용
  total = total - readings[readIndex];
  readings[readIndex] = rawValue;
  total = total + readings[readIndex];
  readIndex = (readIndex + 1) % FILTER_SIZE;
  average = total / FILTER_SIZE;
  
  // 퍼센트 변환 (0-100%)
  lightPercent = map(average, 0, ADC_MAX, 0, 100);
  
  // 조도 레벨 문자열
  if (average < LEVEL_DARK) {
    lightLevel = "매우 어두움 🌑";
  } else if (average < LEVEL_DIM) {
    lightLevel = "어두움 🌒";
  } else if (average < LEVEL_NORMAL) {
    lightLevel = "보통 🌓";
  } else if (average < LEVEL_BRIGHT) {
    lightLevel = "밝음 🌔";
  } else {
    lightLevel = "매우 밝음 🌕";
  }
}

void processSerialInput() {
  String input = Serial.readStringUntil('\n');
  input.trim();
  
  if (input.length() == 0) return;
  
  char cmd = input.charAt(0);
  
  switch (cmd) {
    case '1':
      currentMode = MODE_BASIC;
      Serial.println("[모드] 기본 모드");
      Serial.println("       'm'을 입력하여 측정하세요.");
      break;
      
    case '2':
      currentMode = MODE_MONITOR;
      Serial.println("[모드] 모니터링 모드");
      Serial.println("       500ms 간격으로 측정합니다.");
      break;
      
    case '3':
      currentMode = MODE_AUTO_LED;
      Serial.println("[모드] 자동 LED 모드");
      Serial.println("       어두우면 LED 밝아지고, 밝으면 LED 어두워집니다.");
      break;
      
    case '4':
      currentMode = MODE_THRESHOLD;
      Serial.print("[모드] 임계값 알림 모드 (현재: ");
      Serial.print(thresholdValue);
      Serial.println("%)");
      break;
      
    case 'm':
    case 'M':
      printMeasurement();
      break;
      
    case 't':
    case 'T':
      if (input.length() > 1) {
        int newThreshold = input.substring(1).toInt();
        if (newThreshold >= 0 && newThreshold <= 100) {
          thresholdValue = newThreshold;
          Serial.print("[설정] 임계값: ");
          Serial.print(thresholdValue);
          Serial.println("%");
        } else {
          Serial.println("임계값은 0-100 사이로 설정하세요.");
        }
      }
      break;
      
    default:
      Serial.println("알 수 없는 명령입니다.");
      break;
  }
}

void printMeasurement() {
  Serial.println();
  Serial.println("======== 조도 측정 결과 ========");
  Serial.print("Raw 값: ");
  Serial.print(average);
  Serial.print(" / ");
  Serial.println(ADC_MAX);
  Serial.print("밝기: ");
  Serial.print(lightPercent);
  Serial.println("%");
  Serial.print("상태: ");
  Serial.println(lightLevel);
  Serial.println("================================");
  Serial.println();
}

void monitorMode() {
  static unsigned long lastPrintTime = 0;
  
  if (millis() - lastPrintTime >= 500) {
    lastPrintTime = millis();
    
    // 막대 그래프 출력
    Serial.print("[조도] ");
    Serial.print(lightPercent);
    Serial.print("% ");
    printBar(lightPercent);
    Serial.print(" ");
    Serial.println(lightLevel);
  }
}

void autoLedMode_func() {
  // 어두우면 LED 밝게, 밝으면 LED 어둡게 (역비례)
  ledBrightness = map(average, 0, ADC_MAX, 255, 0);
  ledBrightness = constrain(ledBrightness, 0, 255);
  
  analogWrite(LED_PIN, ledBrightness);
  
  static unsigned long lastPrintTime = 0;
  if (millis() - lastPrintTime >= 1000) {
    lastPrintTime = millis();
    Serial.print("[자동LED] 조도: ");
    Serial.print(lightPercent);
    Serial.print("% → LED: ");
    Serial.print(map(ledBrightness, 0, 255, 0, 100));
    Serial.println("%");
  }
}

void thresholdMode() {
  static bool wasAboveThreshold = false;
  bool isAboveThreshold = (lightPercent >= thresholdValue);
  
  // 상태 변화 감지
  if (isAboveThreshold != wasAboveThreshold) {
    if (isAboveThreshold) {
      Serial.print("💡 [알림] 밝기가 임계값(");
      Serial.print(thresholdValue);
      Serial.println("%)을 초과했습니다!");
      digitalWrite(LED_PIN, HIGH);
    } else {
      Serial.print("🌙 [알림] 밝기가 임계값(");
      Serial.print(thresholdValue);
      Serial.println("%) 아래로 떨어졌습니다!");
      digitalWrite(LED_PIN, LOW);
    }
    wasAboveThreshold = isAboveThreshold;
  }
}

void detectSuddenChange() {
  int change = abs(lightPercent - lastLightPercent);
  
  if (change >= CHANGE_THRESHOLD && (millis() - lastEventTime) > 500) {
    Serial.print("⚡ [이벤트] 급격한 조도 변화 감지! (");
    Serial.print(lastLightPercent);
    Serial.print("% → ");
    Serial.print(lightPercent);
    Serial.println("%)");
    lastEventTime = millis();
  }
  
  lastLightPercent = lightPercent;
}

void printBar(int percent) {
  int barLength = percent / 5;  // 0-20 길이
  Serial.print("[");
  for (int i = 0; i < 20; i++) {
    if (i < barLength) {
      Serial.print("█");
    } else {
      Serial.print("░");
    }
  }
  Serial.print("]");
}
