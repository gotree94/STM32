/**
  ******************************************************************************
  * @file    ir_transmitter.h
  * @brief   KY-005 IR Transmitter driver header (NEC protocol)
  * 
  * Hardware: KY-005 IR LED module (940nm, 38kHz carrier)
  * MCU:     STM32F103RB (NUCLEO-F103RB)
  * 
  * Pin connection:
  *   KY-005 S  -> PA0 (TIM2_CH1 - 38kHz PWM)
  *   KY-005 VCC -> 5V (NUCLEO pin 11 or CN6 pin 5)
  *   KY-005 GND -> GND (NUCLEO pin 14 or CN6 pin 3)
  ******************************************************************************
  */

#ifndef __IR_TRANSMITTER_H
#define __IR_TRANSMITTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/* ========== NEC Protocol Timing (microseconds) ========== */
#define IR_NEC_LEADER_ON      9000   /* Leader code: carrier ON  (9.0 ms) */
#define IR_NEC_LEADER_OFF     4500   /* Leader code: carrier OFF (4.5 ms) */
#define IR_NEC_BIT_PERIOD     562    /* One bit: carrier ON (562 us) */
#define IR_NEC_BIT_0_OFF      562    /* Logic 0: space (562 us)  */
#define IR_NEC_BIT_1_OFF      1687   /* Logic 1: space (1687 us) */
#define IR_NEC_REPEAT_ON      9000   /* Repeat:  carrier ON  (9.0 ms) */
#define IR_NEC_REPEAT_OFF     2250   /* Repeat:  carrier OFF (2.25 ms)*/
#define IR_NEC_END_BURST      562    /* End:     carrier ON  (562 us) */

/* ========== Carrier Frequency ========== */
#define IR_CARRIER_HZ         38000  /* 38 kHz standard IR carrier */

/* ========== API Functions ========== */

/**
  * @brief  Initialize IR transmitter driver
  * @param  htim_pwm   Timer handle for 38kHz PWM output
  * @param  channel    PWM channel (TIM_CHANNEL_x)
  * @param  htim_delay Timer handle for microsecond delay (1us tick)
  */
void IR_Transmitter_Init(TIM_HandleTypeDef *htim_pwm, uint32_t channel,
                         TIM_HandleTypeDef *htim_delay);

/**
  * @brief  Send full NEC protocol frame (address + command)
  * @param  address  16-bit address (for standard NEC: use 8-bit + 0xFF)
  *         Example: IR_Send_NEC(0x00FF, 0x0C) for standard NEC
  * @param  command  8-bit command code
  */
void IR_Send_NEC(uint16_t address, uint8_t command);

/**
  * @brief  Send standard NEC frame (8-bit address auto-inverted)
  * @param  address  8-bit address
  * @param  command  8-bit command code
  */
void IR_Send_NEC_Standard(uint8_t address, uint8_t command);

/**
  * @brief  Send NEC repeat code (for repeated button press)
  */
void IR_Send_NEC_Repeat(void);

/**
  * @brief  Send raw IR timing data (for any protocol)
  * @param  data     Array of timing values in microseconds
  *         Even indices = carrier ON time
  *         Odd indices  = carrier OFF time
  * @param  len      Number of elements in the data array
  */
void IR_Send_Raw(uint16_t *data, uint16_t len);

/**
  * @brief  Send Sony SIRC protocol frame (12-bit)
  * @param  command  7-bit command
  * @param  address  5-bit address
  */
void IR_Send_Sony(uint8_t command, uint8_t address);

/**
  * @brief  Get the last transmission status
  * @retval 0 = idle, 1 = busy
  */
uint8_t IR_Get_Status(void);

#ifdef __cplusplus
}
#endif

#endif /* __IR_TRANSMITTER_H */
