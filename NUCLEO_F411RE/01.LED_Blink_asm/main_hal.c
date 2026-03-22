/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file     : main_hal.c
  * @brief    : I2C CLCD 제어 - HAL 방식
  * @board    : NUCLEO-F411RE
  * @hardware : PCF8574 I2C 모듈 + HD44780 CLCD 16x2
  * @sysclk   : 84MHz (HSI 16MHz → PLL M=16, N=336, P=4)
  * @i2c      : I2C1  PB8(SCL) PB9(SDA)  400kHz Fast Mode
  * @uart     : USART2 PA2(TX) 115200bps → ST-Link Virtual COM
  *
  * [PCF8574 핀 매핑]
  *   bit7  bit6  bit5  bit4  bit3  bit2  bit1  bit0
  *   D7    D6    D5    D4    BL    EN    RW    RS
  *
  * [I2C 주소]
  *   PCF8574  기본 (A2=A1=A0=0) : 0x27 (7bit) → 0x4E (8bit)
  *   PCF8574A 기본 (A2=A1=A0=0) : 0x3F (7bit) → 0x7E (8bit)
  *   I2C_ScanAddresses() 로 실제 주소 확인 후 ADDRESS 수정
  *
  * [사용법]
  *   Core/Src/main.c 로 복사 후 빌드
  *   시리얼 터미널 115200 연결 → I2C 스캔 결과 확인
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"
#include <stdio.h>
#include <string.h>

/* =========================================================================
 * PCF8574 비트 상수 (HD44780 4비트 모드)
 *
 *   PCF8574 출력 비트 구성:
 *   [D7 D6 D5 D4] [BL EN RW RS]
 *
 *   RS : Register Select  0=Command, 1=Data
 *   EN : Enable 펄스      1→0 하강 엣지에서 HD44780 데이터 래치
 *   RW : Read/Write       0=Write (항상 Write 고정)
 *   BL : Backlight        1=ON
 * =========================================================================*/
#define RS0_EN1     0x04    /* RS=0, EN=1, RW=0 : 명령 EN 하이   */
#define RS0_EN0     0x00    /* RS=0, EN=0, RW=0 : 명령 EN 로우   */
#define RS1_EN1     0x05    /* RS=1, EN=1, RW=0 : 데이터 EN 하이 */
#define RS1_EN0     0x01    /* RS=1, EN=0, RW=0 : 데이터 EN 로우 */
#define BackLight   0x08    /* BL=1 : 백라이트 ON                 */

/* I2C 주소 (7bit << 1 = 8bit Write Address) */
#define ADDRESS     (0x27 << 1)   /* PCF8574  기본: 0x4E */
// #define ADDRESS  (0x3F << 1)   /* PCF8574A 기본: 0x7E */

/* ─── 함수 선언 ──────────────────────────────────────────────────────────── */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);

void I2C_ScanAddresses(void);
static void delay_us(uint32_t us);
static void delay_ms(uint32_t ms);

void LCD_CMD(uint8_t cmd);
void LCD_CMD_4bit(uint8_t cmd);
void LCD_DATA(uint8_t data);
void LCD_INIT(void);
void LCD_XY(uint8_t x, uint8_t y);
void LCD_CLEAR(void);
void LCD_PUTS(char *str);

/* HAL 핸들 */
I2C_HandleTypeDef  hi2c1;
UART_HandleTypeDef huart2;

/* =========================================================================
 * printf → USART2 리디렉션
 * =========================================================================*/
#ifdef __GNUC__
int __io_putchar(int ch)
#else
int fputc(int ch, FILE *f)
#endif
{
    if (ch == '\n')
        HAL_UART_Transmit(&huart2, (uint8_t *)"\r", 1, 0xFFFF);
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
}

/* =========================================================================
 * main
 * =========================================================================*/
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    MX_I2C1_Init();

    printf("\r\n=== HAL I2C LCD Test (NUCLEO-F411RE) ===\r\n");
    printf("SYSCLK: 84MHz | I2C1: 400kHz\r\n\r\n");

    I2C_ScanAddresses();

    LCD_INIT();

    LCD_XY(0, 0);  LCD_PUTS("HAL I2C LCD");
    LCD_XY(0, 1);  LCD_PUTS("NUCLEO-F411RE");

    printf("LCD initialized.\r\n");

    uint32_t count = 0;
    char buf[16];

    while (1)
    {
        count++;
        LCD_XY(0, 1);
        snprintf(buf, sizeof(buf), "Count:%5lu", count);
        LCD_PUTS(buf);
        printf("Loop: %lu\r\n", count);
        HAL_Delay(1000);
    }
}

/* =========================================================================
 * I2C_ScanAddresses
 * =========================================================================*/
void I2C_ScanAddresses(void)
{
    printf("Scanning I2C bus...\r\n");
    for (uint8_t i = 1; i < 128; i++) {
        if (HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(i << 1), 1, 10) == HAL_OK)
            printf("  Found: 0x%02X (8bit: 0x%02X)\r\n", i, i << 1);
    }
    printf("Scan done.\r\n\r\n");
}

/* =========================================================================
 * delay_us : 소프트웨어 루프 딜레이
 *   84MHz 기준 보정값 = 3 (루프 1회 ≈ 약 4 사이클)
 * =========================================================================*/
static void delay_us(uint32_t us)
{
    volatile uint32_t d = us * 3;
    for (volatile uint32_t i = 0; i < d; i++);
}

static void delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

/* =========================================================================
 * LCD_CMD_4bit
 *   HD44780 초기화 시 사용하는 단일 니블 전송
 *   cmd의 하위 4비트를 상위 4비트 위치로 이동 후 전송
 *
 *   I2C 전송 1회: START → ADDRESS(W) → DATA → STOP
 *   → HAL_I2C_Master_Transmit() 내부에서 모두 처리
 * =========================================================================*/
void LCD_CMD_4bit(uint8_t cmd)
{
    uint8_t temp;

    temp = ((cmd << 4) & 0xF0) | RS0_EN1 | BackLight;   /* EN 하이 */
    while (HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000) != HAL_OK);

    temp = ((cmd << 4) & 0xF0) | RS0_EN0 | BackLight;   /* EN 로우 */
    while (HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000) != HAL_OK);

    delay_us(50);
}

/* =========================================================================
 * LCD_CMD
 *   8비트 명령을 4비트 × 2회 분할 전송 (RS=0)
 *
 *   전송 순서:
 *     ① 상위 니블 + RS0_EN1 → I2C 전송 (EN 하이)
 *     ② 상위 니블 + RS0_EN0 → I2C 전송 (EN 로우 → 래치)
 *     ③ 하위 니블 + RS0_EN1 → I2C 전송 (EN 하이)
 *     ④ 하위 니블 + RS0_EN0 → I2C 전송 (EN 로우 → 래치)
 *
 *   I2C 전송 총 4회 → HAL_I2C_Master_Transmit() 4번 호출
 * =========================================================================*/
void LCD_CMD(uint8_t cmd)
{
    uint8_t temp;

    /* 상위 니블 EN 펄스 */
    temp = (cmd & 0xF0) | RS0_EN1 | BackLight;
    while (HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000) != HAL_OK);
    temp = (cmd & 0xF0) | RS0_EN0 | BackLight;
    while (HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000) != HAL_OK);
    delay_us(4);

    /* 하위 니블 EN 펄스 */
    temp = ((cmd << 4) & 0xF0) | RS0_EN1 | BackLight;
    while (HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000) != HAL_OK);
    temp = ((cmd << 4) & 0xF0) | RS0_EN0 | BackLight;
    while (HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000) != HAL_OK);
    delay_us(50);
}

/* =========================================================================
 * LCD_DATA
 *   8비트 문자 데이터를 4비트 × 2회 분할 전송 (RS=1)
 *   LCD_CMD와 동일 구조, RS 비트만 다름
 * =========================================================================*/
void LCD_DATA(uint8_t data)
{
    uint8_t temp;

    /* 상위 니블 EN 펄스 */
    temp = (data & 0xF0) | RS1_EN1 | BackLight;
    while (HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000) != HAL_OK);
    temp = (data & 0xF0) | RS1_EN0 | BackLight;
    while (HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000) != HAL_OK);
    delay_us(4);

    /* 하위 니블 EN 펄스 */
    temp = ((data << 4) & 0xF0) | RS1_EN1 | BackLight;
    while (HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000) != HAL_OK);
    temp = ((data << 4) & 0xF0) | RS1_EN0 | BackLight;
    while (HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000) != HAL_OK);
    delay_us(50);
}

/* =========================================================================
 * LCD_INIT  HD44780 4비트 초기화 시퀀스
 *   데이터시트 Figure 24 (Initializing by Instruction) 기준
 * =========================================================================*/
void LCD_INIT(void)
{
    delay_ms(100);              /* VDD 상승 후 안정화 대기           */

    LCD_CMD_4bit(0x03); delay_ms(5);    /* Function Set (8bit 시도1) */
    LCD_CMD_4bit(0x03); delay_us(100);  /* Function Set (8bit 시도2) */
    LCD_CMD_4bit(0x03); delay_us(100);  /* Function Set (8bit 시도3) */
    LCD_CMD_4bit(0x02); delay_us(100);  /* 4비트 모드 전환           */

    LCD_CMD(0x28);  /* Function Set : 4bit, 2-line, 5×8 dot        */
    LCD_CMD(0x08);  /* Display OFF  : 화면/커서/깜박임 모두 OFF     */
    LCD_CMD(0x01);  /* Clear Display                                */
    delay_ms(3);    /* Clear 실행 대기 (최소 1.52ms)                */
    LCD_CMD(0x06);  /* Entry Mode   : 커서 우측 이동, Shift 없음   */
    LCD_CMD(0x0C);  /* Display ON   : 커서 없음, 깜박임 없음       */
}

/* =========================================================================
 * LCD_XY / LCD_CLEAR / LCD_PUTS
 * =========================================================================*/
void LCD_XY(uint8_t x, uint8_t y)
{
    const uint8_t row[] = {0x80, 0xC0, 0x94, 0xD4};
    if (y < 4) LCD_CMD(row[y] + x);
}

void LCD_CLEAR(void)
{
    LCD_CMD(0x01);
    delay_ms(2);
}

void LCD_PUTS(char *str)
{
    while (*str) LCD_DATA((uint8_t)*str++);
}

/* =========================================================================
 * 주변장치 초기화
 * =========================================================================*/
static void MX_I2C1_Init(void)
{
    hi2c1.Instance             = I2C1;
    hi2c1.Init.ClockSpeed      = 400000;
    hi2c1.Init.DutyCycle       = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1     = 0;
    hi2c1.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2     = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) { Error_Handler(); }
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM            = 16;
    RCC_OscInitStruct.PLL.PLLN            = 336;
    RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ            = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }

    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                     | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) { Error_Handler(); }
}

static void MX_USART2_UART_Init(void)
{
    huart2.Instance          = USART2;
    huart2.Init.BaudRate     = 115200;
    huart2.Init.WordLength   = UART_WORDLENGTH_8B;
    huart2.Init.StopBits     = UART_STOPBITS_1;
    huart2.Init.Parity       = UART_PARITY_NONE;
    huart2.Init.Mode         = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK) { Error_Handler(); }
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin  = B1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = LD2_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}
