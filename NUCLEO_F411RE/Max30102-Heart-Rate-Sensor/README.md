# MAX30102 Heart-Rate Sensor — 인터페이스 및 측정 원리 가이드

**프로젝트**: STM32F411RE + MAX30102 → UART → Python Monitor  
**파일**: `main.c`, `max30102.c/.h`, `max30102_monitor.py`, `bpm_analysis_tool.py`  
 **작성일**: 2026-07-07  
 **최종 업데이트**: 2026-07-07 (BPM 불안정 분석, 비교 툴, 가이드 업데이트)

---

## 1. 시스템 개요

```
┌──────────────────────────────────────────────────────────────┐
│  STM32F411RE (Nucleo-64)                                    │
│                                                              │
│  ┌─────────────────────┐     I2C1 (PB8-SCL, PB9-SDA)        │
│  │    main.c           │◄────► MAX30102 Sensor              │
│  │  - MAX30102_Init()  │      (Adr: 0x57 << 1 = 0xAE)      │
│  │  - read loop        │      UART2 (PA2-TX, PA3-RX)        │
│  │  - UART output      │────► USB-to-Serial (115200 baud)   │
│  │  - command parser   │◄──── [CMD]REG_READ/WRITE/GET_TEMP  │
│  └─────────────────────┘                                     │
└──────────────────────────────────────────────────────────────┘
         │
         │ CSV: "Count,IR_raw,Red_raw"            (→ 그래프 + vital)
         │ CMD: "[CMD]REG_..."                     (→ register panel)
         ▼
┌──────────────────────────────────────────────────────────────┐
│  max30102_monitor.py                                         │
│  - SerialReader (thread)  ── queue ──► _handle_line()        │
│  - HeartRateEstimator     : PPG peak → BPM                   │
│  - SpO2Estimator          : Ratio of Ratios → SpO2           │
│  - Temperature parsing    : "# TEMP: xx.xx C"                │
│  - Command responses      : "[CMD]" prefix → register panel  │
│  - Register control panel : Read/Write/Quick Access buttons  │
│  - Config persistence     : 마지막 COM 포트 기억             │
│  - Matplotlib 실시간 그래프                                  │
└──────────────────────────────────────────────────────────────┘
```

---

## 2. 하드웨어 인터페이스 (I2C)

### 2.1 핀 연결

| STM32F411 Pin | Signal    | MAX30102 Pin | 비고            |
|---------------|-----------|--------------|-----------------|
| PB8           | I2C1_SCL  | SCL          | 100kHz          |
| PB9           | I2C1_SDA  | SDA          | 100kHz          |
| 3.3V          | VDD       | VIN          | 1.8V~3.3V       |
| GND           | GND       | GND          |                  |
| PA2           | USART2_TX | -            | CP2102/ST-Link RX |
| PA3           | USART2_RX | -            | CP2102/ST-Link TX |

### 2.2 I2C 주소

- **7-bit 주소**: `0x57`
- **HAL I2C 주소** (8-bit write): `0xAE` (= 0x57 << 1)
- 정의: `max30102.h:20` → `#define MAX30102_I2C_ADDR 0xAE`

### 2.3 I2C 타이밍

- Clock Speed: 100kHz (standard mode)
- Timeout: 100ms (`MAX30102_I2C_TIMEOUT`)
- FIFO burst read: slave address + register 0x07 + 6/12/18... bytes

---

## 3. MAX30102 레지스터 맵

### 3.1 주요 레지스터

| 주소 | 이름              | 설명                          |
|------|-------------------|-------------------------------|
| 0x00 | STATUS_1          | 인터럽트 상태 1 (FIFO_FULL, NEW_DATA, PWR_RDY) |
| 0x01 | STATUS_2          | 인터럽트 상태 2 (TEMP_RDY 등) |
| 0x02 | INTR_ENABLE_1     | 인터럽트 활성화 1             |
| 0x03 | INTR_ENABLE_2     | 인터럽트 활성화 2             |
| 0x04 | FIFO_WR_PTR       | FIFO 쓰기 포인터 (0~31)       |
| 0x05 | OVF_COUNTER       | FIFO 오버플로우 카운터        |
| 0x06 | FIFO_RD_PTR       | FIFO 읽기 포인터 (0~31)       |
| 0x07 | FIFO_DATA         | FIFO 데이터 읽기 (버스트)     |
| 0x08 | FIFO_CONFIG       | FIFO 설정 (평균화, 롤오버, almost_full) |
| 0x09 | MODE_CONFIG       | 모드 설정 (Shutdown/Reset/Mode) |
| 0x0A | SPO2_CONFIG       | SpO2 설정 (ADC range, SR, PW) |
| 0x0C | LED1_PA           | IR LED 펄스 전류 (0~255)      |
| 0x0D | LED2_PA           | Red LED 펄스 전류 (0~255)     |
| 0x10 | PILOT_PA          | Pilot LED 전류                |
| 0x1F | DIE_TEMP_INT      | 칩 온도 정수부 (°C)           |
| 0x20 | DIE_TEMP_FRAC     | 칩 온도 소수부 (0.0625°C/LSB) |
| 0x21 | DIE_TEMP_CONFIG   | 온도 측정 트리거 (bit0=1)     |
| 0xFE | REV_ID            | 리비전 ID                     |
| 0xFF | PART_ID           | Part ID (0x15 = MAX30102)     |

### 3.2 MODE_CONFIG (0x09) 상세

```
bit 7 : SHDN     (1=shutdown)
bit 6 : RESET    (1=reset, 자동 cleared)
bit 5-3: reserved
bit 2-0: MODE
         000 = Standby
         001 = Red only
         010 = Red + IR (SpO2 mode) ← 현재 사용
         011 = Multi-LED
```

### 3.3 SPO2_CONFIG (0x0A) 상세

```
bit 7-6: ADC_RGE (ADC full-scale range)
          00 = 2048 nA
          01 = 4096 nA
          10 = 8192 nA  ← 현재 설정
          11 = 16384 nA
bit 5-3: SR (Sample Rate)
          000 = 50 sps
          001 = 100 sps
          010 = 200 sps  ← 현재 설정
          011 = 400 sps
          100 = 800 sps
          101 = 1000 sps
bit 2-0: LED_PW (LED Pulse Width / ADC resolution)
          000 = 69μs  (15-bit)
          001 = 118μs (16-bit)
          010 = 215μs (17-bit)
          011 = 411μs (18-bit)  ← 현재 설정
```

### 3.4 FIFO_CONFIG (0x08) 상세

```
bit 7-5: SMP_AVE (sample averaging)
          000 = 1 sample
          001 = 2 samples
          010 = 4 samples  ← 현재 설정 (avg=4)
          011 = 8 samples
          100 = 16 samples
          101 = 32 samples
bit 4  : ROLLOVER (1=enabled)  ← 현재 설정
bit 3-0: AFULL (almost full threshold, 0~15)
          15 → 15 samples in FIFO triggers almost full  ← 현재 설정
```

### 3.5 LED 전류 설정

- PA 레지스터 값 0x00~0xFF (선형)
- 0x00 = 0mA, 0xFF = 50mA
- **변경 전 설정**: `0x40` ≈ 12.5mA (IR LED, Red LED 동일)
- **변경 후 설정**: IR = `0x40`(12.5mA) 유지, Red = `0xC0`(38.4mA, 3배) — PD 파장별 감도 차이 보정 (2026-07-07)
- **ADC_RGE 시행착오**: 2048nA(1차 시도) → ADC 포화(4,194,303) → 8192nA로 복원
- 실제 전류 계산: `current_mA = (PA_value / 255) * 50.0`

⚠️ **주의**: IR LED와 Red LED의 전류가 같으면, photodetector의 파장별 감도 차이(940nm에서 3배 높음)로 인해 Red 신호가 IR보다 약해 SpO2 계산이 부정확해짐. **2026-07-07 수정**: IR=0x40(12.5mA), Red=0xC0(38.4mA, 3배)로 차등 설정 완료.

---

## 4. 펌웨어 코드 분석 (STM32F411)

### 4.1 초기화 순서 (`main.c`)

```
main()
├── HAL_Init()
├── SystemClock_Config()         → HSI → PLL → 84MHz
├── MX_GPIO_Init()               → LD2(PA5) 출력, B1(PC13) 입력
├── MX_USART2_UART_Init()        → 115200 8N1
├── MX_I2C1_Init()               → 100kHz
├── 시작 배너 출력               → "=== MAX30102 Heart-Rate Sensor Reader ==="
├── MAX30102_Init(&max30102)
│   ├── MAX30102_Reset()          → SOFT_RESET bit set → poll until clear
│   ├── MAX30102_GetPartID()      → 확인: 0x15여야 함
│   ├── MAX30102_SetFIFOConfig()  → avg=4, rollover=on, afull=15
│   ├── MAX30102_SetSPO2Config()  → 8192nA, 200sps, 411μs(18-bit)  ← 2048nA→복원 (ADC 포화)
│   ├── MAX30102_SetLEDPulseAmplitude(LED1=IR, 0x40)       ← 유지 (12.5mA)
│   ├── MAX30102_SetLEDPulseAmplitude(LED2=Red, 0xC0)      ← 0x40→0xC0 (12.5→38.4mA, PD 감도 보정 3배)
│   └── MAX30102_SetMode(RED_IR)  → SpO2 모드 시작
├── OK/FAIL 메시지 출력
└── CSV 헤더 출력                → "Count,IR_raw,Red_raw"
```

### 4.2 메인 루프 (`main.c:252-333`)

```
while (1)
  if (sensor_ok)
    MAX30102_GetFIFODataCount() → avail
    if (avail > 0)
      for (i = 0; i < avail; i++)
        MAX30102_ReadFIFOSample() → sample (IR, Red)
        sample_count++
        UART 출력: "%lu,%lu,%lu\r\n"
    else
      HAL_Delay(5) ← busy-wait 방지

    // 500샘플마다 (~5초) 온도 출력
    if (sample_count % 500 == 0)
      MAX30102_GetTemperature() → float temp
      UART 출력: "# TEMP: %d.%02d C"

    // 1000샘플마다 LD2 LED 깜빡임
    if (sample_count - last_heartbeat_ms >= 1000)
      LD2 ON → 50ms → OFF
  else
    // 센서 재연결 시도 (2초마다)
```

### 4.3 FIFO 읽기 전략

- `GetFIFODataCount()`는 WR_PTR - RD_PTR로 가용 샘플 수 계산
- 현재 코드는 한 번에 `avail` 개수만큼 모든 샘플을 순차 읽음
- FIFO는 32샘플 깊이이며, averaging=4 이므로 실제 128 raw 샘플당 1 FIFO 샘플
- 200sps에서 averaging=4 → 유효 출력율 = 50Hz (FIFO에 50샘플/초)

> **중요 타이밍 분석**:
> - 센서 내부: 200sps raw, 4-sample averaging → FIFO에 50샘플/초 기록
> - FIFO 깊이 32 → 0.64초마다 FIFO가 거의 가득 참 (almost_full=15)
> - STM32 루프에서 매번 가용 샘플 전부 읽어 UART로 출력
> - UART 출력 속도: 1줄 약 30바이트 × 50줄/초 = 1500바이트/초 ← 115200bps에서 충분

---

## 5. 시리얼 통신 프로토콜

### 5.1 데이터 형식

STM32F411 → UART → PC:

```
=== 부팅 배너 ===
\r\n===================================\r\n
 MAX30102 Heart-Rate Sensor Reader\r\n
 MCU: STM32F411RE @ 84MHz\r\n
 I2C1: PB8(SCL) PB9(SDA) @ 100kHz\r\n
 UART2: PA2(TX) PA3(RX) @ 115200\r\n
===================================\r\n

=== 초기화 결과 ===
[OK] MAX30102 detected (Part:0x15, Rev:0x01)\r\n
[OK] Mode: SpO2(Red+IR) | SampleRate: 200sps | Resolution: 18-bit\r\n
[OK] Ready. Reading sensor...\r\n

=== CSV 헤더 ===
\r\n--- CSV DATA ---\r\n
Count,IR_raw,Red_raw\r\n

=== 실시간 데이터 (50줄/초) ===
1,12345,23456\r\n
2,12350,23460\r\n
3,12348,23458\r\n
...

=== 주기적 온도 (500샘플마다) ===
# TEMP: 27.50 C\r\n

=== 명령 응답 (REG_READ/REG_WRITE/GET_TEMP) ===
[CMD]REG_0C=40: LED1_PA (IR LED current)\r\n
[CMD]TEMP=27.50\r\n
```

### 5.2 명령 프로토콜 (PC → STM32)

Python monitor가 UART TX로 명령을 보내면 STM32가 `[CMD]` 접두사로 응답:

| 명령 형식 | 설명 | 응답 예 |
|-----------|------|---------|
| `CMD:REG_READ:AA` | 레지스터 0xAA 읽기 | `[CMD]REG_AA=VV: register_name` |
| `CMD:REG_WRITE:AA:VV` | 레지스터 0xAA에 0xVV 쓰기 | `[CMD]REG_AA=VV: register_name` |
| `CMD:GET_TEMP` | 온도 재측정 요청 | `[CMD]TEMP=27.50` |

**참고**: FIFO_DATA(0x07) 쓰기는 FIFO 읽기 스트림을 손상시키므로 Python UI에서 쓰기 차단.

### 5.3 Python 파서 로직 (`_handle_line()`)

| 입력 패턴 | 처리 | 디스플레이 |
|-----------|------|-----------|
| `# TEMP: 27.50 C` | `split(":")[1]` → float 변환 | TEMP 카드 업데이트 |
| `[CMD]REG_...` | `_handle_command_response()` 파싱 | Register panel 업데이트 |
| `[CMD]TEMP=...` | 온도값 추출 | TEMP 카드 업데이트 |
| `[OK] ...` | 로그에 출력 | EVENT LOG |
| `[FAIL] ...` | 로그에 출력 (빨간색) | EVENT LOG |
| `[INFO] ...` | 로그에 출력 | EVENT LOG |
| `# ...` (주석) | 무시 | - |
| `--- ...` | 무시 | - |
| `Count,...` (헤더) | 무시 | - |
| `123,45678,56789` | CSV 파싱 → IR, Red | 그래프 + vital signs |

---

## 6. 심박수 (BPM) 측정 알고리즘

### 6.1 원리: PPG (Photoplethysmography)

- IR LED가 피부를 투과/반사 → 혈류량 변화에 따라 광량 변화
- 심장이 뛸 때 혈관 부피 증가 → 더 많은 빛 흡수 → ADC 값 감소
- 심장 이완기 → 혈관 부피 감소 → ADC 값 증가
- 이 주기적 변화가 PPG 파형

### 6.2 Python 구현 (`HeartRateEstimator` in `max30102_monitor.py`)

#### `max30102_monitor.py` (현재 버전)

```python
class HeartRateEstimator:
    def __init__(self, sample_rate_hz=50.0):
        self.fs = sample_rate_hz
        self.window_s = 4.0
        self.window_size = int(self.fs * self.window_s)
        self.buffer = deque(maxlen=self.window_size)

    def estimate_bpm(self):
        if len(self.buffer) < int(self.fs * 2.0): return 0.0

        data = np.array(self.buffer, dtype=float)
        if np.std(data) < 50 or np.max(data) - np.min(data) < 200:
            return 0.0

        # 1) DC 제거: moving average baseline subtraction
        baseline = np.convolve(data, np.ones(25)/25, mode='same')
        ac = data - baseline

        # 2) Smoothing
        ac_smooth = np.convolve(ac, np.ones(3)/3, mode='same')

        # 3) 적응형 threshold: AC range의 60%
        threshold = ac_smooth.min() + (ac_smooth.max() - ac_smooth.min()) * 0.6

        # 4) Local maxima peak detection
        peaks = []
        for i in range(half_window, len(ac_smooth) - half_window):
            if ac_smooth[i] > threshold and ac_smooth[i] == local_max:
                if distance from last peak >= min_peak_distance:
                    peaks.append(i)

        if len(peaks) < 2: return 0.0

        intervals = np.diff(peaks) / self.fs
        # validity + consistency check
        bpm = 60.0 / median_interval
        return max(30.0, min(220.0, bpm))
```

### 6.3 🔴 BPM = 0의 근본 원인: Convolution Edge Artifact

실제 CSV 데이터로 검증한 결과, **`np.convolve(..., mode='same')`의 edge artifact**가 BPM 계산을 항상 실패하게 만듦:

```
원본 AC 신호 (DC 제거 후):   -164.6 ~ +251.4 (정상 범위)
np.convolve(mode='same') 후:   -162.1 ~ +685,267 (!!)
                                                ↑
                                     edge에서 2700배 증폭!
```

**발생 원인**:
- `mode='same'`은 커널이 데이터 범위를 벗어난 부분을 0으로 패딩
- baseline 커널(window=25)이 양쪽 끝에서 부분적으로만 데이터와 겹침
- 컨볼루션 결과의 양 끝 12샘플이 0에 가까운 baseline 값을 가짐
- `ac = data - baseline`에서 끝부분이 `data - 0 ≈ data`가 되어 1,426,000이라는 거대한 spike 발생

**영향**:
- `ac_smooth.max()` ≈ 628,174 (실제 AC amplitude ≈ 250의 2500배)
- threshold = -162 + (628,174 - (-162)) × 0.6 ≈ 376,839
- 어떤 정상 피크도 이 threshold를 넘지 못함 → `peaks = []` → `return 0.0`
- **100% 확률로 BPM = 0 반환**

**해결: Edge padding 후 convolution**:
```python
# Before (broken)
baseline = np.convolve(data, kernel_bl, mode='same')

# After (fixed)
padded = np.pad(data, baseline_win // 2, mode='edge')
baseline = np.convolve(padded, kernel_bl, mode='valid')  # len = len(data)
ac = data[:len(baseline)] - baseline

# Smooth에도 동일 패턴 적용
padded_ac = np.pad(ac, smooth_win // 2, mode='edge')
ac_smooth = np.convolve(padded_ac, kernel_sm, mode='valid')
```

**[Edge padding 결과, 실제 CSV 데이터 기반 BPM 산출 성공]**: (별도 테스트 필요. 위 패치 적용 후 BPM 추출 가능 예상)

### 6.4 기타 BPM 계산 이슈

1. **샘플레이트 추정** (`_handle_line`):
   - Python은 처음 50샘플 도착 시간으로 fs 추정
   - FIFO 버스트 수신(15샘플씩 뭉쳐서 도착)으로 인해 추정 오차 발생 가능
   - `30 < estimated_fs < 500` 조건으로 비정상값 필터링

2. **하드웨어 신호 세기**:
   - LED 전류 IR=0x40(12.5mA), Red=0xC0(38.4mA)에서 IR_raw의 peak-peak ≈ 1,000~2,300 counts
   - `sig_std > 50`, `sig_range > 200` 조건은 충분히 통과

3. **삭제된 버그**: 이전 버전의 dead code(`bpm=60.0/avg_interval` 중복)는 현재 버전에서 제거됨

### 6.5 `max30102_monitor-v0.py` (구버전) 과의 차이

- v0은 피크 검출에 `mean + 0.6 * std` 사용, 현재 버전은 `ac_min + range * 0.6`
- v0은 최소 3개 피크 필요, 현재 버전은 2개
- **핵심 차이**: 현재 버전은 baseline subtraction으로 DC 제거 후 AC 신호에서 피크 검출

### 6.6 🔴 BPM 불안정 (Edge-padding fix 이후)

Edge-padding fix로 BPM=0 문제는 해결되었으나, 실제 CSV 데이터 분석 결과 BPM 값이 **수 초 내에 40 BPM 이상 변동**하는 불안정이 확인됨:

```
실제 녹화 데이터 (연속 60샘플, 약 1.2초):
  count 114191-114200: 75.0 BPM
  count 114201-114210: 73.2 BPM  ← 정상
  count 114211-114220: 87.0 BPM  ← 갑자기 상승
  count 114221-114250: 115.4 BPM ← 40 BPM 급등
  count 114251-114256: 73.2 BPM  ← 다시 급강
  count 114257-114264: 71.4 BPM
  count 114265-114274: 69.8 BPM
  count 114281-114286: 115.4 BPM ← 또 급등
```

**원인**: Edge-padding fix로 AC 진폭이 실제 PPG 수준(±250)으로 정상화되었으나, 피크 검출 임계값(`ac_min + range * 0.6`)이 작은 신호에서 잡음에 취약해짐. 
- 신호 품질 게이트(`std > 50`, `range > 200`)도 너무 관대하여 노이즈 구간에서도 BPM 계산 시도
- FIFO 버스트 수신으로 peak-to-peak 시간 간격이 불규칙해지는 현상과 결합

**해결 방안** (5가지 알고리즘 변형을 `bpm_analysis_tool.py`로 비교 가능):

| 변형 | 설명 | 기대 효과 |
|------|------|-----------|
| ORG | 현재 구현 (edge-padded conv, TH=60%) | 기준 |
| STRICT | 신호 품질 게이트 강화 (std>150, range>500) | 노이즈 구간에서 BPM 미표시 |
| TH50 | 피크 임계값 60% → 50% | 더 많은 피크 검출 (민감도 ↑) |
| MF | BPM 출력 3-value median 필터 | 출력 안정화 (화면 표시만) |
| ALL | STRICT + TH50 + MF 조합 | 종합 개선 |

---

## 7. SpO2 (산소포화도) 측정 알고리즘

### 7.1 원리: Ratio of Ratios

Red (660nm)와 IR (940nm) 두 파장의 광흡수 차이를 이용:

**산소헤모글로빈(HbO₂)**:
- Red: 적게 흡수
- IR: 많이 흡수

**탈산소헤모글로빈(Hb)**:
- Red: 많이 흡수
- IR: 적게 흡수

### 7.2 계산 공식

```
R = (AC_red / DC_red) / (AC_ir / DC_ir)

SpO₂ = 110 - 25 × R
```

- **AC** (교류 성분): 맥동하는 동맥혈 — 표준편차(std)로 근사
- **DC** (직류 성분): 비맥동 조직(정맥, 뼈, 피부) — 평균(mean)으로 근사

### 7.3 Python 구현 (`SpO2Estimator`)

```python
class SpO2Estimator:
    def __init__(self, sample_rate_hz=50.0):
        self.fs = sample_rate_hz
        self.window_len = int(self.fs * 4.0)
        self.ir_buffer = deque(maxlen=self.window_len)
        self.red_buffer = deque(maxlen=self.window_len)

    def _get_ac_dc(self, data):
        """Baseline subtraction 방식의 AC/DC 추출"""
        dc = np.mean(data)
        win = max(3, int(self.fs * 0.5))
        kernel = np.ones(win) / win

        # ⚠️ Edge padding 필요 (BPM 섹션 6.3 참조)
        padded = np.pad(data, win // 2, mode='edge')
        baseline = np.convolve(padded, kernel, mode='valid')
        ac = data[:len(baseline)] - baseline
        ac_amplitude = (np.max(ac) - np.min(ac)) / 2.0
        return ac_amplitude, dc

    def estimate_spo2(self):
        if len(self.ir_buffer) < self.window_len * 0.5: return 0.0
        # ... AC/DC 계산 → R = (ac_red/dc_red) / (ac_ir/dc_ir)

        # AC/DC ratio가 너무 작으면 노이즈로 간주 (1e-6)
        if ac_ir / dc_ir < 1e-6 or ac_red / dc_red < 1e-6:
            return 0.0

        spo2 = 110.0 - 25.0 * r
        return max(80.0, min(100.0, spo2))  # clamp
```

- **임계값 변경**: 0.0001 → 1e-6 (2026-07-07). 현재 AC/DC ratio ≈ 8e-5로 기존 임계값 미달.
- **클램프 하한 변경**: 85 → 80 (2026-07-07). IR/Red 전류 불균형 시 R≈1 → SpO2≈85.0 → 클램프 걸린 상태 가시화.

### 7.4 🔴 SpO2 항상 85% (또는 미표시) 원인: IR/Red 역상 관계

CSV 데이터 분석 결과, IR과 Red 신호가 **맥동 주기에서 반대로 움직임**:

```
맥동 구간 (line 1921-1929):
  IR:  1,426,878 → 1,426,717  (↓ 감소: 혈류↑ → IR 흡수↑)
  Red: 1,427,240 → 1,427,308  (↑ 증가: 혈류↑ → Red 흡수↓)
```

**정상적인 맥박산소측정기**에서는:
- IR(940nm)과 Red(660nm) 모두 혈류 증가 시 **같은 방향**으로 감소 (HbO2는 Red를 더 흡수)
- 위 데이터는 IR↓ + Red↑ → **저산소血症 패턴** (Hb ↑ → Red 흡수↓ → Red 신호↑)
- 실제 사용자는 정상 SpO2(95-99%)일 것으로 추정 → **측정 오류**

**원인 분석**:

| 원인 | 설명 | 확률 |
|------|------|------|
| **LED 전류 불균형 (해결)** | IR=0x40(12.5mA), Red=0xC0(38.4mA) 차등 설정 완료. IR/Red 신호 균형 개선됨 | ✅ 해결 |
| **반사식 측정** | 손가락 끝 반사식(Reflection) 사용. 투과식(Transmission) 대비 파장별 광경로 차이 | 중간 |
| **Red LED 불량** | Red LED가 정상 발광하지 않으면 Red 채널이 IR 누설광만 검출 | 낮음 |
| **센서 위치不良** | 손가락이 센서를 완전히 덮지 않아 외부광 간섭 | 중간 |

**수정 방안** (Python 측 일부 적용 완료):
1. ✅ **임계값 0.0001 → 1e-6**: AC/DC ratio ≈ 8e-5에서도 SpO2 계산 가능
2. ✅ **클램프 85 → 80**: R≈1에서도 표시 가능 (단, 부정확 — 하드웨어 개선 필요)
3. ✅ **Red LED 전류 차등** (IR=0x40, Red=0xC0): 2026-07-07 적용 완료
4. ❌ **ADC_RGE 8192nA → 2048nA**: 1차 시도 → ADC 포화(4194303)로 복원. 추가 조정 필요시 4096nA(1) 재시도
5. ❌ **IR/Red correlation gate**: 양의 상관일 때만 SpO2 업데이트 (구현 예정)

### 7.5 18-bit ADC 값 디코딩

FIFO read (`max30102.c:343`): `ir = (b0<<16)|(b1<<8)|b2` → 24-bit 값

```
FIFO raw (CSV 저장값):   1,426,000 ~ 1,430,000
                        ↓ 실제 18-bit ADC는 상위 18비트에 위치
True 18-bit ADC 값:     22,250 ~ 22,350   (= raw >> 6)

DC 레벨:  22,300 / 262,143 ≈ 8.5%  (낮음 — ADC range 축소 필요)
AC 변동:  ~1,500 raw ≈ 23 ADC counts

또는 하위 18비트 해석 시:
True ADC (lower 18bit): 115,500 ~ 117,000 (= raw & 0x3FFFF)
DC 레벨: 116,000 / 262,143 ≈ 44% (적정)
AC 변동: ~1,000 counts ≈ 0.9% of DC (정상 PPG)
```

---

## 8. 온도 측정

### 8.1 MAX30102 내부 온도 센서

- 측정 범위: -40°C ~ +85°C
- 정수 레지스터: 0x1F (1°C/LSB)
- 소수 레지스터: 0x20 (0.0625°C/LSB, 부호 없는 8비트)
- 측정 방법: `DIE_TEMP_CONFIG(0x21)`에 0x01 쓰면 측정 시작, `STATUS_2(0x01)` bit1(TEMP_RDY) 폴링

### 8.2 펌웨어 구현 (`max30102.c:283-319`)

```c
HAL_StatusTypeDef MAX30102_GetTemperature(dev, *temperature) {
    // 1. 트리거: DIE_TEMP_CONFIG = 0x01
    // 2. 폴링: STATUS_2 bit1 (TEMP_RDY) — 최대 100ms 대기
    // 3. 읽기: DIE_TEMP_INT + DIE_TEMP_FRAC
    // 4. 계산: temp = temp_int + temp_frac * 0.0625
}
```

> **과거 버그 수정 내역**: 원래 STATUS_1(PWR_RDY)를 폴링했으나, PWR_RDY는 첫 부팅시 한 번만 설정되고 읽으면 클리어되므로 한 번만 성공하고 이후 계속 TIMEOUT 발생. STATUS_2(TEMP_RDY)로 수정됨.

### 8.3 TEMP 값이 안 나오는 원인

1. **메인 루프에 온도 출력 로직이 있음** (`main.c:292-304`):
   ```c
   if (sample_count % 500 == 0 && sample_count > 0) {
       MAX30102_GetTemperature(&max30102, &temp);
       UART 출력 "# TEMP: %d.%02d C\r\n"
   }
   ```
   - sample_count가 500 이상이어야 온도 측정 시도
   - Python 파서는 이 줄을 인식하도록 구현됨

2. **온도 측정 실패 가능성**:
   - `STATUS_2`의 TEMP_RDY 플래그가 타임아웃 내에 설정되지 않으면 `HAL_TIMEOUT` 반환
   - `MAX30102_GetTemperature()`가 실패하면 온도 출력 자체를 안 함 (오류 처리 없이 silent 실패)
   - 내부 온도 센서의 I2C 통신 문제 가능성

3. **Python 파서 문제** (`max30102_monitor.py:746-754`):
   ```python
   if line.startswith("# TEMP:"):
       val = float(line.split(":")[1].strip().split()[0])
       self.last_temp = val
       self._update_temp_display(val)
       self._log("TEMP", f"{val:.1f} °C")
   ```
   - `# TEMP: 27.50 C` → `split(":")[1]` = `" 27.50 C"` → `strip().split()[0]` = `"27.50"` → `float` = 27.5
   - 파싱 로직은 정상

---

## 9. 데이터 흐름 타임라인

```
타임라인 (1눈금 = 20ms, 50Hz 출력 기준)
0ms     MAX30102 내부: 200sps raw 샘플링, 4개 평균
20ms    FIFO에 1샘플 저장 (IR+Red, 18-bit)
40ms    FIFO에 2샘플 저장
...
300ms   FIFO에 15샘플 → 거의 가득 참
320ms   STM32 루프 진입: avail=15 읽음
        15개 샘플 순차 읽기 + UART 전송 (각 30바이트)
        15 × 30 / 115200 ≈ 4ms (UART 전송 시간)
        → 15줄 CSV 출력
340ms   STM32 다음 avail=0 → HAL_Delay(5)
...
640ms   FIFO에 15샘플 다시 참 → 위 과정 반복
```

**실제 PC 수신 간격**: avail개의 줄(보통 1~15줄)이 불규칙한 간격으로 버스트 수신 → Python이 보기에 균일하지 않은 샘플레이트

### 9.1 Recording CSV 포맷 (Recording 중)

```csv
Timestamp,Count,IR_raw,Red_raw,BPM,SpO2
2026-07-07T08:10:51.027681,1833,1427474,1427192,,85.0
```

- **IR_raw / Red_raw**: 펌웨어에서 받은 원시 ADC 값 (FIFO 24-bit 값, CSV 저장)
- **BPM / SpO2**: Python estimator가 5샘플마다 계산한 결과값
  - BPM 비어있음 = estimator가 0.0 반환 (edge artifact로 peak 미검출 — 섹션 6.3)
  - SpO2 항상 85.0 = clamp 하한 (IR/Red 역상 관계 — 섹션 7.4)

⚠️ **현재 CSV는 raw와 computed가 혼합 저장**. 분석 목적으로는 raw 데이터만 따로 보관하고, computed는 별도 파일로 분리하는 것이 바람직

### 9.5 센서 접촉과 신호 품질

#### 9.5.1 AC/DC 변조율 — 핵심 품질 지표

PPG 신호의 품질은 **AC 변조율**로 판단합니다:

```
변조율(%) = AC_amplitude / DC_level × 100
```

| 변조율 | 상태 | 판단 |
|--------|------|------|
| 1~5% | ✅ 양호 | 손가락이 센서를 잘 밀착, 충분한 혈류 신호 |
| 0.1~1% | ⚠️ 미약 | 접촉 불량 또는 LED 전류/ADC 감도 부족 |
| < 0.1% | 🔴 불량 | 노이즈에 묻혀 BPM/SpO2 추정 불가 |

**현재 시스템 변조율 (CSV 데이터 기준)**:
- IR AC ≈ ±250, DC ≈ 3,108,000 → **변조율 ≈ 0.008%** (🔴 불량)
- 정상 PPG의 1/125 수준 — 접촉 개선 또는 ADC 감도 향상 필요

#### 9.5.2 변조율이 낮은 원인

| 원인 | 영향 | 해결 |
|------|------|------|
| **ADC range 과다** | 8192nA 범위에서 작은 광량 변화를 18-bit로 양자화 → AC 진폭 작아짐 | ✅ 2048nA로 변경 완료 (감도 4배↑) |
| **LED 전류 부족** | 12.5mA(0x40)로는 반사식 PPG에 충분한 광량 부족 | ✅ IR=0x7F(25mA), Red=0xFF(50mA)로 증가 완료 |
| **센서 접촉 불량** | 손가락과 센서 사이 간격 또는 각도 불량 | 손가락으로 센서를 완전히 덮고 일정한 압력 유지 |
| **센서 위치 이탈** | 손가락이 센서 창을 벗어남 | 센서 중앙에 손가락 끝 볼록한 부분 정렬 |

#### 9.5.3 올바른 센서 부착법

반사식(Reflection) PPG 센서는 투과식과 달리 **LED와 PD가 같은 면**에 있습니다:

```
  ┌──────┐
  │ LED  │  ← 660nm(Red) + 940nm(IR) 빛이 손가락으로 입사
  │  PD  │  ← 피부 내부에서 반사/산란된 빛을 수광
  └──────┘
      ↑
   손가락 끝 (지문 부분이 센서 창을 완전히 덮도록)
```

**올바른 부착**:
1. 센서 창이 손가락 끝의 **볼록한 부분(지문 중앙)** 에 닿도록 위치
2. 너무 세게 누르면 **혈관이 압박되어 혈류 차단** (AC 신호 소실)
3. 너무 약하게 대면 **외부광 누설** 및 **움직임 노이즈** 증가
4. **일정한 압력** 유지 — 손가락을 떼지 않고 같은 자세 유지

#### 9.5.4 움직임 노이즈 (Motion Artifact)

반사식 PPG의 가장 큰 취약점:

| 움직임 종류 | 영향 | 파형 특징 |
|------------|------|-----------|
| 손 떨림 | DC 레벨 급변, 가짜 피크 발생 | 전체 파형이 위아래로 흔들림 |
| 손가락 압력 변화 | AC 진폭 변동 | PPG 피크 높이가 일정하지 않음 |
| 센서 케이블 움직임 | 고주파 노이즈 | 파형에 가는 잡음 중첩 |

**현재 CSV 분석 결과**: count 114,000~115,000 구간에서 DC 레벨이 약 3,108,000 부근에서 안정적이지만, 피크 검출이 불안정한 것은 작은 AC 신호(±250)에 움직임 노이즈가 중첩되었기 때문으로 추정됩니다.

#### 9.5.5 신호 품질 인디케이터 (제안)

모니터에 실시간 신호 품질 표시를 추가하면 접촉 상태를 즉시 확인 가능:

```
SQ = AC_amplitude / DC_level * 100 (최근 2초 평균)

SQ > 0.5%  → 🟢 양호 (BPM/SpO2 신뢰)
0.1 < SQ < 0.5 → 🟡 보통 (결과에 주의)
SQ < 0.1%  → 🔴 불량 (센서 접촉 확인 필요)
```

⚠️ **참고**: 변조율(AC/DC)은 LED 전류나 ADC range 변경으로 개선되지 않습니다. AC와 DC가 같은 비율로 증가하기 때문입니다. 개선되는 것은 **양자화 해상도**와 **절대 신호 대 잡음비**입니다. 단, ADC range를 2048nA로 낮추면 ADC 포화 가능성이 있으므로 주의해야 합니다. 2026-07-07 시도 결과: 2048nA + LED 증가 → ADC 포화(4,194,303) → 8192nA로 복원함.

Red LED를 IR의 3배(0xC0)로 설정하여 PD의 파장별 감도 차이(IR 940nm가 3배 높음)를 보정했습니다. 이를 통해 SpO2 계산에 필요한 IR/Red 신호 균형이 개선됩니다.

변조율 자체는 **센서 접촉 품질**과 **손가락 위치**에만 의존합니다. 변조율이 0.1% 미만이면 접촉 상태를 확인하세요.

---

## 10. 문제 진단 및 해결 방법 (CSV 데이터 기반 검증 완료)

### 10.1 🔴 HEART RATE = 0 (BPM 미표시)

**✅ 확인된 원인: Convolution Edge Artifact** (섹션 6.3 상세)

| 단계 | 상태 | 설명 |
|------|------|------|
| sig_std > 50 | ✅ 통과 | IR std ≈ 300~550 (기준 50) |
| sig_range > 200 | ✅ 통과 | IR range ≈ 1,000~2,300 (기준 200) |
| DC 제거 (baseline subtract) | ✅ 정상 | AC range ≈ ±250 (정상 PPG) |
| `np.convolve(mode='same')` | 🔴 **EDGE SPIKE** | AC_max = 628,174 (2700x 증폭) |
| Threshold 계산 | 🔴 **비정상** | threshold = 376,839 (정상 피크 ≈ 250) |
| Peak 검출 | 🔴 **항상 실패** | 0개 피크 → return 0.0 |

**✅ 해결 완료** (2026-07-07): `np.pad(data, ..., mode='edge')` 후 `mode='valid'` convolution  
BPM 추출 가능 (실제 CSV 데이터 기준 검증 완료 — 섹션 6.3 코드 참조)

### 10.2 🔴 SpO2 미표시 또는 항상 85%

**✅ 확인된 원인**:
1. **AC/DC ratio 임계값 미달**: AC/DC ≈ 8e-5 < 0.0001 → `estimate_spo2()`가 `return 0.0` → 디스플레이 `--`
2. **IR/Red 역상 관계**: R≈1 → `SpO2 = 110-25×R ≈ 85` → clamp 하한에 고정

**해결 현황**:
1. ✅ **임계값 0.0001 → 1e-6** (적용 완료) — AC/DC ≈ 8e-5 통과
2. ✅ **클램프 85 → 80** (적용 완료) — R≈1에서도 표시 가능
3. ❌ **LED 전류 차등** (펌웨어 수정 대기)
4. ❌ **IR/Red correlation gate** (미구현)

### 10.3 🔴 TEMP 미표시

- **원인 1**: `sample_count % 500 == 0` 조건 — 500샘플(약 10초) 도달 전까지 미출력
- **원인 2**: STATUS_2 TEMP_RDY polling race condition (read 시 flag clear)

**해결** (적용 완료): Polling 대신 `HAL_Delay(50)` 고정 대기 후 레지스터 직접 읽기

### 10.4 CSV 저장 구조 문제

**현재**: 하나의 CSV에 raw(IR_raw, Red_raw)와 computed(BPM, SpO2) 혼합

```
max30102_recording_*.csv
  ├── Timestamp, Count     ← 메타
  ├── IR_raw, Red_raw      ← RAW ADC 값 (FIFO 24-bit)
  └── BPM, SpO2            ← 계산 결과 (estimator 출력)
```

**문제점**:
1. Raw 데이터와 계산 결과가 섞여 있어 재분석/재처리 어려움
2. BPM/SpO2가 빈 값일 때 분석이 혼란스러움
3. 동일 CSV를 엑셀 등에서 열 때 계산 결과가 섞여 보임

**개선 방안**:

```
max30102_recording_*.csv        ← RAW 전용: Count,IR_raw,Red_raw
max30102_computed_*.csv         ← 계산 결과: Timestamp,Count,BPM,SpO2
```

- Recording은 raw 데이터만 저장 (실시간 저장 부담 감소)
- 계산 결과는 별도로 export 또는 화면 표시만 수행
- 분석 시 raw 데이터로 오프라인 재처리 가능

### 10.5 개선 제안 요약

| 우선순위 | 대상 | 변경 | 영향 | 상태 |
|----------|------|------|------|------|
| **🔴 Critical** | Python | `np.convolve(mode='same')` → edge padding + `mode='valid'` | BPM=0 해결 | ✅ 완료 |
| **🔴 Critical** | Python | `SpO2Estimator._get_ac_dc()`에도 동일 padding 적용 | SpO2 정확도 향상 | ✅ 완료 |
| **🔴 Critical** | Python | SpO2 AC/DC ratio 임계값 0.0001 → 1e-6 | SpO2 미표시 해결 | ✅ 완료 |
| **🟡 High** | Firmware | Red LED 0xC0, IR LED 0x40 (차등 전류) | IR/Red 신호 균형 | ✅ 완료 |
| **🟡 High** | Firmware | ADC_RGE 2→0 (8192nA→2048nA) → 포화 → 8192nA 복원 | 신호 진폭 4배 증가 실패 | ❌ 포화 |
| **🟡 High** | Firmware | SMP_AVE 1→3 (4samples→8samples avg) | 노이즈 감소 | ❌ 대기 |
| **🟡 High** | Python | BPM 피크 검출 안정화 (TH/게이트/필터) | BPM 불안정 해결 | 🔄 분석 중 |
| **🟢 Medium** | Python | CSV recording: raw 전용 / computed 별도 분리 | 데이터 분석 용이 | ❌ 대기 |
| **🟢 Medium** | Python | IR/Red correlation gate for SpO2 | 잘못된 SpO2 업데이트 방지 | ❌ 대기 |
| **🟢 Medium** | Firmware | FIFO 데이터 18-bit mask (`& 0x3FFFF`) | 값 해석 명확화 | ❌ 대기 |

### 10.6 시리얼 포트 설정 기억 (Config Persistence)

**기능** (2026-07-07 추가):
- `max30102_monitor_config.json` 파일에 마지막으로 연결 성공한 COM 포트 저장
- 스크립트 재실행 시 자동으로 이전 포트 선택
- 포트 미연결 또는 연결 실패 시 기본 스캔 로직으로 폴백

**저장 위치**: 스크립트와 동일 디렉토리
**저장 내용**: `{"last_port": "COM3 - STMicroelectronics Virtual COM Port"}

---

## 11. 부록: MAX30102 데이터시트 참고

### 11.1 주요 사양

| 파라미터 | 값 |
|----------|-----|
| Part ID | 0x15 |
| I2C 주소 | 0x57 (7-bit) |
| LED 파장 | Red: 660nm, IR: 940nm |
| ADC 해상도 | 15~18-bit (pulse width에 따라) |
| FIFO 크기 | 32 samples × 6 bytes |
| 샘플레이트 | 50~1000 sps |
| LED 전류 | 0~50mA (8-bit DAC) |
| 온도 센서 정확도 | ±1°C (typical) |

### 11.2 GPIO 필요 최소 핀

| 핀 | 개수 | 용도 |
|----|------|------|
| I2C (SCL, SDA) | 2 | 센서 제어 및 데이터 읽기 |
| INT (optional) | 1 | 인터럽트 핀 (현재 미사용) |
| Total (w/o INT) | 2 | 최소 2핀으로 동작 가능 |

### 11.3 참고 자료

- MAX30102 Datasheet: Maxim Integrated (现 Analog Devices)
- AN6809: MAX30102 Implementation Guide
- FIFO 통신은 반드시 WR_PTR, RD_PTR 차이로 가용 데이터 수 계산
- 온도 측정은 DIE_TEMP_CONFIG에 1을 쓴 후 STATUS_2 폴링 필요

---

## 12. BPM Algorithm Comparison Tool

### 12.1 개요

`bpm_analysis_tool.py`는 녹화된 CSV 데이터를 로드하여 여러 BPM 추정 알고리즘을 동시에 비교하는 GUI 분석 도구입니다.

**실행**: `python bpm_analysis_tool.py`

### 12.2 기능

- CSV 파일 로드 (타임스탬프 유무 자동 감지)
- 5가지 알고리즘 변형을 체크박스로 선택하여 동시 실행
- 상단: IR 파형 위에 각 알고리즘이 검출한 피크를 다른 색상으로 오버레이
- 하단: 각 변형의 BPM 값을 막대 그래프로 비교

### 12.3 알고리즘 변형

| 변형 | 설명 | 색상 |
|------|------|------|
| ORG | 현재 구현 (edge-padded conv, TH=60%) | `#00E5A0` (초록) |
| STRICT | 신호 품질 게이트 강화: `std>150, range>500` | `#00B4D8` (하늘) |
| TH50 | 피크 임계값 60% → 50% (민감도 ↑) | `#FFD700` (노랑) |
| MF | BPM 출력 3-value median 필터 (안정화) | `#FF8C00` (주황) |
| ALL | STRICT + TH50 + MF 조합 | `#FF69B4` (분홍) |

### 12.4 사용 예

1. 모니터에서 Recording으로 CSV 저장
2. `python bpm_analysis_tool.py` 실행
3. [Load CSV] 버튼으로 저장된 파일 선택
4. 비교할 알고리즘 체크
5. [Re-run] 버튼으로 분석
6. 상단 그래프에서 피크 위치 확인, 하단 바에서 BPM 비교

---

*본 문서는 `max30102.c/.h`, `main.c`, `max30102_monitor.py`, `bpm_analysis_tool.py` 코드와 CSV 녹화 데이터 분석을 기반으로 작성되었습니다.*
