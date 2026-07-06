# STM32_Care_bot — Serial Port ICD (Interface Control Document)

## 1. Hardware Interface

### 1.1 USART2 (Unused)

| Parameter | Value |
|---|---|
| **Peripheral** | USART2 |
| **TX Pin** | PA2 |
| **RX Pin** | PA3 |
| **Baud Rate** | 115200 |
| **Data Bits** | 8 |
| **Stop Bits** | 1 |
| **Parity** | None |
| **Flow Control** | None |
| **Mode** | TX/RX (not used in user code) |

### 1.2 USART3 (Bluetooth — HC-05/HC-06)

| Parameter | Value |
|---|---|
| **Peripheral** | USART3 (partial remap) |
| **TX Pin** | PC10 (→ BT RX) |
| **RX Pin** | PC11 (← BT TX) |
| **Baud Rate** | 115200 |
| **Data Bits** | 8 |
| **Stop Bits** | 1 |
| **Parity** | None |
| **Flow Control** | None |
| **Mode** | TX/RX, full duplex |
| **RX Method** | Interrupt-driven (`HAL_UART_Receive_IT`, 1-byte buffer) |
| **TX Method** | `printf` → `__io_putchar` → `HAL_UART_Transmit(&huart3, ...)` |
| **IRQ** | `USART3_IRQHandler` (NVIC priority 0) |

**Source:** `Core/Src/stm32f1xx_hal_msp.c:349-370`

---

## 2. TX Protocol (Firmware → PC/Smartphone)

All `printf()` output is redirected to USART3 via `__io_putchar` → `HAL_UART_Transmit(&huart3, ...)`.

### 2.1 Telemetry — ~2 Hz (every ~500ms)

```
S,DIST_MM,<dist>,US1,<d1>,US2,<d2>,OBS,<obs>,KY033_RAW,<l_raw>,KY033_R_RAW,<r_raw>,LINE_L,<l_black>,LINE_R,<r_black>,TRACK,<state>\r\n
```

| Field | Type | Format | Description |
|---|---|---|---|
| `dist` | int | `%d` | Min distance of US1/US2 (mm), `-1` if both fail |
| `d1` | int | `%d` | Ultrasonic #1 distance (mm, PB0), `-1` if timeout |
| `d2` | int | `%d` | Ultrasonic #2 distance (mm, PA4), `-1` if timeout |
| `obs` | int | `%d` | KY-032 obstacle detected (`1` = detected) |
| `l_raw` | int | `%d` | KY-033 left sensor raw ADC value |
| `r_raw` | int | `%d` | KY-033 right sensor raw ADC value |
| `l_black` | int | `%d` | KY-033 left line detection (`1` = black line) |
| `r_black` | int | `%d` | KY-033 right line detection (`1` = black line) |
| `state` | string | `%s` | `BLACK` / `LEFT_BLACK` / `RIGHT_BLACK` / `WHITE` |

**Example:**
```
S,DIST_MM,150,US1,150,US2,200,OBS,0,KY033_RAW,512,KY033_R_RAW,480,LINE_L,1,LINE_R,1,TRACK,BLACK\r\n
```

**Source:** `Core/Src/main.c:310-321`

### 2.2 Ultrasonic Sensor Status

```
S1: <d1> mm | S2: <d2> mm\r\n
```
or on timeout:
```
S1: Echo Fail | S2: Echo Fail\r\n
```

**Source:** `Core/Src/main.c:193-200`

### 2.3 Obstacle Detection

```
Obstacle Detected\r\n
No Obstacle\r\n
```

**Source:** `Core/Src/main.c:256,263`

### 2.4 Line Tracking Sensor Raw Data

```
KY033 L RAW = <left_raw> | R RAW = <right_raw>\r\n
```

**Source:** `Core/Src/main.c:271`

### 2.5 Line Tracking State

```
BLACK LINE BOTH\r\n
BLACK LINE LEFT\r\n
BLACK LINE RIGHT\r\n
WHITE AREA BOTH\r\n
```

**Source:** `Core/Src/main.c:275-287`

### 2.6 Boot Messages

```
Care Bot Start\r\n
Bluetooth Ready\r\n
```

**Source:** `Core/Src/main.c:165,171`

### 2.7 Bluetooth Command Echo

```
BT RX: <char>\r\n
Forward\r\n
Backward\r\n
Left\r\n
Right\r\n
Stop\r\n
Laser Toggle\r\n
Servo1 Up\r\n
Servo1 Down\r\n
Servo2 Up\r\n
Servo2 Down\r\n
Servo Center\r\n
```

**Source:** `Core/Src/main.c:711-787`

---

## 3. RX Protocol (PC/Smartphone → Firmware)

**Method:** Interrupt-driven (`HAL_UART_Receive_IT`), single-byte buffer, callback re-arms after each byte.

All commands are **single-byte characters**, case-insensitive.

### 3.1 Motion Commands

| Byte | Function | Description |
|---|---|---|
| `w` / `W` | Forward | Drives motors forward |
| `s` / `S` | Backward | Drives motors backward |
| `a` / `A` | Turn left | Pivot left |
| `d` / `D` | Turn right | Pivot right |
| `x` / `X` | Stop | All motors stop |

### 3.2 Auxiliary Commands

| Byte | Function | Description |
|---|---|---|
| `v` / `V` | Laser toggle | Toggle laser on/off |
| `i` / `I` | Servo1 up | Pan servo up (angle -= 10°) |
| `k` / `K` | Servo1 down | Pan servo down (angle += 10°) |
| `l` / `L` | Servo2 up | Tilt servo up (angle += 10°) |
| `j` / `J` | Servo2 down | Tilt servo down (angle -= 10°) |
| `m` / `M` | Servo center | Both servos → 90° (center) |

### 3.3 Reception Behavior

| Aspect | Detail |
|---|---|
| **Buffer** | 1 byte (`uint8_t btData`) |
| **Mode** | `HAL_UART_Receive_IT` (interrupt-driven, non-blocking) |
| **Re-arm** | Callback calls `HAL_UART_Receive_IT` again after processing |
| **Auto-motion conflict** | Main loop also controls motors via KY-033 line tracking — BT motion may be overridden on next loop iteration if line is detected |

**Source:** `Core/Src/main.c:707-791`

---

## 4. Timing Diagram

```
Main Loop (approximately 500ms per cycle)
├── SuperSonic_Read1_mm()       — blocking ~120ms
├── SuperSonic_Read2_mm()       — blocking ~120ms
├── printf("S1 / S2 ...")       → USART3 TX
├── Cat LCD state machine
├── Servo90 control (up/down)
├── KY032 obstacle check
│   ├── Motor_Backward(1s) if detected
│   └── printf("Obstacle Detected") → USART3 TX
├── KY033 line check
│   ├── Motor_Forward/Left/Right/Stop
│   └── printf("BLACK...") → USART3 TX
├── printf telemetry line       → USART3 TX
└── HAL_Delay(100)

[Background — USART3 RX IRQ]
└── HAL_UART_RxCpltCallback
    ├── printf("BT RX: %c")    → USART3 TX (echo)
    ├── Motor/Laser/Servo control
    └── HAL_UART_Receive_IT()  — re-arm
```

- **Telemetry TX rate:** ~2 Hz (limited by `SuperSonic_Read` blocking delays + `HAL_Delay(100)`)
- **RX latency:** Immediate via interrupt; motor action may be overridden by main loop line-tracking logic
- **Servo angles:** Persist across main loop iterations (global `servo1_angle`, `servo2_angle`)

---

## 5. Error Handling

| Condition | Behavior |
|---|---|
| **Ultrasonic timeout (>30ms)** | Returns `-1`, prints `Echo Fail` |
| **BT RX buffer** | Single byte — no queuing, overwritten on each IRQ |
| **Invalid BT command** | Ignored silently (no NAK) |
| **Motor conflict** | Main loop line-tracking always runs; BT motion only persists until next sensor evaluation |
