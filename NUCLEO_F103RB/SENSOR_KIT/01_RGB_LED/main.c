/**
 ******************************************************************************
 * @file    main.c
 * @author  Namu
 * @brief   RGB LED Module Test for NUCLEO-F103RB
 * @note    RGB LED는 공통 캐소드(Common Cathode) 타입 사용
 *          PWM을 이용한 색상 혼합 구현
 ******************************************************************************
 * @hardware
 *   - Board: NUCLEO-F103RB
 *   - RGB LED Module (KY-016 또는 유사 모듈)
 *   - Connections:
 *     - R (Red)   -> PA0 (TIM2_CH1)
 *     - G (Green) -> PA1 (TIM2_CH2)  
 *     - B (Blue)  -> PB10 (TIM2_CH3)
 *     - GND       -> GND (공통 캐소드) 또는 3.3V (공통 애노드)
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include "stm32f1xx_hal.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
void RGB_SetColor(uint8_t red, uint8_t green, uint8_t blue);
void RGB_Demo_Fade(void);
void RGB_Demo_Rainbow(void);
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PWM_PERIOD      999     // PWM 주기 (0~999 = 1000단계)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
/* UART printf 리다이렉션 */
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * @brief RGB LED 색상 설정 (0~255)
 */
void RGB_SetColor(uint8_t red, uint8_t green, uint8_t blue)
{
    /* 0~255를 0~PWM_PERIOD로 변환 */
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, (red * PWM_PERIOD) / 255);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, (green * PWM_PERIOD) / 255);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, (blue * PWM_PERIOD) / 255);
}

/**
 * @brief 페이드 효과 데모
 */
void RGB_Demo_Fade(void)
{
    /* Red 페이드 인/아웃 */
    for (int i = 0; i <= 255; i += 5) {
        RGB_SetColor(i, 0, 0);
        HAL_Delay(10);
    }
    for (int i = 255; i >= 0; i -= 5) {
        RGB_SetColor(i, 0, 0);
        HAL_Delay(10);
    }

    /* Green 페이드 인/아웃 */
    for (int i = 0; i <= 255; i += 5) {
        RGB_SetColor(0, i, 0);
        HAL_Delay(10);
    }
    for (int i = 255; i >= 0; i -= 5) {
        RGB_SetColor(0, i, 0);
        HAL_Delay(10);
    }

    /* Blue 페이드 인/아웃 */
    for (int i = 0; i <= 255; i += 5) {
        RGB_SetColor(0, 0, i);
        HAL_Delay(10);
    }
    for (int i = 255; i >= 0; i -= 5) {
        RGB_SetColor(0, 0, i);
        HAL_Delay(10);
    }
}

/**
 * @brief 레인보우 효과 데모 (색상환 순환)
 */
void RGB_Demo_Rainbow(void)
{
    uint8_t r, g, b;

    for (int i = 0; i < 360; i += 2) {
        /* HSV to RGB 변환 (S=1, V=1 고정) */
        int region = i / 60;
        int remainder = (i - (region * 60)) * 255 / 60;

        switch (region) {
            case 0:  r = 255; g = remainder; b = 0; break;
            case 1:  r = 255 - remainder; g = 255; b = 0; break;
            case 2:  r = 0; g = 255; b = remainder; break;
            case 3:  r = 0; g = 255 - remainder; b = 255; break;
            case 4:  r = remainder; g = 0; b = 255; break;
            default: r = 255; g = 0; b = 255 - remainder; break;
        }

        RGB_SetColor(r, g, b);
        HAL_Delay(20);
    }

    RGB_SetColor(0, 0, 0);
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
  /* PWM 시작 */
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);  // Red
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);  // Green
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);  // Blue

  printf("\r\n========================================\r\n");
  printf("  RGB LED Module Test - NUCLEO-F103RB\r\n");
  printf("========================================\r\n\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  /* 기본 색상 테스트 */
	         printf("[Test 1] Basic Colors\r\n");

	         printf("  Red...\r\n");
	         RGB_SetColor(255, 0, 0);
	         HAL_Delay(1000);

	         printf("  Green...\r\n");
	         RGB_SetColor(0, 255, 0);
	         HAL_Delay(1000);

	         printf("  Blue...\r\n");
	         RGB_SetColor(0, 0, 255);
	         HAL_Delay(1000);

	         printf("  Yellow (R+G)...\r\n");
	         RGB_SetColor(255, 255, 0);
	         HAL_Delay(1000);

	         printf("  Cyan (G+B)...\r\n");
	         RGB_SetColor(0, 255, 255);
	         HAL_Delay(1000);

	         printf("  Magenta (R+B)...\r\n");
	         RGB_SetColor(255, 0, 255);
	         HAL_Delay(1000);

	         printf("  White (R+G+B)...\r\n");
	         RGB_SetColor(255, 255, 255);
	         HAL_Delay(1000);

	         printf("  OFF...\r\n\n");
	         RGB_SetColor(0, 0, 0);
	         HAL_Delay(500);

	         /* 페이드 효과 */
	         printf("[Test 2] Fade Effect\r\n");
	         RGB_Demo_Fade();
	         HAL_Delay(500);

	         /* 레인보우 효과 */
	         printf("[Test 3] Rainbow Effect\r\n");
	         RGB_Demo_Rainbow();
	         HAL_Delay(500);

	         printf("\r\n--- Cycle Complete ---\r\n\n");
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
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 63;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

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
