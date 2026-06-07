/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   Interrupt Service Routines
  ******************************************************************************
  */

#include "stm32f1xx_it.h"
#include "main.h"

extern UART_HandleTypeDef huart2;

/******************************************************************************/
/*           Cortex-M3 Processor Exception Handlers                           */
/******************************************************************************/

/**
  * @brief  NMI Handler
  */
void NMI_Handler(void)
{
}

/**
  * @brief  Hard Fault Handler
  */
void HardFault_Handler(void)
{
    while (1)
    {
        /* Trap here for debugging */
    }
}

/**
  * @brief  Memory Management Fault Handler
  */
void MemManage_Handler(void)
{
    while (1)
    {
    }
}

/**
  * @brief  Bus Fault Handler
  */
void BusFault_Handler(void)
{
    while (1)
    {
    }
}

/**
  * @brief  Usage Fault Handler
  */
void UsageFault_Handler(void)
{
    while (1)
    {
    }
}

/**
  * @brief  SVC Handler
  */
void SVC_Handler(void)
{
}

/**
  * @brief  Debug Monitor Handler
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  PendSV Handler
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  SysTick Handler (HAL timebase)
  */
void SysTick_Handler(void)
{
    HAL_IncTick();
}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/******************************************************************************/

/**
  * @brief  This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart2);
}

/**
  * @brief  This function handles EXTI line[15:10] interrupts.
  *         Used for User Button (PC13)
  */
void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}
