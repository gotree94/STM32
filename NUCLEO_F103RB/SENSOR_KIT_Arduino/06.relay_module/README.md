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

## 📄 라이선스

MIT License

## 🔗 참고 자료

- [STM32duino 공식 문서](https://github.com/stm32duino/Arduino_Core_STM32)
- [NUCLEO-F103RB 데이터시트](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
