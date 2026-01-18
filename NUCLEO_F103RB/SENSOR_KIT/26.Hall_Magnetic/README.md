# Hall Magnetic Sensor Module Test

STM32F103 NUCLEO 보드를 이용한 홀 마그네틱 센서 모듈 테스트 프로젝트

## 📌 개요

홀 효과(Hall Effect)를 이용하여 자기장을 감지하는 센서 모듈 테스트입니다. 자석이 접근하면 디지털 신호를 출력합니다.

## 🔧 하드웨어

### 필요 부품
| 부품 | 수량 | 비고 |
|------|------|------|
| NUCLEO-F103RB | 1 | STM32F103RB 탑재 |
| Hall Magnetic Sensor Module | 1 | KY-003 또는 호환 모듈 |
| 점퍼 와이어 | 3 | F-F 타입 |
| 자석 | 1 | 테스트용 |

### 핀 연결
```
Hall Sensor Module          NUCLEO-F103RB
==================          ==============
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
     GND ---------|GND      PA2|-----> UART TX (Virtual COM)
                   |             |
 Hall DO ---------|PA0         |
                   |             |
                   +-------------+

Hall Magnetic Sensor Module
+-------------------+
|  [Hall IC]        |
|   _____           |
|  |     |          |
|  |_____|          |
|                   |
|  VCC GND DO       |
+---+---+---+-------+
    |   |   |
   3.3V GND PA0
```

## 💻 소프트웨어

### 개발 환경
- STM32CubeIDE 또는 Keil MDK
- STM32CubeMX (선택)
- STM32F1 HAL Driver

### 주요 기능
1. **자석 감지**: PA0 핀을 통해 센서의 디지털 출력 읽기
2. **LED 표시**: 자석 감지 시 온보드 LED (PA5) 점등
3. **UART 출력**: 상태 변화를 시리얼 모니터에 출력

### 동작 원리
```
센서 상태        출력      LED     설명
-----------     ------    -----   ----------------
자석 없음        HIGH      OFF     대기 상태
자석 감지        LOW       ON      자기장 검출
```

## 📝 코드 설명

### GPIO 설정
```c
/* PA0: Hall Sensor Input (Pull-up) */
GPIO_InitStruct.Pin = GPIO_PIN_0;
GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
GPIO_InitStruct.Pull = GPIO_PULLUP;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
```

### 센서 읽기
```c
curr_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);

if (curr_state == GPIO_PIN_RESET) {
    // 자석 감지됨
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
}
```

## 🚀 사용법

### 1. 빌드 및 업로드
```bash
# STM32CubeIDE 사용 시
1. 프로젝트 Import
2. Build Project (Ctrl+B)
3. Run As > STM32 Application
```

### 2. 시리얼 모니터 연결
- 포트: ST-Link Virtual COM Port
- 보드레이트: 115200
- 데이터 비트: 8
- 패리티: None
- 스톱 비트: 1

### 3. 테스트
```
========================================
  Hall Magnetic Sensor Test Program
  NUCLEO-F103RB
========================================

[1] Magnet DETECTED! (Sensor: LOW)
[1] Magnet REMOVED  (Sensor: HIGH)
[2] Magnet DETECTED! (Sensor: LOW)
[2] Magnet REMOVED  (Sensor: HIGH)
```

## 📊 응용 예제

### 인터럽트 방식 (개선)
```c
/* PA0에 EXTI 인터럽트 설정 */
GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
HAL_NVIC_EnableIRQ(EXTI0_IRQn);

/* 인터럽트 핸들러 */
void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_0) {
        // 자석 감지 처리
    }
}
```

### RPM 측정 (회전 감지)
```c
uint32_t pulse_count = 0;
uint32_t last_time = 0;
float rpm = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    pulse_count++;
    
    uint32_t current_time = HAL_GetTick();
    if (current_time - last_time >= 1000) {
        rpm = pulse_count * 60.0f;  // 1초당 펄스 -> RPM
        pulse_count = 0;
        last_time = current_time;
    }
}
```

## ⚠️ 주의사항

1. **전압**: 센서 모듈의 동작 전압 확인 (3.3V/5V)
2. **자석 극성**: 일부 센서는 특정 극성에만 반응
3. **감지 거리**: 센서에 따라 감지 거리가 다름 (보통 1~3cm)
4. **디바운싱**: 기계적 진동 시 추가 디바운스 처리 필요

## 🔍 트러블슈팅

| 증상 | 원인 | 해결책 |
|------|------|--------|
| 항상 HIGH | 연결 불량 또는 VCC 미연결 | 배선 확인 |
| 항상 LOW | GND 쇼트 | 배선 확인 |
| 불안정한 출력 | 노이즈 또는 Pull-up 미설정 | Pull-up 저항 추가 |
| 감지 안됨 | 자석 거리 또는 극성 | 거리/극성 변경 |

## 📚 참고자료

- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [Hall Effect Sensor Datasheet](https://www.allegromicro.com/en/Products/Sense/Linear-and-Angular-Position/Linear-Position-Sensor-ICs/A1301-2)

## 📜 라이선스

MIT License
