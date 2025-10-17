/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : SD Card File System Example using FatFs
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include <stdio.h>
#include <string.h>

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
FATFS FatFs;     /* FatFs work area */
FIL File;        /* File object */
FRESULT fres;    /* FatFs function common result code */
UINT bytesWritten, bytesRead;
char buffer[100];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);

/* USER CODE BEGIN PFP */
void UART_Print(char *msg);
void SD_Card_Example(void);
/* USER CODE END PFP */

int main(void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();
    
    /* Configure the system clock */
    SystemClock_Config();
    
    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_SPI1_Init();
    MX_USART2_UART_Init();
    MX_FATFS_Init();
    
    /* USER CODE BEGIN 2 */
    UART_Print("\r\n===== STM32 SD Card File System Example =====\r\n");
    
    /* SD Card Example */
    SD_Card_Example();
    /* USER CODE END 2 */
    
    /* Infinite loop */
    while (1)
    {
        HAL_Delay(1000);
    }
}

/* USER CODE BEGIN 4 */

void UART_Print(char *msg)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

void SD_Card_Example(void)
{
    /* 1. Mount SD Card */
    UART_Print("1. Mounting SD Card...\r\n");
    fres = f_mount(&FatFs, "", 1);
    if (fres != FR_OK) {
        sprintf(buffer, "   Error mounting SD Card! Error code: %d\r\n", fres);
        UART_Print(buffer);
        return;
    }
    UART_Print("   SD Card mounted successfully!\r\n\r\n");
    
    /* 2. Check free space */
    UART_Print("2. Checking SD Card space...\r\n");
    DWORD fre_clust, fre_sect, tot_sect;
    FATFS* fs;
    
    f_getfree("", &fre_clust, &fs);
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    fre_sect = fre_clust * fs->csize;
    
    sprintf(buffer, "   Total: %lu KB, Free: %lu KB\r\n\r\n", 
            tot_sect / 2, fre_sect / 2);
    UART_Print(buffer);
    
    /* 3. Create and write to file */
    UART_Print("3. Creating and writing to file...\r\n");
    fres = f_open(&File, "test.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if (fres != FR_OK) {
        sprintf(buffer, "   Error opening file! Error code: %d\r\n", fres);
        UART_Print(buffer);
        return;
    }
    
    const char *writeData = "Hello from STM32!\r\nThis is SD Card test.\r\nLine 3\r\n";
    fres = f_write(&File, writeData, strlen(writeData), &bytesWritten);
    if (fres != FR_OK) {
        sprintf(buffer, "   Error writing file! Error code: %d\r\n", fres);
        UART_Print(buffer);
    } else {
        sprintf(buffer, "   Written %u bytes successfully!\r\n", bytesWritten);
        UART_Print(buffer);
    }
    
    f_close(&File);
    UART_Print("   File closed.\r\n\r\n");
    
    /* 4. Read file */
    UART_Print("4. Reading file...\r\n");
    fres = f_open(&File, "test.txt", FA_READ);
    if (fres != FR_OK) {
        sprintf(buffer, "   Error opening file for read! Error code: %d\r\n", fres);
        UART_Print(buffer);
        return;
    }
    
    char readBuffer[100];
    fres = f_read(&File, readBuffer, sizeof(readBuffer) - 1, &bytesRead);
    if (fres != FR_OK) {
        sprintf(buffer, "   Error reading file! Error code: %d\r\n", fres);
        UART_Print(buffer);
    } else {
        readBuffer[bytesRead] = '\0';
        sprintf(buffer, "   Read %u bytes:\r\n", bytesRead);
        UART_Print(buffer);
        UART_Print("   ---Content Start---\r\n");
        UART_Print(readBuffer);
        UART_Print("   ---Content End---\r\n");
    }
    
    f_close(&File);
    UART_Print("   File closed.\r\n\r\n");
    
    /* 5. Append to file */
    UART_Print("5. Appending to file...\r\n");
    fres = f_open(&File, "test.txt", FA_OPEN_APPEND | FA_WRITE);
    if (fres == FR_OK) {
        const char *appendData = "Appended line!\r\n";
        f_write(&File, appendData, strlen(appendData), &bytesWritten);
        sprintf(buffer, "   Appended %u bytes\r\n", bytesWritten);
        UART_Print(buffer);
        f_close(&File);
    }
    UART_Print("\r\n");
    
    /* 6. List directory */
    UART_Print("6. Listing files in root directory...\r\n");
    DIR dir;
    FILINFO fno;
    
    fres = f_opendir(&dir, "/");
    if (fres == FR_OK) {
        while (1) {
            fres = f_readdir(&dir, &fno);
            if (fres != FR_OK || fno.fname[0] == 0) break;
            
            if (fno.fattrib & AM_DIR) {
                sprintf(buffer, "   [DIR]  %s\r\n", fno.fname);
            } else {
                sprintf(buffer, "   [FILE] %s (%lu bytes)\r\n", fno.fname, fno.fsize);
            }
            UART_Print(buffer);
        }
        f_closedir(&dir);
    }
    UART_Print("\r\n");
    
    /* 7. Create directory */
    UART_Print("7. Creating directory...\r\n");
    fres = f_mkdir("mydir");
    if (fres == FR_OK || fres == FR_EXIST) {
        UART_Print("   Directory 'mydir' ready\r\n");
        
        /* Write file in directory */
        fres = f_open(&File, "mydir/data.txt", FA_CREATE_ALWAYS | FA_WRITE);
        if (fres == FR_OK) {
            const char *dirData = "File in directory\r\n";
            f_write(&File, dirData, strlen(dirData), &bytesWritten);
            f_close(&File);
            UART_Print("   Created file in directory\r\n");
        }
    }
    UART_Print("\r\n");
    
    /* 8. Delete file (optional - commented out) */
    /*
    UART_Print("8. Deleting test file...\r\n");
    fres = f_unlink("test.txt");
    if (fres == FR_OK) {
        UART_Print("   File deleted successfully!\r\n");
    }
    */
    
    /* Unmount */
    f_mount(NULL, "", 0);
    UART_Print("===== SD Card operations completed! =====\r\n");
}

/* USER CODE END 4 */

/**
  * @brief System Clock Configuration
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
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
    
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

static void MX_SPI1_Init(void)
{
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    HAL_SPI_Init(&hspi1);
}

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
    HAL_UART_Init(&huart2);
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    /* Configure SD_CS Pin */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}