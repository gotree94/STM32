# Knock Sensor Module Test

STM32F103 NUCLEO 보드를 이용한 노크(진동) 센서 모듈 테스트 프로젝트

## 📌 개요

압전 소자를 이용하여 진동/충격을 감지하고, 노크 패턴을 분석하는 프로젝트입니다.

## 🔧 하드웨어

### 필요 부품
| 부품 | 수량 | 비고 |
|------|------|------|
| NUCLEO-F103RB | 1 | STM32F103RB 탑재 |
| Knock Sensor Module | 1 | KY-031 또는 호환 모듈 |
| 점퍼 와이어 | 3 | F-F 타입 |

### 핀 연결
```
Knock Sensor Module         NUCLEO-F103RB
===================         ==============
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
Knock DO ---------|PA0 (EXTI)  |
                   |             |
                   +-------------+

KY-031 Knock Sensor Module
+-------------------+
|   [Piezo Disc]    |
|       ___         |
|      /   \        |
|     |     |       |
|      \___/        |
|                   |
|  VCC GND DO       |
+---+---+---+-------+
    |   |   |
   3.3V GND PA0
```

## 💻 소프트웨어

### 주요 기능
1. **진동 감지**: EXTI 인터럽트 방식 감지
2. **디바운싱**: 50ms 소프트웨어 디바운스
3. **노크 카운팅**: 누적 노크 횟수 기록
4. **패턴 분석**: 노크 간격 및 리듬 분석

### 감지 파라미터
```c
#define DEBOUNCE_TIME_MS    50      /* 디바운스 시간 */
#define PATTERN_TIMEOUT_MS  1000    /* 패턴 인식 타임아웃 */
#define MAX_KNOCKS          10      /* 최대 노크 횟수 기록 */
```

## 📝 코드 설명

### EXTI 인터럽트 설정
```c
/* PA0: Falling Edge Interrupt */
GPIO_InitStruct.Pin = GPIO_PIN_0;
GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
GPIO_InitStruct.Pull = GPIO_PULLUP;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* Enable EXTI0 interrupt */
HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
HAL_NVIC_EnableIRQ(EXTI0_IRQn);
```

### 인터럽트 콜백
```c
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_0)
    {
        uint32_t current_time = HAL_GetTick();
        
        /* Debounce check */
        if ((current_time - last_knock_time) > DEBOUNCE_TIME_MS)
        {
            knock_detected = 1;
            knock_count++;
            last_knock_time = current_time;
            
            /* Record for pattern recognition */
            if (knock_index < MAX_KNOCKS) {
                knock_times[knock_index++] = current_time - pattern_start_time;
            }
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
  Knock Sensor Test Program
  NUCLEO-F103RB
========================================
Knock the sensor to detect vibration!

[Knock #1] Detected at 1523 ms
[Knock #2] Detected at 1756 ms
[Knock #3] Detected at 1989 ms

>> Pattern Analysis (3 knocks):
   Times: 0 233 466 ms
   Intervals: 233 233 ms
   Average interval: 233.0 ms (257.5 BPM)
   Pattern: TRIPLE KNOCK
```

## 📊 응용 예제

### 비밀 노크 패턴 잠금
```c
/* 비밀 패턴: 짧음-짧음-길음 (S-S-L) */
#define SHORT_KNOCK_MAX  300   /* 짧은 간격 최대값 (ms) */
#define LONG_KNOCK_MIN   500   /* 긴 간격 최소값 (ms) */

uint8_t Check_Secret_Pattern(void)
{
    if (knock_index != 3) return 0;
    
    uint32_t interval1 = knock_times[1] - knock_times[0];
    uint32_t interval2 = knock_times[2] - knock_times[1];
    
    /* S-S-L 패턴 확인 */
    if (interval1 < SHORT_KNOCK_MAX && 
        interval2 > LONG_KNOCK_MIN) {
        return 1;  /* 패턴 일치 */
    }
    
    return 0;
}
```

### 진동 강도 측정 (ADC 사용)
```c
/* 아날로그 출력이 있는 모듈 사용 시 */
uint16_t Read_Knock_Intensity(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    return HAL_ADC_GetValue(&hadc1);
}

void Process_Knock_With_Intensity(void)
{
    uint16_t intensity = Read_Knock_Intensity();
    
    if (intensity > 3000) {
        printf("STRONG knock!\r\n");
    } else if (intensity > 1500) {
        printf("Medium knock\r\n");
    } else {
        printf("Light knock\r\n");
    }
}
```

## ⚠️ 주의사항

1. **감도 조절**: 모듈에 가변저항이 있는 경우 감도 조절 가능
2. **설치 방법**: 센서를 단단한 표면에 부착
3. **디바운싱**: 진동 특성에 따라 디바운스 시간 조절
4. **노이즈**: 주변 진동원 (모터, 스피커 등) 주의

## 🔍 트러블슈팅

| 증상 | 원인 | 해결책 |
|------|------|--------|
| 감지 안됨 | 감도 낮음 또는 배선 오류 | 감도 조절, 배선 확인 |
| 중복 감지 | 디바운스 부족 | 디바운스 시간 증가 |
| 오감지 | 주변 진동 | 센서 격리 또는 필터링 |
| 불안정 | Pull-up 미설정 | 내부/외부 Pull-up 추가 |

## 📐 센서 특성

### 압전 소자 (Piezoelectric Sensor)
| 특성 | 값 |
|------|------|
| 감지 범위 | 진동/충격 |
| 응답 시간 | ~1ms |
| 출력 타입 | 디지털 (일부 아날로그) |
| 동작 전압 | 3.3V ~ 5V |

### 신호 특성
| 상태 | 출력 |
|------|------|
| 대기 상태 | HIGH (Pull-up) |
| 진동 감지 | LOW (순간적) |

## 📚 참고자료

- [STM32 External Interrupts](https://www.st.com/resource/en/application_note/an4013-stm32-crossseries-timer-overview-stmicroelectronics.pdf)
- [Piezoelectric Sensor Basics](https://www.electronics-tutorials.ws/io/io_6.html)

## 📜 라이선스

MIT License
