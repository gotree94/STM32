# FatFs

### 1. 하드웨어 연결
   * SD 카드를 SPI 모드로 연결합니다:
   * SD Card Pin  →  STM32F103 Pin
```
-----------------------------------
CS (Chip Select) → PA4 (SPI1_NSS)
SCK (Clock)      → PA5 (SPI1_SCK)
MOSI (Data In)   → PA7 (SPI1_MOSI)
MISO (Data Out)  → PA6 (SPI1_MISO)
VCC              → 3.3V
GND              → GND
```

### 2. STM32CubeMX 설정

   * SPI 설정
       1. Connectivity → SPI1 선택
       2. Mode: Full-Duplex Master
       3. Hardware NSS Signal: Disable
       4. Parameter Settings:
   * Prescaler: 128 (초기 통신용, 나중에 속도 증가 가능)
   * Clock Polarity: Low
   * Clock Phase: 1 Edge
   * Data Size: 8 Bits
   * First Bit: MSB First

   * FatFs 미들웨어 활성화
     1. Middleware → FATFS 선택
     2. Mode: User-defined
     3. Configuration:
   * USE_LFN (Long File Name): Enabled with dynamic allocation
   * MAX_SS (Maximum Sector Size): 512

   * GPIO 설정
   1. PA4를 GPIO_Output으로 설정 (CS Pin)
   2. User Label: SD_CS

