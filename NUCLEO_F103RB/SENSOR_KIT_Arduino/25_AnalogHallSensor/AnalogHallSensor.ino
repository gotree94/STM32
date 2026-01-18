/*
 * 아날로그 홀센서 모듈 테스트 (KY-035 Analog Hall Sensor)
 * 보드: NUCLEO-F103RB (STM32F103RBT6)
 * 환경: Arduino IDE with STM32duino
 * 
 * 홀센서: 자기장의 세기를 아날로그 전압으로 출력
 * 
 * 핀 연결:
 *   - VCC: 3.3V 또는 5V
 *   - GND: GND
 *   - Signal: PA0 (A0)
 */

#define HALL_PIN PA0         // 홀센서 아날로그 핀
#define LED_PIN LED_BUILTIN  // 내장 LED (PA5)

// ADC 해상도
const int ADC_MAX = 4095;     // 12-bit ADC
const int ADC_MID = 2048;     // 중간값 (자기장 없을 때)

// 이동 평균 필터
const int FILTER_SIZE = 20;
int readings[FILTER_SIZE];
int readIndex = 0;
long total = 0;
int average = 0;

// 보정값 (자기장 없을 때의 기준값)
int zeroPoint = ADC_MID;
bool isCalibrated = false;

// 측정값
int rawValue = 0;
int magneticField = 0;  // 상대적 자기장 세기 (-100 ~ +100)
String polarity = "없음";

// 임계값
const int DETECT_THRESHOLD = 100;  // 자기장 감지 임계값 (ADC 단위)
const int STRONG_THRESHOLD = 500;  // 강한 자기장 임계값

// 모드
enum Mode {
  MODE_BASIC,      // 기본 측정
  MODE_MONITOR,    // 연속 모니터링
  MODE_DETECT,     // 자석 감지 (근접 센서처럼)
  MODE_GAUSS       // 가우스 미터 모드
};

Mode currentMode = MODE_BASIC;

// RPM 측정용 (자석 부착 회전체)
bool magnetNear = false;
bool lastMagnetNear = false;
unsigned long lastPulseTime = 0;
unsigned long pulseInterval = 0;
float rpm = 0;
int pulseCount = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  
  pinMode(HALL_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // 필터 초기화
  for (int i = 0; i < FILTER_SIZE; i++) {
    readings[i] = ADC_MID;
    total += ADC_MID;
  }
  average = ADC_MID;
  
  Serial.println("================================");
  Serial.println("아날로그 홀센서 모듈 테스트");
  Serial.println("NUCLEO-F103RB");
  Serial.println("================================");
  Serial.println();
  Serial.println("명령어:");
  Serial.println("  1: 기본 모드 (수동 측정)");
  Serial.println("  2: 모니터링 모드 (연속 출력)");
  Serial.println("  3: 자석 감지 모드");
  Serial.println("  4: 가우스 미터 모드");
  Serial.println("  m: 현재 자기장 측정");
  Serial.println("  c: 영점 보정 (자석 없이 실행)");
  Serial.println("  r: RPM 측정 시작/정지");
  Serial.println();
  Serial.println("자석을 센서에 가까이 대보세요!");
  Serial.println("(N극/S극에 따라 값이 다르게 나옵니다)");
  Serial.println();
}

void loop() {
  // 시리얼 명령 처리
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    processCommand(cmd);
  }
  
  // 홀센서 값 읽기
  readHallSensor();
  
  // 모드별 처리
  switch (currentMode) {
    case MODE_BASIC:
      // 수동 측정 모드
      break;
      
    case MODE_MONITOR:
      monitorMode();
      break;
      
    case MODE_DETECT:
      detectMode();
      break;
      
    case MODE_GAUSS:
      gaussMode();
      break;
  }
  
  delay(10);
}

void readHallSensor() {
  // 원시값 읽기
  rawValue = analogRead(HALL_PIN);
  
  // 이동 평균 필터
  total = total - readings[readIndex];
  readings[readIndex] = rawValue;
  total = total + readings[readIndex];
  readIndex = (readIndex + 1) % FILTER_SIZE;
  average = total / FILTER_SIZE;
  
  // 자기장 세기 계산 (기준점 대비)
  int deviation = average - zeroPoint;
  
  // -100 ~ +100 스케일로 변환
  magneticField = map(deviation, -2048, 2048, -100, 100);
  magneticField = constrain(magneticField, -100, 100);
  
  // 극성 판단
  if (abs(deviation) < DETECT_THRESHOLD) {
    polarity = "감지 안됨";
  } else if (deviation > 0) {
    polarity = "S극 (또는 N극 반대면)";
  } else {
    polarity = "N극 (또는 S극 반대면)";
  }
  
  // 자석 근접 여부
  lastMagnetNear = magnetNear;
  magnetNear = (abs(deviation) > DETECT_THRESHOLD);
}

void processCommand(char cmd) {
  switch (cmd) {
    case '1':
      currentMode = MODE_BASIC;
      Serial.println("[모드] 기본 모드");
      Serial.println("       'm'을 입력하여 측정하세요.");
      break;
      
    case '2':
      currentMode = MODE_MONITOR;
      Serial.println("[모드] 모니터링 모드");
      break;
      
    case '3':
      currentMode = MODE_DETECT;
      Serial.println("[모드] 자석 감지 모드");
      Serial.println("       자석 접근/이탈을 감지합니다.");
      break;
      
    case '4':
      currentMode = MODE_GAUSS;
      Serial.println("[모드] 가우스 미터 모드");
      Serial.println("       상대적 자기장 세기를 표시합니다.");
      break;
      
    case 'm':
    case 'M':
      printMeasurement();
      break;
      
    case 'c':
    case 'C':
      calibrate();
      break;
      
    case 'r':
    case 'R':
      toggleRpmMode();
      break;
      
    default:
      if (cmd != '\n' && cmd != '\r') {
        Serial.println("알 수 없는 명령입니다.");
      }
      break;
  }
}

void printMeasurement() {
  Serial.println();
  Serial.println("======== 자기장 측정 결과 ========");
  Serial.print("Raw 값: ");
  Serial.print(average);
  Serial.print(" (기준: ");
  Serial.print(zeroPoint);
  Serial.println(")");
  Serial.print("편차: ");
  Serial.println(average - zeroPoint);
  Serial.print("자기장 세기: ");
  Serial.print(magneticField);
  Serial.println("% (상대값)");
  Serial.print("극성: ");
  Serial.println(polarity);
  
  // 강도 표시
  Serial.print("강도: ");
  int strength = abs(average - zeroPoint);
  if (strength < DETECT_THRESHOLD) {
    Serial.println("없음");
  } else if (strength < STRONG_THRESHOLD) {
    Serial.println("약함");
  } else if (strength < STRONG_THRESHOLD * 2) {
    Serial.println("보통");
  } else {
    Serial.println("강함");
  }
  Serial.println("==================================");
  Serial.println();
}

void calibrate() {
  Serial.println("[보정] 영점 보정 중...");
  Serial.println("       센서 주변에 자석이 없는지 확인하세요.");
  
  delay(500);
  
  // 여러 번 읽어서 평균 계산
  long sum = 0;
  for (int i = 0; i < 100; i++) {
    sum += analogRead(HALL_PIN);
    delay(10);
  }
  zeroPoint = sum / 100;
  isCalibrated = true;
  
  Serial.print("[보정] 완료! 새 기준값: ");
  Serial.println(zeroPoint);
}

void monitorMode() {
  static unsigned long lastPrintTime = 0;
  
  if (millis() - lastPrintTime >= 200) {
    lastPrintTime = millis();
    
    Serial.print("[홀센서] ");
    
    // 자기장 방향과 세기를 막대로 표시
    printFieldBar(magneticField);
    
    Serial.print(" ");
    Serial.print(magneticField);
    Serial.print("% | ");
    Serial.println(polarity);
  }
}

void detectMode() {
  // LED 표시
  digitalWrite(LED_PIN, magnetNear ? HIGH : LOW);
  
  // 상태 변화 감지
  if (magnetNear && !lastMagnetNear) {
    Serial.print("🧲 [감지] 자석 접근! | 극성: ");
    Serial.println(polarity);
    
    // RPM 측정용 펄스 카운트
    unsigned long currentTime = millis();
    pulseInterval = currentTime - lastPulseTime;
    lastPulseTime = currentTime;
    pulseCount++;
    
    if (pulseInterval > 0 && pulseInterval < 5000) {
      rpm = 60000.0 / pulseInterval;
      Serial.print("       RPM: ");
      Serial.println(rpm, 1);
    }
  }
  
  if (!magnetNear && lastMagnetNear) {
    Serial.println("    [감지] 자석 이탈");
  }
}

void gaussMode() {
  static unsigned long lastPrintTime = 0;
  
  if (millis() - lastPrintTime >= 300) {
    lastPrintTime = millis();
    
    // 간이 가우스 표시 (실제 가우스 아님, 상대값)
    int pseudoGauss = map(abs(average - zeroPoint), 0, 2048, 0, 1000);
    
    Serial.print("[가우스] ~");
    Serial.print(pseudoGauss);
    Serial.print(" mG (상대값) | ");
    
    // 강도 바
    int bars = pseudoGauss / 50;
    Serial.print("[");
    for (int i = 0; i < 20; i++) {
      Serial.print(i < bars ? "█" : "░");
    }
    Serial.print("] ");
    Serial.println(polarity);
  }
}

void printFieldBar(int field) {
  // -100 ~ +100을 시각적으로 표시
  // [-----N|S+++++]
  
  Serial.print("[");
  
  int center = 10;
  int pos = map(field, -100, 100, 0, 20);
  
  for (int i = 0; i < 20; i++) {
    if (i == center) {
      Serial.print("|");  // 중앙선
    } else if (i < center) {
      Serial.print(i >= pos && pos < center ? "◀" : "-");
    } else {
      Serial.print(i <= pos && pos > center ? "▶" : "-");
    }
  }
  
  Serial.print("]");
}

void toggleRpmMode() {
  static bool rpmActive = false;
  rpmActive = !rpmActive;
  
  if (rpmActive) {
    pulseCount = 0;
    lastPulseTime = millis();
    Serial.println("[RPM] 측정 시작");
    Serial.println("      회전체에 자석을 부착하고 센서 근처에서 회전시키세요.");
  } else {
    Serial.println("[RPM] 측정 종료");
    Serial.print("      총 펄스: ");
    Serial.println(pulseCount);
  }
}
