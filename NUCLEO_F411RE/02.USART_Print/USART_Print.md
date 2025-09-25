# USART_Print : printf 시리얼 디버깅 

   * 터미널 통신 프로그램 설치 : [https://github.com/TeraTermProject/teraterm/releases?authuser=0]

<img width="600" height="400" alt="001" src="https://github.com/user-attachments/assets/27a8e2d9-03ab-4a6e-bd36-b6942ac06c9d" />
<br>
<img width="492" height="558" alt="002" src="https://github.com/user-attachments/assets/8dfd9e93-c51e-445f-a859-70bbd210f7f2" />
<br>
<img width="486" height="129" alt="003" src="https://github.com/user-attachments/assets/dbd3e3ae-8790-43de-ad4f-0452210bef4f" />
<br>
<img width="600" height="400" alt="004" src="https://github.com/user-attachments/assets/05a4f6fb-26f9-4245-bb91-1f55d52fc2af" />
<br>
<img width="600" height="400" alt="005" src="https://github.com/user-attachments/assets/f4f75e6b-6a31-4e27-a9f9-a76060fb105a" />
<br>


```c
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */
```

```c
/* USER CODE BEGIN 0 */
#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART1 and Loop until the end of transmission */
  if (ch == '\n')
    HAL_UART_Transmit (&huart2, (uint8_t*) "\r", 1, 0xFFFF);
  HAL_UART_Transmit (&huart2, (uint8_t*) &ch, 1, 0xFFFF);

  return ch;
}
```

```c
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  printf("Hello World!\n");
	  HAL_Delay(1000);
    /* USER CODE END WHILE */
```

