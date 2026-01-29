# NUCLEO-F767ZI USB Device MSC - 안정적 전송 모드

STM32 NUCLEO-F767ZI 보드를 USB 메모리 장치로 인식시켜 PC와 파일을 교환하는 예제입니다.
이 버전은 **내부 Flash 또는 SD카드**를 사용하여 안정적인 데이터 전송에 초점을 맞춥니다.

## 📋 프로젝트 개요

| 항목 | 내용 |
|------|------|
| 보드 | NUCLEO-F767ZI |
| MCU | STM32F767ZIT6 (ARM Cortex-M7, 216MHz) |
| IDE | STM32CubeIDE |
| USB 모드 | **USB Device (Full Speed 12Mbps)** |
| 저장매체 | 내부 Flash 또는 SD카드 (SPI 모드) |
| 특징 | **안정성 우선**, 저전력, 단순 구성 |

## 🎯 이 예제의 특징

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      안정적 전송 모드 특징                                    │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   ✅ 장점                              ❌ 단점                              │
│   ─────────────────────────────────    ─────────────────────────────────   │
│   • USB Full Speed (12 Mbps)           • 최대 ~1 MB/s 전송 속도            │
│   • 간단한 하드웨어 구성               • 대용량 파일 전송 시 느림           │
│   • 낮은 전력 소비                     • 내부 Flash 용량 제한              │
│   • 높은 호환성                                                            │
│   • 안정적인 동작                                                          │
│   • 디버깅 용이                                                            │
│                                                                             │
│   권장 용도:                                                                │
│   • 설정 파일 교환                                                         │
│   • 로그 데이터 다운로드                                                   │
│   • 펌웨어 업데이트                                                        │
│   • 소량 데이터 전송 (< 10MB)                                              │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 🔧 하드웨어 구성

### Option A: 내부 Flash 사용 (추가 하드웨어 불필요)

```
┌─────────────────────────────────────────────────────────────────┐
│                    내부 Flash 구성                               │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   PC                              NUCLEO-F767ZI                 │
│   ┌─────────────┐                ┌─────────────────────┐       │
│   │             │   USB Cable    │                     │       │
│   │  Windows    │◄──────────────►│  PA11 (DM)          │       │
│   │  탐색기     │                │  PA12 (DP)          │       │
│   │             │                │                     │       │
│   │  "STM32     │                │  ┌───────────────┐  │       │
│   │   Drive"    │                │  │ Internal Flash│  │       │
│   │             │                │  │ (일부 영역)   │  │       │
│   │  64KB~128KB │                │  │ 64KB~128KB    │  │       │
│   └─────────────┘                │  └───────────────┘  │       │
│                                  └─────────────────────┘       │
│                                                                 │
│   장점: 추가 부품 불필요                                        │
│   단점: 용량 제한 (64~128KB), Flash 수명                        │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Option B: SD카드 사용 (SPI 모드)

```
┌─────────────────────────────────────────────────────────────────┐
│                    SD카드 (SPI) 구성                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   NUCLEO-F767ZI              SD Card Module                     │
│   ┌─────────────┐            ┌─────────────┐                   │
│   │             │            │             │                   │
│   │  PB3 (SCK)  │───────────▶│  CLK        │                   │
│   │  PB4 (MISO) │◀───────────│  MISO (DO)  │                   │
│   │  PB5 (MOSI) │───────────▶│  MOSI (DI)  │                   │
│   │  PB6 (CS)   │───────────▶│  CS         │                   │
│   │             │            │             │                   │
│   │  3.3V       │───────────▶│  VCC        │                   │
│   │  GND        │───────────▶│  GND        │                   │
│   │             │            │             │                   │
│   └─────────────┘            └─────────────┘                   │
│                                                                 │
│   장점: 대용량 (GB 단위), 교체 가능                             │
│   단점: SPI 속도 제한 (~2MB/s), 추가 모듈 필요                  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### USB 핀 매핑 (공통)

| 기능 | GPIO | 설명 |
|------|------|------|
| USB_OTG_FS_DM | PA11 | USB Data Minus |
| USB_OTG_FS_DP | PA12 | USB Data Plus |
| USB_OTG_FS_VBUS | PA9 | VBUS 감지 (선택) |

## ⚙️ CubeMX 설정 (내부 Flash 버전)

### 1. RCC 설정

**Pinout & Configuration → System Core → RCC**

| 항목 | 설정값 |
|------|--------|
| HSE | **BYPASS Clock Source** |

**Clock Configuration:**

| 파라미터 | 값 |
|----------|-----|
| SYSCLK | 216 MHz |
| **USB Clock** | **48 MHz** ⚠️ 필수! |

### 2. USB_OTG_FS 설정

**Pinout & Configuration → Connectivity → USB_OTG_FS**

| 항목 | 설정값 |
|------|--------|
| Mode | **Device_Only** |
| Activate_VBUS | Unchecked (버스 전원 사용) |

### 3. USB_DEVICE 설정

**Pinout & Configuration → Middleware → USB_DEVICE**

| 항목 | 설정값 |
|------|--------|
| Class For FS IP | **Mass Storage Class (MSC)** |

#### Parameter Settings

| 파라미터 | 값 | 설명 |
|----------|-----|------|
| MSC_MEDIA_PACKET | 512 | 패킷 크기 |
| MSC Interface String | "STM32 Mass Storage" | 장치 설명 |

### 4. USART3 설정 (디버그용)

| 항목 | 설정값 |
|------|--------|
| Mode | Asynchronous |
| Baud Rate | 115200 |

### 5. GPIO 설정 (LED)

| 핀 | Mode | User Label |
|----|------|------------|
| PB0 | Output Push Pull | LD1 |
| PB7 | Output Push Pull | LD2 |
| PB14 | Output Push Pull | LD3 |

### 6. 코드 생성

**Ctrl+S** 또는 **Project → Generate Code**

## 💻 소스 코드

### 내부 Flash를 저장매체로 사용

#### usbd_storage_if.c 수정

`USB_DEVICE/App/usbd_storage_if.c` 파일을 수정합니다:

```c
/* USER CODE BEGIN Header */
/**
  * NUCLEO-F767ZI USB Device MSC - Internal Flash Storage
  * 안정적 전송 모드 (USB Full Speed)
  */
/* USER CODE END Header */

/* USER CODE BEGIN INCLUDE */
#include <string.h>
/* USER CODE END INCLUDE */

/* USER CODE BEGIN PRIVATE_DEFINES */
// 내부 Flash의 일부를 USB 저장소로 사용
// Sector 6 (256KB @ 0x08040000) 사용 - 프로그램 영역과 분리

#define STORAGE_LUN_NBR         1
#define STORAGE_BLK_NBR         128        // 128 블록 = 64KB
#define STORAGE_BLK_SIZ         512        // 512 bytes/block (표준)

// Flash Sector 6 시작 주소 (STM32F767)
#define FLASH_STORAGE_BASE      0x08040000
#define FLASH_STORAGE_SIZE      (STORAGE_BLK_NBR * STORAGE_BLK_SIZ)  // 64KB

/* USER CODE END PRIVATE_DEFINES */

/* USER CODE BEGIN PRIVATE_VARIABLES */
// RAM 버퍼 (Flash 쓰기를 위한 섹터 버퍼)
static uint8_t flash_buffer[STORAGE_BLK_SIZ];
/* USER CODE END PRIVATE_VARIABLES */

/**
  * @brief  Initializes the storage unit (medium) for USB MSC
  * @retval USBD_OK if OK, USBD_FAIL if error
  */
int8_t STORAGE_Init_FS(uint8_t lun)
{
    /* USER CODE BEGIN 2 */
    UNUSED(lun);
    return (USBD_OK);
    /* USER CODE END 2 */
}

/**
  * @brief  Returns the medium capacity
  * @param  lun: Logical unit number
  * @param  block_num: Number of total block number
  * @param  block_size: Block size
  * @retval USBD_OK if OK, USBD_FAIL if error
  */
int8_t STORAGE_GetCapacity_FS(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
    /* USER CODE BEGIN 3 */
    UNUSED(lun);
    *block_num  = STORAGE_BLK_NBR;
    *block_size = STORAGE_BLK_SIZ;
    return (USBD_OK);
    /* USER CODE END 3 */
}

/**
  * @brief  Checks whether the medium is ready
  * @retval USBD_OK if OK, USBD_FAIL if error
  */
int8_t STORAGE_IsReady_FS(uint8_t lun)
{
    /* USER CODE BEGIN 4 */
    UNUSED(lun);
    return (USBD_OK);
    /* USER CODE END 4 */
}

/**
  * @brief  Checks whether the medium is write protected
  * @retval USBD_OK if OK, USBD_FAIL if error
  */
int8_t STORAGE_IsWriteProtected_FS(uint8_t lun)
{
    /* USER CODE BEGIN 5 */
    UNUSED(lun);
    return (USBD_OK);  // 쓰기 가능
    /* USER CODE END 5 */
}

/**
  * @brief  Reads data from the medium
  * @param  lun: Logical unit number
  * @param  buf: Pointer to the buffer to save data
  * @param  blk_addr: Block address
  * @param  blk_len: Number of blocks to read
  * @retval USBD_OK if OK, USBD_FAIL if error
  */
int8_t STORAGE_Read_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
    /* USER CODE BEGIN 6 */
    UNUSED(lun);
    
    uint32_t flash_addr = FLASH_STORAGE_BASE + (blk_addr * STORAGE_BLK_SIZ);
    uint32_t length = blk_len * STORAGE_BLK_SIZ;
    
    // 범위 체크
    if ((blk_addr + blk_len) > STORAGE_BLK_NBR)
    {
        return USBD_FAIL;
    }
    
    // Flash에서 직접 읽기
    memcpy(buf, (uint8_t *)flash_addr, length);
    
    return (USBD_OK);
    /* USER CODE END 6 */
}

/**
  * @brief  Writes data into the medium
  * @param  lun: Logical unit number
  * @param  buf: Pointer to the buffer to write from
  * @param  blk_addr: Block address
  * @param  blk_len: Number of blocks to write
  * @retval USBD_OK if OK, USBD_FAIL if error
  */
int8_t STORAGE_Write_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
    /* USER CODE BEGIN 7 */
    UNUSED(lun);
    
    uint32_t flash_addr = FLASH_STORAGE_BASE + (blk_addr * STORAGE_BLK_SIZ);
    uint32_t length = blk_len * STORAGE_BLK_SIZ;
    
    // 범위 체크
    if ((blk_addr + blk_len) > STORAGE_BLK_NBR)
    {
        return USBD_FAIL;
    }
    
    // Flash 쓰기 (HAL 사용)
    HAL_FLASH_Unlock();
    
    // 섹터 삭제 (최초 쓰기 또는 블록 0일 때만)
    // 주의: 실제 구현에서는 더 정교한 wear leveling 필요
    if (blk_addr == 0)
    {
        FLASH_EraseInitTypeDef erase_init;
        uint32_t sector_error;
        
        erase_init.TypeErase = FLASH_TYPEERASE_SECTORS;
        erase_init.Sector = FLASH_SECTOR_6;
        erase_init.NbSectors = 1;
        erase_init.VoltageRange = FLASH_VOLTAGE_RANGE_3;
        
        HAL_FLASHEx_Erase(&erase_init, &sector_error);
    }
    
    // 데이터 쓰기 (워드 단위)
    for (uint32_t i = 0; i < length; i += 4)
    {
        uint32_t data = *(uint32_t *)(buf + i);
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, flash_addr + i, data);
    }
    
    HAL_FLASH_Lock();
    
    return (USBD_OK);
    /* USER CODE END 7 */
}

/**
  * @brief  Returns the Max Supported LUNs
  * @retval lun number
  */
int8_t STORAGE_GetMaxLun_FS(void)
{
    /* USER CODE BEGIN 8 */
    return (STORAGE_LUN_NBR - 1);
    /* USER CODE END 8 */
}
```

### main.c

```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "usb_device.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
extern USBD_HandleTypeDef hUsbDeviceFS;
/* USER CODE END PV */

/* USER CODE BEGIN 0 */

// printf 리다이렉션
#ifdef __GNUC__
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
#endif

/* USER CODE END 0 */

int main(void)
{
    /* MCU Configuration */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART3_UART_Init();
    MX_USB_DEVICE_Init();

    /* USER CODE BEGIN 2 */
    printf("\r\n============================================\r\n");
    printf("  NUCLEO-F767ZI USB Device MSC Demo\r\n");
    printf("  Mode: Stable Transfer (USB Full Speed)\r\n");
    printf("  Storage: Internal Flash (64KB)\r\n");
    printf("  System Clock: %lu MHz\r\n", HAL_RCC_GetSysClockFreq() / 1000000);
    printf("============================================\r\n");
    printf("Connect USB cable to PC...\r\n\n");

    // LED 초기화
    HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);  // Green ON - Ready
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    uint8_t prev_state = 0xFF;
    
    while (1)
    {
        // USB 연결 상태 모니터링
        uint8_t current_state = hUsbDeviceFS.dev_state;
        
        if (current_state != prev_state)
        {
            prev_state = current_state;
            
            switch (current_state)
            {
                case USBD_STATE_DEFAULT:
                    printf("USB State: DEFAULT\r\n");
                    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
                    break;
                    
                case USBD_STATE_ADDRESSED:
                    printf("USB State: ADDRESSED\r\n");
                    break;
                    
                case USBD_STATE_CONFIGURED:
                    printf("USB State: CONFIGURED - Ready for file transfer!\r\n");
                    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);  // Blue ON
                    break;
                    
                case USBD_STATE_SUSPENDED:
                    printf("USB State: SUSPENDED\r\n");
                    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
                    break;
                    
                default:
                    break;
            }
        }
        
        HAL_Delay(100);
        
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}
```

## 📊 SD카드 버전 (SPI 모드)

### 추가 CubeMX 설정

**Connectivity → SPI1**

| 파라미터 | 값 |
|----------|-----|
| Mode | Full-Duplex Master |
| Prescaler | 128 (초기화용, 느림) |
| CPOL | Low |
| CPHA | 1 Edge |
| Data Size | 8 Bits |

**Middleware → FATFS**

| 항목 | 설정값 |
|------|--------|
| Mode | User-defined |
| USE_LFN | Enabled with dynamic working buffer on the HEAP |

### SD카드용 usbd_storage_if.c

```c
/* USER CODE BEGIN PRIVATE_DEFINES */
#define STORAGE_LUN_NBR         1
#define STORAGE_BLK_SIZ         512

// SD카드 용량 (동적으로 읽어옴)
static uint32_t sd_block_count = 0;
/* USER CODE END PRIVATE_DEFINES */

/* USER CODE BEGIN INCLUDE */
#include "fatfs.h"
#include "sd_diskio.h"
/* USER CODE END INCLUDE */

int8_t STORAGE_Init_FS(uint8_t lun)
{
    /* USER CODE BEGIN 2 */
    if (SD_initialize(0) != RES_OK)
    {
        return USBD_FAIL;
    }
    
    // SD카드 용량 읽기
    SD_ioctl(0, GET_SECTOR_COUNT, &sd_block_count);
    
    return (USBD_OK);
    /* USER CODE END 2 */
}

int8_t STORAGE_GetCapacity_FS(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
    /* USER CODE BEGIN 3 */
    *block_num  = sd_block_count;
    *block_size = STORAGE_BLK_SIZ;
    return (USBD_OK);
    /* USER CODE END 3 */
}

int8_t STORAGE_Read_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
    /* USER CODE BEGIN 6 */
    if (SD_read(0, buf, blk_addr, blk_len) != RES_OK)
    {
        return USBD_FAIL;
    }
    return (USBD_OK);
    /* USER CODE END 6 */
}

int8_t STORAGE_Write_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
    /* USER CODE BEGIN 7 */
    if (SD_write(0, buf, blk_addr, blk_len) != RES_OK)
    {
        return USBD_FAIL;
    }
    return (USBD_OK);
    /* USER CODE END 7 */
}
```

## 🔄 동작 방식

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    USB Device MSC 동작 흐름                                  │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   PC (USB Host)                    STM32 (USB Device)                      │
│   ┌─────────────┐                 ┌─────────────┐                          │
│   │             │                 │             │                          │
│   │  Windows    │ ─── USB 연결 ──▶│  USB Device │                          │
│   │             │                 │  Controller │                          │
│   │             │◀── 열거 응답 ───│             │                          │
│   │             │                 │             │                          │
│   │  "드라이브  │                 │  ┌───────┐  │                          │
│   │   인식됨"   │ ── SCSI Read ──▶│  │Storage│  │                          │
│   │             │◀─ 데이터 반환 ──│  │ Media │  │                          │
│   │             │                 │  │(Flash)│  │                          │
│   │  파일 복사  │ ── SCSI Write ─▶│  └───────┘  │                          │
│   │             │◀── 완료 응답 ───│             │                          │
│   │             │                 │             │                          │
│   └─────────────┘                 └─────────────┘                          │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 📺 예상 출력

### STM32 시리얼 출력

```
============================================
  NUCLEO-F767ZI USB Device MSC Demo
  Mode: Stable Transfer (USB Full Speed)
  Storage: Internal Flash (64KB)
  System Clock: 216 MHz
============================================
Connect USB cable to PC...

USB State: DEFAULT
USB State: ADDRESSED
USB State: CONFIGURED - Ready for file transfer!
```

### PC에서 보이는 드라이브

```
Windows 탐색기:
┌─────────────────────────────────────────┐
│  💾 STM32 Mass Storage (E:)             │
│     용량: 64.0 KB                       │
│     파일 시스템: FAT12                  │
└─────────────────────────────────────────┘
```

## ⚠️ 내부 Flash 사용 시 주의사항

### Flash 수명

```
STM32F767 Flash 수명:
• 최소 10,000 회 쓰기/지우기 사이클
• 데이터 보존: 20년 @ 55°C

권장 사항:
• 자주 쓰는 데이터에는 부적합
• 설정 파일, 로그 다운로드 용도로 사용
• 대용량/빈번한 쓰기는 SD카드 사용
```

### 메모리 맵

```
STM32F767 Flash Memory Map:
┌──────────────────────────────────────────┐
│ Sector 0-5: 프로그램 영역 (사용 금지)     │
│ 0x08000000 - 0x0803FFFF (256KB)          │
├──────────────────────────────────────────┤
│ Sector 6: USB Storage 영역 ✅            │
│ 0x08040000 - 0x0807FFFF (256KB)          │
│ (이 예제에서는 64KB만 사용)              │
├──────────────────────────────────────────┤
│ Sector 7-11: 예비 영역                   │
│ 0x08080000 - 0x081FFFFF                  │
└──────────────────────────────────────────┘
```

## 📈 성능 사양

| 항목 | 사양 |
|------|------|
| USB 속도 | Full Speed (12 Mbps) |
| 실제 전송률 | ~800 KB/s ~ 1 MB/s |
| 저장 용량 | 64 KB (내부 Flash) / GB 단위 (SD카드) |
| 패킷 크기 | 512 bytes |
| 전류 소비 | ~50 mA (USB 연결 시) |

## 🔍 트러블슈팅

### PC에서 드라이브가 인식되지 않는 경우

- [ ] USB 클럭이 정확히 48MHz인지 확인
- [ ] USB_DEVICE 미들웨어에서 MSC Class 선택 확인
- [ ] `MX_USB_DEVICE_Init()` 호출 확인
- [ ] USB 케이블이 데이터 전송 가능한 케이블인지 확인

### "포맷하시겠습니까?" 메시지가 나오는 경우

- [ ] 정상 동작! 최초 사용 시 포맷 필요
- [ ] FAT12 또는 FAT16으로 포맷 (용량이 작으므로)

### 쓰기 실패

- [ ] Flash 섹터가 프로그램 영역과 겹치지 않는지 확인
- [ ] `HAL_FLASH_Unlock()` 호출 확인
- [ ] Linker Script에서 해당 영역 제외 확인

### 쓰기 속도가 매우 느린 경우

- [ ] Flash 섹터 삭제가 매번 발생하는지 확인
- [ ] 큰 파일은 SD카드 버전 사용 권장

## 📁 프로젝트 구조

```
09_USB_Device_MSC_Stable/
├── Core/
│   ├── Inc/
│   │   └── main.h
│   └── Src/
│       └── main.c
├── USB_DEVICE/
│   ├── App/
│   │   ├── usb_device.c
│   │   └── usbd_desc.c
│   └── Target/
│       └── usbd_conf.c
├── Middlewares/
│   └── ST/
│       └── STM32_USB_Device_Library/
│           └── Class/
│               └── MSC/
│                   └── Src/
│                       └── usbd_msc_storage_template.c
├── Drivers/
├── 09_USB_Device_MSC_Stable.ioc
└── README.md
```

## 📚 참고 자료

- [UM1734: STM32Cube USB Device Library](https://www.st.com/resource/en/user_manual/um1734-stm32cube-usb-device-library-stmicroelectronics.pdf)
- [AN4879: USB hardware and PCB guidelines](https://www.st.com/resource/en/application_note/an4879-usb-hardware-and-pcb-guidelines-using-stm32-mcus-stmicroelectronics.pdf)
- [STM32F767 Reference Manual - Flash Programming](https://www.st.com/resource/en/reference_manual/rm0410-stm32f76xxx-and-stm32f77xxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)

## 📝 라이선스

This project is licensed under the MIT License.

## ✍️ Author

Created for STM32 embedded systems learning and development.
