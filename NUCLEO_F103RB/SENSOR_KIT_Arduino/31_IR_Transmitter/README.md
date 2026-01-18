# IR Transmitter Module (IR 송신 모듈)

NUCLEO-F103RB 보드를 이용한 IR 송신 모듈 테스트 프로젝트

## 개요

38kHz 캐리어 주파수로 변조된 NEC 프로토콜 IR 신호를 송신합니다.

## 하드웨어 연결

### 핀 배치

| 핀 | 기능 | 설명 |
|----|------|------|
| PA8 (D7) | IR LED | 38kHz PWM 출력 |
| PC13 | Button | 내장 User 버튼 |
| PA5 (D13) | LED | 내장 LED |

### 회로도

```
기본 연결:
    PA8 ----[100Ω]----[IR LED]---- GND

트랜지스터 드라이버 (권장, 더 강한 출력):
                      VCC (3.3V)
                        │
                    [100Ω]
                        │
                   [IR LED]
                        │
                        C
    PA8 ---[1kΩ]--- B  2N2222
                        E
                        │
                       GND
```

### 부품 목록

- IR LED (940nm 권장)
- 100Ω 저항
- (선택) 2N2222 NPN 트랜지스터 + 1kΩ 저항

## 소프트웨어 설정

### Arduino IDE 설정

1. **보드 매니저에서 STM32 패키지 설치**
   - File → Preferences → Additional Boards Manager URLs에 추가:
   ```
   https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
   ```
   - Tools → Board → Boards Manager → "STM32" 검색 후 설치

2. **보드 선택**
   - Tools → Board → STM32 boards groups → Nucleo-64
   - Tools → Board part number → Nucleo F103RB

3. **포트 선택**
   - Tools → Port → COMx (Windows) 또는 /dev/ttyACMx (Linux)

## 사용 방법

1. 회로 연결 후 코드 업로드
2. 시리얼 모니터 열기 (115200 baud)
3. User 버튼(PC13)을 누르면 IR 신호 송신
4. 시리얼 모니터에서 송신 확인

## 시리얼 출력 예시

```
========================================
  IR Transmitter - NUCLEO-F103RB
  NEC Protocol, 38kHz Carrier
========================================
IR Address: 0x00
IR Command: 0x0C

Press USER button to transmit IR signal...

[TX] Sending NEC: Addr=0x00, Cmd=0x0C
[TX] Complete!
```

## NEC 프로토콜 타이밍

```
Leader:  ████████████████████░░░░░░░░░░░░
         |<---- 9ms ---->|<-- 4.5ms -->|

Bit '0': ██░░
         560us 560us

Bit '1': ██░░░░░░
         560us 1690us

Frame:   [Leader][Addr][~Addr][Cmd][~Cmd][Stop]
```

## 코드 수정

IR 주소와 명령을 변경하려면:

```cpp
uint8_t ir_address = 0x00;  // 디바이스 주소 변경
uint8_t ir_command = 0x0C;  // 명령 코드 변경
```

## 문제 해결

| 문제 | 해결 방법 |
|------|----------|
| IR 신호가 약함 | 트랜지스터 드라이버 회로 사용 |
| 수신기에서 인식 안됨 | IR LED 극성, 저항값 확인 |
| 버튼 반응 없음 | PC13 연결 및 풀업 확인 |

## 라이선스

MIT License
