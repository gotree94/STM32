# STEP05 - Zephyr 스레드/디바이스 트리 구현

> 상태: **코드/빌드 완료. 보드 확인 대기.**
> 기준 프로젝트: `D:\github\STM32\NUCLEO_F411RE\00.F411_TASK_LAB\STEP05_Zephyr`
> Zephyr: v4.5.0 / Zephyr SDK 1.0.1 / CMake 4.4.5 / Ninja 1.13.2
> 워크스페이스: `D:\zephyrproject`
> 최종 업데이트: 2026-11-18

---

## 1. 목표

STEP04는 STM32CubeIDE + FreeRTOS로 LED, 초음파, DHT, UART 로거를 분리했다.

STEP05는 같은 동작을 Zephyr RTOS 위에서 다시 구현한다. 목표는 다음과 같다.

- HAL/큐브MX 없이 Zephyr 커널 스레드로 같은 4개 작업을 분리한다.
- GPIO를 디바이스 트리(`.overlay`)로 선언하고 `gpio_dt_spec`으로 사용한다.
- FreeRTOS의 태스크/큐/우선순위 개념을 Zephyr의 스레드/`k_msgq`/ 스레드 우선순위에 대응시킨다.
- 시리얼 CSV 포맷(`D,...`, `E,...`)을 STEP02~STEP04와 동일하게 유지한다.

---

## 2. 준비물/배선

배선은 STEP04와 완전히 같다. 새로 연결할 것은 없다.

| 부품 | 핀 | 비고 |
|---|---|---|
| LED LD2 | PA5 | 보드 내장, `led0` 별칭으로 참조 |
| HC-SR04 TRIG | PA1 | GPIO 출력 |
| HC-SR04 ECHO | PA0 | GPIO 입력 + 양/하강 에지 ISR |
| DHT DATA | PA4 | 4.7 kΩ ~ 10 kΩ 풀업 권장 |
| UART TX | PA2 | ST-LINK VCP, Zephyr 콘솔 |
| UART RX | PA3 | ST-LINK VCP |

Zephyr의 `nucleo_f411re` 보드 DTS에서는 `usart2`가 `zephyr,console`로 지정되어 있다. 따라서 별도 UART 설정 없이 `printk()`가 PA2/PA3으로 출력된다.

---

## 3. 설치/환경

로컬에 설치한 Zephyr 도구는 다음과 같다.

| 항목 | 경로/버전 |
|---|---|
| Python | `C:\ProgramData\anaconda3\python.exe` |
| 가상환경 | `D:\zephyrproject\.venv\Scripts\python.exe` |
| west workspace | `D:\zephyrproject` |
| Zephyr | `D:\zephyrproject\zephyr` (v4.5.0) |
| SDK | `D:\zephyr-sdk-1.0.1` |
| ARM GCC | `D:\zephyr-sdk-1.0.1\gnu\arm-zephyr-eabi\bin\arm-zephyr-eabi-gcc.exe` |

SDK 설치 시 `setup.cmd /c`가 `7z`를 찾지 못하면 `C:\Program Files\7-Zip`을 PATH에 추가한 뒤 다음을 실행한다.

```powershell
$env:PATH = 'C:\Program Files\7-Zip;' + $env:PATH
D:\zephyr-sdk-1.0.1\setup.cmd /t arm-zephyr-eabi /c
```

---

## 4. FreeRTOS → Zephyr 대응

| STEP04 (FreeRTOS) | STEP05 (Zephyr) | 차이 |
|---|---|---|
| `xTaskCreate()` | `k_thread_create()` | 스택을 `K_THREAD_STACK_DEFINE()`으로 정적으로 선언 |
| 우선순위 숫자가 클수록 높음 | **숫자가 작을수록 높음** | 주의: 반대 direction |
| `xQueueCreate()` + `xQueueSend()` | `K_MSGQ_DEFINE()` + `k_msgq_put()` | 메시지 크기/개수를 매크로로 정의 |
| `vTaskDelay()` | `k_sleep(K_MSEC(...))` | 밀리초 단위 지연 |
| `HAL_GetTick()` | `k_uptime_get_32()` | 커널 업타임 |
| TIM2 + EXTI | `k_cycle_get_32()` + `GPIO_INT_EDGE_BOTH` | 에코 폭을 CPU 사이클로 계측 |
| `configASSERT()` | `CONFIG_ASSERT` (선택) | 기본 빌드에서는 끔 |
| TIM6 HAL timebase | Zephyr SysTick | HAL timebase 충돌 없음 |
| `__HAL_GPIO_*` | `gpio_is_ready_dt()`, `gpio_pin_configure_dt()` | 핀 번호가 DTS로 이동 |

우선순위 매핑은 다음과 같다.

| 스레드 | Zephyr 우선순위 | FreeRTOS 대응 | 스택 |
|---|---:|---|---:|
| DHT | 2 | 4 | 1024 B |
| Ultrasonic | 3 | 3 | 1024 B |
| LED | 4 | 2 | 512 B |
| Logger | 5 | 1 | 1024 B |

Zephyr는 숫자가 낮을수록 우선순위가 높으므로 DHT에 `2`, 로거에 `5`를 준다.

---

## 5. 프로젝트 구조

```text
STEP05_Zephyr/
├── CMakeLists.txt
├── prj.conf
├── boards/
│   └── nucleo_f411re.overlay
├── dts/
│   └── bindings/
│       ├── f411,sensors.yaml
│       └── vendor-prefixes.txt
└── src/
    ├── main.c
    ├── ultrasonic.h
    ├── ultrasonic.c
    ├── dht.h
    └── dht.c
```

### `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(f411_task_lab_zephyr)

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_sources(app PRIVATE
  src/main.c
  src/ultrasonic.c
  src/dht.c
)
```

### `prj.conf`

```text
CONFIG_GPIO=y
CONFIG_SERIAL=y
CONFIG_UART_CONSOLE=y
CONFIG_CONSOLE=y
CONFIG_PRINTK=y
CONFIG_SYS_CLOCK_TICKS_PER_SEC=1000
CONFIG_THREAD_NAME=y
CONFIG_MAIN_STACK_SIZE=2048
```

`CONFIG_UART_CONSOLE=y`와 보드 DTS의 `zephyr,console = &usart2` 조합으로 `printk()`가 ST-LINK VCP에 출력된다.

---

## 6. 디바이스 트리

`boards/nucleo_f411re.overlay`는 센서 핀을 커스텀 노드로 정의한다.

```dts
/ {
	f411_sensors {
		compatible = "f411,sensors";
		status = "okay";

		trigger-gpios = <&gpioa 1 GPIO_ACTIVE_HIGH>;
		echo-gpios = <&gpioa 0 GPIO_ACTIVE_HIGH>;
		dht-gpios = <&gpioa 4 GPIO_ACTIVE_HIGH>;
	};
};
```

`dts/bindings/f411,sensors.yaml`은 이 세 프로퍼티를 `phandle-array`로 선언한다.

```yaml
compatible: "f411,sensors"

include: base.yaml

properties:
  trigger-gpios:
    type: phandle-array
    required: true
  echo-gpios:
    type: phandle-array
    required: true
  dht-gpios:
    type: phandle-array
    required: true
```

`f411` vendor prefix 경고를 없애기 위해 `dts/bindings/vendor-prefixes.txt`도 추가했다.

```text
f411	F411_TASK_LAB
```

LED는 보드 DTS의 별칭을 그대로 사용한다.

```c
#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
```

---

## 7. 스레드 구조

`main()`은 초기화 후 4개 스레드를 만들고 종료된다. Zephyr 커널이 남은 스레드를 계속 스케줄링한다.

```c
K_THREAD_STACK_DEFINE(led_stack, 512);
K_THREAD_STACK_DEFINE(us_stack, 1024);
K_THREAD_STACK_DEFINE(dht_stack, 1024);
K_THREAD_STACK_DEFINE(logger_stack, 1024);

K_MSGQ_DEFINE(log_queue, sizeof(LogMessage), 8U, 4U);
```

| 스레드 | 주기 | 동작 |
|---|---:|---|
| DHT | 10 ms poll | 내부에서 2 s마다 실제 읽기, 음속 갱신, `E` publish |
| Ultrasonic | 1 ms | 상태머신, 200 ms마다 TRIG, 1 s마다 `D` publish |
| LED | 500 ms | LD2 토글 |
| Logger | 이벤트 | 큐에서 메시지를 꺼내 `printk()` |

로그는 `vsnprintf()`로 96바이트 버퍼에 만든 뒤 `k_msgq_put(..., K_NO_WAIT)`으로 큐에 넣는다. 큐가 가득 차면 해당 로그는 버려진다. 센서 스레드가 UART 전송을 기다리지 않는 것이 핵심이다.

---

## 8. 초음파 측정

`src/ultrasonic.c`는 STEP02~STEP04와 같은 알고리즘을 Zephyr API로 옮겼다.

- 200 ms마다 TRIG를 10 us 이상 high로 유지
- ECHO 핀에 `GPIO_INT_EDGE_BOTH` 인터럽트 등록
- 상승 에지에서 `k_cycle_get_32()`, 하강 에지에서 `k_cyc_to_us_floor32()`로 폭 계산
- 유효 폭: 150 ~ 30000 us
- 35 ms timeout
- 5개 샘플 median filter
- 1 s마다 `D` publish
- 음속: `3313 + (606 * temperature_c_x10) / 1000` (x10 m/s), 2500~4000 범위로 clamp

---

## 9. DHT 읽기

`src/dht.c`는 DHT11/DHT22 비트뱅킹을 그대로 유지한다.

- `dht_init()`에서 `s_last_read_ms = k_uptime_get_32()`로 설정 → 첫 측정은 부팅 2 s 후
- 시작 신호: 출력 low 20 ms → high 30 us → 입력 전환
- 응답 검사 후 40비트 수신
- checksum 검증
- DHT22는 int16 raw 값, DHT11은 정수부*10 방식으로 처리
- `temperature_c_x10`, `humidity_rh_x10`을 `E` 로그로 publish

---

## 10. 빌드

### 빌드 명령

```powershell
$env:PATH = 'C:\Program Files\7-Zip;D:\zephyrproject\.venv\Scripts;' + $env:PATH
$env:ZEPHYR_SDK_INSTALL_DIR = 'D:\zephyr-sdk-1.0.1'

cd D:\zephyrproject
& 'D:\zephyrproject\.venv\Scripts\python.exe' -m west build `
  -b nucleo_f411re `
  -p always `
  'D:\github\STM32\NUCLEO_F411RE\00.F411_TASK_LAB\STEP05_Zephyr'
```

### 빌드 결과

```text
Memory region         Used Size  Region Size  %age Used
           FLASH:       21260 B       512 KB      4.06%
             RAM:       11136 B       128 KB      8.50%
Generating files from D:/zephyrproject/build/zephyr/zephyr.elf for board: nucleo_f411re/stm32f411xe
```

- exit code: 0
- warning: 없음
- 생성물: `D:\zephyrproject\build\zephyr\zephyr.elf`, `.hex`, `.bin`

> 처음 빌드에서 `GPIO_OUTPUT_INACTIVE`가 DTS에서 오류를 냈다. DTS flags에는 `GPIO_ACTIVE_HIGH`를 써야 한다.
> 로그 포맷도 STEP04와 맞추기 위해 `\n`을 `\r\n`으로 통일했다.

---

## 11. 플래시/보드 확인

기본 flash runner는 `stm32cubeprogrammer`다. SDK에 포함된 OpenOCD를 쓰려면 `--runner openocd`를 지정한다.

```powershell
cd D:\zephyrproject
& 'D:\zephyrproject\.venv\Scripts\python.exe' -m west flash
```

또는:

```powershell
& 'D:\zephyrproject\.venv\Scripts\python.exe' -m west flash --runner openocd
```

플래시 후 시리얼을 115200 8N1로 연다. 예상 배너:

```text
=== F411_TASK_LAB Step 5: Zephyr ===
Zephyr RTOS: 4 threads, GPIO DT, message queue logger
HC-SR04: TRIG=PA1, ECHO=PA0, 5 Hz sample, 1 Hz median
DHT11/DHT22: DATA=PA4, 0.5 Hz, sound speed compensation
```

확인 항목:

1. LD2가 0.5초마다 점멸한다.
2. `D,...`가 1초에 한 번 출력된다.
3. `E,...`가 2초에 한 번 출력된다.
4. `E`의 `valid=1`, `D`의 `delta_mm`이 10~12 mm로 유지된다.

정상 로그 예:

```text
D,3000,1687,1699,12,1,0
E,4014,232,340,1,0,3453
D,4000,1688,1699,11,1,0
E,6014,231,340,1,0,3452
D,6000,1687,1699,12,1,0
```

---

## 12. 문제 해결

### 배너가 안 나온다

- `CONFIG_UART_CONSOLE=y`와 `CONFIG_PRINTK=y`가 있는지 확인한다.
- 보드 DTS의 `zephyr,console`이 `&usart2`인지 확인한다.
- PA2/PA3과 ST-LINK VCP 연결을 확인한다.

### `unknown vendor prefix 'f411'` 경고

- `dts/bindings/vendor-prefixes.txt`에 `f411	F411_TASK_LAB`가 있는지 확인한다.
- 파일이 앱의 `dts/bindings` 아래에 있어야 한다.

### DHT `valid=0`이 반복된다

- PA4, GND, VCC 배선을 다시 확인한다.
- DHT 스레드 우선순위가 `2`인지 확인한다.
- 2 s 읽기 간격을 유지한다.

### 초음파 `errors`가 증가한다

- TRIG가 10 us 이상 high인지 확인한다.
- ECHO 인터럽트가 `GPIO_INT_EDGE_BOTH`로 등록됐는지 확인한다.
- DHT reading 중 짧은 지연은 정상이다.

### 빌드가 안 된다

- `ZEPHYR_SDK_INSTALL_DIR`이 `D:\zephyr-sdk-1.0.1`인지 확인한다.
- venv python (`D:\zephyrproject\.venv\Scripts\python.exe`)을 사용하는지 확인한다.
- 완전 재빌드는 `-p always`를 붙인다.

---

## 13. 체크리스트

- [x] Zephyr v4.5.0 + SDK 1.0.1 설치
- [x] `CMakeLists.txt`, `prj.conf` 작성
- [x] `nucleo_f411re.overlay` + binding 작성
- [x] LED/초음파/DHT/Logger 스레드 분리
- [x] message queue logger 구현
- [x] D/E CSV 포맷 유지
- [x] headless 빌드 성공 (0 warning)
- [ ] 보드에서 LD2 0.5 s 점멸 확인
- [ ] 보드에서 `D,...` 1 Hz 출력 확인
- [ ] 보드에서 `E,...` 0.5 Hz 출력 확인
- [ ] `E`의 `valid=1` 확인
- [ ] `D`의 `delta_mm` 10~12 mm 유지 확인

---

## 14. 다음 단계

보드 확인이 끝나면 STEP06(MNIST PC 학습 → ST Edge AI)으로 넘어간다.

> STEP05의 핵심은 "FreeRTOS 코드를 Zephyr API로 바꿨다"가 아니라,
> "하드웨어 연결을 디바이스 트리로 옮기고, 스레드 우선순위 방향이 반대라는 점을 이해했다"는 것이다.
