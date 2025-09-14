# HC-SR04

<img width="600" height="480" alt="shield-001" src="https://github.com/user-attachments/assets/4c7f5dc6-ffe6-4f62-bcb1-376dc55e13a9" />
<br>
<img width="600" height="480" alt="shield-002" src="https://github.com/user-attachments/assets/e67d445b-c3b0-483c-92b6-4100a39a662e" />
<br>
<img width="600" height="480" alt="shield-002" src="https://github.com/user-attachments/assets/48183bb9-3a11-42a8-9ab9-c07975e4e6f8" />
<br><br>

<img width="600" height="520" alt="ultrasonic_001" src="https://github.com/user-attachments/assets/1827803d-4843-4b12-a703-df4b046097b6" />
<br>
<img width="600" height="450" alt="ultrasonic_002" src="https://github.com/user-attachments/assets/4fcc69d9-bb62-4270-856b-af036091733e" />
<br>
<img width="600" height="450" alt="ultrasonic_003" src="https://github.com/user-attachments/assets/3d8242ce-dfee-400f-bc4e-84e069c0e1b2" />
<br>
<img width="600" height="450" alt="ultrasonic_004" src="https://github.com/user-attachments/assets/878c5a3f-8f04-4ad9-b35d-cde7ff035420" />
<br>

<img width="1392" height="908" alt="hc_sr40_001" src="https://github.com/user-attachments/assets/0957fd23-ceb9-459e-9d91-7be4f3750c26" />
<br>
<img width="1392" height="908" alt="hc_sr40_002" src="https://github.com/user-attachments/assets/8e359bc7-de92-41ec-9859-8f212bbca60f" />
<br>
<img width="995" height="550" alt="hc_sr40_003" src="https://github.com/user-attachments/assets/43f8cc02-e8e5-496e-8031-85a921506eee" />
<br>

```c
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */
```

```c
/* USER CODE BEGIN PD */
#define HIGH 1
#define LOW 0
long unsigned int echo_time;
int dist;
/* USER CODE END PD */
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

void timer_start(void)
{
   HAL_TIM_Base_Start(&htim1);
}

void delay_us(uint16_t us)
{
   __HAL_TIM_SET_COUNTER(&htim1, 0); // initislize counter to start from 0
   while((__HAL_TIM_GET_COUNTER(&htim1))<us); // wait count until us
}

void trig(void)
{
   HAL_GPIO_WritePin(TRIG1_GPIO_Port, TRIG1_Pin, HIGH);
   delay_us(10);
   HAL_GPIO_WritePin(TRIG1_GPIO_Port, TRIG1_Pin, LOW);
}

/**
* @brief echo 신호가 HIGH를 유지하는 시간을 (㎲)단위로 측정하여 리턴한다.
* @param no param(void)
*/
long unsigned int echo(void)
{
   long unsigned int echo = 0;

   while(HAL_GPIO_ReadPin(ECHO1_GPIO_Port, ECHO1_Pin) == LOW){}
        __HAL_TIM_SET_COUNTER(&htim1, 0);
        while(HAL_GPIO_ReadPin(ECHO1_GPIO_Port, ECHO1_Pin) == HIGH);
        echo = __HAL_TIM_GET_COUNTER(&htim1);
   if( echo >= 240 && echo <= 23000 ) return echo;
   else return 0;
}
```

```c
  /* USER CODE BEGIN 2 */
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
```


```c
// 타임아웃 정의 (마이크로초 단위)
#define ECHO_TIMEOUT_US 30000    // 30ms 타임아웃 (약 5m 거리에 해당)
#define TRIG_PULSE_US 10         // 트리거 펄스 폭

/**
 * @brief 초음파 센서 트리거 신호 발생
 * @param none
 */
void trig(void)
{
    // 트리거 신호 전 안정화를 위한 LOW 상태 확보
    HAL_GPIO_WritePin(TRIG1_GPIO_Port, TRIG1_Pin, GPIO_PIN_RESET);
    delay_us(2);  // 2us 대기로 안정화
    
    // 10us HIGH 펄스 발생
    HAL_GPIO_WritePin(TRIG1_GPIO_Port, TRIG1_Pin, GPIO_PIN_SET);
    delay_us(TRIG_PULSE_US);
    HAL_GPIO_WritePin(TRIG1_GPIO_Port, TRIG1_Pin, GPIO_PIN_RESET);
}

/**
 * @brief echo 신호가 HIGH를 유지하는 시간을 (㎲)단위로 측정하여 리턴한다.
 * @param none
 * @return 측정된 에코 시간(us), 오류시 0 리턴
 */
long unsigned int echo(void)
{
    long unsigned int echo_time = 0;
    uint32_t timeout_counter = 0;
    
    // 타이머 초기화 및 시작 확인
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    
    // ECHO 핀이 LOW에서 HIGH로 변할 때까지 대기 (타임아웃 포함)
    timeout_counter = 0;
    while(HAL_GPIO_ReadPin(ECHO1_GPIO_Port, ECHO1_Pin) == GPIO_PIN_RESET)
    {
        delay_us(1);
        timeout_counter++;
        if(timeout_counter > ECHO_TIMEOUT_US)
        {
            return 0; // 타임아웃 발생
        }
    }
    
    // ECHO 신호가 HIGH가 되는 순간 타이머 카운터 리셋
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    
    // ECHO 핀이 HIGH에서 LOW로 변할 때까지 대기 (타임아웃 포함)
    timeout_counter = 0;
    while(HAL_GPIO_ReadPin(ECHO1_GPIO_Port, ECHO1_Pin) == GPIO_PIN_SET)
    {
        delay_us(1);
        timeout_counter++;
        if(timeout_counter > ECHO_TIMEOUT_US)
        {
            return 0; // 타임아웃 발생
        }
    }
    
    // ECHO 신호가 LOW가 되는 순간의 타이머 값 읽기
    echo_time = __HAL_TIM_GET_COUNTER(&htim1);
    
    // 유효한 범위 체크 (2cm ~ 400cm 정도)
    // HC-SR04 기준: 2cm=116us, 400cm=23200us
    if(echo_time >= 116 && echo_time <= 23200)
    {
        return echo_time;
    }
    else
    {
        return 0; // 유효 범위 벗어남
    }
}

/**
 * @brief 초음파 센서로 거리 측정 (전체 프로세스)
 * @param none
 * @return 거리값(cm), 오류시 0 리턴
 */
float measure_distance(void)
{
    long unsigned int echo_time = 0;
    float distance = 0;
    
    // 연속 측정간 간섭 방지를 위한 대기
    HAL_Delay(60); // 60ms 대기
    
    // 트리거 신호 발생
    trig();
    
    // 에코 시간 측정
    echo_time = echo();
    
    if(echo_time == 0)
    {
        return 0; // 측정 실패
    }
    
    // 거리 계산 (음속: 340m/s, 왕복거리 고려)
    // 거리(cm) = (에코시간(us) * 0.034) / 2
    distance = (float)echo_time * 0.017; // 0.034 / 2 = 0.017
    
    return distance;
}

// 사용 예시
void ultrasonic_test(void)
{
    float distance = 0;
    
    while(1)
    {
        distance = measure_distance();
        
        if(distance > 0)
        {
            printf("Distance: %.1f cm\n", distance);
        }
        else
        {
            printf("Measurement failed\n");
        }
        
        HAL_Delay(500); // 500ms마다 측정
    }
}
```
