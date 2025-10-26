# 피에조 부저로 마리오 음악을 연주

<img width="358" height="278" alt="127" src="https://github.com/user-attachments/assets/4ec83cb3-fcac-45a0-b27c-9221ebe30de6" />
<br>

<img width="644" height="586" alt="F103RB-pin" src="https://github.com/user-attachments/assets/774d7903-5392-4df3-b8da-f16d6996ea9c" />
<br>

### 타이머 설정 (TIM1):
   * Timers → TIM1 → Clock Source: Internal Clock
   * Channel1 → PWM Generation CH1
   * Configuration → Parameter Settings:
      * Prescaler: 64MHz ÷ 64 (1MHz 클록)
      * Counter Period: 1000 (초기값, 코드에서 동적 변경)
      * Pulse: 500 (50% duty cycle)

<img width="800" height="600" alt="piezo_002" src="https://github.com/user-attachments/assets/6cd1ec46-4fc0-4612-98e3-4301c19f623a" />
<br>
<img width="800" height="600" alt="piezo_001" src="https://github.com/user-attachments/assets/d07acb56-dbe3-4881-80dd-251518bbb7a4" />
<br>


```c
/* USER CODE BEGIN PD */
// 음표 주파수 정의 (Hz)
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951

#define REST 0

// 음표 길이 정의 (밀리초)
#define WHOLE     1400      // 원래 2000
#define HALF      700       // 원래 1000
#define QUARTER   350       // 원래 500
#define EIGHTH    175       // 원래 250
#define SIXTEENTH 90        // 원래 125
/* USER CODE END PD */
```

```c
/* USER CODE BEGIN PV */
typedef struct {
    uint16_t frequency;
    uint16_t duration;
} Note;

const Note mario_theme[] = {
    // 첫 번째 구간
    {NOTE_E7, EIGHTH}, {NOTE_E7, EIGHTH}, {REST, EIGHTH}, {NOTE_E7, EIGHTH},
    {REST, EIGHTH}, {NOTE_C7, EIGHTH}, {NOTE_E7, EIGHTH}, {REST, EIGHTH},
    {NOTE_G7, QUARTER}, {REST, QUARTER}, {NOTE_G6, QUARTER}, {REST, QUARTER},

    // 두 번째 구간
    {NOTE_C7, QUARTER}, {REST, EIGHTH}, {NOTE_G6, EIGHTH}, {REST, EIGHTH},
    {NOTE_E6, QUARTER}, {REST, EIGHTH}, {NOTE_A6, EIGHTH}, {REST, EIGHTH},
    {NOTE_B6, EIGHTH}, {REST, EIGHTH}, {NOTE_AS6, EIGHTH}, {NOTE_A6, QUARTER},

    // 세 번째 구간
    {NOTE_G6, EIGHTH}, {NOTE_E7, EIGHTH}, {NOTE_G7, EIGHTH}, {NOTE_A7, QUARTER},
    {NOTE_F7, EIGHTH}, {NOTE_G7, EIGHTH}, {REST, EIGHTH}, {NOTE_E7, EIGHTH},
    {REST, EIGHTH}, {NOTE_C7, EIGHTH}, {NOTE_D7, EIGHTH}, {NOTE_B6, QUARTER},

    // 반복 구간
    {NOTE_C7, QUARTER}, {REST, EIGHTH}, {NOTE_G6, EIGHTH}, {REST, EIGHTH},
    {NOTE_E6, QUARTER}, {REST, EIGHTH}, {NOTE_A6, EIGHTH}, {REST, EIGHTH},
    {NOTE_B6, EIGHTH}, {REST, EIGHTH}, {NOTE_AS6, EIGHTH}, {NOTE_A6, QUARTER},

    {NOTE_G6, EIGHTH}, {NOTE_E7, EIGHTH}, {NOTE_G7, EIGHTH}, {NOTE_A7, QUARTER},
    {NOTE_F7, EIGHTH}, {NOTE_G7, EIGHTH}, {REST, EIGHTH}, {NOTE_E7, EIGHTH},
    {REST, EIGHTH}, {NOTE_C7, EIGHTH}, {NOTE_D7, EIGHTH}, {NOTE_B6, QUARTER},

    // 마무리
    {REST, QUARTER}, {NOTE_G7, EIGHTH}, {NOTE_FS7, EIGHTH}, {NOTE_F7, EIGHTH},
    {NOTE_DS7, QUARTER}, {NOTE_E7, EIGHTH}, {REST, EIGHTH}, {NOTE_GS6, EIGHTH},
    {NOTE_A6, EIGHTH}, {NOTE_C7, EIGHTH}, {REST, EIGHTH}, {NOTE_A6, EIGHTH},
    {NOTE_C7, EIGHTH}, {NOTE_D7, EIGHTH}
};

const int mario_theme_length = sizeof(mario_theme) / sizeof(mario_theme[0]);
/* USER CODE END PV */
```

```c
/* USER CODE BEGIN PFP */
void play_tone(uint16_t frequency, uint16_t duration);
void play_mario_theme(void);
/* USER CODE END PFP */
```

```c
/* USER CODE BEGIN 0 */
/**
 * @brief 특정 주파수와 지속시간으로 톤 재생
 * @param frequency: 재생할 주파수 (Hz), 0이면 무음
 * @param duration: 재생 시간 (밀리초)
 */
void play_tone(uint16_t frequency, uint16_t duration) {
    if (frequency == 0) {
        // 무음 처리
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    } else {
        // 주파수에 따른 ARR 값 계산
        // APB2 클록이 72MHz이고, Prescaler가 72-1이면 1MHz
        // ARR = 1000000 / frequency - 1
        uint32_t arr_value = 1000000 / frequency - 1;

        // 타이머 설정 업데이트
        __HAL_TIM_SET_AUTORELOAD(&htim1, arr_value);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, arr_value / 2); // 50% duty cycle

        // PWM 시작
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    }

    // 지정된 시간만큼 대기
    HAL_Delay(duration);

    // 톤 정지
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);

    // 음표 사이의 짧은 간격 (더 빠른 연주를 위해 단축)
    HAL_Delay(30);
}

/**
 * @brief 마리오 테마 음악 재생
 */
void play_mario_theme(void) {
    for (int i = 0; i < mario_theme_length; i++) {
        play_tone(mario_theme[i].frequency, mario_theme[i].duration);
    }
}

/* USER CODE END 0 */
```

```c
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	// 마리오 테마 음악 재생
	play_mario_theme();

	// 음악 종료 후 5초 대기
	HAL_Delay(1);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
```

## 가습기도 가능할까?

<img width="300" height="300" alt="128" src="https://github.com/user-attachments/assets/624c29d0-d512-4d49-ab3b-6123507a595a" />
<br>

