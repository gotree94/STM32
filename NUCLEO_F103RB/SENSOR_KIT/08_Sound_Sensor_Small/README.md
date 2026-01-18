# Small Sound Sensor Module - STM32F103 NUCLEO

소형 소리센서 모듈(KY-038 또는 유사 모듈)을 STM32F103 NUCLEO 보드로 테스트하는 예제입니다.

## 하드웨어 구성

### 부품 목록
| 부품 | 수량 | 비고 |
|------|------|------|
| STM32F103 NUCLEO | 1 | NUCLEO-F103RB |
| 소형 소리센서 모듈 | 1 | KY-038 또는 동급 |
| 점퍼 와이어 | 3+ | M-F 타입 |

### 센서 모듈 사양
- 동작 전압: 3.3V ~ 5V
- 출력: 디지털(DO)만 지원
- 감도 조절: 온보드 가변저항
- 출력 타입: Active Low (소리 감지 시 LOW)

### 고감도 vs 소형 센서 비교

| 특성 | 고감도(KY-037) | 소형(KY-038) |
|------|----------------|--------------|
| 아날로그 출력 | ✓ | ✗ |
| 디지털 출력 | ✓ | ✓ |
| 감도 | 높음 | 중간 |
| 크기 | 큼 | 작음 |
| 가격 | 높음 | 낮음 |

### 핀 연결

```
소형 소리센서         STM32F103 NUCLEO
-----------------------------------------
VCC      ────────►  3.3V (CN7-16)
GND      ────────►  GND (CN7-20)
DO       ────────►  PA0 (CN7-28)
```

### 회로도

```
                     ┌──────────────────┐
                     │  소형 소리센서    │
                     │      [POT]       │ ← 감도 조절
    PA0 (EXTI) ◄──── │ DO              │
                     │                 │
    3.3V ──────────► │ VCC         MIC │
                     │                 │
    GND  ──────────► │ GND             │
                     └──────────────────┘
                     
    PA5 ──────────► LED (상태 표시)
```

## 기능 설명

1. **인터럽트 기반 감지**
   - EXTI (External Interrupt)를 사용하여 즉각적인 소리 감지
   - Falling Edge 트리거 (Active Low)

2. **디바운스 처리**
   - 50ms 이내 재발생 이벤트 무시
   - 안정적인 감지 보장

3. **패턴 인식**
   - 1초 내 연속 감지 횟수 분석
   - 빠른 소리 패턴 감지 알림

4. **LED 표시**
   - 소리 감지 시 200ms 동안 LED 점등

## 빌드 및 업로드

### STM32CubeIDE 사용 시
1. 새 프로젝트 생성 (STM32F103RB 선택)
2. PA0을 GPIO_EXTI0으로 설정
3. NVIC에서 EXTI0 인터럽트 활성화
4. `main.c` 내용을 프로젝트에 복사
5. 빌드 후 업로드

### CubeMX 설정
```
Pinout:
- PA0: GPIO_EXTI0
- PA5: GPIO_Output

GPIO:
- PA0 Mode: External Interrupt Mode with Falling edge trigger
- PA0 Pull: Pull-up

NVIC:
- EXTI line0 interrupt: Enabled
```

## 시리얼 출력 예시

```
============================================
  Small Sound Sensor Module Test
  STM32F103 NUCLEO
============================================
PA0: Digital Output (Interrupt)
PA5: LED Indicator
Waiting for sound...

[1234] Sound Detected! Total Count: 1
[1567] Sound Detected! Total Count: 2
[1823] Sound Detected! Total Count: 3
  >> Rapid sound pattern detected! (3 events/sec)
[3456] Sound Detected! Total Count: 4
--- Status: 4 events detected ---
```

## 감도 조절

### 가변저항 조절 방법
1. 조용한 환경에서 시작
2. 시계 방향으로 조금씩 돌림
3. 원하는 소리 크기에서 감지되면 멈춤
4. 테스트: 손뼉, 말소리, 휘파람 등

### 최적 설정 팁
- **민감**: 작은 말소리도 감지 - 시계방향 끝
- **보통**: 보통 대화 소리 감지 - 중간
- **둔감**: 큰 소리만 감지 - 반시계방향

## 응용 예제

### 소리 카운터
```c
volatile uint32_t sound_count = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    static uint32_t last_time = 0;
    uint32_t now = HAL_GetTick();
    
    if (now - last_time > 100)  // 100ms debounce
    {
        sound_count++;
        last_time = now;
    }
}

// 메인에서
void main_loop(void)
{
    static uint32_t prev_count = 0;
    
    if (sound_count != prev_count)
    {
        printf("Count: %lu\r\n", sound_count);
        prev_count = sound_count;
    }
}
```

### 박수 패턴 인식
```c
#define PATTERN_TIMEOUT 1500  // ms
#define MAX_PATTERN_LEN 5

uint32_t pattern_times[MAX_PATTERN_LEN];
uint8_t pattern_index = 0;

void Record_Pattern(void)
{
    uint32_t now = HAL_GetTick();
    
    // 타임아웃 시 패턴 리셋
    if (pattern_index > 0 && 
        now - pattern_times[pattern_index-1] > PATTERN_TIMEOUT)
    {
        Analyze_Pattern();
        pattern_index = 0;
    }
    
    if (pattern_index < MAX_PATTERN_LEN)
    {
        pattern_times[pattern_index++] = now;
    }
}

void Analyze_Pattern(void)
{
    if (pattern_index == 2)
    {
        uint32_t interval = pattern_times[1] - pattern_times[0];
        if (interval < 500)
            printf("Double clap detected!\r\n");
    }
    else if (pattern_index == 3)
    {
        printf("Triple clap detected!\r\n");
    }
}
```

### 소음 알람
```c
#define ALARM_THRESHOLD 10  // 10초 내 10회
#define ALARM_WINDOW    10000  // 10초

void Check_Noise_Level(void)
{
    static uint32_t events[20];
    static uint8_t event_idx = 0;
    
    uint32_t now = HAL_GetTick();
    events[event_idx++ % 20] = now;
    
    // 최근 10초 내 이벤트 수 계산
    uint8_t count = 0;
    for (int i = 0; i < 20; i++)
    {
        if (now - events[i] < ALARM_WINDOW)
            count++;
    }
    
    if (count >= ALARM_THRESHOLD)
    {
        printf("!!! NOISE ALERT: %d events in 10sec !!!\r\n", count);
        LED_Blink(5, 100);
    }
}
```

## 트러블슈팅

| 증상 | 원인 | 해결방법 |
|------|------|----------|
| 감지 안됨 | 감도 낮음 | 가변저항 시계방향 조절 |
| 계속 감지됨 | 감도 과다 | 가변저항 반시계방향 조절 |
| 불규칙 감지 | 디바운스 부족 | 디바운스 시간 증가 |
| 인터럽트 안됨 | NVIC 설정 | EXTI0 인터럽트 활성화 확인 |

## 인터럽트 우선순위

```c
// 우선순위 설정 (0이 가장 높음)
HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);  // 현재 설정

// 더 높은 우선순위가 필요한 경우
HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);  // 최고 우선순위
```

## 참고 자료

- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [NUCLEO-F103RB User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)

## 라이선스

MIT License
