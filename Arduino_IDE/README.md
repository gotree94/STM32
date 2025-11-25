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






























