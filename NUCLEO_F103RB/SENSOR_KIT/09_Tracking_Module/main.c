/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Line Tracking Module Test for STM32F103 NUCLEO
 * @author         : 
 * @date           : 2025
 ******************************************************************************
 * @description
 * 라인 트래킹(추적) 모듈 테스트 (TCRT5000 기반)
 * - 적외선 센서를 이용한 흑/백 라인 감지
 * - 디지털 출력으로 라인 위치 확인
 * - 3채널/5채널 센서 어레이 지원
 * 
 * @pinout (3채널 기준)
 * - PA0  : Left Sensor (Digital)
 * - PA1  : Center Sensor (Digital)
 * - PA4  : Right Sensor (Digital)
 * - PA5  : LED Indicator (Output)
 * - PA2  : USART2 TX (Debug)
 * - PA3  : USART2 RX (Debug)
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* Private defines */
#define SENSOR_LEFT_PIN     GPIO_PIN_0
#define SENSOR_CENTER_PIN   GPIO_PIN_1
#define SENSOR_RIGHT_PIN    GPIO_PIN_4
#define SENSOR_PORT         GPIOA
#define LED_PIN             GPIO_PIN_5
#define LED_PORT            GPIOA

/* Sensor state macros */
#define LINE_DETECTED       0   // Active Low (흑색 라인 감지)
#define LINE_NOT_DETECTED   1   // 백색 바닥

/* Line position definitions */
typedef enum {
    POS_UNKNOWN = 0,
    POS_LEFT,
    POS_CENTER,
    POS_RIGHT,
    POS_LOST,
    POS_ALL_BLACK,
    POS_SLIGHT_LEFT,
    POS_SLIGHT_RIGHT
} LinePosition;

/* Private variables */
UART_HandleTypeDef huart2;

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
uint8_t Read_Sensor_Left(void);
uint8_t Read_Sensor_Center(void);
uint8_t Read_Sensor_Right(void);
LinePosition Get_Line_Position(void);
const char* Position_To_String(LinePosition pos);
void Print_Sensor_Status(uint8_t left, uint8_t center, uint8_t right);
void Print_Direction_Arrow(LinePosition pos);

/* Printf redirect */
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/**
 * @brief  Main program
 */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    printf("\r\n============================================\r\n");
    printf("  Line Tracking Module Test\r\n");
    printf("  STM32F103 NUCLEO\r\n");
    printf("============================================\r\n");
    printf("PA0: Left Sensor\r\n");
    printf("PA1: Center Sensor\r\n");
    printf("PA4: Right Sensor\r\n");
    printf("(0=Line Detected, 1=No Line)\r\n\r\n");

    uint8_t left, center, right;
    LinePosition position, prev_position = POS_UNKNOWN;
    uint32_t last_print_time = 0;

    while (1)
    {
        /* 센서 읽기 */
        left = Read_Sensor_Left();
        center = Read_Sensor_Center();
        right = Read_Sensor_Right();

        /* 라인 위치 계산 */
        position = Get_Line_Position();

        /* 상태 표시 LED */
        if (position == POS_CENTER)
        {
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
        }
        else
        {
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
        }

        /* 100ms마다 또는 위치 변경 시 출력 */
        if (HAL_GetTick() - last_print_time >= 100 || position != prev_position)
        {
            last_print_time = HAL_GetTick();
            
            printf("Sensors: ");
            Print_Sensor_Status(left, center, right);
            printf(" | Position: %-12s | ", Position_To_String(position));
            Print_Direction_Arrow(position);
            printf("\r\n");
            
            prev_position = position;
        }

        HAL_Delay(10);
    }
}

/**
 * @brief  Read Left Sensor
 */
uint8_t Read_Sensor_Left(void)
{
    return HAL_GPIO_ReadPin(SENSOR_PORT, SENSOR_LEFT_PIN);
}

/**
 * @brief  Read Center Sensor
 */
uint8_t Read_Sensor_Center(void)
{
    return HAL_GPIO_ReadPin(SENSOR_PORT, SENSOR_CENTER_PIN);
}

/**
 * @brief  Read Right Sensor
 */
uint8_t Read_Sensor_Right(void)
{
    return HAL_GPIO_ReadPin(SENSOR_PORT, SENSOR_RIGHT_PIN);
}

/**
 * @brief  Calculate line position
 */
LinePosition Get_Line_Position(void)
{
    uint8_t left = Read_Sensor_Left();
    uint8_t center = Read_Sensor_Center();
    uint8_t right = Read_Sensor_Right();
    
    /* 
     * 센서 조합에 따른 위치 판단
     * 0 = 라인 감지 (검은색)
     * 1 = 라인 없음 (흰색)
     */
    
    // 모든 센서가 라인 감지 (넓은 라인 또는 교차점)
    if (left == LINE_DETECTED && center == LINE_DETECTED && right == LINE_DETECTED)
    {
        return POS_ALL_BLACK;
    }
    
    // 중앙만 감지 - 정확히 중앙
    if (left == LINE_NOT_DETECTED && center == LINE_DETECTED && right == LINE_NOT_DETECTED)
    {
        return POS_CENTER;
    }
    
    // 왼쪽만 감지 - 크게 왼쪽으로 치우침
    if (left == LINE_DETECTED && center == LINE_NOT_DETECTED && right == LINE_NOT_DETECTED)
    {
        return POS_LEFT;
    }
    
    // 오른쪽만 감지 - 크게 오른쪽으로 치우침
    if (left == LINE_NOT_DETECTED && center == LINE_NOT_DETECTED && right == LINE_DETECTED)
    {
        return POS_RIGHT;
    }
    
    // 왼쪽 + 중앙 감지 - 약간 왼쪽
    if (left == LINE_DETECTED && center == LINE_DETECTED && right == LINE_NOT_DETECTED)
    {
        return POS_SLIGHT_LEFT;
    }
    
    // 중앙 + 오른쪽 감지 - 약간 오른쪽
    if (left == LINE_NOT_DETECTED && center == LINE_DETECTED && right == LINE_DETECTED)
    {
        return POS_SLIGHT_RIGHT;
    }
    
    // 왼쪽 + 오른쪽 감지 (중앙 없음) - 이상한 상황
    if (left == LINE_DETECTED && center == LINE_NOT_DETECTED && right == LINE_DETECTED)
    {
        return POS_UNKNOWN;
    }
    
    // 모든 센서가 라인 없음 - 라인 이탈
    if (left == LINE_NOT_DETECTED && center == LINE_NOT_DETECTED && right == LINE_NOT_DETECTED)
    {
        return POS_LOST;
    }
    
    return POS_UNKNOWN;
}

/**
 * @brief  Convert position to string
 */
const char* Position_To_String(LinePosition pos)
{
    switch (pos)
    {
        case POS_LEFT:         return "LEFT";
        case POS_CENTER:       return "CENTER";
        case POS_RIGHT:        return "RIGHT";
        case POS_SLIGHT_LEFT:  return "SLIGHT LEFT";
        case POS_SLIGHT_RIGHT: return "SLIGHT RIGHT";
        case POS_LOST:         return "LINE LOST!";
        case POS_ALL_BLACK:    return "ALL BLACK";
        default:               return "UNKNOWN";
    }
}

/**
 * @brief  Print sensor status with visual representation
 */
void Print_Sensor_Status(uint8_t left, uint8_t center, uint8_t right)
{
    printf("[%c|%c|%c]", 
           left == LINE_DETECTED ? '#' : '_',
           center == LINE_DETECTED ? '#' : '_',
           right == LINE_DETECTED ? '#' : '_');
}

/**
 * @brief  Print direction arrow
 */
void Print_Direction_Arrow(LinePosition pos)
{
    switch (pos)
    {
        case POS_LEFT:
            printf("<<<---");
            break;
        case POS_SLIGHT_LEFT:
            printf(" <<-- ");
            break;
        case POS_CENTER:
            printf("--||--");
            break;
        case POS_SLIGHT_RIGHT:
            printf(" -->> ");
            break;
        case POS_RIGHT:
            printf("--->>>");
            break;
        case POS_LOST:
            printf("??????");
            break;
        case POS_ALL_BLACK:
            printf("######");
            break;
        default:
            printf("  ??  ");
            break;
    }
}

/**
 * @brief System Clock Configuration (72MHz)
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

/**
 * @brief GPIO Initialization
 */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* LED Pin - Output */
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);

    /* Sensor Pins - Input with Pull-up */
    GPIO_InitStruct.Pin = SENSOR_LEFT_PIN | SENSOR_CENTER_PIN | SENSOR_RIGHT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(SENSOR_PORT, &GPIO_InitStruct);
}

/**
 * @brief USART2 Initialization (115200 baud)
 */
static void MX_USART2_UART_Init(void)
{
    __HAL_RCC_USART2_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
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
    HAL_UART_Init(&huart2);
}
