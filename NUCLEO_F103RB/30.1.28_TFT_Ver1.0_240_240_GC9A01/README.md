# 1.28_TFT_Ver1.0_240_240_GC9A01

<img width="220" height="200" alt="006" src="https://github.com/user-attachments/assets/5f91567d-35c8-4f10-a88c-1304b2938f2c" />
<img width="220" height="200" alt="007" src="https://github.com/user-attachments/assets/7c9289c4-a253-4ab1-a88e-0ae09b224275" />
<img width="220" height="200" alt="008" src="https://github.com/user-attachments/assets/4ac1b2a8-8673-4ca4-9b15-3acec78ee34e" />
<br>

<img width="500" height="450" alt="F103RB-pin" src="https://github.com/user-attachments/assets/0b7587df-f5e7-434e-9c47-1aed5de95374" />



<img width="600" height="400" alt="gc9a01_001" src="https://github.com/user-attachments/assets/ba7594fb-f077-4608-8749-5094855a9c39" />
<br>
<img width="600" height="400" alt="gc9a01_002" src="https://github.com/user-attachments/assets/8f8d7a1f-abc7-4665-a001-ae236f707533" />
<br>
<img width="600" height="400" alt="gc9a01_003" src="https://github.com/user-attachments/assets/127686c4-e761-4ab7-a3a9-57345b32da55" />
<br>
<img width="600" height="400" alt="gc9a01_004" src="https://github.com/user-attachments/assets/09557294-f931-4b8c-a2c0-00a3b0c99a8a" />
<br>
<img width="600" height="400" alt="gc9a01_005" src="https://github.com/user-attachments/assets/f1b4b942-ffe3-40a7-97f8-46d2b5b62322" />
<br>

### 하드웨어 연결
   * 제공하신 핀 구성(RST, CD, DC, SDA, SCL)을 보면 SPI 통신을 사용하는 GC9A01 컨트롤러 기반 LCD로 보입니다.
   * 권장 핀 연결:
      * VCC → 3.3V
      * GND → GND
      * SCL → PA5 (SPI1_SCK)
      * SDA → PA7 (SPI1_MOSI)
      * DC → PA9 (Data/Command)
      * RST → PA8 (Reset)
      * CS → PA4 (Chip Select) - CD 핀이 CS일 가능성

### STM32CubeIDE 설정
   * 1. Clock Configuration
      * System Clock을 64MHz로 설정
      * APB1: 32MHz, APB2: 64MHz
   * 2. SPI1 Configuration
      * Mode: Full-Duplex Master
      * Hardware NSS: Disable
      * Frame Format: Motorola
      * Data Size: 8 Bits
      * First Bit: MSB First
      * Prescaler: 2 (32MHz SPI Clock)
      * Clock Polarity: Low
      * Clock Phase: 1 Edge
   * 3. GPIO Configuration
      * PA4: GPIO_Output (CS)
      * PA8: GPIO_Output (RST)
      * PA9: GPIO_Output (DC)


```c
/* USER CODE BEGIN Includes */
#include "gc9a01_driver.h"
#include <stdio.h>
#include <math.h>
/* USER CODE END Includes */
```

```c
/* USER CODE BEGIN PFP */
/* 시계 그리기 함수 */
void DrawClockFace(void);
void DrawClockHands(uint8_t hour, uint8_t minute, uint8_t second);
void DrawDigitalTime(uint8_t hour, uint8_t minute, uint8_t second);
void ClearClockHands(void);

/* 전역 변수 */
#define CENTER_X 120
#define CENTER_Y 120
#define CLOCK_RADIUS 110

// 이전 바늘 위치 저장
int16_t prev_hour_x = 0, prev_hour_y = 0;
int16_t prev_min_x = 0, prev_min_y = 0;
int16_t prev_sec_x = 0, prev_sec_y = 0;
/* USER CODE END PFP */
```

```c
/* USER CODE BEGIN 0 */
///**
//  * @brief 시계판 그리기
//  */
//void DrawClockFace(void)
//{
//    // 배경을 검은색으로
//    GC9A01_FillScreen(COLOR_BLACK);
//
//    // 외곽 원 그리기
//    GC9A01_DrawCircle(CENTER_X, CENTER_Y, CLOCK_RADIUS, COLOR_WHITE);
//    GC9A01_DrawCircle(CENTER_X, CENTER_Y, CLOCK_RADIUS - 1, COLOR_WHITE);
//
//    // 12시간 눈금 그리기
//    for (int i = 0; i < 12; i++) {
//        float angle = (i * 30 - 90) * 3.14159 / 180.0;
//        int x1 = CENTER_X + (CLOCK_RADIUS - 15) * cos(angle);
//        int y1 = CENTER_Y + (CLOCK_RADIUS - 15) * sin(angle);
//        int x2 = CENTER_X + (CLOCK_RADIUS - 5) * cos(angle);
//        int y2 = CENTER_Y + (CLOCK_RADIUS - 5) * sin(angle);
//        GC9A01_DrawLine(x1, y1, x2, y2, COLOR_WHITE);
//    }
//
//    // 5분 단위 작은 눈금
//    for (int i = 0; i < 60; i++) {
//        if (i % 5 != 0) {  // 12시간 눈금이 아닌 경우만
//            float angle = (i * 6 - 90) * 3.14159 / 180.0;
//            int x1 = CENTER_X + (CLOCK_RADIUS - 10) * cos(angle);
//            int y1 = CENTER_Y + (CLOCK_RADIUS - 10) * sin(angle);
//            int x2 = CENTER_X + (CLOCK_RADIUS - 5) * cos(angle);
//            int y2 = CENTER_Y + (CLOCK_RADIUS - 5) * sin(angle);
//            GC9A01_DrawLine(x1, y1, x2, y2, COLOR_GRAY);
//        }
//    }
//
//    // 중앙점
//    GC9A01_FillCircle(CENTER_X, CENTER_Y, 3, COLOR_RED);
//}
//
///**
//  * @brief 시계 바늘 그리기
//  */
//void DrawClockHands(uint8_t hour, uint8_t minute, uint8_t second)
//{
//    // 시침 (빨간색)
//    float hour_angle = ((hour % 12) * 30 + minute * 0.5 - 90) * 3.14159 / 180.0;
//    int hour_x = CENTER_X + 50 * cos(hour_angle);
//    int hour_y = CENTER_Y + 50 * sin(hour_angle);
//    GC9A01_DrawLine(CENTER_X, CENTER_Y, hour_x, hour_y, COLOR_RED);
//    GC9A01_DrawLine(CENTER_X + 1, CENTER_Y, hour_x + 1, hour_y, COLOR_RED);
//    GC9A01_DrawLine(CENTER_X, CENTER_Y + 1, hour_x, hour_y + 1, COLOR_RED);
//
//    // 분침 (녹색)
//    float minute_angle = (minute * 6 - 90) * 3.14159 / 180.0;
//    int minute_x = CENTER_X + 75 * cos(minute_angle);
//    int minute_y = CENTER_Y + 75 * sin(minute_angle);
//    GC9A01_DrawLine(CENTER_X, CENTER_Y, minute_x, minute_y, COLOR_GREEN);
//    GC9A01_DrawLine(CENTER_X + 1, CENTER_Y, minute_x + 1, minute_y, COLOR_GREEN);
//
//    // 초침 (노란색)
//    float second_angle = (second * 6 - 90) * 3.14159 / 180.0;
//    int second_x = CENTER_X + 90 * cos(second_angle);
//    int second_y = CENTER_Y + 90 * sin(second_angle);
//    GC9A01_DrawLine(CENTER_X, CENTER_Y, second_x, second_y, COLOR_YELLOW);
//
//    // 중앙점 다시 그리기
//    GC9A01_FillCircle(CENTER_X, CENTER_Y, 3, COLOR_RED);
//}
//
///**
//  * @brief 디지털 시간 표시
//  */
//void DrawDigitalTime(uint8_t hour, uint8_t minute, uint8_t second)
//{
//    char time_str[9];
//    sprintf(time_str, "%02d:%02d:%02d", hour, minute, second);
//
//    // 화면 하단에 디지털 시간 표시
//    GC9A01_FillRect(70, 170, 100, 20, COLOR_BLACK);
//    GC9A01_DrawString(75, 175, time_str, COLOR_CYAN, COLOR_BLACK, 2);
//}
/**
  * @brief 시계판 그리기 (최초 1회만)
  */
void DrawClockFace(void)
{
    // 배경을 검은색으로
    GC9A01_FillScreen(COLOR_BLACK);

    // 외곽 원 그리기
    GC9A01_DrawCircle(CENTER_X, CENTER_Y, CLOCK_RADIUS, COLOR_WHITE);
    GC9A01_DrawCircle(CENTER_X, CENTER_Y, CLOCK_RADIUS - 1, COLOR_WHITE);

    // 12시간 눈금 그리기
    for (int i = 0; i < 12; i++) {
        float angle = (i * 30 - 90) * 3.14159f / 180.0f;
        int x1 = CENTER_X + (int)((CLOCK_RADIUS - 15) * cosf(angle));
        int y1 = CENTER_Y + (int)((CLOCK_RADIUS - 15) * sinf(angle));
        int x2 = CENTER_X + (int)((CLOCK_RADIUS - 5) * cosf(angle));
        int y2 = CENTER_Y + (int)((CLOCK_RADIUS - 5) * sinf(angle));
        GC9A01_DrawLine(x1, y1, x2, y2, COLOR_WHITE);
    }

    // 5분 단위 작은 눈금
    for (int i = 0; i < 60; i++) {
        if (i % 5 != 0) {
            float angle = (i * 6 - 90) * 3.14159f / 180.0f;
            int x1 = CENTER_X + (int)((CLOCK_RADIUS - 10) * cosf(angle));
            int y1 = CENTER_Y + (int)((CLOCK_RADIUS - 10) * sinf(angle));
            int x2 = CENTER_X + (int)((CLOCK_RADIUS - 5) * cosf(angle));
            int y2 = CENTER_Y + (int)((CLOCK_RADIUS - 5) * sinf(angle));
            GC9A01_DrawLine(x1, y1, x2, y2, COLOR_GRAY);
        }
    }

    // 중앙점
    GC9A01_FillCircle(CENTER_X, CENTER_Y, 4, COLOR_RED);
}

/**
  * @brief 이전 시계 바늘 지우기 (최적화)
  */
void ClearClockHands(void)
{
    // 이전 초침 지우기
    if (prev_sec_x != 0 || prev_sec_y != 0) {
        GC9A01_DrawLine(CENTER_X, CENTER_Y, prev_sec_x, prev_sec_y, COLOR_BLACK);
    }

    // 이전 분침 지우기
    if (prev_min_x != 0 || prev_min_y != 0) {
        GC9A01_DrawLine(CENTER_X, CENTER_Y, prev_min_x, prev_min_y, COLOR_BLACK);
        GC9A01_DrawLine(CENTER_X + 1, CENTER_Y, prev_min_x + 1, prev_min_y, COLOR_BLACK);
    }

    // 이전 시침 지우기
    if (prev_hour_x != 0 || prev_hour_y != 0) {
        GC9A01_DrawLine(CENTER_X, CENTER_Y, prev_hour_x, prev_hour_y, COLOR_BLACK);
        GC9A01_DrawLine(CENTER_X + 1, CENTER_Y, prev_hour_x + 1, prev_hour_y, COLOR_BLACK);
        GC9A01_DrawLine(CENTER_X, CENTER_Y + 1, prev_hour_x, prev_hour_y + 1, COLOR_BLACK);
    }
}

/**
  * @brief 시계 바늘 그리기 (최적화)
  */
void DrawClockHands(uint8_t hour, uint8_t minute, uint8_t second)
{
    // 시침 (빨간색)
    float hour_angle = ((hour % 12) * 30.0f + minute * 0.5f - 90.0f) * 3.14159f / 180.0f;
    prev_hour_x = CENTER_X + (int)(50 * cosf(hour_angle));
    prev_hour_y = CENTER_Y + (int)(50 * sinf(hour_angle));
    GC9A01_DrawLine(CENTER_X, CENTER_Y, prev_hour_x, prev_hour_y, COLOR_RED);
    GC9A01_DrawLine(CENTER_X + 1, CENTER_Y, prev_hour_x + 1, prev_hour_y, COLOR_RED);
    GC9A01_DrawLine(CENTER_X, CENTER_Y + 1, prev_hour_x, prev_hour_y + 1, COLOR_RED);

    // 분침 (녹색)
    float minute_angle = (minute * 6.0f - 90.0f) * 3.14159f / 180.0f;
    prev_min_x = CENTER_X + (int)(75 * cosf(minute_angle));
    prev_min_y = CENTER_Y + (int)(75 * sinf(minute_angle));
    GC9A01_DrawLine(CENTER_X, CENTER_Y, prev_min_x, prev_min_y, COLOR_GREEN);
    GC9A01_DrawLine(CENTER_X + 1, CENTER_Y, prev_min_x + 1, prev_min_y, COLOR_GREEN);

    // 초침 (노란색)
    float second_angle = (second * 6.0f - 90.0f) * 3.14159f / 180.0f;
    prev_sec_x = CENTER_X + (int)(90 * cosf(second_angle));
    prev_sec_y = CENTER_Y + (int)(90 * sinf(second_angle));
    GC9A01_DrawLine(CENTER_X, CENTER_Y, prev_sec_x, prev_sec_y, COLOR_YELLOW);

    // 중앙점 다시 그리기
    GC9A01_FillCircle(CENTER_X, CENTER_Y, 4, COLOR_RED);
}

/**
  * @brief 디지털 시간 표시
  */
void DrawDigitalTime(uint8_t hour, uint8_t minute, uint8_t second)
{
    //char time_str[9];
	char time_str[12];
    sprintf(time_str, "%02d:%02d:%02d", hour, minute, second);

    // 화면 하단에 디지털 시간 표시
    GC9A01_FillRect(70, 170, 100, 20, COLOR_BLACK);
    GC9A01_DrawString(75, 175, time_str, COLOR_CYAN, COLOR_BLACK, 2);
}
/* USER CODE END 0 */
```

```c
  /* USER CODE BEGIN 2 */
  /* Initialize GC9A01 LCD */
  GC9A01_Init(&hspi1);
  //GC9A01_SetRotation(0);  // 0, 1, 2, 3 중 하나 선택
  HAL_Delay(100);

  /* 시계 배경 그리기 */
  DrawClockFace();

  /* 시간 변수 */
  uint8_t hour = 10, minute = 8, second = 0;
  uint8_t prev_second = 0xFF;
//  /* Initialize GC9A01 LCD */
//  GC9A01_Init(&hspi1);
//  HAL_Delay(100);
//
//  /* 시계 배경 그리기 */
//  DrawClockFace();
//
//  /* 시간 변수 */
//  uint8_t hour = 10, minute = 8, second = 0;
//  uint8_t prev_second = 0xFF;
  /* USER CODE END 2 */
```

```c
    /* USER CODE BEGIN 3 */
      // 데모용 시간 증가 (1초마다)
      HAL_Delay(1000);
      second++;
      if (second >= 60) {
          second = 0;
          minute++;
          if (minute >= 60) {
              minute = 0;
              hour++;
              if (hour >= 24) {
                  hour = 0;
              }
          }
      }

      // 초침이 변경되었을 때만 업데이트 (최적화)
      if (second != prev_second) {
          prev_second = second;

          // 이전 바늘 지우기 (검은색으로 덮어쓰기)
          ClearClockHands();

          // 새로운 시계 바늘 그리기
          DrawClockHands(hour, minute, second);

          // 디지털 시간 표시
          DrawDigitalTime(hour, minute, second);
      }
//      // RTC에서 시간 읽기 (RTC 설정 시)
//      // RTC_TimeTypeDef sTime;
//      // HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
//      // hour = sTime.Hours;
//      // minute = sTime.Minutes;
//      // second = sTime.Seconds;
//
//      // 데모용 시간 증가 (1초마다)
//      HAL_Delay(1000);
//      second++;
//      if (second >= 60) {
//          second = 0;
//          minute++;
//          if (minute >= 60) {
//              minute = 0;
//              hour++;
//              if (hour >= 24) {
//                  hour = 0;
//              }
//          }
//      }
//
//      // 초침이 변경되었을 때만 업데이트
//      if (second != prev_second) {
//          prev_second = second;
//
//          // 시계 바늘 그리기 영역 지우기 (중앙 원만)
//          GC9A01_FillCircle(CENTER_X, CENTER_Y, CLOCK_RADIUS - 10, COLOR_BLACK);
//
//          // 시간 눈금은 다시 그리기
//          for (int i = 0; i < 12; i++) {
//              float angle = (i * 30 - 90) * 3.14159 / 180.0;
//              int x1 = CENTER_X + (CLOCK_RADIUS - 15) * cos(angle);
//              int y1 = CENTER_Y + (CLOCK_RADIUS - 15) * sin(angle);
//              int x2 = CENTER_X + (CLOCK_RADIUS - 5) * cos(angle);
//              int y2 = CENTER_Y + (CLOCK_RADIUS - 5) * sin(angle);
//              GC9A01_DrawLine(x1, y1, x2, y2, COLOR_WHITE);
//          }
//
//          // 시계 바늘 그리기
//          DrawClockHands(hour, minute, second);
//
//          // 디지털 시간 표시
//          DrawDigitalTime(hour, minute, second);
//      }
  }
  /* USER CODE END 3 */
```






