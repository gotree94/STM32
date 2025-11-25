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

* NUCLEO-F411RE를 위한 선택
   * STM32 MCU based boards (2.11.0)을 설치하세요!
   * 이것이 STM32F411 같은 32비트 ARM Cortex-M 기반 보드를 위한 패키지입니다.
* 설치 후 보드 선택
   * Tools → Board → STM32 MCU based boards → Nucleo-64
   * Tools → Board part number → Nucleo F411RE
 



