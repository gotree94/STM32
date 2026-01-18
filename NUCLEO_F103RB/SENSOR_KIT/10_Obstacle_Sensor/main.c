/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Obstacle Detection Sensor Module Test for STM32F103
 * @author         : 
 * @date           : 2025
 ******************************************************************************
 * @description
 * 적외선 장애물 감지센서 모듈 테스트
 * - IR 송수신 방식 장애물 감지
 * - 가변저항으로 감지 거리 조절 (2cm ~ 30cm)
 * - 디지털 출력 (Active Low)
 * 
 * @pinout
 * - PA0  : Obstacle Sensor 1 (Digital)
 * - PA1  : Obstacle Sensor 2 (Digital, Optional)
 * - PA5  : LED Indicator (Output)
 * - PA2  : USART2 TX (Debug)
 * - PA3  : USART2 RX (Debug)
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* Private defines */
#define OBSTACLE_1_PIN      GPIO_PIN_0
#define OBSTACLE_2_PIN      GPIO_PIN_1
#define OBSTACLE_PORT       GPIOA
#define LED_PIN             GPIO_PIN_5
#define LED_PORT            GPIOA

#define OBSTACLE_DETECTED   0   // Active Low
#define OBSTACLE_CLEAR      1

/* Detection states */
typedef enum {
    STATE_CLEAR = 0,
    STATE_LEFT_BLOCKED,
    STATE_RIGHT_BLOCKED,
    STATE_BOTH_BLOCKED
} ObstacleState;

/* Private variables */
UART_HandleTypeDef huart2;
volatile uint32_t obstacle_count = 0;
volatile uint32_t last_detection_time = 0;

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
uint8_t Read_Obstacle_Sensor1(void);
uint8_t Read_Obstacle_Sensor2(void);
ObstacleState Get_Obstacle_State(void);
const char* State_To_String(ObstacleState state);
void Print_Status_Bar(uint8_t sensor1, uint8_t sensor2);
void Process_Obstacle_Event(ObstacleState state);

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
    printf("  Obstacle Detection Sensor Test\r\n");
    printf("  STM32F103 NUCLEO\r\n");
    printf("============================================\r\n");
    printf("PA0: Obstacle Sensor 1 (Left/Front)\r\n");
    printf("PA1: Obstacle Sensor 2 (Right, Optional)\r\n");
    printf("Detection Range: 2cm ~ 30cm (adjustable)\r\n");
    printf("(0=Obstacle Detected, 1=Clear)\r\n\r\n");

    uint8_t sensor1, sensor2;
    ObstacleState state, prev_state = STATE_CLEAR;
    uint32_t last_print_time = 0;
    uint32_t detection_duration = 0;

    while (1)
    {
        /* 센서 읽기 */
        sensor1 = Read_Obstacle_Sensor1();
        sensor2 = Read_Obstacle_Sensor2();

        /* 장애물 상태 계산 */
        state = Get_Obstacle_State();

        /* LED 표시 */
        if (state != STATE_CLEAR)
        {
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
            
            if (prev_state == STATE_CLEAR)
            {
                /* 새로운 장애물 감지 */
                obstacle_count++;
                last_detection_time = HAL_GetTick();
            }
            detection_duration = HAL_GetTick() - last_detection_time;
        }
        else
        {
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
            detection_duration = 0;
        }

        /* 100ms마다 또는 상태 변경 시 출력 */
        if (HAL_GetTick() - last_print_time >= 100 || state != prev_state)
        {
            last_print_time = HAL_GetTick();
            
            printf("Sensors: ");
            Print_Status_Bar(sensor1, sensor2);
            printf(" | State: %-14s", State_To_String(state));
            
            if (state != STATE_CLEAR)
            {
                printf(" | Duration: %4lums", detection_duration);
            }
            printf("\r\n");
            
            /* 상태 변경 이벤트 처리 */
            if (state != prev_state)
            {
                Process_Obstacle_Event(state);
            }
            
            prev_state = state;
        }

        HAL_Delay(10);
    }
}

/**
 * @brief  Read Obstacle Sensor 1
 */
uint8_t Read_Obstacle_Sensor1(void)
{
    return HAL_GPIO_ReadPin(OBSTACLE_PORT, OBSTACLE_1_PIN);
}

/**
 * @brief  Read Obstacle Sensor 2
 */
uint8_t Read_Obstacle_Sensor2(void)
{
    return HAL_GPIO_ReadPin(OBSTACLE_PORT, OBSTACLE_2_PIN);
}

/**
 * @brief  Get combined obstacle state
 */
ObstacleState Get_Obstacle_State(void)
{
    uint8_t s1 = Read_Obstacle_Sensor1();
    uint8_t s2 = Read_Obstacle_Sensor2();
    
    if (s1 == OBSTACLE_DETECTED && s2 == OBSTACLE_DETECTED)
    {
        return STATE_BOTH_BLOCKED;
    }
    else if (s1 == OBSTACLE_DETECTED)
    {
        return STATE_LEFT_BLOCKED;
    }
    else if (s2 == OBSTACLE_DETECTED)
    {
        return STATE_RIGHT_BLOCKED;
    }
    else
    {
        return STATE_CLEAR;
    }
}

/**
 * @brief  Convert state to string
 */
const char* State_To_String(ObstacleState state)
{
    switch (state)
    {
        case STATE_CLEAR:         return "CLEAR";
        case STATE_LEFT_BLOCKED:  return "LEFT BLOCKED";
        case STATE_RIGHT_BLOCKED: return "RIGHT BLOCKED";
        case STATE_BOTH_BLOCKED:  return "BOTH BLOCKED";
        default:                  return "UNKNOWN";
    }
}

/**
 * @brief  Print visual status bar
 */
void Print_Status_Bar(uint8_t sensor1, uint8_t sensor2)
{
    printf("[S1:%c|S2:%c]",
           sensor1 == OBSTACLE_DETECTED ? 'X' : 'O',
           sensor2 == OBSTACLE_DETECTED ? 'X' : 'O');
}

/**
 * @brief  Process obstacle event (state change)
 */
void Process_Obstacle_Event(ObstacleState state)
{
    printf(">>> EVENT: ");
    
    switch (state)
    {
        case STATE_CLEAR:
            printf("Path cleared! Total detections: %lu\r\n", obstacle_count);
            break;
            
        case STATE_LEFT_BLOCKED:
            printf("Obstacle on LEFT! Suggest turn RIGHT\r\n");
            break;
            
        case STATE_RIGHT_BLOCKED:
            printf("Obstacle on RIGHT! Suggest turn LEFT\r\n");
            break;
            
        case STATE_BOTH_BLOCKED:
            printf("BLOCKED! Suggest STOP or REVERSE\r\n");
            break;
            
        default:
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
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);

    /* Obstacle Sensor Pins - Input with Pull-up */
    GPIO_InitStruct.Pin = OBSTACLE_1_PIN | OBSTACLE_2_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(OBSTACLE_PORT, &GPIO_InitStruct);
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
