# DHT11 : 온도 습도 센서

   * PA0 - DATA

<img width="659" height="438" alt="001" src="https://github.com/user-attachments/assets/76e6ebc7-e043-4079-9fb6-d45c23d8e00a" />

<img width="671" height="569" alt="nucleo-f411re-pinout" src="https://github.com/user-attachments/assets/bfc2b219-e041-4731-8637-28916b6bb6f1" />

<img width="800" height="600" alt="007" src="https://github.com/user-attachments/assets/333cfa3d-aed2-4a82-8376-20237697436b" />


```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */
```

```c
/* USER CODE BEGIN PTD */
typedef struct {
    uint8_t temperature;
    uint8_t humidity;
    uint8_t temp_decimal;
    uint8_t hum_decimal;
    uint8_t checksum;
} DHT11_Data;
/* USER CODE END PTD */
```

```c
/* USER CODE BEGIN PD */
#define DHT11_PORT GPIOA
#define DHT11_PIN GPIO_PIN_0
/* USER CODE END PD */
```

```c
/* USER CODE BEGIN PV */
DHT11_Data dht11_data;
char uart_buffer[100];  // uart_buffer 변수 선언 추가
/* USER CODE END PV */
```

```c
/* USER CODE BEGIN PFP */
void DHT11_SetPinOutput(void);
void DHT11_SetPinInput(void);
void DHT11_SetPin(GPIO_PinState state);
GPIO_PinState DHT11_ReadPin(void);
void DHT11_DelayUs(uint32_t us);
uint8_t DHT11_Start(void);
uint8_t DHT11_ReadBit(void);
uint8_t DHT11_ReadByte(void);
uint8_t DHT11_ReadData(DHT11_Data *data);
/* USER CODE END PFP */
```

```c
/* USER CODE BEGIN 0 */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

PUTCHAR_PROTOTYPE
{
  if (ch == '\n')
    HAL_UART_Transmit (&huart2, (uint8_t*) "\r", 1, 0xFFFF);
  HAL_UART_Transmit (&huart2, (uint8_t*) &ch, 1, 0xFFFF);
  return ch;
}

// DHT11 함수 구현 (수정된 버전)
void DHT11_SetPinOutput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

void DHT11_SetPinInput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;  // 내부 풀업 저항 사용
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

void DHT11_SetPin(GPIO_PinState state) {
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, state);
}

GPIO_PinState DHT11_ReadPin(void) {
    return HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN);
}

void DHT11_DelayUs(uint32_t us) {
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    while (__HAL_TIM_GET_COUNTER(&htim2) < us);
}

// 수정된 DHT11_Start() 함수
uint8_t DHT11_Start(void) {
    uint32_t timeout;

    // 1. 출력 모드로 설정하고 HIGH로 초기화
    DHT11_SetPinOutput();
    DHT11_SetPin(GPIO_PIN_SET);
    HAL_Delay(1);  // 안정화 시간

    // 2. 시작 신호 전송 (최소 18ms LOW)
    DHT11_SetPin(GPIO_PIN_RESET);
    HAL_Delay(20);  // 20ms LOW

    // 3. 20-40us HIGH 신호
    DHT11_SetPin(GPIO_PIN_SET);
    DHT11_DelayUs(30);

    // 4. 입력 모드로 변경
    DHT11_SetPinInput();

    // 5. DHT11 응답 대기 (80us LOW)
    timeout = 1000;  // 타임아웃 증가
    while (DHT11_ReadPin() && timeout--) {
        DHT11_DelayUs(1);
    }
    if (timeout == 0) {
        sprintf(uart_buffer, "DHT11 Start: No LOW response\r\n");
        HAL_UART_Transmit(&huart2, (uint8_t*)uart_buffer, strlen(uart_buffer), HAL_MAX_DELAY);
        return 0;
    }

    // 6. 80us HIGH 대기
    timeout = 1000;
    while (!(DHT11_ReadPin()) && timeout--) {
        DHT11_DelayUs(1);
    }
    if (timeout == 0) {
        sprintf(uart_buffer, "DHT11 Start: No HIGH response\r\n");
        HAL_UART_Transmit(&huart2, (uint8_t*)uart_buffer, strlen(uart_buffer), HAL_MAX_DELAY);
        return 0;
    }

    // 7. HIGH가 끝날 때까지 대기
    timeout = 1000;
    while (DHT11_ReadPin() && timeout--) {
        DHT11_DelayUs(1);
    }

    return 1;  // 성공
}

// 수정된 DHT11_ReadBit() 함수
uint8_t DHT11_ReadBit(void) {
    uint32_t timeout;

    // 1. LOW 신호가 끝날 때까지 대기 (약 50us)
    timeout = 1000;
    while (!(DHT11_ReadPin()) && timeout--) {
        DHT11_DelayUs(1);
    }
    if (timeout == 0) return 0;

    // 2. HIGH 신호 지속 시간 측정
    DHT11_DelayUs(30);  // 30us 후에 확인
    uint8_t bit = DHT11_ReadPin();

    // 3. HIGH가 끝날 때까지 대기
    timeout = 1000;
    while (DHT11_ReadPin() && timeout--) {
        DHT11_DelayUs(1);
    }

    return bit;
}

uint8_t DHT11_ReadByte(void) {
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
        byte = (byte << 1) | DHT11_ReadBit();
    }
    return byte;
}

uint8_t DHT11_ReadData(DHT11_Data *data) {
    // DHT11 시작 신호 전송
    if (!DHT11_Start()) {
        return 0; // 시작 신호 실패
    }

    // 5바이트 데이터 읽기
    data->humidity = DHT11_ReadByte();
    data->hum_decimal = DHT11_ReadByte();
    data->temperature = DHT11_ReadByte();
    data->temp_decimal = DHT11_ReadByte();
    data->checksum = DHT11_ReadByte();

    // 디버그 출력
    sprintf(uart_buffer, "Raw data: %d, %d, %d, %d, %d\r\n",
            data->humidity, data->hum_decimal, data->temperature, data->temp_decimal, data->checksum);
    HAL_UART_Transmit(&huart2, (uint8_t*)uart_buffer, strlen(uart_buffer), HAL_MAX_DELAY);

    // 체크섬 확인
    uint8_t calculated_checksum = data->humidity + data->hum_decimal +
                                 data->temperature + data->temp_decimal;

    if (calculated_checksum == data->checksum) {
        return 1; // 성공
    } else {
        sprintf(uart_buffer, "Checksum error: calc=%d, recv=%d\r\n",
                calculated_checksum, data->checksum);
        HAL_UART_Transmit(&huart2, (uint8_t*)uart_buffer, strlen(uart_buffer), HAL_MAX_DELAY);
        return 0; // 체크섬 오류
    }
}
/* USER CODE END 0 */
```

```c
  /* USER CODE BEGIN 2 */
  // DHT11 핀 초기 설정
  DHT11_SetPinOutput();
  DHT11_SetPin(GPIO_PIN_SET);  // 초기에 HIGH로 설정
  HAL_Delay(1000);  // DHT11 안정화 시간


  // 타이머 시작 (마이크로초 단위 지연용)
  HAL_TIM_Base_Start(&htim2);

  // UART 초기화 메시지
  sprintf(uart_buffer, "DHT11 Temperature & Humidity Sensor Test\r\n");
  HAL_UART_Transmit(&huart2, (uint8_t*)uart_buffer, strlen(uart_buffer), HAL_MAX_DELAY);

  /* USER CODE END 2 */
```

```c
    /* USER CODE BEGIN 3 */

	    if (DHT11_ReadData(&dht11_data)) {
	      // 데이터 읽기 성공
	      sprintf(uart_buffer, "Temperature: %d°C, Humidity: %d%%\r\n",
	              dht11_data.temperature, dht11_data.humidity);
	      HAL_UART_Transmit(&huart2, (uint8_t*)uart_buffer, strlen(uart_buffer), HAL_MAX_DELAY);
	    } else {
	      // 데이터 읽기 실패
	      sprintf(uart_buffer, "DHT11 Read Error!\r\n");
	      HAL_UART_Transmit(&huart2, (uint8_t*)uart_buffer, strlen(uart_buffer), HAL_MAX_DELAY);
	    }

	    // 2초 대기 (DHT11은 최소 2초 간격으로 읽어야 함)
	    HAL_Delay(2000);

	  }
  /* USER CODE END 3 */
```
