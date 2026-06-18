# STM CUbeIDE Code Structure

## Code

```c
/* USER CODE BEGIN Header */

/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */

  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
```

---

## 약어 설명
| 약어	| 풀네임	| 의미 | 
|:------:|:------:|:------:|
| PTD	| Private Type Definitions	| 사용자 정의 자료형(struct, enum, typedef 등) | 
| PD	| Private Defines	| 매크로 상수(#define) | 
| PM	| Private Macros	| 함수형 매크로 | 
| PV	| Private Variables	| 전역 변수(static 포함) | 
| PFP	| Private Function Prototypes	| 함수 원형 선언 | 

---

1. PTD (Private Type Definitions)
  * 사용자 정의 자료형을 선언하는 곳입니다.

```c
/* USER CODE BEGIN PTD */

typedef struct
{
uint16_t adc;
float temperature;
} SensorData_t;

typedef enum
{
IDLE,
RUN,
ERROR
} SystemState_t;

/* USER CODE END PTD */
```

2. PD (Private Defines)
  * 상수값이나 핀 번호 등을 정의합니다.

```c
/* USER CODE BEGIN PD */

#define LED_ON 1
#define LED_OFF 0

#define ADC_MAX 4095

/* USER CODE END PD */
```

3. PM (Private Macros)
   * 매크로 함수를 정의합니다.

```c
/* USER CODE BEGIN PM */

#define ABS(x) ((x)>0?(x):-(x))
#define SET_BIT(REG,BIT) ((REG)|=(BIT))

/* USER CODE END PM */
```
   * 실제 함수보다 실행 속도가 빠른 간단한 연산에 사용됩니다.

4. PV (Private Variables)
   * 전역 변수들을 선언하는 곳입니다.
```c
/* USER CODE BEGIN PV */

uint8_t rxData[100];

volatile uint32_t msTick;

SensorData_t sensor;

/* USER CODE END PV */
```
   * CubeMX가 생성한 변수들과 별도로 사용자가 추가하는 변수들이 위치합니다.

5. PFP (Private Function Prototypes)
   * 함수 원형(Prototype)을 선언하는 곳입니다.
```c
/* USER CODE BEGIN PFP */

void ReadSensor(void);
void ProcessData(void);
void SendUART(char *str);

/* USER CODE END PFP */
```

   * 함수의 실제 구현은 아래쪽에서 합니다.
```
void ReadSensor(void)
{
...
}
```

---

## 전체 main.c 구조

   * STM32CubeIDE가 생성하는 main.c는 대략 아래와 같은 구조를 가집니다.

```
main.c
│
├── Header
│
├── Include Files
│
├── USER CODE BEGIN Includes
│      추가 헤더파일
│
├── PTD
│      typedef
│      struct
│      enum
│
├── PD
│      #define 상수
│
├── PM
│      매크로 함수
│
├── Peripheral Handle Variables
│      UART_HandleTypeDef
│      I2C_HandleTypeDef
│      ADC_HandleTypeDef
│
├── PV
│      전역 변수
│
├── PFP
│      함수 원형 선언
│
├── USER CODE 0
│      함수 구현부
│
└── main()
      │
      ├── HAL_Init()
      │
      ├── SystemClock_Config()
      │
      ├── MX_GPIO_Init()
      ├── MX_USART2_UART_Init()
      ├── MX_ADC1_Init()
      ├── ...
      │
      ├── USER CODE BEGIN 2
      │      초기화 코드
      │
      └── while(1)
             │
             ├── USER CODE BEGIN WHILE
             │
             └── USER CODE BEGIN 3
                    메인 루프
```

---

## main.c의 계층 구조

```
+---------------------------------------------------+
|                     main.c                        |
+---------------------------------------------------+
| Header                                            |
+---------------------------------------------------+
| #include                                          |
+---------------------------------------------------+
| USER CODE Includes                                |
+---------------------------------------------------+
| PTD : Type Definitions                            |
|    typedef                                        |
|    struct                                         |
|    enum                                           |
+---------------------------------------------------+
| PD : Defines                                      |
|    #define                                        |
+---------------------------------------------------+
| PM : Macros                                       |
|    매크로 함수                                    |
+---------------------------------------------------+
| Peripheral Variables                              |
|    huart1                                         |
|    hadc1                                          |
|    hi2c1                                          |
+---------------------------------------------------+
| PV : Private Variables                            |
|    전역 변수                                      |
+---------------------------------------------------+
| PFP : Function Prototypes                         |
|    void func1(void);                              |
|    void func2(void);                              |
+---------------------------------------------------+
| USER CODE 0                                       |
|    함수 구현                                      |
+---------------------------------------------------+
| main()                                            |
|                                                   |
|  HAL_Init()                                       |
|  SystemClock_Config()                             |
|  MX_GPIO_Init()                                   |
|  MX_USART_Init()                                  |
|  MX_ADC_Init()                                    |
|                                                   |
|  USER CODE 2                                      |
|     사용자 초기화                                 |
|                                                   |
|  while(1)                                         |
|  {                                                |
|      USER CODE WHILE                              |
|      USER CODE 3                                  |
|          메인 루프                                |
|  }                                                |
+---------------------------------------------------+
```


