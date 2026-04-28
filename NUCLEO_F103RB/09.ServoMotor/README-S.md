# STM32F103 NUCLEO - SG90 서보모터 PWM 제어

STM32F103RB (NUCLEO 보드) 에서 TIM2를 이용한 PWM 신호로 SG90 서보모터를 제어합니다.

---

## 1. 하드웨어 연결

| SG90 핀 | 색상 | NUCLEO 핀 |
|:-------:|:----:|:---------:|
| GND     | 갈색 | GND       |
| VCC     | 빨간색 | 5V       |
| Signal  | 주황색 | PA0 (TIM2_CH1) |

> **주의:** SG90은 5V 동작 전압이 필요합니다. 신호선은 3.3V도 인식합니다.

---

## 2. 타이머 설정 (PWM 주파수 계산)

### 클럭 구성

```
HSI (8 MHz) / 2 = 4 MHz
PLL × 16 = 64 MHz  →  SYSCLK = 64 MHz
APB1 = 64 MHz / 2 = 32 MHz
TIM2 클럭 = APB1 × 2 = 64 MHz   ← APB1 프리스케일러 ≠ 1이면 ×2
```

### CubeMX 설정값

| 파라미터 | 설정값 | 비고 |
|:--------:|:------:|:----:|
| Prescaler | 1280 - 1 = **1279** | 타이머 카운트 주파수 = 50 kHz |
| Period (ARR) | 1000 - 1 = **999** | PWM 주기 = 20 ms (50 Hz) |
| Channel | TIM2_CH1 | PA0 핀 |
| Mode | PWM Generation CH1 | |

### 주파수 계산

$$
f_{timer} = \frac{64\,\text{MHz}}{1280} = 50\,\text{kHz}, \quad T_{tick} = 20\,\mu s
$$

$$
T_{PWM} = \frac{1000}{50\,\text{kHz}} = 20\,\text{ms} \quad (50\,\text{Hz}) \checkmark
$$

---

## 3. CCR 값과 각도 관계

SG90의 펄스 폭 범위: **0.5 ms ~ 2.5 ms**

> CCR 1 = 20 ms / 1000 = **0.02 ms**

| 각도 | 펄스 폭 | 계산 | CCR 값 |
|:----:|:-------:|:----:|:------:|
| 0°   | 0.5 ms  | 0.5 ms / 0.02 ms | **25** |
| 90°  | 1.5 ms  | 1.5 ms / 0.02 ms | **75** |
| 180° | 2.5 ms  | 2.5 ms / 0.02 ms | **125** |

```
CCR 범위: 25 (0°) ~ 75 (90°) ~ 125 (180°)
```

---

## 4. 코드

### 4.1 각도 제어 함수

```c
void SG90_SetAngle(uint8_t angle)
{
    // angle: 0 ~ 180도
    // CCR: 25(0.5ms, 0°) ~ 125(2.5ms, 180°)
    uint32_t ccr_val = 25 + ((uint32_t)angle * 100 / 180);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, ccr_val);
}
```

### 4.2 PWM 시작 (필수)

```c
/* USER CODE BEGIN 2 */
HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);   // 반드시 호출해야 출력됨
/* USER CODE END 2 */
```

### 4.3 특정 각도 이동 예시

```c
/* USER CODE BEGIN WHILE */
while (1)
{
    SG90_SetAngle(0);     // 0도
    HAL_Delay(1000);

    SG90_SetAngle(90);    // 90도
    HAL_Delay(1000);

    SG90_SetAngle(180);   // 180도
    HAL_Delay(1000);
}
/* USER CODE END WHILE */
```

### 4.4 0° → 180° → 0° 스윕 예시

```c
/* USER CODE BEGIN WHILE */
while (1)
{
    // 0도 → 180도 (증가)
    for (int angle = 0; angle <= 180; angle++)
    {
        SG90_SetAngle(angle);
        HAL_Delay(10);          // 1도당 10ms → 전체 스윕 약 1.8초
    }

    // 180도 → 0도 (감소)
    for (int angle = 180; angle >= 0; angle--)
    {
        SG90_SetAngle(angle);
        HAL_Delay(10);
    }
}
/* USER CODE END WHILE */
```

> `HAL_Delay()` 값 조정으로 속도 제어 가능 (최소 권장: 5 ms)

---

## 5. Pan/Tilt 2축 제어 (UART 명령)

TIM2_CH1 (PA0) → Pan 서보, TIM3_CH1 (PA6) → Tilt 서보

### 5.1 매크로 정의

```c
/* USER CODE BEGIN PD */
#define MAX    125    // 2.5ms → 180도
#define MIN     25    // 0.5ms → 0도
#define CENTER  75    // 1.5ms → 90도
#define STEP     1    // 1스텝 = 약 1.8도
/* USER CODE END PD */
```

### 5.2 전역 변수

```c
/* USER CODE BEGIN PV */
uint8_t ch;
uint8_t pos_pan  = CENTER;
uint8_t pos_tilt = CENTER;
/* USER CODE END PV */
```

### 5.3 printf 리타겟팅 (UART2)

```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */
```

```c
/* USER CODE BEGIN 0 */
#ifdef __GNUC__
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
    if (ch == '\n')
        HAL_UART_Transmit(&huart2, (uint8_t *)"\r", 1, 0xFFFF);
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
}

/* PWM 값 → 각도 변환 (소수점 1자리, 10배 스케일) */
uint16_t pwm_to_angle(uint8_t pwm_value)
{
    return ((uint16_t)(pwm_value - MIN) * 1800) / (MAX - MIN);
}

/* 서보 상태 출력 */
void display_servo_status(uint8_t pan, uint8_t tilt)
{
    uint16_t pa = pwm_to_angle(pan);
    uint16_t ta = pwm_to_angle(tilt);
    printf("Pan: %3d (%3d.%d deg) | Tilt: %3d (%3d.%d deg)\r\n",
           pan,  pa / 10, pa % 10,
           tilt, ta / 10, ta % 10);
}
/* USER CODE END 0 */
```

### 5.4 초기화

```c
/* USER CODE BEGIN 2 */
HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pos_pan);
__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pos_tilt);

printf("\r\n=== SG90 Pan/Tilt Control ===\r\n");
printf("w: Up  s: Down  a: Left  d: Right  i: Center\r\n");
display_servo_status(pos_pan, pos_tilt);
/* USER CODE END 2 */
```

### 5.5 메인 루프 (switch-case)

```c
/* USER CODE BEGIN WHILE */
while (1)
{
    if (HAL_UART_Receive(&huart2, &ch, sizeof(ch), 10) == HAL_OK)
    {
        switch (ch)
        {
            case 'w':   // Tilt Up
                pos_tilt = (pos_tilt - STEP >= MIN) ? (pos_tilt - STEP) : MIN;
                break;

            case 's':   // Tilt Down
                pos_tilt = (pos_tilt + STEP <= MAX) ? (pos_tilt + STEP) : MAX;
                break;

            case 'a':   // Pan Left
                pos_pan = (pos_pan + STEP <= MAX) ? (pos_pan + STEP) : MAX;
                break;

            case 'd':   // Pan Right
                pos_pan = (pos_pan - STEP >= MIN) ? (pos_pan - STEP) : MIN;
                break;

            case 'i':   // Center
                pos_pan  = CENTER;
                pos_tilt = CENTER;
                break;

            default:
                printf("Unknown command: %c\r\n", ch);
                continue;   // PWM 업데이트 건너뜀
        }

        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pos_pan);
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pos_tilt);

        display_servo_status(pos_pan, pos_tilt);
        HAL_Delay(50);
    }
}
/* USER CODE END WHILE */
```

### 5.6 UART 명령 키맵

| 키 | 동작 |
|:--:|:----:|
| `w` | Tilt Up (위) |
| `s` | Tilt Down (아래) |
| `a` | Pan Left (왼쪽) |
| `d` | Pan Right (오른쪽) |
| `i` | Center (중앙 복귀) |

---

## 6. 얼굴 추적 Python 연동 (선택)

PC에서 OpenCV로 얼굴을 인식하고 UART 명령을 전송해 서보를 자동 제어합니다.

### 설치

```bash
pip install pyserial opencv-python
```

### 실행

```bash
python face_tracking_pantilt.py --port COM3
python face_tracking_pantilt.py --port COM7 --camera 2
```

### Pan/Tilt 방향 로직

```python
# 얼굴이 화면 오른쪽 → Pan을 오른쪽으로
if error_x > deadzone_x:
    send_command('a')       # Pan Left 명령
elif error_x < -deadzone_x:
    send_command('d')       # Pan Right 명령

# 얼굴이 화면 아래쪽 → Tilt를 아래쪽으로
if error_y > deadzone_y:
    send_command('s')       # Tilt Down 명령
elif error_y < -deadzone_y:
    send_command('w')       # Tilt Up 명령
```

---

## 7. 트러블슈팅

| 증상 | 원인 | 해결 |
|------|------|------|
| 서보가 전혀 안 움직임 | `HAL_TIM_PWM_Start()` 누락 | `USER CODE BEGIN 2`에 추가 |
| 90도까지만 움직임 | CCR 범위가 50~100으로 설정됨 | CCR 범위를 **25~125**로 수정 |
| 떨림 발생 | 전원 노이즈 | 서보 전원을 별도 5V로 분리 |
| printf 출력 안 됨 | PUTCHAR 리타겟팅 누락 | `USER CODE BEGIN 0` 코드 추가 |

---

## 8. 참고

- SG90 펄스 범위: **0.5 ms ~ 2.5 ms** (제조사 스펙 기준)
- PWM 주기: **20 ms (50 Hz)**
- TIM2_CH1 핀: **PA0** (NUCLEO-F103RB)
- TIM3_CH1 핀: **PA6** (NUCLEO-F103RB)
