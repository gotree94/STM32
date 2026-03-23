/**
  ******************************************************************************
  * @file    01_lcd_hal.c
  * @brief   [SET2-①] I2C CLCD 제어 - HAL 방식
  * @board   NUCLEO-F411RE
  * @hw      PCF8574 I2C 모듈 + HD44780 CLCD 16x2
  * @i2c     I2C1 PB8(SCL) PB9(SDA) 400kHz
  * @uart    USART2 PA2(TX) 115200bps
  * @sysclk  84MHz
  *
  * [HAL 방식]
  *   HAL_I2C_Master_Transmit(&hi2c1, addr, &data, 1, timeout)
  *   내부: START → ADDR → DATA → STOP + 타임아웃/에러처리
  *
  * [PCF8574 핀 매핑]
  *   P7  P6  P5  P4  P3  P2  P1  P0
  *   D7  D6  D5  D4  BL  EN  RW  RS
  *
  * [I2C 주소] PCF8574: 0x27(7bit)→0x4E(8bit) / PCF8574A: 0x3F→0x7E
  * [사용법]   Core/Src/main.c 로 복사 후 빌드
  ******************************************************************************
  */
#include "main.h"
#include <stdio.h>
#include <string.h>

/* ── PCF8574 비트 상수 ──────────────────────────────────────────────────── */
#define RS0_EN1   0x04   /* RS=0 EN=1: 명령 EN↑  */
#define RS0_EN0   0x00   /* RS=0 EN=0: 명령 EN↓  */
#define RS1_EN1   0x05   /* RS=1 EN=1: 데이터 EN↑*/
#define RS1_EN0   0x01   /* RS=1 EN=0: 데이터 EN↓*/
#define BL        0x08   /* 백라이트 ON           */
#define LCD_ADDR  (0x27<<1)   /* 0x4E  ← 실제 주소로 수정 */

I2C_HandleTypeDef  hi2c1;
UART_HandleTypeDef huart2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);

static void delay_us(uint32_t us){ volatile uint32_t d=us*3; while(d--);}
static void delay_ms(uint32_t ms){ HAL_Delay(ms); }

/* ── LCD 함수 ────────────────────────────────────────────────────────────── */
static void LCD_Write(uint8_t d){
    while(HAL_I2C_Master_Transmit(&hi2c1,LCD_ADDR,&d,1,1000)!=HAL_OK);
}
void LCD_CMD(uint8_t c){
    uint8_t h=(c&0xF0)|RS0_EN1|BL; LCD_Write(h);
    h=(c&0xF0)|RS0_EN0|BL;         LCD_Write(h); delay_us(4);
    h=((c<<4)&0xF0)|RS0_EN1|BL;    LCD_Write(h);
    h=((c<<4)&0xF0)|RS0_EN0|BL;    LCD_Write(h); delay_us(50);
}
void LCD_CMD4(uint8_t c){
    uint8_t h=((c<<4)&0xF0)|RS0_EN1|BL; LCD_Write(h);
    h=((c<<4)&0xF0)|RS0_EN0|BL;         LCD_Write(h); delay_us(50);
}
void LCD_DATA(uint8_t d){
    uint8_t h=(d&0xF0)|RS1_EN1|BL; LCD_Write(h);
    h=(d&0xF0)|RS1_EN0|BL;         LCD_Write(h); delay_us(4);
    h=((d<<4)&0xF0)|RS1_EN1|BL;    LCD_Write(h);
    h=((d<<4)&0xF0)|RS1_EN0|BL;    LCD_Write(h); delay_us(50);
}
void LCD_INIT(void){
    delay_ms(100);
    LCD_CMD4(0x03); delay_ms(5);
    LCD_CMD4(0x03); delay_us(100);
    LCD_CMD4(0x03); delay_us(100);
    LCD_CMD4(0x02); delay_us(100);
    LCD_CMD(0x28); LCD_CMD(0x08); LCD_CMD(0x01); delay_ms(3);
    LCD_CMD(0x06); LCD_CMD(0x0C);
}
void LCD_XY(uint8_t x,uint8_t y){
    const uint8_t r[]={0x80,0xC0,0x94,0xD4};
    if(y<4) LCD_CMD(r[y]+x);
}
void LCD_PUTS(char *s){ while(*s) LCD_DATA((uint8_t)*s++); }

/* ── printf → UART ──────────────────────────────────────────────────────── */
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

    printf("\r\n[SET2-1] HAL I2C LCD | NUCLEO-F411RE\r\n");

    LCD_INIT();
    LCD_XY(0,0); LCD_PUTS("HAL I2C LCD");
    LCD_XY(0,1); LCD_PUTS("F411RE 84MHz");

    uint32_t cnt=0; char buf[17];
    while(1){
        cnt++;
        LCD_XY(0,1);
        snprintf(buf,sizeof(buf),"Count:%5lu",cnt);
        LCD_PUTS(buf);
        printf("HAL loop:%lu\r\n",cnt);
        HAL_Delay(1000);
    }
}

static void MX_I2C1_Init(void){
    hi2c1.Instance=I2C1; hi2c1.Init.ClockSpeed=400000;
    hi2c1.Init.DutyCycle=I2C_DUTYCYCLE_2; hi2c1.Init.OwnAddress1=0;
    hi2c1.Init.AddressingMode=I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode=I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2=0; hi2c1.Init.GeneralCallMode=I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode=I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c1);
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
