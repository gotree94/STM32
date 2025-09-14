# SG90 Servo 제어를 위한 Timer 설정 (STM32 예제)


<img width="644" height="586" alt="F103RB-pin" src="https://github.com/user-attachments/assets/23e365b4-1bdf-4074-9724-d795ea1da5b7" />
<br><br>

<img width="800" height="608" alt="SERVO_004" src="https://github.com/user-attachments/assets/f20ec8df-36e9-42e1-8671-6223fc108338" />
<br>
<img width="800" height="608" alt="SERVO_003" src="https://github.com/user-attachments/assets/a6ca154d-6616-407b-9e77-ab1566bb1a80" />
<br>

## 1. 기본 조건
- **타이머 클럭** = 64 MHz  
- **Prescaler** = 1280 - 1 = 1279  
- **Period** = 1000 - 1 = 999  

---

## 2. 타이머 카운트 주파수
$$
f_{timer} = \frac{64,000,000}{1280} = 50,000 \,\text{Hz}
$$

- 카운트 주파수 = **50 kHz**  
- Tick 주기:  
$$
\frac{1}{50,000} = 20 \,\mu s
$$

---

## 3. PWM 주기
$$
T_{PWM} = \frac{Period + 1}{f_{timer}} = \frac{1000}{50,000} = 0.02 \, s = 20 \, ms
$$

✅ 따라서 PWM 주기 = **20 ms (50 Hz)** → SG90 서보 요구사항과 일치  

---

## 4. 펄스 폭 (CCR 값으로 각도 제어)

- **1 ms** 펄스 폭  
$$\frac{1 \, \text{ms}}{20 \, \mu\text{s}} = 50 \quad \Rightarrow \quad \text{CCR} = 50$$

- **1.5 ms** 펄스 폭  
$$\frac{1.5 \, \text{ms}}{20 \, \mu\text{s}} = 75 \quad \Rightarrow \quad \text{CCR} = 75$$

- **2 ms** 펄스 폭  
$$\frac{2 \, \text{ms}}{20 \, \mu\text{s}} = 100 \quad \Rightarrow \quad \text{CCR} = 100$$

---

## 5. 요약
- Prescaler = **1279**, Period = **999** → 정확히 **50 Hz (20 ms)** PWM 생성  
- CCR 값 50 ~ 100 사이로 설정하여 SG90 서보 각도 (0°~180°) 제어 가능  

---

## 6. 각도별 CCR 값
- 0° → 1 ms → CCR = 50  
- 90° → 1.5 ms → CCR = 75  
- 180° → 2 ms → CCR = 100  

```c
// 0도
__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 50);

// 90도
__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 75);

// 180도
__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 100);
```

---

## 7. 각도를 일반화한 함수
```c
void SG90_SetAngle(uint8_t angle)
{
    // angle: 0 ~ 180도
    // CCR: 50(1ms) ~ 100(2ms)
    uint32_t ccr_val = 50 + ((angle * (100 - 50)) / 180);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, ccr_val);
}
```

---

## 8. 사용 예시
```c
SG90_SetAngle(0);    // 0도
HAL_Delay(1000);

SG90_SetAngle(90);   // 90도
HAL_Delay(1000);

SG90_SetAngle(180);  // 180도
HAL_Delay(1000);
```
----
# 코드 수정
----

   * TIM2_CH1 - PA0
   * TIM3_CH1 - PA6

```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */
```

```c
/* USER CODE BEGIN PD */
#define MAX 125  // 2.5ms pulse width (최대 각도)
#define MIN 25   // 0.5ms pulse width (최소 각도)
#define CENTER 75 // 1.5ms pulse width (중앙 각도)
#define STEP 5
/* USER CODE END PD */

/* USER CODE BEGIN PV */
uint8_t ch;
uint8_t pos_pan = 75;
uint8_t pos_tilt = 75;
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
```

```c
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

  // 초기 위치 설정
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pos_pan);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pos_tilt);

  printf("Servo Control Ready\r\n");
  printf("Commands: w(up), s(down), a(left), d(right), i(center)\r\n");
  /* USER CODE END 2 */
```

```c
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if(HAL_UART_Receive(&huart2, &ch, sizeof(ch), 10) == HAL_OK)
    {
      if(ch == 's')
      {
        printf("Down\r\n");
        if(pos_tilt + STEP <= MAX)
          pos_tilt = pos_tilt + STEP;
        else
          pos_tilt = MAX;
      }
      else if(ch == 'w')
      {
        printf("Up\r\n");
        if(pos_tilt - STEP >= MIN)
          pos_tilt = pos_tilt - STEP;
        else
          pos_tilt = MIN;
      }
      else if(ch == 'a')
      {
        printf("Left\r\n");
        if(pos_pan + STEP <= MAX)
          pos_pan = pos_pan + STEP;
        else
          pos_pan = MAX;
      }
      else if(ch == 'd')
      {
        printf("Right\r\n");
        if(pos_pan - STEP >= MIN)
          pos_pan = pos_pan - STEP;
        else
          pos_pan = MIN;
      }
      else if(ch == 'i')
      {
        printf("Center\r\n");
        pos_pan = CENTER;
        pos_tilt = CENTER;
      }

      // PWM 듀티 사이클 업데이트
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pos_pan);
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pos_tilt);

      printf("Pan: %d, Tilt: %d\r\n", pos_pan, pos_tilt);

      HAL_Delay(50); // 서보 응답 시간
    }

    /* USER CODE END WHILE */
```










