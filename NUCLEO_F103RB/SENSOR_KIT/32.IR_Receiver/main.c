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
#define SYSCLK_FREQ_HZ      64000000UL  // 64MHz
#define APB1_TIMER_FREQ_HZ  64000000UL  // APB1 Timer Clock (APB1×2)

/* NEC Protocol Timing (in microseconds) */
#define NEC_LEADER_PULSE_MIN    8000    // 9ms - tolerance
#define NEC_LEADER_PULSE_MAX    10000   // 9ms + tolerance
#define NEC_LEADER_SPACE_MIN    4000    // 4.5ms - tolerance
#define NEC_LEADER_SPACE_MAX    5000    // 4.5ms + tolerance
#define NEC_REPEAT_SPACE_MIN    2000    // 2.25ms - tolerance
#define NEC_REPEAT_SPACE_MAX    2500    // 2.25ms + tolerance
#define NEC_BIT_PULSE_MIN       400     // 562.5us - tolerance
#define NEC_BIT_PULSE_MAX       750     // 562.5us + tolerance
#define NEC_BIT_0_SPACE_MIN     400     // 562.5us - tolerance
#define NEC_BIT_0_SPACE_MAX     750     // 562.5us + tolerance
#define NEC_BIT_1_SPACE_MIN     1400    // 1687.5us - tolerance
#define NEC_BIT_1_SPACE_MAX     1900    // 1687.5us + tolerance

/* IR Receiver parameters */
#define MAX_PULSES              68      // NEC: 1 leader + 32 bits + 1 stop = 34 edges × 2
#define SIGNAL_TIMEOUT_MS       100     // Timeout for signal detection
#define NOISE_FILTER_US         200     // Minimum pulse width to filter noise

/* NEC Decoder states */
typedef enum {
    IR_IDLE,
    IR_LEADER_PULSE,
    IR_LEADER_SPACE,
    IR_DATA_PULSE,
    IR_DATA_SPACE,
    IR_COMPLETE,
    IR_REPEAT,
    IR_ERROR
} IR_State_t;

/* IR Data structure */
typedef struct {
    uint8_t address;        // 8-bit address
    uint8_t address_inv;    // 8-bit inverted address (or extended address)
    uint8_t command;        // 8-bit command
    uint8_t command_inv;    // 8-bit inverted command
    uint32_t raw_data;      // 32-bit raw data
    uint8_t is_repeat;      // Repeat flag
    uint8_t is_valid;       // Valid data flag
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
/* IR Reception variables */
volatile uint32_t pulse_times[MAX_PULSES];
volatile uint32_t space_times[MAX_PULSES];
volatile uint8_t pulse_index = 0;
volatile uint8_t receiving = 0;
volatile uint32_t last_edge_time = 0;
volatile uint8_t last_pin_state = 1;

/* IR Decoded data */
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
 * @brief  Get microseconds from TIM2
 * @note   TIM2 is configured as a free-running 32-bit counter at 1MHz
 * @retval Current microsecond count (wraps after ~71.6 minutes)
 */

/**
 * @brief Reset IR receiver state
 */
void IR_Reset(void) {
    pulse_index = 0;
    receiving = 0;
    memset((void*)pulse_times, 0, sizeof(pulse_times));
    memset((void*)space_times, 0, sizeof(space_times));
    memset(&ir_data, 0, sizeof(ir_data));
}

/**
 * @brief Decode NEC protocol from captured timing data
 * @retval 1 if decode successful, 0 if failed
 *
 * NEC Protocol Frame:
 * ┌─────────┬─────────┬──────────┬──────────┬─────────┬─────────┬──────┐
 * │ Leader  │ Leader  │ Address  │ Address  │ Command │ Command │ Stop │
 * │ Pulse   │ Space   │ (8 bits) │ Inverted │ (8 bits)│ Inverted│ Bit  │
 * │ 9ms     │ 4.5ms   │          │ (8 bits) │         │ (8 bits)│      │
 * └─────────┴─────────┴──────────┴──────────┴─────────┴─────────┴──────┘
 *
 * Repeat Frame:
 * ┌─────────┬─────────┬──────┐
 * │ Leader  │ Leader  │ Stop │
 * │ Pulse   │ Space   │ Bit  │
 * │ 9ms     │ 2.25ms  │      │
 * └─────────┴─────────┴──────┘
 */
uint8_t IR_Decode_NEC(void) {
    /* Need at least leader pulse */
    if (pulse_index < 2) {
        return 0;
    }

    /* Check leader pulse (9ms) */
    if (pulse_times[0] < NEC_LEADER_PULSE_MIN || pulse_times[0] > NEC_LEADER_PULSE_MAX) {
        return 0;
    }

    /* Check for repeat code (2.25ms space after leader) */
    if (space_times[0] >= NEC_REPEAT_SPACE_MIN && space_times[0] <= NEC_REPEAT_SPACE_MAX) {
        if (pulse_index >= 2) {
            ir_data.is_repeat = 1;
            ir_data.is_valid = 1;
            return 1;
        }
        return 0;
    }

    /* Check leader space (4.5ms) */
    if (space_times[0] < NEC_LEADER_SPACE_MIN || space_times[0] > NEC_LEADER_SPACE_MAX) {
        return 0;
    }

    /* Need 33 pulses for full NEC frame (1 leader + 32 data bits) */
    if (pulse_index < 33) {
        return 0;
    }

    /* Decode 32 bits of data */
    uint32_t raw_data = 0;

    for (int i = 0; i < 32; i++) {
        uint32_t pulse = pulse_times[i + 1];
        uint32_t space = space_times[i + 1];

        /* Validate pulse width */
        if (pulse < NEC_BIT_PULSE_MIN || pulse > NEC_BIT_PULSE_MAX) {
            return 0;
        }

        /* Decode bit based on space duration */
        if (space >= NEC_BIT_1_SPACE_MIN && space <= NEC_BIT_1_SPACE_MAX) {
            /* Bit 1 */
            raw_data |= (1UL << i);  // LSB first
        }
        else if (space >= NEC_BIT_0_SPACE_MIN && space <= NEC_BIT_0_SPACE_MAX) {
            /* Bit 0 - already 0 */
        }
        else if (i == 31) {
            /* Last bit might have no space measurement, use pulse only */
            /* This is okay, we already have the bit from pulse presence */
        }
        else {
            /* Invalid space duration */
            return 0;
        }
    }

    /* Extract fields (LSB first transmission) */
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
        printf("    ◆ REPEAT Code\r\n");
        return;
    }

    printf("    ┌──────────────────────────────────┐\r\n");
    printf("    │         NEC Decoded Data         │\r\n");
    printf("    ├──────────────────────────────────┤\r\n");
    printf("    │ Raw Data   : 0x%08lX           │\r\n", ir_data.raw_data);
    printf("    ├──────────────────────────────────┤\r\n");
    printf("    │ Address    : 0x%02X (%3d)         │\r\n", ir_data.address, ir_data.address);
    printf("    │ Address~   : 0x%02X (%3d)         │\r\n", ir_data.address_inv, ir_data.address_inv);
    printf("    │ Command    : 0x%02X (%3d)         │\r\n", ir_data.command, ir_data.command);
    printf("    │ Command~   : 0x%02X (%3d)         │\r\n", ir_data.command_inv, ir_data.command_inv);
    printf("    ├──────────────────────────────────┤\r\n");

    /* Validate standard NEC (address and command should be inverted) */
    uint8_t addr_valid = ((ir_data.address ^ ir_data.address_inv) == 0xFF);
    uint8_t cmd_valid = ((ir_data.command ^ ir_data.command_inv) == 0xFF);

    if (addr_valid && cmd_valid) {
        printf("    │ ✓ Standard NEC Protocol         │\r\n");
        printf("    │   Device : 0x%02X                  │\r\n", ir_data.address);
        printf("    │   Key    : 0x%02X                  │\r\n", ir_data.command);
    }
    else if (cmd_valid) {
        /* Extended NEC - 16-bit address */
        uint16_t ext_address = ir_data.address | (ir_data.address_inv << 8);
        printf("    │ ✓ Extended NEC Protocol         │\r\n");
        printf("    │   Device : 0x%04X                │\r\n", ext_address);
        printf("    │   Key    : 0x%02X                  │\r\n", ir_data.command);
    }
    else {
        printf("    │ ⚠ Checksum Error                │\r\n");
        printf("    │   Addr check: %s              │\r\n", addr_valid ? "OK" : "FAIL");
        printf("    │   Cmd check : %s              │\r\n", cmd_valid ? "OK" : "FAIL");
    }

    printf("    └──────────────────────────────────┘\r\n");

    /* Print binary representation */
    printf("    Binary: ");
    for (int i = 31; i >= 0; i--) {
        printf("%c", (ir_data.raw_data & (1UL << i)) ? '1' : '0');
        if (i == 24 || i == 16 || i == 8) printf(" ");
    }
    printf("\r\n");
    printf("            CMD~     CMD      ADDR~    ADDR\r\n");
}

/**
 * @brief  Get microseconds from TIM2
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
  /* Start microsecond timer */
  HAL_TIM_Base_Start(&htim2);

  printf("\r\n");
  printf("╔════════════════════════════════════════╗\r\n");
  printf("║   IR Receiver & NEC Decoder - STM32    ║\r\n");
  printf("╠════════════════════════════════════════╣\r\n");
  printf("║ System Clock : 64 MHz                  ║\r\n");
  printf("║ IR Input     : PA0 (Pull-up)           ║\r\n");
  printf("║ Status LED   : PA5                     ║\r\n");
  printf("║ Protocol     : NEC / Extended NEC      ║\r\n");
  printf("╚════════════════════════════════════════╝\r\n");
  printf("\r\nWaiting for IR signals...\r\n\n");

  /* Initialize IR receiver state */
  IR_Reset();

  uint32_t signal_count = 0;
  uint32_t last_activity_time = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      uint8_t ir_state = HAL_GPIO_ReadPin(IR_RECV_GPIO_Port, IR_RECV_Pin);
      uint32_t current_us = GetMicros();
      uint32_t current_ms = HAL_GetTick();

      /* Detect edges */
      if (ir_state != last_pin_state) {
          uint32_t duration = current_us - last_edge_time;

          /* Filter noise */
          if (duration > NOISE_FILTER_US || !receiving) {

              if (last_pin_state == 1 && ir_state == 0) {
                  /* Falling edge - Start of pulse (IR signal active) */
                  if (!receiving) {
                      /* Start of new signal */
                      receiving = 1;
                      pulse_index = 0;
                      signal_count++;
                      HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
                  } else if (pulse_index > 0 && pulse_index < MAX_PULSES) {
                      /* Record space duration */
                      space_times[pulse_index - 1] = duration;
                  }
              }
              else if (last_pin_state == 0 && ir_state == 1) {
                  /* Rising edge - End of pulse */
                  if (receiving && pulse_index < MAX_PULSES) {
                      pulse_times[pulse_index] = duration;
                      pulse_index++;
                  }
              }

              last_edge_time = current_us;
              last_activity_time = current_ms;
          }

          last_pin_state = ir_state;
      }

      /* Check for signal timeout (end of transmission) */
      if (receiving && (current_ms - last_activity_time > SIGNAL_TIMEOUT_MS)) {
          receiving = 0;

          printf("────────────────────────────────────────\r\n");
          printf("[%lu] IR Signal Received\r\n", signal_count);
          printf("    Pulses captured: %d\r\n", pulse_index);

          /* Try to decode NEC protocol */
          if (IR_Decode_NEC()) {
              IR_Print_Result();
          } else {
              printf("    Decode: Failed (Not NEC or corrupted)\r\n");

              /* Print raw timing for debugging */
              printf("    Raw timing (first 10 pulses):\r\n");
              for (int i = 0; i < pulse_index && i < 10; i++) {
                  printf("      [%d] P:%lu us", i, pulse_times[i]);
                  if (i < pulse_index - 1) {
                      printf(", S:%lu us", space_times[i]);
                  }
                  printf("\r\n");
              }
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
