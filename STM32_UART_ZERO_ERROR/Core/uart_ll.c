#include "stm32f1xx_ll_usart.h"

void UART_LL_Init(void)
{
  LL_USART_SetBaudRate(USART2, 36864000, LL_USART_OVERSAMPLING_16, 115200);
  LL_USART_Enable(USART2);
}
