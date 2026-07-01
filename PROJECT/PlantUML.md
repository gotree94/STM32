# STM32F103 NUCLEO 펌웨어 교육 — PlantUML 도입 가이드

---

## 목차

1. [PlantUML 설치](#1-plantuml-설치)
2. [PlantUML로 시퀀스 다이어그램 작성](#2-plantuml로-시퀀스-다이어그램-작성)
3. [STM32 인터럽트/HAL 콜백 흐름 예제](#3-stm32-인터럽트hal-콜백-흐름-예제)
4. [수업 적용 체크리스트](#4-수업-적용-체크리스트)


---

## 1. PlantUML 설치

### 1-1. 사전 요구사항 — Java

```bash
# Ubuntu
sudo apt install default-jre -y
java -version
```

Windows는 https://www.java.com 에서 JRE 설치.

### 1-2. PlantUML 설치 방법 (3가지 중 택1)

**방법 A — VS Code 확장 (수업 진행 시 가장 추천)**

1. VS Code Extensions에서 **"PlantUML"** (jebbs 제작) 검색 후 설치
2. Graphviz가 이미 설치되어 있으면 (2장에서 설치함) 별도 설정 불필요
3. `.puml` 파일 작성 후 `Alt+D`로 미리보기 (Windows), `Option+D` (Mac)

**방법 B — JAR 파일 직접 실행**

```bash
# 다운로드
wget https://github.com/plantuml/plantuml/releases/latest/download/plantuml.jar

# 다이어그램 생성
java -jar plantuml.jar diagram.puml
# → diagram.png 생성됨
```

**방법 C — 온라인 에디터 (설치 없이 즉시 테스트용)**

- https://www.plantuml.com/plantuml/uml/
- 수업 첫 시간에 설치 없이 바로 시연할 때 유용

### 1-3. 설치 확인

```bash
java -jar plantuml.jar -version
```

---

## 2. PlantUML로 시퀀스 다이어그램 작성

### 2-1. 기본 문법

```plantuml
@startuml
participant 사용자
participant "STM32 MCU" as MCU
participant 센서

사용자 -> MCU : 전원 ON
MCU -> 센서 : I2C 초기화 요청
센서 --> MCU : ACK
MCU -> MCU : HAL_Init()
@enduml
```

### 2-2. 활성화 박스(Activation Bar) 사용 — 함수 실행 구간 표현

```plantuml
@startuml
participant Main
participant "TIM2 IRQHandler" as IRQ
participant "HAL_TIM_IRQHandler" as HAL
participant "User Callback" as CB

Main -> Main : HAL_TIM_Base_Start_IT()
...1ms 경과...
[-> IRQ : 타이머 오버플로우 인터럽트
activate IRQ
IRQ -> HAL : 호출
activate HAL
HAL -> CB : HAL_TIM_PeriodElapsedCallback()
activate CB
CB -> CB : servo_update_pwm()
deactivate CB
deactivate HAL
deactivate IRQ
@enduml
```

---

## 3. STM32 인터럽트/HAL 콜백 흐름 예제

UART 수신 인터럽트 예제 (수업에서 바로 사용 가능):

```plantuml
@startuml
title UART 수신 인터럽트 흐름 (HAL_UART_Receive_IT 기반)

participant "main()" as Main
participant "NVIC" as NVIC
participant "USART2_IRQHandler" as IRQH
participant "HAL_UART_IRQHandler" as HALIRQ
participant "HAL_UART_RxCpltCallback" as CB

Main -> Main : HAL_UART_Receive_IT(&huart2, buf, 1)
note right : 인터럽트 모드로 1바이트 수신 대기 등록

... 데이터 1바이트 도착 ...

NVIC -> IRQH : 인터럽트 발생, 벡터 진입
activate IRQH
IRQH -> HALIRQ : HAL_UART_IRQHandler(&huart2) 호출
activate HALIRQ
HALIRQ -> HALIRQ : 수신 레지스터(RDR) 읽기
HALIRQ -> CB : HAL_UART_RxCpltCallback(&huart2) 호출
activate CB
CB -> CB : 사용자 코드 실행 (buf 처리)
CB -> Main : HAL_UART_Receive_IT() 재호출 (다음 바이트 대기)
deactivate CB
deactivate HALIRQ
deactivate IRQH
@enduml
```

이 예제를 학생들에게 먼저 보여주고, 본인이 작업한 I2C/SPI 모듈로 동일한 형식의 다이어그램을 그려보게 하면 "이해도 검증 과제"로 활용할 수 있습니다.

---

## 4. 수업 적용 체크리스트

- [ ] 실습 PC에 Doxygen + Graphviz 설치 및 `dot -V` 동작 확인
- [ ] VS Code Doxygen 확장 설치 및 자동완성 동작 확인
- [ ] 샘플 HAL 프로젝트로 Doxyfile 설정 및 1회 문서 생성 시연
- [ ] `warnings.log` 확인 방법 안내 (자가 점검용)
- [ ] PlantUML VS Code 확장 설치 및 미리보기(`Alt+D`) 동작 확인
- [ ] UART 인터럽트 예제 다이어그램 함께 그려보기
- [ ] 실습 개별 과제: 본인 모듈의 Doxygen 주석 + 콜그래프 + 시퀀스 다이어그램 1개씩 제출

---

# smart_car PlantUML 문서화 가이드 (2조 샘플)

## 1. 컴포넌트 다이어그램 — 하드웨어 구성

```
@startuml
!include <C4/C4_Component>

System_Boundary(mcu, "STM32F103RBT6") {
  Component(spi1, "SPI1", "HAL", "160x80 ST7735S LCD")
  Component(usart2, "USART2", "HAL", "Debug Console (115200)")
  Component(usart3, "USART3", "HAL", "SLAM Data Output")
  Component(tim1_4, "TIM1-4", "HAL", "4ch x2 = 8ch PWM\n4WD Motor Control")
  Component(exti1, "EXTI1", "HAL", "IR Receiver (NEC)")
  Component(exti3_15, "EXTI3/15_10", "HAL", "Encoder Pulse Counter")
  Component(soft_i2c, "Soft I2C", "Bit-bang", "MPU6050 IMU")
  Component(soft_pwm, "Soft PWM", "DWT", "Servo (0-180deg)")
  Component(dwt, "DWT_CYCCNT", "Core", "us timing, pulse measurement")
}

Rel(spi1, "$lcd", "ST7735S TFT")
Rel(usart2, "$pc", "USB-Serial")
Rel(usart3, "$pc", "SLAM Mapper")
Rel(tim1_4, "$motors", "4x DC Motor")
Rel(exti1, "$ir", "IR Remote")
Rel(exti3_15, "$encoders", "2ch Encoder")
Rel(soft_i2c, "$mpu", "MPU6050 IMU")
Rel(soft_pwm, "$servo", "Servo")
Rel(dwt, "$ultrasonic", "2x HC-SR04")
@enduml
```

→ 생성물: components.png — MCU와 외부 HW 연결 관계도

## 2. 상태 다이어그램 — 전체 미션 흐름

```
@startuml
state IDLE : IR 리모컨 START 대기
state DRIVE : 10cm 전진 + 거리 측정
state SCAN : 초음파 센서 0-180deg 스윕
state ROTATE : 90도 회전 (자이로 유도)

[*] --> IDLE
IDLE --> DRIVE : IR start\n수신 (0xC2)
DRIVE --> SCAN : 이동 완료
SCAN --> DRIVE : 일반 step (step < 44)
SCAN --> ROTATE : 11번째 step마다
ROTATE --> DRIVE : 회전 완료
SCAN --> IDLE : step == 44 (완료)
@enduml
```
→ 생성물: state_machine.png — 미션 진행 상태 기계

## 3. 시퀀스 다이어그램 — 미션 1 Step

```
@startuml
actor IR_Remote
participant MCU
participant Motor
participant Encoder
participant MPU6050
participant Ultrasonic
participant Servo
participant LCD
participant SLAM_PC

IR_Remote -> MCU: NEC START code
MCU -> LCD: Show "주행"
MCU -> Motor: move_forward(duty=32767)
loop Encoder pulses
  Encoder -> MCU: EXTI interrupt
  MCU -> Motor: update encoder count
end
MCU -> Motor: stop()
MCU -> MPU6050: read accelerometer
MCU -> SLAM_PC: STEP,<no>,<dist>,...\n

MCU -> LCD: Show "스캔"
MCU -> Servo: sweep 0→180°
loop 91 steps (2° each)
  Servo -> MCU: angle settled
  MCU -> Ultrasonic: fire trigger
  Ultrasonic -> MCU: echo pulse
  MCU -> SLAM_PC: S,<angle>,<d1>,<d2>\n
end

alt step % 11 == 0 && step < 44
  MCU -> LCD: Show "회전"
  MCU -> Motor: turn_right()
  loop until yaw >= 90°
    MPU6050 -> MCU: read GyroZ
  end
  MCU -> Motor: stop()
  MCU -> SLAM_PC: ROT,<yaw>,<ax>,...\n
end

MCU -> LCD: Show "대기" (done)
@enduml
```

→ 생성물: mission_step.png — DRIVE → SCAN → (ROTATE) 1사이클

## 4. 액티비티 다이어그램 — 초음파 측정

```
@startuml
start
:TRIG 핀 10us HIGH;
repeat
  :ECHO 핀 폴링;
repeat while (ECHO == LOW) not timeout
:ECHO HIGH → start DWT counter;
repeat
  :ECHO 핀 폴링;
repeat while (ECHO == HIGH) not timeout
:ECHO LOW → stop DWT counter;
:거리 계산 = pulse_us / 58.0;
:x3 반복 → 중간값(median);
:반환 (cm);
stop
@enduml
```
→ 생성물: ultrasonic_measure.png — 초음파 측정 알고리즘

## 5. 시퀀스 다이어그램 — IR 리모컨 NEC 디코드

```
@startuml
participant IR_Receiver
participant EXTI1_Handler
participant DWT_Counter
participant Main_Loop

IR_Receiver -> EXTI1_Handler: falling edge interrupt
EXTI1_Handler -> DWT_Counter: capture timestamp
DWT_Counter --> EXTI1_Handler: delta_us
alt delta > 13000
  :leader code → reset frame
else delta > 2000
  :bit = 1
else delta > 1000
  :bit = 0
else
  :repeat/ignore
end
EXTI1_Handler -> Main_Loop: ir_ready = 1
Main_Loop -> EXTI1_Handler: read 32-bit ir_code
Note right: START command = 0xC2
@enduml
```

→ 생성물: ir_decode.png — NEC 프로토콜 디코딩

## 6. 타이밍 다이어그램 — 서보 PWM

```
@startuml
concise "PA11 (Servo)" as servo

@0
servo is HIGH : 500us (0°)
@500us
servo is LOW : 19500us
@20ms
servo is HIGH : 1500us (90°)
@21500us
servo is LOW : 18500us
@40ms
servo is HIGH : 2500us (180°)
@42500us
servo is LOW : 17500us
@60ms
@enduml
```

→ 생성물: servo_pwm.png — 서보 각도별 펄스 폭

## 7. 배치 다이어그램 — 물리 시스템 구성

```
@startuml
!include <C4/C4_Deployment>

Deployment_Node(nucleo, "NUCLEO-F103RB", "") {
  Container(mcu, "STM32F103RBT6", "ARM Cortex-M3, 64MHz")
}

Deployment_Node(sensors, "Sensor Board", "") {
  Component(ir, "IR Receiver", "PB1, EXTI")
  Component(eco1, "HC-SR04 #1", "PB12 (ECHO1)")
  Component(eco2, "HC-SR04 #2", "PB2 (ECHO2)")
  Component(mpu, "MPU6050", "PC4/PC5, Soft I2C")
}

Deployment_Node(actuators, "Actuator Board", "") {
  Component(motor_LF, "Left-Front Motor", "TIM2_CH2")
  Component(motor_LB, "Left-Back Motor", "TIM1_CH2")
  Component(motor_RF, "Right-Front Motor", "TIM3_CH1")
  Component(motor_RB, "Right-Back Motor", "TIM4_CH3")
  Component(servo, "Servo S90", "PA11, Soft PWM")
}

Deployment_Node(display, "Display", "") {
  Component(lcd, "ST7735S TFT", "160x80, SPI1")
}

Deployment_Node(pc, "PC (SLAM)", "") {
  Component(slam, "SLAM Processor", "USART3 @ 115200")
}
@enduml
```

→ 생성물: deployment.png — 부품별 물리적 배치

## 8. 전체 미션 시퀀스 — Step별 흐름

```
@startuml
|Main Loop|
repeat
  :step_no = 1 to 44;
  :test_distance_measure(step_no);
  |LCD|
  :Show "주행";
  |SLAM_PC|
  :STEP,no,dist,pulseL,pulseR,ax,ay,az,gz;
  |Main Loop|
  :stationary_scan();
  |LCD|
  :Show "스캔";
  |SLAM_PC|
  :S,angle,d1,d2 (91 lines);
  |Main Loop|
  if (step_no % 11 == 0 &&\nstep_no < 44) then (yes)
    |Main Loop|
    :test_rotate_90();
    |LCD|
    :Show "회전";
    |SLAM_PC|
    :ROT,yaw,ax,ay,az;
  endif
repeat while (step_no < 44) is (진행중)
-> (완료);
|LCD|
:Show "대기";
@enduml
```

→ 생성물: full_mission.png — 44 step 전체 흐름

---

# PlantUML 도구 설치 및 사용법

```
# VSCode 확장 설치
code --install-extension jebbs.plantuml

# 또는 PlantUML jar 직접 사용
# 1. Java 설치 후 PlantUML.jar 다운로드
# 2. 아래 명령으로 SVG/PNG 생성
java -jar plantuml.jar -tsvg diagram.puml
java -jar plantuml.jar -tpng diagram.puml
```

## 권장 작업 순서
1. 상태 다이어그램 (#2) — 전체 미션 구조 이해에 가장 기본
2. 컴포넌트 다이어그램 (#1) — 하드웨어 구성 파악
3. 시퀀스 다이어그램 (#3) — 1 step 실제 동작 흐름
4. 액티비티 다이어그램 (#4) — 초음파 측정 로직 상세
5. IR 디코드 (#5), 서보 (#6) — 주변장치별 상세
6. 배치 (#7) — 최종 보고서용
