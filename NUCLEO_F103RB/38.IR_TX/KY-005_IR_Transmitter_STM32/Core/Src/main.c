/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : KY-005 IR Transmitter Example for NUCLEO-F103RB
  *
  * This example demonstrates:
  *   - 38kHz carrier generation using TIM2 PWM on PA0
  *   - NEC protocol IR transmission via KY-005 module
  *   - USART2 command interface for remote control
  *   - User button (PC13) triggered IR send
  *   - LED (PA5) feedback during transmission
  *
  * Hardware Setup:
  *   NUCLEO-F103RB board
  *   KY-005 IR Transmitter Module:
  *     - S (Signal)   -> PA0  (TIM2_CH1, CN7 pin 30)
  *     - VCC (middle)  -> 5V   (CN7 pin 18)
  *     - GND (-)       -> GND  (CN7 pin 20)
  *
  *   Optional: KY-022 IR Receiver for capture/verify
  *     - S (Signal)   -> PB7  (CN10 pin 28)
  *     - VCC (middle)  -> 5V
  *     - GND (-)       -> GND
  *
  * USART2 (ST-Link Virtual COM port):
  *   TX: PA2, RX: PA3, 115200 baud
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "ir_transmitter.h"

/* Private variables ---------------------------------------------------------*/

/* Timer handles */
TIM_HandleTypeDef htim2;    /* TIM2: 38kHz PWM for IR carrier (PA0) */
TIM_HandleTypeDef htim3;    /* TIM3: 1us tick timer for IR timing */

/* UART handle */
UART_HandleTypeDef huart2;  /* USART2: Virtual COM port */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART2_UART_Init(void);

/* Private user code ---------------------------------------------------------*/

/* Known IR codes (examples - replace with your device codes) */
#define IR_ADDR_SAMSUNG  0x0707  /* Samsung TV address (extended NEC) */
#define IR_ADDR_PANASONIC 0x0004 /* Panasonic TV address */
#define IR_ADDR_LG        0x0404 /* LG TV address */

/* Command codes (common examples) */
#define IR_CMD_POWER      0x40    /* Power toggle */
#define IR_CMD_VOL_UP     0x10    /* Volume up */
#define IR_CMD_VOL_DOWN   0x11    /* Volume down */
#define IR_CMD_MUTE       0x0D    /* Mute */
#define IR_CMD_SOURCE     0x54    /* Input source */
#define IR_CMD_CH_UP      0x20    /* Channel up */
#define IR_CMD_CH_DOWN    0x21    /* Channel down */

/* UART receive buffer */
uint8_t uart_rx_byte;

/**
  * @brief  Print string over UART
  */
static void UART_Print(const char *str)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen(str), 100);
}

/**
  * @brief  Print a hexadecimal byte
  */
static void UART_PrintHex(uint8_t val)
{
    char buf[4];
    sprintf(buf, "%02X ", val);
    HAL_UART_Transmit(&huart2, (uint8_t *)buf, 3, 100);
}

/**
  * @brief  Print welcome message and instructions
  */
static void Print_Menu(void)
{
    UART_Print("\r\n========================================\r\n");
    UART_Print("  KY-005 IR Transmitter (NUCLEO-F103RB)\r\n");
    UART_Print("========================================\r\n");
    UART_Print("Commands:\r\n");
    UART_Print("  1     : Send NEC (Samsung Power)\r\n");
    UART_Print("  2     : Send NEC (Samsung Vol+)\r\n");
    UART_Print("  3     : Send NEC (Samsung Vol-)\r\n");
    UART_Print("  4     : Send NEC (Samsung Mute)\r\n");
    UART_Print("  5     : Send NEC Repeat\r\n");
    UART_Print("  6     : Send Sony (12-bit)\r\n");
    UART_Print("  b     : User Button mode (press to send)\r\n");
    UART_Print("  h     : Print this menu\r\n");
    UART_Print("----------------------------------------\r\n");
    UART_Print("Ready. Enter command: ");
}

/**
  * @brief  Process UART command
  */
static void Process_Command(uint8_t cmd)
{
    UART_Print("\r\n");

    switch (cmd)
    {
        case '1':
            UART_Print("[TX] Samsung Power (0xE0E040BF)\r\n");
            IR_Send_NEC_Standard(0x07, 0x40);
            break;

        case '2':
            UART_Print("[TX] Samsung Vol+ (0xE0E0E01F)\r\n");
            IR_Send_NEC_Standard(0x07, 0x10);
            break;

        case '3':
            UART_Print("[TX] Samsung Vol- (0xE0E0D02F)\r\n");
            IR_Send_NEC_Standard(0x07, 0x11);
            break;

        case '4':
            UART_Print("[TX] Samsung Mute (0xE0E0F00F)\r\n");
            IR_Send_NEC_Standard(0x07, 0x0D);
            break;

        case '5':
            UART_Print("[TX] NEC Repeat\r\n");
            IR_Send_NEC_Repeat();
            break;

        case '6':
            UART_Print("[TX] Sony Power (12-bit: cmd=0x15, addr=0x01)\r\n");
            IR_Send_Sony(0x15, 7, 0x01);
            break;

        case 'b':
            UART_Print("[MODE] Press User Button (PC13) to send...\r\n");
            /* Button mode - handled in main loop */
            break;

        case 'h':
            Print_Menu();
            return;

        default:
            UART_Print("Unknown command. Press 'h' for help.\r\n");
            break;
    }

    UART_Print("\r\nReady. Enter command: ");
}

/**
  * @brief  Send a test sequence of common IR codes
  */
static void Send_Test_Sequence(void)
{
    UART_Print("\r\n[TEST] Sending IR test sequence...\r\n");

    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_GPIO_Pin, GPIO_PIN_SET);

    /* Sony TV Power */
    UART_Print("[TX] Sony Power\r\n");
    IR_Send_Sony(0x15, 7, 0x01);
    HAL_Delay(200);

    /* NEC Samsung Power */
    UART_Print("[TX] Samsung Power (NEC)\r\n");
    IR_Send_NEC_Standard(0x07, 0x40);
    HAL_Delay(200);

    /* NEC Panasonic Power */
    UART_Print("[TX] Panasonic Power (NEC)\r\n");
    IR_Send_NEC_Standard(0x04, 0x40);
    HAL_Delay(200);

    /* NEC LG Power */
    UART_Print("[TX] LG Power (NEC)\r\n");
    IR_Send_NEC_Standard(0x04, 0x10);
    HAL_Delay(200);

    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_GPIO_Pin, GPIO_PIN_RESET);
    UART_Print("[TEST] Sequence complete.\r\n");
}

/**
  * @brief  UART RX complete callback (interrupt-driven)
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        Process_Command(uart_rx_byte);
        HAL_UART_Receive_IT(&huart2, &uart_rx_byte, 1);
    }
}

/**
  * @brief  GPIO EXTI callback (user button)
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == B1_GPIO_Pin)  /* PC13, User button */
    {
        /* Debounce: wait and re-check */
        HAL_Delay(50);
        if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_GPIO_Pin) == GPIO_PIN_RESET)
        {
            /* Send Samsung Volume Up as demo */
            HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_GPIO_Pin);
            IR_Send_NEC_Standard(0x07, 0x10);
            HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_GPIO_Pin);
        }
    }
}

/**
  * @brief  Main program
  */
int main(void)
{
    /* HAL Library initialization */
    HAL_Init();

    /* System clock configuration: HSE 8MHz -> PLL -> 72MHz */
    SystemClock_Config();

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_TIM2_Init();
    MX_TIM3_Init();
    MX_USART2_UART_Init();

    /* ========== Initialize KY-005 IR Transmitter ========== */
    /*
     * TIM2: 38kHz PWM on PA0 (TIM2_CH1)
     *   Clock: 72 MHz, Prescaler: 0, Period: 1894 -> 72MHz/1895 = 38.0kHz
     *   Pulse: 947 (50% duty cycle)
     *
     * TIM3: 1us tick timer
     *   Clock: 72 MHz, Prescaler: 71, Counter period: 65535 (max)
     *   72MHz / 72 = 1MHz -> 1 tick = 1us
     */
    IR_Transmitter_Init(&htim2, TIM_CHANNEL_1, &htim3);

    /* Start UART interrupt reception */
    HAL_UART_Receive_IT(&huart2, &uart_rx_byte, 1);

    /* Print welcome message */
    Print_Menu();

    /* Blink LED to indicate startup complete */
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_GPIO_Pin, GPIO_PIN_SET);
    HAL_Delay(200);
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_GPIO_Pin, GPIO_PIN_RESET);

    /* ========== Infinite Loop ========== */
    while (1)
    {
        /* Main loop is idle - all actions triggered by interrupts */
        /* Add periodic tasks here if needed */
    }
}

/**
  * @brief  System Clock Configuration
  *         System Clock source = PLL (HSE)
  *         SYSCLK = 72 MHz
  *         HCLK   = 72 MHz
  *         APB1   = 36 MHz
  *         APB2   = 72 MHz
  *         HSE    = 8 MHz
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* HSE oscillator configuration */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;  /* 8MHz * 9 = 72MHz */
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    /* Clock tree configuration */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK  |
                                  RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1  |
                                  RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;   /* APB1: 36 MHz */
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;   /* APB2: 72 MHz */
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

/**
  * @brief  TIM2 Initialization for 38kHz PWM on PA0 (IR carrier)
  *
  * Calculation:
  *   TIM2 clock source = APB1 clock * 2 = 36 MHz * 2 = 72 MHz
  *   (since APB1 prescaler != 1, TIM clock = APB1 * 2)
  *
  *   Target PWM frequency: 38,000 Hz
  *   Prescaler (PSC) = 0  -> TIM counter clock = 72 MHz
  *   Auto-reload (ARR) = (72,000,000 / 38,000) - 1 = 1894
  *   Pulse (CCR) = 1894 / 2 = 947  (50% duty cycle)
  *
  *   Actual frequency = 72,000,000 / (0+1) / (1894+1) = 38,015 Hz (close enough)
  */
static void MX_TIM2_Init(void)
{
    TIM_OC_InitTypeDef sConfigOC = {0};

    htim2.Instance = TIM2;
    htim2.Init.Prescaler     = 0;         /* No prescaling */
    htim2.Init.CounterMode   = TIM_COUNTERMODE_UP;
    htim2.Init.Period        = 1894;      /* 72MHz / 1895 = 38.0kHz */
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_PWM_Init(&htim2);

    /* PWM channel configuration */
    sConfigOC.OCMode     = TIM_OCMODE_PWM1;
    sConfigOC.Pulse      = 947;          /* 50% duty cycle */
    sConfigOC.OCPolarity  = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode  = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1);
}

/**
  * @brief  TIM3 Initialization for 1us tick timer (IR timing)
  *
  * Calculation:
  *   TIM3 clock source = APB1 clock * 2 = 36 MHz * 2 = 72 MHz
  *   Prescaler = 72 - 1  -> TIM counter clock = 72 MHz / 72 = 1 MHz
  *   1 tick = 1 microsecond
  *   Period = 65535 (max 16-bit) -> max delay = 65535 us = ~65.5ms
  */
static void MX_TIM3_Init(void)
{
    htim3.Instance = TIM3;
    htim3.Init.Prescaler     = 71;        /* 72MHz / 72 = 1MHz -> 1us tick */
    htim3.Init.CounterMode   = TIM_COUNTERMODE_UP;
    htim3.Init.Period        = 65535;     /* Max 16-bit period */
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim3);
}

/**
  * @brief  USART2 Initialization (ST-Link Virtual COM port)
  *         TX: PA2, RX: PA3, 115200 baud
  */
static void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate     = 115200;
    huart2.Init.WordLength   = UART_WORDLENGTH_8B;
    huart2.Init.StopBits     = UART_STOPBITS_1;
    huart2.Init.Parity       = UART_PARITY_NONE;
    huart2.Init.Mode         = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart2);
}

/**
  * @brief  GPIO Initialization
  *         PA5: LD2 (LED) - Output
  *         PC13: B1 (User Button) - Input with interrupt
  *         PA0: TIM2_CH1 (KY-005 Signal) - AF push-pull
  *         PA2: USART2_TX - AF push-pull
  *         PA3: USART2_RX - Input floating
  */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIO clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* LD2 (PA5): Output */
    GPIO_InitStruct.Pin   = GPIO_PIN_5;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* B1 (PC13): Input with falling-edge interrupt */
    GPIO_InitStruct.Pin    = GPIO_PIN_13;
    GPIO_InitStruct.Mode   = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull   = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* Enable and set EXTI interrupt priority */
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/**
  * @brief  HAL MSP Initialization (timer, UART GPIO and NVIC setup)
  *         This function is called by HAL peripheral Init functions.
  */
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (htim->Instance == TIM2)
    {
        /* TIM2 clock enable */
        __HAL_RCC_TIM2_CLK_ENABLE();

        /* PA0: TIM2_CH1 alternate function */
        GPIO_InitStruct.Pin   = GPIO_PIN_0;
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* TIM2 interrupt for DMA (if used) - not needed for basic PWM */
    }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3)
    {
        /* TIM3 clock enable */
        __HAL_RCC_TIM3_CLK_ENABLE();
        /* No GPIO needed for internal timer */
        /* No interrupt needed for polling mode */
    }
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (huart->Instance == USART2)
    {
        /* USART2 clock enable */
        __HAL_RCC_USART2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /* PA2: USART2_TX */
        GPIO_InitStruct.Pin    = GPIO_PIN_2;
        GPIO_InitStruct.Mode   = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed  = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* PA3: USART2_RX */
        GPIO_InitStruct.Pin    = GPIO_PIN_3;
        GPIO_InitStruct.Mode   = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull   = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* USART2 interrupt */
        HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
    }
}

/**
  * @brief  This function handles HAL peripheral MSP de-initialization.
  */
void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        __HAL_RCC_TIM2_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0);
    }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3)
    {
        __HAL_RCC_TIM3_CLK_DISABLE();
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        __HAL_RCC_USART2_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2);
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_3);
    }
}

/* ========== Interrupt Handlers ========== */

void USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart2);
}

void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}
