# GitHub Markdown Sample

<img width="1392" height="908" alt="TIM_001" src="https://github.com/user-attachments/assets/7ce97fe1-b422-4790-aada-bf41b0a9cf8a" />
<br>
<img width="1392" height="908" alt="TIM_002" src="https://github.com/user-attachments/assets/f6bb2bde-8fd6-476e-93c1-31aad5b148ca" />
<br>
<img width="1392" height="908" alt="TIM_003" src="https://github.com/user-attachments/assets/5468bc02-8ef1-4f53-a35f-dfdf7520b1a0" />
<br>
<img width="1137" height="545" alt="TIM_004" src="https://github.com/user-attachments/assets/c44882ac-41c4-4c04-bb83-d1bbedad22bd" />
<br>
<img width="1137" height="545" alt="TIM_005" src="https://github.com/user-attachments/assets/f228cfd8-2229-4014-8a2d-340ffb4845e4" />
<br>
<img width="1137" height="545" alt="TIM_006" src="https://github.com/user-attachments/assets/f0b0a488-12b7-4dc6-8f71-bbb27e9cbaaa" />
<br>

```c
/* USER CODE BEGIN PV */
volatile int gTimerCnt;
/* USER CODE END PV */
```

```c
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  if(HAL_TIM_Base_Start_IT(&htim3) != HAL_OK)
  {
  	  Error_Handler();
  }
  /* USER CODE END 2 */
```

```c
/* USER CODE BEGIN 0 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	gTimerCnt++;
	if(gTimerCnt == 1000)
	{
		gTimerCnt = 0;
		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
	}
}
/* USER CODE END 0 */

```

