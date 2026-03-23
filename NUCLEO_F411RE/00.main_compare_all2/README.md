# STM32F411 GPIO/I2C 제어 방식 완전 비교
## NUCLEO-F411RE | HAL / LL / CMSIS / REG / ASM

---

## 📋 하드웨어

| 항목 | 내용 |
|------|------|
| 보드 | NUCLEO-F411RE |
| MCU | STM32F411RET6 (ARM Cortex-M4, 84MHz) |
| LED | LD2 = PA5 (Active High) |
| LCD | HD44780 16×2 + PCF8574 I2C 모듈 |
| I2C | I2C1 / PB8(SCL), PB9(SDA) / 400kHz |
| UART | USART2 / PA2(TX) / 115200bps |
| SYSCLK | 84MHz (HSI 16MHz → PLL M=16, N=336, P=4) |

---

## 🗂️ 전체 파일 구성

```
STM32F411_Compare/
│
├── SET1_LED/                    ← PA5(LD2) GPIO 제어 5가지 방식
│   ├── 01_led_hal.c             HAL_GPIO_WritePin()
│   ├── 02_led_ll.c              LL_GPIO_SetOutputPin()
│   ├── 03_led_cmsis.c           GPIOA->BSRR = pin
│   ├── 04_led_reg.c             *(volatile uint32_t*)addr = val
│   ├── 05_led_asm.c             __asm volatile(LDR+MOV+STR)
│   └── 06_led_compare.c         5가지 DWT 사이클 측정 + UART 출력
│
├── SET2_LCD/                    ← I2C CLCD 5가지 방식
│   ├── 01_lcd_hal.c             HAL_I2C_Master_Transmit()
│   ├── 02_lcd_ll.c              LL_I2C_GenerateStart/TransmitData8
│   ├── 03_lcd_cmsis.c           I2C1->CR1 / I2C1->DR 구조체
│   ├── 04_lcd_reg.c             *(volatile uint32_t*)I2C주소
│   ├── 05_lcd_asm.c             __asm volatile(LDR/STRB/TST/BEQ)
│   └── 06_lcd_compare.c         5가지 DWT 사이클 측정 + UART 출력
│
└── README.md
```

> **빌드 방법**: 원하는 파일을 `Core/Src/main.c` 로 복사 후 빌드

---

## 📊 5가지 제어 방식 계층 구조

```
추상화 높음 ◄──────────────────────────────────► 추상화 낮음
실행 느림   ◄──────────────────────────────────► 실행 빠름
이식성 높음 ◄──────────────────────────────────► 이식성 낮음

  ① HAL       ② LL        ③ CMSIS      ④ REG        ⑤ ASM
ST 고수준   ST 저수준   ARM 표준    절대주소     ARM 명령어
  API          API         구조체      직접접근      직접작성
  
HAL_GPIO    LL_GPIO     GPIOA->     REG32(addr) __asm
_WritePin   _SetOutput  BSRR=pin    = val       volatile
()          Pin()                               (STR)
     │            │           │           │           │
     └────────────┴───────────┴───────────┴───────────┘
                         GPIOA BSRR 레지스터
                           (0x40020018)
```

---

## 🔵 SET 1: LED (GPIO PA5) 제어

### 방식별 코드 비교

```c
/* ① HAL - 파라미터 검증 + 분기 포함 → ~13 사이클 */
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

/* ② LL - __STATIC_INLINE → 함수 호출 없음 → ~6 사이클 */
LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);
LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);

/* ③ CMSIS - 구조체 오프셋 자동계산 → ~4 사이클 */
GPIOA->BSRR = GPIO_PIN_5;                   /* SET  */
GPIOA->BSRR = (uint32_t)GPIO_PIN_5 << 16U; /* RESET */

/* ④ REG - volatile 포인터 역참조 → ~4 사이클 (CMSIS와 동일) */
#define GPIOA_BSRR (*(volatile uint32_t *)0x40020018UL)
GPIOA_BSRR = (1UL << 5);    /* SET  */
GPIOA_BSRR = (1UL << 21);   /* RESET */

/* ⑤ ASM - LDR+MOV+STR 3명령어 → 3 사이클 (이론적 최솟값) */
__asm volatile(
    "LDR r0, =%0 \n\t"   /* BSRR 주소 로드              */
    "MOV r1, %1  \n\t"   /* 0x20 (≤255 → MOV 가능)     */
    "STR r1, [r0]\n\t"   /* BSRR = (1<<5) → PA5 High   */
    :: "i"(0x40020018UL), "i"(1UL<<5) : "r0","r1","memory");
```

### DWT 측정 예상값 (SYSCLK=84MHz, -O2)

```
┌──────────────────────────────────────────────────────┐
│ # Method  코드                    SET     RESET      │
│ 1 HAL     HAL_GPIO_WritePin()   13cyc   11cyc       │
│ 2 LL      LL_GPIO_SetOutputPin   6cyc    6cyc       │
│ 3 CMSIS   GPIOA->BSRR = pin      4cyc    4cyc       │
│ 4 REG     *(volatile uint32_t*)  4cyc    4cyc       │
│ 5 ASM     __asm volatile(STR)    3cyc    3cyc       │
└──────────────────────────────────────────────────────┘
※ 실측값은 06_led_compare.c 실행 시 UART 출력
```

### Disassembly 비교 (CubeIDE Window→Disassembly)

```asm
/* ① HAL  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET) */
PUSH    {r3, lr}          ← 함수 호출 오버헤드
CMP     r2, #0
BEQ     .L_reset
STR     r1, [r0, #24]    ← BSRR 쓰기 (오프셋 0x18)
POP     {r3, pc}

/* ② LL  LL_GPIO_SetOutputPin() → 인라인 후 */
STR     r1, [r0, #24]    ← BSRR 직접 (1명령어)

/* ③ CMSIS  GPIOA->BSRR = GPIO_PIN_5 */
LDR     r3, =0x40020000  ← GPIOA 베이스
MOV     r2, #32          ← GPIO_PIN_5
STR     r2, [r3, #24]    ← BSRR 쓰기

/* ④ REG  GPIOA_BSRR = PA5_SET */
LDR     r3, =0x40020018  ← BSRR 절대주소 (③과 동일 코드)
MOV     r2, #32
STR     r2, [r3]

/* ⑤ ASM  __asm volatile */
LDR     r0, =0x40020018  ← 리터럴 풀
MOV     r1, #32
STR     r1, [r0]
```

---

## 🟢 SET 2: I2C CLCD 제어

### PCF8574 핀 매핑

```
PCF8574  P7   P6   P5   P4   P3   P2   P1   P0
         D7   D6   D5   D4   BL   EN   RW   RS
비트     0x80 0x40 0x20 0x10 0x08 0x04 0x02 0x01
```

### I2C 1바이트 전송 시퀀스

```
① START 생성  →  ② 주소 전송(7bit+W)  →  ③ ACK 확인
→  ④ 데이터 전송  →  ⑤ BTF 확인  →  ⑥ STOP 생성
```

### 방식별 LCD_Write 구현

```c
/* ① HAL */
void Write_HAL(uint8_t d) {
    while(HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDR, &d, 1, 1000) != HAL_OK);
}

/* ② LL */
void Write_LL(uint8_t d) {
    LL_I2C_GenerateStartCondition(I2C1);
    while(!LL_I2C_IsActiveFlag_SB(I2C1));
    LL_I2C_TransmitData8(I2C1, LCD_ADDR);
    while(!LL_I2C_IsActiveFlag_ADDR(I2C1));
    LL_I2C_ClearFlag_ADDR(I2C1);
    while(!LL_I2C_IsActiveFlag_TXE(I2C1));
    LL_I2C_TransmitData8(I2C1, d);
    while(!LL_I2C_IsActiveFlag_BTF(I2C1));
    LL_I2C_GenerateStopCondition(I2C1);
}

/* ③ CMSIS */
void Write_CMSIS(uint8_t d) {
    I2C1->CR1 |= (1U<<8);                    /* START     */
    while(!(I2C1->SR1 & (1U<<0)));           /* SB 대기   */
    I2C1->DR = LCD_ADDR;
    while(!(I2C1->SR1 & (1U<<1)));           /* ADDR 대기 */
    (void)I2C1->SR1; (void)I2C1->SR2;        /* 클리어    */
    while(!(I2C1->SR1 & (1U<<7)));           /* TXE 대기  */
    I2C1->DR = d;
    while(!(I2C1->SR1 & (1U<<2)));           /* BTF 대기  */
    I2C1->CR1 |= (1U<<9);                    /* STOP      */
}

/* ④ REG (CMSIS와 동일 어셈블리, 주소를 명시적으로 표현) */
#define I2C1_CR1 (*(volatile uint32_t *)0x40005400UL)
#define I2C1_DR  (*(volatile uint32_t *)0x4000540CUL)
#define I2C1_SR1 (*(volatile uint32_t *)0x40005414UL)
#define I2C1_SR2 (*(volatile uint32_t *)0x40005418UL)

/* ⑤ ASM (SR1 폴링: TST+BEQ, DR 쓰기: STRB) */
__asm volatile(
    "LDR r0, =%0 \n\t"    /* &I2C1_CR1            */
    "LDR r1, [r0]\n\t"
    "ORR r1,r1,%1\n\t"    /* CR1 |= START(bit8)   */
    "STR r1, [r0]\n\t"
    "LDR r0, =%2 \n\t"    /* &I2C1_SR1            */
    "1:\n\t"
    "LDR r1, [r0]\n\t"    /* SR1 읽기             */
    "TST r1, %3  \n\t"    /* & SB(bit0)           */
    "BEQ 1b      \n\t"    /* SB=0 → 대기         */
    :: "i"(0x40005400UL), "i"(1U<<8),
       "i"(0x40005414UL), "i"(1U<<0)
    : "r0","r1","memory");
```

### I2C1 레지스터 맵 (RM0383)

```
주소         레지스터  중요 비트
──────────────────────────────────────────────────────
0x40005400   CR1       bit8:START  bit9:STOP  bit0:PE
0x4000540C   DR        bit[7:0]: 송수신 데이터
0x40005414   SR1       bit0:SB  bit1:ADDR  bit2:BTF  bit7:TXE
0x40005418   SR2       ADDR 클리어를 위한 더미 읽기
```

### DWT 측정 예상값 (SYSCLK=84MHz, I2C=400kHz)

```
┌───────────────────────────────────────────────────────┐
│ # Method  API                    cycles    (us)       │
│ 1 HAL     HAL_I2C_Master_Tx()  2150cyc  (25.6us)    │
│ 2 LL      LL_I2C_Generate*      980cyc  (11.7us)    │
│ 3 CMSIS   I2C1->CR1/DR          850cyc  (10.1us)    │
│ 4 REG     *(volatile uint32_t*) 840cyc  (10.0us)    │
│ 5 ASM     __asm volatile        800cyc   (9.5us)    │
└───────────────────────────────────────────────────────┘
※ 실측값은 06_lcd_compare.c 실행 시 UART 출력

[중요] I2C 버스 400kHz → 1바이트 실제 전송 ≈ 25us (하드웨어 고정)
       측정값 차이 = CPU 소프트웨어 처리 오버헤드만의 차이
       LCD 표시 결과는 5가지 방식 모두 동일
```

---

## ⚖️ 방식별 종합 비교

| 항목 | HAL | LL | CMSIS | REG | ASM |
|------|-----|----|-------|-----|-----|
| SET1 LED 사이클 | ~13 | ~6 | ~4 | ~4 | 3 |
| SET2 LCD 사이클 | ~2150 | ~980 | ~850 | ~840 | ~800 |
| 코드 이식성 | ✅최고 | ✅높음 | ✅ARM표준 | ❌낮음 | ❌없음 |
| 가독성 | ✅최고 | ✅높음 | ✅보통 | ⚠️낮음 | ❌최저 |
| 에러 처리 | ✅있음 | ❌없음 | ❌없음 | ❌없음 | ❌없음 |
| 컴파일러 최적화 | 허용 | 허용 | 허용 | 허용 | volatile로 고정 |
| 사용 시나리오 | 일반 개발 | 성능+가독성 | 드라이버 | 레지스터 학습 | 타이밍 크리티컬 |

### CMSIS ≈ REG인 이유

```c
/* 두 코드는 동일한 어셈블리로 컴파일됨 */
GPIOA->BSRR = GPIO_PIN_5;               /* CMSIS */
GPIOA_BSRR  = (1UL << 5);              /* REG   */

/* 컴파일 결과 */
LDR r3, =0x40020000    ; GPIOA 베이스
MOV r2, #32            ; GPIO_PIN_5
STR r2, [r3, #24]      ; BSRR 쓰기 (오프셋 0x18)
```

---

## 🛠️ 빌드 방법

### 1. 파일 교체

```
원하는 파일을 Core/Src/main.c 로 복사 후 빌드
각 파일은 독립 실행 가능 (main() 포함)
```

### 2. LL 헤더 설정 (02번 파일)

```
방법 A: CubeIDE → Project Manager → Advanced Settings
        GPIO/I2C → Driver: LL 선택
방법 B: 파일 상단에 직접 추가
        #include "stm32f4xx_ll_gpio.h"
        #include "stm32f4xx_ll_i2c.h"
```

### 3. LCD I2C 주소 확인

```c
/* 01_lcd_hal.c의 I2C_ScanAddresses()는 없으므로 아래로 확인 */
for (uint8_t i = 1; i < 128; i++) {
    if (HAL_I2C_IsDeviceReady(&hi2c1, i<<1, 1, 10) == HAL_OK)
        printf("Found: 0x%02X\r\n", i);
}
/* PCF8574: 0x27 / PCF8574A: 0x3F
   확인 후 LCD_ADDR 수정: #define LCD_ADDR (0x27<<1) */
```

### 4. UART 연결 (06번 비교 파일)

```
터미널: 115200, 8N1, No Flow Control
→ 실행 즉시 DWT 측정값 출력
```

### 5. Disassembly View

```
디버그 실행(F11) →
Window → Show View → Disassembly →
Write_HAL / Write_LL / Write_CMSIS 함수에 커서 →
ARM 명령어 직접 카운트
```

---

## 📖 학습 순서

```
[SET1 LED]
  01_led_hal.c  → 기준 동작 확인 (CubeIDE 원본 방식)
  02_led_ll.c   → LL 인라인: Disassembly에서 명령어 1개 확인
  03_led_cmsis.c → GPIOA->BSRR: RM0383 0x18 오프셋 확인
  04_led_reg.c  → 절대주소: CMSIS와 동일 어셈블리 확인
  05_led_asm.c  → LDR+MOV+STR 3명령어: 이론적 최솟값
  06_led_compare.c → UART에서 5가지 DWT 사이클 실측 비교

[SET2 LCD]
  01_lcd_hal.c  → LCD 동작 확인
  02_lcd_ll.c   → LL_I2C_* 함수 흐름: SB→ADDR→TXE→BTF
  03_lcd_cmsis.c → I2C 레지스터 직접: RM0383 Ch.18 대조
  04_lcd_reg.c  → 절대주소: CMSIS와 동일 어셈블리 확인
  05_lcd_asm.c  → ASM: TST+BEQ 폴링 + STRB 데이터 쓰기
  06_lcd_compare.c → UART에서 5가지 DWT 사이클 실측 비교
```

---

## 📚 참고 자료

| 자료 | 내용 |
|------|------|
| RM0383 | STM32F411 레퍼런스 매뉴얼 (GPIO p.147, I2C p.474) |
| UM1724 | NUCLEO-F411RE 핀아웃/회로도 |
| ARM Cortex-M4 TRM | DWT/CoreDebug 레지스터 |
| HD44780 Datasheet | LCD 초기화 시퀀스 Figure 24 |
| PCF8574 Datasheet | I2C 확장 포트 핀 매핑 |
