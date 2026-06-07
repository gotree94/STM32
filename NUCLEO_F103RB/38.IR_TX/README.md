# IR Transmitter Module KY-005

The KY-005 is an infrared (IR) transmitter module from the Keyes 37-in-1 Arduino kit (also sold as HW-489). At heart it’s a single 940 nm IR LED — drive it with the IRremote library and your Arduino can send the same 38 kHz remote-control signals as a TV or air-conditioner remote. This tutorial covers the pinout, wiring diagram, modern IRremote code, a KY-005 + KY-022 record-&-replay project, troubleshooting, and a Fritzing part download.

It works with Arduino, ESP32, ESP8266, Raspberry Pi and Pi Pico (3.3 V or 5 V logic). Pair it with the KY-022 IR receiver to capture and replay real remote codes.

KY-005 Infrared Transmitter Module Fritzing part
KEYES KY-005 Infrared transmitter module for Arduino
KY-005 Specifications
This module is quite simple and consists of a 5mm infrared LED and 3 male header pins. Handle with caution, do not flash IR light directly to the eyes.

Parameter	Value
Also known as	HW-489
Operating Voltage	3.3 – 5 V
IR LED Wavelength	940 nm
Carrier Frequency	38 kHz
Forward Current	30 – 60 mA
Power Consumption	90 mW
Operating Temperature	−10°C to +50°C
Dimensions	18.5 × 15 mm
KY-005 Pinout (HW-489)
The KY-005 has three 2.54 mm header pins. The signal pin is marked S; the unmarked middle pin is VCC, and the – pin is ground.

Pin	Label	Function	Connect to
1	S	Signal — IR carrier output	IRremote send pin (pin 3 on Uno)
2	middle (no label)	VCC	+5 V (or +3.3 V on ESP32/Pi)
3	–	Ground	GND
Good to know: the KY-005 is essentially a bare IR LED on a breakout — there’s no driver transistor on board. See how that affects range below.

KY-005 Wiring Diagram
Connect the board power line (middle) and ground (-) to +5 and GND on the Arduino respectively.

Connect the signal pin (S) to pin 3 on the Arduino Uno.

The pin number for the IR transmitter is determined by IRremote library. Other platforms might use a different pin.

KY-005	Arduino Uno
S	Pin 3
middle	+5V
–	GND
Arduino KY-005 connection diagram
KY-005 Arduino Code (IRremote)

This section uses IRremote v4.x (#include <IRremote.hpp>). If you have an older version installed, update it via the Arduino Library Manager. IRremote supports Sony, NEC, RC5, Samsung, and many more protocols out of the box.

Send an IR code
The IRremote library handles the protocol encoding — you supply the function name and the code values. The example below sends the Sony TV power code; swap sendSony for sendNEC, sendRC5, sendSamsung, and so on. Look up your device’s address and command values in an IR database such as LIRC, or capture them with a KY-022 IR receiver.

#include <IRremote.hpp>   // IRremote v4.x

#define IR_SEND_PIN 3     // KY-005 signal pin (S)

void setup() {
  IrSender.begin(IR_SEND_PIN);
}

void loop() {
  IrSender.sendSony(0xA90, 12);           // Sony TV power (hardcoded example)
  // IrSender.sendNEC(0x04, 0x08, 0);    // NEC — replace with your values
  delay(3000);
}
Protocol not recognised? If your remote’s protocol shows as UNKNOWN, use the Record & Replay project below — it captures raw IR timings that work with any device, no protocol knowledge needed.

Build an IR Record & Replay Remote (KY-005 + KY-022)
Capture any button from any IR remote using the KY-022 IR receiver, then replay it with the KY-005 transmitter. This approach captures raw IR timings — it works with any remote and any protocol, even ones IRremote does not recognise.

Wiring
Module	Pin	Arduino
KY-022 (receiver)	S	Pin 2
KY-022	middle (VCC)	+5 V
KY-022	–	GND
KY-005 (transmitter)	S	Pin 3
KY-005	middle (VCC)	+5 V
KY-005	–	GND
Step 1 — Capture raw timings with the KY-022
Upload this sketch, open the Serial Monitor at 9600 baud, and press the button you want to clone. Copy the uint16_t rawData[] array printed in the Serial Monitor.

#include <IRremote.hpp>   // IRremote v4.x

#define IR_RECEIVE_PIN 2  // KY-022 signal pin (S)

void setup() {
  Serial.begin(9600);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  Serial.println(F("Point a remote at the KY-022 and press a button:"));
}

void loop() {
  if (IrReceiver.decode()) {
    IrReceiver.printIRResultRawFormatted(&Serial, true);  // prints rawData[] array
    Serial.println();
    IrReceiver.resume();
  }
}
Step 2 — Replay with the KY-005
Paste the captured rawData[] array into the sketch below and upload it. The KY-005 will replay the exact same signal the original remote sent.

#include <IRremote.hpp>   // IRremote v4.x

#define IR_SEND_PIN 3     // KY-005 signal pin (S)

// Paste the rawData[] array printed by the capture sketch here
uint16_t rawData[] = {
  9000, 4500,
  560, 560, 560, 1690, 560, 560, 560, 560,
  560, 1690, 560, 560, 560, 1690, 560, 560,
  560
};

void setup() {
  IrSender.begin(IR_SEND_PIN);
}

void loop() {
  IrSender.sendRaw(rawData, sizeof(rawData) / sizeof(rawData[0]), 38);
  delay(3000);
}
Range and line-of-sight: keep the KY-005 within 1–3 metres and aim it directly at the device’s receiver window. Most IR receivers accept ±30°. If replay is unreliable, move closer or increase the delay() between sends.

KY-005 Range and the IR LED Driver
The KY-005 circuit is minimal: a 940 nm infrared LED and a current-limiting resistor, but no transistor driver. That matters for range.

An Arduino digital pin can source up to 40 mA, but the safe continuous limit is closer to 20–25 mA. For reliable operation at 5–10 metres, IR LEDs typically need 100–200 mA peak current. At 20–25 mA you will reliably cover 1–3 metres — enough for a TV remote pointed directly at the set. If your sketch sends codes but the device does not respond, short range is the most common cause.

Boosting range with a transistor
Add a small NPN transistor (2N2222 or BC547) between the Arduino and the KY-005 to drive the LED at full current:

Arduino pin → 1 kΩ resistor → transistor base
Transistor collector → KY-005 S pin
Transistor emitter → GND (KY-005 – pin also to GND)
KY-005 middle pin (VCC) → +5 V
The transistor saturates and pulls full supply current through the LED — typical result is 3× to 5× the range. For most remote-control use cases (sofa to TV) the bare module works fine without the transistor.

Tip: IR LEDs have a narrow beam. Keep the KY-005 aimed directly at the receiver — 30° off-axis can drop signal strength significantly. The IRremote library handles the 38 kHz carrier modulation automatically.

KY-005 Infrared Transmitter Applications
The KY-005 can control any device that responds to IR remote signals. Common use cases:

TV, air-conditioner and stereo control — automate power, volume, and input switching without modifying the device.
Home-automation IR bridge — combine with an ESP8266 or ESP32 to control IR devices over Wi-Fi or via a voice assistant.
Robot-to-robot communication — send simple one-way signals between robots over short distances without wires.
Simple IR data link — transmit sensor readings or commands between two Arduinos over a line-of-sight IR channel.
Troubleshooting & FAQ
Can the KY-005 control any IR device — TV, AC, fan?
Yes, as long as the device uses standard IR remote control. The IRremote library supports NEC, Sony, RC5, Samsung, and many more protocols. If your device ignores commands or you see wrong behaviour: verify the protocol and exact code values match your specific model (the same brand often uses different codes across models). For any unknown protocol, use the KY-022 capture sketch on this page to read codes directly from your original remote, then replay them with the KY-005.

Does the KY-005 transmit and receive?
No — it only transmits. The KY-005 is a single IR LED with no receiver circuit. To capture signals from an existing remote and replay them, you need a KY-022 IR receiver module alongside it.

What is the 38 kHz carrier frequency for?
Most IR remotes modulate their signal at 38 kHz so the receiver can distinguish it from ambient IR sources such as sunlight and incandescent heat. The IRremote library handles the 38 kHz modulation automatically — you just supply the protocol name and code value.

Can I use the KY-005 without the IRremote library?
Yes, but you would need to generate the 38 kHz carrier and all protocol timing manually in code. The IRremote library handles all of that, which is why it is the standard choice. Use it unless you have a specific reason not to.

Does the KY-005 work with ESP32 or Raspberry Pi Pico?
Yes — both accept 3.3 V logic, which the KY-005 supports. On ESP32, IRremote uses the RMT peripheral; initialise with IrSender.begin(IR_SEND_PIN) and avoid sharing the RMT channel with other libraries (e.g. FastLED). If there is a conflict, add #define NO_LED_FEEDBACK_CODE before the include. On Raspberry Pi Pico, use the Arduino-IRremote library compiled for RP2040 — the sketches on this page work without modification.

What is the difference between KY-005, KY-022, and KY-026?
KY-005 is an IR transmitter (940 nm LED). KY-022 is an IR receiver (demodulator) — together they make a complete send-and-receive pair. KY-026 is a flame detector — a completely different sensor that uses a different IR wavelength to detect fire, not remote signals.

Device doesn’t respond or the range is very short
The KY-005 has no transistor driver — at Arduino pin current (20–25 mA) it reliably covers 1–3 metres. First, move the KY-005 closer and aim it directly at the device’s receiver window (usually a small dark lens on the front panel). Also confirm the LED is actually emitting: point it at your smartphone’s front camera and trigger a send — you should see a purple/white flash on screen. No flash means a wiring or power issue. For longer range, add an NPN transistor (2N2222 or BC547) as described in the Range section above.

Signal sends intermittently — how do I diagnose it?
Start with wiring: loose header pins are the most common cause — reseat or resolder them. Next check for ambient IR interference: direct sunlight and some fluorescent lights can saturate the receiver; try shading it or moving indoors. Finally, confirm the LED is emitting by pointing it at a smartphone camera and triggering a send — a pull/white flash confirms emission. No flash = wiring or power problem, not a code problem.

Sketch won’t compile — error about IRremote.h
You have IRremote v2.x installed. The v4 API is not backwards-compatible. Update via Arduino IDE → Sketch → Include Library → Manage Libraries → search IRremote → install the latest 4.x version. Change the include from #include <IRremote.h> to #include <IRremote.hpp> (.hpp, not .h) and replace IRsend irsend; with IrSender.begin(IR_SEND_PIN) in setup().

Related modules: KY-022 IR Receiver — pairs directly with the KY-005 for capture and replay. Browse the Communication modules category for other wireless and signalling options.

---

# 📡 IR Remote Control Project (NEC Protocol) for STM32F103

* 이 프로젝트는 **STM32F103 (Nucleo-F103RB)** 보드와 IR 수신 센서를 이용하여,
* 적외선 리모컨의 신호를 분석하고 데이터를 UART로 출력하는 임베디드 실습 프로젝트입니다.

## 1. 적외선 송신 모듈

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

* 💡 송신 시 주의사항 (README 추가용)
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


---

작성하신 STM32F103 기반의 NEC 수신 코드가 아주 깔끔하게 잘 정리되어 있네요!수신 펌웨어가 Falling Edge 간의 시간 차이를 측정하는 방식이므로, 송신 펌웨어는 이에 정확히 대응되도록 물리적인 신호를 만들어주어야 합니다.여기서 가장 중요한 핵심은 적외선 리모컨 신호의 물리적 특성입니다. IR 수신 센서(주로 수신 모듈 내부)는 38kHz 주파수로 깜빡이는(Modulated) 빛이 들어올 때 LOW(0V)를 출력하고, 빛이 없을 때 HIGH(3,3V/5V)를 출력합니다.따라서 송신할 때는 이 반대로 신호를 쏘아주어야 수신부에서 정상적으로 인식합니다.1. ⚙️ 하드웨어 설정 (STM32 CubeMX)송신을 위해서는 38kHz 반송파(Carrier)를 생성할 타이머와 데이터 전송 타이밍(ms 단위)을 제어할 타이머가 필요합니다. 타이머 하나로 모두 처리하기 위해 TIM2를 사용한 PWM + Delay 방식을 제어하는 것이 가장 직관적입니다.TIM2 설정 (38kHz PWM 생성용 - PA0 또는 PA1 등 PWM 출력 핀)Clock Source: Internal Clock (64MHz 가정)Channel 1: PWM Generation CH1Prescaler (PSC): 0 (64MHz 그대로 사용)Counter Period (ARR): 1684 ($\frac{64MHz}{38kHz} \approx 1684$)Pulse (Duty Cycle): 842 (50% Duty Cycle 유지)TIM3 설정 (1µs 단위 마이크로초 딜레이용)Prescaler (PSC): 64-1 (1MHz 클럭 생성 -> 1카운트당 1µs)Counter Period (ARR): 655352. 💻 핵심 구현 코드 (송신 펌웨어)수신 코드의 데이터 분석 구조([Address(8)] [~Address(8)] [Command(8)] [~Command(8)])에 맞춰 32비트 데이터를 프레임으로 조립하고, NEC 규격에 맞게 PWM을 켜고 끄는 제어 코드입니다.

* 마이크로초 Delay 함수 및 PWM 제어 매크로

```C
/* USER CODE BEGIN PV */
extern TIM_HandleTypeDef htim2; // PWM용 (38kHz)
extern TIM_HandleTypeDef htim3; // 딜레이용 (1µs 카운팅)

// 1µs 단위 정밀 딜레이 함수
void delay_us(uint16_t us) {
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    while (__HAL_TIM_GET_COUNTER(&htim3) < us);
}

// IR LED 켜기 (38kHz PWM 출력 시작)
void IR_LED_ON(void) {
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}

// IR LED 끄기 (PWM 출력 정지 및 핀을 LOW로 유지)
void IR_LED_OFF(void) {
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
    // 안전을 위해 출력 핀 상태를 완전히 LOW로 고정 (CubeMX 설정에 맞춰 확인 필요)
}
/* USER CODE END PV */
```

* NEC 프로토콜 송신 함수

```C
/* USER CODE BEGIN 4 */
void NEC_Transmit(uint8_t address, uint8_t command) {
    // 1. 32비트 데이터 프레임 조립
    // 수신부 비트 시프트 방향(ir_data <<= 1)에 맞춰 MSB(최상위 비트)부터 전송하도록 배치
    uint32_t tx_data = 0;
    tx_data |= ((uint32_t)address) << 24;
    tx_data |= ((uint32_t)~address & 0xFF) << 16;
    tx_data |= ((uint32_t)command) << 8;
    tx_data |= ((uint32_t)~command & 0xFF);

    // 2. Lead Code 송신 (9ms ON + 4.5ms OFF)
    IR_LED_ON();
    delay_us(9000);
    IR_LED_OFF();
    delay_us(4500);

    // 3. 32비트 데이터 순차 송신 (MSB to LSB)
    for (int i = 31; i >= 0; i--) {
        uint8_t bit = (tx_data >> i) & 0x01;

        if (bit == 0) {
            // Logic '0': 0.56ms ON + 0.565ms OFF (총 1.125ms)
            IR_LED_ON();
            delay_us(560);
            IR_LED_OFF();
            delay_us(565);
        } else {
            // Logic '1': 0.56ms ON + 1.69ms OFF (총 2.25ms)
            IR_LED_ON();
            delay_us(560);
            IR_LED_OFF();
            delay_us(1690);
        }
    }

    // 4. Stop Bit 송신 (프레임의 끝을 알리는 0.56ms ON 후 종료)
    IR_LED_ON();
    delay_us(560);
    IR_LED_OFF();
}
/* USER CODE END 4 */
```

* 메인 루프 실행 예시 (1번 버튼 신호 반복 송신)
```C
  /* USER CODE BEGIN 2 */
  // 타이머 3 (딜레이용) 시작
  HAL_TIM_Base_Start(&htim3);
  /* USER CODE END 2 */

  /* USER CODE BEGIN WHILE */
  while (1)
  {
      // 예시: 2초마다 1번 버튼 데이터 (Addr: 0x20, Cmd: 0x88) 송신
      NEC_Transmit(0x20, 0x88);
      
      HAL_Delay(2000); 
      
    /* USER CODE END WHILE */
  }
```

3. 💡 구현 및 테스트 시 주의사항수신 데이터 비트 반전 확인: 보내시는 데이터 구조는 [Addr] [~Addr] [Cmd] [~Cmd] 형태입니다. 작성하신 수신부의 변환 코드(uint8_t address = (ir_data >> 24) & 0xFF;)에 완벽히 대응되도록 송신 코드 내에서 비트 연산(~address & 0xFF) 처리를 해두었으므로, NEC_Transmit(0x20, 0x88); 형태로 직관적으로 호출하시면 됩니다.트랜지스터 구동 회로 필수:STM32 GPIO의 출력 전류(최대 20mA 내외)만으로는 IR 송신 LED를 멀리까지 쏘아주지 못합니다. 수 센티미터 이상의 통신 거리를 확보하려면 NPN 트랜지스터(예: 2N2222)나 MOSFET을 사용하여 외부 전원으로 IR LED를 구동하는 회로를 구성하는 것이 좋습니다.

