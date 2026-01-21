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
#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define IR_RECV_Pin         GPIO_PIN_0
#define IR_RECV_GPIO_Port   GPIOA
#define LED_Pin             GPIO_PIN_5
#define LED_GPIO_Port       GPIOA

/* System Clock defines */
#define SYSCLK_FREQ_HZ      64000000UL

/* NEC Protocol Timing (in microseconds) - 더 넓은 허용 오차 */
#define NEC_LEADER_PULSE_MIN    8000
#define NEC_LEADER_PULSE_MAX    10000
#define NEC_LEADER_SPACE_MIN    4000
#define NEC_LEADER_SPACE_MAX    5000
#define NEC_REPEAT_SPACE_MIN    2000
#define NEC_REPEAT_SPACE_MAX    2800
#define NEC_BIT_THRESHOLD       1000    // Space > 1000us = '1', Space < 1000us = '0'

/* IR Receiver parameters */
#define MAX_EDGES               100
#define SIGNAL_TIMEOUT_MS       80
#define MIN_PULSE_US            300     // 최소 유효 펄스

/* Edge type */
typedef enum {
    EDGE_FALLING = 0,   // HIGH -> LOW (IR 신호 시작)
    EDGE_RISING = 1     // LOW -> HIGH (IR 신호 끝)
} EdgeType_t;

/* Edge record */
typedef struct {
    uint32_t time_us;
    EdgeType_t type;
} EdgeRecord_t;

/* IR Data structure */
typedef struct {
    uint8_t address;
    uint8_t address_inv;
    uint8_t command;
    uint8_t command_inv;
    uint32_t raw_data;
    uint8_t bit_count;
    uint8_t is_repeat;
    uint8_t is_valid;
} IR_Data_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
volatile EdgeRecord_t edges[MAX_EDGES];
volatile uint8_t edge_count = 0;
volatile uint8_t receiving = 0;
volatile uint32_t last_edge_time = 0;
volatile uint8_t last_pin_state = 1;

/* Decoded data */
IR_Data_t ir_data;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* Printf redirect to UART ---------------------------------------------------*/
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/**
 * @brief Reset IR receiver state
 */
void IR_Reset(void) {
    edge_count = 0;
    receiving = 0;
    memset((void*)edges, 0, sizeof(edges));
    memset(&ir_data, 0, sizeof(ir_data));
}

/**
 * @brief Calculate duration between two edges
 */
uint32_t GetDuration(uint8_t start_idx, uint8_t end_idx) {
    if (end_idx <= start_idx || end_idx >= edge_count) {
        return 0;
    }
    return edges[end_idx].time_us - edges[start_idx].time_us;
}

/**
 * @brief Decode NEC protocol from edge timing
 * @retval 1 if successful, 0 if failed
 */
uint8_t IR_Decode(void) {
    /* 최소 edge 수 체크: Leader(2) + 32bits(64) + stop(1) = 67 edges */
    /* 하지만 실제로는 노이즈로 인해 더 많거나 적을 수 있음 */
    if (edge_count < 4) {
        return 0;
    }

    /* Edge 0: Falling (leader pulse start)
     * Edge 1: Rising (leader pulse end)
     * Edge 2: Falling (leader space end / first bit start)
     */

    /* Leader pulse duration: edge[0] -> edge[1] */
    uint32_t leader_pulse = GetDuration(0, 1);

    if (leader_pulse < NEC_LEADER_PULSE_MIN || leader_pulse > NEC_LEADER_PULSE_MAX) {
        return 0;
    }

    /* Leader space duration: edge[1] -> edge[2] */
    uint32_t leader_space = GetDuration(1, 2);

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
     * 각 비트는 다음과 같이 구성:
     * - Falling edge (bit pulse start)
     * - Rising edge (bit pulse end)
     * - 다음 Falling edge까지의 시간이 space
     *
     * Space 시간으로 0/1 구분:
     * - Space < 1000us: bit 0
     * - Space >= 1000us: bit 1
     */

    uint32_t raw_data = 0;
    uint8_t bit_count = 0;

    /* 데이터 비트는 edge index 2부터 시작 */
    /* 각 비트: falling(i) -> rising(i+1) -> falling(i+2) */
    /* Space = edge[i+2].time - edge[i+1].time */

    for (uint8_t i = 2; i + 2 < edge_count && bit_count < 32; i += 2) {
        /* Validate: edge[i] should be falling, edge[i+1] should be rising */
        if (edges[i].type != EDGE_FALLING || edges[i + 1].type != EDGE_RISING) {
            continue;  // Skip invalid edges
        }

        /* Bit pulse duration */
        uint32_t bit_pulse = GetDuration(i, i + 1);

        /* 유효한 비트 펄스인지 확인 (약 560us) */
        if (bit_pulse < MIN_PULSE_US || bit_pulse > 1000) {
            continue;
        }

        /* Space duration (to next falling edge) */
        uint32_t space;
        if (i + 2 < edge_count) {
            space = GetDuration(i + 1, i + 2);
        } else {
            /* 마지막 비트는 space가 없을 수 있음 - 기본값 0 */
            space = 500;  // Assume bit 0
        }

        /* Decode bit based on space duration */
        if (space >= NEC_BIT_THRESHOLD) {
            raw_data |= (1UL << bit_count);  // Bit 1 (LSB first)
        }
        /* else: Bit 0 (already 0) */

        bit_count++;
    }

    ir_data.bit_count = bit_count;

    /* 최소 비트 수 확인 */
    if (bit_count < 16) {  // 최소 16비트는 있어야 함
        return 0;
    }

    /* Extract fields */
    ir_data.raw_data = raw_data;
    ir_data.address = (raw_data >> 0) & 0xFF;
    ir_data.address_inv = (raw_data >> 8) & 0xFF;
    ir_data.command = (raw_data >> 16) & 0xFF;
    ir_data.command_inv = (raw_data >> 24) & 0xFF;
    ir_data.is_repeat = 0;
    ir_data.is_valid = 1;

    return 1;
}

/**
 * @brief Print decoded IR result
 */
void IR_Print_Result(void) {
    if (!ir_data.is_valid) {
        printf("    Result: Invalid data\r\n");
        return;
    }

    if (ir_data.is_repeat) {
        printf("    >> REPEAT Code <<\r\n");
        return;
    }

    printf("\r\n");
    printf("    +----------------------------------+\r\n");
    printf("    |        NEC Decoded Data          |\r\n");
    printf("    +----------------------------------+\r\n");
    printf("    | Bits decoded : %-2d                |\r\n", ir_data.bit_count);
    printf("    | Raw Data     : 0x%08lX        |\r\n", ir_data.raw_data);
    printf("    +----------------------------------+\r\n");
    printf("    | Address      : 0x%02X  (%3d)      |\r\n", ir_data.address, ir_data.address);
    printf("    | Address~     : 0x%02X  (%3d)      |\r\n", ir_data.address_inv, ir_data.address_inv);
    printf("    | Command      : 0x%02X  (%3d)      |\r\n", ir_data.command, ir_data.command);
    printf("    | Command~     : 0x%02X  (%3d)      |\r\n", ir_data.command_inv, ir_data.command_inv);
    printf("    +----------------------------------+\r\n");

    /* Validate */
    uint8_t addr_valid = ((ir_data.address ^ ir_data.address_inv) == 0xFF);
    uint8_t cmd_valid = ((ir_data.command ^ ir_data.command_inv) == 0xFF);

    if (addr_valid && cmd_valid) {
        printf("    | Protocol: Standard NEC          |\r\n");
        printf("    | Device  : 0x%02X                   |\r\n", ir_data.address);
        printf("    | Key Code: 0x%02X                   |\r\n", ir_data.command);
    }
    else if (cmd_valid) {
        uint16_t ext_addr = ir_data.address | (ir_data.address_inv << 8);
        printf("    | Protocol: Extended NEC          |\r\n");
        printf("    | Device  : 0x%04X                 |\r\n", ext_addr);
        printf("    | Key Code: 0x%02X                   |\r\n", ir_data.command);
    }
    else {
        printf("    | Protocol: Unknown/Checksum Err  |\r\n");
    }
    printf("    +----------------------------------+\r\n");

    /* Binary representation */
    printf("\r\n    Binary (LSB first):\r\n");
    printf("    ADDR     : ");
    for (int i = 7; i >= 0; i--) printf("%d", (ir_data.address >> i) & 1);
    printf(" (0x%02X)\r\n", ir_data.address);

    printf("    ADDR~    : ");
    for (int i = 7; i >= 0; i--) printf("%d", (ir_data.address_inv >> i) & 1);
    printf(" (0x%02X)\r\n", ir_data.address_inv);

    printf("    CMD      : ");
    for (int i = 7; i >= 0; i--) printf("%d", (ir_data.command >> i) & 1);
    printf(" (0x%02X)\r\n", ir_data.command);

    printf("    CMD~     : ");
    for (int i = 7; i >= 0; i--) printf("%d", (ir_data.command_inv >> i) & 1);
    printf(" (0x%02X)\r\n", ir_data.command_inv);
}

/**
 * @brief Print raw timing data for debugging
 */
void IR_Print_Raw_Timing(void) {
    printf("\r\n    Raw Edge Timing:\r\n");

    uint8_t print_count = (edge_count > 20) ? 20 : edge_count;

    for (uint8_t i = 0; i < print_count; i++) {
        char edge_char = (edges[i].type == EDGE_FALLING) ? 'F' : 'R';
        uint32_t duration = 0;

        if (i > 0) {
            duration = edges[i].time_us - edges[i-1].time_us;
        }

        printf("    [%2d] %c @ %7lu us", i, edge_char, edges[i].time_us);
        if (i > 0) {
            printf(" (delta: %5lu us)", duration);
        }
        printf("\r\n");
    }

    if (edge_count > 20) {
        printf("    ... (%d more edges)\r\n", edge_count - 20);
    }

    /* 간단한 비트 해석 시도 */
    printf("\r\n    Bit interpretation (space-based):\r\n    ");

    uint8_t bit_idx = 0;
    for (uint8_t i = 2; i + 2 < edge_count && bit_idx < 32; i += 2) {
        if (edges[i].type == EDGE_FALLING && edges[i + 1].type == EDGE_RISING) {
            uint32_t space = edges[i + 2].time_us - edges[i + 1].time_us;

            if (space >= NEC_BIT_THRESHOLD) {
                printf("1");
            } else {
                printf("0");
            }
            bit_idx++;

            if (bit_idx == 8 || bit_idx == 16 || bit_idx == 24) {
                printf(" ");
            }
        }
    }
    printf(" (%d bits)\r\n", bit_idx);

    /* 비트를 바이트로 변환해서 표시 */
    if (bit_idx >= 8) {
        printf("    Hex values: ");
        uint32_t val = 0;
        uint8_t b = 0;

        for (uint8_t i = 2; i + 2 < edge_count && b < 32; i += 2) {
            if (edges[i].type == EDGE_FALLING && edges[i + 1].type == EDGE_RISING) {
                uint32_t space = edges[i + 2].time_us - edges[i + 1].time_us;

                if (space >= NEC_BIT_THRESHOLD) {
                    val |= (1UL << b);
                }
                b++;
            }
        }

        printf("0x%02lX ", (val >> 0) & 0xFF);
        if (bit_idx >= 16) printf("0x%02lX ", (val >> 8) & 0xFF);
        if (bit_idx >= 24) printf("0x%02lX ", (val >> 16) & 0xFF);
        if (bit_idx >= 32) printf("0x%02lX", (val >> 24) & 0xFF);
        printf("\r\n");
    }
}

/**
 * @brief Get microseconds from TIM2
 */
uint32_t GetMicros(void) {
    return __HAL_TIM_GET_COUNTER(&htim2);
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
  HAL_TIM_Base_Start(&htim2);

  printf("\r\n");
  printf("========================================\r\n");
  printf("   IR Receiver & NEC Decoder - STM32   \r\n");
  printf("========================================\r\n");
  printf(" System Clock : 64 MHz\r\n");
  printf(" IR Input     : PA0 (Pull-up)\r\n");
  printf(" Status LED   : PA5\r\n");
  printf(" Protocol     : NEC / Extended NEC\r\n");
  printf("========================================\r\n");
  printf("\r\nWaiting for IR signals...\r\n\n");

  IR_Reset();

  uint32_t signal_count = 0;
  uint32_t last_activity_time = 0;
  uint32_t signal_start_time = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      uint8_t ir_state = HAL_GPIO_ReadPin(IR_RECV_GPIO_Port, IR_RECV_Pin);
      uint32_t current_us = GetMicros();
      uint32_t current_ms = HAL_GetTick();

      /* Edge detection */
      if (ir_state != last_pin_state) {
          if (!receiving) {
              /* 새 신호 시작 - Falling edge만 시작으로 인식 */
              if (last_pin_state == 1 && ir_state == 0) {
                  receiving = 1;
                  edge_count = 0;
                  signal_start_time = current_us;
                  signal_count++;
                  HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

                  /* 첫 번째 falling edge 기록 */
                  edges[edge_count].time_us = 0;  // 상대 시간 0
                  edges[edge_count].type = EDGE_FALLING;
                  edge_count++;
              }
          }
          else if (edge_count < MAX_EDGES) {
              /* 상대 시간 계산 (오버플로우 처리) */
              uint32_t rel_time = current_us - signal_start_time;

              edges[edge_count].time_us = rel_time;
              edges[edge_count].type = (ir_state == 0) ? EDGE_FALLING : EDGE_RISING;
              edge_count++;
          }

          last_activity_time = current_ms;
          last_pin_state = ir_state;
      }

      /* Signal timeout check */
      if (receiving && (current_ms - last_activity_time > SIGNAL_TIMEOUT_MS)) {
          receiving = 0;

          printf("----------------------------------------\r\n");
          printf("[%lu] IR Signal Received (%d edges)\r\n", signal_count, edge_count);

          /* Decode attempt */
          if (IR_Decode()) {
              IR_Print_Result();
          } else {
              printf("    Status: Decode failed\r\n");
              IR_Print_Raw_Timing();
          }
          printf("\r\n");

          IR_Reset();
      }
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
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
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
