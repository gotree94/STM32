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
```
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
```
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


## user_disk_io.c
```
/**
  ******************************************************************************
  * @file    user_diskio.c (수정 버전 2)
  * @brief   FATFS user diskio - 초기화 문제 수정
  ******************************************************************************
  */

/* USER CODE BEGIN HEADER */
#include <string.h>
#include <stdio.h>
/* USER CODE END HEADER */

#ifdef USE_OBSOLETE_USER_CODE_SECTION_0
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */
#endif

/* USER CODE BEGIN HEADER_1 */
#include "sd_spi_driver.h"
/* USER CODE END HEADER_1 */

#include "ff_gen_drv.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static volatile DSTATUS Stat = STA_NOINIT;
static uint8_t CardInitialized = 0;  // ← 초기화 상태 플래그 추가
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
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
#if  _USE_WRITE == 1
  USER_write,
#endif
#if  _USE_IOCTL == 1
  USER_ioctl,
#endif
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes a Drive
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_initialize (
	BYTE pdrv           /* Physical drive nmuber to identify the drive */
)
{
  /* USER CODE BEGIN INIT */
    printf("[DISKIO] USER_initialize called (pdrv=%d)\n", pdrv);
    
    Stat = STA_NOINIT;
    
    if (pdrv != 0) {
        printf("[DISKIO] Invalid drive number\n");
        return STA_NOINIT;
    }
    
    // 이미 초기화되었으면 다시 초기화하지 않음
    if (CardInitialized) {
        printf("[DISKIO] Card already initialized\n");
        Stat = 0;  // 정상 상태
        return Stat;
    }
    
    // SD 카드 초기화
    printf("[DISKIO] Calling SD_SPI_Init...\n");
    if (SD_SPI_Init() == 0) {
        printf("[DISKIO] SD_SPI_Init SUCCESS\n");
        Stat = 0;  // STA_NOINIT 플래그 제거
        CardInitialized = 1;
    } else {
        printf("[DISKIO] SD_SPI_Init FAILED\n");
        Stat = STA_NOINIT;
        CardInitialized = 0;
    }
    
    printf("[DISKIO] Initialize complete - Status: 0x%02X\n", Stat);
    return Stat;
  /* USER CODE END INIT */
}

/**
  * @brief  Gets Disk Status
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_status (
	BYTE pdrv       /* Physical drive number to identify the drive */
)
{
  /* USER CODE BEGIN STATUS */
    if (pdrv != 0) {
        return STA_NOINIT;
    }
    
    // 카드가 초기화되었고 유효한 타입이면 정상
    if (CardInitialized && SD_SPI_GetCardInfo() != 0) {
        Stat = 0;  // 정상
    } else {
        Stat = STA_NOINIT;
    }
    
    return Stat;
  /* USER CODE END STATUS */
}

/**
  * @brief  Reads Sector(s)
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT USER_read (
	BYTE pdrv,      /* Physical drive nmuber to identify the drive */
	BYTE *buff,     /* Data buffer to store read data */
	DWORD sector,   /* Sector address in LBA */
	UINT count      /* Number of sectors to read */
)
{
  /* USER CODE BEGIN READ */
    if (pdrv != 0 || !count) {
        printf("[DISKIO] Read: Invalid parameters\n");
        return RES_PARERR;
    }
    
    if (Stat & STA_NOINIT) {
        printf("[DISKIO] Read: Drive not ready\n");
        return RES_NOTRDY;
    }
    
    printf("[DISKIO] Reading sector %lu, count %u\n", sector, count);
    
    if (count == 1) {
        if (SD_SPI_ReadBlock(buff, sector) == 0) {
            printf("[DISKIO] Read SUCCESS\n");
            return RES_OK;
        }
    } else {
        if (SD_SPI_ReadMultiBlock(buff, sector, count) == 0) {
            printf("[DISKIO] Multi-read SUCCESS\n");
            return RES_OK;
        }
    }
    
    printf("[DISKIO] Read FAILED\n");
    return RES_ERROR;
  /* USER CODE END READ */
}

/**
  * @brief  Writes Sector(s)
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
#if _USE_WRITE == 1
DRESULT USER_write (
	BYTE pdrv,          /* Physical drive nmuber to identify the drive */
	const BYTE *buff,   /* Data to be written */
	DWORD sector,       /* Sector address in LBA */
	UINT count          /* Number of sectors to write */
)
{
  /* USER CODE BEGIN WRITE */
    if (pdrv != 0 || !count) {
        printf("[DISKIO] Write: Invalid parameters\n");
        return RES_PARERR;
    }
    
    if (Stat & STA_NOINIT) {
        printf("[DISKIO] Write: Drive not ready\n");
        return RES_NOTRDY;
    }
    
    printf("[DISKIO] Writing sector %lu, count %u\n", sector, count);
    
    if (count == 1) {
        if (SD_SPI_WriteBlock(buff, sector) == 0) {
            printf("[DISKIO] Write SUCCESS\n");
            return RES_OK;
        }
    } else {
        if (SD_SPI_WriteMultiBlock(buff, sector, count) == 0) {
            printf("[DISKIO] Multi-write SUCCESS\n");
            return RES_OK;
        }
    }
    
    printf("[DISKIO] Write FAILED\n");
    return RES_ERROR;
  /* USER CODE END WRITE */
}
#endif

/**
  * @brief  I/O control operation
  * @param  pdrv: Physical drive number (0..)
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
#if _USE_IOCTL == 1
DRESULT USER_ioctl (
	BYTE pdrv,      /* Physical drive nmuber (0..) */
	BYTE cmd,       /* Control code */
	void *buff      /* Buffer to send/receive control data */
)
{
  /* USER CODE BEGIN IOCTL */
    DRESULT res = RES_ERROR;
    
    if (pdrv != 0) return RES_PARERR;
    
    if (Stat & STA_NOINIT) return RES_NOTRDY;
    
    switch (cmd) {
        case CTRL_SYNC:
            printf("[DISKIO] IOCTL: CTRL_SYNC\n");
            res = RES_OK;
            break;
            
        case GET_SECTOR_COUNT:
            printf("[DISKIO] IOCTL: GET_SECTOR_COUNT\n");
            *(DWORD*)buff = 0;  // FATFS will determine this
            res = RES_OK;
            break;
            
        case GET_SECTOR_SIZE:
            printf("[DISKIO] IOCTL: GET_SECTOR_SIZE\n");
            *(WORD*)buff = 512;
            res = RES_OK;
            break;
            
        case GET_BLOCK_SIZE:
            printf("[DISKIO] IOCTL: GET_BLOCK_SIZE\n");
            *(DWORD*)buff = 1;
            res = RES_OK;
            break;
            
        default:
            printf("[DISKIO] IOCTL: Unknown command %d\n", cmd);
            res = RES_PARERR;
    }
    
    return res;
  /* USER CODE END IOCTL */
}
#endif

/* USER CODE BEGIN DSTATUS */
__weak DWORD get_fattime(void)
{
    /* Return fixed time: 2025-01-01 00:00:00 */
    return ((DWORD)(2025 - 1980) << 25)
         | ((DWORD)1 << 21)
         | ((DWORD)1 << 16)
         | ((DWORD)0 << 11)
         | ((DWORD)0 << 5)
         | ((DWORD)0 >> 1);
}
/* USER CODE END DSTATUS */
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
