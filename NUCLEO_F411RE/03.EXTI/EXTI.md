# EXTI
   * 파란색 버튼을 외부 인터럽트로 받아서 LD2를 켜는 인터럽트를 구현한다.

<img width="600" height="400" alt="006" src="https://github.com/user-attachments/assets/69b890b7-fd5f-434b-9873-ee33e767ceee" />
<br>
<img width="300" height="400" alt="001" src="https://github.com/user-attachments/assets/5fc31358-5680-420e-bb2f-727463e31690" />
<br>
<img width="300" height="100" alt="002" src="https://github.com/user-attachments/assets/2cbb68fb-a2d9-41f3-aecd-88f27539ef59" />
<br>
<img width="600" height="400" alt="003" src="https://github.com/user-attachments/assets/63d42d2a-2a20-4e94-af06-e52b40e99225" />
<br>
<img width="600" height="400" alt="004" src="https://github.com/user-attachments/assets/76b47122-e98e-41dc-bf3b-2e92aec4f605" />
<br>
<img width="600" height="400" alt="005" src="https://github.com/user-attachments/assets/e9ffa37c-ec27-4d9c-812a-e9f6a4b83bcb" />
<br>

```c
/* USER CODE BEGIN 0 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch(GPIO_Pin)
	{
		case B1_Pin:
			HAL_GPIO_TogglePin(LD2_GPIO_Port,LD2_Pin);
			break;
		default:
			;
	}
}
/* USER CODE END 0 */
```
