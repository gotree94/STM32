/**
 ******************************************************************************
 * @file    main.c
 * @brief   IR Remote Control Decoder for STM32F103 NUCLEO
 * @author  Embedded Systems Lab
 * @version V1.0.0
 * @date    2025-01-18
 ******************************************************************************
 * @details
 * 이 프로젝트는 IR 리모컨 신호를 수신하여 NEC 프로토콜로 디코딩하고
 * 주소와 명령 코드를 시리얼로 PC에 전송합니다.
 * 
 * Hardware Setup:
 * - IR Receiver (TSOP1838/VS1838B): PA0 (Timer Input Capture)
 * - LED: PA5 (On-board LED on NUCLEO)
 * - UART2: PA2(TX), PA3(RX) - USB Virtual COM Port
 * 
 * Supported Protocol: NEC (and NEC Extended)
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

/* NEC Protocol Timing (in microseconds) */
#define NEC_LEADER_PULSE    9000    // 9ms leader pulse
#define NEC_LEADER_SPACE    4500    // 4.5ms leader space
#define NEC_REPEAT_SPACE    2250    // 2.25ms repeat space
#define NEC_BIT_PULSE       560     // 560us bit pulse
#define NEC_ONE_SPACE       1690    // 1.69ms space for '1'
#define NEC_ZERO_SPACE      560     // 560us space for '0'

/* Timing tolerances (±25%) */
#define TOLERANCE           25
#define IN_RANGE(val, target) ((val) > ((target) * (100-TOLERANCE) / 100) && \
                               (val) < ((target) * (100+TOLERANCE) / 100))

/* IR Decoder States */
typedef enum {
    IR_STATE_IDLE,
    IR_STATE_LEADER_PULSE,
    IR_STATE_LEADER_SPACE,
    IR_STATE_DATA_PULSE,
    IR_STATE_DATA_SPACE,
    IR_STATE_COMPLETE,
    IR_STATE_REPEAT,
    IR_STATE_ERROR
} IR_State_t;

/* IR Decoded Data */
typedef struct {
    uint8_t address;
    uint8_t address_inv;
    uint8_t command;
    uint8_t command_inv;
    uint8_t is_repeat;
    uint8_t is_valid;
} IR_Data_t;

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

volatile IR_State_t ir_state = IR_STATE_IDLE;
volatile uint32_t ir_raw_data = 0;
volatile uint8_t ir_bit_count = 0;
volatile uint32_t last_capture = 0;
volatile uint8_t capture_ready = 0;
volatile uint32_t pulse_width = 0;
volatile uint32_t space_width = 0;

IR_Data_t ir_data = {0};

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART2_UART_Init(void);
void IR_Decode(void);
void IR_Reset(void);
void IR_ProcessData(void);
const char* IR_GetKeyName(uint8_t address, uint8_t command);

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
    MX_TIM2_Init();
    MX_TIM3_Init();
    MX_USART2_UART_Init();
    
    /* Start timers */
    HAL_TIM_Base_Start(&htim2);
    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
    
    printf("\r\n================================================\r\n");
    printf("   IR Remote Control Decoder - STM32F103\r\n");
    printf("================================================\r\n");
    printf("Protocol: NEC / NEC Extended\r\n");
    printf("IR Receiver Pin: PA0\r\n");
    printf("------------------------------------------------\r\n");
    printf("Point your IR remote at the receiver and\r\n");
    printf("press any button to see the decoded values.\r\n");
    printf("------------------------------------------------\r\n\n");
    
    uint32_t rx_count = 0;
    
    while (1) {
        /* Check if decoding is complete */
        if (ir_state == IR_STATE_COMPLETE || ir_state == IR_STATE_REPEAT) {
            IR_ProcessData();
            
            if (ir_data.is_valid) {
                rx_count++;
                
                /* Toggle LED on valid reception */
                HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
                
                if (ir_data.is_repeat) {
                    printf("[%lu] REPEAT\r\n", rx_count);
                } else {
                    printf("[%lu] Received IR Code:\r\n", rx_count);
                    printf("       Address:     0x%02X (inv: 0x%02X)\r\n", 
                           ir_data.address, ir_data.address_inv);
                    printf("       Command:     0x%02X (inv: 0x%02X)\r\n", 
                           ir_data.command, ir_data.command_inv);
                    
                    /* Validate NEC frame */
                    if ((ir_data.address ^ ir_data.address_inv) == 0xFF &&
                        (ir_data.command ^ ir_data.command_inv) == 0xFF) {
                        printf("       Status:      Valid NEC Frame\r\n");
                    } else if ((ir_data.command ^ ir_data.command_inv) == 0xFF) {
                        printf("       Status:      Valid NEC Extended (16-bit addr)\r\n");
                        printf("       Full Addr:   0x%04X\r\n", 
                               (ir_data.address_inv << 8) | ir_data.address);
                    } else {
                        printf("       Status:      Checksum Error!\r\n");
                    }
                    
                    /* Try to identify key */
                    const char* key_name = IR_GetKeyName(ir_data.address, ir_data.command);
                    if (key_name) {
                        printf("       Key:         %s\r\n", key_name);
                    }
                    
                    printf("\r\n");
                }
            }
            
            IR_Reset();
        }
        
        /* Handle decoding errors */
        if (ir_state == IR_STATE_ERROR) {
            IR_Reset();
        }
    }
}

/**
 * @brief  Reset IR decoder state
 */
void IR_Reset(void) {
    ir_state = IR_STATE_IDLE;
    ir_raw_data = 0;
    ir_bit_count = 0;
    last_capture = 0;
    pulse_width = 0;
    space_width = 0;
    memset(&ir_data, 0, sizeof(ir_data));
}

/**
 * @brief  Process decoded raw data into IR_Data structure
 */
void IR_ProcessData(void) {
    if (ir_state == IR_STATE_REPEAT) {
        ir_data.is_repeat = 1;
        ir_data.is_valid = 1;
        return;
    }
    
    if (ir_bit_count == 32) {
        /* Extract bytes from raw data (LSB first) */
        ir_data.address = (ir_raw_data >> 0) & 0xFF;
        ir_data.address_inv = (ir_raw_data >> 8) & 0xFF;
        ir_data.command = (ir_raw_data >> 16) & 0xFF;
        ir_data.command_inv = (ir_raw_data >> 24) & 0xFF;
        ir_data.is_repeat = 0;
        ir_data.is_valid = 1;
    } else {
        ir_data.is_valid = 0;
    }
}

/**
 * @brief  Timer Input Capture Interrupt Callback
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM3 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        uint32_t current_capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        uint32_t width;
        
        /* Calculate pulse/space width */
        if (current_capture >= last_capture) {
            width = current_capture - last_capture;
        } else {
            width = (0xFFFF - last_capture) + current_capture;
        }
        last_capture = current_capture;
        
        /* Get current pin state to determine pulse or space */
        uint8_t pin_state = HAL_GPIO_ReadPin(IR_RECV_GPIO_Port, IR_RECV_Pin);
        
        /* State machine for NEC decoding */
        switch (ir_state) {
            case IR_STATE_IDLE:
                /* Wait for falling edge (start of leader pulse) */
                if (pin_state == 0) {
                    ir_state = IR_STATE_LEADER_PULSE;
                    ir_raw_data = 0;
                    ir_bit_count = 0;
                }
                break;
                
            case IR_STATE_LEADER_PULSE:
                /* Rising edge - end of leader pulse */
                if (pin_state == 1) {
                    pulse_width = width;
                    if (IN_RANGE(pulse_width, NEC_LEADER_PULSE)) {
                        ir_state = IR_STATE_LEADER_SPACE;
                    } else {
                        ir_state = IR_STATE_ERROR;
                    }
                }
                break;
                
            case IR_STATE_LEADER_SPACE:
                /* Falling edge - end of leader space */
                if (pin_state == 0) {
                    space_width = width;
                    if (IN_RANGE(space_width, NEC_LEADER_SPACE)) {
                        /* Normal frame */
                        ir_state = IR_STATE_DATA_PULSE;
                    } else if (IN_RANGE(space_width, NEC_REPEAT_SPACE)) {
                        /* Repeat frame */
                        ir_state = IR_STATE_REPEAT;
                    } else {
                        ir_state = IR_STATE_ERROR;
                    }
                }
                break;
                
            case IR_STATE_DATA_PULSE:
                /* Rising edge - end of data pulse */
                if (pin_state == 1) {
                    pulse_width = width;
                    if (IN_RANGE(pulse_width, NEC_BIT_PULSE)) {
                        ir_state = IR_STATE_DATA_SPACE;
                    } else {
                        ir_state = IR_STATE_ERROR;
                    }
                }
                break;
                
            case IR_STATE_DATA_SPACE:
                /* Falling edge - end of data space */
                if (pin_state == 0) {
                    space_width = width;
                    
                    if (IN_RANGE(space_width, NEC_ONE_SPACE)) {
                        /* Logic 1 */
                        ir_raw_data |= (1UL << ir_bit_count);
                        ir_bit_count++;
                    } else if (IN_RANGE(space_width, NEC_ZERO_SPACE)) {
                        /* Logic 0 */
                        ir_bit_count++;
                    } else {
                        ir_state = IR_STATE_ERROR;
                        break;
                    }
                    
                    if (ir_bit_count >= 32) {
                        ir_state = IR_STATE_COMPLETE;
                    } else {
                        ir_state = IR_STATE_DATA_PULSE;
                    }
                }
                break;
                
            default:
                break;
        }
    }
}

/**
 * @brief  Get key name for common remote controls
 * @param  address: IR address
 * @param  command: IR command
 * @retval Key name string or NULL if unknown
 */
const char* IR_GetKeyName(uint8_t address, uint8_t command) {
    /* Common NEC remote control key mappings */
    /* Address 0x00 - Generic remotes */
    if (address == 0x00) {
        switch (command) {
            case 0x45: return "CH-";
            case 0x46: return "CH";
            case 0x47: return "CH+";
            case 0x44: return "PREV";
            case 0x40: return "NEXT";
            case 0x43: return "PLAY/PAUSE";
            case 0x07: return "VOL-";
            case 0x15: return "VOL+";
            case 0x09: return "EQ";
            case 0x16: return "0";
            case 0x19: return "100+";
            case 0x0D: return "200+";
            case 0x0C: return "1";
            case 0x18: return "2";
            case 0x5E: return "3";
            case 0x08: return "4";
            case 0x1C: return "5";
            case 0x5A: return "6";
            case 0x42: return "7";
            case 0x52: return "8";
            case 0x4A: return "9";
        }
    }
    
    /* Add more remote control mappings here */
    
    return NULL;
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

    /* LED Output - PA5 */
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = LED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);
}

/**
 * @brief TIM2 Initialization - General purpose timer
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
 * @brief TIM3 Initialization - Input Capture for IR decoding
 */
static void MX_TIM3_Init(void) {
    TIM_IC_InitTypeDef sConfigIC = {0};

    __HAL_RCC_TIM3_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* PA6 -> TIM3_CH1 (Remap PA0 signal internally or use PA6) */
    /* For this example, we use PA6 as TIM3_CH1 */
    /* If using PA0, you need external wiring: PA0 -> PA6 */
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Also configure PA0 for polling/debugging */
    GPIO_InitStruct.Pin = IR_RECV_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(IR_RECV_GPIO_Port, &GPIO_InitStruct);

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 71;          // 72MHz / 72 = 1MHz (1us resolution)
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 0xFFFF;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim3);
    HAL_TIM_IC_Init(&htim3);

    /* Input Capture on both edges */
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_BOTHEDGE;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0x0F;          // Strong filter for noise rejection
    HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1);
    
    /* Enable TIM3 interrupt */
    HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
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
 * @brief TIM3 Interrupt Handler
 */
void TIM3_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim3);
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
