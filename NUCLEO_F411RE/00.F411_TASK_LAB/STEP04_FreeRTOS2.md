# F411_LAB 최종 작업 정리 — 초음파 + 온습도 + FreeRTOS

- 프로젝트: `C:\Users\Administrator\STM32CubeIDE\workspace_2.2.0\F411_LAB`
- MCU: STM32F411RETx / 84 MHz / NUCLEO-F411RE / FW_F4 V1.28.3
- 최종 상태: **`make all` 및 CubeIDE 헤드리스 빌드 0 에러 · 0 경고**
- 산출물: `F411_LAB.elf` — text 33372 / data 108 / bss 21980 / dec 55460 bytes

> 이 문서는 앞서 작성한 `F411_LAB_온습도_연동_작업.md`의 내용을 포함해 최종 상태로 다시 정리한 것입니다.

---

## 1. 전체 작업 흐름

| 단계 | 내용 | 메인 코드 변경 여부 |
|---|---|---|
| Step 2 (기존) | HC-SR04 거리 측정 + TRACE (`D`, `C`) | 이미 완료 |
| Step 3 | DHT 온습도 추가, 음속 보정, `TH`/`ENV`/`S` 출력 | `main.c` + 첨부 `dht.c/.h` 수정 |
| Step 4 (현재) | CubeMX에서 FreeRTOS 추가, 태스크 분리, UART mutex | `.ioc` 재생성 + `main.c` USER 코드 수정 |

핵심 원칙: **메인 프로젝트를 기준으로 두고, 첨부 소스(`dht.c/.h`)를 메인에 맞췄으며**, FreeRTOS 전환은 `.ioc`를 직접 수정해 CubeMX가 코드를 생성하도록 했습니다.

---

## 2. CubeMX에서 실제 반영된 설정

### 2.1 Middleware / NVIC / Clock

| 항목 | 최종 값 | 비고 |
|---|---|---|
| FREERTOS Interface | `CMSIS_V2` | `VP_FREERTOS_VS_CMSIS_V2.Mode=CMSIS_V2` |
| `configTOTAL_HEAP_SIZE` | `16384` | 태스크 3개 + 대기 여유 확보 |
| `configTICK_RATE_HZ` | 1000 | 1 ms tick |
| `configUSE_PREEMPTION` | 1 | 선점형 스케줄링 |
| `configUSE_MUTEXES` | 1 | `myMutex01` 사용 |
| `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` | 5 | EXTI 우선순위 5와 매칭 |
| NVIC Priority Group | `NVIC_PRIORITYGROUP_4` | "4 bits for pre-emption priority" |
| EXTI0 / EXTI15_10 우선순위 | 5 | `HAL_NVIC_SetPriority(..., 5, 0)` 생성됨 |
| SYS Timebase Source | **SysTick 유지** | 아래 참고 참고 |

> **참고 (TIM2 권장안에 대한 정정):** 처음에는 HAL timebase를 TIM2로 옮기는 것을 권장했습니다. 그러나 생성된 코드에서 `stm32f4xx_it.c`의 `SysTick_Handler()`가 `HAL_IncTick()`과 `xPortSysTickHandler()`를 **함께 호출**하고, `USE_CUSTOM_SYSTICK_HANDLER_IMPLEMENTATION=1`로 `cmsis_os2.c`의 중복 핸들러가 비활성화됩니다. 따라서 **SysTick을 그대로 두어도 충돌이 없으며**, 실제 프로젝트도 SysTick으로 정상 빌드·동작합니다. TIM2로 옮기는 것은 선택 사항입니다.

### 2.2 태스크 / 뮤텍스 (실제 생성된 값)

| 이름 | 우선순위 | 스택 | Entry 함수 | 역할 |
|---|---|---|---|---|
| `defaultTask` | 24 (Normal) | 128 words | `StartDefaultTask` | 대기용, `osDelay(1)` |
| `myTask02` | 24 (Normal) | 256 words | `StartUltrasonicTask` | 초음파 폴링 + `D`/`C` 출력 |
| `myTask03` | 25 (Normal+1) | 256 words | `StartDhtTask` | DHT 읽기 + `TH`/`ENV`/`S` 출력 |
| `myMutex01` | Dynamic | — | — | UART(`printf`) 접근 직렬화 |

- 생성된 태스크 함수는 `freertos.c`가 아니라 **`main.c`** 안에 만들어졌습니다.
- `main()`에서 `osKernelInitialize()` → `osMutexNew()` → `osThreadNew()` 3회 → `osKernelStart()` 순서로 생성됩니다.

### 2.3 유지한 설정

- `TIM1`: Prescaler `84-1`, ARR `65535` → 1 MHz 카운터 (초음파 + DHT 공용 시간 기준)
- `USART2`: 115200-8N1, PA2/PA3
- GPIO: PA0 ECHO, PA1 TRIG, PA5 LD2
- **PA4는 `.ioc`에 할당하지 않음** — `dht.c`가 런타임에 입력/OD로 전환

---

## 3. 코드 변경 내용

### 3.1 `Core/Src/dht.c`, `Core/Inc/dht.h` (Step 3)

- 첨부 소스의 미초기화 `TIM2->CNT` 제거 → `__HAL_TIM_GET_COUNTER(&htim1)` 사용
- `DHT_ElapsedUs(start)`를 두어 `(uint16_t)(now - start)`로 16비트 랩어라운드 처리
- 20 ms LOW → 30 µs release → 응답 → 40비트 수신, 50 µs 기준으로 0/1 판별
- 체크섬 검증 + DHT22/DHT11 자동 디코딩, 2초 주기, 에러 카운트 유지

### 3.2 `Core/Src/main.c` (Step 4)

| 위치 | 변경 |
|---|---|
| Includes | `#include "cmsis_os.h"`(생성), `#include "dht.h"` 유지 |
| USER CODE 0 | `Sensor_SoundSpeedX10()`만 유지, 기존 `Sensor_Task()` 전체 제거 |
| USER CODE 2 | `Ultrasonic_Init()`, `Dht_Init()` 유지 + Step 4 배너 및 태스크 주기 출력 추가 |
| while(1) | `Sensor_Task()` 호출 제거 (스케줄러가 점유하므로 도달하지 않음) |
| `StartUltrasonicTask` | 10 ms 주기 `Ultrasonic_Task()` → 결과 발생 시 mutex 안에서 `D`, `C` 출력 → `osDelay(10)` |
| `StartDhtTask` | 100 ms 주기 `Dht_Task()` → 결과 발생 시 mutex 안에서 `TH`, `ENV`, `S` 출력 + 음속 갱신 → `osDelay(100)` |
| `HAL_GPIO_EXTI_Callback` | 유지 (`Ultrasonic_OnEchoEdge()`, B1 LED 토글). RTOS API를 호출하지 않아 우선순위 5에서 안전 |

음속 보정 수식(`Sensor_SoundSpeedX10`):

```
speed_x10 = 3313 + (606 × T_x10) / 1000 + (12 × RH_x10) / 1000
```

- `T_x10` = 온도 ×10 (0.1 °C), `RH_x10` = 습도 ×10 (0.1 %RH)
- 2500~4000 범위로 클램프, `dht.valid == 1`일 때만 `Ultrasonic_SetSoundSpeedX10()` 호출


```c
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "ultrasonic.h"
#include "dht.h"
/* USER CODE END Includes */
```

```c
/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart2;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for myTask02 */
osThreadId_t myTask02Handle;
const osThreadAttr_t myTask02_attributes = {
  .name = "myTask02",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for myTask03 */
osThreadId_t myTask03Handle;
const osThreadAttr_t myTask03_attributes = {
  .name = "myTask03",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal1,
};
/* Definitions for myMutex01 */
osMutexId_t myMutex01Handle;
const osMutexAttr_t myMutex01_attributes = {
  .name = "myMutex01"
};
/* USER CODE BEGIN PV */

/* USER CODE END PV */
```

```c

```


---

## 4. 태스크 동작 타이밍

```
[FreeRTOS 1 ms tick]
 myTask03(DHT)     : 100 ms마다 Dht_Task() 호출 → 내부 2 s 게이트 → 실제 읽기 2 s마다 1회 (약 24 ms 블로킹)
 myTask02(US)      : 10 ms마다 Ultrasonic_Task() → 내부 200 ms 샘플 / 1000 ms 퍼블리시 → D, C 출력 1 Hz
 defaultTask       : osDelay(1) 대기
 ISR(EXTI0)        : 에코 엣지에서 TIM1 카운터만 읽어 펄스 폭 저장 (인터럽트 우선순위 5)
```

- DHT가 24 ms 블로킹해도 초음파 샘플 주기(200 ms)와 에코 타임아웃(35 ms)보다 짧아 충돌하지 않습니다.
- 음속 값(`uint16_t`) 갱신은 DHT 태스크, 참조는 초음파 태스크에서 이루어지지만 32비트 코어에서 16비트 대입은 원자적이라 별도 동기화가 필요 없습니다.

---

## 5. 시리얼 출력 (115200-8N1)

| 프리픽스 | 형식 | 발생 주기 |
|---|---|---|
| `D` | `D,<time_ms>,<distance_mm>,<valid>,<errors>` | 1 Hz (음속 보정 적용) |
| `C` | `C,<echo_ticks>` | 1 Hz (에코 폭, 1 MHz = µs) |
| `TH` | `TH,<time_ms>,<temp_x10>,<rh_x10>,<valid>,<errors>` | 2 s |
| `ENV` | `ENV,<+/-><T>.<t>C,<RH>.<r>%RH` | 2 s |
| `S` | `S,<speed_x10>` | DHT 유효 시 2 s |

예시:

```
=== F411_TASK_LAB Step 4: FreeRTOS Ultrasonic + DHT ===
HC-SR04: TRIG=PA1, ECHO=PA0, 5 Hz sample, 1 Hz median
DHTxx: DATA=PA4, 2 s sample, TIM1 1 MHz timebase
Tasks: US=10 ms poll, DHT=100 ms poll, UART mutex
D,10234,512,1,0
C,2981
TH,10400,205,503,1,0
ENV,+20.5C,50.3%RH
S,3441
```

```c
  /* USER CODE BEGIN 2 */
  Ultrasonic_Init();
  Dht_Init();
  printf("\n=== F411_TASK_LAB Step 4: FreeRTOS Ultrasonic + DHT ===\n");
  printf("HC-SR04: TRIG=PA1, ECHO=PA0, 5 Hz sample, 1 Hz median\n");
  printf("DHTxx: DATA=PA4, 2 s sample, TIM1 1 MHz timebase\n");
  printf("Tasks: US=10 ms poll, DHT=100 ms poll, UART mutex\n");
  printf("TIM1: PCLK2=%luHz divider=%lu ARR=%lu tick=%luHz\n",
         (unsigned long)HAL_RCC_GetPCLK2Freq(),
         (unsigned long)(htim1.Init.Prescaler + 1U),
         (unsigned long)htim1.Init.Period,
         (unsigned long)(HAL_RCC_GetPCLK2Freq() / (htim1.Init.Prescaler + 1U)));
  /* USER CODE END 2 */
```

```c
  /* Init scheduler */
  osKernelInitialize();
  /* Create the mutex(es) */
  /* creation of myMutex01 */
  myMutex01Handle = osMutexNew(&myMutex01_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of myTask02 */
  myTask02Handle = osThreadNew(StartUltrasonicTask, NULL, &myTask02_attributes);

  /* creation of myTask03 */
  myTask03Handle = osThreadNew(StartDhtTask, NULL, &myTask03_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
```

* /* USER CODE END 4 */ 아래

```c
/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}
```

```c
/* USER CODE BEGIN Header_StartUltrasonicTask */
/**
* @brief Function implementing the myTask02 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUltrasonicTask */
void StartUltrasonicTask(void *argument)
{
  /* USER CODE BEGIN StartUltrasonicTask */
  Ultrasonic_Result result;
  char line[80];

  for(;;)
  {
    Ultrasonic_Task();

    if (Ultrasonic_GetResult(&result) != 0U)
    {
      (void)snprintf(line, sizeof(line), "D,%lu,%u,%u,%lu\n",
                     (unsigned long)result.timestamp_ms,
                     (unsigned int)result.distance_mm,
                     (unsigned int)result.valid,
                     (unsigned long)result.errors);

      (void)osMutexAcquire(myMutex01Handle, osWaitForever);
      printf("%s", line);
      printf("C,%u\n", (unsigned int)Ultrasonic_GetLastEchoTicks());
      (void)osMutexRelease(myMutex01Handle);
    }

    osDelay(10);
  }
  /* USER CODE END StartUltrasonicTask */
}
```

```c
/* USER CODE BEGIN Header_StartDhtTask */
/**
* @brief Function implementing the myTask03 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDhtTask */
void StartDhtTask(void *argument)
{
  /* USER CODE BEGIN StartDhtTask */
  Dht_Result dht;
  char line[96];
  int32_t temperature;
  uint16_t speed_x10;

  for(;;)
  {
    Dht_Task();

    if (Dht_GetResult(&dht) != 0U)
    {
      temperature = (int32_t)dht.temperature_c_x10;
      (void)snprintf(line, sizeof(line), "TH,%lu,%d,%u,%u,%lu\n",
                     (unsigned long)dht.timestamp_ms,
                     (int)temperature,
                     (unsigned int)dht.humidity_rh_x10,
                     (unsigned int)dht.valid,
                     (unsigned long)dht.errors);

      (void)osMutexAcquire(myMutex01Handle, osWaitForever);
      printf("%s", line);
      printf("ENV,%c%u.%uC,%u.%u%%RH\n",
             (temperature < 0) ? '-' : '+',
             (unsigned int)((temperature < 0) ? -temperature : temperature) / 10U,
             (unsigned int)((temperature < 0) ? -temperature : temperature) % 10U,
             (unsigned int)dht.humidity_rh_x10 / 10U,
             (unsigned int)dht.humidity_rh_x10 % 10U);

      if (dht.valid != 0U)
      {
        speed_x10 = Sensor_SoundSpeedX10(dht.temperature_c_x10,
                                         dht.humidity_rh_x10);
        Ultrasonic_SetSoundSpeedX10(speed_x10);
        printf("S,%u\n", (unsigned int)speed_x10);
      }
      (void)osMutexRelease(myMutex01Handle);
    }

    osDelay(100);
  }
  /* USER CODE END StartDhtTask */
}
```


## 6. 빌드 / 검증

CubeIDE 헤드리스 빌드 결과:

```
08:43:10 Build Finished. 0 errors, 0 warnings.
   text     data      bss      dec      hex
  33372      108    21980    55460     d8a4   F411_LAB.elf
```

커맨드라인 재현:

```powershell
$env:PATH = "C:\ST\STM32CubeIDE_2.2.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.14.3.rel1.win32_1.0.100.202602081740\tools\bin;C:\ST\STM32CubeIDE_2.2.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.make.win32_2.2.200.202604021615\tools\bin;" + $env:PATH
make all    # Debug 폴더에서 실행
```

- CubeMX 코드 생성 후 `Debug` 폴더의 메이크파일이 갱신되지 않아 헤드리스 빌드로 재생성한 뒤 동기화했습니다.
- IDE에서 `Ctrl+B`로 빌드하면 메이크파일이 자동 재생성되므로 동일 결과가 나옵니다.

---

## 7. 주의사항 및 디버깅 팁

1. **재생성 시 USER CODE 블록 유지**: `main.c`의 `/* USER CODE BEGIN ... */` 안쪽 코드가 태스크 구현입니다. CubeMX에서 코드를 다시 생성해도 이 영역은 유지되지만, `DeletePrevious=true` 설정이므로 **작업 전 프로젝트 백업을 권장**합니다.
2. **printf는 태스크에서만**: `__io_putchar`은 `HAL_UART_Transmit` 블로킹 호출입니다. ISR에서 `printf`를 호출하지 마세요. UART 출력은 반드시 `myMutex01` 안에서 수행합니다.
3. **인터럽트 우선순위**: EXTI는 5로 설정되어 있어 ISR에서 FreeRTOS API를 호출해도 안전합니다. 우선순위를 0으로 되돌리면 `configASSERT`에 걸릴 수 있습니다.
4. **힙 여유**: `configTOTAL_HEAP_SIZE=16384`에 태스크 스택 합계 약 2560 bytes + idle 스택이므로 여유가 충분합니다. 로그 버퍼를 크게 늘릴 때만 재검토하면 됩니다.
5. **DHT 배선**: DATA=PA4, VCC 3.3 V, GND 공통, 데이터선 4.7 kΩ 풀업 권장. 전원 인가 직후 첫 측정은 버려도 됩니다.
6. **HYSTERESIS/TRACE**: 기존 `D`, `C` 포맷을 그대로 유지했으므로 기존 TRACE 파서/그래프 도구를 수정할 필요가 없습니다.
7. **센서 미연결**: `TH`의 `valid=0`, `errors` 증가가 확인되면 배선을 점검하세요. 이때 음속은 마지막 유효 값을 유지합니다.

---

## 8. 관련 파일 목록

| 파일 | 상태 |
|---|---|
| `F411_LAB.ioc` | FreeRTOS / CMSIS_V2 / 태스크 3개 / mutex / heap 16384 / PriorityGroup_4 반영 |
| `Core/Src/main.c` | 초기화, 배너, 3개 태스크 구현, EXTI 콜백 |
| `Core/Src/dht.c`, `Core/Inc/dht.h` | TIM1 기반 DHT 드라이버 |
| `Core/Src/ultrasonic.c`, `Core/Inc/ultrasonic.h` | 변경 없음 (기존 로직) |
| `Core/Inc/FreeRTOSConfig.h`, `Core/Src/freertos.c` | CubeMX 생성 |
| `Middlewares/Third_Party/FreeRTOS/**` | CubeMX 생성 (heap_4, CMSIS_RTOS_V2, port ARM_CM4F) |
| `Debug/**` 메이크파일 | FreeRTOS 소스/인클루드 경로 반영 위해 재생성 |
