# Relay Module - STM32F103 NUCLEO

릴레이 모듈을 STM32F103 NUCLEO 보드로 제어하는 예제입니다.

## 하드웨어 구성

### 부품 목록
| 부품 | 수량 | 비고 |
|------|------|------|
| STM32F103 NUCLEO | 1 | NUCLEO-F103RB |
| 릴레이 모듈 | 1 | 1채널 또는 다채널 |
| 점퍼 와이어 | 3+ | M-F 타입 |

### 핀 연결

```
릴레이 모듈          STM32F103 NUCLEO
-----------------------------------------
VCC      ────────►  5V (CN7-18) 또는 3.3V
GND      ────────►  GND (CN7-20)
IN/SIG   ────────►  PA5 (CN10-11)
```

<img width="644" height="586" alt="F103RB-pin" src="https://github.com/user-attachments/assets/d084e623-5b9c-4724-855e-e72b6bb1ac7b" />

### 회로도

```
                    ┌─────────────────┐
                    │  릴레이 모듈     │
    PA5 ──────────► │ IN (S)          │
                    │                 │
    5V  ──────────► │ VCC (+)     COM ├──► 부하 공통
                    │             NO  ├──► 정상 개방
    GND ──────────► │ GND (-)     NC  ├──► 정상 폐쇄
                    └─────────────────┘
```

## 기능 설명

1. **자동 토글 모드**: 1초 간격으로 릴레이 ON/OFF 반복
2. **수동 제어**: User Button(B1, PC13)으로 릴레이 토글
3. **시리얼 모니터링**: UART2를 통한 상태 출력 (115200 baud)

## 빌드 및 업로드

### STM32CubeIDE 사용 시
1. 새 프로젝트 생성 (STM32F103RB 선택)
2. `main.c` 내용을 프로젝트에 복사
3. 빌드 후 업로드

## 시리얼 출력 예시

```
========================================
  Relay Module Test - STM32F103 NUCLEO
========================================
PA5: Relay Signal Output
PC13: User Button (Manual Control)

Relay State: ON
Relay State: OFF
Relay State: ON
[BUTTON] Relay State: OFF
```

## 동작 확인

1. 보드에 전원 인가 시 릴레이가 1초 간격으로 ON/OFF
2. 릴레이 동작 시 "딸깍" 소리 확인
3. User Button(파란색) 누르면 수동 토글
4. 시리얼 터미널에서 상태 메시지 확인

## 주의사항

⚠️ **안전 주의**
- 릴레이로 AC 220V 등 고전압 제어 시 감전 주의
- 릴레이 정격 전압/전류 확인 필수
- 유도성 부하(모터, 솔레노이드) 연결 시 플라이백 다이오드 사용 권장

⚠️ **전원 관련**
- 릴레이 모듈에 따라 5V 필요 (3.3V로 동작 안 될 수 있음)
- 여러 릴레이 사용 시 외부 전원 공급 권장

## 응용 예제

```c
// 특정 시간 동안 릴레이 ON 유지
void Relay_OnFor(uint32_t duration_ms)
{
    Relay_On();
    HAL_Delay(duration_ms);
    Relay_Off();
}

// PWM 방식 제어 (조광기 등)
void Relay_PWM(uint8_t duty, uint32_t period_ms)
{
    uint32_t on_time = (period_ms * duty) / 100;
    uint32_t off_time = period_ms - on_time;
    
    Relay_On();
    HAL_Delay(on_time);
    Relay_Off();
    HAL_Delay(off_time);
}
```

## 트러블슈팅

| 증상 | 원인 | 해결방법 |
|------|------|----------|
| 릴레이 동작 안함 | 전원 부족 | 5V 전원 사용 |
| 릴레이 떨림 | 접점 바운스 | 디바운스 딜레이 추가 |
| MCU 리셋됨 | 역기전력 | 플라이백 다이오드 추가 |

## 참고 자료

- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [NUCLEO-F103RB User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)

## 라이선스

MIT License
