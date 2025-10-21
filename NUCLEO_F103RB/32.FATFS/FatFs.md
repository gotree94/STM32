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

<img width="529" height="849" alt="001" src="https://github.com/user-attachments/assets/e8964734-5bda-4d01-8458-94e3eab54530" />



### 2. STM32CubeMX 설정

- SPI 설정
  - 1. Connectivity → SPI1 선택 (PA5 LD reset)
  - 2. Mode: Full-Duplex Master
  - 3. Hardware NSS Signal: Disable
  - 4. Parameter Settings:
    - Prescaler: 8 (초기 통신용, 나중에 속도 증가 가능)
    - Clock Polarity(CPOL): Low
    - Clock Phase(CPHA): 1 Edge
    - Data Size: 8 Bits
    - First Bit: MSB First
  - 5. PB6 : SD_CS Gpio_output
    - GPIO output level : High
    - GPIO mode : Output Push Pull
    - GPIO Pull-up/Pull-down : Pull-up
    - Maximum output speed : Medium
    - User Label : SD_CS
- FatFs 미들웨어 활성화
  - 1. Middleware → FATFS 선택
  - 2. Mode: User-defined
  - 3. Configuration:
    - CODE_PAGE : Multilingual Latin 1 (OEM)
    - USE_LFN (Long File Name): Enabled with static working buffer on the BSS
    - MAX_SS (Maximum Sector Size): 4096
    - MIN_SS (Minimum Sector Size): 512
    - FS_NORTC : Fixed timestamp (Dynamic은 RTC 필요)

<img width="731" height="714" alt="002" src="https://github.com/user-attachments/assets/520e793c-e3f2-4294-880e-72393cecaf46" />


<img width="1437" height="855" alt="003" src="https://github.com/user-attachments/assets/a0126452-5415-4b07-8c0b-ee94d5c214d0" />





