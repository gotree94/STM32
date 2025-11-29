# 24.L298N_NEMA17

<img width="671" height="569" alt="nucleo-f411re-pinout" src="https://github.com/user-attachments/assets/c2b1cc58-ef59-46ea-8436-30d41cc2303a" />

## 1. 프로젝트 목표

   * 보드: NUCLEO-STM32F411RE
   * 드라이버: L298N 모듈
   * 모터: NEMA17 17HS3430 (1.8°/step, 200 step/rev, 4-선 bipolar, 17HS3430) (makershop.co.nz)
   * 기능:
      * 정/역방향 회전
      * 일정 속도로 1회전(또는 원하는 step 수) 구동
      * 속도는 코드에서 step_delay_ms로 쉽게 조절
   ※ 주의: 17HS3430은 정격 전류 ≈ 1.2 A/phase, 권선 저항 ≈ 1.9 Ω 수준으로, 권선 정격 전압은 약 2.3 V입니다.
      (makershop.co.nz)

   * L298N은 전류제한 기능이 없는 단순 H-브릿지라서 장시간 고속/고토크 구동에는 적합하지 않고, 저속·짧은 시간 테스트용으로 쓰는 게 안전합니다. \
   * (실사용은 A4988/TMC 계열 전류제어 드라이버 권장)


<img width="450" height="300" alt="1278835" src="https://github.com/user-attachments/assets/941b6745-785e-4424-b2bf-f61838438c17" />
<img width="450" height="300" alt="img" src="https://github.com/user-attachments/assets/aee2114e-fcc7-4dd0-a459-f0a9e98015dd" />
<br>
<img width="500" height="218" alt="17HS3430-Stepper-motor" src="https://github.com/user-attachments/assets/7ce92906-b200-4607-9f1a-201ffe36899b" />
<br>

## 2. 하드웨어 구성
### 2-1. 전원 구성
   * NUCLEO-F411RE
      * USB(PC)로 5V 공급
      * IO는 모두 3.3V (BUT, L298N 입력은 TTL 호환이라 3.3V HIGH 인식 가능)
   * L298N 모듈
      * +12V (또는 6~9V 정도의 외부 전원, 처음에는 6~9V 권장)
      * GND → NUCLEO GND와 반드시 공통 접지
      * 5V-EN 점퍼가 있으면:
         * 모터 전원에서 레귤레이터로 5V 만들어서 사용 → 보통 Nucleo에는 연결하지 말고, L298N 모듈 내에서만 사용
      * EN 핀(ENA, ENB) 은 점퍼로 HIGH 고정하거나, 나중에 PWM 쓰려면 Nucleo 핀에 연결

### 2-2. 스테퍼 모터 → L298N 연결

   * 17HS3430은 4-선 bipolar 스테퍼입니다.
   * 두 가닥씩 저항이 연결되는 쌍이 각 코일입니다.
   * (일반적으로 3D 프린터용 17HS3430은 Black/Green, Red/Blue가 한 쌍이지만, 제조사마다 색 다를 수 있으니 반드시 테스터로 확인!)

   1. 테스터(저항 측정)로 쌍 찾기:
      * 네 선 중 저항 값이 나오는 두 선이 코일 A
      * 나머지 두 선이 코일 B
   2. L298N 연결 (예시):
      * 코일 A → OUT1, OUT2
      * 코일 B → OUT3, OUT4
      * (OUT1~4 중 어느 쪽이 +, − 인지는 방향만 바뀌는 문제라 치명적이진 않습니다.)

### 2-3. NUCLEO-F411RE → L298N 입력 연결

   * Nucleo의 Arduino 커넥터(D2~D7) 핀매핑은 UM1724에 정리되어 있습니다.

   * 여기서는 디지털 D2~D5를 모터 제어용으로 사용하겠습니다.

|기능		|L298N 핀	|Nucleo Arduino 핀	|MCU 핀	|비고|
|:--:|:--:|:--:|:--:|:--:|
|IN1		|IN1		|D2					|PA10	|코일A 방향1|
|IN2		|IN2		|D3					|PB3	|코일A 방향2|
|IN3		|IN3		|D4					|PB5	|코일B 방향1|
|IN4		|IN4		|D5					|PB4	|코일B 방향2|
|ENA(1,2)	|ENA		|점퍼로 5V 고정		|-		|나중에 PWM 쓰면 연결|
|ENB(3,4)	|ENB		|점퍼로 5V 고정		|-		|동일|
|GND		|GND		|GND				|-		|공통 GND 필수|

   * 이렇게 연결하면, PA10/PB3/PB5/PB4를 4-비트 패턴으로 토글하면서 풀스텝 구동할 수 있습니다.

## 3. STM32CubeIDE / CubeMX 설정

   1. New STM32 Project
      * Board Selector에서 NUCLEO-F411RE 선택
   2. Clock은 기본값(내부 HSI + PLL) 그대로 사용해도 무방 (100 MHz 근처)
   3. SYS
      * Debug: Serial Wire (기본)
   4. GPIO 설정
      * PA10 (D2) → GPIO_Output, Push-Pull, No Pull
      * PB3 (D3) → GPIO_Output, Push-Pull, No Pull
      * PB5 (D4) → GPIO_Output, Push-Pull, No Pull
      * PB4 (D5) → GPIO_Output, Push-Pull, No Pull
   5. Project Manager에서 프로젝트 이름 예:
      * F411_NEMA17_L298N
   6. 코드 생성(Generate Code)
      * 이제 Core/Src/main.c 안에 스테퍼 구동 코드를 추가합니다.

## 4. 스테퍼 구동 코드 예제 (풀스텝, 정/역방향)

   * 아래 코드는 CubeMX가 생성한 main.c 에 추가하는 형태입니다.
   * (특히 MX_GPIO_Init()은 CubeMX가 만들어준 그대로 두고, 그 위/아래에 함수만 추가)

### 4-1. pin 매크로 정의 (main.c 상단 부분)

```c
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

// L298N 제어용 핀 매핑 (D2~D5)
#define IN1_GPIO_Port GPIOA
#define IN1_Pin       GPIO_PIN_10   // D2

#define IN2_GPIO_Port GPIOB
#define IN2_Pin       GPIO_PIN_3    // D3

#define IN3_GPIO_Port GPIOB
#define IN3_Pin       GPIO_PIN_5    // D4

#define IN4_GPIO_Port GPIOB
#define IN4_Pin       GPIO_PIN_4    // D5

// 방향 정의
#define DIR_CW   1   // 시계방향
#define DIR_CCW  0   // 반시계방향

/* USER CODE END 0 */
```

   * 만약 CubeMX에서 IN1_Pin, IN1_GPIO_Port 같은 매크로를 이미 생성하게 설정했다면, 위 define는 빼고 CubeMX가 만든 이름을 그대로 쓰셔도 됩니다.

### 4-2. 한 스텝씩 진행하는 함수들

```c
// 풀스텝용 패턴 (IN1, IN2, IN3, IN4)
// OUT1=A+, OUT2=A-, OUT3=B+, OUT4=B- 가정
static const uint8_t STEP_SEQ_FULL[4][4] = {
  {1, 0, 1, 0},  // 스텝 0
  {0, 1, 1, 0},  // 스텝 1
  {0, 1, 0, 1},  // 스텝 2
  {1, 0, 0, 1}   // 스텝 3
};

// 코일 출력을 실제 GPIO에 반영
static void stepper_write_coils(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4)
{
  HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, in1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, in2 ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, in3 ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, in4 ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// index 위치의 풀스텝 패턴 적용
static void stepper_full_step(uint8_t index)
{
  uint8_t i = index & 0x03; // 0~3 만큼만 사용
  stepper_write_coils(
      STEP_SEQ_FULL[i][0],
      STEP_SEQ_FULL[i][1],
      STEP_SEQ_FULL[i][2],
      STEP_SEQ_FULL[i][3]
  );
}

// N 스텝 회전 (dir: DIR_CW / DIR_CCW, step_delay_ms: 스텝 사이 딜레이)
void stepper_rotate_steps(uint32_t steps, uint8_t dir, uint32_t step_delay_ms)
{
  int8_t step_idx = 0;

  for (uint32_t s = 0; s < steps; s++)
  {
    stepper_full_step(step_idx);

    if (dir == DIR_CW)
      step_idx++;
    else
      step_idx--;

    if (step_idx > 3) step_idx = 0;
    if (step_idx < 0) step_idx = 3;

    HAL_Delay(step_delay_ms);
  }

  // 스텝 완료 후 전류를 빼고 싶으면 모두 LOW
  stepper_write_coils(0, 0, 0, 0);
}
```

### 4-3. main() 루프 예제

```c
int main(void)
{
  /* MCU Configuration---------------------------------------------*/

  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  /* USER CODE BEGIN 2 */
  // 초기에는 코일 OFF
  stepper_write_coils(0, 0, 0, 0);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // 1회전 = 200 스텝 (1.8도/스텝 기준)
    uint32_t one_rev_steps = 200;

    // 시계방향 1회전 (속도: step당 5ms → 약 10 rev/min)
    stepper_rotate_steps(one_rev_steps, DIR_CW, 5);
    HAL_Delay(1000);

    // 반시계방향 1회전
    stepper_rotate_steps(one_rev_steps, DIR_CCW, 5);
    HAL_Delay(1000);

    // 속도를 더 빠르게 하고 싶으면 step_delay_ms를 줄이면 됩니다.
  }
  /* USER CODE END WHILE */
}
```

### 4-4. GPIO 초기화 확인 (MX_GPIO_Init)

   * CubeMX에서 설정했다면 대략 이런 코드가 들어있을 겁니다.
   * (없거나 다르면 PA10/PB3/PB4/PB5가 Output으로 설정되었는지만 확인)

```c
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pins : PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB3 PB4 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}
```

## 5. 속도 / 방향 / 고급 기능 확장 아이디어

지금 프로젝트는 정해진 속도 · 정/역 한 번씩 도는 기본 구조입니다.
조금만 확장하면:

User Button (PC13) 으로 방향 토글

TIM2 / TIM3 Interrupt 로 step 타이머 구성 → HAL_Delay 없이 non-blocking 스텝 구동

UART 명령(예: “S1000 D1 R200”)으로 속도/방향/스텝 수 제어

PWM으로 ENA/ENB 제어 → 간단한 전류/토크 제어 흉내 (duty 조절)

6. 전류/발열 관련 꼭 체크

17HS3430의 정격 전류/저항을 보면, 사실상 **저전압+전류제어 드라이버(A4988, DRV8825, TMC 계열)**를 쓰는 게 정석입니다.
makershop.co.nz
+1

L298N으로 장시간 고토크 구동하면:

모터 과열

L298N 자체 발열 심함

처음에는

공급 전압을 6~9V 정도로 낮게,

duty(혹은 step 주기)를 크게 해서

모터가 많이 뜨거워지지 않는지 손으로 자주 확인하면서 테스트하는 걸 추천합니다.
