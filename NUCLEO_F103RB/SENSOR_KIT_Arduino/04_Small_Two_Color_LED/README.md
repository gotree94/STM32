# 04. 소형 2색 LED 모듈

NUCLEO-F103RB에서 소형 2색 LED 모듈을 제어하는 프로젝트입니다.

## 📋 목차

- [부품 정보](#부품-정보)
- [하드웨어 연결](#하드웨어-연결)
- [코드 설명](#코드-설명)
- [실행 결과](#실행-결과)
- [문제 해결](#문제-해결)

---

## 부품 정보

### 소형 2색 LED 모듈 사양

| 항목 | 사양 |
|------|------|
| 동작 전압 | 3.3V ~ 5V |
| LED 크기 | 3mm (일반 5mm보다 작음) |
| LED 색상 | 빨강 + 녹색 |
| 핀 구성 | R, G, GND |
| 제어 방식 | 디지털 또는 PWM |

### 소형 LED 특징

- 3mm LED 사용 (일반 5mm 대비 작음)
- 밝기가 일반 LED보다 다소 어두움
- 상태 표시용으로 적합
- 전류 약 10~15mA

---

## 하드웨어 연결

### 회로도

```
소형 2색 LED 모듈       NUCLEO-F103RB
┌──────────┐          ┌─────────────┐
│          │          │             │
│  R ──────┼──────────┤ D3  (PB3)   │  PWM
│          │          │             │
│  G ──────┼──────────┤ D5  (PB4)   │  PWM
│          │          │             │
│  - ──────┼──────────┤ GND         │
│          │          │             │
└──────────┘          └─────────────┘
```

### 연결 표

| 소형 2색 LED 핀 | NUCLEO 핀 | Arduino 핀 | 기능 |
|----------------|-----------|------------|------|
| R (Red) | PB3 | D3 | PWM 출력 |
| G (Green) | PB4 | D5 | PWM 출력 |
| - (GND) | GND | GND | 접지 |

---

## 실행 결과

### 시리얼 모니터 출력

```
================================
  Small Two Color LED Test
  NUCLEO-F103RB + Arduino
================================
Note: Using 3mm small LED module

=== Status Indicator ===
Status: OK (Green)
Status: Warning (Yellow)
Status: Error (Red)
Status: Standby (Green Blink)

=== Alert Patterns ===
DANGER: Fast red blink
CAUTION: Slow yellow blink
NORMAL: Steady green
```

---

## 일반 2색 LED와의 차이점

| 항목 | 일반 2색 LED | 소형 2색 LED |
|------|-------------|-------------|
| LED 크기 | 5mm | 3mm |
| 밝기 | 높음 | 낮음 |
| 전류 | ~20mA | ~10-15mA |
| 용도 | 범용 | 상태 표시 |

---

## 문제 해결

### LED가 너무 어두움

- 소형 LED의 정상적인 특성입니다
- 필요시 MAX_BRIGHTNESS 값을 255로 조정

### LED가 안 켜짐

- 핀 번호 확인 (D3, D5)
- GND 연결 확인

---

## 라이선스

MIT License
