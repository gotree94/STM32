/**
  ******************************************************************************
  * @file    stm32f1xx_hal_conf.h
  * @brief   HAL configuration file for STM32F103xB
  ******************************************************************************
  */

#ifndef __STM32F1XX_HAL_CONF_H
#define __STM32F1XX_HAL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========== Debug ========== */
#define USE_HAL_ADC_REGISTER_CALLBACKS     0u
#define USE_HAL_CAN_REGISTER_CALLBACKS     0u
#define USE_HAL_CEC_REGISTER_CALLBACKS     0u
#define USE_HAL_CRYP_REGISTER_CALLBACKS    0u
#define USE_HAL_DAC_REGISTER_CALLBACKS     0u
#define USE_HAL_DCMI_REGISTER_CALLBACKS    0u
#define USE_HAL_DFSDM_REGISTER_CALLBACKS   0u
#define USE_HAL_DMA_REGISTER_CALLBACKS     0u
#define USE_HAL_ETH_REGISTER_CALLBACKS     0u
#define USE_HAL_FLASH_REGISTER_CALLBACKS   0u
#define USE_HAL_GPIO_REGISTER_CALLBACKS    0u
#define USE_HAL_HASH_REGISTER_CALLBACKS    0u
#define USE_HAL_I2C_REGISTER_CALLBACKS     0u
#define USE_HAL_I2S_REGISTER_CALLBACKS     0u
#define USE_HAL_IRDA_REGISTER_CALLBACKS    0u
#define USE_HAL_JPEG_REGISTER_CALLBACKS    0u
#define USE_HAL_NAND_REGISTER_CALLBACKS    0u
#define USE_HAL_NOR_REGISTER_CALLBACKS     0u
#define USE_HAL_PCCARD_REGISTER_CALLBACKS  0u
#define USE_HAL_PWR_REGISTER_CALLBACKS     0u
#define USE_HAL_RCC_REGISTER_CALLBACKS     0u
#define USE_HAL_RTC_REGISTER_CALLBACKS     0u
#define USE_HAL_SD_REGISTER_CALLBACKS      0u
#define USE_HAL_SDRAM_REGISTER_CALLBACKS   0u
#define USE_HAL_SMARTCARD_REGISTER_CALLBACKS 0u
#define USE_HAL_SPI_REGISTER_CALLBACKS     0u
#define USE_HAL_SRAM_REGISTER_CALLBACKS    0u
#define USE_HAL_TIM_REGISTER_CALLBACKS     0u
#define USE_HAL_UART_REGISTER_CALLBACKS    0u
#define USE_HAL_USART_REGISTER_CALLBACKS   0u
#define USE_HAL_WWDG_REGISTER_CALLBACKS    0u

/* ========== HAL module enable ========== */
#define HAL_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED

/* ========== Includes ========== */
#include "stm32f1xx.h"
#include "stm32f103xb.h"   /* Specific to STM32F103RB */

/* ========== HAL system parameters ========== */
#define HSE_VALUE            8000000u   /* NUCLEO-F103RB HSE oscillator: 8MHz */
#define HSI_VALUE            8000000u   /* Internal HSI: 8MHz */
#define LSE_VALUE            32768u     /* LSE (if used) */
#define LSI_VALUE            40000u     /* LSI: 40kHz */

#define TICK_INT_PRIORITY    15u
#define USE_RTOS             0u
#define PREFETCH_ENABLE      1u

/* ========== Assert ========== */
#define assert_param(expr) ((void)0u)

#ifdef __cplusplus
}
#endif

#endif /* __STM32F1XX_HAL_CONF_H */
