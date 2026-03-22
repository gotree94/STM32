# STM32F411 GPIO 제어 방식 총망라 비교

> **학습 목적**: STM32에서 제공하는 GPIO 제어 5가지 방식을  
> **DWT Cycle Counter**로 직접 측정해 성능·가독성·이식성을 비교한다.

---

## 📋 하드웨어 정보

| 항목 | 내용 |
|------|------|
| 보드 | NUCLEO-F411RE |
| MCU | STM32F411RET6 (ARM Cortex-M4, 84MHz) |
| LED | **LD2 = PA5** (Active High) |
| UART | USART2 / PA2(TX) / **115200 bps** → ST-Link Virtual COM |
| SYSCLK | **84MHz** (HSI 16MHz → PLL: M=16, N=336, P=4) |

---

## 🗂️ STM32 GPIO 제어 방식 계층 구조

```
┌─────────────────────────────────────────────────────────────────┐
│                     애플리케이션 코드                           │
├──────────────┬──────────────┬──────────────┬────────────────────┤
│   ① HAL      │   ② LL       │  ③ CMSIS     │  ④ REG  ⑤ ASM    │
│  고수준 API  │  저수준 API  │  ARM 표준    │  레지스터 직접    │
│  (ST 제공)   │  (ST 제공)   │  구조체      │  접근             │
├──────────────┴──────────────┴──────────────┴────────────────────┤
│                    하드웨어 레지스터                            │
│              GPIOA->BSRR  (0x40020018)                         │
└─────────────────────────────────────────────────────────────────┘

추상화 수준:  높음 ◄────────────────────────────────► 낮음
실행 속도:    느림 ◄────────────────────────────────► 빠름
코드 이식성:  높음 ◄────────────────────────────────► 낮음
```

---

## 📁 파일 구성

```
STM32F411_GPIO_Compare/
├── Core/
│   ├── Inc/
│   │   └── main.h
│   └── Src/
│       └── main_compare_all.c   ← 5가지 방식 비교 + DWT 측정
└── README.md
```

---

## 1️⃣ HAL (Hardware Abstraction Layer)

### 개요
ST Microelectronics가 제공하는 **고수준 추상화 API**.  
CubeIDE가 자동 생성하는 기본 방식.

### 코드

```c
/* LED ON */
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);

/* LED OFF */
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
```

### HAL 내부 동작 (stm32f4xx_hal_gpio.c)

```c
void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,
                       GPIO_PinState PinState)
{
    assert_param(IS_GPIO_PIN(GPIO_Pin));          /* 파라미터 검증   */
    assert_param(IS_GPIO_PIN_ACTION(PinState));   /* 상태값 검증     */

    if (PinState != GPIO_PIN_RESET)
        GPIOx->BSRR = GPIO_Pin;                  /* SET: bit[0..15] */
    else
        GPIOx->BSRR = (uint32_t)GPIO_Pin << 16U; /* RST: bit[16..31]*/
}
```

### 컴파일 결과 (Disassembly, -O2 기준, 약 6~13 명령어)

```asm
080001c4:  PUSH    {r3, lr}
080001c6:  CMP     r2, #0
080001c8:  BEQ     080001d0
080001ca:  STR     r1, [r0, #24]    ; BSRR SET
080001cc:  POP     {r3, pc}
080001ce:  ...
080001d0:  LSLS    r1, r1, #16
080001d2:  STR     r1, [r0, #24]    ; BSRR RESET
080001d4:  POP     {r3, pc}
```

### 특성

| 항목 | 내용 |
|------|------|
| 오버헤드 | 파라미터 검증 + 함수 호출 + 분기 |
| 이식성 | ✅ STM32 전 시리즈 동일 API |
| 가독성 | ✅ 가장 높음 |
| 에러처리 | ✅ assert_param 내장 |
| 사용 시나리오 | 일반 개발, 프로토타이핑 |

---

## 2️⃣ LL (Low Layer)

### 개요
STM32Cube 패키지에 포함된 **저수준 인라인 API**.  
HAL과 동일 패키지, `stm32f4xx_ll_gpio.h`에 **인라인 함수**로 정의.

### 코드

```c
#include "stm32f4xx_ll_gpio.h"

/* LED ON */
LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);

/* LED OFF */
LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
```

### LL 내부 정의 (stm32f4xx_ll_gpio.h)

```c
__STATIC_INLINE void LL_GPIO_SetOutputPin(GPIO_TypeDef *GPIOx,
                                          uint32_t PinMask)
{
    WRITE_REG(GPIOx->BSRR, PinMask);
    /* 펼치면: GPIOx->BSRR = PinMask; */
}

__STATIC_INLINE void LL_GPIO_ResetOutputPin(GPIO_TypeDef *GPIOx,
                                             uint32_t PinMask)
{
    WRITE_REG(GPIOx->BSRR, PinMask << 16U);
}
```

> `__STATIC_INLINE` → 컴파일러가 함수 호출 없이 **코드 삽입(inline)**  
> 함수 호출 오버헤드 제로, HAL 대비 절반 이하 사이클

### CubeIDE LL 활성화 방법

```
Project Manager → Advanced Settings
  GPIO → Driver: LL  선택
  (또는 #include "stm32f4xx_ll_gpio.h" 직접 포함)
```

### 특성

| 항목 | 내용 |
|------|------|
| 오버헤드 | 인라인 → 함수 호출 없음 |
| 이식성 | ✅ STM32 전 시리즈 (HAL과 동급) |
| 가독성 | ✅ 높음 (함수명 직관적) |
| 에러처리 | ❌ 없음 |
| 사용 시나리오 | 성능 필요 + 가독성 유지 |

---

## 3️⃣ CMSIS (Cortex Microcontroller Software Interface Standard)

### 개요
ARM이 정의한 **표준 레지스터 구조체** 직접 접근.  
HAL/LL 내부에서 실제로 사용하는 방식.  
`stm32f411xe.h`에 정의됨.

### 코드

```c
/* LED ON  - BSRR bit[5] = 1 */
GPIOA->BSRR = GPIO_PIN_5;

/* LED OFF - BSRR bit[21] = 1 */
GPIOA->BSRR = (uint32_t)GPIO_PIN_5 << 16U;
```

### CMSIS 구조체 정의 (stm32f411xe.h)

```c
typedef struct {
    __IO uint32_t MODER;    /* 0x00 : 핀 모드         */
    __IO uint32_t OTYPER;   /* 0x04 : 출력 타입       */
    __IO uint32_t OSPEEDR;  /* 0x08 : 속도            */
    __IO uint32_t PUPDR;    /* 0x0C : 풀업/풀다운     */
    __IO uint32_t IDR;      /* 0x10 : 입력 데이터     */
    __IO uint32_t ODR;      /* 0x14 : 출력 데이터     */
    __IO uint32_t BSRR;     /* 0x18 : 비트 SET/RESET  */
    __IO uint32_t LCKR;     /* 0x1C : 잠금            */
    __IO uint32_t AFR[2];   /* 0x20 : 대체 기능       */
} GPIO_TypeDef;

#define GPIOA_BASE  0x40020000UL
#define GPIOA       ((GPIO_TypeDef *) GPIOA_BASE)
```

> 컴파일러가 구조체 오프셋 자동 계산  
> `GPIOA->BSRR` → `*(volatile uint32_t *)(0x40020000 + 0x18)`

### 특성

| 항목 | 내용 |
|------|------|
| 오버헤드 | 거의 없음 (1~2 명령어) |
| 이식성 | ✅ ARM Cortex-M 표준 |
| 가독성 | ✅ 구조체 멤버명으로 직관적 |
| 에러처리 | ❌ 없음 |
| 사용 시나리오 | 드라이버 개발, 레지스터 학습 |

---

## 4️⃣ REG (절대주소 직접 접근, Bare Metal)

### 개요
`volatile` 포인터 역참조로 **하드웨어 레지스터 절대주소** 직접 쓰기.  
C 레벨에서 가능한 가장 원시적 방법.

### 코드

```c
/* 레지스터 주소 정의 */
#define GPIOA_BSRR  (*(volatile uint32_t *)0x40020018UL)
#define PA5_SET     (1UL <<  5)   /* 0x00000020 */
#define PA5_RESET   (1UL << 21)   /* 0x00200000 */

/* LED ON */
GPIOA_BSRR = PA5_SET;

/* LED OFF */
GPIOA_BSRR = PA5_RESET;
```

### CMSIS와 생성 어셈블리 비교

```c
/* CMSIS */  GPIOA->BSRR = GPIO_PIN_5;
/* REG   */  GPIOA_BSRR  = PA5_SET;

/* 두 코드 모두 동일한 어셈블리로 컴파일됨 */
LDR  r0, =0x40020018   ; BSRR 주소
MOV  r1, #0x20         ; PA5_SET 값
STR  r1, [r0]          ; 레지스터 쓰기
```

> **CMSIS vs REG 차이**: 기능·성능 동일, **이식성과 가독성**만 다름  
> - CMSIS: 구조체 이름으로 레지스터 접근 → 다른 STM32로 이식 가능  
> - REG: 절대 주소 하드코딩 → MCU 변경 시 주소 전부 수정 필요

### 특성

| 항목 | 내용 |
|------|------|
| 오버헤드 | CMSIS와 동일 (거의 없음) |
| 이식성 | ❌ 절대주소 하드코딩 |
| 가독성 | ⚠️ 숫자 주소 노출 |
| 에러처리 | ❌ 없음 |
| 사용 시나리오 | 레지스터 학습, 부트로더, CMSIS 없는 환경 |

---

## 5️⃣ ASM (인라인 어셈블러)

### 개요
GCC `__asm volatile`으로 **ARM Cortex-M4 명령어 직접 작성**.  
이론적 최소 명령어 수로 레지스터 제어.

### 코드

```c
/* LED ON: LDR + MOV + STR = 3 명령어 */
__asm volatile (
    "LDR  r0, =%0  \n\t"   /* r0 = &GPIOA_BSRR (리터럴 풀)  */
    "MOV  r1, %1   \n\t"   /* r1 = 0x20  (≤255 → MOV 가능)  */
    "STR  r1, [r0] \n\t"   /* BSRR = 0x20 → PA5 High         */
    :: "i"(0x40020018UL), "i"(1UL << 5)
    : "r0", "r1", "memory"
);

/* LED OFF: LDR + LDR + STR = 3 명령어 */
__asm volatile (
    "LDR  r0, =%0  \n\t"   /* r0 = &GPIOA_BSRR               */
    "LDR  r1, =%1  \n\t"   /* r1 = 0x00200000 (>255 → LDR)  */
    "STR  r1, [r0] \n\t"   /* BSRR = (1<<21) → PA5 Low       */
    :: "i"(0x40020018UL), "i"(1UL << 21)
    : "r0", "r1", "memory"
);
```

### MOV vs LDR 선택 이유

```
PA5_SET   = 0x00000020  (32)         ≤ 255 → MOV #imm8 가능
PA5_RESET = 0x00200000  (2,097,152)  > 255 → LDR =imm32 필요 (리터럴 풀)
```

### GCC 인라인 어셈블러 문법

```c
__asm volatile (
    "명령어 \n\t"       /* \n\t = 줄바꿈+탭 (어셈블러 파서 구분용)  */
    : 출력 피연산자     /* "=r"(변수): 결과를 C 변수로 전달          */
    : 입력 피연산자     /* "i"(상수): 컴파일 타임 상수 전달          */
    : 클로버 리스트     /* "r0","memory": 변경된 레지스터 명시       */
);
```

| 클로버 | 의미 |
|--------|------|
| `"r0"~"r12"` | 해당 레지스터 값 변경됨 |
| `"memory"` | 메모리/레지스터 쓰기 → 컴파일러 캐시 무효화 강제 |
| `"cc"` | CPSR 조건 코드 변경됨 (SUBS, TST 등) |

### 특성

| 항목 | 내용 |
|------|------|
| 오버헤드 | 이론적 최솟값 (3 명령어) |
| 이식성 | ❌ ARM Cortex-M 전용 |
| 가독성 | ❌ 가장 낮음 |
| 에러처리 | ❌ 없음 |
| 사용 시나리오 | 타이밍 크리티컬, 드라이버 최적화, 학습 |

---

## ⚖️ 5가지 방식 종합 비교

### DWT 측정 예상 결과 (SYSCLK = 84MHz, -O2 최적화)

```
──────────────────────────────────────────────────────────────
 #  방식    코드                         SET      RESET
──────────────────────────────────────────────────────────────
 1  HAL    HAL_GPIO_WritePin(SET)    :  13 cyc ( 0.15us)
 2  LL     LL_GPIO_SetOutputPin()   :   6 cyc ( 0.07us)
 3  CMSIS  GPIOA->BSRR = pin        :   4 cyc ( 0.05us)
 4  REG    *(volatile uint32_t*)    :   4 cyc ( 0.05us)
 5  ASM    __asm volatile(STR)      :   3 cyc ( 0.04us)
──────────────────────────────────────────────────────────────
 기준: ASM = 3 cycles = 0.036us @ 84MHz
```

> 실제 측정값은 최적화 레벨(-O0, -O1, -O2)과 캐시 상태에 따라 다름.  
> `main_compare_all.c` 실행 시 UART에 실측값 출력.

### 내부 컴파일 결과 비교 (Disassembly View)

```asm
/* ① HAL  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET) */
PUSH    {r3, lr}          ; 스택 저장        ← 함수 오버헤드
CMP     r2, #0            ; PinState 검사
BEQ     .reset            ; 분기
STR     r1, [r0, #24]     ; BSRR = GPIO_PIN_5
POP     {r3, pc}          ; 복귀

/* ② LL   LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5) */
STR     r1, [r0, #24]     ; BSRR = PinMask  (인라인 → 1명령어)

/* ③ CMSIS GPIOA->BSRR = GPIO_PIN_5 */
LDR     r3, =0x40020000   ; GPIOA 주소
MOV     r2, #32           ; GPIO_PIN_5 = 0x20
STR     r2, [r3, #24]     ; BSRR 쓰기

/* ④ REG  GPIOA_BSRR_REG = PA5_SET_VAL */
LDR     r3, =0x40020018   ; BSRR 절대주소
MOV     r2, #32           ; PA5_SET = 0x20
STR     r2, [r3]          ; BSRR 쓰기

/* ⑤ ASM  __asm volatile */
LDR     r0, =0x40020018   ; BSRR 주소 (리터럴풀)
MOV     r1, #32           ; PA5_SET = 0x20
STR     r1, [r0]          ; BSRR = 0x20
```

> ③ CMSIS와 ④ REG, ⑤ ASM의 생성 코드가 거의 동일함을 확인할 수 있음.

### 방식 선택 기준

```
┌──────────────────────────────────────────────────────────────┐
│  개발 단계             → ① HAL  (빠른 개발, CubeIDE 자동생성)│
│  성능 + 가독성 균형    → ② LL   (인라인, 오버헤드 최소)      │
│  드라이버 / 이식 코드  → ③ CMSIS (ARM 표준, RM0383 대응)     │
│  MCU 고유 코드         → ④ REG  (절대주소, CMSIS 없는 환경)  │
│  타이밍 크리티컬       → ⑤ ASM  (이론적 최소 사이클)         │
└──────────────────────────────────────────────────────────────┘
```

---

## 🛠️ 빌드 및 실행 방법

### 1. LL 헤더 포함 설정

```
방법 A: CubeIDE 자동 설정
  Project Manager → Advanced Settings
  GPIO → Driver: LL 선택 → Code Generate

방법 B: 수동 포함
  #include "stm32f4xx_ll_gpio.h"
  #include "stm32f4xx_ll_bus.h"
```

### 2. 파일 교체

```
Core/Src/main.c ← main_compare_all.c 복사
```

### 3. UART 연결

```
터미널: 115200, 8N1, No Flow Control
→ 실행 즉시 DWT 측정 결과 출력
```

### 4. Disassembly View 확인

```
디버그 실행(F11) →
Window → Show View → Disassembly →
LED_SET_HAL(), LED_SET_LL() 등에 커서 →
각 함수의 ARM 명령어 직접 확인
```

---

## 📐 최적화 레벨별 사이클 변화

컴파일러 최적화(-O0, -O1, -O2)에 따라 사이클 수가 크게 달라짐.

| 방식 | -O0 (없음) | -O1 | -O2 (기본) |
|------|-----------|-----|-----------|
| HAL | ~25 cyc | ~15 cyc | ~13 cyc |
| LL | ~10 cyc | ~6 cyc | ~6 cyc |
| CMSIS | ~8 cyc | ~5 cyc | ~4 cyc |
| REG | ~8 cyc | ~5 cyc | ~4 cyc |
| ASM | 3 cyc | 3 cyc | 3 cyc |

> ASM은 `__asm volatile`로 최적화 불가 → 항상 3 사이클 고정  
> HAL은 최적화에 의해 assert_param이 제거되어 크게 감소

**최적화 레벨 변경:**
```
Project → Properties → C/C++ Build → Settings
→ MCU GCC Compiler → Optimization
  -O0: 없음  -O1: 기본 최적화  -O2: 고급  -Os: 크기 최적화
```

---

## 📖 학습 권장 순서

```
Step 1   HAL 방식으로 LED 점멸 동작 확인 (CubeIDE 자동 생성)
            ↓
Step 2   LL 방식 추가 → Disassembly View에서 명령어 수 비교
            ↓
Step 3   CMSIS (GPIOA->BSRR) 방식 → RM0383 레지스터 맵 확인
            ↓
Step 4   REG (절대주소) 방식 → CMSIS와 동일 어셈블리 확인
            ↓
Step 5   ASM 방식 → LDR+STR 3 명령어 직접 확인
            ↓
Step 6   main_compare_all.c 빌드 → UART에서 DWT 실측값 확인
         → 5가지 방식의 사이클 차이를 숫자로 비교
```

---

## 📚 참고 자료

| 자료 | 내용 |
|------|------|
| RM0383 | STM32F411 레퍼런스 매뉴얼 (GPIO 레지스터 맵 p.147) |
| STM32CubeF4 HAL/LL 드라이버 문서 | stm32f4xx_hal_gpio.c / stm32f4xx_ll_gpio.h |
| ARM Cortex-M4 TRM | DWT/CoreDebug (사이클 카운터) |
| UM1724 | NUCLEO-F411RE 핀아웃 / 회로도 |
