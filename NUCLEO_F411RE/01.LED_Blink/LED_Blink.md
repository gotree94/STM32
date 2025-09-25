## 1. LED_Blink

<img width="520" height="350" alt="LED_Blinks_001" src="https://github.com/user-attachments/assets/3aa18d2c-5bc8-4d24-ae1f-898272f96bf9" />
<br>

<img width="1359" height="838" alt="007" src="https://github.com/user-attachments/assets/38c66e79-7a7f-43f4-89af-6c443d12fb36" />
<br>
<img width="492" height="558" alt="008" src="https://github.com/user-attachments/assets/8387c001-69e3-489a-af07-dcdc7c2ea3c1" />
<br>
<img width="1118" height="901" alt="009" src="https://github.com/user-attachments/assets/f50f1b35-9af7-41bb-bc3e-e063262d1f90" />
<br>
<img width="1118" height="901" alt="010" src="https://github.com/user-attachments/assets/87e1812b-1152-42f6-8b84-7d467bd87d77" />
<br>
<img width="492" height="558" alt="LED_Blinks_005" src="https://github.com/user-attachments/assets/3197a10b-59cd-43e6-baa2-4d5f46a0e8a1" />
<br>
<img width="486" height="129" alt="LED_Blinks_006" src="https://github.com/user-attachments/assets/4d967a98-f423-4f2f-ac96-e863f1115848" />
<br>
<img width="242" height="297" alt="LED_Blinks_011" src="https://github.com/user-attachments/assets/7bf0e956-97db-48b5-9856-d4a119fc7538" />
<br>
<img width="486" height="165" alt="LED_Blinks_012" src="https://github.com/user-attachments/assets/ca3e7a4a-8acc-4cfd-9905-e01ccbeb556c" />
<br>

<img width="1137" height="545" alt="LED_Blinks_013" src="https://github.com/user-attachments/assets/c4d741b9-39ba-41f5-9f5a-405e0d6c1157" />
<br>
<img width="1137" height="545" alt="LED_Blinks_014" src="https://github.com/user-attachments/assets/e0c09683-07a4-4a1b-8dd1-3e2f91cba259" />
<br>

* main.c
```c
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 1);
	  HAL_Delay(1000);
	  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);
	  HAL_Delay(1000);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
```

