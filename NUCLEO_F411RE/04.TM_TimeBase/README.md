# TIM_TimeBase

<img width="300" height="400" alt="001" src="https://github.com/user-attachments/assets/b2ddf8ea-8a2b-48ba-b77d-82c09a8a13b2" />
<br>
<img width="300" height="100" alt="002" src="https://github.com/user-attachments/assets/ac4b11be-42d0-41fa-910a-657179a1973f" />
<br>
<img width="600" height="400" alt="003" src="https://github.com/user-attachments/assets/59355ab1-151f-4e63-a8e2-83a4582e335b" />
<br>
<img width="600" height="400" alt="004" src="https://github.com/user-attachments/assets/a1677d77-ec1a-425c-9d15-1cae8eb008be" />
<br>
<img width="600" height="400" alt="005" src="https://github.com/user-attachments/assets/f5525561-aee7-4bda-b833-d6050e9bce2f" />
<br>
<img width="600" height="400" alt="006" src="https://github.com/user-attachments/assets/31cddba0-27e3-4ecf-be7a-b5ebdeec7333" />
<br>
<img width="300" height="400" alt="007" src="https://github.com/user-attachments/assets/36ec5eb9-169e-46c3-a09b-6aedc323b5ac" />
<br>
<img width="300" height="100" alt="008" src="https://github.com/user-attachments/assets/effdb785-ad85-405e-a1c1-4c24b274fccd" />
<br>
<img width="600" height="400" alt="009" src="https://github.com/user-attachments/assets/f8b81fa4-1d81-4b2f-a367-3244d6c0bb41" />
<br>
<img width="600" height="400" alt="010" src="https://github.com/user-attachments/assets/c3edbe42-e548-4e12-b691-c8b8726541f4" />
<br>
<img width="600" height="400" alt="011" src="https://github.com/user-attachments/assets/a79355e4-dc81-4bde-999d-b71fbfcc9d5a" />
<br>
<img width="600" height="400" alt="012" src="https://github.com/user-attachments/assets/46e23573-19ad-4e3e-96c9-1062a9055d30" />
<br>
<img width="600" height="400" alt="013" src="https://github.com/user-attachments/assets/756cac3a-39b4-404f-b0f3-a4711256f49e" />
<br>
<img width="600" height="400" alt="014" src="https://github.com/user-attachments/assets/538a1182-692c-4337-9ef8-aa6fa425a1eb" />
<br>

```c
/* USER CODE BEGIN PV */
volatile int gTimerCnt;
/* USER CODE END PV */
```

```c
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

