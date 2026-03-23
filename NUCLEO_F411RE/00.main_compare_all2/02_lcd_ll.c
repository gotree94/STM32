/**
  ******************************************************************************
  * @file    02_lcd_ll.c
  * @brief   [SET2-②] I2C CLCD 제어 - LL(Low Layer) 방식
  * @board   NUCLEO-F411RE | I2C1 PB8/PB9 400kHz
  *
  * [LL I2C 방식]
  *   LL_I2C_GenerateStartCondition()  → START
  *   LL_I2C_TransmitData8()           → DATA
  *   LL_I2C_GenerateStopCondition()   → STOP
  *   LL_I2C_IsActiveFlag_SB/ADDR/BTF/TXE() → 상태 폴링
  *   내부: 레지스터 직접 접근이나 __STATIC_INLINE 래퍼
  *
  * [사용법] Core/Src/main.c 로 복사 후 빌드
  ******************************************************************************
  */
#include "main.h"
#include "stm32f4xx_ll_i2c.h"
#include "stm32f4xx_ll_gpio.h"
#include <stdio.h>
#include <string.h>

#define RS0_EN1   0x04
#define RS0_EN0   0x00
#define RS1_EN1   0x05
#define RS1_EN0   0x01
#define BL        0x08
#define LCD_ADDR  (0x27<<1)

I2C_HandleTypeDef  hi2c1;
UART_HandleTypeDef huart2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);

static void delay_us(uint32_t us){ volatile uint32_t d=us*3; while(d--); }
static void delay_ms(uint32_t ms){ HAL_Delay(ms); }

/* =========================================================================
 * LL I2C 1바이트 전송
 *   LL_I2C_* 함수는 __STATIC_INLINE → 컴파일러가 레지스터 직접 접근으로 펼침
 *   HAL_I2C_Master_Transmit의 경량 버전
 * =========================================================================*/
static void LCD_Write_LL(uint8_t data)
{
    /* START */
    LL_I2C_GenerateStartCondition(I2C1);
    while (!LL_I2C_IsActiveFlag_SB(I2C1));          /* SB=1 대기  */

    /* 주소 전송 */
    LL_I2C_TransmitData8(I2C1, LCD_ADDR);
    while (!LL_I2C_IsActiveFlag_ADDR(I2C1));         /* ADDR=1 대기*/
    LL_I2C_ClearFlag_ADDR(I2C1);                     /* ADDR 클리어*/

    /* 데이터 전송 */
    while (!LL_I2C_IsActiveFlag_TXE(I2C1));          /* TXE=1 대기 */
    LL_I2C_TransmitData8(I2C1, data);
    while (!LL_I2C_IsActiveFlag_BTF(I2C1));          /* BTF=1 대기 */

    /* STOP */
    LL_I2C_GenerateStopCondition(I2C1);
}

void LCD_CMD(uint8_t c){
    uint8_t h=(c&0xF0)|RS0_EN1|BL; LCD_Write_LL(h);
    h=(c&0xF0)|RS0_EN0|BL;         LCD_Write_LL(h); delay_us(4);
    h=((c<<4)&0xF0)|RS0_EN1|BL;    LCD_Write_LL(h);
    h=((c<<4)&0xF0)|RS0_EN0|BL;    LCD_Write_LL(h); delay_us(50);
}
void LCD_CMD4(uint8_t c){
    uint8_t h=((c<<4)&0xF0)|RS0_EN1|BL; LCD_Write_LL(h);
    h=((c<<4)&0xF0)|RS0_EN0|BL;         LCD_Write_LL(h); delay_us(50);
}
void LCD_DATA(uint8_t d){
    uint8_t h=(d&0xF0)|RS1_EN1|BL; LCD_Write_LL(h);
    h=(d&0xF0)|RS1_EN0|BL;         LCD_Write_LL(h); delay_us(4);
    h=((d<<4)&0xF0)|RS1_EN1|BL;    LCD_Write_LL(h);
    h=((d<<4)&0xF0)|RS1_EN0|BL;    LCD_Write_LL(h); delay_us(50);
}
void LCD_INIT(void){
    delay_ms(100);
    LCD_CMD4(0x03); delay_ms(5);  LCD_CMD4(0x03); delay_us(100);
    LCD_CMD4(0x03); delay_us(100); LCD_CMD4(0x02); delay_us(100);
    LCD_CMD(0x28); LCD_CMD(0x08); LCD_CMD(0x01); delay_ms(3);
    LCD_CMD(0x06); LCD_CMD(0x0C);
}
void LCD_XY(uint8_t x,uint8_t y){
    const uint8_t r[]={0x80,0xC0,0x94,0xD4};
    if(y<4) LCD_CMD(r[y]+x);
}
void LCD_PUTS(char *s){ while(*s) LCD_DATA((uint8_t)*s++); }

#ifdef __GNUC__
int __io_putchar(int ch){
    if(ch=='\n') HAL_UART_Transmit(&huart2,(uint8_t*)"\r",1,0xFFFF);
    HAL_UART_Transmit(&huart2,(uint8_t*)&ch,1,0xFFFF); return ch;
}
#endif

int main(void)
{
    HAL_Init(); SystemClock_Config();
    MX_GPIO_Init(); MX_USART2_UART_Init(); MX_I2C1_Init();

    printf("\r\n[SET2-2] LL I2C LCD | NUCLEO-F411RE\r\n");

    LCD_INIT();
    LCD_XY(0,0); LCD_PUTS("LL  I2C LCD");
    LCD_XY(0,1); LCD_PUTS("F411RE 84MHz");

    uint32_t cnt=0; char buf[17];
    while(1){
        cnt++;
        LCD_XY(0,1);
        snprintf(buf,sizeof(buf),"Count:%5lu",cnt);
        LCD_PUTS(buf);
        printf("LL loop:%lu\r\n",cnt);
        HAL_Delay(1000);
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
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
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
