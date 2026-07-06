# 4WD_Control Serial ICD

## 1. Hardware Overview

| 항목 | 값 |
|------|-----|
| MCU | STM32F103RBT6 (LQFP64) |
| Core Clock | 64 MHz (HSI 8MHz → PLL x16) |
| Board | NUCLEO-F103RB |

### UART Port

| Port | TX Pin | RX Pin | Baud | Parity | Stop Bits | Flow Ctrl | 용도 |
|------|--------|--------|------|--------|-----------|-----------|------|
| USART2 | PA2 | PA3 | 115200 | None | 1 | None | HC-06 블루투스 모듈 (무선 조종 + 디버그) |

- 단일 USART2 포트만 사용. CubeMX `.ioc`에 USART2 정식 등록.
- RX는 DMA1 Channel6 (Normal mode, 1-byte circular)로 수신.
- HC-06 블루투스 모듈이 USART2에 직접 연결됨 (main.h: `BT_TX_Pin=PA2`, `BT_RX_Pin=PA3`).

## 2. Protocol Overview

- **Type**: AT-style ASCII line-based command / plain text response
- **RX**: DMA interrupt → `HAL_UART_RxCpltCallback` → line buffer (64 bytes, `\r`/`\n` terminated)
- **TX**: Blocking `HAL_UART_Transmit` with `HAL_MAX_DELAY`
- **Terminator**: `\r` 또는 `\n` (CR/LF 모두 허용, 첫 번째로 도착한 것으로 라인 종료)
- **Buffer overflow**: 64바이트 초과 시 `rx_overflow=1`, 해당 라인 폐기

## 3. TX Format (MCU → BT/PC)

### 3.1 Boot Diagnostics

```
TOF OK\r\n
MPU OK\r\n
PARAM LOADED\r\n
```

또는

```
TOF FAIL\r\n
MPU FAIL\r\n
PARAM DEFAULT\r\n
```

### 3.2 Periodic Stream (50ms 간격)

#### YAW
```
YAW=<centi_degrees>\r\n
```
- YAW in centi-deg (degrees × 100)

#### SPD (Motor Speed & Encoder)
```
SPD=<LF_set>,<LF_meas>,<RF_set>,<RF_meas>,<LR_set>,<LR_meas>,<RR_set>,<RR_meas>;CNT=<LF>,<RF>,<LR>,<RR>;HALL=<LF>,<RF>,<LR>,<RR>\r\n
```
- Setpoint/measured in deci-RPM (RPM × 10)
- CNT: encoder pulse count
- HALL: hall sensor level (0/1)

#### TOF (Time-of-Flight, VL53L1X)
```
TOF=<front_mm>,<left_mm>,<right_mm>;OBS=<0/1>;RDY=<front>,<left>,<right>;PRES=<front>,<left>,<right>;STAT=<front>,<left>,<right>;RAW=<front>,<left>,<right>;ACK=<def>,<front>,<left>,<right>\r\n
```
- OBS: obstacle detected flag
- RDY: sensor ready flag
- PRES: sensor present flag
- STAT: range status
- RAW: raw distance in mm (before range gating)
- ACK: I2C device acknowledge (0x52, 0x54, 0x56, 0x58)

### 3.3 Command Responses

성공 시 현재 FSM 상태 이름 반환:

```
STRAIGHT\r\n
IDLE\r\n
LINE TRACE\r\n
TURN\r\n
AVOID\r\n
MANUAL\r\n
```

실패 또는 알 수 없는 명령:

```
ERROR\r\n
```

응급 상태에서 `AT+RST` 실행 시:

```
EMERGENCY\r\n
```

### 3.4 Parameter Response (`AT+GET`)

```
R=<value>\r\n
BIAS=<value>\r\n
YAW=<value>\r\n
```
- R: Kalman measurement noise variance (micro-units, ×1,000,000)
- BIAS: gyro bias (milli-deg/s, ×1,000)
- YAW: current yaw angle (centi-deg, ×100)

### 3.5 Save Response (`AT+SAVE`)

```
SAVED\r\n
SAVE_FAIL\r\n
```

## 4. RX Command Table (BT/PC → MCU)

| Command | Function | FSM State | Ack |
|---------|----------|-----------|-----|
| `AT+FWD` | 전진 (Yaw PID 보정 직진) | `STRAIGHT` | State name |
| `AT+LEFT` | 좌회전 (90° point turn) | `TURN` | State name |
| `AT+RIGHT` | 우회전 (90° point turn) | `TURN` | State name |
| `AT+STOP` | 정지 | `IDLE` | State name |
| `AT+MAN` | 수동 조종 모드 전환 | `MANUAL` | State name |
| `AT+LINE` | 라인 트레이싱 모드 전환 | `LINE_TRACE` | State name |
| `AT+RST` | 응급 상태 해제 (latch 클리어) | `IDLE` | State name / `EMERGENCY` |
| `AT+GET` | Kalman 필터 파라미터 요청 | (변화 없음) | `R=... BIAS=... YAW=...` |
| `AT+SAVE` | Kalman 파라미터 Flash 저장 요청 | (변화 없음) | `SAVED` / `SAVE_FAIL` |
| (unknown) | - | (변화 없음) | `ERROR` |

### 4.1 Direction Mapping

| Command | FSM Motion | 동작 |
|---------|-----------|------|
| `AT+FWD` | `FSM_MOTION_FORWARD` | IDLE/다른 상태 → `STRAIGHT`; MANUAL 상태 → 수동 방향 `FORWARD` |
| `AT+LEFT` | `FSM_MOTION_LEFT` | IDLE/다른 상태 → `TURN` (90° left); MANUAL 상태 → 수동 방향 `LEFT` |
| `AT+RIGHT` | `FSM_MOTION_RIGHT` | IDLE/다른 상태 → `TURN` (90° right); MANUAL 상태 → 수동 방향 `RIGHT` |
| `AT+STOP` | `FSM_MOTION_STOP` | IDLE 상태로 전환 |

### 4.2 Note on `AT+MAN` + Direction

`AT+MAN`으로 MANUAL 상태에 진입한 후 `AT+FWD`/`AT+LEFT`/`AT+RIGHT`를 보내면 FSM 상태 변경 없이 `manual_motion`만 갱신되어 모터 방향이 즉시 바뀐다. `AT+STOP`은 MANUAL 상태에서도 `manual_motion=STOP`으로 처리된다.

## 5. FSM State Machine

```
                    ┌─────────┐
                    │  IDLE   │ ◄────────── AT+STOP
                    └────┬────┘
                         │
              ┌──────────┼──────────┐
              ▼          ▼          ▼
         AT+FWD     AT+LEFT    AT+RIGHT
              │     AT+RIGHT   AT+LEFT
              ▼          ▼          ▼
         ┌─────────┐ ┌─────────┐ ┌─────────┐
         │STRAIGHT │ │  TURN   │ │  TURN   │
         │(yaw PID)│ │(90° pt) │ │(90° pt) │
         └────┬────┘ └────┬────┘ └────┬────┘
              │           │           │
              │     turn_accum ≥ 90°  │
              │           │           │
              └───────────┴───────────┘
                         │ auto
                         ▼
                    ┌─────────┐
                    │STRAIGHT │
                    └────┬────┘
                         │
              obstacle ──┤
                         ▼
                    ┌─────────────┐
                    │   AVOID     │
                    │(decel→turn→ │
                    │ clear 300ms)│
                    └──────┬──────┘
                           │ auto
                           ▼
                      ┌─────────┐
                      │STRAIGHT │
                      └─────────┘

    ┌─────────┐   AT+LINE      ┌─────────┐   AT+MAN      ┌─────────┐
    │  IDLE   │ ───────────►   │LINE_TRACE│ ───────────►  │ MANUAL  │
    └─────────┘                └──────────┘               └─────────┘
         ▲                          │                         │
         │     AT+STOP              │ line_sensor lost        │ AT+FWD/LEFT/RIGHT
         └──────────────────────────┘                         │ (no state change)
                                                              ▼
                                                         motor direction
                                                         changes immediately

    ┌─────────────┐
    │ EMERGENCY   │ (latched)
    │ (Motor Stop)│
    └──────┬──────┘
           │ AT+RST
           ▼
       ┌─────────┐
       │  IDLE   │
       └─────────┘
```

### FSM States

| State | Description |
|-------|-------------|
| `IDLE` | 정지, 모든 모터 off |
| `STRAIGHT` | 전진, yaw PID로 직진 보정 + S-Curve 가속 |
| `LINE_TRACE` | 라인 트레이싱, line sensor PID로 조향 |
| `TURN` | 90도 point turn (지정 방향으로 회전 후 자동 STRAIGHT 전환) |
| `AVOID` | 장애물 회피 (감속 → 90° 턴 → 300ms 직진 → 자동 STRAIGHT 전환) |
| `MANUAL` | 수동 방향 제어 (AT+FWD/LEFT/RIGHT로 실시간 방향 전환) |
| `EMERGENCY` | 응정 정지 (latch, AT+RST로만 해제) |

## 6. Error Handling

| Condition | Behavior |
|-----------|----------|
| Unknown command | `ERROR\r\n` |
| EMERGENCY 상태에서 명령 | EMERGENCY 상태 유지, Motor_StopAll 유지 |
| EMERGENCY 상태에서 AT+SAVE | g_param_save_request 플래그 → `SAVED` 가능 (모터 무관) |
| EMERGENCY 상태에서 AT+RST | 응급 latch 클리어 시도, 성공 시 `IDLE`, 실패 시 `EMERGENCY` |
| RX line overflow (≥64 bytes) | `rx_overflow=1`, 해당 라인 폐기 |
| RX 종결 문자 없음 | 명령 미처리, 누적된 데이터는 다음 CR/LF에서 한꺼번에 처리 |
| TX (HAL_UART_Transmit) | `HAL_MAX_DELAY` 무한 대기 (blocking), timeout 없음 |

## 7. Data Flow Diagram

```
┌──────────────┐     USART2 (PA2/PA3)     ┌──────────────────────┐
│  HC-06 BT    │ ◄──────────────────────►  │                      │
│  Module      │   115200 8N1             │  STM32F103RBT6       │
└──────────────┘                           │  64MHz HSI           │
                                           │                      │
                                           ├──────────────────────┤
                                           │  hc06.c              │
                                           │  ┌──────────────┐    │
                                           │  │ DMA1 Ch6     │    │
              RX ──────────────────────────►  │ (1-byte      │    │
                                           │  │  circular)   │    │
                                           │  └──────┬───────┘    │
                                           │         ▼            │
                                           │  ┌──────────────┐    │
                                           │  │ rx_line[64]  │    │
                                           │  │ (line buf)   │    │
                                           │  └──────┬───────┘    │
                                           │         ▼            │
                                           │  ┌──────────────┐    │
                                           │  │ HC06_Parse() │    │
                                           │  │ (AT cmd tbl) │    │
                                           │  └──────┬───────┘    │
                                           │         │            │
                                           │  ┌──────▼───────┐    │
                                           │  │   FSM.c      │    │
                                           │  │   Motor.c    │    │
                                           │  │   Kalman.c   │    │
                                           │  │   ParamStore │    │
                                           │  └──────────────┘    │
                                           │                      │
              TX ◄──────────────────────────  main.c loop        │
                                           │  (50ms stream)       │
                                           │  - YAW, SPD, TOF     │
                                           └──────────────────────┘
```

## 8. ICD 버전 관리

| 일자 | 버전 | 변경 내역 |
|------|------|-----------|
| 2026-07-06 | v1.0 | 초안 작성 (분석 기반: hc06.c, fsm.c/h, main.c, main.h, 4WD_Control.ioc) |
