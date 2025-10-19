# STM32F103 + ULN2003 + 28BYJ-48 스텝모터 제어 프로그램

<img width="300" height="200" alt="005" src="https://github.com/user-attachments/assets/a247809b-1f83-4a2b-807d-85373012917b" />
<br>

<img width="500" height="450" alt="F103RB-pin" src="https://github.com/user-attachments/assets/c908088b-cb5d-40cd-8302-1b04dc17bfd1" />

```
STM32F103    용도         ULN2003    28BYJ-48
---------    ----         -------    --------
PA0    -->   MOTOR_IN1 -> IN1        
PA1    -->   MOTOR_IN2 -> IN2        
PA4    -->   MOTOR_IN3 -> IN3        
PA6    -->   MOTOR_IN4 -> IN4        
PA2    -->   UART2_TX
PA3    -->   UART2_RX
PA5    -->   LED
GND    -->                GND   -->  -
5V     -->                VCC   -->  +
```

```
GPIO 설정

PA0, PA1, PA4, PA6: GPIO_Output (스텝모터)
PA5: GPIO_Output (LED)
PA2: USART2_TX
PA3: USART2_RX
```

```c
/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
// 스텝모터 연결 핀 (ULN2003 IN1~IN4)
// PA2, PA3는 UART2 (TX, RX)로 사용
// PA5는 LED로 사용
#define MOTOR_IN1_PIN GPIO_PIN_0
#define MOTOR_IN2_PIN GPIO_PIN_1
#define MOTOR_IN3_PIN GPIO_PIN_4
#define MOTOR_IN4_PIN GPIO_PIN_6
#define MOTOR_PORT GPIOA

// 스텝모터 제어 매크로
#define STEPS_PER_REVOLUTION 4096  // 한 바퀴 회전에 필요한 스텝 수
#define STEP_DELAY 2               // 스텝 간 딜레이 (ms)
/* USER CODE END PV */
```

```c
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void StepMotor_Init(void);
void StepMotor_Step(int step);
void StepMotor_Rotate(int steps, int direction);
void StepMotor_RotateDegrees(float degrees, int direction);

/* 스텝 시퀀스 - Half Step Mode (더 부드러운 동작) */
const uint8_t halfStepSequence[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};

/* 스텝 시퀀스 - Full Step Mode (더 강한 토크) */
const uint8_t fullStepSequence[4][4] = {
    {1, 0, 0, 1},
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 1}
};

int currentStep = 0;
/* USER CODE END PFP */
```

```c
/* USER CODE BEGIN 0 */
/**
  * @brief  스텝모터 초기화
  */
void StepMotor_Init(void)
{
    // 모든 코일 OFF
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_IN2_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_IN3_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_IN4_PIN, GPIO_PIN_RESET);

    currentStep = 0;
}

/**
  * @brief  한 스텝 실행 (Half Step Mode)
  * @param  step: 스텝 번호 (0~7)
  */
void StepMotor_Step(int step)
{
    step = step % 8;  // 0~7 범위로 제한

    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_IN1_PIN,
                      halfStepSequence[step][0] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_IN2_PIN,
                      halfStepSequence[step][1] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_IN3_PIN,
                      halfStepSequence[step][2] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_IN4_PIN,
                      halfStepSequence[step][3] ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
  * @brief  지정된 스텝 수만큼 회전
  * @param  steps: 회전할 스텝 수
  * @param  direction: 회전 방향 (1: 정방향, 0: 역방향)
  */
void StepMotor_Rotate(int steps, int direction)
{
    for (int i = 0; i < steps; i++)
    {
        if (direction == 1)
        {
            currentStep++;
            if (currentStep >= 8) currentStep = 0;
        }
        else
        {
            currentStep--;
            if (currentStep < 0) currentStep = 7;
        }

        StepMotor_Step(currentStep);
        HAL_Delay(STEP_DELAY);
    }

    // 회전 완료 후 모든 코일 OFF (전력 절약 및 발열 방지)
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_IN1_PIN | MOTOR_IN2_PIN |
                      MOTOR_IN3_PIN | MOTOR_IN4_PIN, GPIO_PIN_RESET);
}

/**
  * @brief  각도만큼 회전
  * @param  degrees: 회전할 각도
  * @param  direction: 회전 방향 (1: 정방향, 0: 역방향)
  */
void StepMotor_RotateDegrees(float degrees, int direction)
{
    int steps = (int)((degrees / 360.0) * STEPS_PER_REVOLUTION);
    StepMotor_Rotate(steps, direction);
}
/* USER CODE END 0 */
```

```c
  /* USER CODE BEGIN 2 */
  /* 스텝모터 초기화 */
  StepMotor_Init();
  /* USER CODE END 2 */
```

```c
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      /* 예제 1: 정방향 1회전 */
      StepMotor_Rotate(STEPS_PER_REVOLUTION, 1);
      HAL_Delay(1000);

      /* 예제 2: 역방향 1회전 */
      StepMotor_Rotate(STEPS_PER_REVOLUTION, 0);
      HAL_Delay(1000);

      /* 예제 3: 90도 회전 */
      StepMotor_RotateDegrees(90, 1);
      HAL_Delay(1000);

      /* 예제 4: 180도 역회전 */
      StepMotor_RotateDegrees(180, 0);
      HAL_Delay(1000);
    /* USER CODE END WHILE */
```


