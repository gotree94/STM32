# ADC_TemperatureSensor

<img width="300" height="400" alt="001" src="https://github.com/user-attachments/assets/7fbd13f5-1f60-40b9-911c-26835c6fc8a5" />
<br>
<img width="600" height="400" alt="002" src="https://github.com/user-attachments/assets/e655889c-7f55-4b13-8cb3-71ce6b86b7d9" />
<br>
<img width="600" height="400" alt="003" src="https://github.com/user-attachments/assets/e15a2b0b-f199-40f9-ad7b-54a21c836b61" />
<br>
<img width="600" height="400" alt="005" src="https://github.com/user-attachments/assets/20440010-d87a-4c48-8727-4b7ab530b8ea" />
<br>
<img width="600" height="400" alt="004" src="https://github.com/user-attachments/assets/d8725743-9ef4-4ccc-842c-0cc914bd09ac" />
<br>


```c
/* USER CODE BEGIN PV */
const float AVG_SLOPE = 4.3E-03;
const float V25 = 1.43;
const float ADC_TO_VOLT = 3.3 / 4096;
/* USER CODE END PV */
```


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
  /* USER CODE BEGIN 2 */
  uint16_t adc1;

  float vSense;
  float temp;

  if(HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
  {
	  Error_Handler();
  }

  if(HAL_ADC_Start(&hadc1) != HAL_OK)
  {
	  Error_Handler();
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  HAL_ADC_PollForConversion(&hadc1, 100);
	  adc1 = HAL_ADC_GetValue(&hadc1);
	  //printf("ADC1_temperature : %d \n",adc1);

	  vSense = adc1 * ADC_TO_VOLT;
	  temp = (V25 - vSense) / AVG_SLOPE + 25.0;
	  printf("temperature : %d, %f \n",adc1, temp);

	  HAL_Delay(100);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
```
