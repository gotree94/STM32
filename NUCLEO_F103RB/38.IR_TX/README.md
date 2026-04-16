# 📡 IR Remote Control Project (NEC Protocol) for STM32F103

* 이 프로젝트는 **STM32F103 (Nucleo-F103RB)** 보드와 IR 수신 센서를 이용하여,
* 적외선 리모컨의 신호를 분석하고 데이터를 UART로 출력하는 임베디드 실습 프로젝트입니다.

## 1. 적외선 모듈

<img src="ir_rx_000.png" width="50%"></img><br>

<img src="TEK0002.BMP" width="50%"></img><img src="TEK0003.BMP" width="50%"></img><br>
<img src="TEK0004.BMP" width="50%"></img><img src="TEK0005.BMP" width="50%"></img><br>
<img src="TEK0006.BMP" width="50%"></img><br>


## 2. 📖 IR 통신 프로토콜 이론 (NEC Protocol)

* 가장 흔히 사용되는 **NEC 프로토콜**은 38kHz의 반송파(Carrier)에 실려 전송되며, **펄스 거리 부조화(Pulse Distance Width)** 방식을 사용하여 데이터를 구분합니다.

### 🔹 신호 구조 (Logical '0' vs '1')
NEC 프로토콜은 인터럽트 간의 시간 간격으로 비트를 판별합니다.
- **Lead Code:** 9ms High + 4.5ms Low (약 13.5ms의 간격)로 통신의 시작을 알림.
- **Logical '0':** 총 간격 약 **1.125ms**
- **Logical '1':** 총 간격 약 **2.25ms**

### 🔹 데이터 프레임 (32-bit)
1. **Address (8-bit):** 기기 식별 번호
2. **Address Inverse (8-bit):** 주소 오류 검출
3. **Command (8-bit):** 버튼 고유 기능 코드
4. **Command Inverse (8-bit):** 명령 오류 검출

### 🔹 🛰️ NEC 프로토콜 송신(TX) 프로세스

* 1단계: 동기화 단계 (Lead Code 생성)
   * 수신기가 데이터 전송을 준비할 수 있도록 거대한 "시작 신호"를 쏘는 과정입니다.
   * 물리적 신호: 9ms 동안 38kHz PWM 신호를 출력(Burst)한 후, 4.5ms 동안 신호를 완전히 끕니다(Space).
   * 송신 로직:
      * PWM_Start() 실행 후 9ms 대기
      * PWM_Stop() 실행 후 4.5ms 대기
   * 의미: 이 신호는 주변 노이즈와 실제 데이터를 구분하는 기준점이 되며, 수신기의 AGC(자동 이득 제어)를 설정하는 역할을 합니다.

* 2단계: 데이터 변조 단계 (Bit Encoding)
   * "Pulse Distance" 방식에 따라 비트 0과 1을 시간 간격의 차이로 만들어냅니다.
   * 공통 동작: 모든 비트의 시작은 562.5µs의 PWM Burst로 시작합니다.
   * Logic '0' 생성: Burst 이후 562.5µs 동안 Space(꺼짐) 상태를 유지합니다. (총 간격 1.125ms)
   * Logic '1' 생성: Burst 이후 1.6875ms 동안 Space(꺼짐) 상태를 유지합니다. (총 간격 2.25ms)
   * 송신 로직:
      * PWM_Start() → 562µs 대기 → PWM_Stop() → (비트 값에 따라 562µs 또는 1688µs 대기)
      * 기술적 특징: 수신기는 하강 엣지(Falling Edge) 사이의 시간을 측정하므로, 송신기는 다음 비트의 Burst 시작점까지의 거리를 조절하여 데이터를 실어 보냅니다.

* 3단계: 프레임 구성 및 종료 (Frame Assembly & Stop Bit)
   * 정해진 규격에 맞춰 32비트 데이터를 순차적으로 송출하고 마침표를 찍습니다.
   * 데이터 구성: [Address(8bit)] → [~Address(8bit)] → [Command(8bit)] → [~Command(8bit)] 순서로 총 32번 반복 송신합니다.
   * Stop Bit: 32비트 전송이 끝난 직후, 마지막 비트의 간격을 명시적으로 종료하기 위해 562.5µs의 최종 Burst를 한 번 더 쏘고 마무리합니다.
   * 의미: 마지막 비트가 1인지 0인지 확인하려면 마지막 하강 엣지가 필요하기 때문에, 종료 비트(Stop Bit)를 반드시 보내줘야 수신 측에서 32번째 비트를 정상적으로 처리할 수 있습니다.

💡 송신 시 주의사항 (README 추가용)
   * 반송파(Carrier): 단순히 LED를 켜는 것이 아니라 반드시 38kHz 주파수로 점멸시켜야 수신기가 인식할 수 있습니다.
   * 듀티 사이클(Duty Cycle): 38kHz PWM의 Duty는 보통 **1/3(33%) 또는 1/2(50%)**를 사용합니다. 33%를 사용하면 전력 소모를 줄이면서 도달 거리를 확보할 수 있습니다.
   * 엔디안(Endianness): 대부분의 NEC 리모컨은 각 바이트 내에서 LSB(최하위 비트)부터 먼저 송신합니다. (예: 0x88 송신 시 0비트부터 7비트 순으로 8번 전송)

---

## 3. 🛠️ 하드웨어 설정 (STM32 CubeMX)

  * TIM2 (PWM 생성용):
  	* Channel: PWM Generation CH1 (예: PA0 또는 PA5 등 보드에 따라 다름)
  	* Prescaler: 64-1 (64MHz 기준) -> $1\mu s$ 단위.
  	* Counter Period (ARR): 26 (약 $38kHz$ 주파수 생성: $1,000,000 / 38,000 \approx 26$)
  	* Pulse (Duty Cycle): 13 (50% Duty Cycle 설정)
  * TIM3 (타이밍 제어용):
  	* $\mu s$ 단위의 지연(Delay)을 만들기 위해 기존 수신용 설정을 그대로 사용하거나
  	* HAL_Delay보다 정밀한 delay_us 함수를 구현해야 합니다.

<img src="ir_tx_001.png" width="90%"> <br>
<img src="ir_tx_002.png" width="90%"> <br>
<img src="ir_tx_003.png" width="90%"> <br>
<img src="ir_tx_004.png" width="90%"> <br>

---

## 4. 💻 핵심 구현 코드

```c
void delay_us(uint16_t us);
void IR_Send_Bit(uint8_t bit);
void IR_Send_NEC(uint8_t addr, uint8_t cmd);
void PWM_Start(void);
void PWM_Stop(void);
```


```c
// 마이크로초 단위 지연 함수 (TIM3 활용)
void delay_us(uint16_t us) {
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    while (__HAL_TIM_GET_COUNTER(&htim3) < us);
}

// 38kHz PWM 신호를 켜고 끄는 함수
void PWM_Start(void) {
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}

void PWM_Stop(void) {
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
}

// NEC 프로토콜의 데이터 비트 전송
void IR_Send_Bit(uint8_t bit) {
    // 모든 비트는 562.5us의 Burst(PWM ON)로 시작
    PWM_Start();
    delay_us(562);
    PWM_Stop();

    if (bit) {
        // Logic '1': 1.6875ms Space
        delay_us(1688);
    } else {
        // Logic '0': 562.5us Space
        delay_us(562);
    }
}

// 전체 NEC 프레임 전송 함수
void IR_Send_NEC(uint8_t addr, uint8_t cmd) {
    // 1. Lead Code (9ms PWM ON + 4.5ms Space)
    PWM_Start();
    delay_us(9000);
    PWM_Stop();
    delay_us(4500);

    // 2. Address (8bit) 전송
    for (int i = 7; i >= 0; i--) IR_Send_Bit((addr >> i) & 1);
    // 3. Address Inverse (8bit) 전송
    for (int i = 7; i >= 0; i--) IR_Send_Bit(~(addr >> i) & 1);
    // 4. Command (8bit) 전송
    for (int i = 7; i >= 0; i--) IR_Send_Bit((cmd >> i) & 1);
    // 5. Command Inverse (8bit) 전송
    for (int i = 7; i >= 0; i--) IR_Send_Bit(~(cmd >> i) & 1);

    // 6. Stop Bit (최종 마무리 펄스)
    PWM_Start();
    delay_us(562);
    PWM_Stop();
}
```

```c
while (1)
  {
    // 사용자 버튼(B1)을 누르면 1번 버튼 데이터(Addr: 0x20, Cmd: 0x88) 전송
    if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
        IR_Send_NEC(0x20, 0x88);
        HAL_Delay(500); // 디바운싱 및 중복 전송 방지
    }
  }
```

### 5. 실행결과

* 데이터 형식을 보면 20DF(주소와 주소 반전)는 고정되어 있고, Cmd 부분과 그 뒤의 ~Cmd 부분이 변하고 있습니다.
  * Address: 0x20 (반전된 값 0xDF와 합쳐져 0x20DF 형성)
  * Command: 각 버튼의 고유값입니다.

<img src="ir_rx_001.png" width="50%"></img><br>

```
--- IR Receiver Ready ---
Raw: 0x20DF8877 | Addr: 0x20 | Cmd: 0x88
Raw: 0x20DF48B7 | Addr: 0x20 | Cmd: 0x48
Raw: 0x20DFC837 | Addr: 0x20 | Cmd: 0xC8
Raw: 0x20DF28D7 | Addr: 0x20 | Cmd: 0x28
Raw: 0x20DFA857 | Addr: 0x20 | Cmd: 0xA8
Raw: 0x20DF6897 | Addr: 0x20 | Cmd: 0x68
Raw: 0x20DFE817 | Addr: 0x20 | Cmd: 0xE8
Raw: 0x20DF18E7 | Addr: 0x20 | Cmd: 0x18
Raw: 0x20DF9867 | Addr: 0x20 | Cmd: 0x98
```

| 버튼	| Raw Data (32bit)	| Command (8bit)	| 비고 | 
|:---:|:---:|:---:|:---:|
| 1	| 0x20DF8877	| 0x88	|  | 
| 2	| 0x20DF48B7	| 0x48	| | 
| 3	| 0x20DFC837	| 0xC8	| | 
| 4	| 0x20DF28D7	| 0x28	| | 
| 5	| 0x20DFA857	| 0xA8	| | 
| 6	| 0x20DF6897	| 0x68	| | 
| 7	| 0x20DFE817	| 0xE8	| | 
| 8	| 0x20DF18E7	| 0x18	| | 
| 9	| 0x20DF9867	| 0x98  |  | 


