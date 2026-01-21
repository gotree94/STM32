/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* NEC Protocol Timing (microseconds) */
#define NEC_LEADER_PULSE_MIN    8000
#define NEC_LEADER_PULSE_MAX    10000
#define NEC_LEADER_SPACE_MIN    4000
#define NEC_LEADER_SPACE_MAX    5000
#define NEC_REPEAT_SPACE_MIN    2000
#define NEC_REPEAT_SPACE_MAX    2500
#define NEC_BIT_THRESHOLD       1100    // > 1100us = '1', < 1100us = '0'

/* IR Receiver parameters */
#define MAX_EDGES               100
#define SIGNAL_TIMEOUT_MS       50

/* Edge record structure */
typedef struct {
    uint32_t time_us;       // Timestamp in microseconds
    uint8_t level;          // Pin level AFTER edge (0 or 1)
} EdgeRecord_t;

/* IR decoded data structure */
typedef struct {
    uint8_t address;        // 8-bit address
    uint8_t address_inv;    // 8-bit inverted address
    uint8_t command;        // 8-bit command
    uint8_t command_inv;    // 8-bit inverted command
    uint32_t raw_data;      // 32-bit raw data
    uint8_t bit_count;      // Number of bits decoded
    uint8_t is_repeat;      // Repeat code flag
    uint8_t is_valid;       // Valid data flag
} IR_Data_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define IR_RECV_Pin         GPIO_PIN_0
#define IR_RECV_GPIO_Port   GPIOA
#define LED_Pin             GPIO_PIN_5
#define LED_GPIO_Port       GPIOA
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* IR reception variables - volatile for ISR access */
volatile EdgeRecord_t edges[MAX_EDGES];
volatile uint8_t edge_count = 0;
volatile uint8_t ir_receiving = 0;
volatile uint32_t ir_start_time = 0;

/* Decoded data */
IR_Data_t ir_data;

/* Signal counter */
uint32_t signal_count = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
/* IR processing functions */
uint32_t GetMicros(void);
void IR_Reset(void);
uint32_t IR_GetDuration(uint8_t idx1, uint8_t idx2);
uint8_t IR_Decode(void);
void IR_PrintResult(void);
void IR_PrintRawTiming(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * @brief Printf redirect to UART
 */
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/**
 * @brief Get microseconds from TIM2
 * @retval Current microsecond count
 */
uint32_t GetMicros(void) {
    return __HAL_TIM_GET_COUNTER(&htim2);
}

/**
 * @brief Reset IR receiver state
 */
void IR_Reset(void) {
    __disable_irq();
    edge_count = 0;
    ir_receiving = 0;
    __enable_irq();

    memset(&ir_data, 0, sizeof(ir_data));
}

/**
 * @brief Get duration between two edges
 * @param idx1 First edge index
 * @param idx2 Second edge index
 * @retval Duration in microseconds
 */
uint32_t IR_GetDuration(uint8_t idx1, uint8_t idx2) {
    if (idx2 >= edge_count || idx1 >= edge_count) return 0;
    return edges[idx2].time_us - edges[idx1].time_us;
}

/**
 * @brief Decode NEC protocol from captured edges
 * @retval 1 if successful, 0 if failed
 */
uint8_t IR_Decode(void) {
    if (edge_count < 4) return 0;

    /* Leader pulse: edge[0](falling) -> edge[1](rising) */
    uint32_t leader_pulse = IR_GetDuration(0, 1);

    if (leader_pulse < NEC_LEADER_PULSE_MIN || leader_pulse > NEC_LEADER_PULSE_MAX) {
        return 0;
    }

    /* Leader space: edge[1](rising) -> edge[2](falling) */
    uint32_t leader_space = IR_GetDuration(1, 2);

    /* Check for repeat code */
    if (leader_space >= NEC_REPEAT_SPACE_MIN && leader_space <= NEC_REPEAT_SPACE_MAX) {
        ir_data.is_repeat = 1;
        ir_data.is_valid = 1;
        return 1;
    }

    /* Check normal leader space */
    if (leader_space < NEC_LEADER_SPACE_MIN || leader_space > NEC_LEADER_SPACE_MAX) {
        return 0;
    }

    /* Decode 32 bits
     * Each bit consists of:
     * - Falling edge (pulse start)
     * - Rising edge (pulse end)
     * - Space until next falling edge
     * Bit value is determined by space duration
     */
    uint32_t raw_data = 0;
    uint8_t bit_count = 0;

    for (uint8_t i = 2; i + 2 < edge_count && bit_count < 32; i += 2) {
        /* Validate edge sequence */
        if (edges[i].level != 0) continue;      // Should be falling
        if (edges[i + 1].level != 1) continue;  // Should be rising

        /* Calculate space duration */
        uint32_t space = 0;
        if (i + 2 < edge_count) {
            space = IR_GetDuration(i + 1, i + 2);
        } else {
            space = 500;  // Assume '0' for last bit
        }

        /* Decode bit based on space duration */
        if (space >= NEC_BIT_THRESHOLD) {
            raw_data |= (1UL << bit_count);  // Bit '1'
        }
        /* Bit '0' - already 0 */

        bit_count++;
    }

    ir_data.bit_count = bit_count;

    if (bit_count < 24) return 0;  // Need at least 24 bits

    /* Extract data fields (LSB first) */
    ir_data.raw_data = raw_data;
    ir_data.address = (raw_data >> 0) & 0xFF;
    ir_data.address_inv = (raw_data >> 8) & 0xFF;
    ir_data.command = (raw_data >> 16) & 0xFF;
    ir_data.command_inv = (raw_data >> 24) & 0xFF;
    ir_data.is_valid = 1;

    return 1;
}

/**
 * @brief Print decoded IR result
 */
void IR_PrintResult(void) {
    if (!ir_data.is_valid) {
        printf("    Invalid data\r\n");
        return;
    }

    if (ir_data.is_repeat) {
        printf("    >> REPEAT <<\r\n");
        return;
    }

    /* Checksum validation */
    uint8_t addr_valid = ((ir_data.address ^ ir_data.address_inv) == 0xFF);
    uint8_t cmd_valid = ((ir_data.command ^ ir_data.command_inv) == 0xFF);

    printf("\r\n");
    printf("    +----------------------------------+\r\n");
    printf("    |        NEC Decoded Data          |\r\n");
    printf("    +----------------------------------+\r\n");
    printf("    | Raw (hex)  : 0x%08lX          |\r\n", ir_data.raw_data);
    printf("    | Bits       : %-2d                 |\r\n", ir_data.bit_count);
    printf("    +----------------------------------+\r\n");
    printf("    | Address    : 0x%02X  (%3d)       |\r\n", ir_data.address, ir_data.address);
    printf("    | Address~   : 0x%02X  (%3d) %s   |\r\n",
           ir_data.address_inv, ir_data.address_inv,
           addr_valid ? "[OK]" : "[ER]");
    printf("    | Command    : 0x%02X  (%3d)       |\r\n", ir_data.command, ir_data.command);
    printf("    | Command~   : 0x%02X  (%3d) %s   |\r\n",
           ir_data.command_inv, ir_data.command_inv,
           cmd_valid ? "[OK]" : "[ER]");
    printf("    +----------------------------------+\r\n");

    if (addr_valid && cmd_valid) {
        printf("    | * Standard NEC                  |\r\n");
        printf("    |   Device  : 0x%02X                |\r\n", ir_data.address);
        printf("    |   Key Code: 0x%02X  (Dec: %3d)   |\r\n", ir_data.command, ir_data.command);
    }
    else if (cmd_valid) {
        uint16_t ext_addr = ir_data.address | (ir_data.address_inv << 8);
        printf("    | * Extended NEC                  |\r\n");
        printf("    |   Device  : 0x%04X              |\r\n", ext_addr);
        printf("    |   Key Code: 0x%02X  (Dec: %3d)   |\r\n", ir_data.command, ir_data.command);
    }
    else {
        printf("    | * Checksum Error                |\r\n");
        printf("    |   (Data may be corrupted)       |\r\n");
    }
    printf("    +----------------------------------+\r\n");

    /* Summary */
    printf("\r\n    Key Code Summary:\r\n");
    printf("    -----------------\r\n");
    printf("    Addr=0x%02X, Cmd=0x%02X\r\n", ir_data.address, ir_data.command);
}

/**
 * @brief Print raw timing data for debugging
 */
void IR_PrintRawTiming(void) {
    printf("\r\n    Raw Edge Timing (first 24):\r\n");

    uint8_t print_cnt = (edge_count > 24) ? 24 : edge_count;

    for (uint8_t i = 0; i < print_cnt; i++) {
        char level_char = edges[i].level ? 'H' : 'L';
        uint32_t delta = (i > 0) ? (edges[i].time_us - edges[i-1].time_us) : 0;

        printf("    [%2d] %c %7lu us", i, level_char, edges[i].time_us);
        if (i > 0) printf(" (+%5lu)", delta);
        printf("\r\n");
    }

    if (edge_count > 24) {
        printf("    ... +%d more edges\r\n", edge_count - 24);
    }

    /* Attempt bit decode */
    printf("\r\n    Bit decode attempt:\r\n    ");

    uint32_t decoded = 0;
    uint8_t bits = 0;

    for (uint8_t i = 2; i + 2 < edge_count && bits < 32; i += 2) {
        if (edges[i].level == 0 && edges[i + 1].level == 1) {
            uint32_t space = edges[i + 2].time_us - edges[i + 1].time_us;

            if (space >= NEC_BIT_THRESHOLD) {
                printf("1");
                decoded |= (1UL << bits);
            } else {
                printf("0");
            }
            bits++;

            if (bits == 8 || bits == 16 || bits == 24) printf(" ");
        }
    }

    printf(" (%d bits)\r\n", bits);

    if (bits >= 8) {
        printf("    Hex: ");
        printf("0x%02lX ", (decoded >> 0) & 0xFF);
        if (bits >= 16) printf("0x%02lX ", (decoded >> 8) & 0xFF);
        if (bits >= 24) printf("0x%02lX ", (decoded >> 16) & 0xFF);
        if (bits >= 32) printf("0x%02lX", (decoded >> 24) & 0xFF);
        printf("\r\n");
    }
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

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
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
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  /* Start TIM2 for microsecond counting */
  HAL_TIM_Base_Start(&htim2);

  /* Print startup message */
  printf("\r\n");
  printf("========================================\r\n");
  printf("  IR Receiver & NEC Decoder (v3.0)     \r\n");
  printf("  EXTI Interrupt Based - STM32F103     \r\n");
  printf("========================================\r\n");
  printf(" System Clock : 64 MHz\r\n");
  printf(" IR Input     : PA0 (EXTI0 Interrupt)\r\n");
  printf(" Status LED   : PA5\r\n");
  printf(" Protocol     : NEC / Extended NEC\r\n");
  printf("========================================\r\n");
  printf("\r\nWaiting for IR signals...\r\n\n");

  /* Initialize IR receiver */
  IR_Reset();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      /* Check for signal timeout (end of IR burst) */
      if (ir_receiving && edge_count > 0) {
          uint32_t current_time = GetMicros();
          uint32_t last_edge_time = ir_start_time + edges[edge_count - 1].time_us;
          uint32_t elapsed = current_time - last_edge_time;

          if (elapsed > (SIGNAL_TIMEOUT_MS * 1000)) {
              /* Signal reception complete */
              ir_receiving = 0;
              signal_count++;

              /* Toggle LED to indicate reception */
              HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

              /* Print results */
              printf("----------------------------------------\r\n");
              printf("[%lu] IR Signal Received (%d edges)\r\n", signal_count, edge_count);

              if (IR_Decode()) {
                  IR_PrintResult();
              } else {
                  printf("    Status: Decode failed\r\n");
                  IR_PrintRawTiming();
              }
              printf("\r\n");

              /* Reset for next signal */
              IR_Reset();
          }
      }

      /* Small delay to reduce CPU usage */
      HAL_Delay(1);
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
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

  /** Initializes the CPU, AHB and APB buses clocks
  */
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
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 63;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */
  TIM2->ARR = 0xFFFFFFFF;
  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
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
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
