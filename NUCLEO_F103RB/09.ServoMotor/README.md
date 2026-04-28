# SG90 Servo 제어를 위한 Timer 설정 (STM32 예제)

<img width="418" height="374" alt="130" src="https://github.com/user-attachments/assets/9da3ca1d-416c-4d09-95be-7634f60bd457" />
<br>

<img width="644" height="586" alt="F103RB-pin" src="https://github.com/user-attachments/assets/23e365b4-1bdf-4074-9724-d795ea1da5b7" />
<br>

<img width="800" height="608" alt="SERVO_003" src="https://github.com/user-attachments/assets/a6ca154d-6616-407b-9e77-ab1566bb1a80" />
<br>
<img width="800" height="608" alt="SERVO_004" src="https://github.com/user-attachments/assets/f20ec8df-36e9-42e1-8671-6223fc108338" />
<br>


## 1. 기본 조건
- **타이머 클럭** = 64 MHz  
- **Prescaler** = 1280 - 1 = 1279  (0 ~ 65535)
- **Period** = 1000 - 1 = 999 (0 ~ 65535)

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

$$
\frac{1}{50,000 \, \text{Hz}} = 0.00002 \, \text{s} = 20 \, \mu\text{s}
$$

---

## 3. PWM 주기
$$
T_{PWM} = \frac{Period + 1}{f_{timer}} = \frac{1000}{50,000} = 0.02 \, s = 20 \, ms
$$

✅ 따라서 PWM 주기 = **20 ms (50 Hz)** → SG90 서보 요구사항과 일치  

---

## 4. 펄스 폭 (CCR 값으로 각도 제어)

<img src="001.png" width = "50%">

* Position 
  * "0" (0.5 ms pulse) is middle, 
  * "90" (1.5 ms pulse) is all the way to the right, 
  * "180" (2.5 ms pulse) is all the way to the left.

* CCR은 Capture/Compare Register
   * Compare (비교): 타이머의 현재 카운트 값(CNT)과 내가 설정한 CCR 값을 계속 비교합니다.
   * 동작 원리: PWM 모드에서는 카운트 값이 CCR보다 작을 때는 High(1)를 유지하다가, CCR 값에 도달하는 순간 Low(0)로 떨어뜨립니다. 즉, CCR 값은 PWM의 High 구간(Duty Cycle)의 길이를 결정하는 핵심 레지스터입니다.
   * CCR 1당 0.02ms를 의미하는 셈입니다. ($20ms / 1000 = 0.02ms$)

| 각도 | 펄스 폭 (ms) | 계산식 (ARR=1000 기준) | CCR 값 | 
|:---:|:---:|:---:|:---:|
| 0°| 0.5 ms| 1000×(1ms/20ms)=50| 25 | 
| 90°| 1.5 ms| 1000×(1.5ms/20ms)=75| 75 | 
| 180°| 2.5 ms| 1000×(2ms/20ms)=100| 125 | 

- **1 ms** 펄스 폭  
$$\frac{0.5 \, \text{ms}}{20 \, \mu\text{s}} = 25 \quad \Rightarrow \quad \text{CCR} = 25$$

- **1.5 ms** 펄스 폭  
$$\frac{1.5 \, \text{ms}}{20 \, \mu\text{s}} = 75 \quad \Rightarrow \quad \text{CCR} = 75$$

- **2 ms** 펄스 폭  
$$\frac{2.5 \, \text{ms}}{20 \, \mu\text{s}} = 120 \quad \Rightarrow \quad \text{CCR} = 125$$

---

## 5. 요약
- Prescaler = **1279**, Period = **999** → 정확히 **50 Hz (20 ms)** PWM 생성  
- CCR 값 50 ~ 100 사이로 설정하여 SG90 서보 각도 (0°~180°) 제어 가능  

---

## 6. 각도별 CCR 값
- 0° → 0.5 ms → CCR = 25  
- 90° → 1.5 ms → CCR = 75  
- 180° → 2.5 ms → CCR = 125  

<img src="TEK0005.JPG" width="50%">

```c
// 0도
__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 25);
```
<img src="TEK0006.JPG" width="50%">

```c
// 90도
__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 75);
```
<img src="TEK0007.JPG" width="50%">

```c
// 180도
__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 125);
```
<img src="TEK0008.JPG" width="50%">

```c
SG90_SetAngle(0);   // 0도
HAL_Delay(1000);

SG90_SetAngle(90);   // 90도
HAL_Delay(1000);

SG90_SetAngle(180);  // 180도
HAL_Delay(1000);
```

---

## 7. 각도를 일반화한 함수
```c
void SG90_SetAngle(uint8_t angle)
{
    // Period = 1000, 주기 = 20ms
    // 0도  → 0.5ms → CCR = 25
    // 90도 → 1.5ms → CCR = 75
    // 180도→ 2.5ms → CCR = 125
    uint32_t ccr_val = 25 + ((uint32_t)angle * 100 / 180);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, ccr_val);
}
```

---

## 8. 사용 예시

### 8.1 특정 각도로 이동

```c
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  /* USER CODE END 2 */
```

```c
SG90_SetAngle(0);    // 0도
HAL_Delay(1000);

SG90_SetAngle(90);   // 90도
HAL_Delay(1000);

SG90_SetAngle(180);  // 180도
HAL_Delay(1000);
```

### 8.2 각도 이동 확인 : 0° → 180° → 0° 스윕 예시

```c
/* USER CODE BEGIN WHILE */
while (1)
{
    // 0도 → 180도 증가
    for (int angle = 0; angle <= 180; angle++)
    {
        SG90_SetAngle(angle);
        HAL_Delay(100);
    }

    // 180도 → 0도 감소
    for (int angle = 180; angle >= 0; angle--)
    {
        SG90_SetAngle(angle);
        HAL_Delay(100);
    }
    /* USER CODE END WHILE */
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
#define STEP 1
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
* 위의 if 포현을 switch case로 변경하면?
```c
/* USER CODE BEGIN WHILE */
  while (1)
  {
    if (HAL_UART_Receive(&huart2, &ch, sizeof(ch), 10) == HAL_OK)
    {
      switch (ch)
      {
        case 's':
          printf("Down\r\n");
          if (pos_tilt + STEP <= MAX)
            pos_tilt += STEP;
          else
            pos_tilt = MAX;
          break;

        case 'w':
          printf("Up\r\n");
          if (pos_tilt - STEP >= MIN)
            pos_tilt -= STEP;
          else
            pos_tilt = MIN;
          break;

        case 'a':
          printf("Left\r\n");
          if (pos_pan + STEP <= MAX)
            pos_pan += STEP;
          else
            pos_pan = MAX;
          break;

        case 'd':
          printf("Right\r\n");
          if (pos_pan - STEP >= MIN)
            pos_pan -= STEP;
          else
            pos_pan = MIN;
          break;

        case 'i':
          printf("Center\r\n");
          pos_pan = CENTER;
          pos_tilt = CENTER;
          break;

        default:
          // 정의되지 않은 키가 입력되었을 때의 처리 (선택 사항)
          break;
      }

      // PWM 듀티 사이클 업데이트
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pos_pan);
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pos_tilt);

      printf("Pan: %d, Tilt: %d\r\n", pos_pan, pos_tilt);

      HAL_Delay(50); // 서보 응답 시간
    }
    /* USER CODE END WHILE */
```

---
# 각도표시
---

<img width="995" height="550" alt="servo_result" src="https://github.com/user-attachments/assets/c42adba2-96aa-4ff5-a119-5044486adb6e" />


```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */
```

```c
/* USER CODE BEGIN PD */
#define MAX 125      // 2.5ms pulse width (180도)
#define MIN 25       // 0.5ms pulse width (0도)
#define CENTER 75    // 1.5ms pulse width (90도)
#define STEP 5       // 이동 단위
/* USER CODE END PD */
```

```c
/* USER CODE BEGIN PV */
uint8_t ch;
uint8_t pos_pan = CENTER;
uint8_t pos_tilt = CENTER;
/* USER CODE END PV */
```

```c
/* USER CODE BEGIN PFP */
uint16_t pwm_to_angle(uint8_t pwm_value);
void display_servo_status(uint8_t pan, uint8_t tilt);
/* USER CODE END PFP */
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

/**
  * @brief  PWM 값을 각도로 변환하는 함수
  * @param  pwm_value: PWM 듀티 사이클 값 (25~125)
  * @retval 각도 값 (0~1800, 실제 각도 x 10)
  */
uint16_t pwm_to_angle(uint8_t pwm_value)
{
  // PWM 25~125 범위를 0~180도로 변환
  // 소수점 계산을 위해 10배로 확대 (0~1800)
  // 공식: angle = (pwm_value - 25) * 1800 / (125 - 25)
  return ((uint16_t)(pwm_value - MIN) * 1800) / (MAX - MIN);
}

/**
  * @brief  서보모터 상태를 화면에 출력하는 함수
  * @param  pan: Pan 서보 PWM 값
  * @param  tilt: Tilt 서보 PWM 값
  * @retval None
  */
void display_servo_status(uint8_t pan, uint8_t tilt)
{
  uint16_t pan_angle = pwm_to_angle(pan);
  uint16_t tilt_angle = pwm_to_angle(tilt);
  
  printf("Pan: %3d (%3d.%d°) | Tilt: %3d (%3d.%d°)\r\n", 
         pan, pan_angle/10, pan_angle%10,
         tilt, tilt_angle/10, tilt_angle%10);
}
/* USER CODE END 0 */
```

```c
  /* USER CODE BEGIN 2 */
  // PWM 시작
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  
  // 초기 위치 설정
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pos_pan);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pos_tilt);
  
  printf("\r\n=== SG90 Servo Control System ===\r\n");
  printf("Commands: w(up), s(down), a(left), d(right), i(center)\r\n");
  printf("Initial Position:\r\n");
  display_servo_status(pos_pan, pos_tilt);
  printf("Ready!\r\n\r\n");
  /* USER CODE END 2 */

```

```c
 /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if(HAL_UART_Receive(&huart2, &ch, sizeof(ch), 10) == HAL_OK)
    {
      // 명령 처리
      if(ch == 's')  // Down
      {
        printf("Command: Down\r\n");
        if(pos_tilt + STEP <= MAX) 
          pos_tilt = pos_tilt + STEP;
        else 
          pos_tilt = MAX;
      }
      else if(ch == 'w')  // Up
      {
        printf("Command: Up\r\n");
        if(pos_tilt - STEP >= MIN) 
          pos_tilt = pos_tilt - STEP;
        else 
          pos_tilt = MIN;
      }
      else if(ch == 'a')  // Left
      {
        printf("Command: Left\r\n");
        if(pos_pan + STEP <= MAX)	
          pos_pan = pos_pan + STEP;
        else 
          pos_pan = MAX;
      }
      else if(ch == 'd')  // Right
      {
        printf("Command: Right\r\n");
        if(pos_pan - STEP >= MIN)	
          pos_pan = pos_pan - STEP;
        else 
          pos_pan = MIN;
      }
      else if(ch == 'i')  // Center
      {
        printf("Command: Center\r\n");
        pos_pan = CENTER;
        pos_tilt = CENTER;
      }
      else
      {
        printf("Invalid command: %c\r\n", ch);
        continue;  // 잘못된 명령이면 PWM 업데이트 하지 않음
      }

      // PWM 듀티 사이클 업데이트
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pos_pan);
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pos_tilt);
      
      // 상태 출력 (pwm_to_angle 함수 실제 사용됨)
      display_servo_status(pos_pan, pos_tilt);
      
      HAL_Delay(50); // 서보 응답 시간
    }
    
    /* USER CODE END WHILE */
```


```c
/* USER CODE BEGIN WHILE */
  while (1)
  {
    if (HAL_UART_Receive(&huart2, &ch, sizeof(ch), 10) == HAL_OK)
    {
      switch (ch)
      {
        case 's': // Down
          printf("Command: Down\r\n");
          pos_tilt = (pos_tilt + STEP <= MAX) ? (pos_tilt + STEP) : MAX;
          break;

        case 'w': // Up
          printf("Command: Up\r\n");
          pos_tilt = (pos_tilt - STEP >= MIN) ? (pos_tilt - STEP) : MIN;
          break;

        case 'a': // Left
          printf("Command: Left\r\n");
          pos_pan = (pos_pan + STEP <= MAX) ? (pos_pan + STEP) : MAX;
          break;

        case 'd': // Right
          printf("Command: Right\r\n");
          pos_pan = (pos_pan - STEP >= MIN) ? (pos_pan - STEP) : MIN;
          break;

        case 'i': // Center
          printf("Command: Center\r\n");
          pos_pan = CENTER;
          pos_tilt = CENTER;
          break;

        default:
          printf("Invalid command: %c\r\n", ch);
          continue; // 잘못된 명령이면 아래 PWM 업데이트 로직을 건너뜀
      }

      // PWM 듀티 사이클 업데이트 (정상적인 명령일 때만 실행됨)
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pos_pan);
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pos_tilt);

      // 상태 출력
      display_servo_status(pos_pan, pos_tilt);

      HAL_Delay(50); // 서보 응답 시간
    }
  }
  /* USER CODE END WHILE */
```


## Camera + PAN/TILT

* https://www.anaconda.com/download/success?reg=skipped

카메라 확인 프로그램
- 연결된 카메라 자동 검색
- 다중 카메라 동시 표시

* cam_test.py

```python
import cv2

def find_cameras(max_check=5):
    """연결된 카메라 검색"""
    cameras = []
    for i in range(max_check):
        cap = cv2.VideoCapture(i)
        if cap.isOpened():
            ret, _ = cap.read()
            if ret:
                cameras.append(i)
                print(f"[OK] 카메라 {i} 발견")
            cap.release()
    return cameras

def main():
    print("카메라 검색 중...")
    cameras = find_cameras()
   
    if not cameras:
        print("카메라를 찾을 수 없습니다!")
        return
   
    print(f"\n총 {len(cameras)}개 카메라 발견: {cameras}")
    print("ESC 또는 Q: 종료\n")
   
    # 카메라 열기
    caps = {}
    for cam_id in cameras:
        cap = cv2.VideoCapture(cam_id)
        cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
        cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
        caps[cam_id] = cap
   
    while True:
        for cam_id, cap in caps.items():
            ret, frame = cap.read()
            if ret:
                # 카메라 번호 표시
                cv2.putText(frame, f"Camera {cam_id}", (10, 30),
                            cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
                cv2.imshow(f"Camera {cam_id}", frame)
       
        key = cv2.waitKey(1) & 0xFF
        if key == 27 or key == ord('q'):  # ESC or Q
            break
   
    # 정리
    for cap in caps.values():
        cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()

```


```
pip install pyserial
python face_tracking_pantilt.py --port COM3
python face_tracking_pantilt.py --port COM7 --camera 2
```

* 코드에서 수정

```
# 수평 제어 (Pan)
# 얼굴이 오른쪽에 있으면 -> 카메라를 오른쪽으로 (d)
# 얼굴이 왼쪽에 있으면 -> 카메라를 왼쪽으로 (a)
if error_x > self.config.deadzone_x:
	self._send_command('a')  # 오른쪽으로
	print(f"[PAN] 오른쪽 이동 (error_x: {error_x})")
	command_sent = True
elif error_x < -self.config.deadzone_x:
	self._send_command('d')  # 왼쪽으로
	print(f"[PAN] 왼쪽 이동 (error_x: {error_x})")
	command_sent = True
```

```c
#!/usr/bin/env python3
"""
Face Tracking Pan-Tilt Camera System
=====================================
웹캠으로 얼굴을 감지하고 STM32F103 Pan-Tilt 서보를 제어하여
얼굴을 화면 중앙에 유지하는 프로그램

Requirements:
    pip install opencv-python pyserial

Usage:
    python face_tracking_pantilt.py --port COM3      # Windows
    python face_tracking_pantilt.py --port /dev/ttyUSB0  # Linux
"""

import cv2
import serial
import time
import argparse
from dataclasses import dataclass


@dataclass
class TrackingConfig:
    """추적 설정 파라미터"""
    # 시리얼 설정
    baudrate: int = 115200
    
    # 카메라 설정
    camera_id: int = 0
    frame_width: int = 640
    frame_height: int = 480
    
    # 추적 설정 (화면 중앙 기준 데드존)
    deadzone_x: int = 50  # 수평 데드존 (픽셀)
    deadzone_y: int = 40  # 수직 데드존 (픽셀)
    
    # 명령 전송 간격 (초)
    command_interval: float = 0.1
    
    # 얼굴 감지 설정
    min_face_size: int = 80  # 최소 얼굴 크기 (픽셀)
    scale_factor: float = 1.1
    min_neighbors: int = 5


class FaceTracker:
    """얼굴 추적 및 Pan-Tilt 제어 클래스"""
    
    def __init__(self, port: str, config: TrackingConfig = None):
        self.config = config or TrackingConfig()
        self.serial_port = None
        self.cap = None
        self.face_cascade = None
        self.last_command_time = 0
        
        # 시리얼 포트 초기화
        self._init_serial(port)
        
        # 카메라 초기화
        self._init_camera()
        
        # 얼굴 검출기 초기화
        self._init_face_detector()
        
        # 초기 위치로 이동
        self._send_command('i')
        time.sleep(0.5)
    
    def _init_serial(self, port: str):
        """시리얼 포트 초기화"""
        try:
            self.serial_port = serial.Serial(
                port=port,
                baudrate=self.config.baudrate,
                timeout=0.1
            )
            print(f"[OK] 시리얼 포트 연결: {port} @ {self.config.baudrate}bps")
            time.sleep(2)  # STM32 리셋 대기
        except serial.SerialException as e:
            raise RuntimeError(f"시리얼 포트 열기 실패: {e}")
    
    def _init_camera(self):
        """카메라 초기화"""
        self.cap = cv2.VideoCapture(self.config.camera_id)
        if not self.cap.isOpened():
            raise RuntimeError("카메라를 열 수 없습니다")
        
        self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, self.config.frame_width)
        self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, self.config.frame_height)
        
        # 실제 해상도 확인
        actual_width = int(self.cap.get(cv2.CAP_PROP_FRAME_WIDTH))
        actual_height = int(self.cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
        print(f"[OK] 카메라 초기화: {actual_width}x{actual_height}")
        
        # 화면 중심점 계산
        self.center_x = actual_width // 2
        self.center_y = actual_height // 2
    
    def _init_face_detector(self):
        """얼굴 검출기 초기화"""
        cascade_path = cv2.data.haarcascades + 'haarcascade_frontalface_default.xml'
        self.face_cascade = cv2.CascadeClassifier(cascade_path)
        
        if self.face_cascade.empty():
            raise RuntimeError("Haar Cascade 파일 로드 실패")
        print("[OK] 얼굴 검출기 초기화 완료")
    
    def _send_command(self, cmd: str):
        """STM32에 명령 전송"""
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.write(cmd.encode())
            self.serial_port.flush()
            
            # 응답 읽기 (디버그용)
            time.sleep(0.05)
            if self.serial_port.in_waiting:
                response = self.serial_port.read(self.serial_port.in_waiting)
                print(f"  STM32: {response.decode(errors='ignore').strip()}")
    
    def _detect_faces(self, frame):
        """프레임에서 얼굴 검출"""
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        gray = cv2.equalizeHist(gray)  # 히스토그램 평활화
        
        faces = self.face_cascade.detectMultiScale(
            gray,
            scaleFactor=self.config.scale_factor,
            minNeighbors=self.config.min_neighbors,
            minSize=(self.config.min_face_size, self.config.min_face_size)
        )
        return faces
    
    def _get_largest_face(self, faces):
        """가장 큰 얼굴 선택 (가장 가까운 사람)"""
        if len(faces) == 0:
            return None
        
        # 면적 기준 정렬
        largest = max(faces, key=lambda f: f[2] * f[3])
        return largest
    
    def _calculate_error(self, face):
        """얼굴 중심과 화면 중심 간의 오차 계산"""
        x, y, w, h = face
        face_center_x = x + w // 2
        face_center_y = y + h // 2
        
        error_x = face_center_x - self.center_x
        error_y = face_center_y - self.center_y
        
        return error_x, error_y, face_center_x, face_center_y
    
    def _track_face(self, error_x: int, error_y: int):
        """오차에 따라 Pan-Tilt 명령 전송"""
        current_time = time.time()
        
        # 명령 전송 간격 체크
        if current_time - self.last_command_time < self.config.command_interval:
            return
        
        command_sent = False
        
        # 수평 제어 (Pan)
        # 얼굴이 오른쪽에 있으면 -> 카메라를 오른쪽으로 (d)
        # 얼굴이 왼쪽에 있으면 -> 카메라를 왼쪽으로 (a)
        if error_x > self.config.deadzone_x:
            self._send_command('d')  # 오른쪽으로
            print(f"[PAN] 오른쪽 이동 (error_x: {error_x})")
            command_sent = True
        elif error_x < -self.config.deadzone_x:
            self._send_command('a')  # 왼쪽으로
            print(f"[PAN] 왼쪽 이동 (error_x: {error_x})")
            command_sent = True
        
        # 수직 제어 (Tilt)
        # 얼굴이 아래에 있으면 -> 카메라를 아래로 (s)
        # 얼굴이 위에 있으면 -> 카메라를 위로 (w)
        if error_y > self.config.deadzone_y:
            self._send_command('s')  # 아래로
            print(f"[TILT] 아래 이동 (error_y: {error_y})")
            command_sent = True
        elif error_y < -self.config.deadzone_y:
            self._send_command('w')  # 위로
            print(f"[TILT] 위 이동 (error_y: {error_y})")
            command_sent = True
        
        if command_sent:
            self.last_command_time = current_time
    
    def _draw_overlay(self, frame, face, error_x, error_y, face_cx, face_cy):
        """화면에 추적 정보 오버레이"""
        x, y, w, h = face
        
        # 얼굴 박스
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
        
        # 얼굴 중심점
        cv2.circle(frame, (face_cx, face_cy), 5, (0, 255, 0), -1)
        
        # 화면 중심점
        cv2.circle(frame, (self.center_x, self.center_y), 5, (0, 0, 255), -1)
        
        # 중심 십자선
        cv2.line(frame, (self.center_x - 20, self.center_y), 
                 (self.center_x + 20, self.center_y), (0, 0, 255), 1)
        cv2.line(frame, (self.center_x, self.center_y - 20), 
                 (self.center_x, self.center_y + 20), (0, 0, 255), 1)
        
        # 데드존 표시
        dz_x, dz_y = self.config.deadzone_x, self.config.deadzone_y
        cv2.rectangle(frame, 
                      (self.center_x - dz_x, self.center_y - dz_y),
                      (self.center_x + dz_x, self.center_y + dz_y),
                      (255, 255, 0), 1)
        
        # 추적 선 (얼굴 중심 -> 화면 중심)
        cv2.line(frame, (face_cx, face_cy), 
                 (self.center_x, self.center_y), (255, 0, 255), 2)
        
        # 오차 정보 표시
        info_text = f"Error: X={error_x:+4d}, Y={error_y:+4d}"
        cv2.putText(frame, info_text, (10, 30), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)
        
        # 상태 표시
        if abs(error_x) <= self.config.deadzone_x and abs(error_y) <= self.config.deadzone_y:
            status = "LOCKED"
            color = (0, 255, 0)
        else:
            status = "TRACKING"
            color = (0, 165, 255)
        cv2.putText(frame, status, (10, 60), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, color, 2)
        
        return frame
    
    def _draw_no_face(self, frame):
        """얼굴 미검출 시 화면 표시"""
        cv2.putText(frame, "No Face Detected", (10, 30), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)
        
        # 중심 십자선
        cv2.line(frame, (self.center_x - 20, self.center_y), 
                 (self.center_x + 20, self.center_y), (0, 0, 255), 1)
        cv2.line(frame, (self.center_x, self.center_y - 20), 
                 (self.center_x, self.center_y + 20), (0, 0, 255), 1)
        
        return frame
    
    def run(self):
        """메인 추적 루프"""
        print("\n[시작] 얼굴 추적 시작...")
        print("  - 'q': 종료")
        print("  - 'c': 중앙으로 리셋")
        print("  - '+/-': 데드존 조절")
        print()
        
        try:
            while True:
                ret, frame = self.cap.read()
                if not ret:
                    print("[ERROR] 프레임 읽기 실패")
                    break
                
                # 좌우 반전 (거울 모드)
                frame = cv2.flip(frame, 1)
                
                # 얼굴 검출
                faces = self._detect_faces(frame)
                face = self._get_largest_face(faces)
                
                if face is not None:
                    # 오차 계산
                    error_x, error_y, face_cx, face_cy = self._calculate_error(face)
                    
                    # Pan-Tilt 제어
                    self._track_face(error_x, error_y)
                    
                    # 화면 표시
                    frame = self._draw_overlay(frame, face, error_x, error_y, face_cx, face_cy)
                else:
                    frame = self._draw_no_face(frame)
                
                # 조작 안내
                cv2.putText(frame, "Q:Quit  C:Center  +/-:Deadzone", 
                            (10, frame.shape[0] - 10),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, (200, 200, 200), 1)
                
                cv2.imshow('Face Tracking Pan-Tilt', frame)
                
                # 키 입력 처리
                key = cv2.waitKey(1) & 0xFF
                if key == ord('q'):
                    break
                elif key == ord('c'):
                    print("[CENTER] 중앙으로 리셋")
                    self._send_command('i')
                elif key == ord('+') or key == ord('='):
                    self.config.deadzone_x += 5
                    self.config.deadzone_y += 5
                    print(f"[DEADZONE] {self.config.deadzone_x}, {self.config.deadzone_y}")
                elif key == ord('-'):
                    self.config.deadzone_x = max(10, self.config.deadzone_x - 5)
                    self.config.deadzone_y = max(10, self.config.deadzone_y - 5)
                    print(f"[DEADZONE] {self.config.deadzone_x}, {self.config.deadzone_y}")
                    
        except KeyboardInterrupt:
            print("\n[종료] 사용자 인터럽트")
        finally:
            self.cleanup()
    
    def cleanup(self):
        """리소스 정리"""
        print("[정리] 리소스 해제 중...")
        
        # 중앙으로 복귀
        if self.serial_port and self.serial_port.is_open:
            self._send_command('i')
            time.sleep(0.3)
            self.serial_port.close()
            print("  - 시리얼 포트 닫힘")
        
        if self.cap:
            self.cap.release()
            print("  - 카메라 해제")
        
        cv2.destroyAllWindows()
        print("[완료] 종료됨")


def main():
    parser = argparse.ArgumentParser(
        description='Face Tracking Pan-Tilt Camera System',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
    python face_tracking_pantilt.py --port COM3
    python face_tracking_pantilt.py --port /dev/ttyUSB0 --baudrate 9600
    python face_tracking_pantilt.py --port COM3 --camera 1 --deadzone 30
        """
    )
    
    parser.add_argument('--port', '-p', required=True,
                        help='시리얼 포트 (예: COM3, /dev/ttyUSB0)')
    parser.add_argument('--baudrate', '-b', type=int, default=115200,
                        help='보레이트 (기본: 115200)')
    parser.add_argument('--camera', '-c', type=int, default=0,
                        help='카메라 ID (기본: 0)')
    parser.add_argument('--deadzone', '-d', type=int, default=50,
                        help='데드존 크기 (기본: 50 픽셀)')
    parser.add_argument('--interval', '-i', type=float, default=0.1,
                        help='명령 전송 간격 (기본: 0.1초)')
    
    args = parser.parse_args()
    
    # 설정 생성
    config = TrackingConfig(
        baudrate=args.baudrate,
        camera_id=args.camera,
        deadzone_x=args.deadzone,
        deadzone_y=args.deadzone,
        command_interval=args.interval
    )
    
    print("=" * 50)
    print("  Face Tracking Pan-Tilt Camera System")
    print("=" * 50)
    print(f"  포트: {args.port}")
    print(f"  보레이트: {config.baudrate}")
    print(f"  카메라: {config.camera_id}")
    print(f"  데드존: {config.deadzone_x} px")
    print("=" * 50)
    print()
    
    try:
        tracker = FaceTracker(args.port, config)
        tracker.run()
    except RuntimeError as e:
        print(f"[ERROR] {e}")
        return 1
    except Exception as e:
        print(f"[ERROR] 예외 발생: {e}")
        import traceback
        traceback.print_exc()
        return 1
    
    return 0


if __name__ == '__main__':
    exit(main())
```

