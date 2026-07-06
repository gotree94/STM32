# smart_car — Serial Port ICD (Interface Control Document)

## 1. Hardware Interface

### 1.1 USART2 (Debug Console)

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
| **Mode** | TX only (RX pin defined but unused) |
| **Driver** | `HAL_UART_Transmit(&huart2, ...)` |

### 1.2 USART3 (SLAM Data)

| Parameter | Value |
|---|---|
| **Peripheral** | USART3 |
| **TX Pin** | PC10 |
| **RX Pin** | PC11 |
| **Baud Rate** | 115200 |
| **Data Bits** | 8 |
| **Stop Bits** | 1 |
| **Parity** | None |
| **Flow Control** | None |
| **Mode** | TX only (RX pin defined but unused) |
| **Driver** | `HAL_UART_Transmit(&huart3, ...)` |

---

## 2. TX Protocol Overview

Both UARTs are **TX-only**. There is **no RX protocol** — the firmware never reads serial input.

- **USART2** — human-readable debug console text
- **USART3** — structured CSV data for PC-based SLAM processing

The system runs a **sequential batch process** (44 steps), not a real-time loop:

```
for step = 1 to 44:
    1. test_distance_measure(step)  → 10cm forward
       └── USART3: STEP,...
    2. stationary_scan()            → servo sweeps 0°→180°
       └── USART3: S,angle,d1,d2 (91 lines)
    if step % 11 == 0:
    3. test_rotate_90()             → 90° pivot turn
       └── USART3: ROT,...
```

---

## 3. USART2 — Debug/Diagnostic Messages

All messages end with `\r\n`.

### 3.1 Distance Measurement

| Direction | Format | Example |
|---|---|---|
| TX | `TEST START target=<pulses> pulses (<dist>cm)\r\n` | `TEST START target=5 pulses (10.0cm)\r\n` |
| TX | `STOP pulse_L=<L> pulse_R=<R> calc_dist=<dist>cm time=<ms>ms\r\n` | `STOP pulse_L=8 pulse_R=8 calc_dist=13.30cm time=450ms\r\n` |

- `pulse_L`, `pulse_R` — encoder pulse counts (left/right wheel)
- `dist` = `(pulse_L + pulse_R) / 2 * CM_PER_PULSE + STOP_OVERSHOOT_CM`
- Constants: `CM_PER_PULSE = 1.0`, `STOP_OVERSHOOT_CM = 5.3`

**Source:** `Core/Src/main.c:189-207`

### 3.2 Rotation Measurement

| Direction | Format | Example |
|---|---|---|
| TX | `gyro_offset=<offset>\r\n` | `gyro_offset=12.345\r\n` |
| TX | `ROTATE DONE yaw=<yaw>deg time=<ms>ms\r\n` | `ROTATE DONE yaw=90.5deg time=1200ms\r\n` |

- `gyro_offset` — Z-axis gyro bias from 200 samples
- `yaw` — accumulated yaw during rotation (target ~90°)

**Source:** `Core/Src/main.c:255-278`

### 3.3 I2C Scan

| Direction | Format |
|---|---|
| TX | `I2C scan start\r\n` |
| TX | `Found device at 0x<addr>\r\n` |
| TX | `I2C scan done\r\n` |

**Source:** `Core/Src/main.c:289-302`

### 3.4 MPU6050 Debug

| Direction | Format |
|---|---|
| TX | `MPU6050 init FAIL\r\n` |
| TX | `ack1=<d> ack2=<d> ack3=<d> val=0x<val>\r\n` |
| TX | `pwr_mgmt1=0x<val>\r\n` |

**Source:** `Core/Src/mpu6050.c:61-78`

### 3.5 IR Receiver Debug

| Direction | Format |
|---|---|
| TX | `IR code=0x<code> cmd=0x<cmd>\r\n` |

**Source:** `Core/Src/main.c:360-361`

---

## 4. USART3 — SLAM Data Protocol

### 4.1 Step Data — After Each 10cm Forward Move

```
STEP,<step_no>,<dist_avg>,<pulse_L>,<pulse_R>,<ax>,<ay>,<az>,<gz>\r\n
```

| Field | Type | Format | Unit | Description |
|---|---|---|---|---|
| `step_no` | int | `%d` | — | Step counter (1–44) |
| `dist_avg` | float | `%.2f` | cm | Computed travel distance |
| `pulse_L` | uint32 | `%lu` | pulses | Left encoder pulse count |
| `pulse_R` | uint32 | `%lu` | pulses | Right encoder pulse count |
| `ax` | float | `%.3f` | g | MPU6050 X-axis acceleration |
| `ay` | float | `%.3f` | g | MPU6050 Y-axis acceleration |
| `az` | float | `%.3f` | g | MPU6050 Z-axis acceleration |
| `gz` | float | `%.2f` | dps | MPU6050 Z-axis gyro (raw dps, no offset) |

**Example:**
```
STEP,1,12.35,8,8,0.023,-0.015,1.002,0.45\r\n
```

**Source:** `Core/Src/main.c:216-218`

### 4.2 Rotation Data — After 90° Pivot Turn

```
ROT,<yaw>,<ax>,<ay>,<az>\r\n
```

| Field | Type | Format | Unit | Description |
|---|---|---|---|---|
| `yaw` | float | `%.2f` | deg | Accumulated yaw during rotation |
| `ax` | float | `%.3f` | g | X-axis acceleration |
| `ay` | float | `%.3f` | g | Y-axis acceleration |
| `az` | float | `%.3f` | g | Z-axis acceleration |

**Example:**
```
ROT,90.53,0.031,-0.022,1.001\r\n
```

**Source:** `Core/Src/main.c:282-283`

### 4.3 Stationary Scan Data — 0° to 180° Sweep (2° steps, 91 lines)

```
S,<angle>,<dist1>,<dist2>\r\n
```

| Field | Type | Format | Unit | Description |
|---|---|---|---|---|
| `angle` | int | `%d` | deg | Servo angle (0, 2, 4, ..., 180) |
| `dist1` | float | `%.1f` | cm | Ultrasonic echo #1 (PB12) — median of 3 |
| `dist2` | float | `%.1f` | cm | Ultrasonic echo #2 (PB2) — median of 3 |

**Special Values:**
- `-1.0` — Sensor timeout / no echo

**Example:**
```
S,0,15.0,20.0\r\n
S,2,14.5,19.5\r\n
...
S,180,30.0,35.0\r\n
```

**Source:** `Core/Src/slam.c:150-151`

---

## 5. Timing Diagram

```
Main Sequence (blocking, sequential):
┌─────────────────────────────────────────────────┐
│ IR wait (loop until IR_BTN_START = 0xC2)       │
│   └── USART2: IR code=... cmd=...               │
├─────────────────────────────────────────────────┤
│ motor_pwm_start() + 3s delay                    │
├─────────────────────────────────────────────────┤
│ for step = 1 to 44:                             │
│   ┌───────────────────────────────────────────┐ │
│   │ test_distance_measure(step)               │ │
│   │   └── motor_forward() → poll encoder      │ │
│   │   └── USART2: TEST START/STOP ...         │ │
│   │   └── USART3: STEP,...                    │ │
│   ├───────────────────────────────────────────┤ │
│   │ stationary_scan()                         │ │
│   │   └── servo_write_angle_init(0)           │ │
│   │   └── for angle = 0 to 180 step 2:        │ │
│   │     └── servo_write_angle(angle)          │ │
│   │     └── measure_echo1_median() ×N_SWEEP   │ │
│   │     └── measure_echo2_median() ×N_SWEEP   │ │
│   │     └── USART3: S,angle,d1,d2             │ │
│   ├───────────────────────────────────────────┤ │
│   │ if step % 11 == 0:                        │ │
│   │   test_rotate_90()                        │ │
│   │     └── motor_turn_left() → poll gyro     │ │
│   │     └── USART2: gyro_offset/ROTATE DONE   │ │
│   │     └── USART3: ROT,...                   │ │
│   └───────────────────────────────────────────┘ │
└─────────────────────────────────────────────────┘
```

- **USART2 messages**: Transmitted at various points during each step
- **USART3 messages per step**: 1× STEP + 91× S + (1× ROT every 11 steps)
- **Total USART3 data**: 44 × (1 + 91) + 4 × 1 = **4,048 lines** per full run

---

## 6. Error Handling

| Condition | Behavior |
|---|---|
| **Ultrasonic timeout (>30ms)** | Returns `-1.0` (skipped in median filtering) |
| **Servo settling** | `servo_write_angle_init()` uses 10 pulses (~200ms) for initial homing; `servo_write_angle()` uses 2 pulses (~40ms) for step moves |
| **Encoder timeout (5000ms)** | Distance loop breaks and reports partial pulse count |
| **Gyro timeout (5000ms)** | Rotation loop breaks with partial yaw |
