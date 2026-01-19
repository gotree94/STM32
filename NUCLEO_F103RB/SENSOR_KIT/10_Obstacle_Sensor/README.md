# IR-08H Obstacle Detection Sensor Module - STM32F103 NUCLEO

IR-08H 적외선 장애물 감지센서 모듈을 STM32F103 NUCLEO 보드로 테스트하는 예제입니다.

## 하드웨어 구성

### 부품 목록
| 부품 | 수량 | 비고 |
|------|------|------|
| STM32F103 NUCLEO | 1 | NUCLEO-F103RB |
| 장애물 감지센서 모듈 | 1~2 | IR-08H (4핀) |
| 점퍼 와이어 | 6+ | M-F 타입 |

### 센서 모듈 사양 (IR-08H)
- 모델명: IR-08H
- 센서 타입: 적외선(IR) 반사 센서
- 동작 전압: 3.3V ~ 5V
- 감지 거리: 2cm ~ 30cm (가변저항으로 조절)
- 출력: 디지털 (Active Low)
- 감지 각도: 약 35°
- 핀 구성: EM, VCC, OUT, GND (4핀)

### 핀 설명
| 센서 핀 | 기능 | 설명 |
|---------|------|------|
| EM | Enable | 센서 활성화 (LOW: 활성화, HIGH: 비활성화) |
| VCC | Power | 전원 입력 (3.3V ~ 5V) |
| OUT | Output | 감지 출력 (장애물 감지 시 LOW) |
| GND | Ground | 접지 |

### 핀 연결

```
장애물 감지센서        STM32F103 NUCLEO
-----------------------------------------
EM       ────────►  GND (CN7-20) - 항상 활성화
                    또는 PA4 (CN7-32) - 제어용
VCC      ────────►  3.3V (CN7-16)
OUT      ────────►  PA0 (CN7-28) - Sensor 1
GND      ────────►  GND (CN7-20)

(2채널 구성 시)
EM       ────────►  GND 또는 PA6 (CN10-13)
VCC      ────────►  3.3V
OUT      ────────►  PA1 (CN7-30) - Sensor 2
GND      ────────►  GND
```

### 회로도

```
                     ┌───────────────────┐
                     │      IR-08H       │
                     │     [POT]         │ ← 감지거리 조절
    GND/PA4 ───────► │ EM               │ ← Enable (LOW=활성화)
    3.3V ──────────► │ VCC              │
    PA0 ◄────────────┤ OUT              │
    GND  ──────────► │ GND              │
                     │    ◯    ◯        │
                     │   TX    RX       │ ← IR LED + Receiver
                     └───────────────────┘

    (2채널 구성 시 PA1에 두 번째 센서 연결)
                     
    PA5 ──────────► LED (장애물 감지 시 ON)
```

### EM 핀 사용 방법

**방법 1: 항상 활성화 (기본)**
```
EM 핀을 GND에 직접 연결 → 센서 항상 동작
```

**방법 2: GPIO 제어 (전력 절약/선택적 활성화)**
```c
// PA4를 EM 제어용으로 사용
#define SENSOR1_EM_PIN    GPIO_PIN_4
#define SENSOR1_EM_PORT   GPIOA

// 센서 활성화
HAL_GPIO_WritePin(SENSOR1_EM_PORT, SENSOR1_EM_PIN, GPIO_PIN_RESET);  // LOW

// 센서 비활성화 (전력 절약)
HAL_GPIO_WritePin(SENSOR1_EM_PORT, SENSOR1_EM_PIN, GPIO_PIN_SET);    // HIGH
```

## 센서 동작 원리

```
      IR LED              IR Receiver
         │                    │
         ▼                    │
    ┌────────┐               │
    │ 송신광 │ ─────────┐    │
    └────────┘          │    │
                        ▼    │
                   ┌────────┐ │
                   │ 장애물 │ │
                   └────────┘ │
                        │     │
                        │ 반사│
                        └────►│
                              ▼
                         감지됨 (LOW 출력)

    EM = LOW (활성화 상태)
    장애물 없음: HIGH 출력
    장애물 있음: LOW 출력 (Active Low)
    
    EM = HIGH (비활성화 상태)
    출력 없음 (전력 절약 모드)
```

## 기능 설명

1. **2채널 장애물 감지**
   - 왼쪽/오른쪽 센서 독립 감지
   - 로봇 좌우 장애물 판단 가능

2. **상태 분류**
   - `CLEAR`: 장애물 없음
   - `LEFT_BLOCKED`: 왼쪽 장애물
   - `RIGHT_BLOCKED`: 오른쪽 장애물
   - `BOTH_BLOCKED`: 양쪽 모두 장애물

3. **이벤트 기반 알림**
   - 상태 변경 시 방향 제안
   - 감지 지속 시간 표시

4. **통계 기능**
   - 총 감지 횟수 카운트

5. **전력 관리 (EM 핀 활용)**
   - 필요시에만 센서 활성화
   - 배터리 구동 시 유용

## 빌드 및 업로드

### STM32CubeIDE 사용 시
1. 새 프로젝트 생성 (STM32F103RB 선택)
2. PA0, PA1을 GPIO_Input으로 설정
3. (옵션) PA4, PA6을 GPIO_Output으로 설정 (EM 제어용)
4. `main.c` 내용을 프로젝트에 복사
5. 빌드 후 업로드

### CubeMX 설정
```
Pinout:
- PA0: GPIO_Input (Pull-up) - Sensor 1 OUT
- PA1: GPIO_Input (Pull-up) - Sensor 2 OUT
- PA4: GPIO_Output (옵션)   - Sensor 1 EM
- PA6: GPIO_Output (옵션)   - Sensor 2 EM
- PA5: GPIO_Output          - LED
```

## 시리얼 출력 예시

```
============================================
  Obstacle Detection Sensor Test
  STM32F103 NUCLEO
============================================
PA0: Obstacle Sensor 1 (Left/Front)
PA1: Obstacle Sensor 2 (Right, Optional)
EM pins directly connected to GND (always enabled)
Detection Range: 2cm ~ 30cm (adjustable)
(0=Obstacle Detected, 1=Clear)

Sensors: [S1:O|S2:O] | State: CLEAR
Sensors: [S1:X|S2:O] | State: LEFT BLOCKED   | Duration:  120ms
>>> EVENT: Obstacle on LEFT! Suggest turn RIGHT
Sensors: [S1:X|S2:X] | State: BOTH BLOCKED   | Duration:  450ms
>>> EVENT: BLOCKED! Suggest STOP or REVERSE
Sensors: [S1:O|S2:O] | State: CLEAR
>>> EVENT: Path cleared! Total detections: 2
```

## 감지 거리 조절

### 가변저항 조절 방법
1. 작은 드라이버로 센서 모듈의 POT 조절
2. **시계 방향**: 감지 거리 증가 (최대 ~30cm)
3. **반시계 방향**: 감지 거리 감소 (최소 ~2cm)

### 최적 설정
| 용도 | 권장 거리 | 조절 방향 |
|------|-----------|-----------|
| 근접 감지 | 2-5cm | 반시계 |
| 로봇 충돌 방지 | 10-15cm | 중간 |
| 원거리 감지 | 20-30cm | 시계 |

## 응용 예제

### EM 핀을 활용한 전력 관리
```c
typedef struct {
    GPIO_TypeDef *out_port;
    uint16_t out_pin;
    GPIO_TypeDef *em_port;
    uint16_t em_pin;
    uint8_t enabled;
    uint8_t state;
} ObstacleSensorEx;

ObstacleSensorEx sensor1 = {
    .out_port = GPIOA, .out_pin = GPIO_PIN_0,
    .em_port = GPIOA, .em_pin = GPIO_PIN_4,
    .enabled = 0, .state = 1
};

void Sensor_Enable(ObstacleSensorEx *sensor)
{
    HAL_GPIO_WritePin(sensor->em_port, sensor->em_pin, GPIO_PIN_RESET);
    sensor->enabled = 1;
    HAL_Delay(1);  // 안정화 대기
}

void Sensor_Disable(ObstacleSensorEx *sensor)
{
    HAL_GPIO_WritePin(sensor->em_port, sensor->em_pin, GPIO_PIN_SET);
    sensor->enabled = 0;
}

uint8_t Sensor_Read(ObstacleSensorEx *sensor)
{
    if (!sensor->enabled)
    {
        Sensor_Enable(sensor);
    }
    sensor->state = HAL_GPIO_ReadPin(sensor->out_port, sensor->out_pin);
    return sensor->state;
}

// 주기적 스캔으로 전력 절약
void Power_Saving_Scan(void)
{
    Sensor_Enable(&sensor1);
    HAL_Delay(2);
    uint8_t result = Sensor_Read(&sensor1);
    Sensor_Disable(&sensor1);
    
    // 100ms 중 2ms만 센서 동작 → 약 98% 전력 절약
    HAL_Delay(98);
}
```

### 로봇 장애물 회피
```c
typedef enum {
    MOVE_FORWARD,
    TURN_LEFT,
    TURN_RIGHT,
    MOVE_BACKWARD,
    STOP
} RobotAction;

RobotAction Decide_Action(ObstacleState state)
{
    static ObstacleState last_state = STATE_CLEAR;
    static uint32_t blocked_time = 0;
    
    switch (state)
    {
        case STATE_CLEAR:
            blocked_time = 0;
            return MOVE_FORWARD;
            
        case STATE_LEFT_BLOCKED:
            return TURN_RIGHT;
            
        case STATE_RIGHT_BLOCKED:
            return TURN_LEFT;
            
        case STATE_BOTH_BLOCKED:
            if (blocked_time == 0)
                blocked_time = HAL_GetTick();
            
            // 2초 이상 막혀있으면 후진
            if (HAL_GetTick() - blocked_time > 2000)
                return MOVE_BACKWARD;
            else
                return STOP;
            
        default:
            return STOP;
    }
}
```

### 다중 센서 배열 (4채널)
```c
#define NUM_SENSORS 4

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    uint8_t state;
} ObstacleSensor;

ObstacleSensor sensors[NUM_SENSORS] = {
    {GPIOA, GPIO_PIN_0, 1},  // Far Left
    {GPIOA, GPIO_PIN_1, 1},  // Left
    {GPIOA, GPIO_PIN_4, 1},  // Right
    {GPIOB, GPIO_PIN_0, 1},  // Far Right
};

void Read_All_Sensors(void)
{
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        sensors[i].state = HAL_GPIO_ReadPin(
            sensors[i].port, 
            sensors[i].pin
        );
    }
}

int Calculate_Steering(void)
{
    // -100 (좌회전) ~ +100 (우회전)
    int weights[] = {-100, -50, 50, 100};
    int sum = 0;
    int active = 0;
    
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        if (sensors[i].state == OBSTACLE_DETECTED)
        {
            sum += weights[i];
            active++;
        }
    }
    
    if (active == 0) return 0;  // 직진
    if (active == NUM_SENSORS) return 999;  // 막힘
    
    return sum / active;
}
```

### 인터럽트 기반 감지
```c
volatile uint8_t obstacle_flag = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == OBSTACLE_1_PIN || GPIO_Pin == OBSTACLE_2_PIN)
    {
        obstacle_flag = 1;
        // 긴급 정지 등 즉각 대응
        Emergency_Stop();
    }
}

void MX_GPIO_Init_With_EXTI(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // EXTI 모드로 설정
    GPIO_InitStruct.Pin = OBSTACLE_1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;  // 장애물 감지 시
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(OBSTACLE_PORT, &GPIO_InitStruct);
    
    HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);  // 최고 우선순위
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}
```

### 거리 추정 (아날로그 출력 모듈용)
```c
// 아날로그 출력이 있는 센서 모듈 사용 시
uint16_t Read_Distance_ADC(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 100);
    return HAL_ADC_GetValue(&hadc1);
}

// ADC 값을 거리(cm)로 변환 (센서별 캘리브레이션 필요)
float ADC_To_Distance(uint16_t adc_value)
{
    // 예시: GP2Y0A21YK0F 센서 특성 곡선
    // 실제 센서에 맞게 조정 필요
    if (adc_value < 100) return 80.0f;  // 범위 밖
    
    // 비선형 특성 보정
    float voltage = (adc_value / 4095.0f) * 3.3f;
    float distance = 27.86f * pow(voltage, -1.15f);
    
    return distance;
}
```

## 트러블슈팅

| 증상 | 원인 | 해결방법 |
|------|------|----------|
| 항상 감지됨 | 감도 과다 | 가변저항 반시계방향 조절 |
| 감지 안됨 | 감도 부족 | 가변저항 시계방향 조절 |
| 감지 안됨 | EM 핀 미연결 | EM을 GND에 연결 확인 |
| 감지 불안정 | 햇빛 간섭 | 실내에서 테스트 또는 차광 |
| 특정 물체만 안됨 | 표면 반사율 | 검은색/투명 물체는 감지 어려움 |

## 주의사항

⚠️ **EM 핀 연결 필수**
- EM 핀을 연결하지 않으면 센서가 동작하지 않음
- 간단한 사용: EM을 GND에 직접 연결
- 전력 관리: EM을 GPIO로 제어

⚠️ **센서 특성**
- 검은색 물체는 IR을 흡수하여 감지 어려움
- 투명 물체(유리)는 IR이 통과하여 감지 불가
- 직사광선에서 오작동 가능성

⚠️ **설치 위치**
- 센서 전면에 장애물 없도록 설치
- 진동이 적은 곳에 고정
- 로봇 적용 시 바닥 반사 주의

## 참고 자료

- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [IR-08H Obstacle Sensor Module](https://www.electronicoscaldas.com/datasheet/IR-08H_Steren.pdf)

