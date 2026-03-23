/**
  ******************************************************************************
  * @file    05_lcd_asm.c
  * @brief   [SET2-⑤] I2C CLCD 제어 - 인라인 어셈블러(ASM) 방식
  * @board   NUCLEO-F411RE | I2C1 PB8/PB9 400kHz
  *
  * [ASM I2C 방식]
  *   I2C 레지스터 접근 + 상태 폴링 모두 __asm volatile 로 구현
  *
  * [사용 명령어]
  *   LDR  r0, =addr  : 32비트 주소 로드
  *   LDR  r1, [r0]   : 레지스터 읽기
  *   STR  r1, [r0]   : 레지스터 쓰기
  *   STRB r1, [r0]   : 1바이트 쓰기 (DR 레지스터용)
  *   ORR  r1,r1,#n   : 비트 세트
  *   TST  r1, #n     : 비트 확인 (AND → Z 플래그)
  *   BEQ  1b         : Z=1(비트 0)이면 backward 레이블로 분기
  *
  * [사용법] Core/Src/main.c 로 복사 후 빌드
  ******************************************************************************
  */
#include "main.h"
#include <stdio.h>
#include <string.h>

#define RS0_EN1   0x04
#define RS0_EN0   0x00
#define RS1_EN1   0x05
#define RS1_EN0   0x01
#define BL        0x08
#define LCD_ADDR  (0x27<<1)

/* I2C1 절대 주소 (ASM 즉시값으로 사용) */
#define I2C1_CR1_ADDR   0x40005400UL
#define I2C1_DR_ADDR    0x4000540CUL
#define I2C1_SR1_ADDR   0x40005414UL
#define I2C1_SR2_ADDR   0x40005418UL

#define I2C_CR1_START   (1U<<8)
#define I2C_CR1_STOP    (1U<<9)
#define I2C_SR1_SB      (1U<<0)
#define I2C_SR1_ADDR_F  (1U<<1)
#define I2C_SR1_BTF     (1U<<2)
#define I2C_SR1_TXE     (1U<<7)

I2C_HandleTypeDef  hi2c1;
UART_HandleTypeDef huart2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void delay_us(uint32_t us){ volatile uint32_t d=us*3; while(d--); }
static void delay_ms(uint32_t ms){ HAL_Delay(ms); }

/* =========================================================================
 * I2C_Start_ASM  : CR1|=START → SR1:SB 폴링
 * =========================================================================*/
static void I2C_Start_ASM(void)
{
    __asm volatile (
        "LDR  r0, =%0          \n\t"   /* r0 = &I2C1_CR1                  */
        "LDR  r1, [r0]         \n\t"   /* r1 = CR1 현재값                 */
        "ORR  r1, r1, %1       \n\t"   /* r1 |= START(bit8)               */
        "STR  r1, [r0]         \n\t"   /* CR1 업데이트 → START 생성       */
        "LDR  r0, =%2          \n\t"   /* r0 = &I2C1_SR1                  */
        "1:                    \n\t"
        "LDR  r1, [r0]         \n\t"   /* SR1 읽기                        */
        "TST  r1, %3           \n\t"   /* SR1 & SB(bit0) → Z 플래그       */
        "BEQ  1b               \n\t"   /* Z=1(SB=0) → 계속 대기          */
        :: "i"(I2C1_CR1_ADDR),"i"(I2C_CR1_START),
           "i"(I2C1_SR1_ADDR),"i"(I2C_SR1_SB)
        : "r0","r1","memory"
    );
}

/* =========================================================================
 * I2C_SendAddr_ASM : DR=addr → SR1:ADDR 폴링 → SR2 읽어 클리어
 * =========================================================================*/
static void I2C_SendAddr_ASM(uint8_t addr)
{
    __asm volatile (
        "LDR  r0, =%1          \n\t"   /* r0 = &I2C1_DR                   */
        "STRB %0, [r0]         \n\t"   /* DR = addr (1바이트)             */
        "LDR  r0, =%2          \n\t"   /* r0 = &I2C1_SR1                  */
        "1:                    \n\t"
        "LDR  r1, [r0]         \n\t"
        "TST  r1, %3           \n\t"   /* SR1 & ADDR(bit1)                */
        "BEQ  1b               \n\t"   /* ADDR=0 → 대기                   */
        "LDR  r1, [r0]         \n\t"   /* SR1 읽기 (클리어 시퀀스 ①)     */
        "LDR  r0, =%4          \n\t"   /* r0 = &I2C1_SR2                  */
        "LDR  r1, [r0]         \n\t"   /* SR2 읽기 (클리어 시퀀스 ②)     */
        :: "r"((uint32_t)addr),
           "i"(I2C1_DR_ADDR),
           "i"(I2C1_SR1_ADDR),"i"(I2C_SR1_ADDR_F),
           "i"(I2C1_SR2_ADDR)
        : "r0","r1","memory"
    );
}

/* =========================================================================
 * I2C_SendData_ASM : SR1:TXE 폴링 → DR=data → SR1:BTF 폴링
 * =========================================================================*/
static void I2C_SendData_ASM(uint8_t data)
{
    __asm volatile (
        "LDR  r0, =%1          \n\t"   /* r0 = &I2C1_SR1                  */
        "1:                    \n\t"
        "LDR  r1, [r0]         \n\t"
        "TST  r1, %2           \n\t"   /* SR1 & TXE(bit7)                 */
        "BEQ  1b               \n\t"   /* TXE=0 → 대기                   */
        "LDR  r0, =%3          \n\t"   /* r0 = &I2C1_DR                   */
        "STRB %0, [r0]         \n\t"   /* DR = data                       */
        "LDR  r0, =%1          \n\t"   /* r0 = &I2C1_SR1 (재로드)         */
        "2:                    \n\t"
        "LDR  r1, [r0]         \n\t"
        "TST  r1, %4           \n\t"   /* SR1 & BTF(bit2)                 */
        "BEQ  2b               \n\t"   /* BTF=0 → 대기                   */
        :: "r"((uint32_t)data),
           "i"(I2C1_SR1_ADDR),"i"(I2C_SR1_TXE),
           "i"(I2C1_DR_ADDR),
           "i"(I2C_SR1_BTF)
        : "r0","r1","memory"
    );
}

/* =========================================================================
 * I2C_Stop_ASM : CR1|=STOP
 * =========================================================================*/
static void I2C_Stop_ASM(void)
{
    __asm volatile (
        "LDR  r0, =%0          \n\t"
        "LDR  r1, [r0]         \n\t"
        "ORR  r1, r1, %1       \n\t"   /* CR1 |= STOP(bit9)               */
        "STR  r1, [r0]         \n\t"
        :: "i"(I2C1_CR1_ADDR),"i"(I2C_CR1_STOP)
        : "r0","r1","memory"
    );
}

static void LCD_Write_ASM(uint8_t data)
{
    I2C_Start_ASM();
    I2C_SendAddr_ASM(LCD_ADDR);
    I2C_SendData_ASM(data);
    I2C_Stop_ASM();
}

void LCD_CMD(uint8_t c){
    uint8_t h=(c&0xF0)|RS0_EN1|BL; LCD_Write_ASM(h);
    h=(c&0xF0)|RS0_EN0|BL;         LCD_Write_ASM(h); delay_us(4);
    h=((c<<4)&0xF0)|RS0_EN1|BL;    LCD_Write_ASM(h);
    h=((c<<4)&0xF0)|RS0_EN0|BL;    LCD_Write_ASM(h); delay_us(50);
}
void LCD_CMD4(uint8_t c){
    uint8_t h=((c<<4)&0xF0)|RS0_EN1|BL; LCD_Write_ASM(h);
    h=((c<<4)&0xF0)|RS0_EN0|BL;         LCD_Write_ASM(h); delay_us(50);
}
void LCD_DATA(uint8_t d){
    uint8_t h=(d&0xF0)|RS1_EN1|BL; LCD_Write_ASM(h);
    h=(d&0xF0)|RS1_EN0|BL;         LCD_Write_ASM(h); delay_us(4);
    h=((d<<4)&0xF0)|RS1_EN1|BL;    LCD_Write_ASM(h);
    h=((d<<4)&0xF0)|RS1_EN0|BL;    LCD_Write_ASM(h); delay_us(50);
}
void LCD_INIT(void){
    delay_ms(100);
    LCD_CMD4(0x03); delay_ms(5);  LCD_CMD4(0x03); delay_us(100);
    LCD_CMD4(0x03); delay_us(100); LCD_CMD4(0x02); delay_us(100);
    LCD_CMD(0x28); LCD_CMD(0x08); LCD_CMD(0x01); delay_ms(3);
    LCD_CMD(0x06); LCD_CMD(0x0C);
}
void LCD_XY(uint8_t x,uint8_t y){
    const uint8_t r[]={0x80,0xC0,0x94,0xD4}; if(y<4) LCD_CMD(r[y]+x);
}
void LCD_PUTS(char *s){ while(*s) LCD_DATA((uint8_t)*s++); }

#ifdef __GNUC__
int __io_putchar(int ch){
    if(ch=='\n') HAL_UART_Transmit(&huart2,(uint8_t*)"\r",1,0xFFFF);
    HAL_UART_Transmit(&huart2,(uint8_t*)&ch,1,0xFFFF); return ch;
}
#endif

int main(void){
    HAL_Init(); SystemClock_Config();
    MX_GPIO_Init(); MX_USART2_UART_Init(); MX_I2C1_Init();
    printf("\r\n[SET2-5] ASM I2C LCD | NUCLEO-F411RE\r\n");
    LCD_INIT();
    LCD_XY(0,0); LCD_PUTS("ASM I2C LCD");
    LCD_XY(0,1); LCD_PUTS("F411RE 84MHz");
    uint32_t cnt=0; char buf[17];
    while(1){
        cnt++; LCD_XY(0,1);
        snprintf(buf,sizeof(buf),"Count:%5lu",cnt); LCD_PUTS(buf);
        printf("ASM loop:%lu\r\n",cnt); HAL_Delay(1000);
    }
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
