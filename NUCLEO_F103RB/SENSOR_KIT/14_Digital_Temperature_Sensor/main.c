/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Digital Temperature Sensor Module Test for STM32F103 NUCLEO
 * @author         : Embedded Systems Lab
 * @date           : 2025
 ******************************************************************************
 * @attention
 *
 * Digital Temperature Sensor Module (DS18B20 based) Interface:
 * - DATA -> PA0 (1-Wire interface)
 * - VCC  -> 3.3V or 5V
 * - GND  -> GND
 *
 * Note: 4.7K pull-up resistor required on DATA line
 *       (Some modules have built-in pull-up)
 *
 * DS18B20 provides 9-12 bit temperature readings over 1-Wire protocol
 * Temperature range: -55°C to +125°C, Accuracy: ±0.5°C (-10°C to +85°C)
 *
 * UART2 is used for debug output (connected to ST-Link Virtual COM Port)
 * - TX: PA2, RX: PA3, Baudrate: 115200
 *
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* Private variables */
UART_HandleTypeDef huart2;

/* DS18B20 GPIO Configuration */
#define DS18B20_PORT        GPIOA
#define DS18B20_PIN         GPIO_PIN_0

/* DS18B20 Commands */
#define DS18B20_CMD_SKIP_ROM        0xCC
#define DS18B20_CMD_CONVERT_T       0x44
#define DS18B20_CMD_READ_SCRATCHPAD 0xBE
#define DS18B20_CMD_WRITE_SCRATCHPAD 0x4E
#define DS18B20_CMD_READ_ROM        0x33

/* Resolution settings */
#define DS18B20_RESOLUTION_9BIT     0x1F
#define DS18B20_RESOLUTION_10BIT    0x3F
#define DS18B20_RESOLUTION_11BIT    0x5F
#define DS18B20_RESOLUTION_12BIT    0x7F

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

/* Printf redirect to UART */
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/* Microsecond delay using DWT cycle counter */
static uint32_t DWT_Init(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    return 0;
}

static void delay_us(uint32_t us) {
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < ticks);
}

/* 1-Wire Pin Control */
static void DS18B20_SetPinOutput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DS18B20_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DS18B20_PORT, &GPIO_InitStruct);
}

static void DS18B20_SetPinInput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DS18B20_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DS18B20_PORT, &GPIO_InitStruct);
}

static void DS18B20_WriteBit(uint8_t bit) {
    DS18B20_SetPinOutput();
    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
    
    if (bit) {
        delay_us(2);
        DS18B20_SetPinInput();
        delay_us(60);
    } else {
        delay_us(60);
        DS18B20_SetPinInput();
        delay_us(2);
    }
}

static uint8_t DS18B20_ReadBit(void) {
    uint8_t bit = 0;
    
    DS18B20_SetPinOutput();
    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
    delay_us(2);
    
    DS18B20_SetPinInput();
    delay_us(10);
    
    if (HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN)) {
        bit = 1;
    }
    
    delay_us(50);
    return bit;
}

/**
 * @brief  Send reset pulse and check for presence
 * @retval 1 if device present, 0 otherwise
 */
uint8_t DS18B20_Reset(void) {
    uint8_t presence = 0;
    
    DS18B20_SetPinOutput();
    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
    delay_us(480);
    
    DS18B20_SetPinInput();
    delay_us(70);
    
    if (HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN) == GPIO_PIN_RESET) {
        presence = 1;
    }
    
    delay_us(410);
    return presence;
}

/**
 * @brief  Write byte to DS18B20
 * @param  data: Byte to write
 * @retval None
 */
void DS18B20_WriteByte(uint8_t data) {
    for (uint8_t i = 0; i < 8; i++) {
        DS18B20_WriteBit(data & 0x01);
        data >>= 1;
    }
}

/**
 * @brief  Read byte from DS18B20
 * @retval Read byte
 */
uint8_t DS18B20_ReadByte(void) {
    uint8_t data = 0;
    for (uint8_t i = 0; i < 8; i++) {
        data >>= 1;
        if (DS18B20_ReadBit()) {
            data |= 0x80;
        }
    }
    return data;
}

/**
 * @brief  Start temperature conversion
 * @retval 1 if successful, 0 otherwise
 */
uint8_t DS18B20_StartConversion(void) {
    if (!DS18B20_Reset()) {
        return 0;
    }
    DS18B20_WriteByte(DS18B20_CMD_SKIP_ROM);
    DS18B20_WriteByte(DS18B20_CMD_CONVERT_T);
    return 1;
}

/**
 * @brief  Read temperature from DS18B20
 * @retval Temperature in Celsius * 16 (raw value)
 */
int16_t DS18B20_ReadTemperatureRaw(void) {
    uint8_t temp_l, temp_h;
    
    if (!DS18B20_Reset()) {
        return 0x8000;  // Error value
    }
    
    DS18B20_WriteByte(DS18B20_CMD_SKIP_ROM);
    DS18B20_WriteByte(DS18B20_CMD_READ_SCRATCHPAD);
    
    temp_l = DS18B20_ReadByte();
    temp_h = DS18B20_ReadByte();
    
    return (int16_t)((temp_h << 8) | temp_l);
}

/**
 * @brief  Get temperature in Celsius
 * @retval Temperature in Celsius (float)
 */
float DS18B20_GetTemperature(void) {
    // Start conversion
    if (!DS18B20_StartConversion()) {
        return -999.0f;
    }
    
    // Wait for conversion (750ms for 12-bit)
    HAL_Delay(750);
    
    // Read temperature
    int16_t raw = DS18B20_ReadTemperatureRaw();
    
    if (raw == 0x8000) {
        return -999.0f;
    }
    
    return (float)raw / 16.0f;
}

/**
 * @brief  Read ROM code from DS18B20
 * @param  rom: Pointer to 8-byte array for ROM code
 * @retval 1 if successful, 0 otherwise
 */
uint8_t DS18B20_ReadROM(uint8_t *rom) {
    if (!DS18B20_Reset()) {
        return 0;
    }
    
    DS18B20_WriteByte(DS18B20_CMD_READ_ROM);
    
    for (uint8_t i = 0; i < 8; i++) {
        rom[i] = DS18B20_ReadByte();
    }
    
    return 1;
}

/**
 * @brief  Print temperature bar graph
 * @param  temp: Temperature in Celsius
 * @retval None
 */
void Print_TempBar(float temp) {
    int bars = (int)((temp + 10) / 2);  // -10°C = 0 bars, 50°C = 30 bars
    if (bars < 0) bars = 0;
    if (bars > 30) bars = 30;
    
    printf("[");
    for (int i = 0; i < 30; i++) {
        if (i < bars) {
            if (temp < 10) printf("▓");      // Cold
            else if (temp < 30) printf("█"); // Normal
            else printf("▒");                // Hot
        } else {
            printf("░");
        }
    }
    printf("]");
}

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
    HAL_Init();
    SystemClock_Config();
    DWT_Init();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    printf("\r\n========================================\r\n");
    printf("  Digital Temperature Sensor Test\r\n");
    printf("  DS18B20 on NUCLEO-F103RB\r\n");
    printf("========================================\r\n\n");
    printf("DATA Pin: PA0 (1-Wire)\r\n\n");

    // Check sensor presence
    printf("Checking for DS18B20...\r\n");
    if (DS18B20_Reset()) {
        printf("DS18B20 detected!\r\n\n");
        
        // Read ROM code
        uint8_t rom[8];
        if (DS18B20_ReadROM(rom)) {
            printf("ROM Code: ");
            for (int i = 0; i < 8; i++) {
                printf("%02X", rom[i]);
                if (i < 7) printf("-");
            }
            printf("\r\n");
            printf("Family Code: 0x%02X (DS18B20 = 0x28)\r\n\n", rom[0]);
        }
    } else {
        printf("ERROR: DS18B20 not found!\r\n");
        printf("Check wiring and pull-up resistor.\r\n\n");
    }

    float temp;
    float temp_min = 125.0f;
    float temp_max = -55.0f;
    uint32_t read_count = 0;

    printf("Starting temperature monitoring...\r\n");
    printf("Scale: -10°C to +50°C\r\n\n");

    while (1) {
        temp = DS18B20_GetTemperature();
        
        if (temp > -900.0f) {  // Valid reading
            read_count++;
            
            // Update min/max
            if (temp < temp_min) temp_min = temp;
            if (temp > temp_max) temp_max = temp;
            
            // Print temperature
            printf("Temp: %+6.2f°C ", temp);
            Print_TempBar(temp);
            printf(" Min:%+.1f Max:%+.1f", temp_min, temp_max);
            
            // Temperature alert
            if (temp > 35.0f) {
                printf(" [HIGH!]");
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
            } else if (temp < 10.0f) {
                printf(" [LOW!]");
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
            } else {
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
            }
            
            printf("\r\n");
            
        } else {
            printf("ERROR: Sensor read failed!\r\n");
        }
        
        HAL_Delay(250);  // Small delay between readings
    }
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

/**
 * @brief GPIO Initialization Function
 * @retval None
 */
static void MX_GPIO_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // PA0 - DS18B20 DATA (initially input with pull-up)
    GPIO_InitStruct.Pin = DS18B20_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DS18B20_PORT, &GPIO_InitStruct);

    // PA5 - Onboard LED (Output)
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/**
 * @brief USART2 Initialization Function
 * @retval None
 */
static void MX_USART2_UART_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_USART2_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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

void Error_Handler(void) {
    __disable_irq();
    while (1) {
    }
}
