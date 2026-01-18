# Obstacle Detection Sensor Module - STM32F103 NUCLEO

적외선 장애물 감지센서 모듈을 STM32F103 NUCLEO 보드로 테스트하는 예제입니다.

## 하드웨어 구성

### 부품 목록
| 부품 | 수량 | 비고 |
|------|------|------|
| STM32F103 NUCLEO | 1 | NUCLEO-F103RB |
| 장애물 감지센서 모듈 | 1~2 | IR 반사형 |
| 점퍼 와이어 | 4+ | M-F 타입 |

### 센서 모듈 사양
- 센서 타입: 적외선(IR) 반사 센서
- 동작 전압: 3.3V ~ 5V
- 감지 거리: 2cm ~ 30cm (가변저항으로 조절)
- 출력: 디지털 (Active Low)
- 감지 각도: 약 35°

### 핀 연결

```
장애물 감지센서        STM32F103 NUCLEO
-----------------------------------------
VCC      ────────►  3.3V (CN7-16)
GND      ────────►  GND (CN7-20)
OUT      ────────►  PA0 (CN7-28) - Sensor 1
OUT      ────────►  PA1 (CN7-30) - Sensor 2 (옵션)
```

### 회로도

```
                     ┌───────────────────┐
                     │  장애물 감지센서   │
                     │     [POT]         │ ← 감지거리 조절
    PA0 ◄────────────┤ OUT              │
                     │    ◯    ◯        │
                     │   TX    RX       │ ← IR LED + Receiver
    3.3V ──────────► │ VCC              │
    GND  ──────────► │ GND              │
                     └───────────────────┘

    (2채널 구성 시 PA1에 두 번째 센서 연결)
                     
    PA5 ──────────► LED (장애물 감지 시 ON)
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

    장애물 없음: HIGH 출력
    장애물 있음: LOW 출력 (Active Low)
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

## 빌드 및 업로드

### STM32CubeIDE 사용 시
1. 새 프로젝트 생성 (STM32F103RB 선택)
2. PA0, PA1을 GPIO_Input으로 설정
3. `main.c` 내용을 프로젝트에 복사
4. 빌드 후 업로드

### CubeMX 설정
```
Pinout:
- PA0: GPIO_Input (Pull-up)
- PA1: GPIO_Input (Pull-up)
- PA5: GPIO_Output
```

## 시리얼 출력 예시

```
============================================
  Obstacle Detection Sensor Test
  STM32F103 NUCLEO
============================================
PA0: Obstacle Sensor 1 (Left/Front)
PA1: Obstacle Sensor 2 (Right, Optional)
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
| 감지 불안정 | 햇빛 간섭 | 실내에서 테스트 또는 차광 |
| 특정 물체만 안됨 | 표면 반사율 | 검은색/투명 물체는 감지 어려움 |

## 주의사항

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
- [IR Obstacle Sensor Datasheet](https://components101.com/sensors/ir-sensor-module)

## 라이선스

MIT License
