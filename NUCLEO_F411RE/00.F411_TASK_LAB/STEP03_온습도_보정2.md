# F411_LAB 초음파 + 온습도(DHT) 정확도 개선 작업 보고서

- 대상 프로젝트: `C:\Users\Administrator\STM32CubeIDE\workspace_2.2.0\F411_LAB`
- 기준: 초음파(HC-SR04) 거리 측정 및 TRACE가 완성된 기존 코드를 **메인**으로 유지
- 방식: 첨부된 `dht.c` / `dht.h`를 **메인 코드에 맞게 수정**하여 통합
- 결과: 빌드 성공(`F411_LAB.elf`, text 19612 / data 104 / bss 2176 bytes), 경고 없음

---

## 1. 기존 코드 분석 결과

| 항목 | 기존 메인 코드 상태 |
|---|---|
| MCU | STM32F411RETx, HSI → PLL 84 MHz |
| 초음파 | `TIM1` (PCLK2 84 MHz / 분주 84 = **1 MHz**, 16비트 ARR 65535), TRIG=PA1, ECHO=PA0(EXTI0) |
| 시리얼 | USART2 115200 baud, PA2/PA3, `printf` → `__io_putchar` |
| TRACE | `D,...`(거리/유효/에러), `C,...`(에코 펄스 폭) 유지 |
| 미사용 핀 | **PA4** (USART, SWD, TRIG/ECHO와 충돌 없음) → DHT DATA로 사용 |
| 미사용 타이머 | 첨부 소스가 사용하던 `TIM2`는 메인에서 초기화되지 않음 → **사용 불가** |

### 핵심 판단
첨부 소스는 마이크로초 시간 기준으로 `TIM2->CNT`를 직접 읽었지만, 메인 프로젝트에는 TIM2 초기화가 없다.
메인을 고치지 않고 **첨부 소스 쪽을 TIM1(이미 1 MHz로 구동 중)에 맞추는 것**이 요구사항에 부합하며,
타이머 추가 초기화·핀 충돌·리소스 중복을 모두 피할 수 있다.

---

## 2. 핀 / 타이머 매핑 (최종)

| 신호 | 핀 | 용도 | 비고 |
|---|---|---|---|
| ECHO | PA0 | HC-SR04 에코 | EXTI0, 기존 유지 |
| TRIG | PA1 | HC-SR04 트리거 | 기존 유지 |
| USART2_TX | PA2 | 시리얼 출력 | 기존 유지 |
| USART2_RX | PA3 | 시리얼 수신 | 기존 유지 |
| **DHT DATA** | **PA4** | 온습도 단일선 데이터 | 내부 풀업 사용, Open-Drain으로 전환 |
| LD2 | PA5 | 보드 LED | 기존 유지 |
| 시간 기준 | TIM1 | 1 MHz 카운터 | 초음파·DHT 공용 |

---

## 3. 변경 파일 목록

| 파일 | 변경 내용 |
|---|---|
| `Core/Inc/dht.h` | 신규 추가 (첨부 파일 그대로, `Dht_Result` / `Dht_Init` / `Dht_Task` / `Dht_GetResult`) |
| `Core/Src/dht.c` | 신규 추가 + 메인 맞춤 수정 (아래 3.1) |
| `Core/Src/main.c` | DHT 초기화·태스크 호출, 음속 보정, 시리얼 출력 추가 (아래 3.2) |
| `Debug/Core/Src/subdir.mk` | `dht.c`를 빌드 소스/오브젝트/의존성/clean에 추가 |
| `Debug/objects.list` | 링커 입력에 `./Core/Src/dht.o` 추가 |


### dht.h

```c
#ifndef DHT_H
#define DHT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct
{
  uint32_t timestamp_ms;
  uint32_t errors;
  int16_t temperature_c_x10;
  uint16_t humidity_rh_x10;
  uint8_t valid;
} Dht_Result;

void Dht_Init(void);
void Dht_Task(void);
uint8_t Dht_GetResult(Dht_Result *result);

#ifdef __cplusplus
}
#endif

#endif
```

### dht.c

```c
#include "dht.h"
#include "main.h"

extern TIM_HandleTypeDef htim1;

#define DHT_PORT GPIOA
#define DHT_PIN GPIO_PIN_4

#define DHT_TIMER_TICKS_PER_US 1U

#define DHT_READ_PERIOD_MS 2000U
#define DHT_START_LOW_US 20000U
#define DHT_RELEASE_US 30U
#define DHT_RESPONSE_TIMEOUT_US 200U
#define DHT_BIT_TIMEOUT_US 200U
#define DHT_ONE_THRESHOLD_US 50U

static uint32_t s_lastReadTick;
static uint32_t s_errorCount;
static Dht_Result s_result;
static volatile uint8_t s_resultReady;

static void DHT_SetPinOutput(void);
static void DHT_SetPinInput(void);
static uint32_t DHT_NowUs(void);
static uint32_t DHT_ElapsedUs(uint32_t start);
static void DHT_DelayUs(uint32_t us);
static uint8_t DHT_WaitLevel(GPIO_PinState level, uint32_t timeout_us, uint32_t *duration);
static uint8_t DHT_ReadRaw(uint8_t *data);
static uint8_t DHT_Decode(const uint8_t *data, int32_t *temperature_c_x10, uint32_t *humidity_rh_x10);

void Dht_Init(void)
{
  GPIO_InitTypeDef gpio = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();

  gpio.Pin = DHT_PIN;
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Pull = GPIO_PULLUP;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT_PORT, &gpio);

  s_lastReadTick = HAL_GetTick();
  s_errorCount = 0U;
  s_resultReady = 0U;
  s_result.timestamp_ms = 0U;
  s_result.errors = 0U;
  s_result.temperature_c_x10 = 0;
  s_result.humidity_rh_x10 = 0U;
  s_result.valid = 0U;
}

void Dht_Task(void)
{
  uint32_t now = HAL_GetTick();
  uint8_t data[5];
  int32_t temperature_c_x10;
  uint32_t humidity_rh_x10;
  uint8_t ok = 0U;

  if ((now - s_lastReadTick) < DHT_READ_PERIOD_MS)
  {
    return;
  }
  s_lastReadTick = now;

  DHT_SetPinOutput();
  HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_RESET);
  DHT_DelayUs(DHT_START_LOW_US);
  HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_SET);
  DHT_DelayUs(DHT_RELEASE_US);
  DHT_SetPinInput();

  if (DHT_ReadRaw(data) != 0U)
  {
    uint8_t checksum = (uint8_t)(data[0] + data[1] + data[2] + data[3]);

    if (checksum == data[4])
    {
      if (DHT_Decode(data, &temperature_c_x10, &humidity_rh_x10) != 0U)
      {
        s_result.temperature_c_x10 = (int16_t)temperature_c_x10;
        s_result.humidity_rh_x10 = (uint16_t)humidity_rh_x10;
        s_result.valid = 1U;
        ok = 1U;
      }
    }
  }

  if (ok == 0U)
  {
    s_result.valid = 0U;
    s_errorCount++;
  }

  s_result.timestamp_ms = now;
  s_result.errors = s_errorCount;
  s_resultReady = 1U;
}

uint8_t Dht_GetResult(Dht_Result *result)
{
  if ((result == NULL) || (s_resultReady == 0U))
  {
    return 0U;
  }

  *result = s_result;
  s_resultReady = 0U;
  return 1U;
}

static void DHT_SetPinOutput(void)
{
  GPIO_InitTypeDef gpio = {0};

  gpio.Pin = DHT_PIN;
  gpio.Mode = GPIO_MODE_OUTPUT_OD;
  gpio.Pull = GPIO_PULLUP;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT_PORT, &gpio);
}

static void DHT_SetPinInput(void)
{
  GPIO_InitTypeDef gpio = {0};

  gpio.Pin = DHT_PIN;
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Pull = GPIO_PULLUP;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT_PORT, &gpio);
}

static uint32_t DHT_NowUs(void)
{
  return (uint32_t)(__HAL_TIM_GET_COUNTER(&htim1) / DHT_TIMER_TICKS_PER_US);
}

static uint32_t DHT_ElapsedUs(uint32_t start)
{
  return (uint16_t)(DHT_NowUs() - start);
}

static void DHT_DelayUs(uint32_t us)
{
  uint32_t start = DHT_NowUs();

  while (DHT_ElapsedUs(start) < us)
  {
  }
}

static uint8_t DHT_WaitLevel(GPIO_PinState level, uint32_t timeout_us, uint32_t *duration)
{
  uint32_t start = DHT_NowUs();

  while (HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN) != level)
  {
    if (DHT_ElapsedUs(start) >= timeout_us)
    {
      return 0U;
    }
  }

  if (duration != NULL)
  {
    *duration = DHT_ElapsedUs(start);
  }

  return 1U;
}

static uint8_t DHT_ReadRaw(uint8_t *data)
{
  uint32_t duration;
  uint8_t i;
  uint8_t bit;

  for (i = 0U; i < 5U; i++)
  {
    data[i] = 0U;
  }

  if (DHT_WaitLevel(GPIO_PIN_RESET, DHT_RESPONSE_TIMEOUT_US, NULL) == 0U)
  {
    return 0U;
  }
  if (DHT_WaitLevel(GPIO_PIN_SET, DHT_RESPONSE_TIMEOUT_US, NULL) == 0U)
  {
    return 0U;
  }

  for (i = 0U; i < 40U; i++)
  {
    if (DHT_WaitLevel(GPIO_PIN_RESET, DHT_BIT_TIMEOUT_US, NULL) == 0U)
    {
      return 0U;
    }
    if (DHT_WaitLevel(GPIO_PIN_SET, DHT_BIT_TIMEOUT_US, NULL) == 0U)
    {
      return 0U;
    }
    if (DHT_WaitLevel(GPIO_PIN_RESET, DHT_BIT_TIMEOUT_US, &duration) == 0U)
    {
      return 0U;
    }

    bit = (duration > DHT_ONE_THRESHOLD_US) ? 1U : 0U;
    data[i / 8U] = (uint8_t)((data[i / 8U] << 1U) | bit);
  }

  return 1U;
}

static uint8_t DHT_Decode(const uint8_t *data, int32_t *temperature_c_x10, uint32_t *humidity_rh_x10)
{
  uint32_t humidity;
  int32_t temperature;

  humidity = ((uint32_t)data[0] << 8) | data[1];
  temperature = (int32_t)(((uint16_t)(data[2] & 0x7FU) << 8) | data[3]);
  if ((data[2] & 0x80U) != 0U)
  {
    temperature = -temperature;
  }

  if ((humidity <= 1000U) && (temperature >= -400) && (temperature <= 800))
  {
    *humidity_rh_x10 = humidity;
    *temperature_c_x10 = temperature;
    return 1U;
  }

  if ((data[1] <= 9U) && (data[3] <= 9U))
  {
    humidity = ((uint32_t)data[0] * 10U) + data[1];
    temperature = ((int32_t)data[2] * 10) + (int32_t)data[3];

    if ((humidity <= 1000U) && (temperature >= 0) && (temperature <= 800))
    {
      *humidity_rh_x10 = humidity;
      *temperature_c_x10 = temperature;
      return 1U;
    }
  }

  return 0U;
}
```

### main.c

```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "ultrasonic.h"
#include "dht.h"
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
  {
    HAL_UART_Transmit(&huart2, (uint8_t *)"\r", 1, 0xFFFF);
  }
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}

static uint16_t Sensor_SoundSpeedX10(int16_t temperature_c_x10, uint16_t humidity_rh_x10)
{
  int32_t speed_x10;

  speed_x10 = 3313 + ((606 * (int32_t)temperature_c_x10) / 1000);
  speed_x10 += (12 * (int32_t)humidity_rh_x10) / 1000;

  if (speed_x10 < 2500)
  {
    speed_x10 = 2500;
  }
  else if (speed_x10 > 4000)
  {
    speed_x10 = 4000;
  }

  return (uint16_t)speed_x10;
}

static void Sensor_Task(void)
{
  Ultrasonic_Result result;
  Dht_Result dht;
  char line[96];
  int32_t temperature;

  Dht_Task();
  Ultrasonic_Task();

  if (Dht_GetResult(&dht) != 0U)
  {
    temperature = (int32_t)dht.temperature_c_x10;
    (void)snprintf(line, sizeof(line), "TH,%lu,%d,%u,%u,%lu\n",
                   (unsigned long)dht.timestamp_ms,
                   (int)temperature,
                   (unsigned int)dht.humidity_rh_x10,
                   (unsigned int)dht.valid,
                   (unsigned long)dht.errors);
    printf("%s", line);
    printf("ENV,%c%u.%uC,%u.%u%%RH\n",
           (temperature < 0) ? '-' : '+',
           (unsigned int)((temperature < 0) ? -temperature : temperature) / 10U,
           (unsigned int)((temperature < 0) ? -temperature : temperature) % 10U,
           (unsigned int)dht.humidity_rh_x10 / 10U,
           (unsigned int)dht.humidity_rh_x10 % 10U);

    if (dht.valid != 0U)
    {
      uint16_t speed_x10 = Sensor_SoundSpeedX10(dht.temperature_c_x10,
                                                dht.humidity_rh_x10);
      Ultrasonic_SetSoundSpeedX10(speed_x10);
      printf("S,%u\n", (unsigned int)speed_x10);
    }
  }

  if (Ultrasonic_GetResult(&result) != 0U)
  {
    (void)snprintf(line, sizeof(line), "D,%lu,%u,%u,%lu\n",
                   (unsigned long)result.timestamp_ms,
                   (unsigned int)result.distance_mm,
                   (unsigned int)result.valid,
                   (unsigned long)result.errors);
    printf("%s", line);
    printf("C,%u\n", (unsigned int)Ultrasonic_GetLastEchoTicks());
  }
}
/* USER CODE END 0 */
```

```c
  /* USER CODE BEGIN 2 */
  Ultrasonic_Init();
  Dht_Init();
  printf("\n=== F411_TASK_LAB Step 3: Ultrasonic + DHT ===\n");
  printf("HC-SR04: TRIG=PA1, ECHO=PA0, 5 Hz sample, 1 Hz median\n");
  printf("DHTxx: DATA=PA4, 2 s sample, TIM1 1 MHz timebase\n");
  printf("TIM1: PCLK2=%luHz divider=%lu ARR=%lu tick=%luHz\n",
         (unsigned long)HAL_RCC_GetPCLK2Freq(),
         (unsigned long)(htim1.Init.Prescaler + 1U),
         (unsigned long)htim1.Init.Period,
         (unsigned long)(HAL_RCC_GetPCLK2Freq() / (htim1.Init.Prescaler + 1U)));
  /* USER CODE END 2 */
```

```c
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    Sensor_Task();
  }
  /* USER CODE END 3 */
```


> Eclipse/STM32CubeIDE에서 다시 빌드하면 `subdir.mk`와 `objects.list`는 `.cproject`의 `Core` 소스 경로로부터 자동 재생성되며, `Core/Src/dht.c`가 자동 포함된다. 위 수정은 커맨드라인 `make` 즉시 빌드를 위해 반영한 것이다.

### 3.1 `dht.c` 수정 내용 (첨부 소스 → 메인 적합화)

1. **타이머 기준 교체**
   - `TIM2->CNT` 제거 → `__HAL_TIM_GET_COUNTER(&htim1)` 사용 (`extern TIM_HandleTypeDef htim1;` 추가)
   - `DHT_TIMER_TICKS_PER_US = 1U` 정의로 메인의 1 MHz 설정을 명시
   - `Ultrasonic_Init()`에서 TIM1을 먼저 시작하므로 `Dht_Init()`은 그 이후에 호출

2. **16비트 카운터 랩어라운드 처리**
   - TIM1은 16비트(65.536 ms 주기)이므로 기존의 `uint32_t` 뺄셈은 오버플로 시 오동작
   - `DHT_ElapsedUs(start)`를 추가하여 `(uint16_t)(now - start)`로 모든 지연/타임아웃/비트 폭 측정 통일
   - 20 ms 시작 신호와 40비트 수신(약 4~5 ms)이 카운터 경계를 지나도 정상 동작

3. **나머지 로직은 첨부 소스 유지**
   - 20 ms LOW → 30 µs release → 응답 검출 → 40비트 MSB-first 수신
   - HIGH 펄스 폭이 50 µs 초과이면 `1`, 이하이면 `0`
   - 체크섬 검증, DHT22(16비트) / DHT11(정수+소수 8비트) 자동 디코딩, 범위 필터링 유지
   - 2초 주기, 에러 카운트, `valid` 플래그, `Dht_GetResult()` 원-샷 전달 구조 유지

### 3.2 `main.c` 수정 내용

1. `#include "dht.h"` 추가 (`Core/Src/main.c:26`)
2. 초기화 순서: `Ultrasonic_Init();` → `Dht_Init();` (`Core/Src/main.c:181-182`)
3. 부팅 배너를 Step 3로 갱신하고 DHT 핀/타이머 정보 출력 (`Core/Src/main.c:183-185`)
4. `Sensor_Task()`에서 `Dht_Task()`를 `Ultrasonic_Task()` 앞에 호출 (`Core/Src/main.c:107`)
5. 음속 보정 함수 `Sensor_SoundSpeedX10()` 추가 (`Core/Src/main.c:81-98`)
6. DHT 결과를 시리얼로 출력하고, 유효할 때만 음속을 갱신 (`Core/Src/main.c:110-134`)

---

## 4. 정확도 개선 원리 (음속 보정)

초음파 거리는 `거리 = (에코시간 × 음속) / 2` 이므로 음속 오차가 그대로 거리 오차가 된다.
기존은 고정 음속 3430 ×0.1 m/s(20 °C 기준)를 사용했으나, 온도/습도에 따라 실제 음속은 변한다.

```
v [0.1 m/s] = 3313 + (606 × T_x10) / 1000 + (12 × RH_x10) / 1000
```

- `T_x10` = 온도 ×10 (0.1 °C 단위), `RH_x10` = 습도 ×10 (0.1 %RH 단위)
- 온도항: 0.606 m/s/°C, 습도항: 약 0.0124 m/s/%RH 를 정수 연산으로 근사
- 예) 20.5 °C, 50.3 %RH → 약 3441 (344.1 m/s) → `Ultrasonic_SetSoundSpeedX10()`에 반영
- 보정 결과는 `distance_ref_mm`(고정 음속)와 `distance_mm`(보정 음속)를 함께 두어 비교 가능
- `2500 ~ 4000` 범위 클램프로 비정상 값이 거리 계산에 반영되지 않도록 보호

---

## 5. 시리얼 출력 포맷 (115200 baud)

기존 TRACE는 그대로 유지하고, 온습도 라인을 추가했다.

| 프리픽스 | 형식 | 설명 |
|---|---|---|
| `D` | `D,<time_ms>,<distance_mm>,<valid>,<errors>` | 기존: 보정 음속 적용 거리 (1 Hz) |
| `C` | `C,<echo_ticks>` | 기존: 마지막 에코 펄스 폭 (1 MHz 카운트 = µs) |
| **`TH`** | `TH,<time_ms>,<temp_x10>,<rh_x10>,<valid>,<errors>` | **신규: 온도/습도 원시값(×10), 2 초 주기** |
| **`ENV`** | `ENV,<+/-><T>.<t>C,<RH>.<r>%RH` | **신규: 사람이 읽기 쉬운 온습도** |
| **`S`** | `S,<speed_x10>` | **신규: DHT 유효 시 적용된 음속 (유효하지 않으면 미출력)** |

출력 예시:

```
=== F411_TASK_LAB Step 3: Ultrasonic + DHT ===
HC-SR04: TRIG=PA1, ECHO=PA0, 5 Hz sample, 1 Hz median
DHTxx: DATA=PA4, 2 s sample, TIM1 1 MHz timebase
D,10234,512,1,0
C,2981
TH,10400,205,503,1,0
ENV,+20.5C,50.3%RH
S,3441
D,11234,510,1,0
C,2968
```

---

## 6. 빌드 및 검증

```powershell
$env:PATH = "C:\ST\STM32CubeIDE_2.2.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.14.3.rel1.win32_1.0.100.202602081740\tools\bin;" + $env:PATH
make all
```

- `-Wall -fsyntax-only` 검사 결과: 경고/에러 없음
- 링크 성공: `Finished building target: F411_LAB.elf`
- STM32CubeIDE에서는 `Project > Build Project` 또는 `Ctrl+B`로 동일하게 빌드 가능

실측 확인 방법:
1. 보드에 다운로드 후 시리얼 모니터를 115200-8N1로 설정
2. 부팅 배너에서 `Step 3`과 `DHTxx: DATA=PA4` 확인
3. 약 2초 뒤 `TH`, `ENV`, `S` 라인이 출력되는지 확인
4. `S` 값이 온도 변화에 따라 변하고, `D` 거리가 온도 보정 전(`distance_ref_mm`)과 달라지는지 확인
5. DHT 미연결/오류 시 `TH`의 `valid=0`, `errors` 증가 확인 (음속은 마지막 유효 값 유지)

---

## 7. 주의사항

- **배선**: DHT의 DATA는 PA4, VCC는 3.3 V(또는 모듈 사양에 맞는 전원), GND는 공통. 데이터선에 4.7 kΩ 풀업을 권장(내부 풀업도 활성화되어 있음).
- **전원 후 안정화**: DHT는 전원 인가 직후 수 초간 불안정할 수 있으므로 부팅 직후 첫 1~2회 측정은 무시해도 된다.
- **블로킹 시간**: 1회 DHT 읽기 약 24 ms(20 ms 시작 신호 포함). 초음파 샘플 주기 200 ms / 에코 타임아웃 35 ms보다 짧아 충돌하지 않는다.
- **인터럽트**: DHT 비트 판정 중 ECHO EXTI가 들어올 수 있으나 펄스 폭은 TIM1 카운터로 측정되므로 영향이 제한적이다.
- **DHT11/22 자동 대응**: 디코더가 두 포맷을 모두 지원하므로 센서교체 시 코드 수정이 필요 없다.
- **유효 범위**: 온도 -40~80 °C, 습도 0~100 %RH. 범위를 벗어나면 `valid=0`이 되고 음속은 갱신되지 않는다.
