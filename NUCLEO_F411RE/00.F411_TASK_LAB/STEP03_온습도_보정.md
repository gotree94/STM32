# STEP03 - 온습도 센서 + 음속 보정

> 상태: **코드 구현/빌드 완료. 보드 배선/확인 대기.**
> 센서: DHT11 또는 DHT22 (자동 판별)
> DATA: PA4
> 읽기 주기: 2초 (0.5 Hz)
> 빌드: 0 errors, 0 warnings

---

## 1. 목표

- DHT11/DHT22로 온도와 습도를 읽는다.
- 온도로 음속을 계산해 초음파 거리를 보정한다.
- `E`의 `sound_speed_x10`과 `D`의 보정된 거리를 UART 로그로 확인한다.
- DHT 읽기가 초음파 측정을 방해하지 않도록 **초음파가 idle일 때만** 읽는다.

이번 단계에서 확정한 값:

| 항목 | 값 |
|---|---|
| 센서 | DHT11 / DHT22 자동 판별 |
| DATA 핀 | PA4 |
| 전원 | 3.3 V |
| 풀업 | 4.7 kΩ ~ 10 kΩ (모듈에 내장된 경우 생략) |
| 읽기 주기 | 2000 ms (0.5 Hz) |
| 시작 신호 | LOW 20 ms |
| 데이터 | 40 bit + checksum |
| 기본 음속 | 343.0 m/s (20 °C) |
| 보정 범위 | 250.0 ~ 400.0 m/s |
| 거리 공식 | `distance_mm = pulse_us × v_x10 / 20000` |

> DHT11은 온도 ±2 °C, 습도 ±5 %RH 수준이다.
> DHT22는 온도 ±0.5 °C, 습도 ±2 %RH 수준으로 보정 정확도가 더 좋다.
> 둘 다 이 코드로 읽을 수 있다.

---

## 2. 준비물

- NUCLEO-F411RE (STEP02까지 배선된 상태)
- DHT11 또는 DHT22 모듈
- 저항 4.7 kΩ 또는 10 kΩ 1개 (모듈에 풀업이 없을 때)
- 점퍼선

---

## 3. 배선

### 3.1 3핀 모듈

| 모듈 | NUCLEO |
|---|---|
| VCC | **3.3 V** |
| DATA | PA4 |
| GND | GND |

- DATA와 3.3 V 사이에 **4.7 kΩ ~ 10 kΩ 풀업**을 연결한다.
- 모듈에 이미 풀업 저항이 있으면 추가하지 않는다.

```text
3.3 V ---- 4.7 kΩ ----+---- PA4
                      |
                    DATA
```

### 3.2 4핀 DHT22 (AM2302)

| DHT22 핀 | 연결 |
|---|---|
| 1 VCC | 3.3 V |
| 2 DATA | PA4 |
| 3 NC | 연결하지 않음 |
| 4 GND | GND |

> 모듈마다 핀 순서가 다를 수 있다. **실크스크린의 VCC/DATA/GND 표시를 먼저 확인**한다.

### 3.3 전원 주의

- DHT11/DHT22는 3.3 V에서 동작한다.
- 5 V로 전원을 넣으면 DATA 출력도 5 V가 될 수 있어 STM32에 직접 연결하면 안 된다.
- 이번 단계는 **3.3 V 전원 + 3.3 V 풀업**으로 통일한다.
- GND는 NUCLEO와 공통으로 연결한다.

---

## 4. 설계

### 4.1 DHT 프로토콜

DHT 센서는 1-wire 방식으로 40 bit를 보낸다.

```text
MCU: DATA LOW 20 ms
MCU: DATA HIGH 30 us 후 입력으로 전환
DHT: LOW 80 us 응답
DHT: HIGH 80 us 응답
DHT: 40 bit 전송
     bit 0 = HIGH 26~28 us
     bit 1 = HIGH 70 us
DHT: checksum
```

40 bit 구성:

| 바이트 | DHT22 | DHT11 |
|---|---|---|
| 0 | 습도 상위 | 습도 정수 |
| 1 | 습도 하위 | 습도 소수 |
| 2 | 온도 상위 (부호 포함) | 온도 정수 |
| 3 | 온도 하위 | 온도 소수 |
| 4 | checksum | checksum |

checksum:

```text
(data[0] + data[1] + data[2] + data[3]) & 0xFF == data[4]
```

### 4.2 DHT11/DHT22 자동 판별

DHT22는 습도/온도를 16 bit로 보내고, DHT11은 정수+소수 바이트로 보낸다.

코드는 먼저 DHT22 형식으로 해석해 보고, 값이 유효 범위를 벗어나면 DHT11 형식으로 다시 해석한다.

- DHT22: 습도 0~1000 (0.1 %RH), 온도 -400~800 (0.1 °C)
- DHT11: 습도 0~1000 (0.1 %RH), 온도 0~800 (0.1 °C)

예:

| 원시 바이트 | DHT22 해석 | DHT11 해석 | 선택 |
|---|---|---|---|
| `01 F4 00 EB` | 500 = 50.0 %RH | 습도 소수 244 → 무효 | DHT22 |
| `2D 00 17 00` | 11520 → 무효 | 450 = 45.0 %RH | DHT11 |

### 4.3 왜 2초마다 읽는가

- DHT22는 최소 2초 간격을 권장한다.
- DHT11은 최소 1초 간격을 권장한다.
- 2초 간격이면 둘 다 안전하다.

### 4.4 블로킹 구간

DHT 읽기는 시작 신호 20 ms + 40 bit 수신 약 4 ms로 **약 25 ms 블로킹**된다.
그래서 다음 조건을 지킨다.

- `Ultrasonic_IsIdle()`이 참일 때만 `Dht_Task()`를 호출한다.
- 초음파 측정 중(ECHO 대기 중)에는 DHT를 읽지 않는다.
- DHT 읽기 중에는 초음파 상태 머신이 멈추지만, 이미 idle이므로 ECHO를 놓치지 않는다.

LED는 DHT 읽기 동안 최대 25 ms 늦어질 수 있다. 0.5초 주기에서는 눈에 띄지 않는다.

### 4.5 음속 보정

온도에 따른 음속:

```text
v(m/s) = 331.3 + 0.606 × T(°C)
```

정수 연산을 위해 온도를 0.1 °C 단위(`temp_x10`)로 유지한다.

```text
v_x10 = 3313 + (606 × temp_x10) / 1000
```

예: 23.5 °C

```text
temp_x10 = 235
v_x10 = 3313 + (606 × 235) / 1000
      = 3313 + 142
      = 3455   → 345.5 m/s
```

거리 계산:

```text
distance_mm = pulse_us × v_x10 / 20000
```

기존 343.0 m/s 고정값 대비 23.5 °C에서는 약 0.7 % 거리가 길게 보정된다.
1.685 m에서 약 12 mm 차이다.

---

## 5. 변경된 파일

| 파일 | 변경 |
|---|---|
| `Core/Inc/dht.h` | 새 파일. DHT 결과 구조체와 API |
| `Core/Src/dht.c` | 새 파일. GPIO 전환, 40 bit 읽기, checksum, 자동 판별 |
| `Core/Inc/ultrasonic.h` | `Ultrasonic_IsIdle()`, `Ultrasonic_SetSoundSpeedX10()` 추가 |
| `Core/Src/ultrasonic.c` | 음속 변수 추가, 거리 계산에 적용 |
| `Core/Src/main.c` | `Dht_Init()`, `Environment_Task()`, `E` CSV 출력, 음속 갱신 |
| `F411_TASK_LAB.ioc` | 변경 없음 |

> TIM2는 STEP02에서 이미 1 MHz로 동작 중이다.
> DHT는 이 TIM2 카운터를 마이크로초 시간 기준으로 재사용한다.
> 따라서 `Ultrasonic_Init()`을 먼저 호출한 뒤 `Dht_Init()`을 호출해야 한다.

---

## 6. 코드

### 6.1 DHT API (`dht.h`)

```c
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
```

- 온도/습도는 정수 10배 값으로 저장한다.
  - `235` = 23.5 °C
  - `512` = 51.2 %RH
- `snprintf`의 `%f`는 현재 빌드(`nano.specs`)에서 기본 지원되지 않으므로 정수로 출력한다.

### 6.2 DHT 핀 설정 (`dht.c`)

```c
#define DHT_PORT GPIOA
#define DHT_PIN  GPIO_PIN_4

#define DHT_READ_PERIOD_MS       2000U
#define DHT_START_LOW_US         20000U
#define DHT_RELEASE_US           30U
#define DHT_RESPONSE_TIMEOUT_US  200U
#define DHT_BIT_TIMEOUT_US       200U
#define DHT_ONE_THRESHOLD_US     50U
```

- 시작 신호는 20 ms LOW로 둔다. DHT11(최소 18 ms)과 DHT22(최소 1 ms)를 모두 만족한다.
- bit 1/0 구분 기준은 HIGH 폭 50 us다.

### 6.3 음속 적용 (`ultrasonic.c`)

```c
static uint16_t s_soundSpeedX10;

void Ultrasonic_SetSoundSpeedX10(uint16_t speed_x10)
{
  if ((speed_x10 >= 2500U) && (speed_x10 <= 4000U))
  {
    s_soundSpeedX10 = speed_x10;
  }
}

static void US_StoreSample(uint32_t width_us)
{
  uint32_t distance_mm = (width_us * s_soundSpeedX10) / 20000U;
  ...
}
```

초기값은 `3430` (343.0 m/s)이다.

### 6.4 메인 루프 (`main.c`)

```c
Ultrasonic_Init();
Dht_Init();

while (1)
{
  LED_Task();
  Sensor_Task();
  Environment_Task();
}
```

`Environment_Task()`:

```c
static void Environment_Task(void)
{
  Dht_Result env;

  if (Ultrasonic_IsIdle() == 0U)
  {
    return;
  }

  Dht_Task();

  if (Dht_GetResult(&env) == 0U)
  {
    return;
  }

  if (env.valid != 0U)
  {
    int32_t speed_x10 = 3313 + (606 * (int32_t)env.temperature_c_x10) / 1000;
    ...
    Ultrasonic_SetSoundSpeedX10((uint16_t)speed_x10);
  }

  /* E CSV 출력 */
}
```

---

## 7. 출력 형식

기존 거리 CSV는 그대로 유지하고, 환경 CSV를 추가한다.

### 7.1 거리

```text
D,t_ms,distance_mm,valid,errors
```

### 7.2 환경

```text
E,t_ms,temp_c_x10,humidity_rh_x10,valid,errors,sound_speed_x10
```

| 필드 | 의미 |
|---|---|
| `E` | 환경 데이터 행 |
| `t_ms` | 읽기 시각 (ms) |
| `temp_c_x10` | 온도 × 10 (235 = 23.5 °C) |
| `humidity_rh_x10` | 습도 × 10 (512 = 51.2 %RH) |
| `valid` | 1 = 읽기 성공, 0 = 실패 |
| `errors` | 누적 DHT 오류 횟수 |
| `sound_speed_x10` | 적용한 음속 × 10 (3455 = 345.5 m/s), 실패 시 0 |

예시:

```text
=== F411_TASK_LAB Step 3: Temperature Compensation ===
HC-SR04: TRIG=PA1, ECHO=PA0, 5 Hz sample, 1 Hz median
DHT11/DHT22: DATA=PA4, 0.5 Hz, sound speed compensation
D,1000,1685,1,0
D,2000,1685,1,0
E,2020,235,512,1,0,3455
D,3000,1697,1,0
D,4000,1697,1,0
E,4020,235,513,1,0,3455
...
```

- 첫 `E`가 출력되기 전까지는 기본 음속 343.0 m/s를 사용한다.
- `E`가 출력된 다음 초부터 보정된 음속이 거리 계산에 적용된다.
- DHT 읽기 실패 시 `valid=0`이 출력되고, 마지막으로 성공한 음속을 계속 사용한다.

---

## 8. 빌드 결과

헤드리스 빌드로 검증했다.

```text
Build Finished. 0 errors, 0 warnings.

text    data    bss     dec     hex    filename
15480     92    2148   17720    4538   F411_TASK_LAB.elf
```

---

## 9. 보드에서 확인

1. 전원을 끈 상태에서 DHT를 PA4에 배선한다.
2. DATA-PA4 사이 풀업이 있는지 확인한다. (모듈 내장이면 생략)
3. 빌드 후 플래시한다.
4. 시리얼 모니터를 **115200 8N1**로 연결한다.
5. 보드 RESET 후 2초 정도 기다린다.

### 9.1 기대 결과

- 1초마다 `D,...` 거리 CSV가 출력된다.
- 2초마다 `E,...` 환경 CSV가 출력된다.
- `E`의 `valid=1`이고 `sound_speed_x10`이 3400~3500 근처로 나온다.
- 실온 23 °C 근처라면 `temp_c_x10`이 220~240 근처다.
- 센서에 입김을 불면 `humidity_rh_x10`이 올라간다.
- 초음파 거리는 `E` 출력 이후부터 약간 길게 보정된다.

### 9.2 보정 확인

실온이 23.5 °C이고 실제 거리가 1.685 m라면:

| 항목 | 값 |
|---|---|
| 기본 음속 | 343.0 m/s |
| 보정 음속 | 345.5 m/s |
| 보정 전 거리 | 약 1685 mm |
| 보정 후 거리 | 약 1697 mm |

온도 차이가 10 °C 정도 나면 차이가 더 커진다.
(20 °C vs 30 °C에서 약 1.9 %, 1.685 m 기준 약 32 mm)

---

## 10. 체크리스트

- [ ] 빌드 0 error / 0 warning
- [ ] 플래시 성공
- [ ] 시리얼에 STEP03 배너 출력
- [ ] 1초마다 `D,...` 출력
- [ ] 2초마다 `E,...` 출력
- [ ] `E`의 `valid=1`
- [ ] 온도/습도가 상식적인 범위
- [ ] 입김을 불면 습도가 변함
- [ ] `sound_speed_x10`이 3400~3500 근처
- [ ] `E` 출력 후 거리 값이 보정됨
- [ ] LD2가 계속 0.5초마다 점멸
- [ ] DHT 오류가 계속 증가하지 않음

---

## 11. 안 될 때

| 증상 | 확인 |
|---|---|
| `E` 행이 아예 없음 | `Dht_Init()` 호출, `Environment_Task()` 루프 호출, `Ultrasonic_IsIdle()` 조건 확인 |
| `E`의 `valid=0`, `errors` 증가 | DATA-PA4 배선, 4.7 kΩ 풀업, 3.3 V 전원, 공통 GND |
| `E`의 `valid=0`이 반복됨 | 센서 전원 인가 후 1~2초 대기, DHT11/DHT22 모델 확인, 배선 길이 짧게 |
| 온도/습도가 0 | checksum 실패 또는 자동 판별 실패. 배선/풀업 먼저 확인 |
| 습도가 45.0 대신 4.5로 보임 | DHT11 원시값 해석 문제. `E`의 `humidity_rh_x10`이 450인지 확인 (450 = 45.0 %) |
| 거리가 갑자기 커짐 | 음속 보정이 적용된 것. `E`의 `sound_speed_x10`과 비교 |
| DHT 읽을 때 LED가 살짝 끊김 | 정상. 약 25 ms 블로킹 구간이 2초마다 한 번 있음 |
| `D` 행은 나오는데 `E`가 늦게 나옴 | 첫 DHT 읽기는 부팅 후 2초 뒤에 시작됨 |
| 빌드 오류 `DHT_PORT` 미정의 | `dht.c`의 `#include "main.h"` 확인 |
| HardFault | `Ultrasonic_Init()`보다 `Dht_Init()`을 먼저 호출했는지 확인 (TIM2 미동작) |

---

## 12. 한계와 다음 단계

현재 구현의 한계:

- DHT 읽기 구간(약 25 ms)은 블로킹이다.
- DHT11은 정확도가 낮아 보정 효과가 과소/과대 평가될 수 있다.
- 습도는 거리 계산에 직접 쓰지 않고 로그로만 출력한다.
- 센서 자체의 최소/최대 거리와 빔 각도 한계는 그대로다.

다음 `STEP04`에서는 LED, 초음파, DHT, UART를 FreeRTOS 태스크로 분리한다.

- LED 태스크: 500 ms 주기
- 초음파 태스크: 200 ms 샘플링 + 1초 중앙값
- DHT 태스크: 2초 주기, 블로킹 읽기
- 로그 태스크: 큐 또는 스트림 버퍼로 CSV 출력

FreeRTOS를 쓰면 HAL timebase를 TIM6 등으로 옮겨 SysTick과 충돌하지 않게 해야 한다.
DHT의 25 ms 블로킹 읽기도 별도 태스크로 분리해 LED/초음파 지터를 줄일 수 있다.

---

## 13. 스스로 해보기 (선택)

- DHT 읽기 주기를 2초에서 5초로 바꿔 오류율과 반응 속도를 비교한다.
- `E` 행에 `v_x10` 대신 `v(m/s)`를 소수점으로 출력해 본다.
- DHT11과 DHT22를 각각 연결해 온도 정확도 차이를 비교한다.
- 음속 보정을 잠시 끄고(`Ultrasonic_SetSoundSpeedX10(3430)`) 보정 전/후 거리를 비교한다.
- DHT 읽기 전후의 `HAL_GetTick()` 차이를 출력해 블로킹 시간을 직접 측정한다.
