# LCD-I2C

<img width="600" height="400" alt="Sheild-001" src="https://github.com/user-attachments/assets/9df5b8c3-d81a-4026-9f86-67fa4dde1e38" />
<br>

<img width="600" height="400" alt="Sheild-002" src="https://github.com/user-attachments/assets/29bdd8e8-3e94-45da-87f4-426694f32622" />
<br>

<img width="600" height="600" alt="F103RB-pin" src="https://github.com/user-attachments/assets/45bb557f-9517-419d-b45c-81a92869bac0" />
<br>

## 전체 프로젝트 구성:

### 1. main.c (위 코드)
STM32 CubeIDE에서 프로젝트의 main.c 파일을 위 코드로 교체하세요.

### 2. 추가로 필요한 설정:
 * CubeMX에서 설정할 사항:
 * System Clock: HSE 8MHz → PLL × 8 = 64MHz
    * SPI1 활성화:
       * PA5 (SPI1_SCK)
       * PA7 (SPI1_MOSI)
   * GPIO 출력핀 설정:
      * PA1 (LCD_RES)
      * PA6 (LCD_DC)
      * PB6 (LCD_CS)

### 3. 하드웨어 연결:
   * LCD 핀    →  STM32F103 핀
   * BLK      →  NC (연결 안함)
   * CS       →  PB6
   * DC       →  PA6
   * RES      →  PA1
   * SDA(MOSI)→  PA7
   * SCL(SCK) →  PA5
   * VCC      →  3.3V
   * GND      →  GND

### 4. 코드 특징:
   * 완전한 ST7735S 초기화 시퀀스
   * 8x8 폰트 포함 (ASCII 32-126)
   * RGB565 컬러 지원
   * 텍스트 출력 기능
   * 64MHz 시스템 클럭 설정

### 5. 실행 결과:
   * LCD에 다음과 같이 표시됩니다:
      * "Hello" (흰색)
      * "World!" (흰색)
      * "STM32F103" (녹색)
      * "ST7735S" (시안색)
      * "80x160" (노란색)

