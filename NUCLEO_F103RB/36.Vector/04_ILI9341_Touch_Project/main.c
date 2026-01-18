/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : ILI9341 LCD + 저항막 터치스크린 (ADC 방식)
  ******************************************************************************
  *
  * 수정사항:
  * - XPT2046 (SPI) → TouchADC (저항막 ADC) 로 변경
  * - ADC1 초기화 추가 필요 (PA1, PA4 채널)
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ili9341.h"
#include "touch_adc.h"    // ← 변경: xpt2046.h 대신 touch_adc.h
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BOXSIZE 40
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
TS_Point touch;                 // ← 변경: Touch_Point → TS_Point
char debug_str[60];

// 색상 팔레트
const uint16_t colors[] = {RED, YELLOW, GREEN, CYAN, BLUE, MAGENTA};
uint16_t currentcolor = RED;
int selected_index = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
void Touch_Calibration_Demo(void);
void Touch_Button_Demo(void);
void Touch_Paint_Demo(void);
void Touch_Basic_Demo(void);
void Touch_Hardware_Test(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// ==================== 터치 하드웨어 디버그 테스트 ====================
void Touch_Hardware_Test(void) {
    ILI9341_Fill(WHITE);
    ILI9341_DrawString(10, 10, "ADC Touch Test", WHITE, BLACK);
    ILI9341_DrawString(10, 30, "Resistive Touch", YELLOW, BLACK);

    char test_str[60];
    uint16_t test_count = 0;

    while(1) {
        // 1. 압력 읽기
        uint16_t pressure = TouchADC_GetPressure();
        sprintf(test_str, "Pressure: %4d", pressure);
        ILI9341_FillRect(10, 60, 220, 10, BLACK);
        ILI9341_DrawString(10, 60, test_str, pressure > 10 ? GREEN : WHITE, BLACK);

        // 2. Raw 값 읽기
        uint16_t raw_x, raw_y;
        TouchADC_ReadRaw(&raw_x, &raw_y);

        sprintf(test_str, "Raw X: %4d", raw_x);
        ILI9341_FillRect(10, 80, 220, 10, BLACK);
        ILI9341_DrawString(10, 80, test_str, raw_x > 100 ? CYAN : WHITE, BLACK);

        sprintf(test_str, "Raw Y: %4d", raw_y);
        ILI9341_FillRect(10, 95, 220, 10, BLACK);
        ILI9341_DrawString(10, 95, test_str, raw_y > 100 ? CYAN : WHITE, BLACK);

        // 3. 전체 터치 읽기
        TS_Point touch_test;
        bool touched = TouchADC_Read(&touch_test);

        sprintf(test_str, "Touched: %s", touched ? "YES" : "NO ");
        ILI9341_FillRect(10, 115, 220, 10, BLACK);
        ILI9341_DrawString(10, 115, test_str, touched ? GREEN : RED, BLACK);

        if(touched && touch_test.pressed) {
            sprintf(test_str, "X:%3d Y:%3d Z:%4d",
                    touch_test.x, touch_test.y, touch_test.z);
            ILI9341_FillRect(10, 135, 220, 10, BLACK);
            ILI9341_DrawString(10, 135, test_str, GREEN, BLACK);

            // 터치 위치에 점 그리기
            ILI9341_FillRect(touch_test.x - 2, touch_test.y - 2, 5, 5, RED);
        } else {
            ILI9341_FillRect(10, 135, 220, 10, BLACK);
            ILI9341_DrawString(10, 135, "No Touch Detected", RED, BLACK);
        }

        // 4. 테스트 카운터
        sprintf(test_str, "Count: %5d", test_count++);
        ILI9341_FillRect(10, 160, 220, 10, BLACK);
        ILI9341_DrawString(10, 160, test_str, WHITE, BLACK);

        // 안내
        ILI9341_DrawString(10, 190, "Touch the screen!", YELLOW, BLACK);

        // UART 출력
        sprintf(test_str, "P:%d RX:%d RY:%d X:%d Y:%d\r\n",
                pressure, raw_x, raw_y,
                touch_test.x, touch_test.y);
        HAL_UART_Transmit(&huart2, (uint8_t*)test_str,
                         (uint16_t)strlen(test_str), 100);

        HAL_Delay(100);
    }
}

// ==================== 터치 캘리브레이션 데모 ====================
void Touch_Calibration_Demo(void) {
    ILI9341_Fill(WHITE);
    ILI9341_DrawString(10, 10, "Touch Calibration", WHITE, BLACK);
    ILI9341_DrawString(10, 30, "ADC Resistive Mode", YELLOW, BLACK);

    // 캘리브레이션 포인트 표시
    ILI9341_FillRect(10, 50, 10, 10, RED);
    ILI9341_DrawString(25, 52, "1. Top-Left", WHITE, BLACK);
    ILI9341_FillRect(220, 50, 10, 10, RED);
    ILI9341_DrawString(130, 52, "2. Top-Right", WHITE, BLACK);
    ILI9341_FillRect(10, 300, 10, 10, RED);
    ILI9341_DrawString(25, 302, "3. Bottom-Left", WHITE, BLACK);
    ILI9341_FillRect(220, 300, 10, 10, RED);
    ILI9341_DrawString(110, 302, "4. Bottom-Right", WHITE, BLACK);
    ILI9341_FillRect(115, 155, 10, 10, GREEN);
    ILI9341_DrawString(70, 170, "5. Center", WHITE, BLACK);

    char cal_str[60];
    TS_Point cal_point;

    while(1) {
        if(TouchADC_Read(&cal_point)) {
            if(cal_point.pressed) {
                // Raw 값 표시
                sprintf(cal_str, "Raw X: %4d  Y: %4d",
                        cal_point.raw_x, cal_point.raw_y);
                ILI9341_FillRect(10, 200, 220, 10, BLACK);
                ILI9341_DrawString(10, 200, cal_str, CYAN, BLACK);

                // 캘리브레이션된 값 표시
                sprintf(cal_str, "Cal X: %3d   Y: %3d",
                        cal_point.x, cal_point.y);
                ILI9341_FillRect(10, 215, 220, 10, BLACK);
                ILI9341_DrawString(10, 215, cal_str, GREEN, BLACK);

                // 압력 표시
                sprintf(cal_str, "Pressure: %4d", cal_point.z);
                ILI9341_FillRect(10, 230, 220, 10, BLACK);
                ILI9341_DrawString(10, 230, cal_str, YELLOW, BLACK);

                // UART 출력
                sprintf(cal_str, "Raw: X=%4d, Y=%4d, P=%4d\r\n",
                        cal_point.raw_x, cal_point.raw_y, cal_point.z);
                HAL_UART_Transmit(&huart2, (uint8_t*)cal_str,
                                 (uint16_t)strlen(cal_str), 100);
            }
        } else {
            ILI9341_FillRect(10, 200, 220, 10, BLACK);
            ILI9341_DrawString(10, 200, "No Touch", WHITE, BLACK);
        }

        HAL_Delay(50);
    }
}

// ==================== 버튼 데모 ====================
void Touch_Button_Demo(void) {
    ILI9341_Fill(WHITE);
    ILI9341_DrawString(60, 10, "Button Demo", WHITE, BLACK);

    typedef struct {
        uint16_t x, y, w, h;
        char* text;
        uint16_t color;
    } Button;

    Button buttons[] = {
        {20, 50, 80, 40, "Button 1", RED},
        {120, 50, 80, 40, "Button 2", GREEN},
        {20, 110, 80, 40, "Button 3", BLUE},
        {120, 110, 80, 40, "Button 4", YELLOW},
        {20, 170, 180, 40, "Clear", MAGENTA}
    };
    int num_buttons = 5;

    // 버튼 그리기
    for(int i = 0; i < num_buttons; i++) {
        ILI9341_FillRect(buttons[i].x, buttons[i].y,
                        buttons[i].w, buttons[i].h, buttons[i].color);
        ILI9341_DrawRect(buttons[i].x, buttons[i].y,
                        buttons[i].w, buttons[i].h, WHITE);
        ILI9341_DrawString(buttons[i].x + 10, buttons[i].y + 15,
                          buttons[i].text, BLACK, buttons[i].color);
    }

    ILI9341_DrawRect(10, 230, 220, 80, WHITE);
    ILI9341_DrawString(15, 235, "Touch Status:", CYAN, BLACK);

    TS_Point touch_btn;
    char status_str[40];

    while(1) {
        if(TouchADC_Read(&touch_btn)) {
            if(touch_btn.pressed) {
                for(int i = 0; i < num_buttons; i++) {
                    if(touch_btn.x >= buttons[i].x &&
                       touch_btn.x <= buttons[i].x + buttons[i].w &&
                       touch_btn.y >= buttons[i].y &&
                       touch_btn.y <= buttons[i].y + buttons[i].h) {

                        // 버튼 눌림 표시
                        ILI9341_DrawRect(buttons[i].x, buttons[i].y,
                                        buttons[i].w, buttons[i].h, WHITE);
                        ILI9341_DrawRect(buttons[i].x+1, buttons[i].y+1,
                                        buttons[i].w-2, buttons[i].h-2, WHITE);

                        sprintf(status_str, "%s Pressed!", buttons[i].text);
                        ILI9341_FillRect(15, 255, 210, 10, BLACK);
                        ILI9341_DrawString(15, 255, status_str, buttons[i].color, BLACK);

                        // Clear 버튼
                        if(i == 4) {
                            HAL_Delay(300);
                            ILI9341_Fill(WHITE);
                            ILI9341_DrawString(60, 10, "Button Demo", WHITE, BLACK);
                            for(int j = 0; j < num_buttons; j++) {
                                ILI9341_FillRect(buttons[j].x, buttons[j].y,
                                                buttons[j].w, buttons[j].h, buttons[j].color);
                                ILI9341_DrawRect(buttons[j].x, buttons[j].y,
                                                buttons[j].w, buttons[j].h, WHITE);
                                ILI9341_DrawString(buttons[j].x + 10, buttons[j].y + 15,
                                                  buttons[j].text, BLACK, buttons[j].color);
                            }
                            ILI9341_DrawRect(10, 230, 220, 80, WHITE);
                            ILI9341_DrawString(15, 235, "Touch Status:", CYAN, BLACK);
                        }

                        HAL_Delay(200);
                        ILI9341_DrawRect(buttons[i].x, buttons[i].y,
                                        buttons[i].w, buttons[i].h, WHITE);
                        break;
                    }
                }

                sprintf(status_str, "X:%3d Y:%3d", touch_btn.x, touch_btn.y);
                ILI9341_FillRect(15, 270, 210, 10, BLACK);
                ILI9341_DrawString(15, 270, status_str, WHITE, BLACK);
            }
        }
        HAL_Delay(50);
    }
}

// ==================== 그림판 데모 (아두이노 스타일) ====================
void Touch_Paint_Demo(void) {
    ILI9341_Fill(BLACK);

    // 색상 버튼 그리기
    ILI9341_FillRect(0, 0, BOXSIZE, BOXSIZE, RED);
    ILI9341_FillRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, YELLOW);
    ILI9341_FillRect(BOXSIZE * 2, 0, BOXSIZE, BOXSIZE, GREEN);
    ILI9341_FillRect(BOXSIZE * 3, 0, BOXSIZE, BOXSIZE, CYAN);
    ILI9341_FillRect(BOXSIZE * 4, 0, BOXSIZE, BOXSIZE, BLUE);
    ILI9341_FillRect(BOXSIZE * 5, 0, BOXSIZE, BOXSIZE, MAGENTA);

    // 초기 선택 (빨간색)
    ILI9341_DrawRect(0, 0, BOXSIZE, BOXSIZE, WHITE);
    ILI9341_DrawRect(1, 1, BOXSIZE-2, BOXSIZE-2, WHITE);
    currentcolor = RED;
    selected_index = 0;

    TS_Point p;

    while(1) {
        if(TouchADC_Read(&p)) {
            if(p.pressed) {
                // 색상 선택 영역 (상단)
                if(p.y < BOXSIZE) {
                    // 모든 버튼 다시 그리기 (테두리 제거)
                    ILI9341_FillRect(0, 0, BOXSIZE, BOXSIZE, RED);
                    ILI9341_FillRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, YELLOW);
                    ILI9341_FillRect(BOXSIZE * 2, 0, BOXSIZE, BOXSIZE, GREEN);
                    ILI9341_FillRect(BOXSIZE * 3, 0, BOXSIZE, BOXSIZE, CYAN);
                    ILI9341_FillRect(BOXSIZE * 4, 0, BOXSIZE, BOXSIZE, BLUE);
                    ILI9341_FillRect(BOXSIZE * 5, 0, BOXSIZE, BOXSIZE, MAGENTA);

                    // 선택된 색상 결정
                    if(p.x < BOXSIZE) {
                        currentcolor = RED;
                        ILI9341_DrawRect(0, 0, BOXSIZE, BOXSIZE, WHITE);
                    } else if(p.x < BOXSIZE * 2) {
                        currentcolor = YELLOW;
                        ILI9341_DrawRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, WHITE);
                    } else if(p.x < BOXSIZE * 3) {
                        currentcolor = GREEN;
                        ILI9341_DrawRect(BOXSIZE * 2, 0, BOXSIZE, BOXSIZE, WHITE);
                    } else if(p.x < BOXSIZE * 4) {
                        currentcolor = CYAN;
                        ILI9341_DrawRect(BOXSIZE * 3, 0, BOXSIZE, BOXSIZE, WHITE);
                    } else if(p.x < BOXSIZE * 5) {
                        currentcolor = BLUE;
                        ILI9341_DrawRect(BOXSIZE * 4, 0, BOXSIZE, BOXSIZE, WHITE);
                    } else if(p.x < BOXSIZE * 6) {
                        currentcolor = MAGENTA;
                        ILI9341_DrawRect(BOXSIZE * 5, 0, BOXSIZE, BOXSIZE, WHITE);
                    }

                    HAL_Delay(100);  // 연속 터치 방지
                }
                // 그리기 영역
                else {
                    ILI9341_FillRect(p.x - 1, p.y - 1, 3, 3, currentcolor);
                }
            }
        }

        HAL_Delay(10);  // 빠른 반응을 위해 짧은 딜레이
    }
}

// ==================== 기본 터치 테스트 ====================
void Touch_Basic_Demo(void) {
    ILI9341_Fill(WHITE);
    ILI9341_DrawString(10, 10, "STM32 + ILI9341", WHITE, BLACK);
    ILI9341_DrawString(10, 25, "ADC Touch Demo", CYAN, BLACK);
    ILI9341_DrawString(10, 40, "Touch to draw!", YELLOW, BLACK);

    ILI9341_DrawRect(5, 80, 230, 230, WHITE);

    uint16_t last_x = 0, last_y = 0;
    bool drawing = false;
    TS_Point touch_basic;

    while(1) {
        if(TouchADC_Read(&touch_basic)) {
            if(touch_basic.pressed && touch_basic.y > 80) {
                ILI9341_DrawPixel(touch_basic.x, touch_basic.y, RED);

                if(drawing) {
                    ILI9341_DrawLine(last_x, last_y, touch_basic.x, touch_basic.y, RED);
                }

                last_x = touch_basic.x;
                last_y = touch_basic.y;
                drawing = true;

                sprintf(debug_str, "X:%3d Y:%3d P:%4d",
                        touch_basic.x, touch_basic.y, touch_basic.z);
                ILI9341_FillRect(10, 310, 220, 10, BLACK);
                ILI9341_DrawString(10, 310, debug_str, CYAN, BLACK);
            }
        } else {
            drawing = false;
        }

        HAL_Delay(10);
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
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  // LCD 초기화
  ILI9341_Init();

  // 터치스크린 초기화 (ADC 방식)
  TouchADC_Init(&hadc1);

  // 캘리브레이션 값 설정 (실제 측정 후 조정)
  TouchADC_SetCalibration(150, 120, 920, 940);

  // ═══════════════════════════════════════════════════════════
  // ★★★ 원하는 데모 선택 (하나만 주석 해제) ★★★
  // ═══════════════════════════════════════════════════════════

  // 1. 하드웨어 테스트 (먼저 실행 권장!)
  Touch_Hardware_Test();

  // 2. 캘리브레이션 데모
  // Touch_Calibration_Demo();

  // 3. 그림판 데모 (아두이노 스타일)
  // Touch_Paint_Demo();

  // 4. 버튼 데모
  // Touch_Button_Demo();

  // 5. 기본 터치 테스트
  // Touch_Basic_Demo();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    HAL_Delay(10);
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

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
  HAL_GPIO_WritePin(GPIOC, LCD_RST_Pin|LCD_D1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LCD_RD_Pin|LD2_Pin|LCD_D7_Pin|LCD_D0_Pin
                          |LCD_D2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_CS_Pin|LCD_D6_Pin|LCD_D3_Pin|LCD_D5_Pin
                          |LCD_D4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_RST_Pin LCD_D1_Pin */
  GPIO_InitStruct.Pin = LCD_RST_Pin|LCD_D1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_RD_Pin LD2_Pin LCD_D7_Pin LCD_D0_Pin
                           LCD_D2_Pin */
  GPIO_InitStruct.Pin = LCD_RD_Pin|LD2_Pin|LCD_D7_Pin|LCD_D0_Pin
                          |LCD_D2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_RS_Pin */
  GPIO_InitStruct.Pin = LCD_RS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  HAL_GPIO_Init(LCD_RS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_CS_Pin LCD_D6_Pin LCD_D3_Pin LCD_D5_Pin
                           LCD_D4_Pin */
  GPIO_InitStruct.Pin = LCD_CS_Pin|LCD_D6_Pin|LCD_D3_Pin|LCD_D5_Pin
                          |LCD_D4_Pin;
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
