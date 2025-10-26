# CDS

<img width="346" height="212" alt="125" src="https://github.com/user-attachments/assets/54b585da-0a65-43d6-ae3b-3423e3167582" />
<br>

<img width="600" height="400" alt="Sheild-001" src="https://github.com/user-attachments/assets/9df5b8c3-d81a-4026-9f86-67fa4dde1e38" />
<br>

<img width="600" height="400" alt="Sheild-002" src="https://github.com/user-attachments/assets/29bdd8e8-3e94-45da-87f4-426694f32622" />
<br>

<img width="600" height="600" alt="F103RB-pin" src="https://github.com/user-attachments/assets/45bb557f-9517-419d-b45c-81a92869bac0" />
<br>

<img width="800" height="600" alt="CDS_006" src="https://github.com/user-attachments/assets/f07becbd-db80-402c-906c-9708f405ae2a" />

<img width="800" height="600" alt="CDS_001" src="https://github.com/user-attachments/assets/e40972ce-5d31-42ec-b063-3489b78d5d1b" />
<br>
<img width="800" height="600" alt="CDS_002" src="https://github.com/user-attachments/assets/4d577cc0-a98d-4f99-a027-cb2d1f535584" />
<br>
<img width="800" height="600" alt="CDS_003" src="https://github.com/user-attachments/assets/5ddcd910-a6ba-4b57-862a-f1cf6c7240d0" />
<br>
<img width="800" height="600" alt="CDS_004" src="https://github.com/user-attachments/assets/e06f64a4-343d-40b4-b54d-af91e7e2e355" />
<br>
<img width="800" height="600" alt="CDS_005" src="https://github.com/user-attachments/assets/9265136c-a096-4fff-873f-b62da6a0e7da" />
<br>


```c
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
  /* e.g. write a character to the USART2 and Loop until the end of transmission */
  if (ch == '\n')
    HAL_UART_Transmit(&huart2, (uint8_t*)"\r", 1, 0xFFFF);
  HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, 0xFFFF);

  return ch;
}
/* USER CODE END 0 */
```

```c
	/* USER CODE BEGIN 1 */
	uint32_t adc_value = 0;
	float voltage = 0.0f;
	uint32_t counter = 0;
	/* USER CODE END 1 */
```

```c
  /* USER CODE BEGIN 2 */
  printf("STM32F103 CDS Sensor Reading Example\r\n");
  printf("System Clock: 64MHz\r\n");
  printf("ADC Channel: PC0\r\n");
  printf("Starting measurements...\r\n\r\n");

  /* Calibrate ADC */
  HAL_ADCEx_Calibration_Start(&hadc1);
  /* USER CODE END 2 */
```

```c
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

	    /* USER CODE BEGIN 3 */

	    /* Start ADC conversion */
	    HAL_ADC_Start(&hadc1);

	    /* Wait for conversion to complete */
	    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
	    {
	      /* Get ADC value */
	      adc_value = HAL_ADC_GetValue(&hadc1);

	      /* Convert ADC value to voltage (3.3V reference, 12-bit ADC) */
	      voltage = (adc_value * 3.3f) / 4095.0f;

	      /* Print results */
	      printf("[%lu] CDS ADC Value: %lu, Voltage: %.3fV\r\n",
	             counter++, adc_value, voltage);

	      /* CDS 센서 값에 따른 조도 상태 출력 */
	      if (voltage < 0.5f)
	      {
	        printf("       Light Level: Very Dark\r\n");
	      }
	      else if (voltage < 1.0f)
	      {
	        printf("       Light Level: Dark\r\n");
	      }
	      else if (voltage < 2.0f)
	      {
	        printf("       Light Level: Medium\r\n");
	      }
	      else if (voltage < 2.5f)
	      {
	        printf("       Light Level: Bright\r\n");
	      }
	      else
	      {
	        printf("       Light Level: Very Bright\r\n");
	      }
	      printf("\r\n");
	    }

	    /* Stop ADC */
	    HAL_ADC_Stop(&hadc1);

	    /* Wait 500ms before next reading */
	    HAL_Delay(500);
	  }
	  /* USER CODE END 3 */
```



