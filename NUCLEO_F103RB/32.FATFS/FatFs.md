# FatFs

### 1. 구현기능
  * SD 카드 초기화 및 인식 (SDv1, SDv2, MMC)
  * FAT32 파일시스템 마운트
  * 파일 생성, 읽기, 쓰기, 삭제
  * 디렉토리 생성 및 파일 목록
  * 파일 append (추가 쓰기)
  * 데이터 로깅 (타임스탬프 포함)
  * CSV 파일 생성 및 데이터 추가
  * SD 카드 정보 표시
  * 통계 정보 (읽기/쓰기 바이트)

## 2. 파일구조
```
SD_FATFS/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   └── sd_spi_driver.h        ✅
│   └── Src/
│       ├── main.c                  ✅ (최종 버전)
│       └── sd_spi_driver.c         ✅ (깔끔한 버전)
│
├── FATFS/
│   ├── App/
│   │   ├── fatfs.c
│   │   └── fatfs.h
│   └── Target/
│       └── user_diskio.c           ✅ (깔끔한 버전)
│
└── Middlewares/
    └── Third_Party/
        └── FatFs/
            └── src/
                ├── ff_gen_drv.h    ✅ (수정됨)
                └── ffconf.h        ✅ (수정됨)
```

### 3. 하드웨어 연결
   * SD 카드를 SPI 모드로 연결합니다:
   * SD Card Pin  →  STM32F103 Pin
```
SD Card Module → NUCLEO-F103RB
VCC  → 3.3V
GND  → GND
MISO → PA6 (SPI1_MISO)
MOSI → PA7 (SPI1_MOSI)
SCK  → PA5 (SPI1_SCK)
CS   → PB6 (GPIO)
```
#### 3.1. SD 카드 준비
   * 포맷: FAT32
   * 할당 단위: 4096 bytes
   * 용량: 32GB 이하 권장

#### 3.2. 프로그램 업로드
   * 프로젝트 빌드
   * NUCLEO 보드에 업로드
   * UART 출력 확인 (115200 baud)

#### 3.3. 결과 확인
   * LED 깜빡임: 성공 (1초 간격)
   * UART 출력: 상세 로그
   * SD 카드: 생성된 파일 확인




### 2. STM32CubeMX 설정

- SPI 설정
c1. Connectivity → SPI1 선택
  - 2. Mode: Full-Duplex Master
  - 3. Hardware NSS Signal: Disable
  - 4. Parameter Settings:
    - Prescaler: 128 (초기 통신용, 나중에 속도 증가 가능)
    - Clock Polarity: Low
    - Clock Phase: 1 Edge
    - Data Size: 8 Bits
    - First Bit: MSB First
- FatFs 미들웨어 활성화
  - 1. Middleware → FATFS 선택
  - 2. Mode: User-defined
  - 3. Configuration:
    - USE_LFN (Long File Name): Enabled with dynamic allocation
    - MAX_SS (Maximum Sector Size): 512

- GPIO 설정
  - 1. PA4를 GPIO_Output으로 설정 (CS Pin)
  - 2. User Label: SD_CS

