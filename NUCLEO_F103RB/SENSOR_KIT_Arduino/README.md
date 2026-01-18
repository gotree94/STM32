# NUCLEO STM32F103RB 아두이노 센서 프로젝트

NUCLEO-F103RB 보드를 아두이노 IDE 환경에서 사용하여 다양한 센서 모듈을 테스트하는 프로젝트입니다.

## 📋 목차

- [개발 환경 설정](#개발-환경-설정)
- [NUCLEO-F103RB 보드 정보](#nucleo-f103rb-보드-정보)
- [프로젝트 목록](#프로젝트-목록)

---

## 개발 환경 설정

### 1. Arduino IDE 설치

1. [Arduino IDE 다운로드](https://www.arduino.cc/en/software)에서 최신 버전 설치
2. Arduino IDE 2.x 버전 권장

### 2. STM32 보드 매니저 추가

1. **File → Preferences** (파일 → 환경설정) 메뉴 열기
2. **Additional Boards Manager URLs** 항목에 아래 URL 추가:
   ```
   https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
   ```
3. **OK** 클릭하여 저장

### 3. STM32 보드 패키지 설치

1. **Tools → Board → Boards Manager** (도구 → 보드 → 보드 매니저) 열기
2. 검색창에 `STM32` 입력
3. **STM32 MCU based boards** by STMicroelectronics 설치 (최신 버전)
4. 설치 완료까지 대기 (수 분 소요)

### 4. 보드 설정

**Tools** (도구) 메뉴에서 다음과 같이 설정:

| 항목 | 설정값 |
|------|--------|
| **Board** | Nucleo-64 |
| **Board part number** | Nucleo F103RB |
| **Upload method** | STM32CubeProgrammer (SWD) |
| **Port** | COM포트 선택 (ST-Link Virtual COM Port) |

### 5. STM32CubeProgrammer 설치 (필수)

1. [STM32CubeProgrammer 다운로드](https://www.st.com/en/development-tools/stm32cubeprog.html)
2. 설치 후 환경변수 PATH에 자동 등록됨
3. 설치 확인: 명령 프롬프트에서 `STM32_Programmer_CLI --version`

### 6. 드라이버 설치 확인

- Windows: ST-Link 드라이버 자동 설치됨 (장치 관리자에서 확인)
- Linux: udev rules 설정 필요
  ```bash
  sudo cp ~/.arduino15/packages/STMicroelectronics/tools/STM32Tools/*/drivers/rules/*.rules /etc/udev/rules.d/
  sudo udevadm control --reload-rules
  ```

---

## NUCLEO-F103RB 보드 정보

### 주요 사양

| 항목 | 사양 |
|------|------|
| MCU | STM32F103RBT6 |
| Core | ARM Cortex-M3 |
| Clock | 72 MHz (max) |
| Flash | 128 KB |
| SRAM | 20 KB |
| GPIO | 51개 |
| ADC | 2 x 12-bit (16채널) |
| Timer | 7개 |
| UART | 3개 |
| I2C | 2개 |
| SPI | 2개 |

### 핀 맵 (Arduino 호환)

```
                    NUCLEO-F103RB
                 ┌─────────────────┐
                 │    ST-Link V2   │
                 │   ┌─────────┐   │
                 │   │ USB     │   │
                 │   └─────────┘   │
    ┌────────────┼─────────────────┼────────────┐
    │ CN7        │                 │        CN10│
    │            │                 │            │
    │ PC10  [1]  │                 │  [1]  D10  │ (PA8,  PWM)
    │ PC12  [3]  │                 │  [3]  D9   │ (PC7,  PWM)
    │ VDD   [5]  │                 │  [5]  D8   │ (PA9)
    │ BOOT0 [7]  │                 │  [7]  D7   │ (PA8)
    │ NC    [9]  │                 │  [9]  D6   │ (PB10, PWM)
    │ NC    [11] │                 │  [11] D5   │ (PB4,  PWM)
    │ IOREF [13] │                 │  [13] D4   │ (PB5)
    │ RESET [15] │                 │  [15] D3   │ (PB3,  PWM)
    │ 3V3   [17] │                 │  [17] D2   │ (PA10)
    │ 5V    [19] │                 │  [19] D1   │ (PA2, TX)
    │ GND   [21] │                 │  [21] D0   │ (PA3, RX)
    │ GND   [23] │                 │  [23] GND  │
    │ VIN   [25] │                 │  [25] SCK  │ (PA5)
    │       [27] │                 │  [27] MISO │ (PA6)
    │ A0    [29] │ (PA0)           │  [29] MOSI │ (PA7)
    │ A1    [31] │ (PA1)           │  [31] D10  │ (PB6, CS)
    │ A2    [33] │ (PA4)           │  [33] D11  │ (PA7)
    │ A3    [35] │ (PB0)           │  [35] D12  │ (PA6)
    │ A4    [37] │ (PC1)           │  [37] D13  │ (PA5, LED)
    │ A5    [39] │ (PC0)           │  [39] GND  │
    │            │                 │            │
    └────────────┼─────────────────┼────────────┘
                 │     [USER]      │
                 │     Button      │
                 │      (PC13)     │
                 └─────────────────┘
```

### PWM 지원 핀

| Arduino 핀 | STM32 핀 | Timer |
|-----------|----------|-------|
| D3 | PB3 | TIM2_CH2 |
| D5 | PB4 | TIM3_CH1 |
| D6 | PB10 | TIM2_CH3 |
| D9 | PC7 | TIM3_CH2 |
| D10 | PA8 | TIM1_CH1 |
| D11 | PA7 | TIM3_CH2 |

### ADC 지원 핀

| Arduino 핀 | STM32 핀 | ADC 채널 |
|-----------|----------|----------|
| A0 | PA0 | ADC1_CH0 |
| A1 | PA1 | ADC1_CH1 |
| A2 | PA4 | ADC1_CH4 |
| A3 | PB0 | ADC1_CH8 |
| A4 | PC1 | ADC1_CH11 |
| A5 | PC0 | ADC1_CH10 |

### 내장 LED 및 버튼

| 항목 | 핀 | 설명 |
|------|-----|------|
| User LED | PA5 (D13) | Active High |
| User Button | PC13 | Active Low (풀업) |

---

## 프로젝트 목록

### 1차 배치 (LED 모듈)

| # | 프로젝트 | 설명 | 폴더 |
|---|---------|------|------|
| 01 | [RGB LED 모듈](./01_RGB_LED/) | 3색 LED PWM 제어 | `01_RGB_LED` |
| 02 | [SMD LED 모듈](./02_SMD_LED/) | SMD LED ON/OFF 제어 | `02_SMD_LED` |
| 03 | [2색 LED 모듈](./03_Two_Color_LED/) | 2색 LED 제어 | `03_Two_Color_LED` |
| 04 | [소형 2색 LED 모듈](./04_Small_Two_Color_LED/) | 소형 2색 LED 제어 | `04_Small_Two_Color_LED` |
| 05 | [7색 LED 모듈](./05_Seven_Color_LED/) | 자동 색상 변환 LED | `05_Seven_Color_LED` |

### 2차 배치 (예정)

| # | 프로젝트 | 설명 |
|---|---------|------|
| 06 | 릴레이 모듈 | 릴레이 ON/OFF 제어 |
| 07 | 레이저 모듈 | 레이저 다이오드 제어 |
| 08 | 버튼 스위치 모듈 | 디지털 입력 + 디바운싱 |
| 09 | 터치 센서 모듈 | 정전식 터치 입력 |
| 10 | 충격 센서 모듈 | 진동/충격 감지 |

### 3차 배치 (예정)

| # | 프로젝트 | 설명 |
|---|---------|------|
| 11 | 노크 센서 모듈 | 압전 소자 진동 감지 |
| 12 | 볼 스위치 모듈 | 기울임 감지 |
| 13 | 각도 스위치 모듈 | 특정 각도 감지 |
| 14 | 매직 라이트컵 모듈 | 수은 스위치 + LED |
| 15 | 리드 스위치 모듈 | 자석 감지 |

### 4차 배치 (예정)

| # | 프로젝트 | 설명 |
|---|---------|------|
| 16 | 미니 리드 모듈 | 소형 자석 감지 |
| 17 | 포토 인터럽트 모듈 | 광차단 감지 |
| 18 | 조도 센서 모듈 | CdS 광량 측정 |
| 19 | 불꽃 감지 모듈 | IR 화염 감지 |
| 20 | 심박 센서 모듈 | 광학식 PPG |

### 5차 배치 (예정)

| # | 프로젝트 | 설명 |
|---|---------|------|
| 21 | 고감도 소리센서 모듈 | 아날로그 마이크 |
| 22 | 소형 소리센서 모듈 | 디지털 마이크 |
| 23 | 아날로그 온도센서 모듈 | LM35 온도 측정 |
| 24 | 온도 센서 모듈 | NTC 서미스터 |
| 25 | 디지털 온도센서 모듈 | DS18B20 1-Wire |

### 6차 배치 (예정)

| # | 프로젝트 | 설명 |
|---|---------|------|
| 26 | 리니어 홀센서 모듈 | 아날로그 자기장 |
| 27 | 아날로그 홀센서 모듈 | 자기장 세기 측정 |
| 28 | 홀 마그네틱 모듈 | 디지털 자석 감지 |
| 29 | 트래킹 모듈 | 라인 트레이싱 |
| 30 | 장애물 감지센서 모듈 | IR 반사 감지 |

### 7차 배치 (예정)

| # | 프로젝트 | 설명 |
|---|---------|------|
| 31 | IR 발신 모듈 | 적외선 송신 |
| 32 | IR 수신 모듈 | 적외선 수신 |
| 33 | IR 리모컨 수신 | 리모컨 디코딩 |

---

## 공통 참고사항

### 시리얼 모니터 설정

- **Baud Rate**: 115200
- **Line Ending**: Both NL & CR

### 전원 공급

- USB 전원 (5V) 사용
- 외부 센서는 3.3V 또는 5V 확인 후 연결
- STM32F103은 **3.3V 로직** (5V tolerant 핀 제한적)

### 주의사항

1. GPIO 최대 출력 전류: 25mA (권장 8mA)
2. 총 GPIO 전류 합계: 150mA 이하
3. 5V 센서 연결 시 레벨 시프터 필요 가능
4. 전원 연결 전 회로 확인 필수

## 참고 자료

- [STM32duino GitHub](https://github.com/stm32duino)
- [NUCLEO-F103RB User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
- [STM32F103 Datasheet](https://www.st.com/resource/en/datasheet/stm32f103rb.pdf)
- [Arduino Reference](https://www.arduino.cc/reference/en/)


---

# 릴레이 모듈 (Relay Module)

NUCLEO-F103RB 보드를 이용한 릴레이 모듈 제어 예제입니다.

## 📋 개요

릴레이 모듈은 저전압 신호로 고전압/고전류 회로를 제어할 수 있는 전자식 스위치입니다. 본 예제에서는 GPIO를 이용하여 릴레이를 ON/OFF 제어합니다.

## 🔧 하드웨어 요구사항

- **보드**: NUCLEO-F103RB (STM32F103RBT6)
- **릴레이 모듈**: 1채널 릴레이 모듈 (5V)
- **점퍼 와이어**: 3개

## 📌 핀 연결

| 릴레이 모듈 | NUCLEO-F103RB | 설명 |
|------------|---------------|------|
| VCC | 5V | 전원 공급 |
| GND | GND | 접지 |
| IN (S) | PA8 (D7) | 제어 신호 |

### 회로도

```
NUCLEO-F103RB          릴레이 모듈
    +5V  ─────────────── VCC
    GND  ─────────────── GND
    PA8  ─────────────── IN (S)
```

## 💻 소프트웨어 설정

### Arduino IDE 설정

1. **보드 매니저에서 STM32 보드 패키지 설치**
   - File → Preferences → Additional Board Manager URLs에 추가:
   ```
   https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
   ```
   
2. **보드 선택**
   - Tools → Board → STM32 Boards → Nucleo-64
   - Board Part Number: Nucleo F103RB

3. **포트 선택**
   - Tools → Port → COMx (해당 포트 선택)

## 📝 코드 설명

### 주요 함수

```cpp
void relayOn()      // 릴레이 활성화 (접점 연결)
void relayOff()     // 릴레이 비활성화 (접점 분리)
void relayToggle()  // 릴레이 상태 토글
bool getRelayState() // 현재 릴레이 상태 반환
```

### 동작 원리

- **Active LOW 방식**: 대부분의 릴레이 모듈은 LOW 신호에서 활성화
- `digitalWrite(RELAY_PIN, LOW)` → 릴레이 ON
- `digitalWrite(RELAY_PIN, HIGH)` → 릴레이 OFF

## 🖥️ 시리얼 출력 예시

```
============================================
   릴레이 모듈 테스트 (Relay Module Test)
============================================
보드: NUCLEO-F103RB
릴레이 핀: PA8 (D7)
--------------------------------------------

[릴레이 ON]  - 접점 연결됨 (COM-NO 연결)
[릴레이 OFF] - 접점 분리됨 (COM-NC 연결)

[릴레이 ON]  - 접점 연결됨 (COM-NO 연결)
[릴레이 OFF] - 접점 분리됨 (COM-NC 연결)
```

## 📚 릴레이 단자 설명

| 단자 | 명칭 | 설명 |
|-----|------|------|
| COM | Common | 공통 단자 |
| NO | Normally Open | 평상시 열린 접점 (릴레이 ON 시 연결) |
| NC | Normally Closed | 평상시 닫힌 접점 (릴레이 OFF 시 연결) |

## ⚠️ 주의사항

1. **전원 분리**: AC 전원을 제어할 경우 반드시 전원을 분리한 상태에서 배선
2. **절연**: 고전압 회로와 저전압 제어 회로의 적절한 절연 확보
3. **정격 확인**: 릴레이의 최대 전압/전류 정격 초과 금지
4. **플라이백 다이오드**: 유도성 부하 제어 시 보호 다이오드 사용 권장

## 🔍 트러블슈팅

| 증상 | 원인 | 해결방법 |
|-----|------|---------|
| 릴레이 동작 안함 | 전원 연결 불량 | VCC, GND 연결 확인 |
| 릴레이가 반대로 동작 | Active HIGH 타입 | 코드에서 LOW/HIGH 반전 |
| 딸깍 소리만 나고 부하 동작 안함 | 접점 연결 오류 | COM-NO/NC 배선 확인 |

## 🔗 참고 자료

- [STM32duino 공식 문서](https://github.com/stm32duino/Arduino_Core_STM32)
- [NUCLEO-F103RB 데이터시트](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)

---

# NUCLEO-F103RB 센서 테스트 프로젝트

STM32 NUCLEO-F103RB 보드와 아두이노 환경을 이용한 다양한 센서 모듈 테스트 프로젝트입니다.

## 📋 프로젝트 개요

이 저장소는 NUCLEO-F103RB(STM32F103RBT6) 보드에서 Arduino IDE를 사용하여 다양한 센서 모듈을 테스트하는 예제 코드를 포함합니다.

## 🛠️ 개발 환경

### 하드웨어
- **보드**: NUCLEO-F103RB (STM32F103RBT6)
- **클럭**: 72 MHz (HSE 8MHz + PLL)
- **플래시**: 128 KB
- **SRAM**: 20 KB
- **ADC**: 12-bit

### 소프트웨어
- **IDE**: Arduino IDE 2.x
- **STM32 Core**: stm32duino
- **시리얼 통신**: 115200 bps

## 📦 센서 목록

| # | 센서 | 설명 | 출력 타입 |
|---|------|------|-----------|
| 1 | [불꽃감지 센서](./01_Flame_Sensor/) | IR 파장 기반 화염 감지 | 디지털 + 아날로그 |
| 2 | [리니어 홀 센서](./02_Linear_Hall_Sensor/) | 자기장 세기 선형 측정 | 디지털 + 아날로그 |
| 3 | [터치 센서](./03_Touch_Sensor/) | 정전식 터치 감지 (TTP223) | 디지털 |
| 4 | [디지털 온도 센서](./04_Digital_Temperature_Sensor/) | DS18B20 1-Wire 온도 센서 | 디지털 (1-Wire) |
| 5 | [리드 스위치](./05_Reed_Switch/) | 자기 스위치 (문/창문 감지) | 디지털 |

## 🔧 Arduino IDE 설정

### 1. STM32 보드 매니저 설치

1. Arduino IDE 실행
2. **File** → **Preferences** 클릭
3. **Additional Boards Manager URLs**에 다음 URL 추가:
   ```
   https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
   ```
4. **Tools** → **Board** → **Boards Manager** 클릭
5. "STM32" 검색 후 **STM32 MCU based boards** 설치

### 2. 보드 설정

| 설정 항목 | 값 |
|-----------|-----|
| Board | Nucleo-64 |
| Board part number | Nucleo F103RB |
| Upload method | STM32CubeProgrammer (SWD) |
| Port | COMx (STM32 STLink) |

### 3. 필수 라이브러리 (센서별)

| 센서 | 필요 라이브러리 |
|------|----------------|
| 디지털 온도 센서 | OneWire, DallasTemperature |
| 기타 | 기본 라이브러리만 사용 |

## 📌 공통 핀 배치

### NUCLEO-F103RB Arduino 핀 매핑

```
                    NUCLEO-F103RB
              ┌───────────────────────┐
              │                       │
              │    ┌─────────────┐    │
              │    │   STM32     │    │
              │    │   F103RB    │    │
              │    └─────────────┘    │
              │                       │
   CN6 (좌측) │                       │ CN5 (우측)
   ┌──────────┤                       ├──────────┐
   │ PC10     │                       │     D15  │ PB8 (I2C1_SCL)
   │ PC12     │                       │     D14  │ PB9 (I2C1_SDA)
   │ VDD      │                       │    AVDD  │
   │ BOOT0    │                       │     GND  │
   │ NC       │                       │      D13 │ PA5 (LED, SPI1_SCK)
   │ NC       │                       │      D12 │ PA6 (SPI1_MISO)
   │ PA13     │                       │      D11 │ PA7 (SPI1_MOSI)
   │ PA14     │                       │      D10 │ PB6
   │ PA15     │                       │       D9 │ PC7
   │ GND      │                       │       D8 │ PA9
   │ PB7      │                       │       D7 │ PA8
   │ PC13     │                       │       D6 │ PB10
   │ PC14     │                       │       D5 │ PB4
   │ PC15     │                       │       D4 │ PB5
   │ PH0      │                       │       D3 │ PB3
   │ PH1      │                       │       D2 │ PA10 ← 센서 연결
   │ VBAT     │                       │       D1 │ PA2 (USART2_TX)
   │ PC2      │                       │       D0 │ PA3 (USART2_RX)
   │ PC3      │                       │          │
   └──────────┤                       ├──────────┘
              │                       │
   CN7 (좌측) │                       │ CN8 (우측)
   ┌──────────┤                       ├──────────┐
   │ PC11     │                       │      A0  │ PA0 ← 아날로그 입력
   │ PD2      │                       │      A1  │ PA1
   │ E5V      │                       │      A2  │ PA4
   │ GND      │                       │      A3  │ PB0
   │ IOREF    │                       │      A4  │ PC1
   │ RESET    │                       │      A5  │ PC0
   │ +3V3     │ ← 전원               │          │
   │ +5V      │                       │          │
   │ GND      │ ← 접지               │          │
   │ GND      │                       │          │
   │ VIN      │                       │          │
   └──────────┴───────────────────────┴──────────┘
```

### 센서 공통 연결

| 센서 핀 | NUCLEO 핀 | 설명 |
|---------|-----------|------|
| VCC | 3.3V | 전원 (일부 센서 5V 가능) |
| GND | GND | 접지 |
| DO/SIG | D2 (PA10) | 디지털 출력 |
| AO | A0 (PA0) | 아날로그 출력 |

## 📂 디렉토리 구조

```
NUCLEO-F103RB-Sensors/
├── README.md                              # 메인 README (현재 파일)
├── 01_Flame_Sensor/
│   ├── Flame_Sensor.ino
│   └── README.md
├── 02_Linear_Hall_Sensor/
│   ├── Linear_Hall_Sensor.ino
│   └── README.md
├── 03_Touch_Sensor/
│   ├── Touch_Sensor.ino
│   └── README.md
├── 04_Digital_Temperature_Sensor/
│   ├── Digital_Temperature_Sensor.ino
│   └── README.md
└── 05_Reed_Switch/
    ├── Reed_Switch.ino
    └── README.md
```

## 🚀 빠른 시작

1. **저장소 클론**
   ```bash
   git clone https://github.com/your-username/NUCLEO-F103RB-Sensors.git
   ```

2. **Arduino IDE에서 프로젝트 열기**
   - 원하는 센서 폴더의 `.ino` 파일 열기

3. **보드 및 포트 설정**
   - Tools → Board → Nucleo F103RB
   - Tools → Port → COMx

4. **업로드 및 실행**
   - Upload 버튼 클릭
   - Serial Monitor 열기 (115200 bps)

## 📊 시리얼 모니터 설정

| 설정 | 값 |
|------|-----|
| Baud Rate | 115200 |
| Line Ending | Newline |
| Auto Scroll | On |

## ⚠️ 주의사항

1. **전압 레벨**: STM32F103은 3.3V 로직입니다. 5V 센서 사용 시 주의하세요.
2. **풀업/풀다운**: 일부 센서는 외부 저항이 필요할 수 있습니다.
3. **전류 제한**: GPIO 핀당 최대 전류는 25mA입니다.
4. **ADC 해상도**: 12-bit (0~4095) 범위입니다.

## 🔗 참고 자료

- [STM32F103RB Datasheet](https://www.st.com/resource/en/datasheet/stm32f103rb.pdf)
- [NUCLEO-F103RB User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
- [STM32duino GitHub](https://github.com/stm32duino)
- [Arduino Reference](https://www.arduino.cc/reference/en/)

---

# NUCLEO-F103RB 센서 모듈 테스트 프로젝트

Arduino IDE 환경에서 NUCLEO-F103RB 보드를 이용한 다양한 센서 모듈 테스트 프로젝트 모음입니다.

## 📋 프로젝트 개요

이 저장소는 STM32F103RB 기반 NUCLEO 보드를 Arduino IDE 환경에서 사용하여 다양한 센서 모듈을 테스트하는 예제 코드를 포함합니다.

## 🎯 대상

- 임베디드 시스템 입문자
- STM32 Arduino 개발 환경 학습자
- 센서 모듈 활용 프로젝트 개발자

## 🔧 개발 환경 설정

### 필수 소프트웨어

1. **Arduino IDE** (1.8.x 또는 2.x)
2. **STM32duino 보드 패키지**

### STM32duino 설치 방법

1. Arduino IDE → 파일 → 환경설정
2. 추가적인 보드 매니저 URLs에 다음 추가:
   ```
   https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
   ```
3. 도구 → 보드 → 보드 매니저
4. "STM32" 검색 → **STM32 MCU based boards** 설치

### 보드 설정

| 설정 항목 | 값 |
|-----------|-----|
| 보드 | Nucleo-64 |
| Board part number | Nucleo F103RB |
| Upload method | STM32CubeProgrammer (SWD) |

## 📁 프로젝트 구조

```
sensors/
├── README.md                    # 이 파일
├── 01_MiniReedSwitch/          # 미니 리드 스위치 모듈
│   ├── MiniReedSwitch.ino
│   └── README.md
├── 02_HeartRateSensor/         # 심박 센서 모듈
│   ├── HeartRateSensor.ino
│   └── README.md
├── 03_LaserModule/             # 레이저 모듈
│   ├── LaserModule.ino
│   └── README.md
├── 04_ButtonSwitch/            # 버튼 스위치 모듈
│   ├── ButtonSwitch.ino
│   └── README.md
└── 05_ShockSensor/             # 충격 센서 모듈
    ├── ShockSensor.ino
    └── README.md
```

## 📦 센서 모듈 목록

| # | 모듈 | 설명 | 인터페이스 |
|---|------|------|-----------|
| 01 | 미니 리드 스위치 | 자기장 감지 스위치 | Digital |
| 02 | 심박 센서 | PPG 방식 심박 측정 | Analog |
| 03 | 레이저 모듈 | 650nm 적색 레이저 | Digital/PWM |
| 04 | 버튼 스위치 | 푸시 버튼 입력 | Digital |
| 05 | 충격 센서 | 진동/충격 감지 | Digital |

## 🔌 공통 핀 배치

### NUCLEO-F103RB 핀맵

```
                    NUCLEO-F103RB
              ┌─────────────────────┐
              │                     │
    Arduino   │  STM32    Function  │
    ─────────────────────────────────
       A0     │   PA0     ADC/GPIO  │ ← 센서 연결
       A1     │   PA1     ADC/GPIO  │
       A2     │   PA4     ADC/GPIO  │
       A3     │   PB0     ADC/GPIO  │
       A4     │   PC1     ADC/I2C   │
       A5     │   PC0     ADC/I2C   │
              │                     │
       D13    │   PA5     LED/SPI   │ ← 내장 LED
              │                     │
       3.3V   │   3.3V    Power     │
       5V     │   5V      Power     │
       GND    │   GND     Ground    │
              └─────────────────────┘
```

## 🚀 빠른 시작

1. **저장소 클론**
   ```bash
   git clone https://github.com/your-username/nucleo-sensor-test.git
   ```

2. **Arduino IDE에서 프로젝트 열기**
   - 원하는 센서 폴더의 `.ino` 파일 열기

3. **보드 연결 및 업로드**
   - USB 케이블로 NUCLEO 보드 연결
   - 올바른 포트 선택
   - 업로드 버튼 클릭

4. **시리얼 모니터 확인**
   - 도구 → 시리얼 모니터
   - 보드레이트: 115200

## 📊 각 센서별 특징

### 01. 미니 리드 스위치
- **용도**: 도어/창문 개폐 감지
- **동작**: 자석 접근 시 스위치 ON
- **출력**: 디지털 (HIGH/LOW)

### 02. 심박 센서
- **용도**: 심박수(BPM) 측정
- **동작**: PPG 광학 방식
- **출력**: 아날로그 (0-4095)
- **특징**: 이동평균 필터, BPM 계산

### 03. 레이저 모듈
- **용도**: 레이저 포인터, 정렬
- **동작**: 디지털 ON/OFF, PWM 밝기 조절
- **모드**: ON, OFF, Blink, SOS, Fade
- **⚠️ 주의**: 눈 보호 필수

### 04. 버튼 스위치
- **용도**: 사용자 입력
- **동작**: 디바운싱, 멀티 이벤트
- **이벤트**: 싱글클릭, 더블클릭, 롱프레스

### 05. 충격 센서
- **용도**: 진동/충격 감지
- **동작**: 인터럽트 + 폴링 하이브리드
- **특징**: 강도 추정, 경보 모드

## ⚠️ 주의사항

1. **전원**: 센서별 권장 전압 확인 (3.3V 또는 5V)
2. **레이저**: 절대 눈에 직접 비추지 말 것
3. **심박 센서**: 손가락을 너무 세게 누르지 말 것
4. **정전기**: 센서 취급 시 정전기 주의

## 📚 참고 자료

- [STM32duino GitHub](https://github.com/stm32duino)
- [NUCLEO-F103RB 데이터시트](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
- [Arduino Reference](https://www.arduino.cc/reference/en/)

---




