# Line Tracking Module - STM32F103 NUCLEO

라인 트래킹(추적) 모듈(TCRT5000 기반)을 STM32F103 NUCLEO 보드로 테스트하는 예제입니다.

## 하드웨어 구성

### 부품 목록
| 부품 | 수량 | 비고 |
|------|------|------|
| STM32F103 NUCLEO | 1 | NUCLEO-F103RB |
| 라인 트래킹 모듈 | 1 | 3채널 또는 5채널 |
| 점퍼 와이어 | 5+ | M-F 타입 |
| 테스트 라인 | 1 | 검은색 테이프 또는 인쇄물 |

### 센서 모듈 사양
- 센서 타입: TCRT5000 적외선 반사 센서
- 동작 전압: 3.3V ~ 5V
- 출력: 디지털 (Active Low)
- 감지 거리: 1mm ~ 8mm (최적: 2~3mm)
- 감지 대상: 검은색 = LOW, 흰색 = HIGH

### 핀 연결 (3채널)

```
트래킹 모듈           STM32F103 NUCLEO
-----------------------------------------
VCC      ────────►  3.3V (CN7-16)
GND      ────────►  GND (CN7-20)
L (Left)  ────────►  PA0 (CN7-28)
C (Center)────────►  PA1 (CN7-30)
R (Right) ────────►  PA4 (CN7-32)
```

### 회로도

```
                     ┌────────────────────────┐
                     │    라인 트래킹 모듈     │
                     │   [L]   [C]   [R]      │
    PA0 ◄────────────┤    L                   │
    PA1 ◄────────────┤    C                   │
    PA4 ◄────────────┤    R                   │
                     │                        │
    3.3V ──────────► │ VCC                    │
    GND  ──────────► │ GND                    │
                     └────────────────────────┘
                     
    PA5 ──────────► LED (중앙 정렬 시 ON)
```

## 센서 동작 원리

```
   IR LED ─►  ◯ ─►  [표면]  ─►  ◯  ◄─ IR Receiver
            발광              수광
            
   흰색 표면: 반사 많음 ─► HIGH 출력 (라인 없음)
   검은색 표면: 반사 적음 ─► LOW 출력 (라인 감지)
```

## 기능 설명

1. **3채널 라인 감지**
   - 왼쪽, 중앙, 오른쪽 센서 독립 읽기
   - 조합에 따른 라인 위치 판단

2. **위치 분류**
   - `CENTER`: 중앙 정렬
   - `LEFT` / `RIGHT`: 크게 치우침
   - `SLIGHT_LEFT` / `SLIGHT_RIGHT`: 약간 치우침
   - `LOST`: 라인 이탈
   - `ALL_BLACK`: 교차점 또는 넓은 라인

3. **시각적 표시**
   - 센서 상태: `[#|_|_]` 형태로 표시
   - 방향 화살표: `<<<---`, `--||--`, `--->>>` 등

## 빌드 및 업로드

### STM32CubeIDE 사용 시
1. 새 프로젝트 생성 (STM32F103RB 선택)
2. PA0, PA1, PA4를 GPIO_Input으로 설정
3. `main.c` 내용을 프로젝트에 복사
4. 빌드 후 업로드

### CubeMX 설정
```
Pinout:
- PA0: GPIO_Input (Pull-up)
- PA1: GPIO_Input (Pull-up)
- PA4: GPIO_Input (Pull-up)
- PA5: GPIO_Output
```

## 시리얼 출력 예시

```
============================================
  Line Tracking Module Test
  STM32F103 NUCLEO
============================================
PA0: Left Sensor
PA1: Center Sensor
PA4: Right Sensor
(0=Line Detected, 1=No Line)

Sensors: [_|#|_] | Position: CENTER       | --||--
Sensors: [#|#|_] | Position: SLIGHT LEFT  |  <<-- 
Sensors: [#|_|_] | Position: LEFT         | <<<---
Sensors: [_|_|_] | Position: LINE LOST!   | ??????
Sensors: [#|#|#] | Position: ALL BLACK    | ######
```

## 센서 조합 해석표

| L | C | R | 위치 | 설명 |
|---|---|---|------|------|
| 0 | 0 | 0 | ALL_BLACK | 교차점/정지선 |
| 0 | 0 | 1 | SLIGHT_LEFT | 약간 왼쪽 |
| 0 | 1 | 0 | LEFT | 크게 왼쪽 |
| 0 | 1 | 1 | LEFT | 왼쪽 이탈 |
| 1 | 0 | 0 | SLIGHT_RIGHT | 약간 오른쪽 |
| 1 | 0 | 1 | CENTER | 정확히 중앙 |
| 1 | 1 | 0 | RIGHT | 크게 오른쪽 |
| 1 | 1 | 1 | LOST | 라인 이탈 |

(0 = 검은색 감지, 1 = 흰색/라인없음)

## 응용 예제

### 라인트레이서 로봇 제어
```c
typedef struct {
    int16_t left_speed;
    int16_t right_speed;
} MotorSpeed;

MotorSpeed Calculate_Motor_Speed(LinePosition pos)
{
    MotorSpeed speed = {100, 100};  // 기본 속도
    
    switch (pos)
    {
        case POS_CENTER:
            speed.left_speed = 100;
            speed.right_speed = 100;
            break;
            
        case POS_SLIGHT_LEFT:
            speed.left_speed = 80;
            speed.right_speed = 100;
            break;
            
        case POS_LEFT:
            speed.left_speed = 50;
            speed.right_speed = 100;
            break;
            
        case POS_SLIGHT_RIGHT:
            speed.left_speed = 100;
            speed.right_speed = 80;
            break;
            
        case POS_RIGHT:
            speed.left_speed = 100;
            speed.right_speed = 50;
            break;
            
        case POS_LOST:
            // 마지막 방향으로 회전하며 탐색
            speed.left_speed = -50;
            speed.right_speed = 50;
            break;
            
        case POS_ALL_BLACK:
            // 정지 또는 직진
            speed.left_speed = 100;
            speed.right_speed = 100;
            break;
            
        default:
            speed.left_speed = 0;
            speed.right_speed = 0;
            break;
    }
    
    return speed;
}
```

### PID 제어
```c
typedef struct {
    float kp, ki, kd;
    float integral;
    float prev_error;
} PID_Controller;

float PID_Calculate(PID_Controller *pid, float error)
{
    pid->integral += error;
    float derivative = error - pid->prev_error;
    pid->prev_error = error;
    
    return (pid->kp * error) + 
           (pid->ki * pid->integral) + 
           (pid->kd * derivative);
}

// 사용 예
float Get_Line_Error(void)
{
    uint8_t left = Read_Sensor_Left();
    uint8_t center = Read_Sensor_Center();
    uint8_t right = Read_Sensor_Right();
    
    // -100 ~ +100 범위의 에러값 계산
    // 음수: 왼쪽으로 치우침, 양수: 오른쪽으로 치우침
    int weighted_sum = (left * -100) + (center * 0) + (right * 100);
    int active_count = (!left) + (!center) + (!right);
    
    if (active_count == 0) return 0;  // 라인 이탈
    
    return (float)weighted_sum / active_count;
}
```

### 5채널 센서 확장
```c
#define SENSOR_L2_PIN   GPIO_PIN_6   // 가장 왼쪽
#define SENSOR_L1_PIN   GPIO_PIN_0   // 왼쪽
#define SENSOR_C_PIN    GPIO_PIN_1   // 중앙
#define SENSOR_R1_PIN   GPIO_PIN_4   // 오른쪽
#define SENSOR_R2_PIN   GPIO_PIN_7   // 가장 오른쪽

int Get_Line_Position_5CH(void)
{
    int position = 0;
    int weight = 0;
    
    if (!Read_Sensor(SENSOR_L2_PIN)) { position += -2000; weight++; }
    if (!Read_Sensor(SENSOR_L1_PIN)) { position += -1000; weight++; }
    if (!Read_Sensor(SENSOR_C_PIN))  { position += 0;     weight++; }
    if (!Read_Sensor(SENSOR_R1_PIN)) { position += 1000;  weight++; }
    if (!Read_Sensor(SENSOR_R2_PIN)) { position += 2000;  weight++; }
    
    if (weight == 0) return 9999;  // 라인 이탈
    
    return position / weight;  // -2000 ~ +2000 범위
}
```

## 트러블슈팅

| 증상 | 원인 | 해결방법 |
|------|------|----------|
| 감지 안됨 | 거리 너무 멂 | 센서를 표면에 2-3mm로 조절 |
| 항상 감지 | 거리 너무 가까움 | 센서 높이 증가 |
| 불안정 | 조명 간섭 | 센서에 차광 튜브 추가 |
| 흰색도 감지 | 감도 과다 | 모듈 가변저항 조절 |

## 테스트 환경 만들기

### 라인 트랙 제작
1. 흰색 바탕에 검은색 테이프(폭 2-3cm)
2. A4 용지에 검은색 선 인쇄
3. 센서-표면 거리: 2-3mm 유지

### 조명 조건
- 직사광선 피하기
- 일정한 조명 환경 권장
- 필요시 센서 주변에 차광판 설치

## 참고 자료

- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [TCRT5000 Datasheet](https://www.vishay.com/docs/83760/tcrt5000.pdf)

## 라이선스

MIT License
