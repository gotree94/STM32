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
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define RED_PIN         GPIO_PIN_6
#define GREEN_PIN       GPIO_PIN_7
#define LED_PORT        GPIOA
#define PWM_PERIOD      999

/* LED Colors */
typedef enum {
    COLOR_OFF = 0,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,    // Red + Green
    COLOR_ORANGE     // Red 많이 + Green 조금
} LED_Color_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART2_UART_Init(void);
void DualLED_SetColor(LED_Color_t color);
void DualLED_SetRGB(uint8_t red, uint8_t green);
void DualLED_TrafficLight(void);
void DualLED_StatusIndicator(void);
void DualLED_ColorMix(void);
void DualLED_Alternating(void);

/* UART printf 리다이렉션 */
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * @brief 사전 정의 색상 설정
 */
void DualLED_SetColor(LED_Color_t color)
{
    switch (color) {
        case COLOR_OFF:
            DualLED_SetRGB(0, 0);
            break;
        case COLOR_RED:
            DualLED_SetRGB(255, 0);
            break;
        case COLOR_GREEN:
            DualLED_SetRGB(0, 255);
            break;
        case COLOR_YELLOW:
            DualLED_SetRGB(255, 255);
            break;
        case COLOR_ORANGE:
            DualLED_SetRGB(255, 80);
            break;
    }
}

/**
 * @brief Red/Green 개별 밝기 설정 (0~255)
 */
void DualLED_SetRGB(uint8_t red, uint8_t green)
{
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, (red * PWM_PERIOD) / 255);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, (green * PWM_PERIOD) / 255);
}

/**
 * @brief 신호등 시뮬레이션
 *        Green(3초) -> Yellow(1초) -> Red(3초) -> 반복
 */
void DualLED_TrafficLight(void)
{
    for (int cycle = 0; cycle < 2; cycle++) {
        printf("  GREEN (Go) - 3 sec\r\n");
        DualLED_SetColor(COLOR_GREEN);
        HAL_Delay(3000);

        printf("  YELLOW (Caution) - 1 sec\r\n");
        DualLED_SetColor(COLOR_YELLOW);
        HAL_Delay(1000);

        printf("  RED (Stop) - 3 sec\r\n");
        DualLED_SetColor(COLOR_RED);
        HAL_Delay(3000);
    }
}

/**
 * @brief 상태 표시기 데모
 *        배터리 레벨, 시스템 상태 등 시각화
 */
void DualLED_StatusIndicator(void)
{
    printf("  Simulating battery level:\r\n");

    /* 배터리 레벨 시뮬레이션 */
    printf("    100%% - Green\r\n");
    DualLED_SetColor(COLOR_GREEN);
    HAL_Delay(1000);

    printf("    75%% - Green (dim)\r\n");
    DualLED_SetRGB(0, 180);
    HAL_Delay(1000);

    printf("    50%% - Yellow\r\n");
    DualLED_SetColor(COLOR_YELLOW);
    HAL_Delay(1000);

    printf("    25%% - Orange\r\n");
    DualLED_SetColor(COLOR_ORANGE);
    HAL_Delay(1000);

    printf("    10%% - Red (blinking)\r\n");
    for (int i = 0; i < 6; i++) {
        DualLED_SetColor(COLOR_RED);
        HAL_Delay(250);
        DualLED_SetColor(COLOR_OFF);
        HAL_Delay(250);
    }

    printf("    0%% - Red (fast blink)\r\n");
    for (int i = 0; i < 12; i++) {
        DualLED_SetColor(COLOR_RED);
        HAL_Delay(100);
        DualLED_SetColor(COLOR_OFF);
        HAL_Delay(100);
    }
}

/**
 * @brief 색상 그라데이션 효과
 *        Red -> Orange -> Yellow -> Green 부드러운 전환
 */
void DualLED_ColorMix(void)
{
    /* Red에서 시작 */
    DualLED_SetRGB(255, 0);
    HAL_Delay(500);

    /* Red -> Yellow (Green 증가) */
    printf("  Red -> Yellow...\r\n");
    for (int g = 0; g <= 255; g += 3) {
        DualLED_SetRGB(255, g);
        HAL_Delay(10);
    }
    HAL_Delay(300);

    /* Yellow -> Green (Red 감소) */
    printf("  Yellow -> Green...\r\n");
    for (int r = 255; r >= 0; r -= 3) {
        DualLED_SetRGB(r, 255);
        HAL_Delay(10);
    }
    HAL_Delay(300);

    /* Green -> Yellow (Red 증가) */
    printf("  Green -> Yellow...\r\n");
    for (int r = 0; r <= 255; r += 3) {
        DualLED_SetRGB(r, 255);
        HAL_Delay(10);
    }
    HAL_Delay(300);

    /* Yellow -> Red (Green 감소) */
    printf("  Yellow -> Red...\r\n");
    for (int g = 255; g >= 0; g -= 3) {
        DualLED_SetRGB(255, g);
        HAL_Delay(10);
    }
    HAL_Delay(300);

    DualLED_SetColor(COLOR_OFF);
}

/**
 * @brief 교대 점멸 효과
 */
void DualLED_Alternating(void)
{
    printf("  Slow alternating...\r\n");
    for (int i = 0; i < 5; i++) {
        DualLED_SetColor(COLOR_RED);
        HAL_Delay(500);
        DualLED_SetColor(COLOR_GREEN);
        HAL_Delay(500);
    }

    printf("  Fast alternating...\r\n");
    for (int i = 0; i < 20; i++) {
        DualLED_SetColor(COLOR_RED);
        HAL_Delay(100);
        DualLED_SetColor(COLOR_GREEN);
        HAL_Delay(100);
    }

    printf("  Cross-fade...\r\n");
    for (int i = 0; i < 3; i++) {
        /* Red -> Green fade */
        for (int j = 0; j <= 255; j += 5) {
            DualLED_SetRGB(255 - j, j);
            HAL_Delay(8);
        }
        /* Green -> Red fade */
        for (int j = 255; j >= 0; j -= 5) {
            DualLED_SetRGB(255 - j, j);
            HAL_Delay(8);
        }
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
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  /* PWM 시작 */
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);  // Red
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);  // Green

  printf("\r\n============================================\r\n");
  printf("  Dual Color LED Module Test - NUCLEO-F103RB\r\n");
  printf("============================================\r\n\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      /* Test 1: 기본 색상 테스트 */
      printf("[Test 1] Basic Colors\r\n");

      printf("  OFF...\r\n");
      DualLED_SetColor(COLOR_OFF);
      HAL_Delay(1000);

      printf("  RED...\r\n");
      DualLED_SetColor(COLOR_RED);
      HAL_Delay(1000);

      printf("  GREEN...\r\n");
      DualLED_SetColor(COLOR_GREEN);
      HAL_Delay(1000);

      printf("  YELLOW (R+G)...\r\n");
      DualLED_SetColor(COLOR_YELLOW);
      HAL_Delay(1000);

      printf("  ORANGE (R+g)...\r\n");
      DualLED_SetColor(COLOR_ORANGE);
      HAL_Delay(1000);

      DualLED_SetColor(COLOR_OFF);
      HAL_Delay(500);

      /* Test 2: 신호등 시뮬레이션 */
      printf("\r\n[Test 2] Traffic Light Simulation\r\n");
      DualLED_TrafficLight();
      HAL_Delay(500);

      /* Test 3: 상태 표시기 */
      printf("\r\n[Test 3] Status Indicator\r\n");
      DualLED_StatusIndicator();
      HAL_Delay(500);

      /* Test 4: 색상 그라데이션 */
      printf("\r\n[Test 4] Color Gradient (Red -> Yellow -> Green)\r\n");
      DualLED_ColorMix();
      HAL_Delay(500);

      /* Test 5: 교대 점멸 */
      printf("\r\n[Test 5] Alternating Blink\r\n");
      DualLED_Alternating();

      DualLED_SetColor(COLOR_OFF);

      printf("\r\n--- Cycle Complete ---\r\n\n");
      HAL_Delay(2000);
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
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 63;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

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
