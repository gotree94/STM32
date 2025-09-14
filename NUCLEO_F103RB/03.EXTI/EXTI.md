# EXTI


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
