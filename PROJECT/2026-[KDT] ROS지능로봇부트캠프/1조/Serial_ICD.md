# SLAM_sample — Serial Port ICD (Interface Control Document)

## 1. Hardware Interface (USART2)

| Parameter | Value |
|---|---|
| **MCU** | STM32F103RB (Cortex-M3) |
| **Peripheral** | USART2 |
| **TX Pin** | PA2 |
| **RX Pin** | PA3 |
| **Baud Rate** | 115200 |
| **Data Bits** | 8 |
| **Stop Bits** | 1 |
| **Parity** | None |
| **Flow Control** | None |
| **Mode** | Full-duplex (TX/RX) |
| **Driver** | STM32 HAL (`HAL_UART_Transmit` / `HAL_UART_Receive`) |
| **Frame Format** | `printf` → `HAL_UART_Transmit(&huart2, ...)` |

---

## 2. TX Protocol (Firmware → PC)

### 2.1 SLAM Telemetry — 10 Hz (every 100ms)

```
SLAM,<dist>,<yaw>,<sonar_r>,<sonar_l>,<sonar_f>\r\n
```

| Field | Type | Format | Unit | Description |
|---|---|---|---|---|
| `dist` | float | `%.2f` | cm | Cumulative travel distance from encoder |
| `yaw` | float | `%.2f` | deg | Accumulated yaw from MPU6050 gyro |
| `sonar_r` | float | `%.1f` | cm | Right ultrasonic (HC-SR04 #0, PB0) |
| `sonar_l` | float | `%.1f` | cm | Left ultrasonic (HC-SR04 #1, PA4) |
| `sonar_f` | float | `%.1f` | cm | Front ultrasonic (HC-SR04 #2, PC11) |

**Example:**
```
SLAM,12.35,-45.20,30.5,25.0,15.3\r\n
```

**Special Values:**
- `999.0` — Sensor timeout / no echo (treated as open space)
- `0.0` — Sensor read failure (replaced with 999.0 internally)

**Source:** `Core/Src/main.c:240`

---

### 2.2 Fire Alert — One-shot

```
FIRE_ALERT,TEMP:<temp_c>\r\n
```

| Field | Type | Format | Unit | Description |
|---|---|---|---|---|
| `temp_c` | float | `%.1f` | °C | Temperature from KY-013 (ADC CH0) |

**Formula:** `temp_c = (raw_adc * 330.0) / 4095.0`

**Example:**
```
FIRE_ALERT,TEMP:25.4\r\n
```

**Triggers when:** Flame sensor detects target + laser ON + buzzer alert.

**Source:** `Core/Inc/motor_drive.h:169`

---

### 2.3 Crash Alert — One-shot

```
CRASH\r\n
```

**Triggers when:**
- Shock sensor interrupt (`PB12`, falling edge, 1500ms cooldown)
- MPU6050 accelerometer G-force > 5.0G on X or Y axis
- Front ultrasonic distance ≤ 4.0cm

**Source:** `Core/Inc/motor_drive.h:223`, `Core/Inc/shock_sensor.h:35`, `Core/Inc/MPU-6050.h:230`

---

### 2.4 CDS Raw Value — 5 Hz (every 200ms)

```
CDS Raw Value: <value>\r\n
```

| Field | Type | Format | Description |
|---|---|---|---|
| `value` | uint32_t | `%lu` | Raw ADC reading (CH10, 0–4095) |

**Threshold:** `is_dark = (value < 3000)`

**Example:**
```
CDS Raw Value: 2048\r\n
```

**Source:** `Core/Inc/env_sensors.h:71`

---

### 2.5 System Boot/Status Messages

| Message | Description |
|---|---|
| `\n\n--- SLAM & Drive System Started ---\n` | Boot complete |
| `MPU6050 Init Failed!\n` | MPU6050 not detected |
| `자이로 영점 조절 중.가만히 두세요.\n` | Gyro calibrating |
| `System Ready. Let's Map!\n` | All systems ready |

---

## 3. RX Protocol (PC → Firmware)

**Method:** Polled with 1ms blocking timeout via:
```c
HAL_UART_Receive(&huart2, &rx_char, 1, 1)
```

All commands are **single-byte characters**. Firmware processes one byte per main loop iteration.

### 3.1 Mode & Motion Commands

| Byte | Function | Mode | Description |
|---|---|---|---|
| `z` / `Z` | Start auto | Any → Auto | Begins autonomous exploration |
| `x` | Stop | Any → Manual | Stops all motion, exits auto mode, sets `car_state = 'x'` |
| `w` | Forward | Manual only | Moves forward at current speed |
| `a` | Left | Manual only | Spins left in place |
| `s` | Backward | Manual only | Moves backward at current speed |
| `d` | Right | Manual only | Spins right in place |

**Source:** `Core/Inc/motor_drive.h:203-218`

### 3.2 Speed Control

| Byte | Speed |
|---|---|
| `1` | 50% (PWM) |
| `2` | 70% (PWM) |
| `3` | 100% (PWM) |

**Source:** `Core/Inc/motor_drive.h:207-209`

### 3.3 Reception Behavior

| Aspect | Detail |
|---|---|
| **Buffer** | Single byte (no RX buffer / FIFO) |
| **Timeout** | 1ms polling (non-blocking fallback) |
| **Auto-mode guard** | `w`/`a`/`s`/`d` ignored if `auto_mode == 1` |
| **ORE handling** | Overrun error flag cleared each loop at `Drive_System_Update` entry |

---

## 4. Timing Diagram

```
Firmware Main Loop (approx 5–10ms per cycle)
├── EnvSensors_Update()         — digital/ADC reads (every 200ms for CDS)
├── Sonar_Scheduler()           — triggers next sonar (25ms interval per sensor)
├── MPU6050_Update()            — gyro polling + yaw integration
├── Drive_System_Update(&huart2)
│   ├── ORE clear
│   ├── UART RX poll (1ms)      — ← PC command may arrive here
│   ├── Collision checks
│   ├── Auto-navigation FSM
│   └── Manual mode FSM
├── Feedback_Update()           — buzzer PWM toggle
└── [if 100ms elapsed]
    └── printf("SLAM,...\r\n")  — → PC receives telemetry
```

- **SLAM TX:** 10Hz (100ms interval, `main.c:230`)
- **RX poll:** Every main loop iteration (~5-10ms latency)
- **CDS TX:** 5Hz (200ms interval, `env_sensors.h:67`)

---

## 5. Error Handling

| Issue | Mitigation |
|---|---|
| **UART Overrun (ORE)** | `__HAL_UART_CLEAR_OREFLAG()` at each loop iteration |
| **No RX data** | `HAL_UART_Receive` returns `HAL_TIMEOUT` after 1ms — no action taken |
| **Invalid byte** | Ignored (only known chars processed) |
