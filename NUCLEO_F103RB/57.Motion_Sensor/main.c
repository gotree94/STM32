/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : MH-SR602 PIR Motion Sensor Test for STM32F103 NUCLEO Board
  * @author         : Embedded Systems Lab
  * @date           : 2026
  ******************************************************************************
  * @attention
  *
  * MH-SR602 PIR Motion Sensor Module Interface:
  * - OUT (Digital Output) -> PA0 (Digital Input, Active HIGH when motion detected)
  * - VCC (+) -> 5V
  * - GND (-) -> GND
  *
  * The MH-SR602 PIR sensor detects human/motion movement.
  * - Output HIGH (3.3V) when motion is detected
  * - Output LOW (0V) when no motion
  * - Default output duration: ~2.5 seconds (adjustable via onboard resistor)
  * - Detection range: up to 5 meters (0-3.5m recommended)
  * - Built-in 3.3V LDO regulator, wide input voltage range (3.3V-15V)
  *
  * UART2 is used for debug output (connected to ST-Link Virtual COM Port)
  * - TX: PA2, RX: PA3, Baudrate: 115200
  *
  ******************************************************************************
  */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* Private variables */
UART_HandleTypeDef huart2;

/* PIR motion sensor state tracking */
typedef struct {
    uint8_t current_state;          /* Current sensor state (1=motion, 0=idle) */
    uint8_t previous_state;         /* Previous sensor state */
    uint32_t motion_count;          /* Total motion events detected */
    uint32_t last_motion_time;      /* Timestamp of last motion event */
    uint32_t motion_start_time;     /* When current motion started */
    uint32_t motion_duration;       /* Current motion duration in ms */
    uint32_t total_motion_time;     /* Accumulated motion time in ms */
    uint32_t total_idle_time;       /* Accumulated idle time in ms */
    uint32_t state_start_time;      /* When current state started */
    uint32_t cooldown_until;        /* Cooldown timer to avoid retrigger */
} PIRState_t;

PIRState_t pir = {0};

/* Configuration */
#define DEBOUNCE_TIME_MS        30      /* Debounce time for stable reading */
#define COOLDOWN_TIME_MS        2500    /* Match sensor's default output pulse (2.5s) */
#define STATS_INTERVAL_MS       5000    /* Statistics print interval */

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

/* Printf redirect to UART */
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/**
  * @brief  Read PIR sensor raw value
  * @retval 1 if motion detected (HIGH), 0 if idle (LOW)
  */
uint8_t PIR_ReadRaw(void) {
    return (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET) ? 1 : 0;
}

/**
  * @brief  Read PIR sensor state with software debouncing
  * @retval 1 if motion detected, 0 if idle
  */
uint8_t PIR_Read(void) {
    static uint8_t stable_state = 0;
    static uint32_t last_change_time = 0;

    uint8_t raw_state = PIR_ReadRaw();
    uint32_t current_time = HAL_GetTick();

    if (raw_state != stable_state) {
        if (current_time - last_change_time >= DEBOUNCE_TIME_MS) {
            stable_state = raw_state;
            last_change_time = current_time;
        }
    } else {
        last_change_time = current_time;
    }

    return stable_state;
}

/**
  * @brief  Initialize PIR state tracking
  * @retval None
  */
void PIR_Init(void) {
    pir.current_state = PIR_Read();
    pir.previous_state = pir.current_state;
    pir.state_start_time = HAL_GetTick();
    pir.cooldown_until = 0;

    printf("PIR Sensor initialized. Initial state: %s\r\n",
           pir.current_state ? "MOTION DETECTED" : "IDLE");
}

/**
  * @brief  Update PIR state machine and detect motion events
  * @retval None
  */
void PIR_Update(void) {
    pir.previous_state = pir.current_state;
    pir.current_state = PIR_Read();

    uint32_t current_time = HAL_GetTick();

    /* Rising edge: IDLE -> MOTION (motion detected) */
    if (pir.current_state && !pir.previous_state) {
        /* Check if cooldown has expired */
        if (current_time >= pir.cooldown_until) {
            pir.motion_count++;
            pir.last_motion_time = current_time;
            pir.motion_start_time = current_time;

            uint32_t idle_duration = current_time - pir.state_start_time;
            pir.total_idle_time += idle_duration;

            printf("\r\n>>> [MOTION DETECTED] #%lu <<<\r\n", pir.motion_count);
            printf("    Previous idle duration: %lu ms\r\n", idle_duration);

            pir.state_start_time = current_time;
        } else {
            /* Within cooldown period - ignore */
            pir.current_state = 0;
        }
    }

    /* Falling edge: MOTION -> IDLE (motion ended) */
    if (!pir.current_state && pir.previous_state) {
        uint32_t motion_dur = current_time - pir.state_start_time;
        pir.total_motion_time += motion_dur;

        printf(">>> [MOTION ENDED] Duration: %lu ms <<<\r\n", motion_dur);

        /* Set cooldown to prevent immediate retrigger */
        pir.cooldown_until = current_time + COOLDOWN_TIME_MS;
        pir.state_start_time = current_time;
    }

    /* Update current motion duration if in motion state */
    if (pir.current_state) {
        pir.motion_duration = current_time - pir.state_start_time;
    }
}

/**
  * @brief  Check if motion just started (rising edge)
  * @retval 1 if motion just started, 0 otherwise
  */
uint8_t PIR_JustActivated(void) {
    return (pir.current_state && !pir.previous_state);
}

/**
  * @brief  Check if motion just ended (falling edge)
  * @retval 1 if motion just ended, 0 otherwise
  */
uint8_t PIR_JustDeactivated(void) {
    return (!pir.current_state && pir.previous_state);
}

/**
  * @brief  Get current PIR state
  * @retval 1 if motion detected, 0 if idle
  */
uint8_t PIR_IsMotionDetected(void) {
    return pir.current_state;
}

/**
  * @brief  Get total motion event count
  * @retval Motion count
  */
uint32_t PIR_GetMotionCount(void) {
    return pir.motion_count;
}

/**
  * @brief  Format time as hours:minutes:seconds
  * @param  ms: Time in milliseconds
  * @param  buffer: Output buffer (must be at least 16 bytes)
  * @retval None
  */
void FormatTime(uint32_t ms, char *buffer) {
    uint32_t seconds = ms / 1000;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;
    sprintf(buffer, "%02lu:%02lu:%02lu", hours, minutes % 60, seconds % 60);
}

/**
  * @brief  Print PIR sensor status line
  * @retval None
  */
void Print_Status(void) {
    uint32_t current_time = HAL_GetTick();
    char motion_time_str[16], idle_time_str[16];
    uint32_t current_duration = current_time - pir.state_start_time;

    FormatTime(pir.total_motion_time + (pir.current_state ? current_duration : 0), motion_time_str);
    FormatTime(pir.total_idle_time + (!pir.current_state ? current_duration : 0), idle_time_str);

    /* Visual motion indicator */
    if (PIR_IsMotionDetected()) {
        int bars = (pir.motion_duration / 250);
        if (bars > 20) bars = 20;
        printf("\r[");
        for (int i = 0; i < 20; i++) {
            printf("%s", (i < bars) ? "█" : "░");
        }
        printf("] MOTION! (%lu ms)  ", pir.motion_duration);
    } else {
        printf("\r[░░░░░░░░░░░░░░░░░░░░] IDLE              ");
    }

    printf("| Events: %3lu | Motion: %s | Idle: %s     ",
           pir.motion_count, motion_time_str, idle_time_str);
    fflush(stdout);
}

/**
  * @brief  Print statistics report
  * @retval None
  */
void Print_Statistics(void) {
    uint32_t total_time = pir.total_motion_time + pir.total_idle_time;
    uint32_t motion_pct = (total_time > 0) ?
                          (pir.total_motion_time * 100 / total_time) : 0;

    printf("\r\n\r\n========== PIR Sensor Statistics ==========\r\n");
    printf("  Total motion events : %lu\r\n", pir.motion_count);
    printf("  Total motion time   : %lu ms (%lu%%)\r\n",
           pir.total_motion_time, motion_pct);
    printf("  Total idle time     : %lu ms (%lu%%)\r\n",
           pir.total_idle_time, 100 - motion_pct);
    printf("  Total elapsed time  : %lu ms\r\n", total_time);
    printf("  Avg event duration  : %lu ms\r\n",
           pir.motion_count > 0 ? pir.total_motion_time / pir.motion_count : 0);
    printf("============================================\r\n\r\n");
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    printf("\r\n============================================\r\n");
    printf("   MH-SR602 PIR Motion Sensor Test Program\r\n");
    printf("   NUCLEO-F103RB Board\r\n");
    printf("============================================\r\n\n");
    printf("OUT Pin: PA0 (Digital Input, Active HIGH)\r\n");
    printf("Detection Range: 0~5 meters (0~3.5m recommended)\r\n");
    printf("Output Duration: ~2.5s (default, adjustable)\r\n\n");
    printf("Sensor Behavior:\r\n");
    printf("  - HIGH signal when motion is detected\r\n");
    printf("  - LOW signal when no motion (idle)\r\n");
    printf("  - Cooldown period: %d ms to avoid retrigger\r\n\n", COOLDOWN_TIME_MS);
    printf("Wave your hand in front of the sensor to test.\r\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\r\n\n");

    /* Initialize PIR state tracking */
    PIR_Init();

    uint32_t last_print_time = 0;
    uint32_t last_stats_time = 0;

    while (1) {
        /* Update PIR state machine */
        PIR_Update();

        uint32_t current_time = HAL_GetTick();

        /* LED control: ON when motion detected, OFF when idle */
        if (PIR_IsMotionDetected()) {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        }

        /* Periodic status update (every 250ms) */
        if (current_time - last_print_time >= 250) {
            last_print_time = current_time;
            Print_Status();
        }

        /* Periodic statistics (every 30 seconds) */
        if (current_time - last_stats_time >= 30000) {
            last_stats_time = current_time;
            Print_Statistics();
        }

        HAL_Delay(10);
    }
}

/**
  * @brief System Clock Configuration
  * @retval None
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
  * @brief GPIO Initialization Function
  * @retval None
  */
static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* PA0 - PIR sensor OUT (Digital Input with pull-down) */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;  /* Pull-down: default LOW = idle */
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PA5 - Onboard LED (Output) */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Configure user button (PC13, B1) as EXTI */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* EXTI interrupt for user button */
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/**
  * @brief USART2 Initialization Function
  * @retval None
  */
static void MX_USART2_UART_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_USART2_CLK_ENABLE();

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
  * @brief  EXTI line detection callbacks
  * @param  GPIO_Pin: Port pin number
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_13) {
        /* User button pressed - reset statistics */
        pir.motion_count = 0;
        pir.total_motion_time = 0;
        pir.total_idle_time = 0;
        printf("\r\n>>> Statistics reset by user button <<<\r\n");
    }

    /* PIR motion can also be connected via EXTI for instant response */
    if (GPIO_Pin == GPIO_PIN_0) {
        /* PIR EXTI mode - handled in main loop but available for interrupt use */
        /* Uncomment if PIR is configured as EXTI in MX_GPIO_Init */
    }
}

void Error_Handler(void) {
    __disable_irq();
    while (1) {
    }
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line) {
    printf("Wrong parameters value: file %s on line %d\r\n", file, line);
}
#endif /* USE_FULL_ASSERT */
