# NUCLEO-F767ZI USB Device MSC - 고속 대용량 전송 모드

STM32 NUCLEO-F767ZI 보드를 USB 메모리 장치로 인식시켜 PC와 **고속으로 대용량 파일**을 교환하는 예제입니다.
이 버전은 **USB High Speed + SDMMC + DMA**를 사용하여 최대 전송 속도에 초점을 맞춥니다.

## 📋 프로젝트 개요

| 항목 | 내용 |
|------|------|
| 보드 | NUCLEO-F767ZI |
| MCU | STM32F767ZIT6 (ARM Cortex-M7, 216MHz) |
| IDE | STM32CubeIDE |
| USB 모드 | **USB Device (High Speed 480Mbps)** |
| 저장매체 | **SD카드 (SDMMC 4-bit + DMA)** |
| 특징 | **최대 속도**, 대용량, 실시간 데이터 전송 |

## 🎯 이 예제의 특징

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      고속 대용량 전송 모드 특징                               │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   ✅ 장점                              ❌ 단점                              │
│   ─────────────────────────────────    ─────────────────────────────────   │
│   • USB High Speed (480 Mbps)          • 외부 USB HS PHY 필요              │
│   • SDMMC 4-bit 모드 (최대 50MB/s)     • 복잡한 하드웨어 구성              │
│   • DMA 사용 (CPU 부하 최소)           • 전력 소비 증가                    │
│   • 대용량 지원 (GB~TB)                • 디버깅 어려움                     │
│   • 실시간 데이터 스트리밍 가능                                             │
│                                                                             │
│   권장 용도:                                                                │
│   • 대용량 로그 데이터 전송 (GB 단위)                                       │
│   • 실시간 센서 데이터 저장                                                │
│   • 고속 데이터 수집 시스템                                                │
│   • 영상/음성 데이터 저장                                                  │
│                                                                             │
│   예상 전송 속도:                                                           │
│   • USB HS + SDMMC: 15~25 MB/s (읽기)                                      │
│   • USB HS + SDMMC: 10~20 MB/s (쓰기)                                      │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 🔧 하드웨어 구성

### USB High Speed 옵션

NUCLEO-F767ZI는 USB HS를 지원하지만, **외부 PHY**가 필요합니다.

#### Option A: USB HS with External ULPI PHY (최대 성능)

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    USB High Speed + ULPI PHY 구성                            │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   NUCLEO-F767ZI                    USB3300 ULPI PHY Module                 │
│   ┌─────────────────┐              ┌─────────────────┐                     │
│   │                 │              │                 │                     │
│   │  PA3  (D0)      │─────────────▶│  DATA0          │                     │
│   │  PB0  (D1)      │─────────────▶│  DATA1          │                     │
│   │  PB1  (D2)      │─────────────▶│  DATA2          │                     │
│   │  PB10 (D3)      │─────────────▶│  DATA3          │                     │
│   │  PB11 (D4)      │─────────────▶│  DATA4          │                     │
│   │  PB12 (D5)      │─────────────▶│  DATA5          │                     │
│   │  PB13 (D6)      │─────────────▶│  DATA6          │                     │
│   │  PB5  (D7)      │─────────────▶│  DATA7          │                     │
│   │                 │              │                 │                     │
│   │  PA5  (CLK)     │◀─────────────│  CLK (60MHz)    │                     │
│   │  PC0  (STP)     │─────────────▶│  STP            │                     │
│   │  PC2  (DIR)     │◀─────────────│  DIR            │                     │
│   │  PC3  (NXT)     │◀─────────────│  NXT            │                     │
│   │                 │              │                 │      USB Type-A    │
│   │  3.3V           │─────────────▶│  VCC            │──────▶  또는       │
│   │  GND            │─────────────▶│  GND            │       Type-C      │
│   │                 │              │                 │                     │
│   └─────────────────┘              └─────────────────┘                     │
│                                                                             │
│   전송 속도: ~20-25 MB/s (읽기), ~15-20 MB/s (쓰기)                        │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

#### Option B: USB FS + SDMMC (외부 PHY 없이 고속화)

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    USB Full Speed + SDMMC (PHY 없이)                        │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   이 구성은 USB 자체는 12Mbps이지만,                                        │
│   SDMMC + DMA 최적화로 안정적인 고속 전송 달성                              │
│                                                                             │
│   NUCLEO-F767ZI                    SD Card (Direct)                        │
│   ┌─────────────────┐              ┌─────────────────┐                     │
│   │                 │              │                 │                     │
│   │  USB FS         │              │                 │                     │
│   │  PA11/PA12      │─── PC ───    │  ┌───────────┐  │                     │
│   │                 │              │  │  SD Card  │  │                     │
│   │  SDMMC1         │              │  │           │  │                     │
│   │  PC8  (D0)      │─────────────▶│  │  DAT0     │  │                     │
│   │  PC9  (D1)      │─────────────▶│  │  DAT1     │  │                     │
│   │  PC10 (D2)      │─────────────▶│  │  DAT2     │  │                     │
│   │  PC11 (D3)      │─────────────▶│  │  DAT3     │  │                     │
│   │  PC12 (CLK)     │─────────────▶│  │  CLK      │  │                     │
│   │  PD2  (CMD)     │─────────────▶│  │  CMD      │  │                     │
│   │                 │              │  └───────────┘  │                     │
│   │  3.3V           │─────────────▶│  VCC            │                     │
│   │  GND            │─────────────▶│  GND            │                     │
│   │                 │              │                 │                     │
│   └─────────────────┘              └─────────────────┘                     │
│                                                                             │
│   전송 속도: ~1 MB/s (USB FS 한계, 하지만 안정적)                           │
│   장점: 외부 PHY 불필요, SDMMC로 SD카드 접근은 고속                         │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### SDMMC 핀 매핑 (공통)

| 기능 | GPIO | 설명 |
|------|------|------|
| SDMMC1_D0 | PC8 | Data Line 0 |
| SDMMC1_D1 | PC9 | Data Line 1 |
| SDMMC1_D2 | PC10 | Data Line 2 |
| SDMMC1_D3 | PC11 | Data Line 3 |
| SDMMC1_CK | PC12 | Clock (최대 50MHz) |
| SDMMC1_CMD | PD2 | Command Line |
| SD_Detect | PG2 | 카드 감지 (선택) |

### 필요한 부품

| 부품 | 용도 | 비고 |
|------|------|------|
| SD카드 슬롯 모듈 | SDMMC 직결 | Micro SD 또는 표준 SD |
| SD카드 | Class 10 / UHS-I 권장 | 빠른 카드일수록 좋음 |
| USB3300 모듈 | USB HS PHY (Option A) | AliExpress 등에서 구매 |

## ⚙️ CubeMX 설정

### 1. RCC 설정

**Pinout & Configuration → System Core → RCC**

| 항목 | 설정값 |
|------|--------|
| HSE | **BYPASS Clock Source** |

**Clock Configuration:**

| 파라미터 | 값 |
|----------|-----|
| SYSCLK | 216 MHz |
| HCLK | 216 MHz |
| SDMMC1 Clock | 48 MHz |
| **USB Clock** | **48 MHz** |

### 2. SDMMC1 설정

**Pinout & Configuration → Connectivity → SDMMC1**

| 항목 | 설정값 |
|------|--------|
| Mode | **SD 4 bits Wide bus** |

#### Parameter Settings

| 파라미터 | 값 | 설명 |
|----------|-----|------|
| Clock Division Factor | 0 | 최대 속도 (SDMMC_CLK/2) |
| Hardware Flow Control | Enable | 오버런 방지 |
| Clock Edge | Rising Edge | |
| Clock Power Saving | Enable | |

#### DMA Settings

| DMA Request | Stream | Direction | Priority | Mode |
|-------------|--------|-----------|----------|------|
| SDMMC1_RX | DMA2 Stream 3 | Peripheral to Memory | High | PFCTRL |
| SDMMC1_TX | DMA2 Stream 6 | Memory to Peripheral | High | PFCTRL |

> ⚠️ **PFCTRL (Peripheral Flow Control)** 모드 필수!

#### NVIC Settings

| 인터럽트 | Enable |
|----------|--------|
| SDMMC1 global interrupt | ✅ |
| DMA2 Stream3 global interrupt | ✅ |
| DMA2 Stream6 global interrupt | ✅ |

### 3. USB_OTG_HS 설정 (Option A: 외부 PHY 사용 시)

**Pinout & Configuration → Connectivity → USB_OTG_HS**

| 항목 | 설정값 |
|------|--------|
| External Phy | **Device_Only** |

#### Parameter Settings

| 파라미터 | 값 |
|----------|-----|
| Speed | High Speed |
| Phy Interface | ULPI |

### 3-B. USB_OTG_FS 설정 (Option B: 외부 PHY 없이)

| 항목 | 설정값 |
|------|--------|
| Mode | **Device_Only** |

### 4. USB_DEVICE 설정

**Pinout & Configuration → Middleware → USB_DEVICE**

| 항목 | 설정값 |
|------|--------|
| Class For HS/FS IP | **Mass Storage Class (MSC)** |

#### Parameter Settings

| 파라미터 | 값 | 설명 |
|----------|-----|------|
| MSC_MEDIA_PACKET | **16384** | 큰 패킷 = 고속 전송 |
| MSC Interface String | "STM32 High Speed Storage" | |

> 💡 **MSC_MEDIA_PACKET을 16KB로 늘리면 전송 효율 대폭 향상!**

### 5. FATFS 설정 (선택사항 - STM32에서 파일 접근 필요 시)

| 항목 | 설정값 |
|------|--------|
| SD Card | ✅ Enable |
| USE_LFN | Enabled with dynamic working buffer on the HEAP |
| MAX_SS | 4096 |
| USE_DMA_TEMPLATE | ✅ Enable |

### 6. 코드 생성

**Ctrl+S** 또는 **Project → Generate Code**

## 💻 소스 코드

### usbd_storage_if.c (SDMMC + DMA)

```c
/* USER CODE BEGIN Header */
/**
  * NUCLEO-F767ZI USB Device MSC - High Speed Transfer
  * SDMMC 4-bit + DMA for maximum throughput
  */
/* USER CODE END Header */

/* USER CODE BEGIN INCLUDE */
#include "main.h"
#include <string.h>
/* USER CODE END INCLUDE */

/* USER CODE BEGIN PRIVATE_DEFINES */
#define STORAGE_LUN_NBR         1
#define STORAGE_BLK_SIZ         512
/* USER CODE END PRIVATE_DEFINES */

/* USER CODE BEGIN PRIVATE_VARIABLES */
extern SD_HandleTypeDef hsd1;
static volatile uint8_t sd_ready = 0;
static uint32_t sd_block_count = 0;
static uint32_t sd_block_size = STORAGE_BLK_SIZ;

// 전송 상태
static volatile uint8_t tx_complete = 1;
static volatile uint8_t rx_complete = 1;
/* USER CODE END PRIVATE_VARIABLES */

/* USER CODE BEGIN 0 */
/**
  * @brief  SD카드 DMA 전송 완료 콜백
  */
void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
    tx_complete = 1;
}

void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
    rx_complete = 1;
}

/**
  * @brief  DMA 전송 대기 (타임아웃 포함)
  */
static int8_t Wait_SDMMC_Ready(uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    
    while (HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER)
    {
        if ((HAL_GetTick() - start) > timeout_ms)
        {
            return -1;
        }
    }
    return 0;
}
/* USER CODE END 0 */

/**
  * @brief  Initializes the storage unit
  */
int8_t STORAGE_Init_FS(uint8_t lun)
{
    /* USER CODE BEGIN 2 */
    UNUSED(lun);
    
    HAL_SD_CardInfoTypeDef card_info;
    
    // SD카드 정보 읽기
    if (HAL_SD_GetCardInfo(&hsd1, &card_info) != HAL_OK)
    {
        sd_ready = 0;
        return USBD_FAIL;
    }
    
    sd_block_count = card_info.BlockNbr;
    sd_block_size = card_info.BlockSize;
    sd_ready = 1;
    
    return USBD_OK;
    /* USER CODE END 2 */
}

/**
  * @brief  Returns the medium capacity
  */
int8_t STORAGE_GetCapacity_FS(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
    /* USER CODE BEGIN 3 */
    UNUSED(lun);
    
    if (!sd_ready)
    {
        return USBD_FAIL;
    }
    
    *block_num = sd_block_count;
    *block_size = sd_block_size;
    
    return USBD_OK;
    /* USER CODE END 3 */
}

/**
  * @brief  Checks whether the medium is ready
  */
int8_t STORAGE_IsReady_FS(uint8_t lun)
{
    /* USER CODE BEGIN 4 */
    UNUSED(lun);
    
    if (!sd_ready)
    {
        return USBD_FAIL;
    }
    
    if (HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER)
    {
        return USBD_FAIL;
    }
    
    return USBD_OK;
    /* USER CODE END 4 */
}

/**
  * @brief  Checks whether the medium is write protected
  */
int8_t STORAGE_IsWriteProtected_FS(uint8_t lun)
{
    /* USER CODE BEGIN 5 */
    UNUSED(lun);
    return USBD_OK;  // 쓰기 가능
    /* USER CODE END 5 */
}

/**
  * @brief  Reads data from the medium (DMA)
  */
int8_t STORAGE_Read_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
    /* USER CODE BEGIN 6 */
    UNUSED(lun);
    
    if (!sd_ready)
    {
        return USBD_FAIL;
    }
    
    // 카드 상태 대기
    if (Wait_SDMMC_Ready(1000) != 0)
    {
        return USBD_FAIL;
    }
    
    // DMA 읽기
    rx_complete = 0;
    
    if (HAL_SD_ReadBlocks_DMA(&hsd1, buf, blk_addr, blk_len) != HAL_OK)
    {
        return USBD_FAIL;
    }
    
    // DMA 완료 대기
    uint32_t timeout = HAL_GetTick();
    while (!rx_complete)
    {
        if ((HAL_GetTick() - timeout) > 5000)
        {
            return USBD_FAIL;
        }
    }
    
    // 전송 완료 대기
    if (Wait_SDMMC_Ready(1000) != 0)
    {
        return USBD_FAIL;
    }
    
    return USBD_OK;
    /* USER CODE END 6 */
}

/**
  * @brief  Writes data into the medium (DMA)
  */
int8_t STORAGE_Write_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
    /* USER CODE BEGIN 7 */
    UNUSED(lun);
    
    if (!sd_ready)
    {
        return USBD_FAIL;
    }
    
    // 카드 상태 대기
    if (Wait_SDMMC_Ready(1000) != 0)
    {
        return USBD_FAIL;
    }
    
    // DMA 쓰기
    tx_complete = 0;
    
    if (HAL_SD_WriteBlocks_DMA(&hsd1, buf, blk_addr, blk_len) != HAL_OK)
    {
        return USBD_FAIL;
    }
    
    // DMA 완료 대기
    uint32_t timeout = HAL_GetTick();
    while (!tx_complete)
    {
        if ((HAL_GetTick() - timeout) > 5000)
        {
            return USBD_FAIL;
        }
    }
    
    // 전송 완료 대기
    if (Wait_SDMMC_Ready(1000) != 0)
    {
        return USBD_FAIL;
    }
    
    return USBD_OK;
    /* USER CODE END 7 */
}

/**
  * @brief  Returns the Max Supported LUNs
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
#include <string.h>
#include "usb_device.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
extern USBD_HandleTypeDef hUsbDeviceHS;  // HS 사용 시
// extern USBD_HandleTypeDef hUsbDeviceFS;  // FS 사용 시

extern SD_HandleTypeDef hsd1;
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

/**
  * @brief  SD카드 정보 출력
  */
void Print_SD_Info(void)
{
    HAL_SD_CardInfoTypeDef card_info;
    
    if (HAL_SD_GetCardInfo(&hsd1, &card_info) == HAL_OK)
    {
        printf("\r\n=== SD Card Information ===\r\n");
        printf("Card Type: ");
        switch (card_info.CardType)
        {
            case CARD_SDSC: printf("SDSC\r\n"); break;
            case CARD_SDHC_SDXC: printf("SDHC/SDXC\r\n"); break;
            default: printf("Unknown\r\n"); break;
        }
        printf("Card Capacity: %lu MB\r\n", 
               (uint32_t)((uint64_t)card_info.BlockNbr * card_info.BlockSize / 1024 / 1024));
        printf("Block Size: %lu bytes\r\n", card_info.BlockSize);
        printf("Block Count: %lu\r\n", card_info.BlockNbr);
        printf("===========================\r\n\n");
    }
    else
    {
        printf("SD Card info read failed!\r\n");
    }
}

/**
  * @brief  벤치마크 테스트 (SD카드 직접 읽기/쓰기 속도)
  */
void Run_SD_Benchmark(void)
{
    #define BENCH_BUFFER_SIZE   (32 * 1024)  // 32KB
    #define BENCH_ITERATIONS    100
    
    static uint8_t __attribute__((aligned(4))) bench_buffer[BENCH_BUFFER_SIZE];
    uint32_t start_tick, elapsed;
    float speed;
    
    printf("=== SD Card Benchmark ===\r\n");
    
    // 버퍼 초기화
    memset(bench_buffer, 0xAA, BENCH_BUFFER_SIZE);
    
    // 쓰기 테스트
    printf("Write test (%d KB × %d)...\r\n", BENCH_BUFFER_SIZE/1024, BENCH_ITERATIONS);
    start_tick = HAL_GetTick();
    
    for (int i = 0; i < BENCH_ITERATIONS; i++)
    {
        // 시작 블록을 변경하며 쓰기 (실제 데이터 덮어쓰기 주의!)
        uint32_t block_addr = 1000 + (i * (BENCH_BUFFER_SIZE / 512));
        HAL_SD_WriteBlocks_DMA(&hsd1, bench_buffer, block_addr, BENCH_BUFFER_SIZE / 512);
        
        // 완료 대기
        while (HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER);
    }
    
    elapsed = HAL_GetTick() - start_tick;
    speed = (float)(BENCH_BUFFER_SIZE * BENCH_ITERATIONS) / elapsed / 1000.0f;
    printf("Write Speed: %.2f MB/s (%lu ms)\r\n", speed, elapsed);
    
    // 읽기 테스트
    printf("Read test (%d KB × %d)...\r\n", BENCH_BUFFER_SIZE/1024, BENCH_ITERATIONS);
    start_tick = HAL_GetTick();
    
    for (int i = 0; i < BENCH_ITERATIONS; i++)
    {
        uint32_t block_addr = 1000 + (i * (BENCH_BUFFER_SIZE / 512));
        HAL_SD_ReadBlocks_DMA(&hsd1, bench_buffer, block_addr, BENCH_BUFFER_SIZE / 512);
        
        while (HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER);
    }
    
    elapsed = HAL_GetTick() - start_tick;
    speed = (float)(BENCH_BUFFER_SIZE * BENCH_ITERATIONS) / elapsed / 1000.0f;
    printf("Read Speed: %.2f MB/s (%lu ms)\r\n", speed, elapsed);
    
    printf("=========================\r\n\n");
}

/* USER CODE END 0 */

int main(void)
{
    /* MCU Configuration */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_USART3_UART_Init();
    MX_SDMMC1_SD_Init();
    MX_USB_DEVICE_Init();

    /* USER CODE BEGIN 2 */
    printf("\r\n============================================\r\n");
    printf("  NUCLEO-F767ZI USB Device MSC Demo\r\n");
    printf("  Mode: High Speed Transfer\r\n");
    printf("  Storage: SD Card (SDMMC 4-bit + DMA)\r\n");
    printf("  System Clock: %lu MHz\r\n", HAL_RCC_GetSysClockFreq() / 1000000);
    printf("============================================\r\n\n");

    // SD카드 정보 출력
    Print_SD_Info();
    
    // 벤치마크 (선택사항)
    // Run_SD_Benchmark();

    // LED 초기화
    HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);  // Green ON
    
    printf("Connect USB cable to PC...\r\n\n");
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    uint8_t prev_state = 0xFF;
    
    while (1)
    {
        // USB 연결 상태 모니터링
        uint8_t current_state = hUsbDeviceHS.dev_state;  // HS 사용 시
        // uint8_t current_state = hUsbDeviceFS.dev_state;  // FS 사용 시
        
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
                    printf("USB State: CONFIGURED - Ready for HIGH SPEED transfer!\r\n");
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
        
        // 전송 중 LED 토글 (활동 표시)
        static uint32_t led_tick = 0;
        if (HAL_GetTick() - led_tick > 500)
        {
            led_tick = HAL_GetTick();
            if (current_state == USBD_STATE_CONFIGURED)
            {
                HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
            }
        }
        
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}
```

### stm32f7xx_it.c (인터럽트 핸들러)

```c
/* USER CODE BEGIN 1 */
extern SD_HandleTypeDef hsd1;
/* USER CODE END 1 */

/**
  * @brief This function handles DMA2 stream3 global interrupt (SDMMC RX)
  */
void DMA2_Stream3_IRQHandler(void)
{
    HAL_DMA_IRQHandler(hsd1.hdmarx);
}

/**
  * @brief This function handles DMA2 stream6 global interrupt (SDMMC TX)
  */
void DMA2_Stream6_IRQHandler(void)
{
    HAL_DMA_IRQHandler(hsd1.hdmatx);
}

/**
  * @brief This function handles SDMMC1 global interrupt
  */
void SDMMC1_IRQHandler(void)
{
    HAL_SD_IRQHandler(&hsd1);
}
```

## 📊 성능 최적화 팁

### 1. MSC_MEDIA_PACKET 크기 조정

```c
// usbd_conf.h 또는 CubeMX에서 설정
#define MSC_MEDIA_PACKET    16384   // 16KB (기본 512)

// 더 큰 값 = 더 빠른 전송 (메모리 사용 증가)
// 추천 값: 4096, 8192, 16384, 32768
```

### 2. 캐시 최적화 (Cortex-M7)

```c
// main.c - SystemClock_Config() 후
SCB_EnableICache();  // 명령 캐시 활성화
SCB_EnableDCache();  // 데이터 캐시 활성화

// DMA 버퍼는 캐시 정렬 필요
__attribute__((aligned(32))) uint8_t dma_buffer[BUFFER_SIZE];

// DMA 전송 전 캐시 클린
SCB_CleanDCache_by_Addr((uint32_t*)dma_buffer, BUFFER_SIZE);

// DMA 수신 후 캐시 무효화
SCB_InvalidateDCache_by_Addr((uint32_t*)dma_buffer, BUFFER_SIZE);
```

### 3. SDMMC 클럭 최대화

```c
// CubeMX에서 Clock Division = 0 (최대 속도)
// 또는 코드에서 직접 설정
hsd1.Init.ClockDiv = 0;  // SDMMC_CLK / 2 = 24MHz
```

### 4. 더블 버퍼링 (고급)

```c
// 두 개의 버퍼를 번갈아 사용하여 지연 최소화
static uint8_t buffer_a[16384] __attribute__((aligned(32)));
static uint8_t buffer_b[16384] __attribute__((aligned(32)));
static uint8_t *active_buffer = buffer_a;

// 하나의 버퍼로 DMA 전송하는 동안 다른 버퍼 준비
```

## 📈 성능 비교

| 구성 | 읽기 속도 | 쓰기 속도 | 비고 |
|------|----------|----------|------|
| USB FS + SPI SD | ~0.5 MB/s | ~0.3 MB/s | 느림 |
| USB FS + SDMMC | ~1 MB/s | ~0.8 MB/s | USB 병목 |
| **USB HS + SDMMC** | **~20 MB/s** | **~15 MB/s** | **최고 성능** |
| USB HS + SDMMC + DCache | ~23 MB/s | ~18 MB/s | 캐시 최적화 |

### 실제 테스트 결과 (예상)

```
============================================
  NUCLEO-F767ZI USB Device MSC Demo
  Mode: High Speed Transfer
  Storage: SD Card (SDMMC 4-bit + DMA)
  System Clock: 216 MHz
============================================

=== SD Card Information ===
Card Type: SDHC/SDXC
Card Capacity: 31914 MB
Block Size: 512 bytes
Block Count: 62333952
===========================

=== SD Card Benchmark ===
Write test (32 KB × 100)...
Write Speed: 18.52 MB/s (173 ms)
Read test (32 KB × 100)...
Read Speed: 22.86 MB/s (140 ms)
=========================

Connect USB cable to PC...

USB State: DEFAULT
USB State: ADDRESSED
USB State: CONFIGURED - Ready for HIGH SPEED transfer!
```

## 🔍 트러블슈팅

### SD카드 인식 안 됨

- [ ] SDMMC 핀 연결 확인 (특히 CMD, CLK)
- [ ] SD카드 전원 (3.3V) 확인
- [ ] SD카드 포맷 (FAT32) 확인
- [ ] 다른 SD카드로 테스트

### DMA 전송 실패

- [ ] DMA 스트림 설정 확인 (PFCTRL 모드)
- [ ] NVIC에서 DMA 인터럽트 활성화 확인
- [ ] 버퍼 정렬 확인 (4바이트 또는 32바이트)
- [ ] 캐시 사용 시 Clean/Invalidate 호출 확인

### USB HS 인식 안 됨

- [ ] ULPI PHY 연결 확인 (모든 12핀)
- [ ] PHY 전원 (3.3V) 확인
- [ ] CubeMX에서 ULPI 모드 선택 확인

### 전송 속도가 느림

- [ ] MSC_MEDIA_PACKET 크기 확인 (16KB 권장)
- [ ] SDMMC Clock Division 확인 (0 = 최대 속도)
- [ ] SD카드 등급 확인 (Class 10 / UHS-I 권장)
- [ ] USB 케이블 품질 확인 (USB 3.0 케이블 권장)

## 📁 프로젝트 구조

```
10_USB_Device_MSC_HighSpeed/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   └── stm32f7xx_it.h
│   └── Src/
│       ├── main.c
│       ├── stm32f7xx_it.c
│       └── stm32f7xx_hal_msp.c
├── USB_DEVICE/
│   ├── App/
│   │   ├── usb_device.c
│   │   ├── usbd_desc.c
│   │   └── usbd_storage_if.c      # ⭐ 핵심 수정 파일
│   └── Target/
│       └── usbd_conf.c
├── Middlewares/
│   └── ST/
│       └── STM32_USB_Device_Library/
├── Drivers/
├── 10_USB_Device_MSC_HighSpeed.ioc
└── README.md
```

## 📚 참고 자료

- [UM1734: STM32Cube USB Device Library](https://www.st.com/resource/en/user_manual/um1734-stm32cube-usb-device-library-stmicroelectronics.pdf)
- [AN4879: USB hardware and PCB guidelines](https://www.st.com/resource/en/application_note/an4879-usb-hardware-and-pcb-guidelines-using-stm32-mcus-stmicroelectronics.pdf)
- [AN5601: Getting started with SDMMC on STM32](https://www.st.com/resource/en/application_note/an5601-getting-started-with-sdmmc-on-stm32h7-series-stmicroelectronics.pdf)
- [USB 2.0 Specification](https://www.usb.org/document-library/usb-20-specification)
- [USB3300 ULPI PHY Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/00001783C.pdf)

## 📝 라이선스

This project is licensed under the MIT License.

## ✍️ Author

Created for STM32 embedded systems learning and development.
