# LCD-SPI Serial ICD

## 1. Hardware Overview

| 항목 | 값 |
|------|-----|
| MCU | STM32F103RBT6 (LQFP64) |
| Core Clock | 64 MHz (HSE 8MHz → PLL x16) |
| Board | NUCLEO-F103RB |

### UART Ports

| Port | TX Pin | RX Pin | Baud | Parity | Stop Bits | Flow Ctrl | 용도 |
|------|--------|--------|------|--------|-----------|-----------|------|
| USART2 | PA2 | PA3 | 115200 | None | 1 | None | USB Virtual COM Port (main debug/control) |
| USART3 (HC-06) | PC10 | PC11 | 115200 | None | 1 | None | 블루투스 무선 조종 (Partial Remap) |

- USART2는 CubeMX `.ioc`에 정식 등록됨.
- USART3는 CubeMX baseline에 없으며, `app_protocol.c::Protocol_InitHc06Uart()`에서 AFIO Partial Remap + GPIO PC10/PC11을 직접 초기화.

## 2. Protocol Overview

- **Type**: Single-character command / JSON response
- **RX**: Interrupt-driven circular ring buffer (64 bytes each for USB and BT)
- **TX**: Blocking with timeout, JSON format
- **Line handling**: CR/LF are silently discarded; any single printable character triggers action immediately
- **Case**: Letters are case-insensitive (converted to uppercase)

## 3. TX Format (MCU → PC/BT)

All responses are JSON. Common template:

### 3.1 Boot

```json
{"type":"boot","build_id":"MICROGUARD_SELFTEST_DASH_AUDIT_20260703_I","fw":"microguard_amr_lite","tft":"st7735_spi","motor":"tim_pwm_v31","sensors":"runtime_on","authority":"sensor_drive_guard","field_safe":0,"status_stream":0}
```

### 3.2 Ack

```json
{"type":"ack","cmd":"MOTOR_ARM","result":"OK","reason":"PWM_READY_ARMED"}
```

### 3.3 Full State (V command)

```json
{"type":"verify","build_id":"...","fw":"microguard_amr_lite","basis":"tft_baseline_pinmap_v31","cmds":"K/M/U/R/0/X/1-8/F1-8/QWEASDZC/T/W/N/P/G/H/O/Y/I/J/B/L/V/?","checks":{...},"selftest":{...},"rt":{...},"motor":{...},"ultrasonic":{...},"line":{...},"ir":{...},"outputs":{...}}
```

### 3.4 Status (polled state)

```json
{"v":"mg/1.0","type":"state","build_id":"...","seq":...,"rx":...,"safety":{...},"motor":{...},"sensors":{...},"outputs":{...}}
```

### 3.5 Compact State (APP_STATUS_STREAM_ENABLE)

```json
{"v":"mg/1.0","type":"state_compact","build_id":"...","seq":...,"rx":{...},"tx":{...},"bt":{...},"safety":{...},"motor":{...},"line":{...},"us":{...},"ir":{...},"env":{...},"outputs":{...}}
```

### 3.6 UI Map Frame (`>` stream or `=` one-shot)

```json
{"v":"mg/1.0","type":"ui_map","build_id":"...","t_ms":...,"frame":"front_ultrasonic_2p5d","stream":1,"period_ms":200,"front_left_cm":...,"front_right_cm":...,"us1_cm":...,"us1_valid":...,"us1_echo_raw":...,"us1_pulse_us":...,"us1_status":"...","us2_cm":...,"us2_valid":...,"us2_echo_raw":...,"us2_pulse_us":...,"us2_status":"...","edges":...,"timeouts":...,"reason":"STREAM"}
```

### 3.7 Field Diag (`?`)

Multi-line JSON/human-readable diagnostic output over 8-second capture window. Collects sound, joystick, MPU, line sensor, IR remote variations.

## 4. RX Command Table (PC/BT → MCU)

### 4.1 Motor & Drive Commands (Letter Map)

| Key | Uppercase | Action | Ack Cmd |
|-----|-----------|--------|---------|
| `q` / `Q` | `Q` | Forward-Left diagonal | `REMOTE_MOVE` |
| `w` / `W` | `W` | Forward (or Line Trace cruise if enabled) | `REMOTE_MOVE` / `LINE_TRACE_CRUISE` |
| `e` / `E` | `E` | Forward-Right diagonal | `REMOTE_MOVE` |
| `a` / `A` | `A` | Left | `REMOTE_MOVE` |
| `s` / `S` | `S` | Backward | `REMOTE_MOVE` |
| `d` / `D` | `D` | Right | `REMOTE_MOVE` |
| `z` / `Z` | `Z` | Backward-Left diagonal | `REMOTE_MOVE` |
| `c` / `C` | `C` | Backward-Right diagonal | `REMOTE_MOVE` |

### 4.2 Remote Digit Map (Numpad Layout)

| Key | Direction | Move |
|-----|-----------|------|
| `1` | Forward-Left | APP_MOVE_Q |
| `2` | Forward | APP_MOVE_W |
| `3` | Forward-Right | APP_MOVE_E |
| `4` | Left | APP_MOVE_A |
| `5` | Backward | APP_MOVE_S |
| `6` | Right | APP_MOVE_D |
| `7` | Backward-Left | APP_MOVE_Z |
| `8` | STOP | APP_MOVE_STOP |
| `9` | Backward-Right | APP_MOVE_C |

When `8` is pressed: `OperatorInputs_ClearRemoteMove()`, `LineTrace_SetEnabled(0,"REMOTE_STOP")`, `Safety_StopNow(ctx)`.

### 4.3 Motor Authority

| Key | Ack Cmd | Action |
|-----|---------|--------|
| `K` | `MOTOR_PWM_ENABLE` | PWM unlock for bring-up (only if `APP_MOTOR_PWM_UNLOCK_KEY_ENABLE`) |
| `M` | `MOTOR_ARM` | Motor arm: enable PWM, set `motor_armed=1` |
| `U` | `MOTOR_DISARM` | Motor disarm: clear remote move, set `motor_armed=0`, disable line trace |
| `R` | `EMERGENCY_CLEAR` | Emergency clear: reset latch, clear remote move, disable line trace |

### 4.4 Emergency Stop

| Key | Ack Cmd | Action |
|-----|---------|--------|
| `X` or `0` | `STOP` | Immediate stop: clear remote move, disable line trace, `Safety_StopNow()` |

### 4.5 Line Trace

| Key | Ack Cmd | Action |
|-----|---------|--------|
| `T` | `LINE_TRACE_CRUISE` | Enable line trace + cruise forward |
| `N` | `LINE_TRACE` | Disable line trace |
| `L` | `LINE_TRACE_CRUISE` | Enable line trace + cruise (left-biased variant) |

When `W` is pressed while line trace is enabled: goes into cruise mode instead of manual move.

### 4.6 Motor Proof (Individual Wheel Test)

| Sequence | Ack Cmd | Action |
|----------|---------|--------|
| `F` + `1`..`8` | `MOTOR_PROOF` | Prefix `F` sets proof flag; next digit 1-8 executes proof. |
| `1`..`8` (direct) | `MOTOR_PROOF` | If `APP_MOTOR_PROOF_ONEKEY_ENABLE`: direct proof without `F` prefix. |

Proof ID mapping:
| Key | Wheel |
|-----|-------|
| `1` | LF forward |
| `2` | RF forward |
| `3` | LB forward |
| `4` | RB forward |
| `5` | LF backward |
| `6` | RF backward |
| `7` | LB backward |
| `8` | RB backward |

### 4.7 TFT Display Diagnostics

| Key | Ack Cmd | Action |
|-----|---------|--------|
| `P` | `TFT_PURPLE` | Show purple diagnostic dashboard |
| `G` | `TFT_GRID` | Show geometry offset/clip grid |
| `H` | `TFT_COLOR` | Show RGB color check (purple diagnostic) |
| `O` | `TFT_X_OFFSET` | Cycle X offset, report current offset/inversion/MADCTL |
| `Y` | `TFT_Y_OFFSET` | Cycle Y offset, report current offset/inversion/MADCTL |
| `I` | `TFT_INVERSION` | Toggle display inversion, report current state |
| `J` | `TFT_MADCTL` | Cycle MADCTL register (display rotation/orientation) |

### 4.8 UI Map Stream Control

| Key | Ack Cmd | Action |
|-----|---------|--------|
| `>` | `UI_MAP_STREAM` | Enable periodic UI map frame (period: `APP_UI_MAP_STREAM_PERIOD_MS` = 200ms) |
| `<` | `UI_MAP_STREAM` | Disable UI map stream |
| `=` | (included in JSON) | One-shot UI map frame |

### 4.9 Other

| Key | Ack Cmd | Action |
|-----|---------|--------|
| `B` | `BUZZER_BEEP` | Buzzer beep (PB7, 2200Hz, 120ms, non-blocking) |
| `V` | (verify JSON) | Send full verification JSON state |
| `?` | (field diag) | Start 8-second field diagnostic capture |
| (any other) | `UNKNOWN` | Ack with `DENIED` + hex code |

## 5. Protocol State Machine

### 5.1 RX Processing

```
Protocol_Service()
  ├── Protocol_RingPop(&s_usb_ring) → Protocol_ProcessRxChar("USART2_USB_IRQ", from_bt=0)
  └── Protocol_RingPop(&s_bt_ring)  → Protocol_ProcessRxChar("HC06_USART3_IRQ", from_bt=1)
       └── Protocol_HandleOneKey(ctx, ch)
            ├── Motor/Drive commands (letter map)
            ├── Remote digit commands (numpad map)
            ├── Motor authority (M, U, K, R)
            ├── Emergency stop (X, 0)
            ├── Line trace (T, N, L)
            ├── Motor proof (F+digit, or direct digit)
            ├── TFT diag (P, G, H, O, Y, I, J)
            ├── UI map stream (>, <, =)
            ├── Buzzer (B)
            ├── Verify (V)
            ├── Field diag (?)
            └── UNKNOWN ack
```

### 5.2 Motor Proof Prefix State

- `s_proof_prefix` flag, default 0.
- Pressing `F` sets flag to 1.
- Next `1`-`8` executes motor proof for that wheel.
- Any other character clears the flag and sends DENIED.

### 5.3 UI Map Stream State

- `s_ui_map_stream_enabled` flag, default 0.
- `>` sets to 1, `<` sets to 0.
- When enabled, `Protocol_Service()` sends `type=ui_map` JSON every `APP_UI_MAP_STREAM_PERIOD_MS` (200ms).
- `=` sends one frame regardless of flag.

## 6. Error Handling

| Condition | Behavior |
|-----------|----------|
| Unknown command | JSON ack: `{"type":"ack","cmd":"UNKNOWN","result":"DENIED","reason":"UNKNOWN_LINE","ascii_hex":"0xNN"}` |
| TX timeout (USB) | Increment `s_tx_error_count`, report in state JSON |
| TX timeout (BT) | Increment `s_bt_tx_error_count`, report in state JSON |
| TX buffer truncation | Increment `s_tx_truncate_count`, report in state JSON |
| RX ring buffer overflow | Increment `dropped` / `overrun` per ring, reported in state JSON |
| HC-06 init failure | `s_bt_ready=0`, `s_bt_status="USART3_INIT_FAIL"`, BT path skipped |
| CR/LF in RX | Silently discarded |

## 7. JSON Type Reference

| `type` field | Trigger | Description |
|-------------|---------|-------------|
| `boot` | Power-on | Build ID, FW name, HW config summary |
| `ack` | Any command | Command acknowledgment with result/reason |
| `state` | `V` command | Full subsystem snapshot (motor, sensors, outputs) |
| `state_compact` | Periodic (if enabled) | Lightweight state for continuous monitoring |
| `verify` | `V` command | Certification-oriented detailed state + diagnostics |
| `ui_map` | `>` stream / `=` | Ultrasonic range frame for 2.5D UI mapping |
| (diag lines) | `?` command | Field diagnostic capture output |

## 8. Data Flow Diagram

```
┌──────────────┐     USART2 (PA2/PA3)     ┌──────────────────┐
│  PC (Tera    │ ◄──────────────────────►  │                  │
│  Term/USB)   │   115200 8N1             │   STM32F103RBT6  │
└──────────────┘                           │                  │
                                           │  app_protocol.c  │
┌──────────────┐     USART3 (PC10/PC11)    │  ┌────────────┐  │
│  HC-06 BT    │ ◄──────────────────────►  │  │ Ring Buffer│  │
│  Module      │   115200 8N1             │  │ (64 bytes) │  │
└──────────────┘                           │  └────────────┘  │
                                           │        │         │
                                           │  Protocol_Service │
                                           │  (polled in loop) │
                                           │        │         │
                                           │  ┌─────▼──────┐  │
                                           │  │ Protocol_  │  │
                                           │  │ HandleOne  │  │
                                           │  │ Key()      │  │
                                           │  └─────┬──────┘  │
                                           │        │         │
                                           │  ┌─────▼──────┐  │
                                           │  │ Motor /    │  │
                                           │  │ Sensor /   │  │
                                           │  │ Display    │  │
                                           │  │ Modules    │  │
                                           │  └────────────┘  │
                                           └──────────────────┘
```

## 9. ICD 버전 관리

| 일자 | 버전 | 변경 내역 |
|------|------|-----------|
| 2026-07-06 | v1.0 | 초안 작성 (분석 기반: app_protocol.c, app_config.h, LCD-SPI.ioc) |
