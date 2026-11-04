# STEP01 - LED 점멸

> 상태: **코드 구현/빌드 완료. 보드 확인 대기.**
> 이 단계에서는 센서와 RTOS를 넣지 않는다.

---

## 1. 목표

- LD2(PA5)를 500 ms 주기로 켜고 끈다.
- USART2(115200)로 `LED ON` / `LED OFF` 로그를 출력한다.
- 다음 단계에서 센서를 추가할 수 있도록 **CPU를 블로킹하지 않는 구조**로 만든다.

---

## 2. 준비물

- NUCLEO-F411RE
- USB 케이블 (ST-LINK, 데이터 지원)
- STM32CubeIDE 2.2.0
- 시리얼 프로그램 (Tera Term / PuTTY / VS Code)

---

## 3. 설계 선택: 왜 `HAL_Delay`를 쓰지 않았나

가장 단순한 점멸은 다음과 같다.

```c
while (1)
{
  HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
  HAL_Delay(500);
}
```

동작은 하지만 500 ms 동안 CPU가 아무 일도 못 한다.
나중에 초음파 센서를 추가하면:

- 센서 측정이 LED 주기에 끌려다니고
- UART 로그가 겹치면 주기가 흔들리며
- "몇 초마다 정확한 데이터" 요구사항을 맞출 수 없다.

그래서 이번 단계부터 **"마지막 실행 시각 + 주기"** 패턴을 쓴다.

```c
if ((now - last) >= period)
{
  last = now;
  /* 주기 작업 */
}
```

이 패턴은 나중에 FreeRTOS의 `vTaskDelayUntil()`과 같은 개념이다.
지금 만드는 `LED_Task()`가 나중에 그대로 RTOS 태스크가 된다.

### 3.1 여러 태스크를 넣을 때: 주기 보장 조건

슈퍼루프에서 한 바퀴 도는 시간을 `P`라 하면:

```text
P = C_LED + C_SENSOR + C_UART + ...
```

- `C`는 **최악 실행시간(WCET)** 이다. 평균 시간이 아니다.
- `P`가 LED 주기(500 ms)보다 충분히 작아야 다른 태스크를 넣어도 LED가 제때 실행된다.
- 하지만 **"다른 태스크 합 < 500 ms"만으로는 부족하다.**

추가로 확인할 것:

- 태스크별 주기와 **마감시간(deadline)**
- `HAL_Delay()`, 긴 `HAL_UART_Transmit()` 같은 **blocking 호출**
- 인터럽트/ISR 실행 시간
- 허용 jitter와 CPU 여유율 (보통 50~70% 이하)

예시:

| 태스크 | WCET | 주기 | 사용률 |
|---|---:|---:|---:|
| LED | 1 ms | 500 ms | 0.2% |
| 초음파 | 20 ms | 100 ms | 20% |
| UART | 10 ms | 50 ms | 20% |
| 합계 | | | **40.2%** |

이 정도면 여유가 있어 수행 가능하다. 단, 초음파가 타임아웃으로 150 ms 걸리는 경우처럼 **최악의 경우**까지 계산해야 한다.

현재 코드의 `g_ledLastTick = now` 방식은 실제 LED 주기가 **500 ms + 루프 1바퀴 시간**이 된다. 즉 최대 `500 + P`까지 늘어날 수 있다.

- "500 ms마다 한 번 이상" → 최악 응답시간이 500 ms 이하인지 계산
- "정확히 500 ms 주기" → 타이머 인터럽트 또는 FreeRTOS `vTaskDelayUntil()` + 우선순위 사용
- 검증 → GPIO 토글 + 로직 아날라이저/오실로스코프, 또는 ms 타임스탬프 로그

> 현재 단계에는 LED 태스크만 있으므로 `P ≈ 1 ms` 수준이다. 센서를 추가하는 STEP02부터 이 조건을 실제로 계산한다.

---

## 4. 변경된 파일

| 파일 | 변경 |
|---|---|
| `Core/Src/main.c` | USER CODE 영역에 LED 태스크 + UART 출력 추가 |
| `Core/Inc/main.h` | 변경 없음 (기존 `LD2_Pin`, `LD2_GPIO_Port` 사용) |
| `F411_TASK_LAB.ioc` | 변경 없음 (PA5는 이미 GPIO_Output) |

> CubeMX가 생성한 초기 소스에는 LED **점멸 로직**이 없고, PA5 핀 설정만 있다.
> 이 단계에서 점멸 로직을 처음 추가한 것이다.

---

## 5. 코드

### 5.1 정의와 변수 (`main.c`)

```c
/* USER CODE BEGIN Includes */
#include <string.h>
/* USER CODE END Includes */

/* USER CODE BEGIN PD */
#define LED_BLINK_PERIOD_MS   500U
#define UART_PRINT_TIMEOUT_MS 100U
/* USER CODE END PD */

/* USER CODE BEGIN PV */
static uint32_t g_ledLastTick = 0U;
static uint8_t  g_ledState = 0U;
/* USER CODE END PV */

/* USER CODE BEGIN PFP */
static void LED_Task(void);
static void UART_Print(const char *msg);
/* USER CODE END PFP */
```

> **`500U`의 `U`는 단위가 아니다.** C 언어의 `unsigned int` 정수 리터럴 접미사다.
> 즉 `500U`는 "unsigned 500"이며, `HAL_GetTick()`이 밀리초(ms)를 반환하므로
> 이 코드에서는 **500 ms**를 의미한다. 마이크로초(us)가 아니다.
> `UART_PRINT_TIMEOUT_MS 100U`도 같은 이유로 100 ms다.
>
> | 표기 | 의미 |
> |---|---|
> | `500U` | `unsigned int` 500 |
> | `500L` | `long` 500 |
> | `500UL` | `unsigned long` 500 |
> | `1.0f` | `float` 1.0 |
>
> `U`를 붙이는 이유는 signed/unsigned 비교 경고를 줄이고 타입을 명확히 하기 위해서다.
> 마이크로초 단위가 필요하면 `HAL_GetTick()` 대신 1 MHz 타이머나 DWT 카운터를 사용하고,
> 매크로 이름도 `LED_BLINK_PERIOD_US`처럼 단위를 바꿔야 한다.

### 5.2 초기화 (`main.c`, `USER CODE BEGIN 2`)

```c
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
  g_ledLastTick = HAL_GetTick();
  g_ledState = 0U;

  UART_Print("\r\n=== F411_TASK_LAB Step 1: LED Blink ===\r\n");
  UART_Print("LD2 (PA5) toggles every 500 ms (non-blocking).\r\n");
```

### 5.3 메인 루프

```c
while (1)
{
  /* USER CODE BEGIN 3 */
  LED_Task();
}
```

### 5.4 태스크 구현 (`main.c`, `USER CODE BEGIN 4`)

```c
static void UART_Print(const char *msg)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)msg, (uint16_t)strlen(msg), UART_PRINT_TIMEOUT_MS);
}

static void LED_Task(void)
{
  uint32_t now = HAL_GetTick();

  if ((now - g_ledLastTick) >= LED_BLINK_PERIOD_MS)
  {
    g_ledLastTick = now;
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    g_ledState ^= 1U;
    UART_Print((g_ledState != 0U) ? "LED ON\r\n" : "LED OFF\r\n");
  }
}
```

실제 위치:

- `LED_BLINK_PERIOD_MS`: `main.c:34`
- 변수: `main.c:47`
- 프로토타입: `main.c:56`
- 초기화: `main.c:95`
- 루프 호출: `main.c:111`
- 함수 구현: `main.c:234`

---

## 6. 빌드 결과

임시 워크스페이스에서 헤드리스 빌드로 검증했다.

```text
Build Finished. 0 errors, 0 warnings.

text    data    bss     dec     hex    filename
8760      12   1652   10424    28b8   F411_TASK_LAB.elf
```

---

## 7. 보드에서 확인

1. `Project` → `Build Project` (`Ctrl+B`)
2. `Run` → `Run` (`Ctrl+F11`)로 플래시
3. 시리얼 모니터를 **115200 8N1**로 연결
4. 보드의 검은 `RESET` 버튼을 누른다

### 기대 결과

- LD2가 0.5초마다 ON/OFF
- 시리얼에 아래가 보인다.

```text
=== F411_TASK_LAB Step 1: LED Blink ===
LD2 (PA5) toggles every 500 ms (non-blocking).
LED ON
LED OFF
LED ON
LED OFF
...
```

> LD2는 보드의 녹색 LED다. 위치는 보드 실크스크린의 `LD2` 표시를 확인한다.

---

## 8. 체크리스트

- [ ] 빌드 0 error / 0 warning
- [ ] 플래시 성공
- [ ] LD2가 0.5초마다 깜빡임
- [ ] 시리얼에 시작 배너 출력
- [ ] `LED ON` / `LED OFF`가 0.5초 간격으로 출력
- [ ] 보드 리셋 후에도 동일하게 동작

---

## 9. 안 될 때

| 증상 | 확인 |
|---|---|
| 빌드 오류 | `main.c`의 USER CODE 영역만 수정했는지, 괄호/세미콜론 확인 |
| 플래시 실패 | ST-LINK 연결, 보드 전원, 다른 프로그램이 포트를 잡고 있는지 |
| LD2 안 켜짐 | PA5가 GPIO_Output인지, `LD2_GPIO_Port`/`LD2_Pin` 정의 확인 |
| 시리얼 출력 없음 | COM 포트, 115200 8N1, USB 케이블 데이터 지원 여부 |
| 글자가 깨짐 | Baud rate 115200, 8N1, 잘못된 COM 포트 |
| 출력이 한 번만 보임 | 시리얼 프로그램에서 포트를 연 뒤 보드 RESET |

---

## 10. 직접 처음부터 해보려면

이미 코드가 적용되어 있지만, CubeMX부터 직접 해보려면:

1. `F411_TASK_LAB.ioc` 열기
2. PA5를 `GPIO_Output`으로 설정, Label `LD2`
3. USART2가 `Asynchronous`, 115200인지 확인
4. `GENERATE CODE`
5. `main.c`의 USER CODE BEGIN/END 사이에 위 코드를 추가

> `USER CODE` 영역 밖을 수정하면 코드 재생성 시 사라질 수 있다.

---

## 11. 스스로 해보기 (선택)

- `LED_BLINK_PERIOD_MS`를 100, 1000으로 바꿔 본다.
- `UART_Print` 호출을 잠시 주석 처리하고 LED 주기가 더 정확해지는지 본다.
- `HAL_Delay(500)` 버전으로 바꿔 보고 시리얼 로그 간격이 어떻게 변하는지 비교한다.

---

## 12. 다음 단계 예고

`STEP02_거리측정.md`

- 초음파 센서로 거리 측정
- "몇 초마다 정확한 데이터"를 숫자 요구사항으로 바꾸기
- 비차단 상태 머신, 타임아웃, 필터, 로그

> **먼저 위 체크리스트를 보드에서 확인한 뒤** 결과를 알려주면 STEP02를 시작한다.
