# STM32F411 iwth Arduino IDE

* https://www.arduino.cc/en/software/#ide

## Arduino IDE에서 STM32 코어 업데이트

### 1. 수동으로 보드 매니저 URL 확인
File → Preferences에서 Additional Boards Manager URLs에 아래 URL이 있는지 확인:

```
https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
```

<img width="1083" height="763" alt="001" src="https://github.com/user-attachments/assets/e205a678-5d0a-4df4-878b-a9a9a60887f8" />
<br>
<img width="1083" height="763" alt="002" src="https://github.com/user-attachments/assets/6df13f21-b44b-49ff-895e-224f7867073b" />
<br>
<img width="1081" height="705" alt="003" src="https://github.com/user-attachments/assets/60f1731a-227e-4bf5-bed4-3481c00bb5f0" />
<br>


## STM32 Arduino 코어 두 가지 버전
### 1. STM32 MCU based boards (2.11.0) - 공식 추천 ✅

* 제공: STMicroelectronics 공식
* 최신 버전: 2.x 계열
* 지원 보드: Nucleo-144, Nucleo-64, Nucleo-32, Discovery, Eval 등 광범위
* 특징:
   * ST 공식 지원 및 업데이트 활발
   * HAL/LL 라이브러리 기반
   * 최신 STM32 시리즈 지원
   * NUCLEO-F411RE 완벽 지원

### 2. STM8 MCU based boards (1.0.0) - STM8 전용
* 제공: STMicroelectronics
* 대상: STM8 시리즈 (8비트 마이크로컨트롤러)
* 지원 보드: Nucleo-8S208RB, Nucleo-8L152R8
* STM32와는 다른 아키텍처

### NUCLEO-F411RE를 위한 선택
   * STM32 MCU based boards (2.11.0)을 설치하세요!
   * 이것이 STM32F411 같은 32비트 ARM Cortex-M 기반 보드를 위한 패키지입니다.
### 설치 후 보드 선택
   * Tools → Board → STM32 MCU based boards → Nucleo-64
   * Tools → Board part number → Nucleo F411RE
 

<img width="1088" height="1503" alt="004" src="https://github.com/user-attachments/assets/b1eaad8c-b5a9-4769-ab79-d90d32fbc892" />

<img width="1085" height="1532" alt="005" src="https://github.com/user-attachments/assets/c312f522-2299-48b2-aa9d-a8adc837cbe9" />


## 업로드 설정

### 1. 필수 설정 확인
   * Tools → Board → STM32 MCU based boards → Nucleo-64
   * Tools → Board part number → Nucleo F411RE
   * Tools → Upload method → STM32CubeProgrammer (SWD)
   * Tools → Port → (NUCLEO 보드의 COM 포트 선택)

### 2. STM32CubeProgrammer 설치 확인
   * 만약 업로드 시 에러가 발생하면 STM32CubeProgrammer가 필요해요:

   * STM32CubeProgrammer 다운로드
   * 설치 후 Arduino IDE 재시작

### 간단한 테스트 예제
* 예제 1: Blink (LED 깜빡임)
* File → Examples → 01.Basics → Blink

```cpp
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);  // NUCLEO-F411RE는 PA5가 기본 LED
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}
```

* 업로드 후 **녹색 LED(LD2)**가 1초 간격으로 깜빡이면 성공!

* 예제 2: Serial 통신 테스트
```cpp
void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.println("Hello from NUCLEO-F411RE!");
  delay(1000);
}
```

* 업로드 후:
   * Tools → Serial Monitor 열기
   * 우측 하단 보드레이트 115200 설정
   * "Hello from NUCLEO-F411RE!" 메시지가 1초마다 출력되면 성공!


* 업로드 버튼 클릭!
: Arduino IDE 상단의 → (Upload) 버튼을 누르면:
   * 컴파일 진행
   * ST-Link를 통해 자동 업로드
   * 보드 자동 리셋 후 실행


* 다운로드 해결
  * https://www.st.com/en/development-tools/stm32cubeprog.html


<img width="1024" height="1365" alt="012" src="https://github.com/user-attachments/assets/6f9640d9-41f7-4103-8178-475521bf7ae1" />
<br><br>
<img width="1083" height="763" alt="006" src="https://github.com/user-attachments/assets/05e9bc39-fa72-4740-a7bd-fa5475b8914d" />
<br><br>
<img width="1083" height="763" alt="007" src="https://github.com/user-attachments/assets/097a4b07-e85e-48be-b8ea-a7529caeef81" />
<br><br>
<img width="1022" height="528" alt="008" src="https://github.com/user-attachments/assets/d1d20f50-1e0b-40ad-8c70-2faa4c01b293" />
<br><br>

```
Sketch uses 10256 bytes (1%) of program storage space. Maximum is 524288 bytes.
Global variables use 1092 bytes (0%) of dynamic memory, leaving 129980 bytes for local variables. Maximum is 131072 bytes.
STM32CubeProgrammer not found (STM32_Programmer_CLI.exe).
  Please install it or add '<STM32CubeProgrammer path>/bin' to your PATH environment:
  https://www.st.com/en/development-tools/stm32cubeprog.html
  Aborting!
Failed uploading: uploading error: exit status 1
```
<br><br>

<img width="1024" height="1365" alt="009" src="https://github.com/user-attachments/assets/8e8ca39e-6e4e-4672-9110-63eb8a8d434b" />
<br><br>
<img width="1024" height="1365" alt="010" src="https://github.com/user-attachments/assets/b294f540-1628-4c1c-8a1d-845cc5905237" />
<br><br>
<img width="659" height="374" alt="011" src="https://github.com/user-attachments/assets/c88dc2e0-37dd-4f8f-a474-28e8e29b085a" />
<br><br>

# Arduino IDE vs STM32CubeIDE 비교

## 1. 개발 철학 및 추상화 레벨

* Arduino:
  * 고수준 추상화 (HAL 위에 Arduino API 래퍼)
  * 하드웨어 독립적 코드 작성 가능
  * digitalWrite(), analogRead() 같은 간단한 함수
  * 빠른 프로토타이핑 중심

* CubeIDE:
  * 저수준 접근 (HAL/LL 라이브러리 직접 사용)
  * STM32 레지스터 직접 제어 가능
  * HAL_GPIO_WritePin(), __HAL_TIM_SET_COMPARE() 같은 상세 제어
  * 프로덕션 레벨 최적화 가능

## 2. 성능 및 최적화
* Arduino:
```cpp
digitalWrite(LED_BUILTIN, HIGH);  // 약 수십 사이클
```

* CubeIDE (HAL):
```c
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);  // 약 10-20 사이클
```

* CubeIDE (레지스터 직접):
```c
GPIOA->BSRR = GPIO_PIN_5;  // 1-2 사이클
```

## 3. 메모리 사용량

|항목|Arduino|CubeIDE|
|:--:|:--:|:--:|
|최소 프로그램 크기|~20-30KB|~5-10KB|
|RAM 오버헤드|상대적으로 높음|최소화 가능|
|부트로더|필요 (16-32KB)|선택적|

## 4. 주변장치 설정

**Arduino:**
```cpp
SPI.begin();  // 기본 핀으로 자동 설정
SPI.transfer(0x55);
```

**CubeIDE:**
```c
// CubeMX에서 그래픽으로 핀 설정
MX_SPI1_Init();  // 생성된 초기화 코드
HAL_SPI_Transmit(&hspi1, data, 1, 100);
```

## 5. 개발 도구
**Arduino:**
* 간단한 IDE
* 라이브러리 매니저로 쉬운 확장
* 디버거 없음 (Serial.print 디버깅)
* 빠른 컴파일/업로드

**CubeIDE:**
* Eclipse 기반 전문 IDE
* CubeMX 통합 (그래픽 핀 설정)
* 실시간 디버거 (breakpoint, watch)
* 코드 분석 도구
* FreeRTOS 통합

## 6. 실제 코드 비교 예제
* 타이머 PWM 설정:

**Arduino:**
```cpp
analogWrite(9, 128);  // 50% duty cycle
```

**CubeIDE:**
```c
// CubeMX에서 TIM3 CH4 설정 후
htim3.Instance = TIM3;
htim3.Init.Prescaler = 84-1;
htim3.Init.Period = 1000-1;
HAL_TIM_PWM_Init(&htim3);
HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 500);
```

### 7. **언제 무엇을 사용할까?**

**Arduino가 적합한 경우:**
- 빠른 프로토타이핑
- 센서/액추에이터 테스트
- 교육용 프로젝트
- 커뮤니티 라이브러리 활용
- 크로스 플랫폼 코드

**CubeIDE가 필수인 경우:**
- 실시간 성능이 중요한 경우 (모터 제어, 통신 프로토콜)
- 저전력 최적화
- 복잡한 주변장치 조합 (DMA, 다중 타이머, ADC 등)
- **RTOS 사용** (FreeRTOS)
- 프로덕션 제품 개발
- **디버깅이 필수적인 복잡한 로직**

### 8. **내부 구조**

**Arduino on STM32:**
```
Your Code
    ↓
Arduino API (digitalWrite, Serial, etc)
    ↓
STM32duino Core (핀 매핑, 추상화)
    ↓
STM32 HAL Library
    ↓
Hardware Registers
```

**CubeIDE:**
```
Your Code
    ↓
STM32 HAL/LL Library (직접 호출)
    ↓
Hardware Registers
```

## 실무 팁
* 많은 개발자들이 이렇게 사용해요:
   * Arduino로 프로토타입 → 동작 확인, 알고리즘 검증
   * CubeIDE로 최적화 → 성능/메모리 최적화, 디버깅, 양산


# NUCLEO-F411RE의 부트로더 상황

## 1. ST 공식 NUCLEO 보드의 경우
* NUCLEO 보드는 출하 시 부트로더 없이 나옵니다:
  * 0x08000000부터 사용자 코드 영역
  * ST-Link를 통해 직접 프로그래밍

## 2. Arduino로 업로드할 때
**Tools → Upload method → STM32CubeProgrammer (SWD)**를 선택했기 때문에:
```
Arduino IDE
    ↓
STM32CubeProgrammer
    ↓
ST-Link (SWD)
    ↓
Flash 0x08000000에 직접 업로드
```
* 부트로더를 거치지 않고 직접 업로드됩니다!

## 3. 부트로더가 필요한 경우
* 만약 Upload method를 다음 중 하나로 선택했다면 부트로더가 필요해요:
   * Serial (USB to TTL)
   * STM32 bootloader (DFU)
   * HID bootloader

* 이 경우에는:

```
0x08000000 - 0x0800XXXX: 부트로더
0x0800XXXX - 0x080FFFFF: 사용자 코드
```

## 4. 현재 상태 확인
* 현재 NUCLEO-F411RE는:

```
0x08000000: Arduino 스케치 시작 주소
           (부트로더 없음, 코드가 직접 실행됨)
```

* STM32CubeProgrammer로 Flash 내용을 확인하면:
   * 0x08000000부터 바로 vector table이 시작됨
   * Reset_Handler 주소가 바로 있음

## CubeIDE vs Arduino 차이점 (부트로더 관점)
* CubeIDE로 작업할 때:
```
0x08000000: 당신의 코드 시작
```

* 부트로더 없음
* ST-Link로 직접 프로그래밍

* 현재 Arduino 방식:
```
0x08000000: Arduino 스케치 시작
```

* 부트로더 없음 (SWD 업로드 사용)
* ST-Link로 직접 프로그래밍
* CubeIDE와 동일한 메모리 레이아웃!

* Arduino (Serial/DFU 업로드 시):
```
0x08000000: 부트로더
0x08008000: Arduino 스케치 시작
```

* USB 시리얼로 업로드 가능
* ST-Link 불필요

## 정리
* 현재 방식으로는 부트로더를 올리지 않았습니다
* ST-Link를 통해 Flash에 직접 업로드했어요
* CubeIDE로 작업하는 것과 메모리 구조는 동일합니다

* 차이점:
* CubeIDE: HAL 코드 직접 작성
* Arduino: Arduino API로 래핑된 코드 작성
* 둘 다 0x08000000부터 시작하는 동일한 메모리 레이아웃






















