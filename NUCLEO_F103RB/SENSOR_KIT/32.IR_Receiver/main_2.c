/**
 ******************************************************************************
 * @file    main.c
 * @brief   IR Receiver Module Test for STM32F103 NUCLEO
 * @author  Embedded Systems Lab
 * @version V2.1.0
 * @date    2025-01-22
 ******************************************************************************
 * @details
 * 이 프로젝트는 IR 수신 모듈을 테스트하고 NEC 프로토콜을 디코딩합니다.
 * 
 * System Clock: 64MHz (HSI + PLL)
 * 
 * Hardware Setup:
 * - IR Receiver (TSOP1838/VS1838B): PA0 (Digital Input with Pull-up)
 * - LED: PA5 (On-board LED on NUCLEO)
 * - UART2: PA2(TX), PA3(RX) - USB Virtual COM Port @ 115200 baud
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* Private defines -----------------------------------------------------------*/
#define IR_RECV_Pin         GPIO_PIN_0
#define IR_RECV_GPIO_Port   GPIOA
#define LED_Pin             GPIO_PIN_5
#define LED_GPIO_Port       GPIOA

/* System Clock defines */
#define SYSCLK_FREQ_HZ      64000000UL

/* NEC Protocol Timing (in microseconds) - 더 넓은 허용 오차 */
#define NEC_LEADER_PULSE_MIN    8000
#define NEC_LEADER_PULSE_MAX    10000
#define NEC_LEADER_SPACE_MIN    4000
#define NEC_LEADER_SPACE_MAX    5000
#define NEC_REPEAT_SPACE_MIN    2000
#define NEC_REPEAT_SPACE_MAX    2800
#define NEC_BIT_THRESHOLD       1000    // Space > 1000us = '1', Space < 1000us = '0'

/* IR Receiver parameters */
#define MAX_EDGES               100
#define SIGNAL_TIMEOUT_MS       80
#define MIN_PULSE_US            300     // 최소 유효 펄스

/* Edge type */
typedef enum {
    EDGE_FALLING = 0,   // HIGH -> LOW (IR 신호 시작)
    EDGE_RISING = 1     // LOW -> HIGH (IR 신호 끝)
} EdgeType_t;

/* Edge record */
typedef struct {
    uint32_t time_us;
    EdgeType_t type;
} EdgeRecord_t;

/* IR Data structure */
typedef struct {
    uint8_t address;
    uint8_t address_inv;
    uint8_t command;
    uint8_t command_inv;
    uint32_t raw_data;
    uint8_t bit_count;
    uint8_t is_repeat;
    uint8_t is_valid;
} IR_Data_t;

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;
TIM_HandleTypeDef htim2;

/* Edge recording */
volatile EdgeRecord_t edges[MAX_EDGES];
volatile uint8_t edge_count = 0;
volatile uint8_t receiving = 0;
volatile uint32_t last_edge_time = 0;
volatile uint8_t last_pin_state = 1;

/* Decoded data */
IR_Data_t ir_data;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_UART_Init(void);
uint32_t GetMicros(void);
void Error_Handler(void);

void IR_Reset(void);
uint8_t IR_Decode(void);
void IR_Print_Result(void);
void IR_Print_Raw_Timing(void);

/* Printf redirect to UART ---------------------------------------------------*/
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/**
 * @brief  Main function
 */
int main(void) {
    HAL_Init();
    SystemClock_Config();
    
    MX_GPIO_Init();
    MX_TIM2_Init();
    MX_USART2_UART_Init();
    
    HAL_TIM_Base_Start(&htim2);
    
    printf("\r\n");
    printf("========================================\r\n");
    printf("   IR Receiver & NEC Decoder - STM32   \r\n");
    printf("========================================\r\n");
    printf(" System Clock : 64 MHz\r\n");
    printf(" IR Input     : PA0 (Pull-up)\r\n");
    printf(" Status LED   : PA5\r\n");
    printf(" Protocol     : NEC / Extended NEC\r\n");
    printf("========================================\r\n");
    printf("\r\nWaiting for IR signals...\r\n\n");
    
    IR_Reset();
    
    uint32_t signal_count = 0;
    uint32_t last_activity_time = 0;
    uint32_t signal_start_time = 0;
    
    while (1) {
        uint8_t ir_state = HAL_GPIO_ReadPin(IR_RECV_GPIO_Port, IR_RECV_Pin);
        uint32_t current_us = GetMicros();
        uint32_t current_ms = HAL_GetTick();
        
        /* Edge detection */
        if (ir_state != last_pin_state) {
            if (!receiving) {
                /* 새 신호 시작 - Falling edge만 시작으로 인식 */
                if (last_pin_state == 1 && ir_state == 0) {
                    receiving = 1;
                    edge_count = 0;
                    signal_start_time = current_us;
                    signal_count++;
                    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
                    
                    /* 첫 번째 falling edge 기록 */
                    edges[edge_count].time_us = 0;  // 상대 시간 0
                    edges[edge_count].type = EDGE_FALLING;
                    edge_count++;
                }
            }
            else if (edge_count < MAX_EDGES) {
                /* 상대 시간 계산 (오버플로우 처리) */
                uint32_t rel_time = current_us - signal_start_time;
                
                edges[edge_count].time_us = rel_time;
                edges[edge_count].type = (ir_state == 0) ? EDGE_FALLING : EDGE_RISING;
                edge_count++;
            }
            
            last_activity_time = current_ms;
            last_pin_state = ir_state;
        }
        
        /* Signal timeout check */
        if (receiving && (current_ms - last_activity_time > SIGNAL_TIMEOUT_MS)) {
            receiving = 0;
            
            printf("----------------------------------------\r\n");
            printf("[%lu] IR Signal Received (%d edges)\r\n", signal_count, edge_count);
            
            /* Decode attempt */
            if (IR_Decode()) {
                IR_Print_Result();
            } else {
                printf("    Status: Decode failed\r\n");
                IR_Print_Raw_Timing();
            }
            printf("\r\n");
            
            IR_Reset();
        }
    }
}

/**
 * @brief Reset IR receiver state
 */
void IR_Reset(void) {
    edge_count = 0;
    receiving = 0;
    memset((void*)edges, 0, sizeof(edges));
    memset(&ir_data, 0, sizeof(ir_data));
}

/**
 * @brief Calculate duration between two edges
 */
uint32_t GetDuration(uint8_t start_idx, uint8_t end_idx) {
    if (end_idx <= start_idx || end_idx >= edge_count) {
        return 0;
    }
    return edges[end_idx].time_us - edges[start_idx].time_us;
}

/**
 * @brief Decode NEC protocol from edge timing
 * @retval 1 if successful, 0 if failed
 */
uint8_t IR_Decode(void) {
    /* 최소 edge 수 체크: Leader(2) + 32bits(64) + stop(1) = 67 edges */
    /* 하지만 실제로는 노이즈로 인해 더 많거나 적을 수 있음 */
    if (edge_count < 4) {
        return 0;
    }
    
    /* Edge 0: Falling (leader pulse start)
     * Edge 1: Rising (leader pulse end)
     * Edge 2: Falling (leader space end / first bit start)
     */
    
    /* Leader pulse duration: edge[0] -> edge[1] */
    uint32_t leader_pulse = GetDuration(0, 1);
    
    if (leader_pulse < NEC_LEADER_PULSE_MIN || leader_pulse > NEC_LEADER_PULSE_MAX) {
        return 0;
    }
    
    /* Leader space duration: edge[1] -> edge[2] */
    uint32_t leader_space = GetDuration(1, 2);
    
    /* Check for repeat code */
    if (leader_space >= NEC_REPEAT_SPACE_MIN && leader_space <= NEC_REPEAT_SPACE_MAX) {
        ir_data.is_repeat = 1;
        ir_data.is_valid = 1;
        return 1;
    }
    
    /* Check normal leader space */
    if (leader_space < NEC_LEADER_SPACE_MIN || leader_space > NEC_LEADER_SPACE_MAX) {
        return 0;
    }
    
    /* Decode 32 bits
     * 각 비트는 다음과 같이 구성:
     * - Falling edge (bit pulse start)
     * - Rising edge (bit pulse end)
     * - 다음 Falling edge까지의 시간이 space
     * 
     * Space 시간으로 0/1 구분:
     * - Space < 1000us: bit 0
     * - Space >= 1000us: bit 1
     */
    
    uint32_t raw_data = 0;
    uint8_t bit_count = 0;
    
    /* 데이터 비트는 edge index 2부터 시작 */
    /* 각 비트: falling(i) -> rising(i+1) -> falling(i+2) */
    /* Space = edge[i+2].time - edge[i+1].time */
    
    for (uint8_t i = 2; i + 2 < edge_count && bit_count < 32; i += 2) {
        /* Validate: edge[i] should be falling, edge[i+1] should be rising */
        if (edges[i].type != EDGE_FALLING || edges[i + 1].type != EDGE_RISING) {
            continue;  // Skip invalid edges
        }
        
        /* Bit pulse duration */
        uint32_t bit_pulse = GetDuration(i, i + 1);
        
        /* 유효한 비트 펄스인지 확인 (약 560us) */
        if (bit_pulse < MIN_PULSE_US || bit_pulse > 1000) {
            continue;
        }
        
        /* Space duration (to next falling edge) */
        uint32_t space;
        if (i + 2 < edge_count) {
            space = GetDuration(i + 1, i + 2);
        } else {
            /* 마지막 비트는 space가 없을 수 있음 - 기본값 0 */
            space = 500;  // Assume bit 0
        }
        
        /* Decode bit based on space duration */
        if (space >= NEC_BIT_THRESHOLD) {
            raw_data |= (1UL << bit_count);  // Bit 1 (LSB first)
        }
        /* else: Bit 0 (already 0) */
        
        bit_count++;
    }
    
    ir_data.bit_count = bit_count;
    
    /* 최소 비트 수 확인 */
    if (bit_count < 16) {  // 최소 16비트는 있어야 함
        return 0;
    }
    
    /* Extract fields */
    ir_data.raw_data = raw_data;
    ir_data.address = (raw_data >> 0) & 0xFF;
    ir_data.address_inv = (raw_data >> 8) & 0xFF;
    ir_data.command = (raw_data >> 16) & 0xFF;
    ir_data.command_inv = (raw_data >> 24) & 0xFF;
    ir_data.is_repeat = 0;
    ir_data.is_valid = 1;
    
    return 1;
}

/**
 * @brief Print decoded IR result
 */
void IR_Print_Result(void) {
    if (!ir_data.is_valid) {
        printf("    Result: Invalid data\r\n");
        return;
    }
    
    if (ir_data.is_repeat) {
        printf("    >> REPEAT Code <<\r\n");
        return;
    }
    
    printf("\r\n");
    printf("    +----------------------------------+\r\n");
    printf("    |        NEC Decoded Data          |\r\n");
    printf("    +----------------------------------+\r\n");
    printf("    | Bits decoded : %-2d                |\r\n", ir_data.bit_count);
    printf("    | Raw Data     : 0x%08lX        |\r\n", ir_data.raw_data);
    printf("    +----------------------------------+\r\n");
    printf("    | Address      : 0x%02X  (%3d)      |\r\n", ir_data.address, ir_data.address);
    printf("    | Address~     : 0x%02X  (%3d)      |\r\n", ir_data.address_inv, ir_data.address_inv);
    printf("    | Command      : 0x%02X  (%3d)      |\r\n", ir_data.command, ir_data.command);
    printf("    | Command~     : 0x%02X  (%3d)      |\r\n", ir_data.command_inv, ir_data.command_inv);
    printf("    +----------------------------------+\r\n");
    
    /* Validate */
    uint8_t addr_valid = ((ir_data.address ^ ir_data.address_inv) == 0xFF);
    uint8_t cmd_valid = ((ir_data.command ^ ir_data.command_inv) == 0xFF);
    
    if (addr_valid && cmd_valid) {
        printf("    | Protocol: Standard NEC          |\r\n");
        printf("    | Device  : 0x%02X                   |\r\n", ir_data.address);
        printf("    | Key Code: 0x%02X                   |\r\n", ir_data.command);
    }
    else if (cmd_valid) {
        uint16_t ext_addr = ir_data.address | (ir_data.address_inv << 8);
        printf("    | Protocol: Extended NEC          |\r\n");
        printf("    | Device  : 0x%04X                 |\r\n", ext_addr);
        printf("    | Key Code: 0x%02X                   |\r\n", ir_data.command);
    }
    else {
        printf("    | Protocol: Unknown/Checksum Err  |\r\n");
    }
    printf("    +----------------------------------+\r\n");
    
    /* Binary representation */
    printf("\r\n    Binary (LSB first):\r\n");
    printf("    ADDR     : ");
    for (int i = 7; i >= 0; i--) printf("%d", (ir_data.address >> i) & 1);
    printf(" (0x%02X)\r\n", ir_data.address);
    
    printf("    ADDR~    : ");
    for (int i = 7; i >= 0; i--) printf("%d", (ir_data.address_inv >> i) & 1);
    printf(" (0x%02X)\r\n", ir_data.address_inv);
    
    printf("    CMD      : ");
    for (int i = 7; i >= 0; i--) printf("%d", (ir_data.command >> i) & 1);
    printf(" (0x%02X)\r\n", ir_data.command);
    
    printf("    CMD~     : ");
    for (int i = 7; i >= 0; i--) printf("%d", (ir_data.command_inv >> i) & 1);
    printf(" (0x%02X)\r\n", ir_data.command_inv);
}

/**
 * @brief Print raw timing data for debugging
 */
void IR_Print_Raw_Timing(void) {
    printf("\r\n    Raw Edge Timing:\r\n");
    
    uint8_t print_count = (edge_count > 20) ? 20 : edge_count;
    
    for (uint8_t i = 0; i < print_count; i++) {
        char edge_char = (edges[i].type == EDGE_FALLING) ? 'F' : 'R';
        uint32_t duration = 0;
        
        if (i > 0) {
            duration = edges[i].time_us - edges[i-1].time_us;
        }
        
        printf("    [%2d] %c @ %7lu us", i, edge_char, edges[i].time_us);
        if (i > 0) {
            printf(" (delta: %5lu us)", duration);
        }
        printf("\r\n");
    }
    
    if (edge_count > 20) {
        printf("    ... (%d more edges)\r\n", edge_count - 20);
    }
    
    /* 간단한 비트 해석 시도 */
    printf("\r\n    Bit interpretation (space-based):\r\n    ");
    
    uint8_t bit_idx = 0;
    for (uint8_t i = 2; i + 2 < edge_count && bit_idx < 32; i += 2) {
        if (edges[i].type == EDGE_FALLING && edges[i + 1].type == EDGE_RISING) {
            uint32_t space = edges[i + 2].time_us - edges[i + 1].time_us;
            
            if (space >= NEC_BIT_THRESHOLD) {
                printf("1");
            } else {
                printf("0");
            }
            bit_idx++;
            
            if (bit_idx == 8 || bit_idx == 16 || bit_idx == 24) {
                printf(" ");
            }
        }
    }
    printf(" (%d bits)\r\n", bit_idx);
    
    /* 비트를 바이트로 변환해서 표시 */
    if (bit_idx >= 8) {
        printf("    Hex values: ");
        uint32_t val = 0;
        uint8_t b = 0;
        
        for (uint8_t i = 2; i + 2 < edge_count && b < 32; i += 2) {
            if (edges[i].type == EDGE_FALLING && edges[i + 1].type == EDGE_RISING) {
                uint32_t space = edges[i + 2].time_us - edges[i + 1].time_us;
                
                if (space >= NEC_BIT_THRESHOLD) {
                    val |= (1UL << b);
                }
                b++;
            }
        }
        
        printf("0x%02lX ", (val >> 0) & 0xFF);
        if (bit_idx >= 16) printf("0x%02lX ", (val >> 8) & 0xFF);
        if (bit_idx >= 24) printf("0x%02lX ", (val >> 16) & 0xFF);
        if (bit_idx >= 32) printf("0x%02lX", (val >> 24) & 0xFF);
        printf("\r\n");
    }
}

/**
 * @brief Get microseconds from TIM2
 */
uint32_t GetMicros(void) {
    return __HAL_TIM_GET_COUNTER(&htim2);
}

/**
 * @brief System Clock Configuration - 64MHz
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief GPIO Initialization
 */
static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = IR_RECV_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(IR_RECV_GPIO_Port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = LED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);
}

/**
 * @brief TIM2 Initialization - 1MHz (1us tick)
 */
static void MX_TIM2_Init(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 63;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 0xFFFFFFFF;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief USART2 Initialization - 115200 baud
 */
static void MX_USART2_UART_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

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
    
    if (HAL_UART_Init(&huart2) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief Error Handler
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
