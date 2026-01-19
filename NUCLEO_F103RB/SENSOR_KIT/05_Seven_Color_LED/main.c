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

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LED_PIN         GPIO_PIN_0
#define LED_PORT        GPIOB
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* UART printf 리다이렉션 */
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * @brief LED ON (내장 IC 자동 색상 변환 시작)
 */
void SevenColorLED_On(void)
{
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
}

/**
 * @brief LED OFF
 */
void SevenColorLED_Off(void)
{
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
}

/**
 * @brief LED 토글
 */
void SevenColorLED_Toggle(void)
{
    HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
}

/**
 * @brief 자동 색상 변환 관찰
 *        내장 IC가 자동으로 색상을 변환하므로 전원만 공급하면 됨
 */
void SevenColorLED_AutoCycleDemo(void)
{
    printf("  Observing auto color cycle for 10 seconds...\r\n");
    printf("  Colors: Red -> Orange -> Yellow -> Green -> Cyan -> Blue -> Purple\r\n");

    SevenColorLED_On();

    /* 색상 변화 관찰 */
    for (int i = 0; i < 10; i++) {
        printf("  [%d sec]\r\n", i + 1);
        HAL_Delay(1000);
    }

    SevenColorLED_Off();
}

/**
 * @brief 스트로브 효과 (ON/OFF 반복)
 */
void SevenColorLED_StrobeDemo(void)
{
    /* 느린 스트로브 */
    printf("  Slow strobe (500ms)...\r\n");
    for (int i = 0; i < 6; i++) {
        SevenColorLED_On();
        HAL_Delay(500);
        SevenColorLED_Off();
        HAL_Delay(500);
    }

    HAL_Delay(300);

    /* 빠른 스트로브 */
    printf("  Fast strobe (100ms)...\r\n");
    for (int i = 0; i < 20; i++) {
        SevenColorLED_On();
        HAL_Delay(100);
        SevenColorLED_Off();
        HAL_Delay(100);
    }

    HAL_Delay(300);

    /* 점점 빨라지는 스트로브 */
    printf("  Accelerating strobe...\r\n");
    for (int delay = 500; delay >= 50; delay -= 50) {
        SevenColorLED_On();
        HAL_Delay(delay);
        SevenColorLED_Off();
        HAL_Delay(delay);
    }

    SevenColorLED_Off();
}

/**
 * @brief 다양한 패턴 데모 (ON/OFF 타이밍 기반)
 */
void SevenColorLED_PatternDemo(void)
{
    /* 패턴 1: 심박 효과 (두 번 빠르게, 긴 휴식) */
    printf("  Heartbeat pattern...\r\n");
    for (int beat = 0; beat < 5; beat++) {
        /* 첫 번째 박동 */
        SevenColorLED_On();
        HAL_Delay(100);
        SevenColorLED_Off();
        HAL_Delay(100);

        /* 두 번째 박동 */
        SevenColorLED_On();
        HAL_Delay(100);
        SevenColorLED_Off();
        HAL_Delay(500);
    }

    HAL_Delay(500);

    /* 패턴 2: SOS 패턴 */
    printf("  SOS pattern...\r\n");
    for (int sos = 0; sos < 2; sos++) {
        /* S: 짧게 3번 */
        for (int i = 0; i < 3; i++) {
            SevenColorLED_On();
            HAL_Delay(200);
            SevenColorLED_Off();
            HAL_Delay(200);
        }
        HAL_Delay(300);

        /* O: 길게 3번 */
        for (int i = 0; i < 3; i++) {
            SevenColorLED_On();
            HAL_Delay(500);
            SevenColorLED_Off();
            HAL_Delay(200);
        }
        HAL_Delay(300);

        /* S: 짧게 3번 */
        for (int i = 0; i < 3; i++) {
            SevenColorLED_On();
            HAL_Delay(200);
            SevenColorLED_Off();
            HAL_Delay(200);
        }
        HAL_Delay(1000);
    }

    HAL_Delay(500);

    /* 패턴 3: 카운트다운 */
    printf("  Countdown pattern (5 to 1)...\r\n");
    for (int count = 5; count >= 1; count--) {
        printf("    %d\r\n", count);
        for (int i = 0; i < count; i++) {
            SevenColorLED_On();
            HAL_Delay(150);
            SevenColorLED_Off();
            HAL_Delay(150);
        }
        HAL_Delay(500);
    }

    /* 완료 표시: 길게 점등 */
    SevenColorLED_On();
    HAL_Delay(1000);
    SevenColorLED_Off();
}

/**
 * @brief 알림 패턴 데모
 */
void SevenColorLED_NotificationDemo(void)
{
    /* 알림 1: 새 메시지 (부드러운 펄스 3회) */
    printf("  New message notification...\r\n");
    for (int i = 0; i < 3; i++) {
        SevenColorLED_On();
        HAL_Delay(300);
        SevenColorLED_Off();
        HAL_Delay(300);
    }

    HAL_Delay(500);

    /* 알림 2: 경고 (빠른 점멸 6회) */
    printf("  Warning notification...\r\n");
    for (int i = 0; i < 6; i++) {
        SevenColorLED_On();
        HAL_Delay(100);
        SevenColorLED_Off();
        HAL_Delay(100);
    }

    HAL_Delay(500);

    /* 알림 3: 대기 중 (느린 점멸) */
    printf("  Standby notification...\r\n");
    for (int i = 0; i < 3; i++) {
        SevenColorLED_On();
        HAL_Delay(1000);
        SevenColorLED_Off();
        HAL_Delay(1000);
    }

    HAL_Delay(500);

    /* 알림 4: 완료 (길게 점등) */
    printf("  Completion notification...\r\n");
    SevenColorLED_On();
    HAL_Delay(300);
    SevenColorLED_Off();
    HAL_Delay(200);
    SevenColorLED_On();
    HAL_Delay(300);
    SevenColorLED_Off();
    HAL_Delay(200);
    SevenColorLED_On();
    HAL_Delay(1500);

    SevenColorLED_Off();
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
  /* USER CODE BEGIN 2 */
  printf("\r\n=============================================\r\n");
  printf("  7-Color LED Module Test - NUCLEO-F103RB\r\n");
  printf("=============================================\r\n");
  printf("  Module: KY-034 (Auto Color Cycling)\r\n");
  printf("  Control: GPIO ON/OFF (No PWM)\r\n");
  printf("  Pin: PB0\r\n");
  printf("\r\n");

  /* 초기 테스트 */
  printf("[Init] Quick LED test...\r\n");
  SevenColorLED_On();
  HAL_Delay(1000);
  SevenColorLED_Off();
  HAL_Delay(500);
  printf("[Init] LED test complete.\r\n\r\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      /* Test 1: 기본 ON/OFF */
      printf("[Test 1] Basic ON/OFF Control\r\n");

      printf("  LED ON (5 seconds - watch color changes)...\r\n");
      SevenColorLED_On();
      HAL_Delay(5000);

      printf("  LED OFF...\r\n");
      SevenColorLED_Off();
      HAL_Delay(1000);

      /* Test 2: 자동 색상 변환 관찰 */
      printf("\r\n[Test 2] Auto Color Cycle Observation\r\n");
      SevenColorLED_AutoCycleDemo();
      HAL_Delay(500);

      /* Test 3: 스트로브 효과 */
      printf("\r\n[Test 3] Strobe Effect\r\n");
      SevenColorLED_StrobeDemo();
      HAL_Delay(500);

      /* Test 4: 패턴 데모 */
      printf("\r\n[Test 4] Pattern Demo\r\n");
      SevenColorLED_PatternDemo();
      HAL_Delay(500);

      /* Test 5: 알림 데모 */
      printf("\r\n[Test 5] Notification Patterns\r\n");
      SevenColorLED_NotificationDemo();

      SevenColorLED_Off();

      printf("\r\n--- Cycle Complete ---\r\n\r\n");
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

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

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

  /*Configure GPIO pin : PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
