# IR 리모컨 수신부 Projects for STM32F103




```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */
```


```c
/* USER CODE BEGIN PV */
// IR 수신 관련 변수
uint32_t last_tick = 0;
uint32_t diff_tick = 0;
uint32_t ir_data = 0;
uint8_t bit_count = 0;
uint8_t receive_flag = 0;
char msg[100];

// TIM3 핸들러 (CubeMX가 생성한 변수와 일치해야 함)
extern TIM_HandleTypeDef htim3;
/* USER CODE END PV */
```

```c
  /* USER CODE BEGIN 2 */
    // 1. 타이머 시작 (1us 단위로 카운팅되도록 설정되어 있어야 함)
    HAL_TIM_Base_Start(&htim3);

    HAL_UART_Transmit(&huart2, (uint8_t*)"--- IR Receiver Ready ---\r\n", 27, 100);
  /* USER CODE END 2 */
```

```c
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if (receive_flag)
	      {
	        // NEC 프로토콜: [Address(8)] [~Address(8)] [Command(8)] [~Command(8)]
	        uint8_t address = (ir_data >> 24) & 0xFF;
	        uint8_t command = (ir_data >> 8) & 0xFF;

	        sprintf(msg, "Raw: 0x%08X | Addr: 0x%02X | Cmd: 0x%02X\r\n",
	                (unsigned int)ir_data, address, command);
	        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);

			switch(command) {
				case 0x88:
					printf("1번 버튼을 눌렀습니다.\r\n");
					// HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET); // LED ON 예시
					break;
				case 0x48:
					printf("2번 버튼을 눌렀습니다.\r\n");
					break;
				// ... 나머지 버튼들 추가
				case 0x98:
					printf("9번 버튼을 눌렀습니다.\r\n");
					break;
				default:
					printf("알 수 없는 버튼: 0x%02X\r\n", command);
					break;
			}

	        // 데이터 초기화
	        receive_flag = 0;
	        ir_data = 0;
	        bit_count = 0;
	      }
    /* USER CODE END WHILE */
```

```c
/* USER CODE BEGIN 4 */
// 외부 인터럽트(GPIO EXTI) 콜백 함수
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    // PA0 핀 인터럽트 발생 시 (IR 센서 OUT 핀)
    if (GPIO_Pin == GPIO_PIN_0)
    {
        uint32_t current_tick = __HAL_TIM_GET_COUNTER(&htim3);

        // 시간 간격 계산 (타이머 카운트값 차이)
        if (current_tick >= last_tick) {
            diff_tick = current_tick - last_tick;
        } else {
            diff_tick = (0xFFFF - last_tick) + current_tick;
        }
        last_tick = current_tick;

        // NEC 프로토콜 판별 로직
        // 1. Lead Code (9ms High + 4.5ms Low ≈ 13.5ms)
        if (diff_tick > 13000 && diff_tick < 14000) {
            bit_count = 0;
            ir_data = 0;
        }
        // 2. Data Bit 판별 (Logic 0: 1.125ms, Logic 1: 2.25ms)
        else if (diff_tick > 1000 && diff_tick < 2500) {
            ir_data <<= 1;
            if (diff_tick > 1800) { // 2.25ms 근처면 Logic 1
                ir_data |= 1;
            } else {                // 1.125ms 근처면 Logic 0
                ir_data |= 0;
            }
            bit_count++;
        }

        // 32비트(주소 16 + 명령 16) 수신 완료 시
        if (bit_count == 32) {
            receive_flag = 1;
        }
    }
}
/* USER CODE END 4 */
```


### 실행결과

* 데이터 형식을 보면 20DF(주소와 주소 반전)는 고정되어 있고, Cmd 부분과 그 뒤의 ~Cmd 부분이 변하고 있습니다.
  * Address: 0x20 (반전된 값 0xDF와 합쳐져 0x20DF 형성)
  * Command: 각 버튼의 고유값입니다.

```
--- IR Receiver Ready ---
Raw: 0x20DF8877 | Addr: 0x20 | Cmd: 0x88
Raw: 0x20DF48B7 | Addr: 0x20 | Cmd: 0x48
Raw: 0x20DFC837 | Addr: 0x20 | Cmd: 0xC8
Raw: 0x20DF28D7 | Addr: 0x20 | Cmd: 0x28
Raw: 0x20DFA857 | Addr: 0x20 | Cmd: 0xA8
Raw: 0x20DF6897 | Addr: 0x20 | Cmd: 0x68
Raw: 0x20DFE817 | Addr: 0x20 | Cmd: 0xE8
Raw: 0x20DF18E7 | Addr: 0x20 | Cmd: 0x18
Raw: 0x20DF9867 | Addr: 0x20 | Cmd: 0x98
```

| 버튼	| Raw Data (32bit)	| Command (8bit)	| 비고 | 
|:---:|:---:|:---:|:---:|
| 1	| 0x20DF8877	| 0x88	|  | 
| 2	| 0x20DF48B7	| 0x48	| | 
| 3	| 0x20DFC837	| 0xC8	| | 
| 4	| 0x20DF28D7	| 0x28	| | 
| 5	| 0x20DFA857	| 0xA8	| | 
| 6	| 0x20DF6897	| 0x68	| | 
| 7	| 0x20DFE817	| 0xE8	| | 
| 8	| 0x20DF18E7	| 0x18	| | 
| 9	| 0x20DF9867	| 0x98  |  | 


