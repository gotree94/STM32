# HC-SR04

<img width="600" height="480" alt="shield-001" src="https://github.com/user-attachments/assets/4c7f5dc6-ffe6-4f62-bcb1-376dc55e13a9" />
<br>
<img width="600" height="480" alt="shield-002" src="https://github.com/user-attachments/assets/e67d445b-c3b0-483c-92b6-4100a39a662e" />
<br>
<img width="600" height="480" alt="shield-002" src="https://github.com/user-attachments/assets/48183bb9-3a11-42a8-9ab9-c07975e4e6f8" />
<br><br>

<img width="400" height="200" alt="ultrasonic_001" src="https://github.com/user-attachments/assets/1827803d-4843-4b12-a703-df4b046097b6" />
<br>
<img width="500" height="320" alt="ultrasonic_002" src="https://github.com/user-attachments/assets/4fcc69d9-bb62-4270-856b-af036091733e" />
<br>
<img width="500" height="320" alt="ultrasonic_003" src="https://github.com/user-attachments/assets/3d8242ce-dfee-400f-bc4e-84e069c0e1b2" />
<br>
<img width="500" height="320" alt="ultrasonic_004" src="https://github.com/user-attachments/assets/878c5a3f-8f04-4ad9-b35d-cde7ff035420" />
<br>


```c
void timer_start(void)
{
   HAL_TIM_Base_Start(&htim1);
}
```

```c
void delay_us(uint16_t us)
{
   __HAL_TIM_SET_COUNTER(&htim1, 0); // initislize counter to start from 0
   while((__HAL_TIM_GET_COUNTER(&htim1))<us); // wait count until us
}
```

```c
    void trig(void)
   {
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, HIGH);
       delay_us(10);
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, LOW);
   }

   /**
    * @brief echo 신호가 HIGH를 유지하는 시간을 (㎲)단위로 측정하여 리턴한다.
    * @param no param(void)
    */
   long unsigned int echo(void)
   {
       long unsigned int echo = 0;

       while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == LOW){}
            __HAL_TIM_SET_COUNTER(&htim1, 0);
            while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == HIGH);
            echo = __HAL_TIM_GET_COUNTER(&htim1);
       if( echo >= 240 && echo <= 23000 ) return echo;
       else return 0;
   }
```

```c
timer_start();
printf("Ranging with HC-SR04\n");
/* USER CODE END 2 */

/* Infinite loop */
/* USER CODE BEGIN WHILE */
while (1)
{
    trig();
    echo_time = echo();
    if( echo_time != 0){
        dist = (int)(17 * echo_time / 100);
        printf("Distance = %d(mm)\n", dist);
    }
    else printf("Out of Range!\n");

/* USER CODE END WHILE */

/* USER CODE BEGIN 3 */
}
/* USER CODE END 3 */
}
```
