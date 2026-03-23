/**
  ******************************************************************************
  * @file    06_led_compare.c
  * @brief   [SET1-⑥] LD2 LED 제어 - 5가지 방식 DWT 사이클 비교
  * @board   NUCLEO-F411RE  |  LED: LD2 = PA5
  * @uart    USART2 PA2(TX) 115200bps → ST-Link Virtual COM
  * @sysclk  84MHz
  *
  * [비교 대상]
  *   ① HAL   HAL_GPIO_WritePin()
  *   ② LL    LL_GPIO_SetOutputPin()
  *   ③ CMSIS GPIOA->BSRR = pin
  *   ④ REG   *(volatile uint32_t*)addr = val
  *   ⑤ ASM   __asm volatile("LDR+MOV+STR")
  *
  * [측정 방법]
  *   DWT Cycle Counter: 각 방식 10회 측정 → 최솟값 채택
  *   최솟값: 캐시/파이프라인이 안정된 순수 실행 사이클
  *
  * [UART 출력 예시]
  *   ┌─────────────────────────────────────────────────────┐
  *   │  STM32F411 LED Control - 5 Methods DWT Compare     │
  *   │  NUCLEO-F411RE | SYSCLK:84MHz | PA5(LD2)           │
  *   ├─────────────────────────────────────────────────────┤
  *   │  # Method  Code                    SET    RESET    │
  *   │  1 HAL     HAL_GPIO_WritePin()   13cyc   11cyc    │
  *   │  2 LL      LL_GPIO_SetOutputPin   6cyc    6cyc    │
  *   │  3 CMSIS   GPIOA->BSRR=pin       4cyc    4cyc    │
  *   │  4 REG     *(volatile uint32_t*)  4cyc    4cyc    │
  *   │  5 ASM     __asm volatile(STR)    3cyc    3cyc    │
  *   └─────────────────────────────────────────────────────┘
  *
  * [사용법] Core/Src/main.c 로 복사 후 빌드
  *          시리얼 터미널 115200 연결 후 실행
  ******************************************************************************
  */
#include "main.h"
#include "stm32f4xx_ll_gpio.h"
#include <string.h>

/* ── 레지스터 주소 상수 ─────────────────────────────────────────────────── */
#define GPIOA_BSRR_ADDR     0x40020018UL
#define REG32(a)            (*(volatile uint32_t *)(a))
#define GPIOA_BSRR_REG      REG32(GPIOA_BSRR_ADDR)
#define PA5_SET             (1UL <<  5)
#define PA5_RESET           (1UL << 21)

/* ── DWT ────────────────────────────────────────────────────────────────── */
#define DWT_CYCCNT          REG32(0xE0001004UL)
#define DWT_CTRL            REG32(0xE0001000UL)
#define MEASURE_N           10

UART_HandleTypeDef huart2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void DWT_Init(void);
static void UART_Str(const char *s);
static void UART_Line(int idx, const char *method, const char *code,
                      uint32_t set_c, uint32_t rst_c);
static void print_u32_padded(char *buf, int *p, uint32_t v, int width);

/* ── 측정 결과 저장 ─────────────────────────────────────────────────────── */
static uint32_t g_set[5], g_rst[5];   /* [0]=HAL [1]=LL [2]=CMSIS [3]=REG [4]=ASM */

static uint32_t min_of(uint32_t *a, int n)
{
    uint32_t m = a[0];
    for (int i = 1; i < n; i++) if (a[i] < m) m = a[i];
    return m;
}

static void measure_all(void)
{
    uint32_t t0, t1, buf[MEASURE_N];

    /* ① HAL */
    for (int i = 0; i < MEASURE_N; i++) {
        DWT_CYCCNT=0; t0=DWT_CYCCNT;
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        t1=DWT_CYCCNT; buf[i]=t1-t0;
    }
    g_set[0] = min_of(buf, MEASURE_N);
    for (int i = 0; i < MEASURE_N; i++) {
        DWT_CYCCNT=0; t0=DWT_CYCCNT;
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        t1=DWT_CYCCNT; buf[i]=t1-t0;
    }
    g_rst[0] = min_of(buf, MEASURE_N);

    /* ② LL */
    for (int i = 0; i < MEASURE_N; i++) {
        DWT_CYCCNT=0; t0=DWT_CYCCNT;
        LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);
        t1=DWT_CYCCNT; buf[i]=t1-t0;
    }
    g_set[1] = min_of(buf, MEASURE_N);
    for (int i = 0; i < MEASURE_N; i++) {
        DWT_CYCCNT=0; t0=DWT_CYCCNT;
        LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
        t1=DWT_CYCCNT; buf[i]=t1-t0;
    }
    g_rst[1] = min_of(buf, MEASURE_N);

    /* ③ CMSIS */
    for (int i = 0; i < MEASURE_N; i++) {
        DWT_CYCCNT=0; t0=DWT_CYCCNT;
        GPIOA->BSRR = GPIO_PIN_5;
        t1=DWT_CYCCNT; buf[i]=t1-t0;
    }
    g_set[2] = min_of(buf, MEASURE_N);
    for (int i = 0; i < MEASURE_N; i++) {
        DWT_CYCCNT=0; t0=DWT_CYCCNT;
        GPIOA->BSRR = (uint32_t)GPIO_PIN_5 << 16U;
        t1=DWT_CYCCNT; buf[i]=t1-t0;
    }
    g_rst[2] = min_of(buf, MEASURE_N);

    /* ④ REG */
    for (int i = 0; i < MEASURE_N; i++) {
        DWT_CYCCNT=0; t0=DWT_CYCCNT;
        GPIOA_BSRR_REG = PA5_SET;
        t1=DWT_CYCCNT; buf[i]=t1-t0;
    }
    g_set[3] = min_of(buf, MEASURE_N);
    for (int i = 0; i < MEASURE_N; i++) {
        DWT_CYCCNT=0; t0=DWT_CYCCNT;
        GPIOA_BSRR_REG = PA5_RESET;
        t1=DWT_CYCCNT; buf[i]=t1-t0;
    }
    g_rst[3] = min_of(buf, MEASURE_N);

    /* ⑤ ASM */
    for (int i = 0; i < MEASURE_N; i++) {
        DWT_CYCCNT=0; t0=DWT_CYCCNT;
        __asm volatile(
            "LDR r0,=%0\n\t" "MOV r1,%1\n\t" "STR r1,[r0]\n\t"
            ::"i"(GPIOA_BSRR_ADDR),"i"(PA5_SET):"r0","r1","memory");
        t1=DWT_CYCCNT; buf[i]=t1-t0;
    }
    g_set[4] = min_of(buf, MEASURE_N);
    for (int i = 0; i < MEASURE_N; i++) {
        DWT_CYCCNT=0; t0=DWT_CYCCNT;
        __asm volatile(
            "LDR r0,=%0\n\t" "LDR r1,=%1\n\t" "STR r1,[r0]\n\t"
            ::"i"(GPIOA_BSRR_ADDR),"i"(PA5_RESET):"r0","r1","memory");
        t1=DWT_CYCCNT; buf[i]=t1-t0;
    }
    g_rst[4] = min_of(buf, MEASURE_N);
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    DWT_Init();
    HAL_Delay(200);

    /* ── 헤더 ──────────────────────────────────────────────────────────── */
    UART_Str("\r\n");
    UART_Str("┌─────────────────────────────────────────────────────┐\r\n");
    UART_Str("│  STM32F411 LED Control - 5 Methods DWT Compare     │\r\n");
    UART_Str("│  NUCLEO-F411RE | SYSCLK:84MHz | PA5(LD2)           │\r\n");
    UART_Str("│  측정: 10회 반복 → 최솟값 채택                     │\r\n");
    UART_Str("├──────────────────────────────────────────────────────┤\r\n");
    UART_Str("│ #  Method  코드                    SET     RESET    │\r\n");
    UART_Str("├──────────────────────────────────────────────────────┤\r\n");

    /* ── 측정 ──────────────────────────────────────────────────────────── */
    measure_all();

    UART_Line(1, "HAL  ", "HAL_GPIO_WritePin()   ", g_set[0], g_rst[0]);
    UART_Line(2, "LL   ", "LL_GPIO_SetOutputPin()", g_set[1], g_rst[1]);
    UART_Line(3, "CMSIS", "GPIOA->BSRR = pin     ", g_set[2], g_rst[2]);
    UART_Line(4, "REG  ", "*(volatile uint32_t*) ", g_set[3], g_rst[3]);
    UART_Line(5, "ASM  ", "__asm volatile(STR)   ", g_set[4], g_rst[4]);

    UART_Str("└──────────────────────────────────────────────────────┘\r\n\r\n");

    UART_Str("[설명]\r\n");
    UART_Str(" HAL  : assert+분기+BSRR → 가장 많은 명령어\r\n");
    UART_Str(" LL   : __STATIC_INLINE → 함수 호출 없이 BSRR 직접\r\n");
    UART_Str(" CMSIS: 구조체 오프셋 자동계산 → REG와 동일 어셈블리\r\n");
    UART_Str(" REG  : volatile 포인터 역참조 → CMSIS와 동일\r\n");
    UART_Str(" ASM  : LDR+MOV+STR 3명령어 → 이론적 최솟값\r\n\r\n");

    UART_Str("[STEP2] 5가지 방식 순서대로 LED 점멸 (각 3회 × 300ms)\r\n");
    UART_Str("────────────────────────────────────────────────────────\r\n\r\n");

    /* ── LED 점멸 루프 ──────────────────────────────────────────────────── */
    uint32_t loop = 0;
    while (1)
    {
        loop++;

        UART_Str("[1-HAL  ] 3x blink\r\n");
        for (int i=0;i<3;i++){
            HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET);   HAL_Delay(300);
            HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET); HAL_Delay(300);
        }

        UART_Str("[2-LL   ] 3x blink\r\n");
        for (int i=0;i<3;i++){
            LL_GPIO_SetOutputPin(GPIOA,LL_GPIO_PIN_5);   HAL_Delay(300);
            LL_GPIO_ResetOutputPin(GPIOA,LL_GPIO_PIN_5); HAL_Delay(300);
        }

        UART_Str("[3-CMSIS] 3x blink\r\n");
        for (int i=0;i<3;i++){
            GPIOA->BSRR=(uint32_t)GPIO_PIN_5;           HAL_Delay(300);
            GPIOA->BSRR=(uint32_t)GPIO_PIN_5<<16U;      HAL_Delay(300);
        }

        UART_Str("[4-REG  ] 3x blink\r\n");
        for (int i=0;i<3;i++){
            GPIOA_BSRR_REG=PA5_SET;   HAL_Delay(300);
            GPIOA_BSRR_REG=PA5_RESET; HAL_Delay(300);
        }

        UART_Str("[5-ASM  ] 3x blink\r\n");
        for (int i=0;i<3;i++){
            __asm volatile("LDR r0,=%0\n\t""MOV r1,%1\n\t""STR r1,[r0]\n\t"
                ::"i"(GPIOA_BSRR_ADDR),"i"(PA5_SET):"r0","r1","memory");
            HAL_Delay(300);
            __asm volatile("LDR r0,=%0\n\t""LDR r1,=%1\n\t""STR r1,[r0]\n\t"
                ::"i"(GPIOA_BSRR_ADDR),"i"(PA5_RESET):"r0","r1","memory");
            HAL_Delay(300);
        }

        UART_Str("[NOTE] 5가지 모두 동일하게 점멸 - 차이는 사이클 수\r\n\r\n");
        HAL_Delay(500);

        if (loop % 5 == 0) {
            UART_Str("=== 재측정 ===\r\n");
            measure_all();
            UART_Line(1,"HAL  ","HAL_GPIO_WritePin()   ",g_set[0],g_rst[0]);
            UART_Line(2,"LL   ","LL_GPIO_SetOutputPin()",g_set[1],g_rst[1]);
            UART_Line(3,"CMSIS","GPIOA->BSRR = pin     ",g_set[2],g_rst[2]);
            UART_Line(4,"REG  ","*(volatile uint32_t*) ",g_set[3],g_rst[3]);
            UART_Line(5,"ASM  ","__asm volatile(STR)   ",g_set[4],g_rst[4]);
            UART_Str("==============\r\n\r\n");
        }
    }
}

/* =========================================================================
 * UART 출력 유틸
 * =========================================================================*/
static void UART_Str(const char *s)
{
    HAL_UART_Transmit(&huart2,(uint8_t*)s,strlen(s),HAL_MAX_DELAY);
}

static void print_u32_padded(char *buf, int *p, uint32_t v, int width)
{
    char tmp[12]; int n=0;
    if(v==0){tmp[n++]='0';}
    else{uint32_t x=v; while(x){tmp[n++]='0'+(char)(x%10);x/=10;}
         for(int i=0,j=n-1;i<j;i++,j--){char t=tmp[i];tmp[i]=tmp[j];tmp[j]=t;}}
    int pad=width-n; while(pad-->0)buf[(*p)++]=' ';
    for(int i=0;i<n;i++)buf[(*p)++]=tmp[i];
}

static void UART_Line(int idx, const char *method, const char *code,
                      uint32_t set_c, uint32_t rst_c)
{
    /* "│ N  Method  code                    SSS cyc  RRR cyc │\r\n" */
    char buf[90]; int p=0;
    buf[p++]='│'; buf[p++]=' ';
    buf[p++]='0'+(char)idx; buf[p++]=' '; buf[p++]=' ';
    for(int i=0;method[i];i++) buf[p++]=method[i];
    buf[p++]=' ';
    for(int i=0;code[i];i++)   buf[p++]=code[i];
    buf[p++]=' ';
    print_u32_padded(buf,&p,set_c,4);
    const char *u1=" cyc  "; for(int i=0;u1[i];i++) buf[p++]=u1[i];
    print_u32_padded(buf,&p,rst_c,4);
    const char *u2=" cyc │\r\n"; for(int i=0;u2[i];i++) buf[p++]=u2[i];
    buf[p]='\0';
    UART_Str(buf);
}

static void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT_CYCCNT=0;
    DWT_CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct={0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct={0};
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    RCC_OscInitStruct.OscillatorType=RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState=RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue=RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState=RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource=RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM=16; RCC_OscInitStruct.PLL.PLLN=336;
    RCC_OscInitStruct.PLL.PLLP=RCC_PLLP_DIV4; RCC_OscInitStruct.PLL.PLLQ=4;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
    RCC_ClkInitStruct.ClockType=RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                               |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource=RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider=RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider=RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider=RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct,FLASH_LATENCY_2);
}
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef g={0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET);
    g.Pin=GPIO_PIN_5; g.Mode=GPIO_MODE_OUTPUT_PP;
    g.Pull=GPIO_NOPULL; g.Speed=GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA,&g);
}
static void MX_USART2_UART_Init(void)
{
    huart2.Instance=USART2; huart2.Init.BaudRate=115200;
    huart2.Init.WordLength=UART_WORDLENGTH_8B; huart2.Init.StopBits=UART_STOPBITS_1;
    huart2.Init.Parity=UART_PARITY_NONE; huart2.Init.Mode=UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl=UART_HWCONTROL_NONE; huart2.Init.OverSampling=UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart2);
}
void Error_Handler(void){__disable_irq();while(1){}}
