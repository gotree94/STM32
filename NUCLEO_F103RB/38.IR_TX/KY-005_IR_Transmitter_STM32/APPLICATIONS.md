# KY-005 IR Transmitter — 응용 사례 및 프로젝트 아이디어

KY-005 IR 송신기 모듈과 STM32F103 NUCLEO 보드를 활용한 다양한 응용 사례를 소개합니다.

---

## 1. 📺 범용 IR 리모컨 (Universal IR Remote)

가장 기본적인 응용입니다. UART 명령이나 버튼 입력으로 TV, 에어컨, 셋톱박스 등을 제어합니다.

**기능:**
- NEC, Sony SIRC, RC5, Samsung 등 여러 프로토콜 지원
- 시리얼 명령어로 장치/코드 선택
- 매크로 기능 (버튼 하나로 여러 코드 순차 전송)

**확장 아이디어:**
- OLED 화면(SSD1306)으로 현재 선택된 장치 표시
- 로터리 엔코더로 메뉴 탐색
- IR 코드 라이브러리를 SD 카드나 외부 EEPROM에 저장

```c
// 예: 삼성 TV 전원 + 볼륨 10으로 설정 매크로
void Macro_Watch_TV(void)
{
    IR_Send_NEC_Standard(0x07, 0x40);  // Power
    HAL_Delay(500);
    IR_Send_NEC_Standard(0x07, 0x10);  // Vol+
    HAL_Delay(200);
    IR_Send_NEC_Standard(0x07, 0x10);  // Vol+
    HAL_Delay(200);
    // ... 10번 반복
}
```

---

## 2. 🌐 WiFi IR 브리지 (ESP8266/ESP32 연동)

STM32가 ESP8266 또는 ESP32와 UART 통신하여 스마트폰이나 음성 비서로 IR 장치를 제어합니다.

**아키텍처:**
```
스마트폰 App
    ↓ (WiFi)
ESP8266 (UART-WiFi 게이트웨이)
    ↓ (UART, AT commands)
STM32F103 (IR 컨트롤러)
    ↓ (PWM 38kHz)
KY-005 (IR LED)
```

**프로토콜 예시 (UART):**
```
TX→ "IR:NEC:07:40\r\n"   → Samsung Power 전송
TX→ "IR:SONY:1521\r\n"   → Sony 12비트 전송
TX→ "IR:RAW:9000,4500,...\r\n" → Raw 데이터 전송
TX→ "MACRO:1\r\n"        → 저장된 매크로 #1 실행
```

**확장:**
- MQTT 프로토콜로 홈어시스턴트(Home Assistant) 연동
- "Alexa, turn on TV" → Node-RED → MQTT → ESP8266 → STM32 → KY-005

---

## 3. 🎙️ IR 신호 분석기 + Replay (KY-022 필요)

KY-022 IR 수신기로 리모컨 신호를 캡처하고 분석하여 재생합니다.

**동작 흐름:**
```
KY-022 → STM32 (Input Capture) → 타이밍 분석 → LCD/시리얼 출력
                                           ↓
                                      EEPROM 저장
                                           ↓
                                   KY-005로 재생
```

**기능:**
- 알 수 없는 리모컨의 코드 학습 및 저장
- 캡처한 신호의 프로토콜 자동 판별 (NEC/Sony/RC5 등)
- 최대 128개의 버튼 코드를 내부 Flash에 저장
- PC 도구 없이 현장에서 학습 및 재생

---

## 4. 🕹️ IR 원격 데이터 전송기

KY-005를 단방향 데이터 링크로 사용하여 센서 데이터나 제어 명령을 전송합니다.

**NEC 응용:**
- 8비트 주소 + 8비트 명령으로 총 256개 장치 구분 가능
- 각 장치당 256개 명령 → 총 65,536개 상태 전송 가능

**확장 프로토콜:**
- NEC 프레임을 연속 전송하여 더 많은 데이터 전송
- 4개의 NEC 프레임으로 16바이트 데이터 전송 (오류 검출 포함)

```
프레임 1: 주소(8) + 명령(8) → 데이터 바이트 0
프레임 2: 주소(8) + 명령(8) → 데이터 바이트 1
프레임 3: 주소(8) + 명령(8) → 데이터 바이트 2
프레임 4: 주소(8) + 명령(8) → 체크섬
```

**활용:**
- 무선 온도 센서 데이터 전송
- 로봇 원격 제어
- 간단한 1-Way 통신 시스템

---

## 5. 🔄 IR 리피터/익스텐더 (KY-022 + KY-005)

KY-022로 IR 신호를 수신하여 KY-005로 재전송, IR 신호의 도달 거리를 연장합니다.

**동작:**
```
리모컨 → [KY-022 수신] → STM32 → [KY-005 재전송] → TV
          (2m)                          (5m)          (7m)
```

**구현:**
- Input Capture로 원시 타이밍 캡처
- 버퍼에 저장 후 즉시 KY-005로 재전송
- 지연 시간 < 1ms (실시간에 가까운 재생)

**고급 기능:**
- 신호 증폭 (재생 시 PWM 듀티 조정)
- 프로토콜 변환 (예: NEC → Sony 변환)
- 멀티 수신기로 전방향 커버리지

---

## 6. 🏭 적외선 신호 발생기 (테스트 장비)

IR 장비 테스트를 위한 정밀 신호 발생기입니다.

**주요 기능:**
- 38kHz 이외의 다양한 반송파 주파수 지원 (36kHz, 40kHz, 56kHz)
- 정밀한 타이밍 조정 (1us 단위)
- 연속 전송 모드
- 버스트 모드 (지정된 횟수만큼 전송)

**적용 분야:**
- IR 리모컨 생산 라인 테스트
- IR 수신기 감도 측정
- 프로토콜 분석기 calibration

---

## 7. 🏠 홈 오토메이션 시나리오

### 시나리오 A: "굿모닝"
```
RTC 알람 (오전 7시) →
  [STM32] → KY-005: TV Power (NEC)
         → KY-005: 볼륨 15 설정
         → UART: ESP8266 → MQTT → 조명 On
```

### 시나리오 B: "무드 라이트"
```
BLE/핸드폰 App →
  [STM32] → KY-005: 조명 On (IR 조명 스위치)
         → KY-005: 색상 변경
         → UART: WS2812 LED 제어
```

### 시나리오 C: "에너지 세이브"
```
PIR 센서 (30분간 움직임 없음) →
  [STM32] → KY-005: 에어컨 Off
         → KY-005: TV Off
         → KY-005: 조명 Off
```

---

## 8. 🎮 로봇 간 IR 통신

**시스템:**
```
로봇 A (STM32 + KY-005)
  → 송신: 명령 코드 (전진/후진/회전)
로봇 B (STM32 + KY-022)
  → 수신: 명령 디코딩 후 모터 제어
```

**장점:**
- 저렴한 비용 (LED + 수신기 모듈만 필요)
- 좁은 지향성으로 원하지 않는 수신 방지
- 실내 조명과 구분되는 38kHz 변조

**한계:**
- 가시선 통신만 가능
- 거리 약 5~10m (드라이버 추가 시)
- 1-Way 통신만 가능 (별도의 수신기 필요)

---

## 9. 🎵 IR 동기화 멀티미디어 시스템

여러 IR 제어 장치를 동기화하여 미디어 경험을 향상시킵니다.

**영화 감상 모드:**
```
버튼 하나로:
  1. TV Power On (Samsung NEC 0x07 0x40)
  2. 사운드바 Power On (LG NEC)
  3. 블루레이 플레이어 Power On
  4. 조명 어둡게 (IR 조명 스위치)
  5. 커튼 닫기 (IR 커튼 모터)
```

---

## 10. 📋 IR 코드 데이터베이스 예제

### NEC 프로토콜 (일반적인 코드)

| 제조사 | 주소(Hex) | 명령 | 기능 |
|--------|-----------|------|------|
| Samsung | 0x07 | 0x40 | Power |
| Samsung | 0x07 | 0x10 | Vol+ |
| Samsung | 0x07 | 0x11 | Vol- |
| Samsung | 0x07 | 0x0D | Mute |
| Samsung | 0x07 | 0x20 | CH+ |
| Samsung | 0x07 | 0x21 | CH- |
| Panasonic | 0x04 | 0x40 | Power |
| LG | 0x04 | 0x10 | Power |
| Sony (SIRC) | Addr=0x01 | Cmd=0x15 | Power |
| Sony (SIRC) | Addr=0x01 | Cmd=0x12 | Vol+ |
| Sony (SIRC) | Addr=0x01 | Cmd=0x13 | Vol- |

### 코드 검색 방법
1. **LIRC 데이터베이스**: https://lirc.org/ (가장 방대한 IR 코드 DB)
2. **직접 캡처**: KY-022 수신기로 원격 리모컨 코드 캡처
3. **스마트폰 앱**: "IR Remote" 앱으로 코드 탐색 후 UART로 STM32에 전달

---

## 프로젝트 확장을 위한 팁

### FreeRTOS 통합
IR 송신은 블로킹 방식이므로, FreeRTOS 태스크로 분리하는 것이 좋습니다:
```c
void IR_Task(void *argument)
{
    IR_CommandQueue_t cmd;
    while (1)
    {
        if (xQueueReceive(IR_CommandQueue, &cmd, portMAX_DELAY))
        {
            IR_Send_NEC_Standard(cmd.address, cmd.command);
        }
    }
}
```

### 전력 최적화
배터리 구동 시:
- IR 송신 전에만 TIM2 클럭 활성화
- 송신 완료 후 즉시 TIM2 클럭 비활성화
- STM32 Stop 모드에서 RTC 알람으로 웨이크업

### DMA 전송
고급 응용을 위해 DMA + Timer로 완전 자동화된 IR 송신 가능:
- DMA로 타이밍 테이블 전송
- Timer One-Pulse Mode로 38kHz 버스트 제어
- CPU 간섭 없이 IR 프레임 전체 전송
