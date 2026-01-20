# Photo Interrupter Module Test

STM32F103 NUCLEO 보드를 이용한 포토 인터럽터(광차단) 센서 모듈 테스트 프로젝트

<img width="200" height="200" alt="photo_interrupt" src="https://github.com/user-attachments/assets/60c24318-2a60-4d78-8139-6b009af9ab61" />


## 📌 개요

슬롯형 광센서를 이용하여 물체의 통과를 감지하고, 펄스 카운팅 및 RPM 측정을 수행하는 프로젝트입니다.

## 🔧 하드웨어

### 필요 부품
| 부품 | 수량 | 비고 |
|------|------|------|
| NUCLEO-F103RB | 1 | STM32F103RB 탑재 |
| Photo Interrupter Module | 1 | KY-010 또는 호환 모듈 |
| 점퍼 와이어 | 3 | F-F 타입 |
| 엔코더 디스크 | 1 | RPM 측정용 (선택) |

<img width="644" height="586" alt="F103RB-pin" src="https://github.com/user-attachments/assets/8d79c009-095f-4cb5-bfa3-9c50a0d1bcf6" />


### 핀 연결
```
Photo Interrupter Module    NUCLEO-F103RB
========================    ==============
VCC  ------------------>    3.3V
GND  ------------------>    GND
DO   ------------------>    PA0 (A0, CN8-1)
```

### 회로도
```
                    NUCLEO-F103RB
                   +-------------+
                   |             |
    +3.3V ---------|3.3V     PA5|----[LD2]
                   |             |
     GND ---------|GND      PA2|-----> UART TX
                   |             |
Photo DO ---------|PA0 (EXTI)  |
                   |             |
                   +-------------+

KY-010 Photo Interrupter Module
+---------------------------+
|     [IR LED] --> [Photo]  |
|        |           |      |
|        |   SLOT    |      |
|        |___________|      |
|                           |
|      VCC  GND  DO         |
+-------+----+----+---------+
        |    |    |
       3.3V GND  PA0
```

## 💻 소프트웨어

### 주요 기능
1. **물체 감지**: 양방향 엣지 인터럽트 (차단/통과)
2. **펄스 카운팅**: 총 펄스 수 누적
3. **RPM 계산**: 1초 단위 회전 속도 계산
4. **주파수 측정**: Hz 단위 출력

### 설정 파라미터
```c
#define SLOTS_PER_REVOLUTION    1       /* 회전당 슬롯 수 */
#define RPM_UPDATE_INTERVAL_MS  1000    /* RPM 업데이트 주기 */
```

## 📝 코드 설명

### 양방향 EXTI 설정
```c
/* PA0: Both Rising and Falling Edge */
GPIO_InitStruct.Pin = GPIO_PIN_0;
GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
GPIO_InitStruct.Pull = GPIO_PULLUP;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
```

### RPM 계산
```c
void Calculate_RPM(void)
{
    /* RPM = (Pulses/sec / Slots per rev) * 60 */
    float pulses_per_second = (float)pulses_for_rpm * 
                              (1000.0f / RPM_UPDATE_INTERVAL_MS);
    current_rpm = (pulses_per_second / SLOTS_PER_REVOLUTION) * 60.0f;
    
    pulses_for_rpm = 0;  /* Reset counter */
}
```

### 인터럽트 콜백
```c
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_0)
    {
        uint32_t current_time = HAL_GetTick();
        
        if ((current_time - last_pulse_time) > 2)  /* 2ms debounce */
        {
            pulse_count++;
            total_pulses++;
            pulses_for_rpm++;
            
            /* Check state */
            object_detected = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET);
            state_changed = 1;
        }
    }
}
```

## 🚀 사용법

### 1. 빌드 및 업로드
```bash
1. STM32CubeIDE에서 프로젝트 Import
2. Build Project (Ctrl+B)
3. Run As > STM32 Application
```

### 2. 시리얼 모니터 설정
- 보드레이트: 115200

### 3. 출력 예시
```
========================================
  Photo Interrupter Test Program
  NUCLEO-F103RB
========================================
Place objects in the slot to detect!
Slots per revolution: 1

Initial state: CLEAR

[1523] BLOCKED - Object detected (Pulse #1)
[1756] CLEAR   - Object removed
[2234] BLOCKED - Object detected (Pulse #2)
[2489] CLEAR   - Object removed

--- Status ---
Total Pulses: 4
RPM: 120.0
Frequency: 2.00 Hz
--------------
```

## 📊 응용 예제

### 엔코더 디스크 RPM 측정
```c
/* 20슬롯 엔코더 디스크 사용 시 */
#define SLOTS_PER_REVOLUTION    20

/* RPM 정밀도가 향상됨 */
/* 1000 RPM = 333 pulses/sec */
```

### 물체 통과 속도 측정
```c
#define SLOT_WIDTH_MM    5.0f    /* 슬롯 너비 */

float Calculate_Speed_MPS(uint32_t block_duration_ms)
{
    /* 속도 = 거리 / 시간 */
    float distance_m = SLOT_WIDTH_MM / 1000.0f;
    float time_s = block_duration_ms / 1000.0f;
    return distance_m / time_s;
}

/* 인터럽트에서 측정 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    static uint32_t block_start = 0;
    
    if (object_detected) {
        block_start = HAL_GetTick();
    } else {
        uint32_t duration = HAL_GetTick() - block_start;
        float speed = Calculate_Speed_MPS(duration);
        printf("Speed: %.3f m/s\r\n", speed);
    }
}
```

### 컨베이어 카운터
```c
typedef struct {
    uint32_t count;
    uint32_t count_per_minute;
    uint32_t last_reset;
} ConveyorCounter;

void Update_Conveyor_Counter(ConveyorCounter *counter)
{
    uint32_t current = HAL_GetTick();
    
    if ((current - counter->last_reset) >= 60000) {
        counter->count_per_minute = counter->count;
        counter->count = 0;
        counter->last_reset = current;
        
        printf("Items/min: %lu\r\n", counter->count_per_minute);
    }
}
```

### 방향 감지 (2센서 사용)
```c
/* PA0: 센서 A, PA1: 센서 B */
volatile uint8_t sensor_a_state = 0;
volatile uint8_t sensor_b_state = 0;
volatile int8_t direction = 0;  /* 1: CW, -1: CCW */

void Detect_Direction(void)
{
    static uint8_t last_a = 0, last_b = 0;
    
    /* Quadrature decoding */
    if (sensor_a_state != last_a) {
        if (sensor_a_state == sensor_b_state) {
            direction = 1;   /* Clockwise */
        } else {
            direction = -1;  /* Counter-clockwise */
        }
    }
    
    last_a = sensor_a_state;
    last_b = sensor_b_state;
}
```

## ⚠️ 주의사항

1. **슬롯 너비**: 감지 물체가 슬롯보다 커야 함
2. **고속 회전**: 높은 RPM에서는 디바운스 시간 단축 필요
3. **먼지/오염**: 광학 경로 청결 유지
4. **주변광**: 직사광선 회피

## 🔍 트러블슈팅

| 증상 | 원인 | 해결책 |
|------|------|--------|
| 항상 BLOCKED | IR LED 불량 또는 오염 | 청소 또는 모듈 교체 |
| 감지 안됨 | 물체가 슬롯 통과 안함 | 위치 조정 |
| 펄스 누락 | 디바운스 시간 너무 김 | 디바운스 감소 |
| RPM 부정확 | SLOTS_PER_REVOLUTION 오류 | 슬롯 수 확인 |

## 📐 센서 특성

### 포토 인터럽터
| 특성 | 값 |
|------|------|
| 슬롯 너비 | ~3mm (모듈마다 다름) |
| 응답 시간 | ~10μs |
| 출력 타입 | Open Collector / Push-Pull |
| 동작 전압 | 3.3V ~ 5V |

### 신호 특성
```
센서 상태        출력      설명
-----------     ------    ----------------
빛 통과 (Clear)  HIGH     슬롯이 비어있음
빛 차단 (Block)  LOW      물체가 슬롯에 있음
```

### 최대 감지 속도
```
디바운스 2ms 기준:
최대 주파수 = 1 / (2 × 0.002) = 250 Hz
최대 RPM (1 slot) = 250 × 60 = 15,000 RPM
최대 RPM (20 slots) = 250 × 60 / 20 = 750 RPM
```

## 📚 참고자료

- [STM32 Timer Encoder Mode](https://www.st.com/resource/en/application_note/an4013-stm32-crossseries-timer-overview-stmicroelectronics.pdf)
- [Optical Encoder Basics](https://www.electronics-tutorials.ws/io/io_2.html)

## 📜 라이선스

MIT License
