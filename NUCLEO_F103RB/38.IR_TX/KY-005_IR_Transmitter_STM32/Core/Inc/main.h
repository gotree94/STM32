/**
  ******************************************************************************
  * @file    main.h
  * @brief   Header for main.c (NUCLEO-F103RB KY-005 IR Transmitter)
  ******************************************************************************
  */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include "stm32f1xx_it.h"

/* ========== Board-specific defines ========== */
#define LED_GPIO_Port       GPIOA
#define LED_GPIO_Pin        GPIO_PIN_5
#define B1_GPIO_Port        GPIOC
#define B1_GPIO_Pin         GPIO_PIN_13

/* ========== Exported functions ========== */
void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
