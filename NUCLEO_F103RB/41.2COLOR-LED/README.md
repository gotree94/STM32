# 2 COLOR LED Projects for STM32F103


<img src="2color_led.jpg">

<img src="2color_001.png">  <br>
<img src="2color_002.png">  <br>
<img src="2color_003.png">  <br>
<img src="2color_004.png">  <br>


```c
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  // 1. LED1만 켜기 (LED2 끄기)
	      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
	      HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
	      HAL_Delay(1000); // 1초 대기

	      // 2. LED2만 켜기 (LED1 끄기)
	      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
	      HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
	      HAL_Delay(1000);

	      // 3. 둘 다 켜기 (혼합색)
	      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
	      HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
	      HAL_Delay(1000);

	      // 4. 둘 다 끄기
	      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
	      HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
	      HAL_Delay(1000);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
```
