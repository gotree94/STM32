# ADC_TemperatureSensor

<img width="500" height="300" alt="adc_004" src="https://github.com/user-attachments/assets/47beb889-817f-458e-aeca-9897a0c2a7de" />
<br>

<img width="300" height="400" alt="001" src="https://github.com/user-attachments/assets/7fbd13f5-1f60-40b9-911c-26835c6fc8a5" />
<br>
<img width="600" height="400" alt="002" src="https://github.com/user-attachments/assets/e655889c-7f55-4b13-8cb3-71ce6b86b7d9" />
<br>
<img width="600" height="400" alt="003" src="https://github.com/user-attachments/assets/e15a2b0b-f199-40f9-ad7b-54a21c836b61" />
<br>
<img width="600" height="400" alt="005" src="https://github.com/user-attachments/assets/20440010-d87a-4c48-8727-4b7ab530b8ea" />
<br>
<img width="600" height="400" alt="adc_003" src="https://github.com/user-attachments/assets/f54c002b-79d3-4335-96a8-7cbd80c9ae56" />
<br>

<img width="600" height="400" alt="adc_001" src="https://github.com/user-attachments/assets/cbf9a19a-582a-4086-b537-749032e283f5" />
<br>
<img width="600" height="400" alt="adc_002" src="https://github.com/user-attachments/assets/2b50bbf3-f688-4223-a208-be9745a2c25a" />
<br>


```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */
```


```c
/* USER CODE BEGIN PV */
// STM32F411 Temperature Sensor Calibration Values (from datasheet)
// STM32F411CE/RE Reference Manual - Section 13.10
const float AVG_SLOPE = 2.5E-03;    // V/°C (2.5 mV/°C typical)
const float V25 = 0.76;             // Voltage at 25°C (0.76V typical)
const float ADC_TO_VOLT = 3.3 / 4095.0;  // 12-bit ADC: 0~4095, VREF=3.3V

// 추가적인 오프셋 보정을 위한 상수들
const float TEMP_OFFSET = 0.0;      // 필요시 실측값과 비교하여 조정
/* USER CODE END PV */
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
/* USER CODE END 0 */
```

```c
	  /* USER CODE BEGIN 1 */
	  uint32_t adc_raw;
	  float vSense;
	  float temp_celsius;

	  // 여러 샘플의 평균을 위한 변수들
	  uint32_t adc_sum = 0;
	  const uint8_t num_samples = 10;
	  /* USER CODE END 1 */
```


```c
  /* USER CODE BEGIN 2 */
   printf("STM32F411 Internal Temperature Sensor\r\n");
   printf("=====================================\r\n");

   // *** 중요: STM32F4xx에서는 온도센서를 명시적으로 활성화해야 함 ***
   ADC->CCR |= ADC_CCR_TSVREFE;  // 온도센서와 VREFINT 활성화

   // ADC 시작
   if(HAL_ADC_Start(&hadc1) != HAL_OK)
   {
     Error_Handler();
   }

   // 온도센서 안정화를 위한 초기 지연 (중요!)
   HAL_Delay(100);
   /* USER CODE END 2 */
```

```c
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	   // 여러 샘플의 평균값 계산 (노이즈 감소)
	    adc_sum = 0;
	    uint8_t valid_samples = 0;

	    for(uint8_t i = 0; i < num_samples; i++)
	    {
	      // 새로운 변환 시작
	      HAL_ADC_Start(&hadc1);

	      // ADC 변환 완료 대기
	      if(HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
	      {
	        uint32_t adc_value = HAL_ADC_GetValue(&hadc1);
	        if(adc_value > 0)  // 유효한 값만 사용
	        {
	          adc_sum += adc_value;
	          valid_samples++;
	        }
	      }
	      HAL_ADC_Stop(&hadc1);
	      HAL_Delay(10);  // 샘플 간 작은 지연
	    }

	    if(valid_samples > 0)
	    {
	      adc_raw = adc_sum / valid_samples;

	      // 전압 계산
	      vSense = (float)adc_raw * ADC_TO_VOLT;

	      // 온도 계산 (STM32F411 공식 + 오프셋 보정)
	      temp_celsius = (V25 - vSense) / AVG_SLOPE + 25.0 + TEMP_OFFSET;

	      // 결과 출력
	      printf("ADC Raw: %lu (samples: %d), Voltage: %.3fV, Temperature: %.1f°C\r\n",
	             adc_raw, valid_samples, vSense, temp_celsius);
	    }
	    else
	    {
	      printf("Error: No valid ADC readings!\r\n");
	    }

	    HAL_Delay(1000);  // 1초 간격
	    /* USER CODE END WHILE */
```

<img width="600" height="400" alt="004" src="https://github.com/user-attachments/assets/d8725743-9ef4-4ccc-842c-0cc914bd09ac" />
<br>




