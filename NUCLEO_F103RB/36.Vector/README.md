# Vector Robot Eye Animation Projects for STM32F103

로봇 눈 애니메이션을 위한 STM32F103 기반 LCD 프로젝트 모음입니다.

---

## 📁 프로젝트 구성

### 1. ST7735S SPI (0.96" 160x80 컬러 LCD)
**폴더:** `01_ST7735S_SPI_160x80/`

| 항목 | 내용 |
|------|------|
| LCD | ST7735S 0.96" TFT (160x80, 16-bit color) |
| 인터페이스 | SPI (4-wire) |
| 핀 연결 | PA5(SCK), PA7(MOSI), PA1(RES), PA6(DC), PB6(CS) |
| 특징 | 프레임버퍼 방식, 부분 업데이트로 고속 |

**CubeMX 설정:**
- SPI1: Transmit Only Master, Prescaler=4
- GPIO Output: PA1, PA6, PB6
- Clock: 64MHz

<img width="945" height="920" alt="Vector_eyes" src="https://github.com/user-attachments/assets/53959064-5ec3-40ef-a595-a55f5119c4d2" />

![LCD2-SPI](https://github.com/user-attachments/assets/965a8470-b695-40bb-bb92-f692bc773d2d)


---

### 2. SSD1306 I2C (0.96" 128x64 OLED)
**폴더:** `02_SSD1306_I2C_128x64/`

| 항목 | 내용 |
|------|------|
| LCD | SSD1306 0.96" OLED (128x64, 단색) |
| 인터페이스 | I2C (400kHz) |
| 핀 연결 | PB6(SCL), PB7(SDA) |
| I2C 주소 | 0x3C (또는 0x3D) |
| 특징 | 1KB 프레임버퍼, 매우 적은 핀 사용 |

**CubeMX 설정:**
- I2C1: Fast Mode 400kHz
- Clock: 64MHz

**주의:** I2C 주소가 안 맞으면 `SSD1306_I2C_ADDR`를 `0x7A`로 변경

<img width="1006" height="1013" alt="Vector_eyes_i2c" src="https://github.com/user-attachments/assets/1ae23ead-18a3-42db-9aa8-1aeb8c7b910b" />

![LCD2-I2C](https://github.com/user-attachments/assets/0db133eb-f22b-4fc4-b455-8bbb33c05f58)


---

### 3. ILI9341 Parallel (2.8" 240x320 컬러 LCD) - 고속 버전
**폴더:** `03_ILI9341_Parallel_240x320/`

| 항목 | 내용 |
|------|------|
| LCD | ILI9341 2.8" TFT (240x320, 16-bit color) |
| 인터페이스 | 8-bit Parallel |
| 핀 연결 | D0-D7 (분산), PA0(RD), PA1(WR), PA4(RS), PB0(CS), PC1(RST) |
| 특징 | GPIO 레지스터 직접 접근, 초고속 |

**데이터 핀 배치:**
```
D0 = PA9    D4 = PB5
D1 = PC7    D5 = PB4
D2 = PA10   D6 = PB10
D3 = PB3    D7 = PA8
```

**CubeMX 설정:**
- GPIO Output (High Speed): 모든 LCD 핀
- Clock: 64MHz

---

### 4. ILI9341 Touch Project (원본)
**폴더:** `04_ILI9341_Touch_Project/`

| 파일 | 설명 |
|------|------|
| main.c | 터치 데모 (캘리브레이션, 버튼, 페인트) |
| ili9341.c | LCD 드라이버 |
| Ili9341.h | LCD 헤더 |

**참고:** 터치 기능 사용시 `touch_adc.c/h` 필요

---

## 🎭 지원 표정 (모든 버전 공통)

| 표정 | 설명 |
|------|------|
| NORMAL | 일반 (둥근 사각형 눈) |
| HAPPY | 행복 (^ ^ 모양) |
| SAD | 슬픔 (처진 눈썹) |
| ANGRY | 화남 (찡그린 눈썹) |
| SURPRISED | 놀람 (큰 동그란 눈) |
| SLEEPY | 졸림 (반쯤 감은 눈) |
| WINK_LEFT | 왼쪽 윙크 |
| WINK_RIGHT | 오른쪽 윙크 |
| BLINK | 깜빡임 |
| LOVE | 하트 눈 |
| DIZZY | 어지러움 (X X 눈) |
| LOOK_LEFT | 왼쪽 보기 |
| LOOK_RIGHT | 오른쪽 보기 |
| LOOK_UP | 위 보기 |
| LOOK_DOWN | 아래 보기 |


<img width="1268" height="1194" alt="Vector_ili9341" src="https://github.com/user-attachments/assets/dd46cf44-fbfb-4fe1-b929-d7dd6334df94" />

![LCD2-1](https://github.com/user-attachments/assets/7632225a-6112-48a9-a988-f198e04e598b)


---

## 🚀 사용 방법

### 데모 모드 (모든 표정 순환)
```c
while(1) {
    Anim_Demo();
}
```

### Idle 모드 (자연스러운 깜빡임)
```c
while(1) {
    Anim_Idle();
    HAL_Delay(30);
}
```

### 수동 표정 제어
```c
Anim_SetExpr(EXPR_HAPPY);   // 행복한 표정
HAL_Delay(1000);
Anim_Blink();                // 깜빡임
Anim_SetExpr(EXPR_NORMAL);  // 일반으로 복귀
```

---

## ⚙️ 눈 위치 조정

각 프로젝트의 main.c에서 다음 값을 조정:

```c
#define EYE_AREA_Y      20      // 눈 영역 Y 위치 (아래로: 증가)
#define LX              40      // 왼쪽 눈 X 위치
#define RX              100     // 오른쪽 눈 X 위치
#define CY              30      // 눈 중심 Y 위치
```

---

## 📊 성능 비교

| 프로젝트 | 해상도 | 색상 | 프레임버퍼 | 속도 |
|---------|--------|------|-----------|------|
| ST7735S SPI | 160x80 | 16-bit | 14KB | ★★★☆☆ |
| SSD1306 I2C | 128x64 | 1-bit | 1KB | ★★☆☆☆ |
| ILI9341 Parallel | 240x320 | 16-bit | 부분 | ★★★★★ |

---

