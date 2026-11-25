# F411_TASK_LAB Step 2: B1/LD2 인터럽트 + HC-SR04 초음파 거리 측정

대상 프로젝트: `C:\Users\Administrator\STM32CubeIDE\workspace_2.2.0\F411_TASK_LAB1`

## 1. 전체 구조

| 영역 | 파일 | 역할 |
|---|---|---|
| CubeMX 설정 | `.ioc`, `main.c`, `main.h`, `stm32f4xx_it.c/h`, `stm32f4xx_hal_msp.c` | 핀, 클록, TIM1, NVIC, UART2, 인터럽트 핸들러를 생성 |
| 사용자 코드 | `main.c`의 `USER CODE` 영역 | 초기화 호출, `printf` 리타겟, `Sensor_Task()`, EXTI 콜백 분기 |
| 초음파 로직 | `ultrasonic.c/h` | TRIG 펄스, ECHO 에지 캡처, 거리 계산, 중앙값 필터 |

`ultrasonic.c`는 GPIO/TIM1/NVIC를 다시 초기화하지 않습니다. CubeMX가 만든 `MX_GPIO_Init()`, `MX_TIM1_Init()` 결과를 사용하고, 생성된 전역 `htim1`을 `extern`으로 참조해 TIM1만 시작합니다.

## 2. CubeMX 설정 ↔ 코드 수정 병행 표

아래 표에서 왼쪽은 STM32CubeMX에서 지정할 내용이고, 오른쪽은 같은 단계에서 `USER CODE BEGIN/END` 안에 넣을 코드입니다.

| 단계 | CubeMX에서 설정 | 코드에서 수정 |
|---|---|---|
| 1. 시스템 클록 | HSI → PLL, `PLLM=16`, `PLLN=336`, `PLLP=/4` → SYSCLK 84 MHz, APB2 `/1` | 생성된 `SystemClock_Config()` 사용. 별도 수정 없음 |
| 2. TIM1 시간 기준 | `TIM1`, Internal Clock, Prescaler `84-1`, Counter Up, Period `65535`, ARR Preload Enable | `Ultrasonic_Init()`에서 `HAL_TIM_Base_Start(&htim1)`. `US_TIMER_TICKS_PER_US = 1` (1 tick = 1 us) |
| 3. TRIG 출력 | `PA1` = `GPIO_Output`, 라벨 `TRIG` | `Sensor_Task()`가 200 ms마다 PA1을 10 us 동안 HIGH → LOW |
| 4. ECHO 입력 | `PA0` = `GPIO_EXTI0`, `Rising/Falling`, `No pull`, 라벨 `ECHO` | `HAL_GPIO_EXTI_Callback()`에서 `GPIO_Pin == ECHO_Pin`이면 `Ultrasonic_OnEchoEdge()` 호출 |
| 5. NVIC | `EXTI line0 interrupt`, `EXTI line[15:10] interrupts` 활성화, Priority 0/0 | `stm32f4xx_it.c`에 `EXTI0_IRQHandler()`, `EXTI15_10_IRQHandler()` 추가 |
| 6. B1 / LD2 | `PC13` = Falling EXTI, `PA5` = `GPIO_Output`, NVIC `EXTI15_10` | 같은 콜백에서 `GPIO_Pin == B1_Pin`이면 `HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin)` |
| 7. UART2 | `USART2`, 115200 baud, 8 data, 1 stop, no parity, `PA2/PA3` | `#include <stdio.h>` + `__io_putchar()` 리타겟. `D,...`와 `C,...` 출력 |
| 8. 거리 계산 | TIM1 1 MHz를 시간 기준으로 사용 | `distance_mm = echo_ticks * 3430 / 20000`, 5개 샘플 중앙값, 1초마다 publish |

`.ioc`에 반영된 주요 설정:

```properties
PA0-WKUP.GPIO_ModeDefaultEXTI=GPIO_MODE_IT_RISING_FALLING
PC13-ANTI_TAMP.GPIO_ModeDefaultEXTI=GPIO_MODE_IT_FALLING
NVIC.EXTI0_IRQn=true\:0\:0\:false\:false\:true\:true\:true\:true
NVIC.EXTI15_10_IRQn=true\:0\:0\:false\:false\:true\:true\:true\:true
TIM1.Prescaler=84-1
TIM1.Period=65535
TIM1.IPParameters=Prescaler,Period,AutoReloadPreload
```

## 3. 코드 수정 상세

### 3.1 `main.c` - Include / 초기화 / 루프

```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "ultrasonic.h"
/* USER CODE END Includes */
```

```c
/* USER CODE BEGIN 2 */
Ultrasonic_Init();
printf("\n=== F411_TASK_LAB Step 2: Ultrasonic ===\n");
printf("HC-SR04: TRIG=PA1, ECHO=PA0, 5 Hz sample, 1 Hz median\n");
printf("TIM1: PCLK2=%luHz divider=%lu ARR=%lu tick=%luHz\n",
       (unsigned long)HAL_RCC_GetPCLK2Freq(),
       (unsigned long)(htim1.Init.Prescaler + 1U),
       (unsigned long)htim1.Init.Period,
       (unsigned long)(HAL_RCC_GetPCLK2Freq() / (htim1.Init.Prescaler + 1U)));
/* USER CODE END 2 */
```

```c
/* USER CODE BEGIN 3 */
Sensor_Task();
/* USER CODE END 3 */
```

### 3.2 `main.c` - printf 리타겟과 `Sensor_Task()`

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

static void Sensor_Task(void)
{
  Ultrasonic_Result result;
  char line[80];

  Ultrasonic_Task();

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

`printf(line)`은 포맷 문자열 경고 때문에 `printf("%s", line)`으로 처리했습니다. 리타겟이 `\n` 앞에 `\r`을 추가하므로 문자열에서는 `\n`만 사용합니다.

### 3.3 `main.c` - EXTI 콜백 분기

```c
/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == ECHO_Pin)
  {
    Ultrasonic_OnEchoEdge();
  }
  else if (GPIO_Pin == B1_Pin)
  {
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
  }
}
/* USER CODE END 4 */
```

### 3.4 `stm32f4xx_it.c` / `stm32f4xx_it.h`

```c
void EXTI0_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(ECHO_Pin);
}

void EXTI15_10_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(B1_Pin);
}
```

```c
void EXTI0_IRQHandler(void);
void EXTI15_10_IRQHandler(void);
```

### 3.5 `ultrasonic.h` API

```c
typedef struct
{
  uint32_t timestamp_ms;
  uint32_t errors;
  uint16_t distance_ref_mm;
  uint16_t distance_mm;
  uint8_t valid;
} Ultrasonic_Result;

void Ultrasonic_Init(void);
void Ultrasonic_Task(void);
uint8_t Ultrasonic_GetResult(Ultrasonic_Result *result);
uint16_t Ultrasonic_GetLastEchoTicks(void);
uint8_t Ultrasonic_IsIdle(void);
void Ultrasonic_SetSoundSpeedX10(uint16_t speed_x10);
void Ultrasonic_OnEchoEdge(void);
```

### 3.6 `ultrasonic.c` - 거리 계산 핵심

```c
#define US_TIMER_TICKS_PER_US 1U
#define US_MIN_ECHO_TICKS (100U * US_TIMER_TICKS_PER_US)
#define US_MAX_ECHO_TICKS (30000U * US_TIMER_TICKS_PER_US)
```

```c
uint16_t Ultrasonic_GetLastEchoTicks(void)
{
  return s_echoWidthTicks;
}
```

```c
static void US_StoreSample(uint32_t width_ticks)
{
  uint32_t width_us = width_ticks / US_TIMER_TICKS_PER_US;
  uint32_t distance_ref_mm = (width_us * US_REFERENCE_SOUND_SPEED_X10) / 20000U;
  uint32_t distance_mm = (width_us * s_soundSpeedX10) / 20000U;
  ...
}
```

- `US_REFERENCE_SOUND_SPEED_X10 = 3430`은 343.0 m/s입니다.
- `/20000`은 mm/us로 환산한 뒤 왕복 시간을 2로 나누는 부분입니다.
- `width = (uint16_t)(fall_cnt - rise_cnt)`이므로 TIM1 16-bit 오버플로도 처리됩니다.

## 4. 시리얼 출력 형식

부팅 직후 배너 예시:

```text
=== F411_TASK_LAB Step 2: Ultrasonic ===
HC-SR04: TRIG=PA1, ECHO=PA0, 5 Hz sample, 1 Hz median
TIM1: PCLK2=84000000Hz divider=84 ARR=65535 tick=1000000Hz
```

1초마다 아래 두 줄이 출력됩니다.

```text
D,58123,1681,1,0
C,9801
```

| 필드 | 의미 |
|---|---|
| `D` | `timestamp_ms`, `distance_mm`, `valid`, `errors` |
| `C` | 마지막 ECHO 펄스의 TIM1 카운트 차이. 검증용 디버그 출력 |

검증이 끝나면 `printf("C,%u\n", ...)` 한 줄만 제거하고 `D,` 라인만 남겨도 됩니다.

## 5. 측정 흐름

1. 200 ms마다 TRIG(PA1)를 10 us 동안 HIGH로 유지합니다.
2. ECHO(PA0) Rising 인터럽트에서 `TIM1->CNT`를 저장합니다.
3. ECHO Falling 인터럽트에서 `TIM1->CNT`를 저장합니다.
4. `(uint16_t)(fall - rise)`로 펄스 폭(ticks)을 구합니다.
5. 유효 범위(100~30000 us)이면 거리 샘플로 저장합니다.
6. 5개 샘플의 중앙값을 계산해 1초마다 UART2로 publish합니다.
7. B1 버튼을 누르면 같은 EXTI 콜백에서 LD2를 토글합니다.

## 6. 실험 결과와 분석

| 조건 | 스코프로 본 ECHO 폭 | `C` 카운트 | 계산 거리 | 판정 |
|---|---:|---:|---:|---|
| 15 cm 자 | 약 1.0 ms | 약 1000 | 171 mm | 근접 기준점/반올림 오차 고려 시 정상. 이론값은 0.875 ms |
| 최초 1.5 m 기대 | 4.16 ms | 약 4160~4222 | 713~724 mm | 계산은 맞음. 센서가 더 가까운 면을 보고 있었음 |
| 각도 조정 후 | 약 9.7 ms | 약 9700 | 1664 mm | 1.7~1.8 m 기대와 일치 |
| 최종 UART 값 | 약 9.80 ms | 약 9801 | **1681 mm** | 정상 측정으로 확정 |

계산 예시:

```text
9.7 ms  → 9700 × 3430 / 20000 = 1663.55 mm
4.16 ms → 4160 × 3430 / 20000 =  713.44 mm
1.0 ms  → 1000 × 3430 / 20000 =  171.50 mm
1681 mm → 1681 × 20000 / 3430 ≈ 9801 ticks
```

### conclusion

- 거리 공식과 TIM1 설정은 수정할 필요가 없었습니다.
- 4.16 ms는 실제 1.5 m 거리가 아니라 약 0.71 m 지점의 반사면을 잡은 결과였습니다.
- HC-SR04는 빔 각도가 좁고 반사면에 민감합니다. 수직이 조금만 어긋나도 옆/아래의 다른 물체를 측정합니다.
- 센서를 각도 조정해 타겟 면에 정확히 마주하게 한 뒤 9.7 ms, 1681 mm가 나와 계산식이 검증되었습니다.
- 음속은 온도에 따라 변하지만(약 0.17 %/°C) 이번 차이처럼 2배 수준의 오차는 만들지 못합니다.

## 7. 주의사항

- `HAL_GPIO_EXTI_Callback()`은 모든 GPIO EXTI가 공유하므로 반드시 `GPIO_Pin`을 확인해야 합니다.
- TIM1은 16-bit 카운터이므로 `uint16_t` 뺄셈으로 오버플로를 처리합니다.
- B1은 하드웨어 디바운스가 없어 여러 번 인터럽트가 발생할 수 있습니다.
- HC-SR04는 약 2 cm 이하 근거리에서 측정이 불안정할 수 있습니다.
- CubeMX에서 코드를 다시 생성해도 `USER CODE BEGIN/END` 영역의 호출과 콜백은 유지됩니다.

## 8. SWV / 트레이스 관찰 설정

CubeMX에서 `SYS → Debug = Trace Asynchronous SW`를 선택하면 `PA13=SWDIO`, `PA14=SWCLK`, `PB3=SWO`가 할당됩니다.

```properties
PA13.Mode=Trace_Asynchronous_SW
PA14.Mode=Trace_Asynchronous_SW
PB3.Mode=Trace_Asynchronous_SW
PB3.Signal=SYS_JTDO-SWO
```

Debug Configuration에서는 다음을 확인합니다.

- `Debugger → Serial Wire Viewer` 활성화 (`enable_swv=true`)
- `swd_mode=true`
- `swv_trace_hclk=84000000` (SYSCLK 84 MHz)
- `swv_port=61235` (기본값)

디버그 실행 후 `Window → Show View → SWV`에서 다음 뷰를 엽니다.

| 뷰 | 설정 | 코드 수정 필요 여부 |
|---|---|---|
| `SWV Data Trace Timeline Graph` | 비교기에 `g_trace_echo_ticks`, `g_trace_echo_edge`, `g_trace_echo_state`, `g_trace_b1_irq` 추가 | 변수는 이미 추가되어 있음. 뷰에서 비교기 등록만 필요 |
| `SWV ITM Data Console` | Stimulus Port 0 활성화 후 Start Trace | UART `printf`는 여기 나오지 않음. ITM 출력은 `ITM_SendChar()` 별도 호출 필요 |
| `SWV Statistical Profiling` | Trace 시작 후 실행 | 불필요. 함수별 실행 시간/인터럽트 통계 표시 |

추가된 트레이스 변수:

| 변수 | 의미 |
|---|---|
| `g_trace_echo_ticks` | 마지막 ECHO 펄스 폭, TIM1 ticks (1 tick = 1 us) |
| `g_trace_echo_edge` | ECHO Rising에서 1, Falling/Timeout에서 0 |
| `g_trace_echo_state` | 0=Idle/Echo Low, 1=ECHO Rising 검출, 2=TRIG 후 대기 |
| `g_trace_edge_count` | ECHO 에지 발생 횟수 |
| `g_trace_b1_irq` | `EXTI15_10_IRQHandler()` 진입마다 토글 |
| `g_trace_b1_irq_count` | B1 EXTI 인터럽트 진입 횟수 |

### 8.1 비교기 등록 절차

Debugger 탭에서 `Debug probe=ST-LINK`, `Interface=SWD`, `Freq=Auto`, `SWV=Enable`이 보이면 설정은 정상입니다. Comparator 등록은 Debug Configuration이 아니라 **디버그 실행 후 SWV 뷰**에서 합니다.

1. Debug를 실행해 `main()`에서 멈춘 상태로 둡니다.
2. `Window → Show View → SWV → SWV Data Trace Timeline Graph`를 엽니다. 목록에 없으면 `Other… → SWV`를 선택합니다. `SWV Data Trace`, `SWV Trace Log`도 함께 열면 숫자 기록을 보기 좋습니다.
3. SWV 탭 오른쪽 위의 렌치/톱니 아이콘(`Configure Trace` 또는 `SWV Settings`)을 클릭합니다.
4. `Comparator 0~3`에서 `Enable`을 체크하고 변수를 등록합니다. 비교기는 최대 4개까지만 사용할 수 있습니다.

| Comparator | Var/Addr | Access | Generate |
|---|---|---|---|
| 0 | `g_trace_echo_ticks` | `Write` | `Data Value + PC` |
| 1 | `g_trace_echo_edge` | `Write` | `Data Value + PC` |
| 2 | `g_trace_echo_state` | `Write` | `Data Value + PC` |
| 3 | `g_trace_edge_count` | `Write` | `Data Value + PC` |

5. OK를 누른 뒤 SWV 뷰의 빨간 **Start Trace** 버튼을 먼저 누르고, 그다음 **Resume(F8)** 합니다.
6. `SWV Data Trace`의 Watch에서 행을 클릭하면 해당 comparator의 History가 표시됩니다. Timeline Graph에서는 표시할 comparator 체크도 켜야 선이 보입니다.

변수 이름이 해석되지 않으면 현재 ELF의 심볼 주소를 `Var/Addr`에 넣을 수 있습니다. 빌드 후에는 주소가 바뀔 수 있으므로 이름을 먼저 사용합니다.

| 변수 | 주소 | 크기 |
|---|---|---:|
| `g_trace_b1_irq` | `0x20000114` | 1 byte |
| `g_trace_b1_irq_count` | `0x20000118` | 4 bytes |
| `g_trace_echo_ticks` | `0x20000120` | 2 bytes |
| `g_trace_echo_edge` | `0x20000122` | 1 byte |
| `g_trace_echo_state` | `0x20000123` | 1 byte |
| `g_trace_edge_count` | `0x20000124` | 4 bytes |

Statistical Profiling은 comparators 등록 없이 트레이스만 켜면 `EXTI0_IRQHandler()`, `EXTI15_10_IRQHandler()`, `Ultrasonic_OnEchoEdge()` 같은 함수 통계를 보여줍니다.

### 8.2 Data Trace 결과 읽는 법

실제 저장된 `SWV 데이터 트레이스.txt`의 한 줄 예시:

```text
"쓰기";"4196";"0x8001064";"10792159";"128.478083 ms"
```

| 필드 | 의미 |
|---|---|
| `쓰기` | DWT comparator가 잡은 접근 종류. `Access=Write`이므로 CPU가 변수에 값을 쓸 때마다 기록됨 |
| `4196` | 기록 순간 `g_trace_echo_ticks`에 써진 값. TIM1이 1 MHz이므로 `4196 ticks = 4196 us` |
| `0x8001064` | 값을 쓴 코드의 PC 주소. 변수 주소가 아님. `addr2line` 결과는 `ultrasonic.c:221`, 즉 `g_trace_echo_ticks = width;` |
| `10792159` | 이벤트 시점의 DWT 사이클 카운터 |
| `128.478083 ms` | 같은 시점의 시간. `10,792,159 / 84,000,000 ≈ 0.128478 s`로 Core Clock 84 MHz 설정도 확인됨 |

거리 환산 예:

```text
4196 × 3430 / 20000 ≈ 719.6 mm
4225 × 3430 / 20000 ≈ 724.6 mm
4174 × 3430 / 20000 ≈ 715.8 mm
4145 × 3430 / 20000 ≈ 710.9 mm
```

### 8.3 Watch 변수 동작과 실제 흐름

| 변수 | 관측된 값 | 동작 |
|---|---:|---|
| `g_trace_echo_ticks` | `4225` | Falling 에지에서 `(uint16_t)(now - s_echoStartCnt)`로 계산한 ECHO 펄스 폭을 저장 (`ultrasonic.c:221`) |
| `g_trace_echo_edge` | `0` | ECHO가 HIGH이면 `1`, LOW이면 `0`. Rising에서 `1` (`ultrasonic.c:211`), Falling/Timeout에서 `0` (`:222`, `:146`) |
| `g_trace_echo_state` | `1` | `2`=TRIG 후 대기, `1`=ECHO Rising 검출, `0`=Idle/ECHO Low. TRIG 후 `2` (`:124`), Rising에서 `1` (`:212`), Falling에서 `0` (`:223`) |
| `g_trace_edge_count` | `620` | 유효 ECHO Rising/Falling마다 1씩 증가 (`:213`, `:224`). Trace 시작 전부터 누적된 값이므로 현재 구간의 에지 수와 다를 수 있음 |

동작 순서:

1. 200 ms마다 `TRIG(PA1)`를 10 us 동안 HIGH → LOW로 만들고 `g_trace_echo_state=2`로 둡니다.
2. HC-SR04의 ECHO가 HIGH가 되면 `EXTI0_IRQHandler()` → `HAL_GPIO_EXTI_Callback()` → `Ultrasonic_OnEchoEdge()`가 호출됩니다.
3. Rising에서 TIM1 `CNT`를 `s_echoStartCnt`에 저장하고 `edge=1`, `state=1`, `edge_count++`를 수행합니다.
4. ECHO가 LOW가 되면 다시 `CNT`를 읽어 `width = now - start`를 구합니다.
5. `g_trace_echo_ticks = width;`가 실행될 때 History에 `쓰기` 이벤트가 남습니다. 이후 `edge=0`, `state=0`, `edge_count++`, `s_echoDone=1`이 됩니다.
6. `Ultrasonic_Task()`가 완료를 감지해 샘플을 저장하고, 1초마다 중앙값을 UART로 publish합니다.

실제 캡처 해석:

- History의 시간 간격이 `0.128 → 0.328 → 0.528 …`로 정확히 200 ms입니다. `US_SAMPLE_PERIOD_MS=200U`와 일치합니다.
- 값은 대부분 `4145~4225 ticks`, 즉 `4.145~4.225 ms`입니다. 이는 거리 약 `711~725 mm`이며 이전 스코프 측정값 `4.16 ms / 713 mm`와 일치합니다.
- `g_trace_echo_ticks`의 마지막 Watch 값 `4225`와 History 마지막 줄 `4225`가 같습니다.
- Watch의 4개 값은 같은 순간의 원자적 스냅샷이 아닙니다. comparator별 FIFO와 SWO 대역폭 때문에 `edge=0`, `state=1`처럼 섞여 보일 수 있으므로 정확한 타이밍은 각 변수의 History를 따로 봐야 합니다.
- Timeline Graph는 이미지로만 저장되지만, `SWV Data Trace`의 History는 텍스트/CSV로 저장할 수 있습니다. 이번에 저장한 `SWV_데이터_트레이스.txt`가 숫자 원본 데이터입니다.
