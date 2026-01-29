# NUCLEO-F767ZI USB Host MSC (Mass Storage Class)

STM32 NUCLEO-F767ZI 보드의 USB Host 기능을 이용한 USB 메모리(Mass Storage) 읽기/쓰기 예제입니다.

## 📋 프로젝트 개요

| 항목 | 내용 |
|------|------|
| 보드 | NUCLEO-F767ZI |
| MCU | STM32F767ZIT6 (ARM Cortex-M7, 216MHz) |
| IDE | STM32CubeIDE |
| 기능 | USB Host로 USB 메모리 연결, 파일 읽기/쓰기 + USART3 디버그 출력 |

## 🔧 하드웨어 구성

### USB OTG FS 핀 매핑

NUCLEO-F767ZI는 USB OTG FS(Full Speed)를 지원합니다.

| 기능 | GPIO | CN 커넥터 |
|------|------|-----------|
| USB_OTG_FS_DM | PA11 | CN12 Pin 14 |
| USB_OTG_FS_DP | PA12 | CN12 Pin 12 |
| USB_OTG_FS_ID | PA10 | CN12 Pin 16 (선택) |
| USB_OTG_FS_VBUS | PA9 | - (선택) |
| **USB_PowerSwitchOn** | **PG6** | - (전원 제어) |
| **USB_OverCurrent** | **PG7** | - (과전류 감지) |

### 외부 하드웨어 필요

```
┌─────────────────────────────────────────────────────────────────┐
│                    USB Host 연결 구성                            │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   NUCLEO-F767ZI              USB Connector                      │
│   ┌─────────────┐            ┌─────────────┐                   │
│   │             │            │  USB Type-A │                   │
│   │  PA11 (DM)  │────────────│  D-         │                   │
│   │  PA12 (DP)  │────────────│  D+         │                   │
│   │             │            │             │                   │
│   │  5V         │────┬───────│  VBUS (5V)  │                   │
│   │             │    │       │             │                   │
│   │  GND        │────┼───────│  GND        │                   │
│   │             │    │       └─────────────┘                   │
│   │             │    │                                         │
│   │  PG6        │────┴── (전원 스위치, 선택사항)                │
│   │             │                                              │
│   └─────────────┘                                              │
│                                                                 │
│   ※ USB Host 모드에서는 NUCLEO가 5V 전원을 공급해야 함         │
│   ※ 간단한 테스트: USB OTG 케이블 + USB 메모리 직접 연결       │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 필요한 부품

| 부품 | 용도 | 비고 |
|------|------|------|
| USB OTG 케이블 | Micro-B to Type-A Female | Host 모드용 |
| USB 메모리 | FAT32 포맷 | 32GB 이하 권장 |
| (선택) USB 커넥터 보드 | Type-A Female 브레이크아웃 | 직접 배선 시 |

### LED 및 USART

| 기능 | GPIO |
|------|------|
| LD1 (Green) | PB0 |
| LD2 (Blue) | PB7 |
| LD3 (Red) | PB14 |
| USART3_TX | PD8 |
| USART3_RX | PD9 |

## ⚙️ CubeMX 설정

### 1. RCC 설정

**Pinout & Configuration → System Core → RCC**

| 항목 | 설정값 |
|------|--------|
| HSE | **BYPASS Clock Source** |

**Clock Configuration:**

| 파라미터 | 값 | 비고 |
|----------|-----|------|
| SYSCLK | 216 MHz | |
| HCLK | 216 MHz | |
| APB1 | 54 MHz | |
| APB2 | 108 MHz | |
| **USB Clock** | **48 MHz** | ⚠️ 필수! |

> ⚠️ **중요**: USB는 정확히 48MHz 클럭이 필요합니다. Clock Configuration에서 USB 클럭이 48MHz인지 반드시 확인하세요.

### 2. USB_OTG_FS 설정

**Pinout & Configuration → Connectivity → USB_OTG_FS**

| 항목 | 설정값 |
|------|--------|
| Mode | **Host_Only** |
| Activate_VBUS | ✅ Checked (PA9) |
| Activate_SOF | Unchecked |

#### Parameter Settings

| 파라미터 | 값 |
|----------|-----|
| Speed | Full Speed 12MBit/s |
| Signal start of frame | Disabled |
| Low power | Disabled |

### 3. USB_HOST 설정

**Pinout & Configuration → Middleware → USB_HOST**

| 항목 | 설정값 |
|------|--------|
| Class For FS IP | **Mass Storage Host Class** |

#### Parameter Settings - Platform Settings

| 파라미터 | 값 | 설명 |
|----------|-----|------|
| VBUS Driving | GPIO (PG6) | 전원 제어 핀 |
| Overcurrent detection | GPIO (PG7) | 과전류 감지 핀 |

> 💡 VBUS 제어를 사용하지 않으려면 GPIO 대신 "Not Used" 선택

### 4. FATFS 설정

**Pinout & Configuration → Middleware → FATFS**

| 항목 | 설정값 |
|------|--------|
| Mode | **USB Disk** |

#### Set Defines

| 파라미터 | 값 | 설명 |
|----------|-----|------|
| USE_LFN | **Enabled with dynamic working buffer on the HEAP** | 긴 파일명 지원 |
| MAX_SS | 4096 | 최대 섹터 크기 |
| FS_EXFAT | Disabled | exFAT 비활성화 (FAT32만) |
| CODE_PAGE | Korean (949) | 한글 파일명 지원 |

#### Platform Settings

| 파라미터 | 값 |
|----------|-----|
| Use dma template | Disabled |

### 5. USART3 설정

| 항목 | 설정값 |
|------|--------|
| Mode | Asynchronous |
| Baud Rate | 115200 |

### 6. GPIO 설정 (LED)

| 핀 | Mode | User Label |
|----|------|------------|
| PB0 | Output Push Pull | LD1 |
| PB7 | Output Push Pull | LD2 |
| PB14 | Output Push Pull | LD3 |

### 7. NVIC 설정

**Pinout & Configuration → System Core → NVIC**

| 인터럽트 | Enable | Priority |
|----------|--------|----------|
| USB On The Go FS global interrupt | ✅ | 5 |

### 8. 코드 생성

**Ctrl+S** 또는 **Project → Generate Code**

## 💻 소스 코드

### main.c

```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "fatfs.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
extern ApplicationTypeDef Appli_state;

FATFS USBDISKFatFs;    // 파일 시스템 객체
FIL MyFile;            // 파일 객체
char USBDISKPath[4];   // USB 디스크 경로
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
  * @brief  USB 연결 상태 문자열 반환
  */
const char* Get_Application_State_String(ApplicationTypeDef state)
{
    switch (state)
    {
        case APPLICATION_IDLE:         return "IDLE";
        case APPLICATION_START:        return "START";
        case APPLICATION_READY:        return "READY";
        case APPLICATION_DISCONNECT:   return "DISCONNECT";
        default:                       return "UNKNOWN";
    }
}

/**
  * @brief  USB 메모리 테스트 - 파일 쓰기
  */
FRESULT USB_Write_Test(void)
{
    FRESULT res;
    uint32_t byteswritten;
    char write_buffer[] = "Hello from STM32F767ZI USB Host!\r\n"
                          "This is a test file.\r\n"
                          "NUCLEO-F767ZI USB MSC Example.\r\n";

    printf("Creating test file...\r\n");

    // 파일 생성 (덮어쓰기)
    res = f_open(&MyFile, "STM32_TEST.TXT", FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK)
    {
        printf("f_open error: %d\r\n", res);
        return res;
    }

    // 데이터 쓰기
    res = f_write(&MyFile, write_buffer, strlen(write_buffer), (UINT *)&byteswritten);
    if (res != FR_OK)
    {
        printf("f_write error: %d\r\n", res);
        f_close(&MyFile);
        return res;
    }

    printf("Written %lu bytes to STM32_TEST.TXT\r\n", byteswritten);

    // 파일 닫기
    f_close(&MyFile);

    return FR_OK;
}

/**
  * @brief  USB 메모리 테스트 - 파일 읽기
  */
FRESULT USB_Read_Test(void)
{
    FRESULT res;
    uint32_t bytesread;
    char read_buffer[256];

    printf("Reading test file...\r\n");

    // 파일 열기
    res = f_open(&MyFile, "STM32_TEST.TXT", FA_READ);
    if (res != FR_OK)
    {
        printf("f_open error: %d\r\n", res);
        return res;
    }

    // 데이터 읽기
    memset(read_buffer, 0, sizeof(read_buffer));
    res = f_read(&MyFile, read_buffer, sizeof(read_buffer) - 1, (UINT *)&bytesread);
    if (res != FR_OK)
    {
        printf("f_read error: %d\r\n", res);
        f_close(&MyFile);
        return res;
    }

    printf("Read %lu bytes:\r\n", bytesread);
    printf("─────────────────────────────────\r\n");
    printf("%s", read_buffer);
    printf("─────────────────────────────────\r\n");

    // 파일 닫기
    f_close(&MyFile);

    return FR_OK;
}

/**
  * @brief  USB 메모리 정보 출력
  */
FRESULT USB_Print_Disk_Info(void)
{
    FRESULT res;
    FATFS *fs;
    DWORD fre_clust, fre_sect, tot_sect;

    // 여유 클러스터 수 얻기
    res = f_getfree(USBDISKPath, &fre_clust, &fs);
    if (res != FR_OK)
    {
        printf("f_getfree error: %d\r\n", res);
        return res;
    }

    // 총 섹터 수 및 여유 섹터 수 계산
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    fre_sect = fre_clust * fs->csize;

    printf("\r\n=== USB Disk Information ===\r\n");
    printf("Total: %10lu KB (%lu MB)\r\n", tot_sect / 2, tot_sect / 2048);
    printf("Free:  %10lu KB (%lu MB)\r\n", fre_sect / 2, fre_sect / 2048);
    printf("Used:  %10lu KB (%lu MB)\r\n", (tot_sect - fre_sect) / 2, 
           (tot_sect - fre_sect) / 2048);
    printf("============================\r\n\n");

    return FR_OK;
}

/**
  * @brief  디렉토리 내용 출력
  */
FRESULT USB_List_Directory(const char *path)
{
    FRESULT res;
    DIR dir;
    FILINFO fno;

    printf("\r\n=== Directory: %s ===\r\n", path);

    res = f_opendir(&dir, path);
    if (res != FR_OK)
    {
        printf("f_opendir error: %d\r\n", res);
        return res;
    }

    while (1)
    {
        res = f_readdir(&dir, &fno);
        if (res != FR_OK || fno.fname[0] == 0)
            break;

        if (fno.fattrib & AM_DIR)
        {
            // 디렉토리
            printf("  [DIR]  %s\r\n", fno.fname);
        }
        else
        {
            // 파일
            printf("  %8lu  %s\r\n", fno.fsize, fno.fname);
        }
    }

    f_closedir(&dir);
    printf("=============================\r\n\n");

    return FR_OK;
}

/* USER CODE END 0 */

int main(void)
{
    /* MCU Configuration */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART3_UART_Init();
    MX_FATFS_Init();
    MX_USB_HOST_Init();

    /* USER CODE BEGIN 2 */
    printf("\r\n============================================\r\n");
    printf("  NUCLEO-F767ZI USB Host MSC Demo\r\n");
    printf("  System Clock: %lu MHz\r\n", HAL_RCC_GetSysClockFreq() / 1000000);
    printf("============================================\r\n");
    printf("Waiting for USB device...\r\n\n");

    // LED 초기화 - 대기 상태
    HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);  // Red ON
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    ApplicationTypeDef prev_state = APPLICATION_IDLE;
    uint8_t test_done = 0;

    while (1)
    {
        // USB Host 프로세스 (필수!)
        MX_USB_HOST_Process();

        // 상태 변경 감지
        if (Appli_state != prev_state)
        {
            printf("USB State: %s\r\n", Get_Application_State_String(Appli_state));
            prev_state = Appli_state;

            // LED 상태 업데이트
            switch (Appli_state)
            {
                case APPLICATION_READY:
                    HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);    // Green ON
                    HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);  // Red OFF
                    break;

                case APPLICATION_DISCONNECT:
                    HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);  // Green OFF
                    HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);    // Red ON
                    test_done = 0;  // 다시 연결하면 테스트 가능
                    break;

                default:
                    break;
            }
        }

        // USB 메모리가 준비되면 테스트 수행
        if (Appli_state == APPLICATION_READY && !test_done)
        {
            printf("\r\n>>> USB Device Ready! Starting test... <<<\r\n\n");

            // 마운트
            FRESULT res = f_mount(&USBDISKFatFs, USBDISKPath, 1);
            if (res != FR_OK)
            {
                printf("f_mount error: %d\r\n", res);
            }
            else
            {
                printf("USB Disk mounted successfully!\r\n");

                // 디스크 정보 출력
                USB_Print_Disk_Info();

                // 루트 디렉토리 목록
                USB_List_Directory("/");

                // 파일 쓰기 테스트
                if (USB_Write_Test() == FR_OK)
                {
                    HAL_Delay(100);

                    // 파일 읽기 테스트
                    USB_Read_Test();
                }

                // 다시 디렉토리 목록 (새 파일 확인)
                USB_List_Directory("/");

                printf(">>> Test completed! <<<\r\n\n");
            }

            test_done = 1;
        }

        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}
```

### usb_host.c 수정 (선택사항)

`USB_HOST/App/usb_host.c` 파일의 `USBH_UserProcess` 함수에서 추가 처리 가능:

```c
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id)
{
    /* USER CODE BEGIN CALL_BACK_1 */
    switch(id)
    {
        case HOST_USER_SELECT_CONFIGURATION:
            break;

        case HOST_USER_DISCONNECTION:
            Appli_state = APPLICATION_DISCONNECT;
            break;

        case HOST_USER_CLASS_ACTIVE:
            Appli_state = APPLICATION_READY;
            break;

        case HOST_USER_CONNECTION:
            Appli_state = APPLICATION_START;
            break;

        default:
            break;
    }
    /* USER CODE END CALL_BACK_1 */
}
```

## 🔄 동작 방식

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         USB Host MSC 동작 흐름                               │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   ┌─────────────┐                                                          │
│   │   USB 대기   │ ← LD3 (Red) ON                                          │
│   └──────┬──────┘                                                          │
│          │ USB 메모리 삽입                                                  │
│          ▼                                                                  │
│   ┌─────────────┐                                                          │
│   │ HOST_USER_  │                                                          │
│   │ CONNECTION  │ → APPLICATION_START                                      │
│   └──────┬──────┘                                                          │
│          │ 열거(Enumeration) 완료                                           │
│          ▼                                                                  │
│   ┌─────────────┐                                                          │
│   │ HOST_USER_  │                                                          │
│   │ CLASS_ACTIVE│ → APPLICATION_READY                                      │
│   └──────┬──────┘                                                          │
│          │                                                                  │
│          ▼                                                                  │
│   ┌─────────────────────────────────────────────────┐                      │
│   │              FATFS 마운트                        │ ← LD1 (Green) ON    │
│   │                     │                           │                      │
│   │   ┌─────────────────┼─────────────────┐         │                      │
│   │   ▼                 ▼                 ▼         │                      │
│   │ f_mount()     f_open()          f_readdir()     │                      │
│   │ f_getfree()   f_write()         f_closedir()    │                      │
│   │               f_read()                          │                      │
│   │               f_close()                         │                      │
│   └─────────────────────────────────────────────────┘                      │
│          │                                                                  │
│          │ USB 메모리 제거                                                  │
│          ▼                                                                  │
│   ┌─────────────┐                                                          │
│   │ HOST_USER_  │                                                          │
│   │DISCONNECTION│ → APPLICATION_DISCONNECT                                 │
│   └─────────────┘ ← LD3 (Red) ON                                           │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 📊 FATFS 에러 코드

| 코드 | 이름 | 설명 |
|------|------|------|
| 0 | FR_OK | 성공 |
| 1 | FR_DISK_ERR | 디스크 I/O 에러 |
| 2 | FR_INT_ERR | 내부 에러 |
| 3 | FR_NOT_READY | 드라이브 준비 안됨 |
| 4 | FR_NO_FILE | 파일 없음 |
| 5 | FR_NO_PATH | 경로 없음 |
| 6 | FR_INVALID_NAME | 잘못된 파일명 |
| 7 | FR_DENIED | 접근 거부 |
| 8 | FR_EXIST | 파일 이미 존재 |
| 9 | FR_INVALID_OBJECT | 잘못된 객체 |
| 10 | FR_WRITE_PROTECTED | 쓰기 금지 |
| 11 | FR_INVALID_DRIVE | 잘못된 드라이브 |
| 12 | FR_NOT_ENABLED | 볼륨 마운트 안됨 |
| 13 | FR_NO_FILESYSTEM | 유효한 FAT 없음 |
| 14 | FR_MKFS_ABORTED | f_mkfs 중단됨 |
| 15 | FR_TIMEOUT | 타임아웃 |

## 📺 예상 출력

```
============================================
  NUCLEO-F767ZI USB Host MSC Demo
  System Clock: 216 MHz
============================================
Waiting for USB device...

USB State: START
USB State: READY

>>> USB Device Ready! Starting test... <<<

USB Disk mounted successfully!

=== USB Disk Information ===
Total:   15625728 KB (15259 MB)
Free:    15620096 KB (15254 MB)
Used:        5632 KB (5 MB)
============================

=== Directory: / ===
  [DIR]  System Volume Information
      1024  config.txt
      2048  readme.txt
=============================

Creating test file...
Written 89 bytes to STM32_TEST.TXT
Reading test file...
Read 89 bytes:
─────────────────────────────────
Hello from STM32F767ZI USB Host!
This is a test file.
NUCLEO-F767ZI USB MSC Example.
─────────────────────────────────

=== Directory: / ===
  [DIR]  System Volume Information
      1024  config.txt
      2048  readme.txt
        89  STM32_TEST.TXT
=============================

>>> Test completed! <<<
```

## 🔧 고급 기능 예제

### 대용량 파일 쓰기

```c
FRESULT USB_Write_Large_File(const char *filename, uint32_t size_kb)
{
    FRESULT res;
    FIL file;
    uint32_t byteswritten;
    uint8_t buffer[512];
    
    // 버퍼 초기화
    memset(buffer, 'A', sizeof(buffer));
    
    res = f_open(&file, filename, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) return res;
    
    uint32_t total_written = 0;
    uint32_t target_bytes = size_kb * 1024;
    
    while (total_written < target_bytes)
    {
        res = f_write(&file, buffer, sizeof(buffer), (UINT *)&byteswritten);
        if (res != FR_OK) break;
        
        total_written += byteswritten;
        
        // 진행률 출력 (1MB마다)
        if ((total_written % (1024 * 1024)) == 0)
        {
            printf("Written: %lu KB\r\n", total_written / 1024);
        }
    }
    
    f_close(&file);
    printf("Total written: %lu bytes\r\n", total_written);
    
    return res;
}
```

### 파일 복사

```c
FRESULT USB_Copy_File(const char *src, const char *dst)
{
    FRESULT res;
    FIL fsrc, fdst;
    uint8_t buffer[512];
    uint32_t br, bw;
    
    // 소스 파일 열기
    res = f_open(&fsrc, src, FA_READ);
    if (res != FR_OK) return res;
    
    // 대상 파일 생성
    res = f_open(&fdst, dst, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK)
    {
        f_close(&fsrc);
        return res;
    }
    
    // 복사
    while (1)
    {
        res = f_read(&fsrc, buffer, sizeof(buffer), (UINT *)&br);
        if (res != FR_OK || br == 0) break;
        
        res = f_write(&fdst, buffer, br, (UINT *)&bw);
        if (res != FR_OK || bw < br) break;
    }
    
    f_close(&fsrc);
    f_close(&fdst);
    
    return res;
}
```

### 데이터 로깅

```c
FRESULT USB_Log_Data(float temperature, float humidity)
{
    FRESULT res;
    FIL file;
    uint32_t byteswritten;
    char log_line[128];
    
    // 타임스탬프 생성 (HAL_GetTick 사용)
    uint32_t tick = HAL_GetTick();
    uint32_t hours = (tick / 3600000) % 24;
    uint32_t minutes = (tick / 60000) % 60;
    uint32_t seconds = (tick / 1000) % 60;
    
    // 로그 라인 생성
    sprintf(log_line, "%02lu:%02lu:%02lu, %.2f, %.2f\r\n",
            hours, minutes, seconds, temperature, humidity);
    
    // 파일 열기 (추가 모드)
    res = f_open(&file, "datalog.csv", FA_OPEN_APPEND | FA_WRITE);
    if (res != FR_OK) return res;
    
    // 쓰기
    res = f_write(&file, log_line, strlen(log_line), (UINT *)&byteswritten);
    
    f_close(&file);
    
    return res;
}
```

## 🔍 트러블슈팅

### USB 메모리가 인식되지 않는 경우

- [ ] USB 클럭이 정확히 48MHz인지 확인
- [ ] USB OTG 케이블 사용 여부 확인 (일반 충전 케이블 X)
- [ ] USB 메모리가 FAT32로 포맷되었는지 확인
- [ ] `MX_USB_HOST_Process()`가 메인 루프에서 호출되는지 확인
- [ ] 전원 공급이 충분한지 확인 (5V, 500mA 이상)

### APPLICATION_READY 상태가 안 되는 경우

- [ ] USB_HOST 미들웨어에서 MSC Class 선택 확인
- [ ] NVIC에서 USB 인터럽트 활성화 확인
- [ ] USB 메모리가 정상 동작하는지 PC에서 확인

### 마운트 실패 (FR_DISK_ERR)

- [ ] USB 메모리 포맷 확인 (FAT32, 클러스터 크기 기본값)
- [ ] 다른 USB 메모리로 테스트
- [ ] USB 케이블/커넥터 접촉 확인

### 파일 쓰기 실패

- [ ] USB 메모리 쓰기 금지 스위치 확인
- [ ] 디스크 용량 확인
- [ ] 파일명 유효성 확인 (특수문자 제외)

### 한글 파일명이 깨지는 경우

- [ ] FATFS 설정에서 CODE_PAGE를 Korean (949)로 설정
- [ ] USE_LFN 활성화 확인

## 📁 프로젝트 구조

```
08_USB_Host_MSC/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── stm32f7xx_hal_conf.h
│   │   └── stm32f7xx_it.h
│   └── Src/
│       ├── main.c                     # 메인 로직
│       ├── stm32f7xx_hal_msp.c
│       ├── stm32f7xx_it.c
│       └── system_stm32f7xx.c
├── FATFS/
│   ├── App/
│   │   └── fatfs.c                    # FATFS 초기화
│   └── Target/
│       ├── ffconf.h                   # FATFS 설정
│       └── usbh_diskio.c              # USB 디스크 I/O
├── USB_HOST/
│   ├── App/
│   │   └── usb_host.c                 # USB Host 초기화
│   └── Target/
│       └── usbh_conf.c                # USB Host 설정
├── Middlewares/
│   ├── ST/
│   │   └── STM32_USB_Host_Library/
│   └── Third_Party/
│       └── FatFs/
├── Drivers/
│   ├── CMSIS/
│   └── STM32F7xx_HAL_Driver/
├── 08_USB_Host_MSC.ioc
└── README.md
```

## 📚 참고 자료

- [NUCLEO-F767ZI User Manual (UM1974)](https://www.st.com/resource/en/user_manual/um1974-stm32-nucleo144-boards-mb1137-stmicroelectronics.pdf)
- [STM32F767ZI Reference Manual (RM0410) - USB OTG](https://www.st.com/resource/en/reference_manual/rm0410-stm32f76xxx-and-stm32f77xxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [UM1720: STM32Cube USB Host Library](https://www.st.com/resource/en/user_manual/um1720-stm32cube-usb-host-library-stmicroelectronics.pdf)
- [FatFs Generic FAT Filesystem Module](http://elm-chan.org/fsw/ff/00index_e.html)
- [AN4879: USB hardware and PCB guidelines](https://www.st.com/resource/en/application_note/an4879-usb-hardware-and-pcb-guidelines-using-stm32-mcus-stmicroelectronics.pdf)

## 📝 라이선스

This project is licensed under the MIT License.

## ✍️ Author

Created for STM32 embedded systems learning and development.
