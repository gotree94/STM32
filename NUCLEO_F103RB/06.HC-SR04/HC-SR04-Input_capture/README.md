# HC-SR04 Input Capture

![](Fig.NUCLEO-F103RB.png)



```c
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */
```

```c
/* USER CODE BEGIN PV */

/* Input capture state */
static volatile uint32_t ic_rising_tick = 0;
static volatile uint32_t ic_falling_tick = 0;
static volatile uint8_t  ic_measurement_done = 0;
static volatile uint8_t  ic_edge = 0;   /* 0=wait rising, 1=wait falling */
static volatile uint8_t  ic_overflow = 0; /* timeout via update event */

/* USER CODE END PV */
```

```c
/* USER CODE BEGIN PFP */
static void delay_us(uint32_t us);
static void HCSR04_Trigger(void);
/* USER CODE END PFP */
```

```c
  /* USER CODE BEGIN 2 */
  printf("HC-SR04 Input Capture initialized\r\n");
  printf("System Clock: %lu Hz\r\n", HAL_RCC_GetSysClockFreq());
  /* USER CODE END 2 */
```

```c
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    uint32_t pulse_width = 0;
    float distance_cm = 0.0f;

    /* Send trigger pulse */
    HCSR04_Trigger();

    /* Wait for measurement with timeout */
    uint32_t tick_start = HAL_GetTick();
    while (ic_measurement_done == 0)
    {
      if ((HAL_GetTick() - tick_start) > TIMEOUT_MS)
      {
        /* Timeout - no echo received */
        HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_3);
        ic_edge = 0;
        break;
      }
    }

    if (ic_measurement_done)
    {
      /* Calculate pulse width using hardware overflow flag */
      if (ic_overflow)
      {
        /* Timer overflowed between rising and falling edges */
        pulse_width = (IC_PERIOD - ic_rising_tick) + ic_falling_tick;
      }
      else
      {
        /* No overflow */
        pulse_width = ic_falling_tick - ic_rising_tick;
      }

      /* Convert to distance (58 us per cm at sea level) */
      distance_cm = (float)pulse_width / SOUND_SPEED_FACTOR;

      printf("Pulse: %lu us  Distance: %.1f cm\r\n", pulse_width, distance_cm);

      ic_measurement_done = 0;
      ic_overflow = 0;
    }
    else
    {
      printf("Timeout - no object detected\r\n");
    }

    HAL_Delay(200); /* Wait 200ms between measurements */
    /* USER CODE END WHILE */
```

```c
/* USER CODE BEGIN 4 */

/**
  * @brief  Retarget printf to UART2
  */
int __io_putchar(int ch)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

int __io_getchar(void)
{
  uint8_t ch;
  HAL_UART_Receive(&huart2, &ch, 1, HAL_MAX_DELAY);
  return ch;
}

/**
  * @brief  Microsecond delay using DWT cycle counter
  *         Uses Cortex-M3 DWT CYCCNT (independent of TIM3)
  *         SystemCoreClock = 64 MHz → 64 cycles = 1 us
  */
static void delay_us(uint32_t us)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  uint32_t start = DWT->CYCCNT;
  uint32_t cycles = us * (SystemCoreClock / 1000000UL);
  while ((DWT->CYCCNT - start) < cycles);
}

/**
  * @brief  Send 10us trigger pulse to HC-SR04 and start input capture
  */
static void HCSR04_Trigger(void)
{
  /* Reset state */
  ic_measurement_done = 0;
  ic_edge = 0;
  ic_overflow = 0;

  /* Send 10 us HIGH pulse on TRIG pin */
  HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);
  delay_us(TRIG_PULSE_US);
  HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);

  /* Start input capture on rising edge */
  __HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_RISING);
  HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_3);
}

/**
  * @brief  HAL TIM IC Capture Callback
  *         Called on each capture event (rising -> falling edge)
  */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM3)
  {
    if (ic_edge == 0)
    {
      /* Rising edge captured - save timestamp, switch to falling edge */
      ic_rising_tick = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_3);
      ic_edge = 1;
      __HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_FALLING);
    }
    else
    {
      /* Falling edge captured - save timestamp, stop capture */
      ic_falling_tick = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_3);
      HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_3);
      ic_measurement_done = 1;
    }
  }
}

/**
  * @brief  HAL TIM Period Elapsed Callback (overflow handler)
  *         Used to detect timeout when no echo is received
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM3)
  {
    ic_overflow = 1;
  }
}

/* USER CODE END 4 */
```



