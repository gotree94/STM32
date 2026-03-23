/**
  ******************************************************************************
  * @file    06_lcd_compare.c
  * @brief   [SET2-⑥] I2C CLCD - 5가지 방식 DWT 사이클 비교
  * @board   NUCLEO-F411RE | I2C1 PB8/PB9 400kHz | UART 115200
  *
  * [측정 대상]  LCD_Write() 1회 (I2C 1바이트 전송) CPU 사이클
  *   ① HAL   HAL_I2C_Master_Transmit()
  *   ② LL    LL_I2C_GenerateStart/TransmitData8/Stop
  *   ③ CMSIS I2C1->CR1 / I2C1->DR 구조체 직접
  *   ④ REG   *(volatile uint32_t*)주소 직접
  *   ⑤ ASM   __asm volatile (LDR/STRB/TST/BEQ)
  *
  * [핵심 이해]
  *   I2C 버스 400kHz → 1바이트 전송시간 ≈ 25us (하드웨어 고정)
  *   CPU 사이클 차이 = 각 방식의 소프트웨어 오버헤드 차이
  *   LCD 표시 결과는 5가지 모두 동일
  *
  * [UART 출력 예시]
  *   ┌────────────────────────────────────────────────────┐
  *   │  STM32F411 I2C LCD - 5 Methods DWT Compare        │
  *   │  NUCLEO-F411RE | SYSCLK:84MHz | I2C:400kHz        │
  *   ├────────────────────────────────────────────────────┤
  *   │  # Method  I2C_Write 1byte     CPU cycles         │
  *   │  1 HAL     HAL_I2C_Master_Tx : 2150 cyc (25.6us) │
  *   │  2 LL      LL_I2C_*          :  980 cyc (11.7us) │
  *   │  3 CMSIS   I2C1->CR1/DR      :  850 cyc (10.1us) │
  *   │  4 REG     *(volatile*)       :  840 cyc (10.0us) │
  *   │  5 ASM     __asm volatile     :  800 cyc ( 9.5us) │
  *   └────────────────────────────────────────────────────┘
  *
  * [사용법] Core/Src/main.c 로 복사 후 빌드
  ******************************************************************************
  */
#include "main.h"
#include "stm32f4xx_ll_i2c.h"
#include <string.h>

/* ── 공통 상수 ──────────────────────────────────────────────────────────── */
#define RS0_EN1   0x04
#define RS0_EN0   0x00
#define RS1_EN1   0x05
#define RS1_EN0   0x01
#define BL        0x08
#define LCD_ADDR  (0x27<<1)

/* ── I2C1 레지스터 절대 주소 ────────────────────────────────────────────── */
#define REG32(a)          (*(volatile uint32_t *)(a))
#define I2C1_CR1_R        REG32(0x40005400UL)
#define I2C1_DR_R         REG32(0x4000540CUL)
#define I2C1_SR1_R        REG32(0x40005414UL)
#define I2C1_SR2_R        REG32(0x40005418UL)

#define I2C1_CR1_ADDR     0x40005400UL
#define I2C1_DR_ADDR      0x4000540CUL
#define I2C1_SR1_ADDR_C   0x40005414UL
#define I2C1_SR2_ADDR_C   0x40005418UL

#define CR1_START  (1U<<8)
#define CR1_STOP   (1U<<9)
#define SR1_SB     (1U<<0)
#define SR1_ADDR   (1U<<1)
#define SR1_BTF    (1U<<2)
#define SR1_TXE    (1U<<7)

/* ── DWT ────────────────────────────────────────────────────────────────── */
#define DWT_CYCCNT  REG32(0xE0001004UL)
#define DWT_CTRL    REG32(0xE0001000UL)
#define MEASURE_N   10

I2C_HandleTypeDef  hi2c1;
UART_HandleTypeDef huart2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void DWT_Init(void);
static void UART_Str(const char *s);
static void UART_ResultLine(int idx, const char *method, const char *api,
                            uint32_t cyc);

static void delay_us(uint32_t us){ volatile uint32_t d=us*3; while(d--); }
static void delay_ms(uint32_t ms){ HAL_Delay(ms); }

/* =========================================================================
 * 5가지 LCD_Write 구현
 * =========================================================================*/
static void Write_HAL(uint8_t d){
    while(HAL_I2C_Master_Transmit(&hi2c1,LCD_ADDR,&d,1,1000)!=HAL_OK);
}
static void Write_LL(uint8_t d){
    LL_I2C_GenerateStartCondition(I2C1);
    while(!LL_I2C_IsActiveFlag_SB(I2C1));
    LL_I2C_TransmitData8(I2C1,LCD_ADDR);
    while(!LL_I2C_IsActiveFlag_ADDR(I2C1));
    LL_I2C_ClearFlag_ADDR(I2C1);
    while(!LL_I2C_IsActiveFlag_TXE(I2C1));
    LL_I2C_TransmitData8(I2C1,d);
    while(!LL_I2C_IsActiveFlag_BTF(I2C1));
    LL_I2C_GenerateStopCondition(I2C1);
}
static void Write_CMSIS(uint8_t d){
    I2C1->CR1|=CR1_START;
    while(!(I2C1->SR1&SR1_SB));
    I2C1->DR=LCD_ADDR;
    while(!(I2C1->SR1&SR1_ADDR));
    (void)I2C1->SR1;(void)I2C1->SR2;
    while(!(I2C1->SR1&SR1_TXE));
    I2C1->DR=d;
    while(!(I2C1->SR1&SR1_BTF));
    I2C1->CR1|=CR1_STOP;
}
static void Write_REG(uint8_t d){
    I2C1_CR1_R|=CR1_START;
    while(!(I2C1_SR1_R&SR1_SB));
    I2C1_DR_R=LCD_ADDR;
    while(!(I2C1_SR1_R&SR1_ADDR));
    (void)I2C1_SR1_R;(void)I2C1_SR2_R;
    while(!(I2C1_SR1_R&SR1_TXE));
    I2C1_DR_R=d;
    while(!(I2C1_SR1_R&SR1_BTF));
    I2C1_CR1_R|=CR1_STOP;
}
static void Write_ASM(uint8_t d){
    /* START */
    __asm volatile(
        "LDR r0,=%0\n\t""LDR r1,[r0]\n\t""ORR r1,r1,%1\n\t""STR r1,[r0]\n\t"
        "LDR r0,=%2\n\t""1:\n\t""LDR r1,[r0]\n\t""TST r1,%3\n\t""BEQ 1b\n\t"
        ::"i"(I2C1_CR1_ADDR),"i"(CR1_START),"i"(I2C1_SR1_ADDR_C),"i"(SR1_SB)
        :"r0","r1","memory");
    /* ADDR */
    __asm volatile(
        "LDR r0,=%1\n\t""STRB %0,[r0]\n\t"
        "LDR r0,=%2\n\t""1:\n\t""LDR r1,[r0]\n\t""TST r1,%3\n\t""BEQ 1b\n\t"
        "LDR r1,[r0]\n\t""LDR r0,=%4\n\t""LDR r1,[r0]\n\t"
        ::"r"((uint32_t)LCD_ADDR),"i"(I2C1_DR_ADDR),
          "i"(I2C1_SR1_ADDR_C),"i"(SR1_ADDR),"i"(I2C1_SR2_ADDR_C)
        :"r0","r1","memory");
    /* DATA */
    __asm volatile(
        "LDR r0,=%1\n\t""1:\n\t""LDR r1,[r0]\n\t""TST r1,%2\n\t""BEQ 1b\n\t"
        "LDR r0,=%3\n\t""STRB %0,[r0]\n\t"
        "LDR r0,=%1\n\t""2:\n\t""LDR r1,[r0]\n\t""TST r1,%4\n\t""BEQ 2b\n\t"
        ::"r"((uint32_t)d),"i"(I2C1_SR1_ADDR_C),"i"(SR1_TXE),
          "i"(I2C1_DR_ADDR),"i"(SR1_BTF)
        :"r0","r1","memory");
    /* STOP */
    __asm volatile(
        "LDR r0,=%0\n\t""LDR r1,[r0]\n\t""ORR r1,r1,%1\n\t""STR r1,[r0]\n\t"
        ::"i"(I2C1_CR1_ADDR),"i"(CR1_STOP):"r0","r1","memory");
}

/* =========================================================================
 * LCD 공통 함수 (HAL 방식으로 초기화 후 비교 사용)
 * =========================================================================*/
typedef void (*WriteFunc)(uint8_t);

static void LCD_CMD_F(WriteFunc f, uint8_t c){
    uint8_t h=(c&0xF0)|RS0_EN1|BL; f(h); h=(c&0xF0)|RS0_EN0|BL; f(h); delay_us(4);
    h=((c<<4)&0xF0)|RS0_EN1|BL;    f(h); h=((c<<4)&0xF0)|RS0_EN0|BL; f(h); delay_us(50);
}
static void LCD_DATA_F(WriteFunc f, uint8_t d){
    uint8_t h=(d&0xF0)|RS1_EN1|BL; f(h); h=(d&0xF0)|RS1_EN0|BL; f(h); delay_us(4);
    h=((d<<4)&0xF0)|RS1_EN1|BL;    f(h); h=((d<<4)&0xF0)|RS1_EN0|BL; f(h); delay_us(50);
}
static void LCD_XY_F(WriteFunc f, uint8_t x, uint8_t y){
    const uint8_t r[]={0x80,0xC0,0x94,0xD4};
    if(y<4) LCD_CMD_F(f,r[y]+x);
}
static void LCD_PUTS_F(WriteFunc f, char *s){
    while(*s) LCD_DATA_F(f,(uint8_t)*s++);
}
static void LCD_INIT_HAL(void){
    delay_ms(100);
    uint8_t t; WriteFunc f=Write_HAL;
    t=((0x03<<4)&0xF0)|RS0_EN1|BL; f(t); t=((0x03<<4)&0xF0)|RS0_EN0|BL; f(t); delay_ms(5);
    t=((0x03<<4)&0xF0)|RS0_EN1|BL; f(t); t=((0x03<<4)&0xF0)|RS0_EN0|BL; f(t); delay_us(100);
    t=((0x03<<4)&0xF0)|RS0_EN1|BL; f(t); t=((0x03<<4)&0xF0)|RS0_EN0|BL; f(t); delay_us(100);
    t=((0x02<<4)&0xF0)|RS0_EN1|BL; f(t); t=((0x02<<4)&0xF0)|RS0_EN0|BL; f(t); delay_us(100);
    LCD_CMD_F(f,0x28); LCD_CMD_F(f,0x08); LCD_CMD_F(f,0x01); delay_ms(3);
    LCD_CMD_F(f,0x06); LCD_CMD_F(f,0x0C);
}

/* =========================================================================
 * 측정 결과 저장
 * =========================================================================*/
static uint32_t g_cyc[5];

static uint32_t min_of(uint32_t *a,int n){
    uint32_t m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m;
}

static void measure_all(void)
{
    uint32_t t0,t1,buf[MEASURE_N];
    uint8_t dummy=RS0_EN0|BL;   /* 더미 바이트: EN=0, RS=0, BL=1 */

    /* ① HAL */
    for(int i=0;i<MEASURE_N;i++){
        DWT_CYCCNT=0;t0=DWT_CYCCNT; Write_HAL(dummy); t1=DWT_CYCCNT; buf[i]=t1-t0;
    }
    g_cyc[0]=min_of(buf,MEASURE_N);

    /* ② LL */
    for(int i=0;i<MEASURE_N;i++){
        DWT_CYCCNT=0;t0=DWT_CYCCNT; Write_LL(dummy); t1=DWT_CYCCNT; buf[i]=t1-t0;
    }
    g_cyc[1]=min_of(buf,MEASURE_N);

    /* ③ CMSIS */
    for(int i=0;i<MEASURE_N;i++){
        DWT_CYCCNT=0;t0=DWT_CYCCNT; Write_CMSIS(dummy); t1=DWT_CYCCNT; buf[i]=t1-t0;
    }
    g_cyc[2]=min_of(buf,MEASURE_N);

    /* ④ REG */
    for(int i=0;i<MEASURE_N;i++){
        DWT_CYCCNT=0;t0=DWT_CYCCNT; Write_REG(dummy); t1=DWT_CYCCNT; buf[i]=t1-t0;
    }
    g_cyc[3]=min_of(buf,MEASURE_N);

    /* ⑤ ASM */
    for(int i=0;i<MEASURE_N;i++){
        DWT_CYCCNT=0;t0=DWT_CYCCNT; Write_ASM(dummy); t1=DWT_CYCCNT; buf[i]=t1-t0;
    }
    g_cyc[4]=min_of(buf,MEASURE_N);
}

int main(void)
{
    HAL_Init(); SystemClock_Config();
    MX_GPIO_Init(); MX_USART2_UART_Init(); MX_I2C1_Init();
    DWT_Init();
    HAL_Delay(200);

    UART_Str("\r\n┌────────────────────────────────────────────────────┐\r\n");
    UART_Str("│  STM32F411 I2C LCD - 5 Methods DWT Compare        │\r\n");
    UART_Str("│  NUCLEO-F411RE | SYSCLK:84MHz | I2C1:400kHz       │\r\n");
    UART_Str("│  측정: I2C_Write 1byte | 10회 최솟값              │\r\n");
    UART_Str("├────────────────────────────────────────────────────┤\r\n");
    UART_Str("│ # Method  API                    cycles   (us)    │\r\n");
    UART_Str("├────────────────────────────────────────────────────┤\r\n");

    LCD_INIT_HAL();
    LCD_XY_F(Write_HAL,0,0); LCD_PUTS_F(Write_HAL,"I2C LCD Compare");
    LCD_XY_F(Write_HAL,0,1); LCD_PUTS_F(Write_HAL,"Measuring...   ");

    measure_all();

    UART_ResultLine(1,"HAL  ","HAL_I2C_Master_Transmit()",g_cyc[0]);
    UART_ResultLine(2,"LL   ","LL_I2C_GenerateStart/TX  ",g_cyc[1]);
    UART_ResultLine(3,"CMSIS","I2C1->CR1/DR 구조체      ",g_cyc[2]);
    UART_ResultLine(4,"REG  ","*(volatile uint32_t*)     ",g_cyc[3]);
    UART_ResultLine(5,"ASM  ","__asm volatile(LDR/STRB) ",g_cyc[4]);
    UART_Str("└────────────────────────────────────────────────────┘\r\n\r\n");

    UART_Str("[핵심]\r\n");
    UART_Str(" I2C 버스 400kHz → 실제 전송시간은 모두 동일(~25us)\r\n");
    UART_Str(" 측정값 차이 = CPU 소프트웨어 오버헤드만의 차이\r\n");
    UART_Str(" LCD 표시 결과는 5가지 모두 동일\r\n\r\n");

    UART_Str("[STEP2] 5가지 방식 교대 LCD 출력\r\n");
    UART_Str("────────────────────────────────────────────────────\r\n\r\n");

    uint32_t loop=0; char buf[17];
    WriteFunc funcs[5]={Write_HAL,Write_LL,Write_CMSIS,Write_REG,Write_ASM};
    const char *names[5]={"HAL","LL ","CMS","REG","ASM"};
    const char *labels[5]={"HAL  I2C LCD","LL   I2C LCD","CMSIS I2C   ","REG  I2C LCD","ASM  I2C LCD"};

    while(1)
    {
        loop++;
        for(int m=0;m<5;m++){
            UART_Str("["); UART_Str(names[m]); UART_Str("] LCD write\r\n");
            LCD_XY_F(funcs[m],0,0); LCD_PUTS_F(funcs[m],(char*)labels[m]);
            snprintf(buf,sizeof(buf),"Loop%s:%4lu",names[m],loop);
            LCD_XY_F(funcs[m],0,1); LCD_PUTS_F(funcs[m],buf);
            HAL_Delay(1500);
        }
        UART_Str("[NOTE] LCD 출력 5가지 동일 → 차이는 CPU 사이클뿐\r\n\r\n");

        if(loop%5==0){
            UART_Str("=== 재측정 ===\r\n");
            measure_all();
            UART_ResultLine(1,"HAL  ","HAL_I2C_Master_Transmit()",g_cyc[0]);
            UART_ResultLine(2,"LL   ","LL_I2C_GenerateStart/TX  ",g_cyc[1]);
            UART_ResultLine(3,"CMSIS","I2C1->CR1/DR 구조체      ",g_cyc[2]);
            UART_ResultLine(4,"REG  ","*(volatile uint32_t*)     ",g_cyc[3]);
            UART_ResultLine(5,"ASM  ","__asm volatile(LDR/STRB) ",g_cyc[4]);
            UART_Str("==============\r\n\r\n");
        }
    }
}

/* =========================================================================
 * UART 출력 유틸
 * =========================================================================*/
static void UART_Str(const char *s){
    HAL_UART_Transmit(&huart2,(uint8_t*)s,strlen(s),HAL_MAX_DELAY);
}

static void UART_ResultLine(int idx, const char *method, const char *api, uint32_t cyc)
{
    char buf[80]; int p=0;
    char nc='0'+(char)idx;
    buf[p++]='│'; buf[p++]=' '; buf[p++]=nc; buf[p++]=' ';
    for(int i=0;method[i];i++) buf[p++]=method[i]; buf[p++]=' ';
    for(int i=0;api[i];i++)    buf[p++]=api[i]; buf[p++]=' ';

    /* cycles 우측정렬 5자리 */
    char tmp[12]; int n=0;
    if(cyc==0){tmp[n++]='0';}
    else{uint32_t x=cyc;while(x){tmp[n++]='0'+(char)(x%10);x/=10;}
         for(int i=0,j=n-1;i<j;i++,j--){char t=tmp[i];tmp[i]=tmp[j];tmp[j]=t;}}
    int pad=5-n; while(pad-->0) buf[p++]=' ';
    for(int i=0;i<n;i++) buf[p++]=tmp[i];

    /* us 환산 */
    uint32_t us=cyc/84, uf=(cyc%84)*10/84;
    const char *u1=" cyc ("; for(int i=0;u1[i];i++) buf[p++]=u1[i];
    n=0; uint32_t x=us;
    if(x==0){tmp[n++]='0';}
    else{while(x){tmp[n++]='0'+(char)(x%10);x/=10;}
         for(int i=0,j=n-1;i<j;i++,j--){char t=tmp[i];tmp[i]=tmp[j];tmp[j]=t;}}
    for(int i=0;i<n;i++) buf[p++]=tmp[i];
    buf[p++]='.'; buf[p++]='0'+(char)uf;
    const char *u2="us) │\r\n"; for(int i=0;u2[i];i++) buf[p++]=u2[i];
    buf[p]='\0';
    UART_Str(buf);
}

static void DWT_Init(void){
    CoreDebug->DEMCR|=CoreDebug_DEMCR_TRCENA_Msk;
    DWT_CYCCNT=0; DWT_CTRL|=DWT_CTRL_CYCCNTENA_Msk;
}
static void MX_I2C1_Init(void){
    hi2c1.Instance=I2C1; hi2c1.Init.ClockSpeed=400000;
    hi2c1.Init.DutyCycle=I2C_DUTYCYCLE_2; hi2c1.Init.OwnAddress1=0;
    hi2c1.Init.AddressingMode=I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode=I2C_DUALADDRESS_DISABLE; hi2c1.Init.OwnAddress2=0;
    hi2c1.Init.GeneralCallMode=I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode=I2C_NOSTRETCH_DISABLE; HAL_I2C_Init(&hi2c1);
}
void SystemClock_Config(void){
    RCC_OscInitTypeDef o={0}; RCC_ClkInitTypeDef c={0};
    __HAL_RCC_PWR_CLK_ENABLE(); __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    o.OscillatorType=RCC_OSCILLATORTYPE_HSI; o.HSIState=RCC_HSI_ON;
    o.HSICalibrationValue=RCC_HSICALIBRATION_DEFAULT; o.PLL.PLLState=RCC_PLL_ON;
    o.PLL.PLLSource=RCC_PLLSOURCE_HSI; o.PLL.PLLM=16; o.PLL.PLLN=336;
    o.PLL.PLLP=RCC_PLLP_DIV4; o.PLL.PLLQ=4; HAL_RCC_OscConfig(&o);
    c.ClockType=RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    c.SYSCLKSource=RCC_SYSCLKSOURCE_PLLCLK; c.AHBCLKDivider=RCC_SYSCLK_DIV1;
    c.APB1CLKDivider=RCC_HCLK_DIV2; c.APB2CLKDivider=RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&c,FLASH_LATENCY_2);
}
static void MX_GPIO_Init(void){
    GPIO_InitTypeDef g={0};
    __HAL_RCC_GPIOA_CLK_ENABLE(); __HAL_RCC_GPIOB_CLK_ENABLE(); __HAL_RCC_GPIOC_CLK_ENABLE();
    g.Pin=GPIO_PIN_5; g.Mode=GPIO_MODE_OUTPUT_PP; g.Pull=GPIO_NOPULL; g.Speed=GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA,&g);
}
static void MX_USART2_UART_Init(void){
    huart2.Instance=USART2; huart2.Init.BaudRate=115200;
    huart2.Init.WordLength=UART_WORDLENGTH_8B; huart2.Init.StopBits=UART_STOPBITS_1;
    huart2.Init.Parity=UART_PARITY_NONE; huart2.Init.Mode=UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl=UART_HWCONTROL_NONE; huart2.Init.OverSampling=UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart2);
}
void Error_Handler(void){__disable_irq();while(1){}}
