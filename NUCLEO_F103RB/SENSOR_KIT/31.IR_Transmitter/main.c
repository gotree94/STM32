/**
 ******************************************************************************
 * @file    main.c
 * @brief   IR Transmitter Module Test for STM32F103 NUCLEO
 * @author  Embedded Systems Lab
 * @version V1.0.0
 * @date    2025-01-18
 ******************************************************************************
 * @details
 * 이 프로젝트는 IR 송신 모듈을 테스트합니다.
 * 38kHz 캐리어 주파수로 변조된 IR 신호를 송신합니다.
 * 
 * Hardware Setup:
 * - IR LED: PA8 (TIM1_CH1 PWM output)
 * - Button: PC13 (User Button on NUCLEO board)
 * - UART2: PA2(TX), PA3(RX) - USB Virtual COM Port
 * 
 * 버튼을 누르면 IR 신호를 송신합니다.
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* Private defines -----------------------------------------------------------*/
#define IR_LED_Pin          GPIO_PIN_8
#define IR_LED_GPIO_Port    GPIOA
#define USER_BTN_Pin        GPIO_PIN_13
#define USER_BTN_GPIO_Port  GPIOC

/* IR Protocol Parameters (NEC Protocol) */
#define IR_CARRIER_FREQ     38000   // 38kHz carrier frequency
#define IR_LEAD_PULSE       9000    // 9ms leader pulse
#define IR_LEAD_SPACE       4500    // 4.5ms leader space
#define IR_BIT_PULSE        560     // 560us bit pulse
#define IR_ONE_SPACE        1690    // 1.69ms space for '1'
#define IR_ZERO_SPACE       560     // 560us space for '0'
#define IR_STOP_PULSE       560     // 560us stop bit

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;    // For PWM (38kHz carrier)
TIM_HandleTypeDef htim2;    // For timing delays
UART_HandleTypeDef huart2;

/* Test data to transmit */
uint8_t ir_address = 0x00;      // Device address
uint8_t ir_command = 0x12;      // Command code

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_UART_Init(void);
void IR_CarrierOn(void);
void IR_CarrierOff(void);
void IR_DelayUs(uint32_t us);
void IR_SendPulse(uint32_t pulse_us, uint32_t space_us);
void IR_SendByte(uint8_t data);
void IR_SendNEC(uint8_t address, uint8_t command);

/* Printf redirect to UART ---------------------------------------------------*/
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/**
 * @brief  Main function
 */
int main(void) {
    /* MCU Configuration */
    HAL_Init();
    SystemClock_Config();
    
    /* Initialize peripherals */
    MX_GPIO_Init();
    MX_TIM1_Init();
    MX_TIM2_Init();
    MX_USART2_UART_Init();
    
    printf("\r\n========================================\r\n");
    printf("  IR Transmitter Test - STM32F103\r\n");
    printf("========================================\r\n");
    printf("Press USER button to send IR signal\r\n");
    printf("Address: 0x%02X, Command: 0x%02X\r\n", ir_address, ir_command);
    printf("Carrier Frequency: %d Hz\r\n", IR_CARRIER_FREQ);
    printf("----------------------------------------\r\n\n");
    
    uint8_t last_btn_state = 1;  // Button is active low
    uint32_t tx_count = 0;
    
    while (1) {
        uint8_t btn_state = HAL_GPIO_ReadPin(USER_BTN_GPIO_Port, USER_BTN_Pin);
        
        /* Detect button press (falling edge) */
        if (last_btn_state == 1 && btn_state == 0) {
            tx_count++;
            printf("[%lu] Transmitting IR signal...\r\n", tx_count);
            
            /* Send NEC protocol IR signal */
            IR_SendNEC(ir_address, ir_command);
            
            printf("       Address: 0x%02X, Command: 0x%02X\r\n", ir_address, ir_command);
            printf("       Transmission complete!\r\n\n");
            
            /* Debounce delay */
            HAL_Delay(200);
        }
        
        last_btn_state = btn_state;
        HAL_Delay(10);
    }
}

/**
 * @brief  Turn on IR carrier (38kHz PWM)
 */
void IR_CarrierOn(void) {
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}

/**
 * @brief  Turn off IR carrier
 */
void IR_CarrierOff(void) {
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
}

/**
 * @brief  Microsecond delay using TIM2
 * @param  us: delay time in microseconds
 */
void IR_DelayUs(uint32_t us) {
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    HAL_TIM_Base_Start(&htim2);
    while (__HAL_TIM_GET_COUNTER(&htim2) < us);
    HAL_TIM_Base_Stop(&htim2);
}

/**
 * @brief  Send IR pulse with specified timing
 * @param  pulse_us: carrier on time in microseconds
 * @param  space_us: carrier off time in microseconds
 */
void IR_SendPulse(uint32_t pulse_us, uint32_t space_us) {
    IR_CarrierOn();
    IR_DelayUs(pulse_us);
    IR_CarrierOff();
    IR_DelayUs(space_us);
}

/**
 * @brief  Send one byte LSB first (NEC protocol)
 * @param  data: byte to send
 */
void IR_SendByte(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        if (data & (1 << i)) {
            /* Send '1': 560us pulse + 1690us space */
            IR_SendPulse(IR_BIT_PULSE, IR_ONE_SPACE);
        } else {
            /* Send '0': 560us pulse + 560us space */
            IR_SendPulse(IR_BIT_PULSE, IR_ZERO_SPACE);
        }
    }
}

/**
 * @brief  Send complete NEC protocol frame
 * @param  address: device address (8-bit)
 * @param  command: command code (8-bit)
 */
void IR_SendNEC(uint8_t address, uint8_t command) {
    /* Disable interrupts for precise timing */
    __disable_irq();
    
    /* Leader code: 9ms pulse + 4.5ms space */
    IR_SendPulse(IR_LEAD_PULSE, IR_LEAD_SPACE);
    
    /* Address byte */
    IR_SendByte(address);
    
    /* Address inverse byte */
    IR_SendByte(~address);
    
    /* Command byte */
    IR_SendByte(command);
    
    /* Command inverse byte */
    IR_SendByte(~command);
    
    /* Stop bit: 560us pulse */
    IR_CarrierOn();
    IR_DelayUs(IR_STOP_PULSE);
    IR_CarrierOff();
    
    __enable_irq();
}

/**
 * @brief System Clock Configuration
 *        72MHz from HSE (8MHz) with PLL
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

/**
 * @brief GPIO Initialization
 */
static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* User Button PC13 - Input with pull-up */
    GPIO_InitStruct.Pin = USER_BTN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(USER_BTN_GPIO_Port, &GPIO_InitStruct);
}

/**
 * @brief TIM1 Initialization - 38kHz PWM for IR carrier
 */
static void MX_TIM1_Init(void) {
    __HAL_RCC_TIM1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = IR_LED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(IR_LED_GPIO_Port, &GPIO_InitStruct);

    TIM_OC_InitTypeDef sConfigOC = {0};
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};

    htim1.Instance = TIM1;
    /* 72MHz / 1 / 1895 = ~38kHz */
    htim1.Init.Prescaler = 0;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 1894;   // (72000000 / 38000) - 1
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim1);

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig);
    HAL_TIM_PWM_Init(&htim1);

    /* 50% duty cycle for optimal IR transmission */
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 947;      // 50% of 1895
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
}

/**
 * @brief TIM2 Initialization - Microsecond timer
 */
static void MX_TIM2_Init(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 71;          // 72MHz / 72 = 1MHz (1us tick)
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 0xFFFF;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim2);
}

/**
 * @brief USART2 Initialization - 115200 baud
 */
static void MX_USART2_UART_Init(void) {
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* PA2 - TX */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* PA3 - RX */
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
 * @brief  Error Handler
 */
void Error_Handler(void) {
    __disable_irq();
    while (1) {}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {
    printf("Assert failed: %s line %lu\r\n", file, line);
}
#endif
