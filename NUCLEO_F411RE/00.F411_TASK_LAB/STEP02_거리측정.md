# STEP02 - 초음파 거리 측정

> 상태: **코드 구현/빌드 완료. 보드 배선/확인 대기.**
> 센서: HC-SR04 (5 V) 기준
> 측정: 5 Hz 샘플링, 1 Hz 중앙값 출력
> 빌드: 0 errors, 0 warnings

---

## 1. 목표

- HC-SR04로 거리를 측정한다.
- "몇 초마다 정확한 데이터를 확인한다"를 **측정 가능한 숫자**로 바꾼다.
- `HAL_Delay()` 없이 **비차단 상태 머신**으로 측정한다.
- 타임스탬프, 오류 횟수, 필터 결과를 UART CSV로 확인한다.
- LED 점멸은 계속 동작하게 두어 **여러 작업이 서로를 막지 않는지** 확인한다.

이번 단계에서 확정한 값:

| 항목 | 값 |
|---|---|
| 센서 | HC-SR04 |
| TRIG | PA1 (GPIO 출력) |
| ECHO | PA0 (EXTI 양쪽 에지) |
| 샘플 주기 | 200 ms (5 Hz) |
| 출력 주기 | 1000 ms (1 Hz) |
| 필터 | 최근 5개 중앙값 |
| 측정 타임아웃 | 35 ms |
| 유효 펄스 | 100 ~ 30000 us |
| 유효 거리 | 20 ~ 4000 mm |
| 음속 | 343 m/s (20 °C 기준) |
| 목표 정확도 | ±1 cm (0.2 ~ 2 m, 평평한 물체) |

---

## 2. 준비물

- NUCLEO-F411RE
- HC-SR04 초음파 센서
- 저항 1 kΩ, 2 kΩ 각 1개
- 점퍼선, 브레드보드
- USB 케이블 (ST-LINK, 데이터 지원)
- 시리얼 프로그램 (Tera Term / PuTTY / VS Code)
- 자 또는 줄자

---

## 3. 배선

### 3.1 HC-SR04 핀

| HC-SR04 | 연결 |
|---|---|
| VCC | NUCLEO **5 V** |
| GND | NUCLEO GND |
| TRIG | PA1 |
| ECHO | 분압 후 PA0 |

### 3.2 ECHO 전압 분압

HC-SR04의 ECHO는 **5 V 출력**이다. STM32 GPIO에 직접 연결하지 않는다.

```text
HC-SR04 ECHO ---- 1 kΩ ----+---- PA0
                           |
                          2 kΩ
                           |
                          GND
```

- PA0 전압: `5 V × 2 kΩ / (1 kΩ + 2 kΩ) ≈ 3.33 V`
- GND는 센서와 보드가 **반드시 공통**이어야 한다.
- HC-SR04는 3.3 V에서도 동작하는 것처럼 보일 수 있지만, 거리 정확도와 안정성이 떨어진다. **5 V 전원**을 사용한다.

### 3.3 연결 확인

- 전원을 넣기 전에 5 V와 GND가 바뀌지 않았는지 확인한다.
- PA0에는 분압 노드만 연결한다. ECHO를 직접 PA0에 꽂지 않는다.
- PA1은 TRIG 출력이다.
- PA0/PA1은 NUCLEO의 Arduino 아날로그 헤더 A0/A1 위치다.

---

## 4. 설계

### 4.1 왜 TIM2를 쓰는가

HC-SR04의 ECHO 펄스 폭은 수백 us ~ 수십 ms다. `HAL_GetTick()`은 1 ms 해상도라서 정확도가 부족하다.

TIM2를 1 MHz로 동작시켜 **1 tick = 1 us**로 만든다.

```text
TIM2 입력 클럭 = 84 MHz
Prescaler = 83
카운터 클럭 = 84 MHz / (83 + 1) = 1 MHz
```

TIM2는 32비트 카운터이므로 `0xFFFFFFFF`까지 세고 다시 0으로 돌아간다.
펄스 폭은 `끝 시각 - 시작 시각`을 `uint32_t`로 계산하면 **한 번의 wrap까지 자동으로 처리**된다.
(측정 최대값 30 ms는 32비트 wrap 주기 약 71분보다 훨씬 짧다.)

### 4.2 왜 EXTI를 쓰는가

ECHO의 상승 에지와 하강 에지 시각을 인터럽트에서 기록한다.

- 상승 에지: TIM2 카운터 저장
- 하강 에지: `현재 카운터 - 상승 에지 카운터` = 펄스 폭(us)

ISR에서는 **시각 기록만** 하고, 거리 계산/필터/출력은 메인 루프에서 한다.
ISR을 짧게 유지해야 다른 인터럽트와 LED 작업이 막히지 않는다.

### 4.3 상태 머신

`HAL_Delay()`로 ECHO를 기다리지 않고, 메인 루프가 자주 호출하는 `Ultrasonic_Task()` 안에서 상태를 진행한다.

```text
IDLE
  └─ 200 ms 경과 → TRIG HIGH, 시작 시각 기록
       └─ TRIG_HIGH
            └─ 10 us 경과 → TRIG LOW, 타임아웃 시작
                 └─ WAIT_ECHO
                      ├─ ECHO 펄스 완료 → 거리 저장, IDLE
                      └─ 35 ms 초과 → 오류 +1, IDLE
```

- `IDLE`: 다음 샘플 시간을 기다린다.
- `TRIG_HIGH`: TRIG를 10 us 이상 유지한다.
- `WAIT_ECHO`: ECHO 상승/하강을 기다린다. 35 ms 안에 끝나지 않으면 오류다.

### 4.4 필터와 출력

- 200 ms마다 1개씩 측정한다.
- 1000 ms마다 최근 5개(없으면 있는 만큼)의 **중앙값**을 출력한다.
- 중앙값은 단발 튐(노이즈)을 억제한다.
- 평균은 큰 이상값에 끌리지만, 중앙값은 상대적으로 덜 끌린다.

### 4.5 거리 계산

20 °C 기준 음속 343 m/s:

```text
거리(mm) = 펄스폭(us) × 343 / 2000
```

- `펄스폭 × 343`은 최대 `30000 × 343 = 10,290,000`으로 `uint32_t` 범위 안이다.
- 왕복이므로 2로 나눈다.
- 단위를 mm로 맞추기 위해 `m/s → mm/us` 변환을 포함한 식이 위와 같다.

온도 보정은 STEP03에서 다룬다.

```text
v(m/s) = 331.3 + 0.606 × T(°C)
```

---

## 5. 변경된 파일

| 파일 | 변경 |
|---|---|
| `Core/Inc/ultrasonic.h` | 새 파일. 결과 구조체와 API 선언 |
| `Core/Src/ultrasonic.c` | 새 파일. TIM2/GPIO/EXTI 초기화, 상태 머신, 필터 |
| `Core/Src/main.c` | `Ultrasonic_Init()`, `Sensor_Task()`, EXTI 콜백 추가 |
| `Core/Src/stm32f4xx_it.c` | `EXTI0_IRQHandler()` 추가 |
| `Core/Inc/stm32f4xx_hal_conf.h` | `HAL_TIM_MODULE_ENABLED` 활성화 |
| `Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c` | TIM HAL 소스 추가 |
| `Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c` | TIM 확장 HAL 소스 추가 |
| `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h` | TIM HAL 헤더 추가 |
| `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim_ex.h` | TIM 확장 HAL 헤더 추가 |
| `F411_TASK_LAB.ioc` | 변경 없음 |

### 5.1 `.ioc`를 수정하지 않은 이유

이 프로젝트는 CubeMX의 **Advanced Folder Structure**로 생성되어, 사용하는 HAL 모듈의 소스만 들어 있다.
TIM2/EXTI를 `.ioc`에서 켜면 코드 재생성 시 `main.c`의 초기화 코드가 바뀌고, 기존 USER CODE와 충돌할 수 있다.

이번 단계에서는 `.ioc`를 건드리지 않고 `ultrasonic.c`에서 직접 초기화한다.

> **주의:** 나중에 `.ioc`에서 `GENERATE CODE`를 실행하면 `stm32f4xx_hal_conf.h`의 `HAL_TIM_MODULE_ENABLED`가 다시 주석 처리될 수 있다.
> 그 경우 이 파일의 변경 내용과 TIM HAL 파일이 그대로 있는지 확인하고 다시 활성화한다.

---

## 6. 코드

### 6.1 API (`ultrasonic.h`)

```c
typedef struct
{
  uint32_t timestamp_ms;
  uint32_t errors;
  uint16_t distance_mm;
  uint8_t valid;
} Ultrasonic_Result;

void Ultrasonic_Init(void);
void Ultrasonic_Task(void);
uint8_t Ultrasonic_GetResult(Ultrasonic_Result *result);
void Ultrasonic_OnEchoEdge(void);
```

- `Ultrasonic_Task()`: 메인 루프에서 계속 호출한다.
- `Ultrasonic_GetResult()`: 새 결과가 있으면 1, 없으면 0을 반환한다.
- `Ultrasonic_OnEchoEdge()`: EXTI 콜백에서 호출한다.

### 6.2 주요 설정 (`ultrasonic.c`)

```c
#define US_SAMPLE_PERIOD_MS  200U
#define US_PUBLISH_PERIOD_MS 1000U
#define US_ECHO_TIMEOUT_MS   35U
#define US_TRIG_PULSE_US     10U
#define US_MIN_PULSE_US      100U
#define US_MAX_PULSE_US      30000U
#define US_MIN_DISTANCE_MM   20U
#define US_MAX_DISTANCE_MM   4000U
#define US_FILTER_SIZE       5U
```

TIM2 설정:

```c
s_tim2.Instance = TIM2;
s_tim2.Init.Prescaler = 83U;          /* 84 MHz / 84 = 1 MHz */
s_tim2.Init.CounterMode = TIM_COUNTERMODE_UP;
s_tim2.Init.Period = 0xFFFFFFFFU;     /* 32-bit free running */
s_tim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
s_tim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
HAL_TIM_Base_Init(&s_tim2);
__HAL_TIM_SET_COUNTER(&s_tim2, 0U);
HAL_TIM_Base_Start(&s_tim2);
```

PA0/PA1 설정:

```c
/* PA1: TRIG 출력 */
gpio.Pin = GPIO_PIN_1;
gpio.Mode = GPIO_MODE_OUTPUT_PP;
gpio.Pull = GPIO_NOPULL;
gpio.Speed = GPIO_SPEED_FREQ_LOW;
HAL_GPIO_Init(GPIOA, &gpio);

/* PA0: ECHO 양쪽 에지 인터럽트 */
gpio.Pin = GPIO_PIN_0;
gpio.Mode = GPIO_MODE_IT_RISING_FALLING;
gpio.Pull = GPIO_NOPULL;
HAL_GPIO_Init(GPIOA, &gpio);

HAL_NVIC_SetPriority(EXTI0_IRQn, 5U, 0U);
HAL_NVIC_EnableIRQ(EXTI0_IRQn);
```

### 6.3 EXTI 처리 (`stm32f4xx_it.c`)

```c
void EXTI0_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}
```

`main.c`의 콜백:

```c
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == GPIO_PIN_0)
  {
    Ultrasonic_OnEchoEdge();
  }
}
```

### 6.4 메인 루프 (`main.c`)

```c
Ultrasonic_Init();

UART_Print("\r\n=== F411_TASK_LAB Step 2: Ultrasonic ===\r\n");
UART_Print("HC-SR04: TRIG=PA1, ECHO=PA0, 5 Hz sample, 1 Hz median\r\n");

while (1)
{
  LED_Task();
  Sensor_Task();
}
```

`Sensor_Task()`:

```c
static void Sensor_Task(void)
{
  Ultrasonic_Result result;
  char line[80];

  Ultrasonic_Task();

  if (Ultrasonic_GetResult(&result) != 0U)
  {
    (void)snprintf(line, sizeof(line), "D,%lu,%u,%u,%lu\r\n",
                   (unsigned long)result.timestamp_ms,
                   (unsigned int)result.distance_mm,
                   (unsigned int)result.valid,
                   (unsigned long)result.errors);
    UART_Print(line);
  }
}
```

> STEP02부터 LED는 0.5초마다 **무음으로** 점멸한다.
> CSV 로그를 깨끗하게 유지하기 위해 `LED ON` / `LED OFF` UART 출력은 제거했다.

---

## 7. 출력 형식

```text
D,t_ms,distance_mm,valid,errors
```

| 필드 | 의미 |
|---|---|
| `D` | 거리 데이터 행 |
| `t_ms` | `HAL_GetTick()` 기준 출력 시각 (ms) |
| `distance_mm` | 중앙값 거리 (mm) |
| `valid` | 1 = 유효, 0 = 이번 1초 구간에 유효 샘플 없음 |
| `errors` | 누적 오류 횟수 (타임아웃 + 범위 밖 + 펄스 폭 이상) |

예시:

```text
=== F411_TASK_LAB Step 2: Ultrasonic ===
HC-SR04: TRIG=PA1, ECHO=PA0, 5 Hz sample, 1 Hz median
D,1000,245,1,0
D,2000,246,1,0
D,3000,244,1,0
D,4000,0,0,3
D,5000,512,1,3
```

- `valid=0`이면 그 1초 동안 유효한 측정이 없었다는 뜻이다.
- `errors`는 누적값이므로 증가 추세를 보면 배선/센서 문제를 알 수 있다.

---

## 8. 빌드 결과

헤드리스 빌드로 검증했다.

```text
Build Finished. 0 errors, 0 warnings.

text    data    bss     dec     hex    filename
13804     92    2116   16012    3e8c   F411_TASK_LAB.elf
```

빌드 중 `TIM_HandleTypeDef` 오류가 나면:

1. `Core/Inc/stm32f4xx_hal_conf.h`에서 `HAL_TIM_MODULE_ENABLED`가 주석 해제되었는지 확인
2. `Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c`가 있는지 확인
3. `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h`가 있는지 확인

---

## 9. 보드에서 확인

1. 전원을 끈 상태에서 배선한다.
2. `Project` → `Build Project` (`Ctrl+B`)
3. `Run` → `Run` (`Ctrl+F11`)로 플래시
4. 시리얼 모니터를 **115200 8N1**로 연결
5. 보드의 검은 `RESET` 버튼을 누른다
6. 센서 정면에 평평한 물체를 둔다

### 9.1 기대 결과

- LD2가 0.5초마다 점멸한다. (시리얼 로그는 없음)
- 1초마다 `D,...` 행이 출력된다.
- 물체를 가까이/멀리 하면 `distance_mm`가 따라 변한다.
- 손을 센서 정면 가까이 대면 값이 줄어든다.
- 센서를 벽 쪽으로 돌리면 값이 커지고, 유효 범위를 벗어나면 `valid=0` 또는 `errors` 증가가 나타난다.

### 9.2 정확도 확인

센서 정면에서 **센서 앞면 기준**으로 거리를 잰다.

| 기준 거리 | 기대 |
|---:|---|
| 10 cm | 90 ~ 110 mm 근처 |
| 30 cm | 290 ~ 310 mm 근처 |
| 100 cm | 990 ~ 1010 mm 근처 |

- HC-SR04의 스펙은 ±3 mm 수준이지만, 실제로는 대상물 재질/각도/온도에 따라 ±1 cm 이상 차이가 날 수 있다.
- 평평하고 충분히 큰 물체(벽, 책, 판자)를 사용한다.
- 센서를 비스듬히 향하게 하면 반사가 약해져 오차가 커진다.
- 20 °C가 아닌 환경에서는 음속 차이로 약 `0.17 %/°C` 오차가 생긴다. 이 보정은 STEP03에서 한다.

---

## 10. 체크리스트

- [ ] 빌드 0 error / 0 warning
- [ ] 플래시 성공
- [ ] 시리얼에 STEP02 배너 출력
- [ ] LD2가 0.5초마다 점멸
- [ ] 1초마다 `D,...` CSV 출력
- [ ] 10 cm / 30 cm / 100 cm에서 값이 대략 맞음
- [ ] 물체를 치우면 `valid=0` 또는 `errors` 증가
- [ ] 보드 리셋 후에도 동일하게 동작
- [ ] ECHO에 5 V가 직접 들어가지 않음 (분압 확인)

---

## 11. 안 될 때

| 증상 | 확인 |
|---|---|
| 배너는 나오는데 `D` 행이 없음 | `EXTI0_IRQHandler` 추가 여부, PA0 배선, `HAL_GPIO_EXTI_Callback` 연결 |
| `valid=0`이고 `errors`만 증가 | ECHO가 PA0에 도달하는지, 분압 저항 값, 공통 GND, TRIG 펄스 확인 |
| 거리가 항상 0 | ECHO 상승/하강을 못 잡음. PA0 EXTI 설정과 배선 확인 |
| 거리가 항상 4000 근처 | 반사가 약하거나 유효 범위 밖. 대상물/각도 확인 |
| 값이 크게 튐 | 전원 불안정, 공통 GND, 센서 앞면 근처 장애물, 중앙값 창 확인 |
| 빌드 시 `unknown type name 'TIM_HandleTypeDef'` | `HAL_TIM_MODULE_ENABLED`와 TIM HAL 파일 확인 |
| 빌드 시 `undefined reference to HAL_TIM_Base_Init` | `stm32f4xx_hal_tim.c`가 빌드 대상에 포함되었는지 확인 |
| 시리얼 글자가 깨짐 | 115200 8N1, 올바른 COM 포트 확인 |
| HardFault | EXTI 콜백에서 잘못된 핸들 접근 여부, `Ultrasonic_Init()` 호출 순서 확인 |

---

## 12. 한계와 다음 단계

현재 구현의 한계:

- 음속을 20 °C 고정값으로 사용한다.
- 중앙값 5개는 단발 노이즈에는 강하지만, 연속적인 편향은 보정하지 못한다.
- HC-SR04는 최소 측정 거리(약 2 cm)와 최대 측정 거리(약 4 m) 제한이 있다.
- 센서 빔 각도(약 15°) 안의 물체 중 가장 가까운 것에 반응할 수 있다.

다음 `STEP03`에서는 온습도 센서를 추가해 실제 온도로 음속을 보정한다.

```text
v(m/s) = 331.3 + 0.606 × T(°C)
거리(mm) = 펄스폭(us) × v / 2000
```

그 다음 `STEP04`에서 LED, 초음파, UART 로그를 FreeRTOS 태스크로 분리한다.
이때 HAL timebase를 TIM6 등으로 옮겨 SysTick과 충돌하지 않게 해야 한다.

---

## 13. 스스로 해보기 (선택)

- `US_SAMPLE_PERIOD_MS`를 100, 500으로 바꿔 샘플 수와 출력 안정성을 비교한다.
- `US_FILTER_SIZE`를 3, 7로 바꿔 반응 속도와 노이즈 억제의 trade-off를 본다.
- `US_ECHO_TIMEOUT_MS`를 20, 50으로 바꿔 장애물이 없을 때 오류 처리 속도를 비교한다.
- 중앙값 대신 평균을 구현해 보고, 큰 이상값이 들어올 때 결과가 어떻게 달라지는지 비교한다.
- `errors`를 1초 구간 오류 수와 누적 오류 수로 나누어 출력해 본다.
