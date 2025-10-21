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

<img width="240" height="819" alt="004" src="https://github.com/user-attachments/assets/424900c3-1df0-4c2d-b67d-c28a1877be82" />

---

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

<img width="1437" height="855" alt="003" src="https://github.com/user-attachments/assets/a0126452-5415-4b07-8c0b-ee94d5c214d0" />
<br>

<img width="731" height="714" alt="002" src="https://github.com/user-attachments/assets/520e793c-e3f2-4294-880e-72393cecaf46" />
<br>

---

### 1. sd_spi_driver.h

```c
/**
  ******************************************************************************
  * @file    sd_spi_driver.h
  * @brief   SD Card SPI driver header
  * @note    Core/Inc 폴더에 추가하세요
  ******************************************************************************
  */

#ifndef __SD_SPI_DRIVER_H
#define __SD_SPI_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/* SD Card Commands */
#define SD_CMD0     0x40  /* GO_IDLE_STATE */
#define SD_CMD1     0x41  /* SEND_OP_COND (MMC) */
#define SD_CMD8     0x48  /* SEND_IF_COND */
#define SD_CMD9     0x49  /* SEND_CSD */
#define SD_CMD10    0x4A  /* SEND_CID */
#define SD_CMD12    0x4C  /* STOP_TRANSMISSION */
#define SD_CMD16    0x50  /* SET_BLOCKLEN */
#define SD_CMD17    0x51  /* READ_SINGLE_BLOCK */
#define SD_CMD18    0x52  /* READ_MULTIPLE_BLOCK */
#define SD_CMD23    0x57  /* SET_BLOCK_COUNT (MMC) */
#define SD_CMD24    0x58  /* WRITE_BLOCK */
#define SD_CMD25    0x59  /* WRITE_MULTIPLE_BLOCK */
#define SD_CMD41    0x69  /* SEND_OP_COND (SDC) */
#define SD_CMD55    0x77  /* APP_CMD */
#define SD_CMD58    0x7A  /* READ_OCR */

/* Card type flags */
#define CT_MMC      0x01  /* MMC ver 3 */
#define CT_SD1      0x02  /* SD ver 1 */
#define CT_SD2      0x04  /* SD ver 2 */
#define CT_SDC      (CT_SD1|CT_SD2)
#define CT_BLOCK    0x08  /* Block addressing */

/* Function Prototypes */
uint8_t SD_SPI_Init(void);
uint8_t SD_SPI_ReadBlock(uint8_t *buf, uint32_t sector);
uint8_t SD_SPI_WriteBlock(const uint8_t *buf, uint32_t sector);
uint8_t SD_SPI_ReadMultiBlock(uint8_t *buf, uint32_t sector, uint32_t count);
uint8_t SD_SPI_WriteMultiBlock(const uint8_t *buf, uint32_t sector, uint32_t count);
uint8_t SD_SPI_GetCardInfo(void);

#ifdef __cplusplus
}
#endif

#endif /* __SD_SPI_DRIVER_H */

```

### 1. sd_spi_driver.c

```c
/**
  ******************************************************************************
  * @file    sd_spi_driver.c (Clean Version - No Debug Messages)
  * @brief   SD Card SPI driver - Production version
  ******************************************************************************
  */

#include "sd_spi_driver.h"
#include "main.h"

/* External SPI handle */
extern SPI_HandleTypeDef hspi1;

/* CS Pin Control Macros */
#define SD_CS_LOW()     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET)
#define SD_CS_HIGH()    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET)

/* Private variables */
static uint8_t CardType = 0;

/* Private function prototypes */
static uint8_t SD_SendCommand(uint8_t cmd, uint32_t arg);
static uint8_t SD_WaitReady(uint32_t timeout);
static void SD_PowerOn(void);
static uint8_t SD_RxByte(void);
static void SD_TxByte(uint8_t data);
static void SD_SetSpeed(uint8_t speed);

/**
  * @brief  SPI Transmit single byte
  */
static void SD_TxByte(uint8_t data)
{
    HAL_SPI_Transmit(&hspi1, &data, 1, 100);
}

/**
  * @brief  SPI Receive single byte
  */
static uint8_t SD_RxByte(void)
{
    uint8_t dummy = 0xFF, data;
    HAL_SPI_TransmitReceive(&hspi1, &dummy, &data, 1, 100);
    return data;
}

/**
  * @brief  Set SPI speed
  */
static void SD_SetSpeed(uint8_t speed)
{
    if (speed == 0) {
        hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
    } else {
        hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    }
    HAL_SPI_Init(&hspi1);
}

/**
  * @brief  Power on the SD card
  */
static void SD_PowerOn(void)
{
    SD_CS_HIGH();
    for (uint8_t i = 0; i < 10; i++) {
        SD_TxByte(0xFF);
    }
}

/**
  * @brief  Wait for card ready
  */
static uint8_t SD_WaitReady(uint32_t timeout)
{
    uint8_t res;
    uint32_t start = HAL_GetTick();

    SD_RxByte();
    do {
        res = SD_RxByte();
        if (res == 0xFF) return 0xFF;
    } while ((HAL_GetTick() - start) < timeout);

    return res;
}

/**
  * @brief  Send command to SD card
  */
static uint8_t SD_SendCommand(uint8_t cmd, uint32_t arg)
{
    uint8_t res, crc = 0x01;

    if (cmd & 0x80) {
        cmd &= 0x7F;
        res = SD_SendCommand(SD_CMD55, 0);
        if (res > 1) return res;
    }

    SD_CS_HIGH();
    SD_RxByte();
    SD_CS_LOW();
    SD_RxByte();

    if (SD_WaitReady(500) != 0xFF) {
        SD_CS_HIGH();
        return 0xFF;
    }

    SD_TxByte(cmd);
    SD_TxByte((uint8_t)(arg >> 24));
    SD_TxByte((uint8_t)(arg >> 16));
    SD_TxByte((uint8_t)(arg >> 8));
    SD_TxByte((uint8_t)arg);

    if (cmd == SD_CMD0) crc = 0x95;
    if (cmd == SD_CMD8) crc = 0x87;
    SD_TxByte(crc);

    if (cmd == SD_CMD12) SD_RxByte();

    uint8_t n = 10;
    do {
        res = SD_RxByte();
    } while ((res & 0x80) && --n);

    return res;
}

/**
  * @brief  Initialize SD card
  */
uint8_t SD_SPI_Init(void)
{
    uint8_t n, cmd, ty = 0;
    uint8_t ocr[4];

    SD_SetSpeed(0);
    SD_PowerOn();

    if (SD_SendCommand(SD_CMD0, 0) == 1) {
        if (SD_SendCommand(SD_CMD8, 0x1AA) == 1) {
            /* SDv2 */
            for (n = 0; n < 4; n++) ocr[n] = SD_RxByte();

            if (ocr[2] == 0x01 && ocr[3] == 0xAA) {
                uint32_t start = HAL_GetTick();
                while ((HAL_GetTick() - start) < 1000) {
                    if (SD_SendCommand(SD_CMD41 | 0x80, 1UL << 30) == 0) break;
                    HAL_Delay(10);
                }

                if ((HAL_GetTick() - start) < 1000 && SD_SendCommand(SD_CMD58, 0) == 0) {
                    for (n = 0; n < 4; n++) ocr[n] = SD_RxByte();
                    ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
                }
            }
        } else {
            /* SDv1 or MMC */
            if (SD_SendCommand(SD_CMD55 | 0x80, 0) <= 1) {
                ty = CT_SD1;
                cmd = SD_CMD41 | 0x80;
            } else {
                ty = CT_MMC;
                cmd = SD_CMD1;
            }

            uint32_t start = HAL_GetTick();
            while ((HAL_GetTick() - start) < 1000) {
                if (SD_SendCommand(cmd, 0) == 0) break;
                HAL_Delay(10);
            }

            if ((HAL_GetTick() - start) >= 1000 || SD_SendCommand(SD_CMD16, 512) != 0) {
                ty = 0;
            }
        }
    }

    CardType = ty;
    SD_CS_HIGH();
    SD_RxByte();

    if (ty) {
        SD_SetSpeed(1);
        return 0;
    }

    return 1;
}

/**
  * @brief  Read single block
  */
uint8_t SD_SPI_ReadBlock(uint8_t *buf, uint32_t sector)
{
    uint8_t result = 1;

    if (!(CardType & CT_BLOCK)) sector *= 512;

    if (SD_SendCommand(SD_CMD17, sector) == 0) {
        uint32_t timeout = 200;
        uint8_t token;

        do {
            token = SD_RxByte();
            if (token == 0xFE) break;
            HAL_Delay(1);
        } while (--timeout);

        if (token == 0xFE) {
            for (uint16_t i = 0; i < 512; i++) {
                buf[i] = SD_RxByte();
            }
            SD_RxByte();
            SD_RxByte();
            result = 0;
        }
    }

    SD_CS_HIGH();
    SD_RxByte();

    return result;
}

/**
  * @brief  Write single block
  */
uint8_t SD_SPI_WriteBlock(const uint8_t *buf, uint32_t sector)
{
    uint8_t result = 1;

    if (!(CardType & CT_BLOCK)) sector *= 512;

    if (SD_SendCommand(SD_CMD24, sector) == 0) {
        if (SD_WaitReady(500) == 0xFF) {
            SD_TxByte(0xFE);

            for (uint16_t i = 0; i < 512; i++) {
                SD_TxByte(buf[i]);
            }

            SD_TxByte(0xFF);
            SD_TxByte(0xFF);

            uint8_t resp = SD_RxByte();
            if ((resp & 0x1F) == 0x05) {
                if (SD_WaitReady(500) == 0xFF) {
                    result = 0;
                }
            }
        }
    }

    SD_CS_HIGH();
    SD_RxByte();

    return result;
}

/**
  * @brief  Read multiple blocks
  */
uint8_t SD_SPI_ReadMultiBlock(uint8_t *buf, uint32_t sector, uint32_t count)
{
    if (!(CardType & CT_BLOCK)) sector *= 512;

    if (SD_SendCommand(SD_CMD18, sector) == 0) {
        for (uint32_t i = 0; i < count; i++) {
            uint32_t timeout = 200;
            uint8_t token;

            do {
                token = SD_RxByte();
                if (token == 0xFE) break;
                HAL_Delay(1);
            } while (--timeout);

            if (token != 0xFE) {
                SD_SendCommand(SD_CMD12, 0);
                SD_CS_HIGH();
                return 1;
            }

            for (uint16_t j = 0; j < 512; j++) {
                buf[j] = SD_RxByte();
            }
            buf += 512;

            SD_RxByte();
            SD_RxByte();
        }

        SD_SendCommand(SD_CMD12, 0);
    }

    SD_CS_HIGH();
    SD_RxByte();

    return 0;
}

/**
  * @brief  Write multiple blocks
  */
uint8_t SD_SPI_WriteMultiBlock(const uint8_t *buf, uint32_t sector, uint32_t count)
{
    if (!(CardType & CT_BLOCK)) sector *= 512;

    if (CardType & CT_SDC) {
        SD_SendCommand(SD_CMD55 | 0x80, 0);
        SD_SendCommand(SD_CMD23, count);
    }

    if (SD_SendCommand(SD_CMD25, sector) == 0) {
        for (uint32_t i = 0; i < count; i++) {
            if (SD_WaitReady(500) != 0xFF) break;

            SD_TxByte(0xFC);

            for (uint16_t j = 0; j < 512; j++) {
                SD_TxByte(buf[j]);
            }
            buf += 512;

            SD_TxByte(0xFF);
            SD_TxByte(0xFF);

            uint8_t resp = SD_RxByte();
            if ((resp & 0x1F) != 0x05) break;
        }

        SD_TxByte(0xFD);
        SD_WaitReady(500);
    }

    SD_CS_HIGH();
    SD_RxByte();

    return 0;
}

/**
  * @brief  Get card info
  */
uint8_t SD_SPI_GetCardInfo(void)
{
    return CardType;
}

```


### user_disk_io.c

```c
/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * @file    user_diskio.c (Clean Version - No Debug Messages)
  * @brief   FATFS user diskio - Production version
  ******************************************************************************
  */
 /* USER CODE END Header */

#ifdef USE_OBSOLETE_USER_CODE_SECTION_0
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */
#endif

/* USER CODE BEGIN DECL */
#include <string.h>
#include "ff_gen_drv.h"
#include "sd_spi_driver.h"

/* Private variables */
static volatile DSTATUS Stat = STA_NOINIT;
static uint8_t CardInitialized = 0;

/* USER CODE END DECL */

/* Private function prototypes */
DSTATUS USER_initialize (BYTE pdrv);
DSTATUS USER_status (BYTE pdrv);
DRESULT USER_read (BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
#if _USE_WRITE == 1
  DRESULT USER_write (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
#endif
#if _USE_IOCTL == 1
  DRESULT USER_ioctl (BYTE pdrv, BYTE cmd, void *buff);
#endif

Diskio_drvTypeDef  USER_Driver =
{
  USER_initialize,
  USER_status,
  USER_read,
#if  _USE_WRITE
  USER_write,
#endif
#if  _USE_IOCTL == 1
  USER_ioctl,
#endif
};

/* Private functions */

/**
  * @brief  Initializes a Drive
  */
DSTATUS USER_initialize (BYTE pdrv)
{
  /* USER CODE BEGIN INIT */
    Stat = STA_NOINIT;

    if (pdrv != 0) {
        return STA_NOINIT;
    }

    if (CardInitialized) {
        Stat = 0;
        return Stat;
    }

    if (SD_SPI_Init() == 0) {
        Stat = 0;
        CardInitialized = 1;
    } else {
        Stat = STA_NOINIT;
        CardInitialized = 0;
    }

    return Stat;
  /* USER CODE END INIT */
}

/**
  * @brief  Gets Disk Status
  */
DSTATUS USER_status (BYTE pdrv)
{
  /* USER CODE BEGIN STATUS */
    if (pdrv != 0) {
        return STA_NOINIT;
    }

    if (CardInitialized && SD_SPI_GetCardInfo() != 0) {
        Stat = 0;
    } else {
        Stat = STA_NOINIT;
    }

    return Stat;
  /* USER CODE END STATUS */
}

/**
  * @brief  Reads Sector(s)
  */
DRESULT USER_read (BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
  /* USER CODE BEGIN READ */
    if (pdrv != 0 || !count) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;

    if (count == 1) {
        if (SD_SPI_ReadBlock(buff, sector) == 0) {
            return RES_OK;
        }
    } else {
        if (SD_SPI_ReadMultiBlock(buff, sector, count) == 0) {
            return RES_OK;
        }
    }

    return RES_ERROR;
  /* USER CODE END READ */
}

/**
  * @brief  Writes Sector(s)
  */
#if _USE_WRITE == 1
DRESULT USER_write (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
  /* USER CODE BEGIN WRITE */
    if (pdrv != 0 || !count) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;

    if (count == 1) {
        if (SD_SPI_WriteBlock(buff, sector) == 0) {
            return RES_OK;
        }
    } else {
        if (SD_SPI_WriteMultiBlock(buff, sector, count) == 0) {
            return RES_OK;
        }
    }

    return RES_ERROR;
  /* USER CODE END WRITE */
}
#endif

/**
  * @brief  I/O control operation
  */
#if _USE_IOCTL == 1
DRESULT USER_ioctl (BYTE pdrv, BYTE cmd, void *buff)
{
  /* USER CODE BEGIN IOCTL */
    DRESULT res = RES_ERROR;

    if (pdrv != 0) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;

    switch (cmd) {
        case CTRL_SYNC:
            res = RES_OK;
            break;

        case GET_SECTOR_COUNT:
            *(DWORD*)buff = 0;
            res = RES_OK;
            break;

        case GET_SECTOR_SIZE:
            *(WORD*)buff = 512;
            res = RES_OK;
            break;

        case GET_BLOCK_SIZE:
            *(DWORD*)buff = 1;
            res = RES_OK;
            break;

        default:
            res = RES_PARERR;
    }

    return res;
  /* USER CODE END IOCTL */
}
#endif

/* USER CODE BEGIN AFTER */
__weak DWORD get_fattime(void)
{
    return ((DWORD)(2025 - 1980) << 25)
         | ((DWORD)1 << 21)
         | ((DWORD)1 << 16)
         | ((DWORD)0 << 11)
         | ((DWORD)0 << 5)
         | ((DWORD)0 >> 1);
}
/* USER CODE END AFTER */

```

### 4. main.c

```c
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : SD Card FATFS Final Version
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include "sd_spi_driver.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SD_DEBUG 0  // 1=디버그 출력 활성화, 0=비활성화
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
FATFS FatFs;
FIL File;
FRESULT fres;
UINT bytesWritten, bytesRead;
char buffer[512];

// 통계 정보
uint32_t totalFilesCreated = 0;
uint32_t totalBytesWritten = 0;
uint32_t totalBytesRead = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);

/* USER CODE BEGIN PFP */
// SD Card 기본 함수
FRESULT SD_WriteFile(const char* filename, const char* data);
FRESULT SD_ReadFile(const char* filename, char* buffer, uint32_t bufsize);
FRESULT SD_AppendFile(const char* filename, const char* data);
FRESULT SD_DeleteFile(const char* filename);
FRESULT SD_CreateDirectory(const char* dirname);
FRESULT SD_ListFiles(const char* path);

// 데이터 로깅 함수
FRESULT SD_LogData(const char* filename, const char* data);
FRESULT SD_CreateCSV(const char* filename, const char* header);
FRESULT SD_AppendCSV(const char* filename, const char* data);

// 유틸리티 함수
void SD_GetInfo(void);
void SD_ShowStatistics(void);
void LED_Blink(uint32_t count, uint32_t delay_ms);

// 데모 함수
void SD_Demo_BasicOperations(void);
void SD_Demo_DataLogging(void);
void SD_Demo_CSVLogging(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief  printf 리다이렉트
  */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
    if (ch == '\n')
        HAL_UART_Transmit(&huart2, (uint8_t*)"\r", 1, 0xFFFF);
    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, 0xFFFF);
    return ch;
}

/**
  * @brief  LED 깜빡이기
  */
void LED_Blink(uint32_t count, uint32_t delay_ms)
{
    for(uint32_t i = 0; i < count; i++) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_Delay(delay_ms);
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_Delay(delay_ms);
    }
}

/**
  * @brief  파일 쓰기 (새로 생성 또는 덮어쓰기)
  */
FRESULT SD_WriteFile(const char* filename, const char* data)
{
    fres = f_open(&File, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (fres == FR_OK) {
        UINT bw;
        fres = f_write(&File, data, strlen(data), &bw);
        totalBytesWritten += bw;
        totalFilesCreated++;
        f_close(&File);
        printf("[SD] Created: %s (%u bytes)\n", filename, bw);
    } else {
        printf("[SD] Error creating file: %d\n", fres);
    }
    return fres;
}

/**
  * @brief  파일 읽기
  */
FRESULT SD_ReadFile(const char* filename, char* buffer, uint32_t bufsize)
{
    fres = f_open(&File, filename, FA_READ);
    if (fres == FR_OK) {
        UINT br;
        fres = f_read(&File, buffer, bufsize - 1, &br);
        buffer[br] = '\0';  // Null terminator
        totalBytesRead += br;
        f_close(&File);
        printf("[SD] Read: %s (%u bytes)\n", filename, br);
    } else {
        printf("[SD] Error reading file: %d\n", fres);
    }
    return fres;
}

/**
  * @brief  파일에 데이터 추가 (append)
  */
FRESULT SD_AppendFile(const char* filename, const char* data)
{
    fres = f_open(&File, filename, FA_WRITE | FA_OPEN_EXISTING);
    if (fres == FR_OK) {
        /* Move to end of file */
        f_lseek(&File, f_size(&File));
        
        UINT bw;
        fres = f_write(&File, data, strlen(data), &bw);
        totalBytesWritten += bw;
        f_close(&File);
        printf("[SD] Appended to: %s (%u bytes)\n", filename, bw);
    } else {
        printf("[SD] Error appending: %d\n", fres);
    }
    return fres;
}

/**
  * @brief  파일 삭제
  */
FRESULT SD_DeleteFile(const char* filename)
{
    fres = f_unlink(filename);
    if (fres == FR_OK) {
        printf("[SD] Deleted: %s\n", filename);
    } else {
        printf("[SD] Error deleting file: %d\n", fres);
    }
    return fres;
}

/**
  * @brief  디렉토리 생성
  */
FRESULT SD_CreateDirectory(const char* dirname)
{
    fres = f_mkdir(dirname);
    if (fres == FR_OK) {
        printf("[SD] Created directory: %s\n", dirname);
    } else if (fres == FR_EXIST) {
        printf("[SD] Directory already exists: %s\n", dirname);
    } else {
        printf("[SD] Error creating directory: %d\n", fres);
    }
    return fres;
}

/**
  * @brief  파일/디렉토리 목록 표시
  */
FRESULT SD_ListFiles(const char* path)
{
    DIR dir;
    FILINFO fno;
    
    printf("\n[SD] Listing files in: %s\n", path);
    printf("-------------------------------------\n");
    
    fres = f_opendir(&dir, path);
    if (fres == FR_OK) {
        int count = 0;
        while (1) {
            fres = f_readdir(&dir, &fno);
            if (fres != FR_OK || fno.fname[0] == 0) break;
            
            if (fno.fattrib & AM_DIR) {
                printf("  [DIR]  %s\n", fno.fname);
            } else {
                printf("  [FILE] %s (%lu bytes)\n", fno.fname, fno.fsize);
            }
            count++;
        }
        printf("-------------------------------------\n");
        printf("Total: %d items\n\n", count);
        f_closedir(&dir);
    }
    return fres;
}

/**
  * @brief  데이터 로깅 (타임스탬프 포함)
  */
FRESULT SD_LogData(const char* filename, const char* data)
{
    char logEntry[256];
    uint32_t timestamp = HAL_GetTick() / 1000;  // seconds
    
    snprintf(logEntry, sizeof(logEntry), "[%06lu] %s\n", timestamp, data);
    return SD_AppendFile(filename, logEntry);
}

/**
  * @brief  CSV 파일 생성 (헤더 포함)
  */
FRESULT SD_CreateCSV(const char* filename, const char* header)
{
    char csvHeader[256];
    snprintf(csvHeader, sizeof(csvHeader), "%s\n", header);
    return SD_WriteFile(filename, csvHeader);
}

/**
  * @brief  CSV 파일에 데이터 추가
  */
FRESULT SD_AppendCSV(const char* filename, const char* data)
{
    char csvLine[256];
    snprintf(csvLine, sizeof(csvLine), "%s\n", data);
    return SD_AppendFile(filename, csvLine);
}

/**
  * @brief  SD 카드 정보 표시
  */
void SD_GetInfo(void)
{
    FATFS *fs;
    DWORD fre_clust;
    
    printf("\n=== SD Card Information ===\n");
    
    if (f_getfree("", &fre_clust, &fs) == FR_OK) {
        DWORD tot_sect = (fs->n_fatent - 2) * fs->csize;
        DWORD fre_sect = fre_clust * fs->csize;
        
        printf("Type: FAT%d\n", fs->fs_type);
        printf("Sector Size: 512 bytes\n");
        printf("Cluster Size: %d sectors\n", fs->csize);
        printf("Total Size: %lu MB\n", tot_sect / 2048);
        printf("Used: %lu MB\n", (tot_sect - fre_sect) / 2048);
        printf("Free: %lu MB\n", fre_sect / 2048);
        printf("Card Type: 0x%02X\n", SD_SPI_GetCardInfo());
    }
    printf("===========================\n\n");
}

/**
  * @brief  통계 정보 표시
  */
void SD_ShowStatistics(void)
{
    printf("\n=== Session Statistics ===\n");
    printf("Files Created: %lu\n", totalFilesCreated);
    printf("Bytes Written: %lu\n", totalBytesWritten);
    printf("Bytes Read: %lu\n", totalBytesRead);
    printf("==========================\n\n");
}

/**
  * @brief  데모 1: 기본 파일 작업
  */
void SD_Demo_BasicOperations(void)
{
    printf("\n>>> Demo 1: Basic File Operations <<<\n\n");
    
    // 1. 파일 생성 및 쓰기
    SD_WriteFile("demo1.txt", "Hello from NUCLEO-F103RB!\n");
    
    // 2. 파일 읽기
    SD_ReadFile("demo1.txt", buffer, sizeof(buffer));
    printf("Content: %s\n", buffer);
    
    // 3. 파일에 데이터 추가
    SD_AppendFile("demo1.txt", "This is an appended line.\n");
    
    // 4. 다시 읽기
    SD_ReadFile("demo1.txt", buffer, sizeof(buffer));
    printf("Updated content:\n%s\n", buffer);
    
    // 5. 디렉토리 생성
    SD_CreateDirectory("logs");
    
    // 6. 디렉토리 안에 파일 생성
    SD_WriteFile("logs/test.txt", "File inside logs directory\n");
    
    // 7. 파일 목록 표시
    SD_ListFiles("/");
    SD_ListFiles("/logs");
}

/**
  * @brief  데모 2: 데이터 로깅
  */
void SD_Demo_DataLogging(void)
{
    printf("\n>>> Demo 2: Data Logging <<<\n\n");
    
    // 로그 파일 생성
    SD_WriteFile("system.log", "=== System Log Started ===\n");
    
    // 주기적 로깅 시뮬레이션
    for(int i = 0; i < 5; i++) {
        char logData[128];
        snprintf(logData, sizeof(logData), 
                 "Temperature: %d°C, Humidity: %d%%", 
                 20 + i, 50 + i*2);
        
        SD_LogData("system.log", logData);
        HAL_Delay(100);
    }
    
    // 로그 파일 읽기
    SD_ReadFile("system.log", buffer, sizeof(buffer));
    printf("\nLog contents:\n%s\n", buffer);
}

/**
  * @brief  데모 3: CSV 데이터 로깅
  */
void SD_Demo_CSVLogging(void)
{
    printf("\n>>> Demo 3: CSV Data Logging <<<\n\n");
    
    // CSV 파일 생성 (헤더)
    SD_CreateCSV("data.csv", "Time,Temperature,Humidity,Pressure");
    
    // 데이터 추가
    for(int i = 0; i < 10; i++) {
        char csvData[128];
        uint32_t time = HAL_GetTick() / 1000;
        snprintf(csvData, sizeof(csvData), 
                 "%lu,%d,%d,%d", 
                 time, 20+i, 50+i, 1013+i);
        
        SD_AppendCSV("data.csv", csvData);
        HAL_Delay(50);
    }
    
    printf("[SD] Created data.csv with 10 entries\n");
    printf("      You can open this file in Excel!\n\n");
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_FATFS_Init();

  /* USER CODE BEGIN 2 */
  
  printf("\n\n");
  printf("========================================\n");
  printf("  NUCLEO-F103RB SD Card FATFS Demo\n");
  printf("  System Clock: 64MHz\n");
  printf("  SPI Speed: 8MHz\n");
  printf("========================================\n\n");
  
  // SD 카드 초기화 대기
  HAL_Delay(500);
  
  // SD 카드 초기화
  printf("Initializing SD Card...\n");
  uint8_t init_result = SD_SPI_Init();
  
  if (init_result != 0) {
      printf("❌ SD Card initialization FAILED!\n");
      printf("   Check connections and try again.\n\n");
      LED_Blink(10, 100);  // 빠른 깜빡임
      while(1) { HAL_Delay(1000); }
  }
  
  printf("✓ SD Card initialized (Type: 0x%02X)\n\n", SD_SPI_GetCardInfo());
  
  // FATFS 마운트
  printf("Mounting filesystem...\n");
  fres = f_mount(&FatFs, "", 1);
  
  if (fres != FR_OK) {
      printf("❌ Mount FAILED! Error: %d\n", fres);
      printf("   Format SD card as FAT32 and try again.\n\n");
      LED_Blink(10, 100);
      while(1) { HAL_Delay(1000); }
  }
  
  printf("✓ Filesystem mounted successfully!\n");
  LED_Blink(3, 200);  // 성공 신호
  
  // SD 카드 정보 표시
  SD_GetInfo();
  
  // 데모 실행
  SD_Demo_BasicOperations();
  HAL_Delay(500);
  
  SD_Demo_DataLogging();
  HAL_Delay(500);
  
  SD_Demo_CSVLogging();
  HAL_Delay(500);
  
  // 통계 표시
  SD_ShowStatistics();
  
  // 최종 파일 목록
  printf("=== Final File List ===\n");
  SD_ListFiles("/");
  
  printf("\n========================================\n");
  printf("  All demos completed successfully!\n");
  printf("  Remove SD card and check files on PC\n");
  printf("========================================\n\n");
  
  // 성공 - 느린 깜빡임
  while (1) {
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
      HAL_Delay(1000);
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  */
static void MX_SPI1_Init(void)
{
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  */
static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);

  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */
```

---

## 결과

<img width="611" height="854" alt="005" src="https://github.com/user-attachments/assets/8263906e-a4be-43aa-a111-839459fb1209" />
<br>

<img width="607" height="193" alt="006" src="https://github.com/user-attachments/assets/35d32136-7702-42fc-afb2-cc0c298dcc67" />
<br>

<img width="585" height="143" alt="007" src="https://github.com/user-attachments/assets/ae9d08ce-bbb1-4bd8-aa5f-64b1a1efbf79" />
<br>

<img width="564" height="129" alt="008" src="https://github.com/user-attachments/assets/82fb31f3-5007-4f38-abe7-1909b3a9f8d0" />
<br>
