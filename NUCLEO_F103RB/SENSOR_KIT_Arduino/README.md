# NUCLEO STM32F103RB 아두이노 센서 프로젝트

NUCLEO-F103RB 보드를 아두이노 IDE 환경에서 사용하여 다양한 센서 모듈을 테스트하는 프로젝트입니다.

<img width="797" height="515" alt="016" src="https://github.com/user-attachments/assets/c1b7755b-5fee-49ca-80ca-d538b5927ad6" />

## 📋 목차

- [개발 환경 설정](#개발-환경-설정)
- [NUCLEO-F103RB 보드 정보](#nucleo-f103rb-보드-정보)
- [프로젝트 목록](#프로젝트-목록)

---

## 개발 환경 설정

### 1. Arduino IDE 설치

1. [Arduino IDE 다운로드](https://www.arduino.cc/en/software)에서 최신 버전 설치
2. Arduino IDE 2.x 버전 권장

<img width="581" height="360" alt="002" src="https://github.com/user-attachments/assets/1b187be9-9a95-4a1b-90b2-d95fbcad96b4" /> <br>
<img width="581" height="360" alt="003" src="https://github.com/user-attachments/assets/f8ca3b50-5814-407e-b087-bde7dc50976e" /> <br>
<img width="581" height="360" alt="004" src="https://github.com/user-attachments/assets/a3c32fc9-463e-4231-9c4c-4b112f6fc08e" /> <br>
<img width="581" height="360" alt="005" src="https://github.com/user-attachments/assets/ad32dd41-d429-4f0d-b207-ce7805889d01" /> <br>
<img width="581" height="360" alt="006" src="https://github.com/user-attachments/assets/eacff9b5-8fd8-431a-b17c-0cf0b7c6b7f7" /> <br>
<img width="581" height="360" alt="007" src="https://github.com/user-attachments/assets/767fd1e4-dbfa-4ad6-812b-3befcb153f8c" /> <br>

<img width="1272" height="721" alt="009" src="https://github.com/user-attachments/assets/4ea9c482-a8e1-447e-9367-e7aca98b24a1" /> <br>

### 2. STM32 보드 매니저 추가

1. **File → Preferences** (파일 → 환경설정) 메뉴 열기
2. **Additional Boards Manager URLs** 항목에 아래 URL 추가:
   ```
   https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
   ```
3. **OK** 클릭하여 저장

<img width="1272" height="721" alt="010" src="https://github.com/user-attachments/assets/4a15da44-4528-4676-be03-f4458f658d40" />


### 3. STM32 보드 패키지 설치

1. **Tools → Board → Boards Manager** (도구 → 보드 → 보드 매니저) 열기
2. 검색창에 `STM32` 입력
3. **STM32 MCU based boards** by STMicroelectronics 설치 (최신 버전)
4. 설치 완료까지 대기 (수 분 소요)

<img width="641" height="489" alt="011" src="https://github.com/user-attachments/assets/57ceb5e0-130d-4f9b-ba3f-27398e3c9a95" />
<br>
<img width="1272" height="721" alt="012" src="https://github.com/user-attachments/assets/3ac8b772-7bdc-4aed-9e7e-98247e3f6596" />
<br>
<img width="1272" height="721" alt="013" src="https://github.com/user-attachments/assets/e6f73dff-17c2-44b1-b8fa-c51617099798" />
<br>

### 4. 보드 설정

**Tools** (도구) 메뉴에서 다음과 같이 설정:

| 항목 | 설정값 |
|------|--------|
| **Board** | Nucleo-64 |
| **Board part number** | Nucleo F103RB |
| **Upload method** | STM32CubeProgrammer (SWD) |
| **Port** | COM포트 선택 (ST-Link Virtual COM Port) |

<img width="908" height="581" alt="014" src="https://github.com/user-attachments/assets/e004b61c-6cca-4e68-badf-172beba2b854" />
<br>
<img width="708" height="731" alt="015" src="https://github.com/user-attachments/assets/801133a5-ee28-4e47-9280-c3324c89bfea" />

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

```arduino
/**
 ******************************************************************************
 * @file    LD2_Blink.ino
 * @brief   NUCLEO-F103RB LD2 LED Blink Test
 * @date    2025-01-18
 ******************************************************************************
 * @details
 * NUCLEO-F103RB 보드의 내장 LED(LD2)를 켜고 끄는 기본 테스트
 * 
 * LD2 LED: PA5 (D13)
 * - Active HIGH (HIGH = ON, LOW = OFF)
 ******************************************************************************
 */

#define LD2_PIN     PA5     // 내장 LED (D13과 동일)

void setup() {
    // 시리얼 초기화
    Serial.begin(115200);
    while (!Serial) { delay(10); }
    
    Serial.println();
    Serial.println("NUCLEO-F103RB LD2 Blink Test");
    Serial.println("LED Pin: PA5 (LD2)");
    Serial.println();
    
    // LED 핀 출력 모드 설정
    pinMode(LD2_PIN, OUTPUT);
    
    // 초기 상태: OFF
    digitalWrite(LD2_PIN, LOW);
}

void loop() {
    // LED ON
    digitalWrite(LD2_PIN, HIGH);
    Serial.println("LD2: ON");
    delay(1000);
    
    // LED OFF
    digitalWrite(LD2_PIN, LOW);
    Serial.println("LD2: OFF");
    delay(1000);
}

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

| # | 프로젝트 | 설명 | 폴더 |
|---|---------|------|------|
| 01 | [RGB LED 모듈](./01_RGB_LED/) | 3색 LED PWM 제어 | `01_RGB_LED` |
| 02 | [SMD LED 모듈](./02_SMD_LED/) | SMD LED ON/OFF 제어 | `02_SMD_LED` |
| 03 | [2색 LED 모듈](./03_Two_Color_LED/) | 2색 LED 제어 | `03_Two_Color_LED` |
| 04 | [소형 2색 LED 모듈](./04_Small_Two_Color_LED/) | 소형 2색 LED 제어 | `04_Small_Two_Color_LED` |
| 05 | [7색 LED 모듈](./05_Seven_Color_LED/) | 자동 색상 변환 LED | `05_Seven_Color_LED` |
| 06 | [릴레이 모듈](./06.relay_module/) | 릴레이 ON/OFF 제어 | `06.relay_module` |
| 07 | [레이저 모듈](./07.sound_sensor_high/) | 레이저 다이오드 제어 | `07.sound_sensor_high` | 
| 08 | [버튼 스위치 모듈](./08.sound_sensor_small/) | 디지털 입력 + 디바운싱 | `08.sound_sensor_small` |
| 09 | [터치 센서 모듈](./09.tracking_module/) | 정전식 터치 입력 | `09.tracking_module` |
| 10 | [충격 센서 모듈](./10.obstacle_sensor/) | 진동/충격 감지 | `10.obstacle_sensor` |
| 11 | [노크 센서 모듈](./11_Flame_Sensor/) | 압전 소자 진동 감지 |`11_Flame_Sensor` | 
| 12 | [볼 스위치 모듈](./ /) | 기울임 감지 | `12_Linear_Hall_Sensor` | 
| 13 | [각도 스위치 모듈](./ /) | 특정 각도 감지 | `13_Touch_Sensor` |
| 14 | [매직 라이트컵 모듈](./ /) | 수은 스위치 + LED | `14_Digital_Temperature_Sensor` |
| 15 | [리드 스위치 모듈](./ /) | 자석 감지 | `15_Reed_Switch` | 
| 16 | [미니 리드 모듈](./ /) | 소형 자석 감지 | `16_MiniReedSwitch` |
| 17 | [포토 인터럽트 모듈](./ /) | 광차단 감지 | `17_HeartRateSensor` |
| 18 | [조도 센서 모듈](./ /) | CdS 광량 측정 | `18_LaserModule` |
| 19 | [불꽃 감지 모듈](./ /) | IR 화염 감지 | `19_ButtonSwitch` |
| 20 | [심박 센서 모듈](./ /) | 광학식 PPG | `20_ShockSensor` |
| 21 | [고감도 소리센서 모듈](./ /) | 아날로그 마이크 | `21_MagicLightCup` |
| 22 | [소형 소리센서 모듈](./ /) | 디지털 마이크 |  `22_TiltSwitch` | 
| 23 | [아날로그 온도센서 모듈](./ /) | LM35 온도 측정 | `23_BallSwitch` | 
| 24 | [온도 센서 모듈](./ /) | NTC 서미스터 |  `24_LightSensor` |
| 25 | [디지털 온도센서 모듈](./ /) | DS18B20 1-Wire | `25_AnalogHallSensor` |
| 26 | [리니어 홀센서 모듈](./ /) | 아날로그 자기장 |`26_Hall_Magnetic_Module` |
| 27 | [아날로그 홀센서 모듈](./ /) | 자기장 세기 측정 | `27_Temperature_Sensor_Module` |
| 28 | [홀 마그네틱 모듈](./ /) | 디지털 자석 감지 | `28_Analog_Light_Sensor_Module` | 
| 29 | [트래킹 모듈](./ /) | 라인 트레이싱 | `29_Knock_Sensor_Module` | 
| 30 | [장애물 감지센서 모듈](./ /) | IR 반사 감지 |`30_Photo_Interrupt_Module` |
| 31 | [IR 발신 모듈](./ /) | 적외선 송신 | `31_IR_Transmitter` |
| 32 | [IR 수신 모듈](./ /) | 적외선 수신 | `32_IR_Receiver` |
| 33 | [IR 리모컨 수신](./ /) | 리모컨 디코딩 | `33_IR_Remote_Decoder` | 
| 34 | [조이스틱](./ /) | 조이스틱 | `34_Joystick` |
| 37 | [수동부저](./ /) | 수동부저 | `37_Passive_Buzzer` |
| 38 | [로터리 스위치](./ /) | 로터리 스위치 | `38_Rotary_Encoder` |
| 39 | [피에조](./ /) | 피에조 | `39_Active_Buzzer` |

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

# NUCLEO-F103RB 센서 모듈 테스트 프로젝트 (세트 2)

Arduino IDE 환경에서 NUCLEO-F103RB 보드를 이용한 다양한 센서 모듈 테스트 프로젝트 모음입니다.

## 📋 프로젝트 개요

이 저장소는 STM32F103RB 기반 NUCLEO 보드를 Arduino IDE 환경에서 사용하여 기울기, 움직임, 조도, 자기장 감지 센서 모듈을 테스트하는 예제 코드를 포함합니다.

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
├── README.md                       # 이 파일
├── 01_MagicLightCup/              # 매직 라이트컵 모듈
│   ├── MagicLightCup.ino
│   └── README.md
├── 02_TiltSwitch/                 # 각도 스위치 모듈
│   ├── TiltSwitch.ino
│   └── README.md
├── 03_BallSwitch/                 # 볼 스위치 모듈
│   ├── BallSwitch.ino
│   └── README.md
├── 04_LightSensor/                # 조도 센서 모듈
│   ├── LightSensor.ino
│   └── README.md
└── 05_AnalogHallSensor/           # 아날로그 홀센서 모듈
    ├── AnalogHallSensor.ino
    └── README.md
```

## 📦 센서 모듈 목록

| # | 모듈 | 설명 | 인터페이스 |
|---|------|------|-----------|
| 01 | 매직 라이트컵 | 수은 스위치 + LED (기울기 감지) | Digital + PWM |
| 02 | 각도 스위치 | 기울기 감지 (틸트 스위치) | Digital |
| 03 | 볼 스위치 | 움직임/진동 감지 | Digital |
| 04 | 조도 센서 | 빛의 밝기 측정 (LDR) | Analog |
| 05 | 아날로그 홀센서 | 자기장 세기/극성 감지 | Analog |

## 🔌 공통 핀 배치

### NUCLEO-F103RB 핀맵

```
                    NUCLEO-F103RB
              ┌─────────────────────┐
              │                     │
    Arduino   │  STM32    Function  │
    ─────────────────────────────────
       A0     │   PA0     ADC/GPIO  │ ← 센서 연결
       A1     │   PA1     ADC/GPIO  │ ← LED 제어 (PWM)
       A2     │   PA4     ADC/GPIO  │
       A3     │   PB0     ADC/GPIO  │
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
   git clone https://github.com/your-username/nucleo-sensor-test-set2.git
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

### 01. 매직 라이트컵 (KY-027)
- **용도**: 기울기에 따른 LED 효과
- **구성**: 수은 스위치 + LED 일체형
- **모드**: 기본 ON/OFF, 페이드, 물결 효과, 밝기 조절
- **특징**: 물이 흐르는 듯한 조명 효과 구현 가능

### 02. 각도 스위치 (KY-020)
- **용도**: 기울기 감지
- **동작**: 일정 각도 이상 기울어지면 감지
- **특징**: 통계 기능, 경보 모드, 안정성 감지
- **장점**: 수은 미사용으로 친환경

### 03. 볼 스위치 (KY-017)
- **용도**: 움직임/진동 감지
- **동작**: 매우 민감한 움직임 감지
- **모드**: 기본, 움직임 강도, 흔들기, 경보
- **특징**: 흔들기 감지, RPM 측정 가능

### 04. 조도 센서 (KY-018)
- **용도**: 주변 밝기 측정
- **동작**: LDR (광저항) 방식
- **모드**: 측정, 모니터링, 자동 LED, 임계값 알림
- **특징**: 이동평균 필터, 막대 그래프 표시

### 05. 아날로그 홀센서 (KY-035)
- **용도**: 자기장 감지
- **동작**: 홀 효과로 자기장 세기/극성 측정
- **모드**: 측정, 모니터링, 자석 감지, 가우스 미터
- **특징**: N/S 극성 구분, RPM 측정, 영점 보정

## 📊 센서 비교표

| 센서 | 감지 대상 | 출력 타입 | 민감도 | 응용 |
|------|----------|----------|--------|------|
| 매직 라이트컵 | 기울기 | Digital+PWM | 중 | 조명 효과 |
| 각도 스위치 | 기울기 | Digital | 낮음 | 기울기 감지 |
| 볼 스위치 | 움직임 | Digital | 높음 | 진동/흔들기 |
| 조도 센서 | 빛 | Analog | - | 밝기 측정 |
| 홀센서 | 자기장 | Analog | - | 자석 감지 |

## ⚠️ 주의사항

1. **전원**: 센서별 권장 전압 확인 (3.3V 또는 5V)
2. **매직 라이트컵**: 수은 스위치 포함 - 취급 주의
3. **볼 스위치**: 매우 민감 - 디바운싱 필수
4. **조도 센서**: 비선형 특성 - 정밀 측정에는 Lux 센서 권장
5. **홀센서**: 영점 보정 필요 - 주변 자기장 영향 주의

## 📚 참고 자료

- [STM32duino GitHub](https://github.com/stm32duino)
- [NUCLEO-F103RB 데이터시트](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
- [Arduino Reference](https://www.arduino.cc/reference/en/)

---

# STM32F103 Sensor Modules - Arduino Environment

NUCLEO-F103RB 보드에서 다양한 센서 모듈을 아두이노 환경으로 테스트하는 프로젝트 모음입니다.

## 📋 프로젝트 개요

이 저장소는 STM32F103RB 기반 NUCLEO 보드에서 기초 센서 모듈들을 테스트하고 학습할 수 있는 예제 코드를 제공합니다. 모든 코드는 Arduino IDE의 STM32duino 환경에서 동작합니다.

## 🔧 하드웨어 요구사항

| 항목 | 사양 |
|------|------|
| 개발 보드 | NUCLEO-F103RB |
| MCU | STM32F103RBT6 (ARM Cortex-M3, 72MHz) |
| 개발 환경 | Arduino IDE 1.8.x / 2.x |
| 보드 패키지 | STM32duino |

## 📁 프로젝트 구조

```
stm32f103_sensors/
│
├── README.md                              # 이 파일
│
├── 01_Hall_Magnetic_Module/               # 홀 마그네틱 센서
│   ├── Hall_Magnetic_Module.ino
│   └── README.md
│
├── 02_Temperature_Sensor_Module/          # 온도 센서
│   ├── Temperature_Sensor_Module.ino
│   └── README.md
│
├── 03_Analog_Light_Sensor_Module/         # 아날로그 조도 센서
│   ├── Analog_Light_Sensor_Module.ino
│   └── README.md
│
├── 04_Knock_Sensor_Module/                # 노크(진동) 센서
│   ├── Knock_Sensor_Module.ino
│   └── README.md
│
└── 05_Photo_Interrupt_Module/             # 포토 인터럽트 센서
    ├── Photo_Interrupt_Module.ino
    └── README.md
```

## 📦 센서 모듈 목록

| # | 센서 모듈 | 주요 기능 | 출력 타입 |
|---|----------|----------|----------|
| 01 | 홀 마그네틱 | 자기장/자석 감지 | Digital + Analog |
| 02 | 온도 센서 | 온도 측정 (NTC/LM35) | Digital + Analog |
| 03 | 아날로그 조도 센서 | 광량/밝기 측정 | Digital + Analog |
| 04 | 노크 센서 | 진동/충격 감지 | Digital + Analog |
| 05 | 포토 인터럽트 | 물체 차단 감지, RPM | Digital + Analog |

## 🚀 시작하기

### 1. Arduino IDE 설정

1. Arduino IDE를 설치합니다.
2. File → Preferences → Additional Boards Manager URLs에 추가:
   ```
   https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
   ```
3. Tools → Board → Boards Manager에서 "STM32" 검색 후 설치

### 2. 보드 설정

Tools 메뉴에서 다음을 설정합니다:
- **Board**: STM32 Boards → Nucleo-64
- **Board part number**: Nucleo F103RB
- **Upload method**: STM32CubeProgrammer (SWD) 또는 Mass Storage
- **Port**: 해당 COM 포트

### 3. 코드 업로드

1. 원하는 센서 폴더의 .ino 파일을 엽니다.
2. 회로를 연결합니다 (각 README.md 참조).
3. 업로드 버튼을 클릭합니다.
4. 시리얼 모니터를 열고 (115200 baud) 결과를 확인합니다.

## 📌 공통 핀 배치

모든 센서 모듈에서 사용하는 공통 핀:

| 기능 | NUCLEO 핀 | STM32 핀 | 설명 |
|------|----------|----------|------|
| Digital Out | D2 | PA10 | 센서 디지털 출력 |
| Analog Out | A0 | PA0 | 센서 아날로그 출력 |
| LED | LED_BUILTIN | PA5 | 내장 LED |
| Serial TX | - | PA2 | USB 시리얼 |
| Serial RX | - | PA3 | USB 시리얼 |

## 🔌 NUCLEO-F103RB 핀맵

```
                    NUCLEO-F103RB
               ┌─────────────────────┐
               │      [USB]          │
               │                     │
       PC13 ───┤                     ├─── PB8
         PC14 ─┤                     ├─── PB9
         PC15 ─┤                     ├─── AVDD
        RESET ─┤                     ├─── GND
          PC0 ─┤ A0                  ├─── PB5
          PC1 ─┤ A1                  ├─── PB4
          PC2 ─┤ A2                  ├─── PB3
          PC3 ─┤ A3                  ├─── PA10 (D2)
          PA0 ─┤ A4                  ├─── PA2
          PA1 ─┤ A5                  ├─── PA3
          PA4 ─┤                     ├─── ...
          PB0 ─┤                     ├─── 
               │       [LED]         │
               │        PA5          │
               └─────────────────────┘
```

## ⚙️ 각 센서 모듈 개요

### 01. 홀 마그네틱 모듈
자석이 접근하면 홀 효과에 의해 전압이 변화합니다.
- 응용: 도어 센서, 회전 감지, 위치 센서

### 02. 온도 센서 모듈
NTC 서미스터 또는 LM35로 주변 온도를 측정합니다.
- 응용: 온도 모니터링, 온도 조절 시스템

### 03. 아날로그 조도 센서 모듈
LDR(광저항)로 주변 밝기를 측정합니다.
- 응용: 자동 조명, 밝기 제어

### 04. 노크 센서 모듈
압전 소자로 진동/충격을 감지합니다.
- 응용: 비밀 노크, 진동 알람

### 05. 포토 인터럽트 모듈
적외선 차단으로 물체를 감지합니다.
- 응용: RPM 측정, 카운팅, 엔코더

## 📚 추가 자료

- [STM32duino Wiki](https://github.com/stm32duino/wiki/wiki)
- [NUCLEO-F103RB 공식 페이지](https://www.st.com/en/evaluation-tools/nucleo-f103rb.html)
- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/cd00171190.pdf)
- [Arduino Reference](https://www.arduino.cc/reference/en/)

## 🐛 문제 해결

### 업로드 실패 시
1. ST-Link 드라이버가 설치되어 있는지 확인
2. STM32CubeProgrammer 설치 확인
3. USB 케이블이 데이터 전송용인지 확인
4. 다른 USB 포트 시도

### 시리얼 출력이 안 보일 때
1. 올바른 COM 포트 선택 확인
2. Baud rate가 115200인지 확인
3. 시리얼 모니터 재시작

### 센서 값이 이상할 때
1. 배선 연결 확인 (VCC, GND 극성)
2. 센서 모듈의 전원 전압 확인 (3.3V/5V)
3. 풀업/풀다운 저항 필요 여부 확인

---

# STM32 IR Sensor Projects (Arduino)

NUCLEO-F103RB 보드를 이용한 IR(적외선) 센서 Arduino 프로젝트 모음

## 프로젝트 목록

| 프로젝트 | 설명 | 난이도 |
|----------|------|--------|
| [01_IR_Transmitter](./01_IR_Transmitter) | IR LED를 이용한 38kHz NEC 신호 송신 | ⭐⭐ |
| [02_IR_Receiver](./02_IR_Receiver) | IR 수신 모듈 기본 테스트 (펄스 측정) | ⭐ |
| [03_IR_Remote_Decoder](./03_IR_Remote_Decoder) | IR 리모컨 NEC 프로토콜 디코딩 | ⭐⭐⭐ |

## 하드웨어 요구사항

### 필수
- NUCLEO-F103RB 개발보드
- USB 케이블 (Mini-B)
- 브레드보드 및 점퍼 와이어

### IR 송신용
- IR LED (940nm)
- 100Ω 저항
- (선택) 2N2222 트랜지스터 + 1kΩ 저항

### IR 수신용
- IR 수신 모듈 (TSOP1838, VS1838B 등)
- IR 리모컨 (NEC 프로토콜)

## 전체 핀 배치

```
           NUCLEO-F103RB
          ┌─────────────┐
          │             │
[IR LED]──│ PA8 (D7)    │ ← 38kHz PWM 출력
          │             │
[IR RX] ──│ PA0 (A0)    │ ← IR 수신 입력
          │             │
[LED]   ──│ PA5 (D13)   │ ← 내장 LED
          │             │
[BTN]   ──│ PC13        │ ← 내장 버튼
          │             │
[USB]   ──│ PA2/PA3     │ ← UART2 (Virtual COM)
          │             │
          │     GND     │─── GND
          │    3.3V     │─── VCC
          └─────────────┘
```

## Arduino IDE 설정

### 1. STM32 보드 패키지 설치

File → Preferences → Additional Boards Manager URLs에 추가:
```
https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
```

Tools → Board → Boards Manager → "STM32" 검색 후 설치

### 2. 보드 선택

- Board: **Nucleo-64**
- Board part number: **Nucleo F103RB**
- Upload method: **STM32CubeProgrammer (SWD)**

### 3. 시리얼 모니터 설정

- Baud Rate: **115200**
- Line ending: **Both NL & CR**

## 빠른 시작

```bash
# 1. 저장소 클론
git clone https://github.com/your-username/stm32-ir-arduino.git

# 2. Arduino IDE에서 .ino 파일 열기
# 3. 보드 및 포트 선택
# 4. 업로드 (Ctrl+U)
# 5. 시리얼 모니터 열기 (Ctrl+Shift+M)
```

## NEC 프로토콜 요약

```
프레임: [Leader 9ms][Space 4.5ms][32-bit Data][Stop]

Data: [Address 8-bit][~Address 8-bit][Command 8-bit][~Command 8-bit]

Bit 0: 560us pulse + 560us space
Bit 1: 560us pulse + 1690us space

Repeat: 9ms pulse + 2.25ms space + Stop bit
```

## 참고 자료

- [NEC Protocol](https://www.sbprojects.net/knowledge/ir/nec.php)
- [STM32duino Wiki](https://github.com/stm32duino/wiki/wiki)
- [NUCLEO-F103RB User Manual](https://www.st.com/resource/en/user_manual/um1724.pdf)


---

# STM32F103 Arduino 모듈 프로젝트

NUCLEO-F103RB 보드와 Arduino 프레임워크를 사용한 다양한 센서/모듈 제어 프로젝트입니다.

## 📋 개요

| 모듈 | 설명 | 난이도 |
|------|------|--------|
| [조이스틱](#1-조이스틱-모듈) | 2축 아날로그 + 버튼 입력 | ⭐ |
| [릴레이](#2-릴레이-모듈) | 고전압 회로 스위칭 | ⭐ |
| [능동 부저](#3-능동-부저-모듈) | ON/OFF 사운드 출력 | ⭐ |
| [수동 부저](#4-수동-부저-모듈) | 주파수 기반 멜로디 재생 | ⭐⭐ |
| [로터리 엔코더](#5-로터리-엔코더-모듈) | 회전 위치 감지 | ⭐⭐ |

## 🛠️ 개발 환경

- **보드**: NUCLEO-F103RB (STM32F103RBT6)
- **프레임워크**: Arduino
- **IDE**: Arduino IDE 2.x / PlatformIO
- **시리얼 통신**: 115200 baud

## 📌 핀 배치 요약

```
NUCLEO-F103RB 핀 배치
┌─────────────────────────────────────┐
│  모듈           │ 핀              │
├─────────────────┼─────────────────┤
│  조이스틱 VRx   │ A0 (PA0)        │
│  조이스틱 VRy   │ A1 (PA1)        │
│  조이스틱 SW    │ D2 (PA10)       │
│  릴레이 IN      │ D3 (PB3)        │
│  능동 부저      │ D4 (PB5)        │
│  수동 부저      │ D5 (PB4)        │
│  엔코더 CLK     │ D6 (PB10)       │
│  엔코더 DT      │ D7 (PA8)        │
│  엔코더 SW      │ D8 (PA9)        │
│  User Button    │ PC13            │
│  User LED       │ PA5             │
└─────────────────┴─────────────────┘
```

---

## 1. 조이스틱 모듈

### 회로 연결

```
조이스틱 모듈          NUCLEO-F103RB
┌──────────┐          ┌──────────┐
│ VCC      │──────────│ 3.3V/5V  │
│ GND      │──────────│ GND      │
│ VRx      │──────────│ A0       │
│ VRy      │──────────│ A1       │
│ SW       │──────────│ D2       │
└──────────┘          └──────────┘
```

### 기능
- X/Y축 아날로그 값 읽기 (0-1023)
- 8방향 판별 (상하좌우 + 대각선)
- 데드존 설정으로 중앙 감지
- 버튼 디바운싱 처리

### 시리얼 출력 예시
```
X: 512	Y: 512	Direction: CENTER	Button: Released
X: 1023	Y: 512	Direction: RIGHT	Button: Released
X: 0	Y: 0	Direction: LEFT-UP	Button: PRESSED
```

---

## 2. 릴레이 모듈

### 회로 연결

```
릴레이 모듈            NUCLEO-F103RB
┌──────────┐          ┌──────────┐
│ VCC      │──────────│ 5V       │
│ GND      │──────────│ GND      │
│ IN       │──────────│ D3       │
└──────────┘          └──────────┘

※ 릴레이 코일은 5V 필요
※ LOW 트리거 타입 기준
```

### 시리얼 명령어

| 명령 | 기능 |
|------|------|
| `1` | 릴레이 ON |
| `0` | 릴레이 OFF |
| `t` | 토글 |
| `T` | 타이머 모드 (5초 후 자동 OFF) |
| `b` | 블링크 (3회) |

### 기능
- ON/OFF 제어
- 토글 기능
- 타이머 모드
- 블링크 패턴

---

## 3. 능동 부저 모듈

### 회로 연결

```
능동 부저 모듈         NUCLEO-F103RB
┌──────────┐          ┌──────────┐
│ VCC      │──────────│ 3.3V/5V  │
│ GND      │──────────│ GND      │
│ I/O      │──────────│ D4       │
└──────────┘          └──────────┘
```

### 시리얼 명령어

| 명령 | 기능 |
|------|------|
| `1` | 부저 ON |
| `0` | 부저 OFF |
| `b` | 짧은 비프 |
| `B` | 긴 비프 |
| `a` | 알람 패턴 |
| `s` | SOS 패턴 |
| `m` | 멜로디 패턴 |

### 능동 vs 수동 부저

| 특성 | 능동 부저 | 수동 부저 |
|------|----------|----------|
| 발진 회로 | 내장 | 없음 |
| 제어 방식 | HIGH/LOW | PWM (tone) |
| 음 높이 | 고정 | 조절 가능 |
| 용도 | 알림음 | 멜로디 |

---

## 4. 수동 부저 모듈

### 회로 연결

```
수동 부저 모듈         NUCLEO-F103RB
┌──────────┐          ┌──────────┐
│ VCC      │──────────│ 3.3V/5V  │
│ GND      │──────────│ GND      │
│ I/O      │──────────│ D5 (PWM) │
└──────────┘          └──────────┘
```

### 시리얼 명령어

| 명령 | 기능 |
|------|------|
| `1`-`8` | 음계 재생 (도~높은도) |
| `s` | 스케일 (도레미파솔라시도) |
| `m` | 작은 별 멜로디 |
| `h` | 생일 축하 노래 |
| `a` | 알람 사운드 |
| `p` | 경찰 사이렌 |
| `w` | 앰뷸런스 사이렌 |
| `0` | 정지 |

### 음계 주파수

```
도(C4)=262Hz  레(D4)=294Hz  미(E4)=330Hz  파(F4)=349Hz
솔(G4)=392Hz 라(A4)=440Hz  시(B4)=494Hz  도(C5)=523Hz
```

---

## 5. 로터리 엔코더 모듈

### 회로 연결

```
KY-040 엔코더          NUCLEO-F103RB
┌──────────┐          ┌──────────┐
│ VCC (+)  │──────────│ 3.3V/5V  │
│ GND      │──────────│ GND      │
│ CLK      │──────────│ D6       │
│ DT       │──────────│ D7       │
│ SW       │──────────│ D8       │
└──────────┘          └──────────┘
```

### 동작 모드

| 모드 | 설명 |
|------|------|
| Counter | 회전 위치 카운트 (무한) |
| Volume | 볼륨 조절 시뮬레이션 (0-100%) |
| Brightness | 밝기 조절 시뮬레이션 (0-100%) |

### 조작 방법
- **시계방향 회전**: 값 증가
- **반시계방향 회전**: 값 감소
- **버튼 누름**: 모드 전환

### 시리얼 출력 예시
```
Mode Changed: Volume
Volume: 55% [===========         ]
Volume: 60% [============        ]
Volume: 65% [=============       ]
```

---

## 📁 프로젝트 구조

```
stm32_modules/
├── README.md
├── 01_Joystick/
│   └── Joystick.ino
├── 02_Relay/
│   └── Relay.ino
├── 03_Active_Buzzer/
│   └── Active_Buzzer.ino
├── 04_Passive_Buzzer/
│   └── Passive_Buzzer.ino
└── 05_Rotary_Encoder/
    └── Rotary_Encoder.ino
```

---

## 🚀 시작하기

### Arduino IDE 설정

1. **보드 매니저에서 STM32 패키지 설치**
   - File → Preferences → Additional Boards Manager URLs에 추가:
   ```
   https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
   ```

2. **보드 선택**
   - Tools → Board → STM32 boards groups → Nucleo-64
   - Board part number: Nucleo F103RB

3. **업로드 방법 선택**
   - Tools → Upload method → STM32CubeProgrammer (SWD)

### PlatformIO 설정

`platformio.ini` 예시:
```ini
[env:nucleo_f103rb]
platform = ststm32
board = nucleo_f103rb
framework = arduino
monitor_speed = 115200
```

---

## ⚠️ 주의사항

1. **릴레이 모듈**
   - 고전압/고전류 회로 제어 시 안전에 주의
   - 릴레이 코일은 5V 전원 필요
   - HIGH/LOW 트리거 타입 확인 필요

2. **부저 모듈**
   - 능동/수동 부저 구분 필요
   - 수동 부저는 PWM 지원 핀 사용 권장

3. **엔코더 모듈**
   - 고속 회전 시 펄스 누락 가능
   - 노이즈 방지를 위해 풀업 저항 사용

---

## 📚 참고 자료

- [STM32duino GitHub](https://github.com/stm32duino)
- [NUCLEO-F103RB User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
- [Arduino Reference](https://www.arduino.cc/reference/en/)

---









