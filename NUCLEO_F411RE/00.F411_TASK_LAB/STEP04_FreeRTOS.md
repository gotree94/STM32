# STEP04 - FreeRTOS 태스크 분리

> 상태: **코드/빌드 완료. 보드 확인 대기.**
> 기준 프로젝트: `C:\Users\Administrator\STM32CubeIDE\workspace_2.2.0\F411_TASK_LAB`
> FreeRTOS: Kernel V10.3.1 (STM32Cube FW_F4 V1.28.3에 포함된 소스)
> 백업: `C:\Users\Administrator\Desktop\F411_TASK_LAB_STEP03_backup_20261104.zip`

---

## 1. 목표

STEP03까지는 `main()`의 `while (1)` 안에서 LED, 초음파, DHT, UART를 순서대로 호출했다.

```c
while (1)
{
  LED_Task();
  Sensor_Task();
  Environment_Task();
}
```

이 구조는 DHT를 읽는 동안 초음파 측정이 멈추고, UART 출력이 끝날 때까지 다른 작업이 기다리는 문제가 있다.

STEP04의 목표는 다음과 같다.

- LED, 초음파, DHT, UART 로그를 각각 독립 태스크로 분리한다.
- DHT가 느려도 초음파 측정이 멈추지 않게 한다.
- UART 출력은 큐로 분리해서 센서 태스크가 UART 전송을 기다리지 않게 한다.
- FreeRTOS의 태스크, 우선순위, 주기, 큐, 힙, 스택을 직접 확인한다.

---

## 2. 준비물/배선

배선은 STEP03과 동일하다. 새로 연결할 것은 없다.

| 부품 | 핀 | 비고 |
|---|---|---|
| LED LD2 | PA5 | NUCLEO 보드 내장 |
| HC-SR04 TRIG | PA1 | 출력 |
| HC-SR04 ECHO | PA0 | EXTI rising/falling |
| DHT DATA | PA4 | 4.7 kΩ ~ 10 kΩ 풀업 권장 |
| UART TX | PA2 | ST-LINK VCP |
| UART RX | PA3 | ST-LINK VCP |

DHT는 모듈에 풀업이 내장된 경우가 많지만, 없으면 DATA-PA4와 3.3 V 사이에 풀업을 넣는다. STEP03에서 DHT가 `valid=0`이었던 원인은 배선/접촉이었다. 같은 증상이 나오면 먼저 DATA, GND, VCC를 다시 확인한다.

---

## 3. STEP03과 무엇이 달라졌나

### STEP03

- 단일 `while (1)` 루프
- DHT 읽는 동안 초음파 태스크가 대기
- UART 전송이 끝날 때까지 센서 태스크가 대기
- `HAL_GetTick()` 하나로 모든 주기 관리

### STEP04

- FreeRTOS 스케줄러가 태스크를 전환
- DHT, 초음파, LED, 로거가 독립적으로 실행
- UART 로그는 큐에 넣고 로거 태스크가 전송
- FreeRTOS tick은 SysTick, HAL tick은 TIM6로 분리
- 태스크별 우선순위와 주기를 코드로 명시

---

## 4. 핵심 개념

### 태스크

FreeRTOS에서 태스크는 독립 실행 흐름이다. 각 태스크는 자기 스택을 가지고, 스케줄러가 우선순위와 상태를 보고 실행 순서를 결정한다.

### 우선순위

숫자가 클수록 먼저 실행된다. 이 프로젝트에서는 다음과 같이 정했다.

| 태스크 | 우선순위 | 이유 |
|---|---:|---|
| DHT | 4 | 비트 타이밍이 민감하고 20 ms 정도 블로킹한다 |
| Ultrasonic | 3 | 1 ms 주기 상태머신과 10 us TRIG 펄스 |
| LED | 2 | 500 ms 주기, 정밀도 낮음 |
| Logger | 1 | UART 블로킹 전송, 가장 낮아도 됨 |

DHT가 가장 높은 이유는 DHT22의 응답 비트가 수십 us 단위이기 때문이다. DHT가 실행되는 동안 초음파 태스크가 잠시 멈출 수 있지만, ECHO는 EXTI로 계측되므로 에코 폭 자체는 유지된다.

### 주기

| 태스크 | 주기 | 실제 동작 |
|---|---:|---|
| LED | 500 ms | LD2 토글 |
| Ultrasonic | 1 ms | 상태머신 호출, 200 ms마다 TRIG, 1 s마다 `D` publish |
| DHT | 10 ms poll | 내부에서 2 s마다 실제 DHT 읽기 |
| Logger | 이벤트 | 큐에 로그가 들어올 때만 UART 전송 |

### 큐

센서 태스크가 직접 UART를 호출하면 UART 전송 시간만큼 센서 주기가 밀린다. 그래서 `LogMessage` 구조체를 큐에 넣고 Logger 태스크가 꺼내서 전송한다.

```c
typedef struct
{
  char line[96];
} LogMessage;
```

큐 길이는 8이다. UART가 잠시 느려져도 로그 8개까지 버퍼링한다.

### 힙과 스택

FreeRTOS는 `heap_4.c`를 사용한다. 태스크 스택과 큐는 FreeRTOS 힙에서 동적 할당된다.

| 항목 | 값 |
|---|---:|
| `configTOTAL_HEAP_SIZE` | 12 KB |
| Logger 스택 | 256 words (1 KB) |
| LED 스택 | 128 words (512 B) |
| Ultrasonic 스택 | 256 words (1 KB) |
| DHT 스택 | 256 words (1 KB) |
| Idle 스택 | 128 words (512 B) |

---

## 5. 변경 파일

| 파일 | 변경 내용 |
|---|---|
| `Core/Inc/FreeRTOSConfig.h` | FreeRTOS 설정 추가 |
| `Core/Src/stm32f4xx_hal_timebase_tim.c` | TIM6 기반 HAL tick 추가 |
| `Core/Src/stm32f4xx_it.c` | `SVC_Handler`, `PendSV_Handler`, `SysTick_Handler` 제거 |
| `Core/Src/main.c` | 태스크 4개, 큐 로거, 훅 추가 |
| `Core/Src/ultrasonic.c` | 10 us TRIG 펄스를 한 번에 생성하도록 수정 |
| `Middlewares/Third_Party/FreeRTOS/` | FreeRTOS V10.3.1 소스 추가 |
| `.cproject` | FreeRTOS include path와 source entry 추가 |
| `Debug/.../subdir.mk`, `Debug/makefile`, `Debug/objects.list` | headless 검증용 빌드 갱신 |

FreeRTOS 소스는 다음 위치에서 복사했다.

```text
C:\Users\Administrator\STM32Cube\Repository\STM32Cube_FW_F4_V1.28.3\
  Middlewares\Third_Party\FreeRTOS\Source
```

복사한 파일:

```text
tasks.c
queue.c
list.c
timers.c
event_groups.c
stream_buffer.c
croutine.c
include/*.h
portable/GCC/ARM_CM4F/port.c
portable/GCC/ARM_CM4F/portmacro.h
portable/MemMang/heap_4.c
```

---

## 6. FreeRTOSConfig.h 핵심 설정

```c
#define configUSE_PREEMPTION                    1
#define configUSE_TICKLESS_IDLE                 0
#define configCPU_CLOCK_HZ                      (SystemCoreClock)
#define configTICK_RATE_HZ                      ((TickType_t)1000)
#define configMAX_PRIORITIES                    7
#define configMINIMAL_STACK_SIZE                ((uint16_t)128)
#define configTOTAL_HEAP_SIZE                   ((size_t)(12 * 1024))
#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_MALLOC_FAILED_HOOK            1
```

### 우선순위 비트

STM32F411은 NVIC 우선순위 비트가 4개다.

```c
#define configPRIO_BITS                         4
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

#define configKERNEL_INTERRUPT_PRIORITY \
  (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
  (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
```

- SysTick과 TIM6는 가장 낮은 우선순위(15)를 사용한다.
- EXTI0는 우선순위 5를 사용한다.
- FreeRTOS API를 ISR에서 호출할 때는 `configMAX_SYSCALL_INTERRUPT_PRIORITY`보다 낮은 우선순위여야 한다.
- 현재 EXTI0 ISR은 FreeRTOS API를 호출하지 않고 `Ultrasonic_OnEchoEdge()`만 호출한다.

### 훅

```c
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
  (void)xTask;
  (void)pcTaskName;
  Error_Handler();
}

void vApplicationMallocFailedHook(void)
{
  Error_Handler();
}
```

스택 오버플로나 힙 할당 실패가 발생하면 `Error_Handler()`에서 멈춘다. 디버거를 붙이면 어느 지점에서 멈췄는지 확인할 수 있다.

---

## 7. SysTick과 TIM6를 나누는 이유

FreeRTOS는 SysTick을 자신의 tick으로 사용한다. 그런데 HAL도 기본적으로 SysTick을 1 ms tick으로 사용한다. 둘이 같은 SysTick을 쓰면 충돌한다.

그래서 역할을 나눴다.

| 타이머 | 사용처 | 주기 |
|---|---|---|
| SysTick | FreeRTOS tick, `vTaskDelay()` 기준 | 1 ms |
| TIM6 | HAL tick, `HAL_GetTick()`, `HAL_UART_Transmit()` timeout | 1 ms |

`stm32f4xx_it.c`에서 다음 핸들러를 제거했다.

```c
SVC_Handler
PendSV_Handler
SysTick_Handler
```

대신 FreeRTOS port가 `FreeRTOSConfig.h`의 매크로를 통해 같은 이름의 핸들러를 제공한다.

```c
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler
```

TIM6 핸들러는 `stm32f4xx_hal_timebase_tim.c`에 있다.

```c
void TIM6_DAC_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&s_halTimebase);
}
```

TIM6 update interrupt에서 `HAL_IncTick()`이 호출되어 `HAL_GetTick()`이 계속 증가한다.

---

## 8. main.c 태스크 구조

### 태스크 생성

```c
g_logQueue = xQueueCreate(LOGGER_QUEUE_LENGTH, sizeof(LogMessage));

xTaskCreate(Logger_Task, "LOG", STACK_LOGGER, NULL, PRIO_LOGGER, NULL);
xTaskCreate(LED_Task,    "LED", STACK_LED,    NULL, PRIO_LED,    NULL);
xTaskCreate(US_Task,     "US",  STACK_US,     NULL, PRIO_US,     NULL);
xTaskCreate(DHT_Task,    "DHT", STACK_DHT,    NULL, PRIO_DHT,    NULL);

vTaskStartScheduler();
```

`vTaskStartScheduler()`가 호출되면 main의 `while (1)`은 실행되지 않는다. 스케줄러가 태스크를 선택해서 실행한다.

### LED 태스크

```c
static void LED_Task(void *argument)
{
  TickType_t lastWake = xTaskGetTickCount();

  (void)argument;

  for (;;)
  {
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(LED_BLINK_PERIOD_MS));
  }
}
```

`vTaskDelayUntil()`은 절대 시간 기준으로 주기를 유지한다. 처리 시간이 조금 흔들려도 평균 주기가 500 ms에 가깝게 유지된다.

### 초음파 태스크

```c
static void US_Task(void *argument)
{
  TickType_t lastWake = xTaskGetTickCount();

  (void)argument;

  for (;;)
  {
    Ultrasonic_Result result;

    Ultrasonic_Task();

    if (Ultrasonic_GetResult(&result) != 0U)
    {
      /* D,... 로그 생성 후 Log_Send() */
    }

    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(US_TASK_PERIOD_MS));
  }
}
```

`Ultrasonic_Task()`는 내부 상태머신을 1 ms마다 호출한다.

- `US_STATE_IDLE`: 200 ms마다 TRIG 펄스 생성
- `US_STATE_WAIT_ECHO`: ECHO 완료 또는 35 ms timeout 처리
- 1 s마다 `US_Publish()`로 중앙값 결과 생성

### DHT 태스크

```c
static void DHT_Task(void *argument)
{
  (void)argument;

  for (;;)
  {
    Dht_Result env;

    Dht_Task();

    if (Dht_GetResult(&env) != 0U)
    {
      /* E,... 로그 생성 후 Log_Send() */
    }

    vTaskDelay(pdMS_TO_TICKS(DHT_TASK_PERIOD_MS));
  }
}
```

`Dht_Task()` 내부에서 2 s 주기를 검사하므로 10 ms마다 호출해도 실제 DHT 읽기는 2 s에 한 번이다.

DHT는 우선순위 4로 가장 높다. DHT 읽기 중에는 약 20 ms 동안 바쁘게 대기한다. 이때 초음파 태스크는 잠시 멈출 수 있지만, ECHO 펄스는 EXTI로 계측되므로 측정값은 유지된다. 2 s에 한 번, 최대 1개 샘플을 놓칠 수 있는 정도는 이 학습 단계에서 허용한다.

### Logger 태스크

```c
static void Logger_Task(void *argument)
{
  LogMessage msg;

  (void)argument;

  for (;;)
  {
    if (xQueueReceive(g_logQueue, &msg, portMAX_DELAY) == pdPASS)
    {
      UART_Print(msg.line);
    }
  }
}
```

`UART_Print()`는 `HAL_UART_Transmit()`으로 블로킹 전송한다. Logger 우선순위가 가장 낮기 때문에 다른 태스크가 먼저 실행된다. 큐가 비어 있으면 `portMAX_DELAY`로 무한 대기한다.

---

## 9. 초음파 TRIG 펄스 수정

STEP03 코드는 `US_STATE_TRIG_HIGH` 상태에서 TRIG를 HIGH로 두고, 다음 호출에서 10 us가 지났는지 확인했다.

하지만 STEP04에서는 `Ultrasonic_Task()`가 1 ms마다 호출된다. 상태머신을 그대로 두면 TRIG가 최대 1 ms 동안 HIGH로 유지될 수 있다. HC-SR04는 10 us 이상의 TRIG 펄스를 요구하지만, 1 ms는 불필요하게 길다.

그래서 IDLE 상태에서 TRIG 펄스를 한 번에 생성하도록 바꿨다.

```c
uint32_t trigStartUs = __HAL_TIM_GET_COUNTER(&s_tim2);

HAL_GPIO_WritePin(US_TRIG_PORT, US_TRIG_PIN, GPIO_PIN_SET);
while ((__HAL_TIM_GET_COUNTER(&s_tim2) - trigStartUs) < US_TRIG_PULSE_US)
{
}
HAL_GPIO_WritePin(US_TRIG_PORT, US_TRIG_PIN, GPIO_PIN_RESET);
```

TIM2는 1 MHz로 동작하므로 `US_TRIG_PULSE_US = 10`은 10 us다. 이 루프는 약 10 us만 블로킹한다.

---

## 10. 빌드 결과

검증 빌드는 STM32CubeIDE의 headless make로 수행했다.

```text
arm-none-eabi-gcc ... (Core + FreeRTOS)
Finished building target: F411_TASK_LAB.elf

text    data    bss     dec     hex
23700   96      14784   38580   96b4
```

- exit code: 0
- warning: 없음
- bss 증가: FreeRTOS heap 12 KB + 태스크/큐 정적 구조 영향
- Flash 사용량: 약 23.7 KB / 512 KB
- RAM 사용량: 약 14.9 KB / 128 KB

`Debug/`의 makefile은 headless 검증용으로 갱신했다. STM32CubeIDE에서 프로젝트를 다시 빌드할 때는 `.cproject`에 추가한 include path와 `Middlewares` source entry가 사용된다. IDE에서 파일이 보이지 않으면 프로젝트를 refresh하고 `Clean` 후 다시 빌드한다.

---

## 11. 보드 확인 방법

1. STM32CubeIDE에서 `F411_TASK_LAB`을 연다.
2. `Project > Clean` 후 `Build`한다.
3. ST-LINK로 플래시한다.
4. 시리얼 터미널을 115200 8N1로 연다.
5. 다음 배너가 나오는지 확인한다.

```text
=== F411_TASK_LAB Step 4: FreeRTOS ===
Tasks: LED 500ms, US 1ms state machine, DHT 10ms poll, UART logger
HC-SR04: TRIG=PA1, ECHO=PA0, 5 Hz sample, 1 Hz median
DHT11/DHT22: DATA=PA4, 0.5 Hz, sound speed compensation
```

6. LD2가 0.5초마다 점멸하는지 확인한다.
7. `D,...`가 1초에 한 번 출력되는지 확인한다.
8. `E,...`가 2초에 한 번 출력되는지 확인한다.
9. `E`의 `valid=1`, `D`의 `delta_mm`이 10~12 mm로 유지되는지 확인한다.

정상 로그 예:

```text
D,3000,1687,1699,12,1,0
E,4014,232,340,1,0,3453
D,4000,1688,1699,11,1,0
E,6014,231,340,1,0,3452
D,6000,1687,1699,12,1,0
```

STEP03과 비교했을 때 숫자 자체는 거의 같아야 한다. 달라진 것은 내부 실행 구조다. DHT를 읽는 동안 초음파가 멈추지 않고, UART 전송이 센서 태스크를 막지 않는다.

---

## 12. 문제 해결

### 빌드는 되는데 보드에서 배너가 안 나온다

- Logger 태스크 생성이 실패했을 수 있다. `xTaskCreate()` 반환값이 `pdPASS`인지 확인한다.
- `configTOTAL_HEAP_SIZE`를 12 KB에서 16 KB로 늘려 본다.
- `g_logQueue`가 `NULL`인지 확인한다.
- UART2 핀(PA2/PA3)과 ST-LINK VCP 설정을 확인한다.

### HardFault 또는 멈춤

- `vApplicationStackOverflowHook()`에 걸렸는지 디버거로 확인한다.
- DHT 태스크 스택을 256에서 384 words로 늘려 본다.
- `configCHECK_FOR_STACK_OVERFLOW`를 2로 유지하고, 스택 사용량을 확인한다.
- `configASSERT()`가 걸리면 `FreeRTOSConfig.h`의 우선순위 비트와 NVIC 설정을 확인한다.

### DHT `valid=0`이 반복된다

- PA4 배선, GND, VCC를 다시 확인한다. STEP03에서도 같은 증상은 배선 문제였다.
- DHT 태스크가 우선순위 4인지 확인한다.
- DHT 읽기 중 다른 태스크가 DHT 핀을 건드리지 않는지 확인한다.
- 2 s 주기를 유지한다. DHT11은 읽기 간격이 너무 짧으면 실패할 수 있다.

### 초음파 `errors`가 증가한다

- TRIG 펄스가 10 us로 생성되는지 확인한다.
- ECHO EXTI0 우선순위가 5인지 확인한다.
- DHT 읽기 중 초음파 태스크가 잠시 멈추는 것은 정상이다. `errors`가 2 s마다 1씩 증가하는 정도면 현재 구조의 한계다.
- 더 엄격하게 하려면 DHT를 별도 타이머/DMA로 옮기거나, DHT 읽기 동안 초음파 TRIG를 잠시 멈추는 설계를 검토한다.

### CubeIDE에서 FreeRTOS 파일이 빌드되지 않는다

- `.cproject`의 `sourceEntries`에 `Middlewares`가 있는지 확인한다.
- include path에 다음 두 경로가 있는지 확인한다.

```text
../Middlewares/Third_Party/FreeRTOS/Source/include
../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
```

- `Project > Refresh` 후 `Clean` > `Build`를 다시 실행한다.
- 그래도 안 되면 `Debug/`를 삭제하고 IDE가 빌드 설정을 다시 생성하도록 한다.

---

## 13. 체크리스트

- [x] FreeRTOS V10.3.1 소스 추가
- [x] `FreeRTOSConfig.h` 작성
- [x] TIM6 HAL timebase 추가
- [x] `stm32f4xx_it.c`에서 SVC/PendSV/SysTick 제거
- [x] LED 태스크 분리
- [x] 초음파 태스크 분리
- [x] DHT 태스크 분리
- [x] UART Logger 큐 분리
- [x] 10 us TRIG 펄스 수정
- [x] headless 빌드 성공 (0 warning)
- [ ] 보드에서 LD2 0.5 s 점멸 확인
- [ ] 보드에서 `D,...` 1 Hz 출력 확인
- [ ] 보드에서 `E,...` 0.5 Hz 출력 확인
- [ ] `E`의 `valid=1` 확인
- [ ] `D`의 `delta_mm` 10~12 mm 유지 확인

---

## 14. 다음 단계

보드에서 위 체크리스트가 통과하면 STEP05(Zephyr)로 넘어간다.

STEP05에서는 같은 센서 구성을 Zephyr의 스레드, 디바이스 트리, 센서 드라이버로 다시 구성한다. STEP04에서 익힌 태스크/주기/큐 개념을 Zephyr의 스레드와 메시지 큐에 대응시키는 것이 목표다.

> STEP04의 핵심은 "센서 코드를 FreeRTOS로 바꿨다"가 아니라,
> "느린 작업과 빠른 작업을 분리하고, 공유 자원(UART)을 큐로 격리했다"는 점이다.
