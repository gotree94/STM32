/**
  ******************************************************************************
  * @file    ir_transmitter.c
  * @brief   KY-005 IR Transmitter driver implementation
  *
  * Supported protocols:
  *   - NEC (standard 8-bit address + inverted address)
  *   - NEC (extended 16-bit address)
  *   - Sony SIRC (12-bit)
  *   - Raw timing (any protocol)
  *
  * Hardware principle:
  *   The KY-005 is a 940nm IR LED. The IRremote library modulates data at
  *   38kHz carrier frequency. This driver uses a hardware timer (PWM mode)
  *   to generate the 38kHz carrier and another timer for microsecond timing.
  *
  * NEC Protocol frame format:
  *   Leader (9ms ON + 4.5ms OFF)
  *   8-bit address + 8-bit inverted address
  *   8-bit command  + 8-bit inverted command
  *   Stop bit (562us ON)
  *
  *   Logic 0: 562us ON + 562us OFF
  *   Logic 1: 562us ON + 1687us OFF
  ******************************************************************************
  */

#include "ir_transmitter.h"

/* ========== Private Variables ========== */
static TIM_HandleTypeDef *hIR_PWM   = NULL;   /* Timer for 38kHz carrier */
static uint32_t            IR_Channel = 0;     /* PWM channel */
static TIM_HandleTypeDef *hIR_Delay = NULL;   /* Timer for 1us tick */

static volatile uint8_t ir_busy_flag = 0;     /* Transmission busy flag */

/* ========== Private Helper Functions ========== */

/**
  * @brief  Microsecond delay using timer polling
  * @param  us  Delay time in microseconds (max: depends on timer period)
  */
static void IR_Delay_US(uint32_t us)
{
    if (hIR_Delay == NULL) return;

    __HAL_TIM_SET_COUNTER(hIR_Delay, 0);
    while (__HAL_TIM_GET_COUNTER(hIR_Delay) < us)
    {
        /* Wait */
    }
}

/**
  * @brief  Turn on IR carrier (start PWM output)
  */
static void IR_Carrier_On(void)
{
    HAL_TIM_PWM_Start(hIR_PWM, IR_Channel);
}

/**
  * @brief  Turn off IR carrier (stop PWM output)
  */
static void IR_Carrier_Off(void)
{
    HAL_TIM_PWM_Stop(hIR_PWM, IR_Channel);
}

/**
  * @brief  Send one data bit using NEC timing
  * @param  bit  0 = logic 0, 1 = logic 1
  */
static void IR_Send_Bit(uint8_t bit)
{
    /* Common 562us carrier burst */
    IR_Carrier_On();
    IR_Delay_US(IR_NEC_BIT_PERIOD);
    IR_Carrier_Off();

    /* Space depends on bit value */
    if (bit)
        IR_Delay_US(IR_NEC_BIT_1_OFF);   /* Logic 1: 1687us space */
    else
        IR_Delay_US(IR_NEC_BIT_0_OFF);   /* Logic 0: 562us space */
}

/**
  * @brief  Send NEC leader code
  */
static void IR_Send_Leader(void)
{
    IR_Carrier_On();
    IR_Delay_US(IR_NEC_LEADER_ON);
    IR_Carrier_Off();
    IR_Delay_US(IR_NEC_LEADER_OFF);
}

/**
  * @brief  Send NEC end burst (stop bit)
  */
static void IR_Send_Stop(void)
{
    IR_Carrier_On();
    IR_Delay_US(IR_NEC_END_BURST);
    IR_Carrier_Off();
}

/* ========== Public API Implementation ========== */

/**
  * @brief  Initialize the IR transmitter
  * @note   Call this after HAL timer initialization (MX_TIMx_Init)
  *         The PWM timer must be configured for 38kHz, 50% duty cycle
  *         The delay timer must be configured for 1us tick
  */
void IR_Transmitter_Init(TIM_HandleTypeDef *htim_pwm, uint32_t channel,
                         TIM_HandleTypeDef *htim_delay)
{
    hIR_PWM   = htim_pwm;
    IR_Channel = channel;
    hIR_Delay = htim_delay;

    /* Start the delay timer in counter mode */
    HAL_TIM_Base_Start(hIR_Delay);

    /* Ensure PWM output is disabled initially */
    IR_Carrier_Off();

    ir_busy_flag = 0;
}

/**
  * @brief  Send NEC frame (extended 16-bit address)
  *
  * Frame: Leader + 16-bit address (LSB first) + 8-bit cmd + ~cmd + Stop
  *
  * For extended NEC: address is 16-bit
  * For standard NEC: use IR_Send_NEC_Standard() instead
  */
void IR_Send_NEC(uint16_t address, uint8_t command)
{
    uint16_t addr = address;
    uint8_t  cmd  = command;
    uint8_t  inv_cmd;
    uint8_t  i;

    if (ir_busy_flag) return;
    ir_busy_flag = 1;

    /* 1. Leader code */
    IR_Send_Leader();

    /* 2. Address: 16 bits, LSB first */
    for (i = 0; i < 16; i++)
    {
        IR_Send_Bit(addr & 0x01);
        addr >>= 1;
    }

    /* 3. Command: 8 bits, LSB first */
    for (i = 0; i < 8; i++)
    {
        IR_Send_Bit(cmd & 0x01);
        cmd >>= 1;
    }

    /* 4. Inverted command: 8 bits, LSB first */
    inv_cmd = ~command;
    for (i = 0; i < 8; i++)
    {
        IR_Send_Bit(inv_cmd & 0x01);
        inv_cmd >>= 1;
    }

    /* 5. Stop bit */
    IR_Send_Stop();

    ir_busy_flag = 0;
}

/**
  * @brief  Send standard NEC frame (8-bit address + inverted address)
  *
  * Frame: Leader + 8-bit addr + ~addr + 8-bit cmd + ~cmd + Stop
  *
  * Example: IR_Send_NEC_Standard(0x00, 0x0C) for Panasonic TV power
  */
void IR_Send_NEC_Standard(uint8_t address, uint8_t command)
{
    uint8_t addr    = address;
    uint8_t inv_addr;
    uint8_t cmd     = command;
    uint8_t inv_cmd;
    uint8_t i;

    if (ir_busy_flag) return;
    ir_busy_flag = 1;

    /* 1. Leader code */
    IR_Send_Leader();

    /* 2. Address: 8 bits, LSB first */
    for (i = 0; i < 8; i++)
    {
        IR_Send_Bit(addr & 0x01);
        addr >>= 1;
    }

    /* 3. Inverted address: 8 bits, LSB first */
    inv_addr = ~address;
    for (i = 0; i < 8; i++)
    {
        IR_Send_Bit(inv_addr & 0x01);
        inv_addr >>= 1;
    }

    /* 4. Command: 8 bits, LSB first */
    for (i = 0; i < 8; i++)
    {
        IR_Send_Bit(cmd & 0x01);
        cmd >>= 1;
    }

    /* 5. Inverted command: 8 bits, LSB first */
    inv_cmd = ~command;
    for (i = 0; i < 8; i++)
    {
        IR_Send_Bit(inv_cmd & 0x01);
        inv_cmd >>= 1;
    }

    /* 6. Stop bit */
    IR_Send_Stop();

    ir_busy_flag = 0;
}

/**
  * @brief  Send NEC repeat code
  *
  * Repeat frame: 9ms ON + 2.25ms OFF + 562us ON
  * Used when a button is held down continuously.
  */
void IR_Send_NEC_Repeat(void)
{
    if (ir_busy_flag) return;
    ir_busy_flag = 1;

    IR_Carrier_On();
    IR_Delay_US(IR_NEC_REPEAT_ON);
    IR_Carrier_Off();
    IR_Delay_US(IR_NEC_REPEAT_OFF);

    IR_Carrier_On();
    IR_Delay_US(IR_NEC_END_BURST);
    IR_Carrier_Off();

    ir_busy_flag = 0;
}

/**
  * @brief  Send raw IR data array (for arbitrary protocols)
  * @param  data  Timing pairs: [ON_time, OFF_time, ON_time, OFF_time, ...]
  *               Values in microseconds
  * @param  len   Number of elements (must be even for valid pairs)
  *
  * Example for NEC leader:
  *   uint16_t leader[] = {9000, 4500};
  *   IR_Send_Raw(leader, 2);
  */
void IR_Send_Raw(uint16_t *data, uint16_t len)
{
    if (ir_busy_flag) return;
    if (data == NULL) return;

    ir_busy_flag = 1;

    for (uint16_t i = 0; i < len; i++)
    {
        if (i % 2 == 0)
            IR_Carrier_On();
        else
            IR_Carrier_Off();

        IR_Delay_US(data[i]);
    }

    /* Ensure carrier is off after transmission */
    IR_Carrier_Off();

    ir_busy_flag = 0;
}

/**
  * @brief  Send Sony SIRC protocol frame (12-bit)
  *
  * Sony SIRC frame format:
  *   Leader: 2.4ms ON + 600us OFF
  *   7-bit command (LSB first)
  *   5-bit address (LSB first)
  *   Stop bit (no explicit stop - last bit space ends the frame)
  *
  * Timing:
  *   Carrier ON pulse:  600us
  *   Logic 0 space:     600us
  *   Logic 1 space:     1200us
  *
  * @param  command  7-bit command code
  * @param  address  5-bit device address
  */
void IR_Send_Sony(uint8_t command, uint8_t command_bits, uint8_t address)
{
    uint8_t i;

    if (ir_busy_flag) return;
    ir_busy_flag = 1;

    /* Sony leader: 2.4ms ON + 600us OFF */
    IR_Carrier_On();
    IR_Delay_US(2400);
    IR_Carrier_Off();
    IR_Delay_US(600);

    /* Send command bits */
    for (i = 0; i < command_bits; i++)
    {
        IR_Carrier_On();
        IR_Delay_US(600);
        IR_Carrier_Off();

        if (command & 0x01)
            IR_Delay_US(1200);    /* Logic 1: 1200us space */
        else
            IR_Delay_US(600);     /* Logic 0: 600us space */

        command >>= 1;
    }

    /* Send address bits (5-bit) */
    for (i = 0; i < 5; i++)
    {
        IR_Carrier_On();
        IR_Delay_US(600);
        IR_Carrier_Off();

        if (address & 0x01)
            IR_Delay_US(1200);
        else
            IR_Delay_US(600);

        address >>= 1;
    }

    IR_Carrier_Off();
    ir_busy_flag = 0;
}

/**
  * @brief  Get IR transmitter status
  * @retval 0 = idle (ready to send), 1 = busy (transmission in progress)
  */
uint8_t IR_Get_Status(void)
{
    return ir_busy_flag;
}
