# 🤖 STM32 Robot Car Peripheral Drivers

> STM32F103 기반 4륜 로봇카 주변장치 드라이버 모음  
> HAL 라이브러리 기반 논블로킹(Non-blocking) 설계

---

## 📁 드라이버 파일 구조

```
drivers/
├── anim.c          # 애니메이션 (LCD 눈 표정 시퀀서)
├── eyes.c          # 눈 표정 렌더러 (dirty flag 최적화)
├── lcd_st7735.c    # ST7735 SPI LCD 드라이버
├── lcd_gfx.c       # 그래픽 프리미티브 (사각형, 원, 선)
├── buzzer.c        # PWM 부저 드라이버 (멜로디 논블로킹)
├── motor.c         # DC 모터 방향 제어 (4WD)
├── servo.c         # SG90 서보모터 PWM 제어
├── rgb_led.c       # RGB LED 색상 제어
└── ultrasonic.c    # HC-SR04 초음파 거리 센서
```

---

## 🗂️ 하드웨어별 분류 목록

| 카테고리 | 파일 | 하드웨어 | 인터페이스 |
|----------|------|----------|-----------|
| 디스플레이 | `lcd_st7735.c`, `lcd_gfx.c`, `eyes.c`, `anim.c` | ST7735 1.8" LCD | SPI2 |
| 음향 | `buzzer.c` | 수동 부저 | TIM3 PWM |
| 구동계 | `motor.c` | DC 모터 × 4 (L298N) | GPIO |
| 구동계 | `servo.c` | SG90 서보모터 | TIM PWM |
| 센서 | `ultrasonic.c` | HC-SR04 초음파 | TIM2 IC |
| 표시등 | `rgb_led.c` | 공통양극 RGB LED | GPIO |

---

## 📺 1. LCD 디스플레이 그룹

### 1-1. ST7735 LCD 드라이버 (`lcd_st7735.c`)

#### 하드웨어 사양

| 항목 | 내용 |
|------|------|
| 모듈명 | ST7735S 1.8인치 TFT LCD |
| 해상도 | 128 × 160 px (RGB 16bit) |
| 인터페이스 | 4-wire SPI |
| 색상 포맷 | RGB565 (16bit/pixel) |
| 화면 회전 | MADCTL = 0x60 (가로 방향) |

#### 핀 연결 (STM32F103)

| LCD 핀 | STM32 핀 | 설명 |
|--------|----------|------|
| SCK | PB13 (SPI2_SCK) | 클럭 |
| SDA/MOSI | PB15 (SPI2_MOSI) | 데이터 |
| CS | PB12 | Chip Select (GPIO OUT) |
| DC/RS | PA8 | Data/Command 선택 |
| RES | PD2 | 하드웨어 리셋 |
| VCC | 3.3V | 전원 |
| GND | GND | 접지 |

#### CubeMX 설정

```
SPI2:
  Mode: Transmit Only Master
  Data Size: 8 Bits
  Clock Polarity (CPOL): Low
  Clock Phase (CPHA): 1 Edge
  Baud Rate: 18 Mbits/s 권장 (APB1 36MHz ÷ 2)
  DMA: SPI2_TX (선택, WriteBuffer용)

GPIO:
  PB12 → GPIO_Output (CS)
  PA8  → GPIO_Output (DC)
  PD2  → GPIO_Output (RES)
```

#### 주요 API

```c
void LCD_Init(void);                              // 초기화 (main에서 1회만)
void LCD_Clear(uint16_t color);                   // 전체 화면 채우기
void LCD_SetWindow(uint16_t x0, uint16_t y0,
                   uint16_t x1, uint16_t y1);    // 출력 영역 설정
void LCD_WriteColorFast(uint16_t color,
                        uint32_t count);          // 단색 고속 출력
void LCD_WriteBuffer(uint16_t *buf,
                     uint32_t count);             // 이미지 버퍼 DMA 출력
void LCD_DrawChar(uint16_t x, uint16_t y,
                  char c, uint16_t fg, uint16_t bg); // 문자 출력
void LCD_DrawString(uint16_t x, uint16_t y,
                    const char *str,
                    uint16_t fg, uint16_t bg);   // 문자열 출력
```

#### RGB565 색상 참조

```c
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define YELLOW  0xFFE0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
```

#### 신호 확인 방법

```
오실로스코프 측정:
  - SCK (PB13): SPI 클럭 파형 확인 (18MHz)
  - MOSI (PB15): 데이터 파형 확인
  - CS (PB12): 전송 시작/종료 시 LOW 펄스 확인
  - DC (PA8): 명령(LOW) / 데이터(HIGH) 전환 확인
  - RES (PD2): 초기화 시 50ms LOW 펄스 확인

논리분석기 설정:
  - Protocol: SPI
  - CPOL: 0, CPHA: 0
  - Bit Order: MSB First
```

---

### 1-2. 그래픽 프리미티브 (`lcd_gfx.c`)

#### 제공 함수

```c
void LCD_FillRect(int16_t x, int16_t y,
                  int16_t w, int16_t h, uint16_t color);    // 채운 사각형
void LCD_FillCircle(int16_t x0, int16_t y0,
                    int16_t r, uint16_t color);              // 채운 원
void LCD_RoundRect(int16_t x, int16_t y,
                   int16_t w, int16_t h,
                   int16_t r, uint16_t color);               // 둥근 사각형
void LCD_ThickLine(int16_t x0, int16_t y0,
                   int16_t x1, int16_t y1,
                   int16_t t, uint16_t color);               // 두꺼운 선
void LCD_VLine(int16_t x, int16_t y,
               int16_t h, uint16_t color);                   // 수직선
void LCD_DrawPixel(int16_t x, int16_t y, uint16_t color);   // 픽셀 점찍기
```

#### 성능 최적화 특징

- `FillCircle`: 픽셀 단위 → **수평선(HLine) 기반** (10배 이상 속도 향상)
- `RoundRect`: 중앙 직사각형 + 4 모서리 1/4원 분리 방식 (중복 픽셀 없음)
- `ThickLine`: Bresenham 알고리즘 + 원형 캡 적용

---

### 1-3. 눈 표정 드라이버 (`eyes.c`)

#### 표정 종류 (Expression_t)

| 상수 | 설명 | 렌더링 방법 |
|------|------|-------------|
| `EXPR_NEUTRAL` | 기본 눈 | RoundRect (30×50, radius 10) |
| `EXPR_BLINK` | 깜빡임 | 가로 얇은 사각형 (30×6) |
| `EXPR_HAPPY` | 웃는 눈 | 아래쪽 반원형 RoundRect |
| `EXPR_ANGRY` | 화난 눈 | 대각선 ThickLine |
| `EXPR_SAD` | 슬픈 눈 | 처진 ThickLine + 하단 선 |
| `EXPR_SLEEPY` | 졸린 눈 | BLINK와 동일 (연속 감김) |
| `EXPR_LOOK_LEFT` | 왼쪽 시선 | 동공(검은 사각형) 왼쪽 배치 |
| `EXPR_LOOK_RIGHT` | 오른쪽 시선 | 동공(검은 사각형) 오른쪽 배치 |

#### 눈 좌표 설정

```c
#define LX  40      // 왼쪽 눈 중심 X
#define RX  120     // 오른쪽 눈 중심 X
#define CY  40      // 눈 중심 Y
#define EYE_COLOR   0x07E0  // 녹색 (변경 가능)
```

#### 주요 API

```c
void Eyes_SetExpression(Expression_t expr);  // 표정 설정 (dirty flag 적용)
void Eyes_Update(void);                       // 메인 루프에서 호출 (변경 시만 그림)
void Eyes_Draw(Expression_t expr);            // 강제 즉시 렌더링
void Eyes_Invalidate(void);                   // 강제 갱신 플래그 세트
Expression_t Eyes_GetExpression(void);        // 현재 표정 반환
```

#### 최적화 메커니즘

```c
// dirty flag: 표정이 변경될 때만 LCD 갱신
static uint8_t dirty = 1;

// servo 동작 중 LCD 갱신 억제 (SPI 충돌 방지)
extern volatile uint8_t servo_moving;
if (dirty && (now - lastTick >= 100) && !servo_moving)
    Eyes_Draw(current_expr);
```

---

### 1-4. 애니메이션 시퀀서 (`anim.c`)

#### 깜빡임 타이밍 (논블로킹)

```c
#define BLINK_INTERVAL_MS   3000   // 기본 깜빡임 주기
#define BLINK_DURATION_MS   150    // 눈 감는 시간
// 실제로는 rand()로 가변: 1~5초 주기, 80~200ms 감김
// 20% 확률로 더블 블링크 (0.1~0.3초 내 재깜빡임)
```

#### 초기화 순서

```c
// main.c에서 순서 준수:
LCD_Init();           // 1. LCD 하드웨어 초기화
LCD_Clear(0x0000);    // 2. 화면 클리어
Anim_Init();          // 3. 애니메이션 초기화 (표정 + 랜덤 시드)

// 메인 루프:
while (1) {
    Anim_Update();    // 깜빡임 + dirty flag 기반 렌더링
}
```

---

## 🔊 2. 부저 (Buzzer)

### 하드웨어 사양

| 항목 | 내용 |
|------|------|
| 타입 | 수동 부저 (Passive Buzzer) |
| 제어 방식 | PWM (가변 주파수) |
| 타이머 | TIM3 채널 1 (또는 설정에 따라 변경) |
| 듀티 비 | 50% (고정, 음질 최적) |
| 주파수 범위 | 31Hz ~ 65535Hz |

### 핀 연결

| 부저 핀 | STM32 핀 | 설명 |
|---------|----------|------|
| + (양극) | TIM3_CH1 (PA6 또는 PB4) | PWM 출력 |
| - (음극) | GND | 접지 |

> 능동 부저(Active Buzzer)는 사용 불가. 반드시 **수동 부저** 사용.

### CubeMX 설정

```
TIM3:
  Clock Source: Internal Clock
  Channel 1: PWM Generation CH1
  Prescaler: 1279  → Timer clock = 84MHz / 1280 = 1MHz (1μs 해상도)
  Counter Period (ARR): 초기값 임의 (코드에서 동적 변경)
  Pulse (CCR1): ARR/2 (50% 듀티)
```

### 주파수-ARR 관계

```c
// ARR = 1,000,000 / frequency - 1
// 예시:
// 440Hz (A4음) → ARR = 1000000/440 - 1 ≈ 2271
// 1000Hz       → ARR = 999
// 4186Hz (C8)  → ARR ≈ 238
```

### 사전 정의 멜로디

```c
void Buzzer_PlayMario(void);   // 슈퍼마리오 테마
void Buzzer_PlayAlert(void);   // 경고음 (3회 반복)
void Buzzer_PlayElise(void);   // 엘리제를 위하여
void Buzzer_PlayReset(void);   // 리셋 완료음
void Buzzer_PlayStop(void);    // 정지음
void Buzzer_PlayStart(void);   // 출발음
void Buzzer_PlayOk(void);      // OK 확인음
void Buzzer_PlayPing(void);    // 아이들 알림음
```

### 커스텀 멜로디 정의 방법

```c
// BuzzerNote_t 구조체 배열로 정의
// {주파수(Hz), 지속시간(ms)}
// REST(0Hz)로 쉼표, {0, 0}으로 종료

static const BuzzerNote_t my_melody[] = {
    {NOTE_C6, 200},   // 도
    {NOTE_E6, 200},   // 미
    {NOTE_G6, 200},   // 솔
    {REST,    100},   // 쉼표
    {NOTE_C7, 400},   // 높은 도
    {REST, 0}         // 종료 (필수!)
};

// 재생:
Buzzer_PlayMelody(my_melody,
    sizeof(my_melody) / sizeof(my_melody[0]));
```

### 주요 API

```c
void    Buzzer_Init(TIM_HandleTypeDef *htim, uint32_t channel);
void    Buzzer_PlayTone(uint16_t freq, uint16_t duration);  // 블로킹
void    Buzzer_PlayMelody(const BuzzerNote_t *melody, uint16_t length); // 논블로킹
void    Buzzer_Stop(void);
uint8_t Buzzer_IsPlaying(void);
void    Buzzer_Update(void);  // 메인 루프에서 반드시 호출
```

### 메인 루프 필수 호출

```c
// Buzzer_PlayMelody는 논블로킹 → 반드시 Update 호출 필요
while (1) {
    Buzzer_Update();   // ← 없으면 첫 음만 재생되고 멈춤
}
```

### 신호 확인 방법

```
오실로스코프:
  - TIM3 출력 핀에서 PWM 파형 확인
  - 음표 재생 중: 50% 듀티 가변 주파수
  - 쉼표 구간: 파형 없음 (Low)
  - 음표 간격: 30ms 묵음 구간 확인
```

---

## ⚙️ 3. DC 모터 (Motor)

### 하드웨어 사양

| 항목 | 내용 |
|------|------|
| 모터 드라이버 | L298N 듀얼 H-브리지 |
| 제어 모터 수 | 4개 (RF, RB, LF, LB) |
| 제어 방식 | GPIO 방향 제어 (PWM 속도 제어 미포함) |
| 전원 | 모터: 6~12V / 로직: 5V |

### 핀 연결 (robot_config.h 설정 기준)

| 모터 | 신호 | STM32 핀 | L298N 핀 |
|------|------|----------|----------|
| RF (우전) | FORWARD | 설정파일 참조 | IN1 |
| RF (우전) | BACKWARD | 설정파일 참조 | IN2 |
| RB (우후) | FORWARD | 설정파일 참조 | IN3 |
| RB (우후) | BACKWARD | 설정파일 참조 | IN4 |
| LF (좌전) | FORWARD | 설정파일 참조 | IN1 |
| LF (좌전) | BACKWARD | 설정파일 참조 | IN2 |
| LB (좌후) | FORWARD | 설정파일 참조 | IN3 |
| LB (좌후) | BACKWARD | 설정파일 참조 | IN4 |

> 실제 핀 번호는 `robot_config.h`의 `MOTOR_*_PORT`, `MOTOR_*_PIN` 매크로로 정의

### 방향 제어 진리표

| 함수 | RF | RB | LF | LB | 동작 |
|------|----|----|----|-----|------|
| `Motor_Forward()` | FWD | FWD | FWD | FWD | 전진 |
| `Motor_Backward()` | BWD | BWD | BWD | BWD | 후진 |
| `Motor_Right()` | BWD | BWD | FWD | FWD | 우회전 (제자리) |
| `Motor_Left()` | FWD | FWD | BWD | BWD | 좌회전 (제자리) |
| `Motor_Stop()` | STOP | STOP | STOP | STOP | 정지 |

### 주요 API

```c
void Motor_Init(void);      // 초기화 (정지 상태)
void Motor_Forward(void);   // 전진
void Motor_Backward(void);  // 후진
void Motor_Right(void);     // 우회전
void Motor_Left(void);      // 좌회전
void Motor_Stop(void);      // 정지
```

### 신호 확인 방법

```
멀티미터 / 오실로스코프:
  - 전진 시: RF_F=HIGH, RF_B=LOW (모든 채널 동일)
  - 후진 시: RF_F=LOW,  RF_B=HIGH
  - 정지 시: RF_F=LOW,  RF_B=LOW

L298N 출력 확인:
  - OUT1/OUT2 차동전압: 전진 시 +Vmotor, 후진 시 -Vmotor
  - 전류 클램프로 모터 소비전류 확인 (무부하: ~200mA, 스톨: ~1A)
```

---

## 🔄 4. 서보모터 (Servo)

### 하드웨어 사양

| 항목 | 내용 |
|------|------|
| 모델 | SG90 마이크로 서보 |
| 동작 전압 | 4.8V ~ 6V |
| 제어 방식 | PWM (50Hz, 20ms 주기) |
| 각도 범위 | 0° ~ 180° |
| 펄스 폭 | 500μs (0°) ~ 2500μs (180°) |
| 중립 위치 | 1500μs (90°) |

### 핀 연결

| SG90 핀 | 색상 | STM32 핀 | 설명 |
|---------|------|----------|------|
| 신호 | 황색/흰색 | TIM_CHx 출력 | PWM 신호 |
| VCC | 적색 | 5V | 전원 |
| GND | 갈색/흑색 | GND | 접지 |

### CubeMX 설정

```
TIM (서보용, 예: TIM1 또는 TIM4):
  Clock Source: Internal Clock
  Channel: PWM Generation
  Prescaler: 71  → 72MHz / 72 = 1MHz (1μs 해상도)
  Counter Period (ARR): 19999  → 20ms 주기 (50Hz)
  Pulse: 1500  → 초기 중립 위치
```

### 각도-펄스 변환

```c
// pulse(μs) = 500 + angle × (2500 - 500) / 180
// angle = 0   → pulse = 500  μs (0.5ms)
// angle = 90  → pulse = 1500 μs (1.5ms)  ← 중립
// angle = 180 → pulse = 2500 μs (2.5ms)
```

### 주요 API

```c
void Servo_Init(TIM_HandleTypeDef *htim, uint32_t channel);
// 예: Servo_Init(&htim4, TIM_CHANNEL_1);

void Servo_SetAngle(uint8_t angle);
// angle: 0 ~ 180 (범위 초과 시 자동 클리핑)
```

### 사용 예시

```c
Servo_Init(&htim4, TIM_CHANNEL_1);

Servo_SetAngle(0);     // 최좌
HAL_Delay(500);
Servo_SetAngle(90);    // 중립
HAL_Delay(500);
Servo_SetAngle(180);   // 최우
```

### 신호 확인 방법

```
오실로스코프:
  - 주기: 20ms (50Hz) 확인
  - 펄스 폭: 각도 0° → 0.5ms, 90° → 1.5ms, 180° → 2.5ms
  - 논리 레벨: 3.3V (STM32 출력)

주의: SG90은 3.3V 신호에서 동작하나 VCC는 반드시 5V 공급
      서보 전원은 STM32 3.3V 핀이 아닌 외부 5V 전원 사용 권장
```

---

## 📡 5. 초음파 센서 (Ultrasonic)

### 하드웨어 사양

| 항목 | 내용 |
|------|------|
| 모델 | HC-SR04 |
| 동작 전압 | 5V |
| 측정 범위 | 2cm ~ 400cm |
| 측정 각도 | ±15° |
| 분해능 | 0.3cm |
| 제어 방식 | Trigger 펄스 → Echo 폭 측정 (Input Capture) |

### 핀 연결

| HC-SR04 핀 | STM32 핀 | 설명 |
|-----------|----------|------|
| VCC | 5V | 전원 |
| GND | GND | 접지 |
| TRIG | PA0 | 트리거 GPIO 출력 |
| ECHO | PA1 (TIM2_CH2) | 에코 Input Capture |

> **주의**: HC-SR04 ECHO 출력은 5V 레벨 → 저항 분압(10kΩ/20kΩ) 또는 레벨 시프터 필수

### CubeMX 설정

```
GPIO:
  PA0: GPIO_Output (TRIG)

TIM2:
  Clock Source: Internal Clock
  Channel 2: Input Capture Direct Mode
  Prescaler: 71  → 1MHz (1μs 해상도)
  Counter Period: 65535
  NVIC: TIM2 global interrupt 활성화
```

### 거리 계산 원리

```
거리(cm) = 에코 펄스 폭(μs) / 58
           (음속 340m/s 기준, 왕복 시간 고려)

유효 범위 필터: 100μs < diff < 30000μs
  - 100μs 미만: 노이즈
  - 30000μs 초과: 측정 범위 초과 (약 5m)
```

### 주요 API

```c
void     Ultrasonic_Init(void);               // 타이머 Input Capture 시작
void     Ultrasonic_Trigger(void);            // 트리거 펄스 발사
uint16_t Ultrasonic_Read(void);              // 거리 반환 (cm), 미준비 시 999
void     Ultrasonic_IC_Callback(
             TIM_HandleTypeDef *htim);        // HAL IC 콜백에서 호출
```

### HAL 콜백 연결

```c
// stm32f1xx_it.c 또는 main.c에 추가:
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        Ultrasonic_IC_Callback(htim);
    }
}
```

### 사용 예시 (메인 루프 논블로킹)

```c
Ultrasonic_Init();

uint32_t last_trig = 0;

while (1) {
    // 60ms마다 트리거
    if (HAL_GetTick() - last_trig >= 60) {
        Ultrasonic_Trigger();
        last_trig = HAL_GetTick();
    }

    uint16_t dist = Ultrasonic_Read();
    if (dist != 999) {
        // 유효한 거리 값 처리
        if (dist < 20) {
            Motor_Stop();
        }
    }
}
```

### 신호 확인 방법

```
오실로스코프 2채널 동시 측정:
  CH1 - TRIG (PA0): 10μs HIGH 펄스 확인
  CH2 - ECHO (PA1): TRIG 후 HIGH 펄스 폭 확인

예시 측정:
  10cm → ECHO 폭 ≈ 580μs
  50cm → ECHO 폭 ≈ 2900μs
 100cm → ECHO 폭 ≈ 5800μs
```

---

## 💡 6. RGB LED

### 하드웨어 사양

| 항목 | 내용 |
|------|------|
| 타입 | 공통 음극(Common Cathode) RGB LED |
| 제어 방식 | GPIO ON/OFF (PWM 미사용) |
| 포트 | GPIOC |

### 핀 연결

| LED 핀 | STM32 핀 | 설명 |
|--------|----------|------|
| R (적색) | PC (RED_Pin) | 적색 채널 |
| G (녹색) | PC (GREEN_Pin) | 녹색 채널 |
| B (청색) | PC (BLUE_Pin) | 청색 채널 |
| 공통 음극 | GND (저항 경유) | 전류 제한 저항 330Ω 권장 |

> `RED_Pin`, `GREEN_Pin`, `BLUE_Pin`은 CubeMX에서 User Label로 정의

### 색상 코드

| 상수 | 값 | R | G | B | 색상 |
|------|-----|---|---|---|------|
| `RGB_COLOR_GREEN` | 0 | OFF | ON | OFF | 🟢 녹색 |
| `RGB_COLOR_RED` | 1 | ON | OFF | OFF | 🔴 적색 |
| `RGB_COLOR_ORANGE` | 2 | ON | ON | OFF | 🟠 주황색 |

### 주요 API

```c
void RGB_Init(void);          // 초기화 (전체 OFF)
void RGB_Set(int color);      // 색상 설정 (RGB_COLOR_* 상수 사용)
void RGB_Off(void);           // 전체 소등
```

### 사용 예시

```c
RGB_Init();

RGB_Set(RGB_COLOR_GREEN);   // 정상 동작: 녹색
HAL_Delay(1000);

RGB_Set(RGB_COLOR_RED);     // 장애물 감지: 적색
HAL_Delay(1000);

RGB_Set(RGB_COLOR_ORANGE);  // 경고: 주황색
HAL_Delay(1000);

RGB_Off();                  // 소등
```

---

## 🔗 7. 메인 루프 통합 예시

```c
/* main.c */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_SPI2_Init();
    MX_TIM2_Init();
    MX_TIM3_Init();
    MX_TIM4_Init();

    /* 드라이버 초기화 */
    LCD_Init();
    LCD_Clear(0x0000);
    Anim_Init();

    Buzzer_Init(&htim3, TIM_CHANNEL_1);
    Servo_Init(&htim4, TIM_CHANNEL_1);
    Motor_Init();
    RGB_Init();
    Ultrasonic_Init();

    /* 시작 연출 */
    Buzzer_PlayStart();
    Anim_Set(EXPR_HAPPY);
    RGB_Set(RGB_COLOR_GREEN);

    uint32_t last_trig = 0;

    while (1)
    {
        /* 논블로킹 업데이트 */
        Anim_Update();     // LCD 눈 표정 + 깜빡임
        Buzzer_Update();   // 멜로디 진행

        /* 초음파 60ms 주기 */
        if (HAL_GetTick() - last_trig >= 60)
        {
            Ultrasonic_Trigger();
            last_trig = HAL_GetTick();
        }

        uint16_t dist = Ultrasonic_Read();
        if (dist < 20 && dist != 999)
        {
            Motor_Stop();
            RGB_Set(RGB_COLOR_RED);
            Anim_Set(EXPR_ANGRY);
            if (!Buzzer_IsPlaying())
                Buzzer_PlayAlert();
        }
    }
}
```

---

## ⚠️ 주의사항 및 트러블슈팅

### LCD 화면이 안 나올 때

```
1. SPI2 핀 확인 (PB13/PB15/PB12/PA8/PD2)
2. Y_OFFSET 값 조정 (모듈에 따라 24~26 차이)
3. MADCTL 값 확인 (0x60: 가로, 0x00: 세로)
4. LCD_Init() 중복 호출 여부 확인
```

### 부저 소리가 안 날 때

```
1. 능동 부저(Active) vs 수동 부저(Passive) 확인
2. TIM Prescaler 계산 재확인 (1MHz 기준)
3. Buzzer_Update()가 메인 루프에 있는지 확인
4. HAL_TIM_PWM_Start() 호출 여부 확인
```

### 초음파 거리가 999만 반환될 때

```
1. ECHO 핀 레벨 시프터(5V→3.3V) 확인
2. HAL_TIM_IC_CaptureCallback 연결 확인
3. TIM2 NVIC 인터럽트 활성화 여부 확인
4. Ultrasonic_Trigger() 호출 주기 확인 (최소 60ms)
```

### 서보가 떨릴 때

```
1. VCC 5V 공급 여부 확인 (3.3V 부족)
2. ARR = 19999 (20ms 주기) 정확히 설정됐는지 확인
3. 서보 전원과 STM32 GND 공통 연결 확인
4. servo_moving 플래그 사용 시 LCD 갱신 억제 확인
```

---

## 📋 개발 환경

| 항목 | 내용 |
|------|------|
| MCU | STM32F103C8T6 (또는 F103RET6) |
| IDE | STM32CubeIDE 1.13+ |
| HAL 라이브러리 | STM32CubeF1 v1.8.x |
| 시스템 클럭 | 72MHz (HSE + PLL) |
| 디버거 | ST-Link V2 |

---

## 📄 라이선스

MIT License — 교육 및 개인 프로젝트 자유 사용 가능

---

*광주인력개발원 임베디드시스템 과정 실습 프로젝트*
