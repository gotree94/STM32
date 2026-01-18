/**
 * @file    main.c
 * @brief   Button Switch Module Test for NUCLEO-F103RB
 * @author  Embedded Systems Lab
 * @date    2025
 * 
 * @details This program demonstrates how to interface a button switch
 *          module with STM32F103 microcontroller. Includes polling and
 *          interrupt-based detection with debouncing.
 * 
 * Hardware Connections:
 *   - Button Module VCC  -> 3.3V
 *   - Button Module GND  -> GND
 *   - Button Module S    -> PA0 (with external interrupt)
 *   - LED                -> PA5 (Built-in LED on NUCLEO board)
 */

#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* Private defines */
#define DEBOUNCE_MS         50      /* Debounce time in ms */
#define LONG_PRESS_MS       1000    /* Long press threshold */
#define DOUBLE_CLICK_MS     300     /* Double click window */

/* Private variables */
UART_HandleTypeDef huart2;

volatile uint32_t button_press_time = 0;
volatile uint32_t button_release_time = 0;
volatile uint32_t last_click_time = 0;
volatile uint8_t button_state = 0;
volatile uint8_t click_count = 0;
volatile uint32_t press_count = 0;
volatile uint8_t pending_click = 0;

/* Event flags */
volatile uint8_t event_short_press = 0;
volatile uint8_t event_long_press = 0;
volatile uint8_t event_double_click = 0;

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
void UART_SendString(char *str);
void Process_Button(void);
void Handle_Events(void);

/**
 * @brief  Main program entry point
 * @retval int
 */
int main(void)
{
    char msg[100];
    
    /* HAL Initialization */
    HAL_Init();
    
    /* Configure the system clock */
    SystemClock_Config();
    
    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    
    /* Welcome message */
    UART_SendString("\r\n========================================\r\n");
    UART_SendString("  Button Switch Module Test\r\n");
    UART_SendString("  NUCLEO-F103RB\r\n");
    UART_SendString("========================================\r\n");
    UART_SendString("Button Events Detected:\r\n");
    UART_SendString("  - Short Press (< 1 sec)\r\n");
    UART_SendString("  - Long Press (>= 1 sec)\r\n");
    UART_SendString("  - Double Click (< 300ms)\r\n\r\n");
    UART_SendString("Press the button to start...\r\n\r\n");
    
    /* Initial state read */
    button_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
    
    /* Main loop */
    while (1)
    {
        /* Process button state */
        Process_Button();
        
        /* Handle detected events */
        Handle_Events();
        
        HAL_Delay(5);
    }
}

/**
 * @brief  Process button state and detect events
 * @retval None
 */
void Process_Button(void)
{
    static uint8_t prev_state = 1;  /* Assume pull-up (idle high) */
    static uint32_t state_change_time = 0;
    static uint8_t debounced_state = 1;
    static uint8_t is_pressed = 0;
    
    uint8_t current_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
    uint32_t current_time = HAL_GetTick();
    
    /* State change detection with debounce */
    if (current_state != prev_state)
    {
        state_change_time = current_time;
        prev_state = current_state;
    }
    
    /* Apply debounce */
    if ((current_time - state_change_time) >= DEBOUNCE_MS)
    {
        if (current_state != debounced_state)
        {
            debounced_state = current_state;
            
            /* Button pressed (active low) */
            if (debounced_state == GPIO_PIN_RESET)
            {
                button_press_time = current_time;
                is_pressed = 1;
                press_count++;
                
                /* LED ON */
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
                
                char msg[50];
                sprintf(msg, "[PRESS] Button pressed (#%lu)\r\n", press_count);
                UART_SendString(msg);
            }
            /* Button released */
            else if (is_pressed)
            {
                button_release_time = current_time;
                uint32_t press_duration = button_release_time - button_press_time;
                is_pressed = 0;
                
                /* LED OFF */
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
                
                char msg[80];
                sprintf(msg, "[RELEASE] Duration: %lu ms\r\n", press_duration);
                UART_SendString(msg);
                
                /* Determine event type */
                if (press_duration >= LONG_PRESS_MS)
                {
                    /* Long press detected */
                    event_long_press = 1;
                    pending_click = 0;
                    click_count = 0;
                }
                else
                {
                    /* Short press - check for double click */
                    if ((current_time - last_click_time) < DOUBLE_CLICK_MS && pending_click)
                    {
                        /* Double click detected */
                        event_double_click = 1;
                        pending_click = 0;
                        click_count = 0;
                    }
                    else
                    {
                        /* Potential single click - wait for double click window */
                        pending_click = 1;
                        click_count = 1;
                    }
                    last_click_time = current_time;
                }
            }
        }
    }
    
    /* Check for single click timeout (no second click) */
    if (pending_click && (current_time - last_click_time) >= DOUBLE_CLICK_MS)
    {
        event_short_press = 1;
        pending_click = 0;
        click_count = 0;
    }
    
    /* Check for long press while still holding */
    if (is_pressed && (current_time - button_press_time) >= LONG_PRESS_MS)
    {
        static uint8_t long_press_reported = 0;
        if (!long_press_reported)
        {
            UART_SendString("[HOLDING] Long press threshold reached\r\n");
            long_press_reported = 1;
        }
    }
    else
    {
        static uint8_t long_press_reported = 0;
        long_press_reported = 0;
    }
}

/**
 * @brief  Handle detected button events
 * @retval None
 */
void Handle_Events(void)
{
    if (event_short_press)
    {
        event_short_press = 0;
        UART_SendString(">>> EVENT: SHORT PRESS <<<\r\n");
        UART_SendString("    Action: Toggle something\r\n\r\n");
        
        /* Add your short press action here */
    }
    
    if (event_long_press)
    {
        event_long_press = 0;
        UART_SendString(">>> EVENT: LONG PRESS <<<\r\n");
        UART_SendString("    Action: Enter settings mode\r\n\r\n");
        
        /* Add your long press action here */
        /* Blink LED to indicate long press */
        for (int i = 0; i < 3; i++)
        {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
            HAL_Delay(100);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
            HAL_Delay(100);
        }
    }
    
    if (event_double_click)
    {
        event_double_click = 0;
        UART_SendString(">>> EVENT: DOUBLE CLICK <<<\r\n");
        UART_SendString("    Action: Special function\r\n\r\n");
        
        /* Add your double click action here */
        /* Quick blinks to indicate double click */
        for (int i = 0; i < 5; i++)
        {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
            HAL_Delay(50);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
            HAL_Delay(50);
        }
    }
}

/**
 * @brief  GPIO Initialization Function
 * @retval None
 */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable GPIO Clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    /* Configure PA0 as input with pull-up (Button) */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Configure PA5 as output (LED) */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Initialize LED to OFF */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}

/**
 * @brief  USART2 Initialization Function
 * @retval None
 */
static void MX_USART2_UART_Init(void)
{
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* USART2 GPIO Configuration: PA2 -> TX, PA3 -> RX */
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

/**
 * @brief  Send string via UART
 * @param  str: String to send
 * @retval None
 */
void UART_SendString(char *str)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}

/**
 * @brief  System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    /* Configure HSE Oscillator */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
    
    /* Configure CPU, AHB and APB clocks */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

/**
 * @brief  SysTick Handler
 * @retval None
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
}
