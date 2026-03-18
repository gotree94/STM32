# STM32F411 GPIO 제어: HAL vs 인라인 어셈블러 비교

> **학습 목적**: NUCLEO-F411RE의 LD2(PA5) LED를 HAL 라이브러리와  
> GCC 인라인 어셈블러(`__asm volatile`)로 각각 제어하며  
> 두 방식의 동작 원리와 차이를 이해한다.

---

## 📋 보드 및 핀 정보

| 항목 | 내용 |
|------|------|
| 보드 | NUCLEO-F411RE |
| MCU | STM32F411RET6 (ARM Cortex-M4) |
| LED | **LD2 = PA5** (Active High) |
| 버튼 | B1 = PC13 (Active Low) |
| UART | USART2 / PA2(TX), PA3(RX) / 115200bps |
| SYSCLK | **84MHz** (HSI 16MHz → PLL: M=16, N=336, P=4) |

### 클럭 계산
```
SYSCLK = HSI × (PLLN / PLLM) / PLLP
       = 16MHz × (336 / 16) / 4
       = 84MHz
```

---

## 📁 파일 구성

```
STM32F411_GPIO_ASM/
├── Core/
│   ├── Inc/
│   │   └── main.h
│   └── Src/
│       ├── main_hal.c         ← 원본: HAL 방식 (CubeIDE 생성)
│       ├── main_asm.c         ← ASM 전용: GPIO Init + LED + Delay 모두 ASM
│       └── main_compare.c     ← 비교용: HAL 3회 → ASM 3회 교대 점멸
└── README.md
```

> **빌드 시 주의**: 세 파일 중 하나만 `Core/Src/`에 `main.c`로 복사하여 사용

---

## 📄 파일 1: HAL 방식 (원본 `main_hal.c`)

CubeIDE가 자동 생성한 코드 그대로 사용.

```c
/* ── 핵심 루프 ─────────────────────────────────────────────── */
while (1)
{
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 1);  /* PA5 High → ON  */
    HAL_Delay(500);
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);  /* PA5 Low  → OFF */
    HAL_Delay(500);
}
```

### HAL 내부 동작 (컴파일 결과 약 10~15 명령어)
```asm
; HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET) 의 컴파일 결과 (예시)
PUSH    {r3, lr}
LDR     r3, [r0, #0]         ; GPIOx->MODER 로드
CMP     r2, #0               ; GPIO_PIN_SET 여부 확인
BEQ     .reset_branch        ; 0이면 RESET 처리
LDR     r3, =GPIO_PIN_5
STR     r3, [r0, #24]        ; BSRR 쓰기 (오프셋 0x18)
POP     {r3, pc}
```

---

## 📄 파일 2: 인라인 어셈블러 전용 (`main_asm.c`)

GPIO 초기화(RCC + MODER), LED 제어(BSRR), 딜레이까지 **모두 `__asm volatile`** 로 구현.

### 레지스터 맵 (RM0383 기준)

```
RCC  베이스: 0x40023800
  + 0x30  = AHB1ENR   bit[0]: GPIOAEN (GPIOA 클럭)

GPIOA 베이스: 0x40020000
  + 0x00  = MODER     bit[11:10] = 01 → PA5 출력 모드
  + 0x18  = BSRR      bit[5]  = 1 → PA5 Set  (LED ON)
                       bit[21] = 1 → PA5 Reset (LED OFF)
```

### GPIO 초기화

```c
static void GPIO_Init_ASM(void)
{
    /* Step 1: GPIOA 클럭 활성화 (RCC->AHB1ENR |= 1) */
    __asm volatile (
        "LDR  r0, =%0          \n\t"   /* r0 = &RCC_AHB1ENR              */
        "LDR  r1, [r0]         \n\t"   /* r1 = 현재값 읽기               */
        "ORR  r1, r1, %1       \n\t"   /* r1 |= GPIOAEN (bit0)           */
        "STR  r1, [r0]         \n\t"   /* 레지스터 업데이트              */
        :
        : "i"(RCC_AHB1ENR), "i"(GPIOAEN_BIT)
        : "r0", "r1", "memory"
    );

    /* Step 2: PA5 출력 모드 (GPIOA->MODER[11:10] = 01) */
    __asm volatile (
        "LDR  r0, =%0          \n\t"   /* r0 = &GPIOA_MODER              */
        "LDR  r1, [r0]         \n\t"   /* r1 = 현재값 읽기               */
        "LDR  r2, =%1          \n\t"   /* r2 = 클리어 마스크 (3<<10)     */
        "BIC  r1, r1, r2       \n\t"   /* bit[11:10] 클리어              */
        "LDR  r2, =%2          \n\t"   /* r2 = 출력 값 (1<<10)           */
        "ORR  r1, r1, r2       \n\t"   /* bit[10] = 1 (출력 모드)        */
        "STR  r1, [r0]         \n\t"   /* MODER 업데이트                 */
        :
        : "i"(GPIOA_MODER), "i"(PA5_MODER_CLR), "i"(PA5_MODER_OUT)
        : "r0", "r1", "r2", "memory"
    );
}
```

### LED 제어

```c
/* LED ON: BSRR[5] = 1 → PA5 High */
static void LED_ON_ASM(void)
{
    __asm volatile (
        "LDR  r0, =%0          \n\t"   /* r0 = &GPIOA_BSRR               */
        "MOV  r1, %1           \n\t"   /* r1 = 0x20 (PA5_SET, ≤ 8bit)   */
        "STR  r1, [r0]         \n\t"   /* BSRR = 0x20 → PA5 High         */
        :
        : "i"(GPIOA_BSRR), "i"(PA5_SET)
        : "r0", "r1", "memory"
    );
}

/* LED OFF: BSRR[21] = 1 → PA5 Low */
static void LED_OFF_ASM(void)
{
    __asm volatile (
        "LDR  r0, =%0          \n\t"   /* r0 = &GPIOA_BSRR               */
        "LDR  r1, =%1          \n\t"   /* r1 = 0x00200000 (> 8bit → LDR) */
        "STR  r1, [r0]         \n\t"   /* BSRR = (1<<21) → PA5 Low       */
        :
        : "i"(GPIOA_BSRR), "i"(PA5_RESET)
        : "r0", "r1", "memory"
    );
}
```

### 딜레이 루프

```c
/* SYSCLK=84MHz, 약 3사이클/루프 → 500ms = 14,000,000 */
static void Delay_ASM(uint32_t count)
{
    __asm volatile (
        "1:                    \n\t"
        "SUBS %0, %0, #1       \n\t"   /* count--, 플래그 업데이트       */
        "BNE  1b               \n\t"   /* count≠0 → 레이블 1로 분기     */
        : "+r"(count)
        :
        : "cc"
    );
}
```

---

## 📄 파일 3: HAL ↔ ASM 비교 (`main_compare.c`)

두 방식을 같은 보드에서 교대 실행하여 동작 차이를 시각적으로 확인.

```
┌─────────────────────────────────────────────────────┐
│  [HAL 방식] LD2 3회 점멸 (200ms 간격, 빠름)         │
│  ────────────────────────────────────────────────── │
│  ON──OFF──ON──OFF──ON──OFF                          │
│  200ms 간격                                         │
├─────────────────────────────────────────────────────┤
│  1초 정지 (HAL/ASM 구분)                            │
├─────────────────────────────────────────────────────┤
│  [ASM 방식] LD2 3회 점멸 (500ms 간격, 느림)         │
│  ────────────────────────────────────────────────── │
│  ON────OFF────ON────OFF────ON────OFF                │
│  500ms 간격                                         │
├─────────────────────────────────────────────────────┤
│  1초 정지 → 처음부터 반복                           │
└─────────────────────────────────────────────────────┘
```

```c
/* ── 비교 루프 핵심 ───────────────────────────────────────── */
while (1)
{
    /* HAL 방식: 3회 × 200ms */
    for (int i = 0; i < 3; i++) {
        LED_ON_HAL();   HAL_Delay(200);
        LED_OFF_HAL();  HAL_Delay(200);
    }
    HAL_Delay(1000);

    /* ASM 방식: 3회 × 500ms */
    for (int i = 0; i < 3; i++) {
        LED_ON_ASM();   Delay_ASM(DELAY_500MS_COUNT);
        LED_OFF_ASM();  Delay_ASM(DELAY_500MS_COUNT);
    }
    HAL_Delay(1000);
}
```

---

## 🔬 GCC 인라인 어셈블러 문법 해설

### 기본 구조

```c
__asm volatile (
    "어셈블리 명령어\n\t"    /* \n = 줄바꿈  \t = 탭 (어셈블러 파서 구분용) */
    : 출력 피연산자          /* Output operands  ← C 변수로 결과 전달     */
    : 입력 피연산자          /* Input operands   ← C 값을 ASM으로 전달    */
    : 클로버 리스트          /* Clobber list     ← 변경된 레지스터 명시   */
);
```

### 제약 문자 (Constraint)

| 제약 | 의미 | 사용 예 |
|------|------|---------|
| `"r"` | 범용 레지스터 | `"r"(count)` |
| `"i"` | 컴파일 타임 상수 (즉시값) | `"i"(0x40020018)` |
| `"m"` | 메모리 피연산자 | `"m"(var)` |
| `"="` | 쓰기 전용 출력 | `"=r"(result)` |
| `"+"` | 읽기 + 쓰기 (입출력) | `"+r"(count)` |

### 클로버 리스트

| 클로버 | 의미 |
|--------|------|
| `"r0"` ~ `"r12"` | 해당 레지스터 값이 변경됨 |
| `"memory"` | 메모리가 변경됨 → 컴파일러에 캐시 무효화 강제 |
| `"cc"` | CPSR 조건 코드(플래그) 변경됨 |

> **`memory` 클로버가 중요한 이유**  
> 컴파일러는 최적화 시 메모리 읽기를 레지스터에 캐시한다.  
> `"memory"`를 명시하면 해당 ASM 블록 전후로 메모리를 반드시 다시 읽도록 강제한다.  
> GPIO 레지스터처럼 하드웨어가 값을 바꾸는 경우 필수.

### ARM Cortex-M4 주요 명령어 요약

```asm
LDR  rd, =imm32    ; 32비트 상수 → rd  (리터럴 풀, pseudo 명령)
LDR  rd, [rn]      ; 메모리[rn] → rd
STR  rs, [rn]      ; rs → 메모리[rn]
MOV  rd, #imm8     ; 8비트 즉시값 이동 (0~255 범위만 가능)
ORR  rd, rn, rm    ; rd = rn | rm   (비트 OR)
BIC  rd, rn, rm    ; rd = rn & ~rm  (비트 클리어, Bit Clear)
SUBS rd, rn, #1    ; rd = rn - 1, Zero/Carry 플래그 업데이트
BNE  label         ; Zero 플래그 = 0 이면 label로 분기
```

> **MOV vs LDR 선택 기준**  
> `MOV r1, #imm` : 즉시값이 8비트(0~255) 이하일 때만 사용 가능  
> `LDR r1, =imm`  : 32비트 상수, 주소값 등 큰 값에 사용 (리터럴 풀 활용)  
> → PA5_SET = 0x20 (32) → **MOV 가능**  
> → PA5_RESET = 0x00200000 → **LDR 필요**

---

## ⚖️ HAL vs 인라인 어셈블러 최종 비교

| 항목 | HAL 방식 | 인라인 어셈블러 |
|------|----------|----------------|
| **LED ON 명령 수** | ~10~15개 | **3개** (LDR+MOV+STR) |
| **코드 가독성** | ✅ 높음 | ⚠️ 낮음 |
| **이식성** | ✅ MCU 독립 | ❌ ARM Cortex-M 전용 |
| **실행 속도** | 보통 | ✅ 빠름 |
| **코드 크기** | 큼 (HAL 포함) | ✅ 최소 |
| **컴파일러 최적화** | 허용 | `volatile`로 제한 |
| **인터럽트 안전성** | ✅ BSRR 사용 | ✅ BSRR 사용 |
| **디버깅 난이도** | ✅ 쉬움 | ❌ 어려움 |
| **사용 시나리오** | 일반 개발 | 타이밍 크리티컬, 드라이버 최적화 |

### 실제 컴파일 결과 비교 (Disassembly View에서 확인 가능)

```asm
/* ── HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET) ─────── */
0x080001a4  PUSH   {r3, lr}
0x080001a6  CMP    r2, #0
0x080001a8  BEQ    0x080001b4
0x080001aa  LSLS   r1, r1, #0
0x080001ac  STR    r1, [r0, #24]     ; BSRR 쓰기
0x080001ae  POP    {r3, pc}
; (실제로는 분기 포함 더 많은 명령어 실행)

/* ── LED_ON_ASM() ──────────────────────────────────────────── */
0x08000210  LDR    r0, [pc, #8]      ; r0 = 0x40020018 (BSRR)
0x08000212  MOV    r1, #32           ; r1 = 0x20 (PA5_SET)
0x08000214  STR    r1, [r0, #0]      ; BSRR = 0x20 → PA5 High
; 끝. 딱 3 명령어.
```

---

## 🛠️ STM32CubeIDE 사용법

### 1. 프로젝트 생성

```
File → New → STM32 Project
  → Board Selector → NUCLEO-F411RE
  → Project Name: STM32F411_GPIO_ASM
  → Language: C
```

### 2. 파일 교체 방법

```
Core/Src/main.c  ← 아래 셋 중 하나를 복사하여 교체
  ├─ main_hal.c      (HAL 전용)
  ├─ main_asm.c      (ASM 전용)
  └─ main_compare.c  (비교용)
```

### 3. Disassembly View로 어셈블리 확인

```
디버그 실행 후:
  Window → Show View → Disassembly
  → HAL_GPIO_WritePin() 위에 커서 → 생성된 ARM 명령어 직접 확인
```

### 4. 빌드 옵션 확인

```
Project → Properties → C/C++ Build → Settings
  → MCU GCC Compiler → General
    CPU: -mcpu=cortex-m4
    FPU: -mfpu=fpv4-sp-d16 -mfloat-abi=hard
    Thumb: -mthumb
```

### 5. 인라인 어셈블러 주의사항

```c
/* ❌ volatile 없으면 최적화로 제거될 수 있음 */
__asm ("MOV r0, #1");

/* ✅ 항상 volatile 사용 */
__asm volatile ("MOV r0, #1" ::: "r0");

/* ❌ 클로버 미선언 → 컴파일러와 레지스터 충돌 */
__asm volatile ("LDR r0, =0x40020018 \n\t STR r1, [r0]");

/* ✅ 사용한 레지스터와 memory 반드시 클로버 명시 */
__asm volatile (
    "LDR r0, =0x40020018 \n\t"
    "STR r1, [r0]        \n\t"
    ::: "r0", "memory"
);
```

---

## 📐 딜레이 카운트 계산표

SYSCLK = 84MHz, SUBS+BNE ≈ 3사이클/루프

| 목표 시간 | 계산식 | count 값 |
|-----------|--------|----------|
| 1ms | 84,000,000 / 3 / 1000 | 28,000 |
| 100ms | 28,000 × 100 | 2,800,000 |
| 200ms | 28,000 × 200 | 5,600,000 |
| 500ms | 28,000 × 500 | 14,000,000 |
| 1000ms | 28,000 × 1000 | 28,000,000 |

> **주의**: 컴파일러 최적화 레벨(-O0, -O2 등)에 따라 사이클 수가 달라진다.  
> 정확한 딜레이가 필요하면 TIM 타이머 또는 HAL_Delay() 사용 권장.

---

## 📚 참고 자료

| 자료 | 링크 |
|------|------|
| STM32F411 Reference Manual | RM0383 (st.com) |
| NUCLEO-F411RE User Manual | UM1724 (st.com) |
| ARM Cortex-M4 TRM | developer.arm.com |
| GCC Inline Assembly HOWTO | ibiblio.org/gferg/ldp |
| STM32CubeIDE User Guide | st.com |

---

## 📖 학습 순서

```
1단계  원본 main_hal.c 빌드 → LD2 점멸 동작 확인
          ↓
2단계  RM0383에서 GPIO 레지스터 맵 확인
       (MODER / BSRR 오프셋 및 비트 의미 파악)
          ↓
3단계  main_asm.c 빌드 → LED_ON/OFF ASM 함수로 동일 동작 확인
          ↓
4단계  CubeIDE Disassembly View에서 두 방식 명령어 수 비교
          ↓
5단계  main_compare.c 빌드 → 200ms(HAL) / 500ms(ASM) 교대 점멸 확인
          ↓
6단계  로직 분석기 또는 오실로스코프로 PA5 파형 측정
       → HAL vs ASM 실제 토글 타이밍 차이 측정
```
