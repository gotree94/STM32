# IR Transmitter Module Test

STM32F103 NUCLEO 보드를 이용한 IR 송신 모듈 테스트 프로젝트

## 개요

이 프로젝트는 IR(적외선) 송신 모듈을 테스트하기 위한 코드입니다. 38kHz 캐리어 주파수로 변조된 NEC 프로토콜 IR 신호를 송신합니다.

## 하드웨어 구성

### 필요 부품

| 부품 | 수량 | 설명 |
|------|------|------|
| NUCLEO-F103RB | 1 | STM32F103 개발보드 |
| IR LED (940nm) | 1 | 적외선 발광 다이오드 |
| 저항 100Ω | 1 | IR LED 전류 제한용 |
| 트랜지스터 2N2222 | 1 | IR LED 드라이버 (선택사항) |

### 핀 연결

```
STM32F103 NUCLEO          IR Transmitter Module
-----------------          --------------------
PA8 (TIM1_CH1) ---------> IR LED (+) 또는 Signal
GND           ---------> IR LED (-) 또는 GND
3.3V          ---------> VCC (모듈 사용 시)

PC13 (USER BTN) : 내장 버튼 (송신 트리거)
PA2  (USART2_TX): USB Virtual COM (디버그 출력)
PA3  (USART2_RX): USB Virtual COM
```

### 회로도

```
방법 1: 직접 연결 (저전류)
       PA8 ----[100Ω]----[IR LED]---- GND

방법 2: 트랜지스터 드라이버 (고전류, 장거리)
                           +3.3V
                             |
                         [IR LED]
                             |
       PA8 ----[1kΩ]----B|   |C
                         |NPN|
                         |   |E
                            GND
```

## 소프트웨어 설명

### NEC 프로토콜

이 프로젝트는 NEC IR 프로토콜을 사용합니다:

```
[Leader] [Address] [Address Inv] [Command] [Command Inv] [Stop]
  9ms      8bit       8bit         8bit        8bit       560us
 +4.5ms

비트 인코딩:
- Logic 0: 560us pulse + 560us space
- Logic 1: 560us pulse + 1690us space
```

### 주요 함수

| 함수 | 설명 |
|------|------|
| `IR_CarrierOn()` | 38kHz PWM 캐리어 시작 |
| `IR_CarrierOff()` | 캐리어 정지 |
| `IR_DelayUs()` | 마이크로초 딜레이 |
| `IR_SendPulse()` | 펄스+스페이스 송신 |
| `IR_SendByte()` | 1바이트 송신 (LSB first) |
| `IR_SendNEC()` | NEC 프레임 송신 |

### 타이머 설정

**TIM1 (38kHz PWM 캐리어)**
```c
Prescaler = 0
Period = 1894  // 72MHz / 38kHz - 1
Pulse = 947    // 50% duty cycle
```

**TIM2 (마이크로초 타이머)**
```c
Prescaler = 71  // 72MHz / 72 = 1MHz
Period = 0xFFFF
```

## 빌드 및 실행

### STM32CubeIDE 사용 시

1. 새 STM32 프로젝트 생성 (STM32F103RB 선택)
2. `main.c` 파일을 생성된 프로젝트에 복사
3. STM32F1xx HAL 드라이버 포함 확인
4. 빌드 후 플래시

### Makefile 프로젝트 시

```bash
# arm-none-eabi-gcc 설치 필요
make all
make flash
```

## 사용 방법

1. 보드에 프로그램 업로드
2. 시리얼 터미널 연결 (115200 baud)
3. USER 버튼(파란색)을 눌러 IR 신호 송신
4. 터미널에서 송신 결과 확인

### 시리얼 출력 예시

```
========================================
  IR Transmitter Test - STM32F103
========================================
Press USER button to send IR signal
Address: 0x00, Command: 0x12
Carrier Frequency: 38000 Hz
----------------------------------------

[1] Transmitting IR signal...
       Address: 0x00, Command: 0x12
       Transmission complete!

[2] Transmitting IR signal...
       Address: 0x00, Command: 0x12
       Transmission complete!
```

## 커스터마이징

### 주소/명령 변경

```c
// main.c에서 수정
uint8_t ir_address = 0x00;  // 원하는 주소로 변경
uint8_t ir_command = 0x12;  // 원하는 명령으로 변경
```

### 캐리어 주파수 변경

일부 장치는 36kHz 또는 40kHz를 사용합니다:

```c
// 36kHz의 경우
htim1.Init.Period = 1999;  // 72MHz / 36kHz - 1

// 40kHz의 경우  
htim1.Init.Period = 1799;  // 72MHz / 40kHz - 1
```

## 문제 해결

| 증상 | 원인 | 해결 방법 |
|------|------|----------|
| 신호 안 나감 | IR LED 극성 | LED 방향 확인 |
| 거리 짧음 | 전류 부족 | 트랜지스터 드라이버 추가 |
| 수신기 인식 불가 | 주파수 불일치 | 캐리어 주파수 조정 |
| 불안정한 신호 | 타이밍 오류 | 인터럽트 비활성화 확인 |

## 참고 자료

- [NEC Protocol Specification](https://www.sbprojects.net/knowledge/ir/nec.php)
- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [NUCLEO-F103RB User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)

## 라이선스

MIT License
