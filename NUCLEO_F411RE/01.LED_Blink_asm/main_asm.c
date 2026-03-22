/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file     : main_asm.c
  * @brief    : I2C CLCD 제어 - 인라인 어셈블러 방식
  * @board    : NUCLEO-F411RE
  * @hardware : PCF8574 I2C 모듈 + HD44780 CLCD 16x2
  * @sysclk   : 84MHz (HSI 16MHz → PLL M=16, N=336, P=4)
  * @i2c      : I2C1  PB8(SCL) PB9(SDA)  400kHz Fast Mode
  * @uart     : USART2 PA2(TX) 115200bps
  *
  * [인라인 어셈블러로 직접 제어하는 레지스터]
  *
  *   I2C1 베이스: 0x40005400  (APB1, RM0383 p.474)
  *   ┌─────────────────────────────────────────────────────┐
  *   │ 오프셋  레지스터  주요 비트                         │
  *   │ 0x00    CR1       bit0:PE  bit8:START  bit9:STOP    │
  *   │                   bit10:ACK                         │
  *   │ 0x04    CR2       bit[5:0]: FREQ (APB1 clock MHz)   │
  *   │ 0x0C    DR        bit[7:0]: 송수신 데이터           │
  *   │ 0x14    SR1       bit0:SB  bit1:ADDR  bit2:BTF      │
  *   │                   bit7:TxE bit10:BERR               │
  *   │ 0x18    SR2       bit0:MSL(Master) bit1:BUSY        │
  *   └─────────────────────────────────────────────────────┘
  *
  * [I2C 마스터 전송 시퀀스 (1바이트)]
  *   1. CR1:START=1  → START 조건 생성
  *   2. SR1:SB=1 대기 (Start Bit 확인)
  *   3. DR = ADDRESS  → 주소 전송 (8bit: 7bit주소 + W비트0)
  *   4. SR1:ADDR=1 대기, SR2 읽어 ADDR 플래그 클리어
  *   5. SR1:TxE=1 대기
  *   6. DR = DATA  → 데이터 전송
  *   7. SR1:BTF=1 대기 (Byte Transfer Finished)
  *   8. CR1:STOP=1  → STOP 조건 생성
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"
#include <stdio.h>
#include <string.h>

/* =========================================================================
 * I2C1 레지스터 주소 (RM0383 기준)
 * =========================================================================*/
#define I2C1_BASE       0x40005400UL
#define I2C1_CR1        (I2C1_BASE + 0x00UL)   /* Control Register 1    */
#define I2C1_CR2        (I2C1_BASE + 0x04UL)   /* Control Register 2    */
#define I2C1_DR         (I2C1_BASE + 0x0CUL)   /* Data Register         */
#define I2C1_SR1        (I2C1_BASE + 0x14UL)   /* Status Register 1     */
#define I2C1_SR2        (I2C1_BASE + 0x18UL)   /* Status Register 2     */

/* CR1 비트 */
#define CR1_PE          (1UL <<  0)   /* Peripheral Enable              */
#define CR1_START       (1UL <<  8)   /* Generate START condition       */
#define CR1_STOP        (1UL <<  9)   /* Generate STOP condition        */
#define CR1_ACK         (1UL << 10)   /* Acknowledge Enable             */

/* SR1 비트 */
#define SR1_SB          (1UL <<  0)   /* Start Bit (Master mode)        */
#define SR1_ADDR        (1UL <<  1)   /* Address sent (Master mode)     */
#define SR1_BTF         (1UL <<  2)   /* Byte Transfer Finished         */
#define SR1_TXE         (1UL <<  7)   /* Data Register Empty (Tx)       */

/* =========================================================================
 * PCF8574 비트 상수
 * =========================================================================*/
#define RS0_EN1     0x04
#define RS0_EN0     0x00
#define RS1_EN1     0x05
#define RS1_EN0     0x01
#define BackLight   0x08

#define ADDRESS     (0x27 << 1)   /* PCF8574  기본: 0x4E */

/* =========================================================================
 * Delay 카운트  SYSCLK=84MHz, ≈3 사이클/루프
 * =========================================================================*/
#define DELAY_1MS_COUNT    28000UL

/* ─── 함수 선언 ──────────────────────────────────────────────────────────── */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);

static void Delay_ASM(uint32_t count);
static void delay_us_asm(uint32_t us);
static void delay_ms_asm(uint32_t ms);

/* I2C ASM 함수 */
static void I2C_Start_ASM(void);
static void I2C_Stop_ASM(void);
static void I2C_SendAddr_ASM(uint8_t addr);
static void I2C_SendData_ASM(uint8_t data);
static void I2C_Write_ASM(uint8_t addr, uint8_t data);

/* LCD 함수 */
void LCD_CMD(uint8_t cmd);
void LCD_CMD_4bit(uint8_t cmd);
void LCD_DATA(uint8_t data);
void LCD_INIT(void);
void LCD_XY(uint8_t x, uint8_t y);
void LCD_CLEAR(void);
void LCD_PUTS(char *str);

/* HAL 핸들 (UART / I2C HAL Init에만 사용) */
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
    MX_I2C1_Init();   /* I2C 클럭/핀 설정은 HAL 사용, 데이터 전송은 ASM */

    printf("\r\n=== ASM I2C LCD Test (NUCLEO-F411RE) ===\r\n");
    printf("SYSCLK: 84MHz | I2C1: 400kHz | Transfer: Inline ASM\r\n\r\n");

    LCD_INIT();

    LCD_XY(0, 0);  LCD_PUTS("ASM I2C LCD");
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
 * Delay_ASM
 *   SUBS/BNE 루프 딜레이
 *   "+r"(count): 읽기+쓰기 피연산자
 *   "cc"       : SUBS가 CPSR 플래그 변경
 * =========================================================================*/
static void Delay_ASM(uint32_t count)
{
    __asm volatile (
        "1:                \n\t"
        "SUBS %0, %0, #1   \n\t"
        "BNE  1b           \n\t"
        : "+r"(count)
        :
        : "cc"
    );
}

static void delay_us_asm(uint32_t us)
{
    Delay_ASM(us * 3);   /* 84MHz 기준 1us ≈ 3 루프 */
}

static void delay_ms_asm(uint32_t ms)
{
    Delay_ASM(ms * DELAY_1MS_COUNT);
}

/* =========================================================================
 * I2C_Start_ASM
 *   I2C START 조건 생성
 *
 *   동작:
 *     CR1 |= START 비트 세트
 *     SR1:SB(bit0) = 1 이 될 때까지 폴링 대기
 *
 *   [어셈블리 설명]
 *     LDR  r0, =addr  : 32비트 레지스터 주소 로드
 *     LDR  r1, [r0]   : 현재 레지스터 값 읽기
 *     ORR  r1, r1, #n : 특정 비트 세트
 *     STR  r1, [r0]   : 레지스터 업데이트
 *     1: (폴링 루프)
 *     LDR  r1, [r0]   : SR1 읽기
 *     TST  r1, #n     : 비트 확인 (AND 후 결과가 0이면 Z=1)
 *     BEQ  1b         : Z=1(비트 0) 이면 다시 폴링
 * =========================================================================*/
static void I2C_Start_ASM(void)
{
    __asm volatile (
        /* CR1 |= START */
        "LDR  r0, =%0          \n\t"   /* r0 = &I2C1_CR1                 */
        "LDR  r1, [r0]         \n\t"   /* r1 = CR1 현재값                */
        "ORR  r1, r1, %1       \n\t"   /* r1 |= CR1_START (bit8)         */
        "STR  r1, [r0]         \n\t"   /* CR1 업데이트                   */
        /* SR1:SB 폴링 - SB=1 될 때까지 대기 */
        "LDR  r0, =%2          \n\t"   /* r0 = &I2C1_SR1                 */
        "1:                    \n\t"
        "LDR  r1, [r0]         \n\t"   /* r1 = SR1 읽기                  */
        "TST  r1, %3           \n\t"   /* SR1 & SR1_SB (bit0) 확인       */
        "BEQ  1b               \n\t"   /* SB=0 이면 계속 대기            */
        :
        : "i"(I2C1_CR1), "i"(CR1_START),
          "i"(I2C1_SR1), "i"(SR1_SB)
        : "r0", "r1", "memory"
    );
}

/* =========================================================================
 * I2C_Stop_ASM
 *   I2C STOP 조건 생성
 *   CR1 |= STOP 비트 세트 → I2C 하드웨어가 자동으로 STOP 생성
 * =========================================================================*/
static void I2C_Stop_ASM(void)
{
    __asm volatile (
        "LDR  r0, =%0          \n\t"   /* r0 = &I2C1_CR1                 */
        "LDR  r1, [r0]         \n\t"   /* r1 = CR1 현재값                */
        "ORR  r1, r1, %1       \n\t"   /* r1 |= CR1_STOP (bit9)          */
        "STR  r1, [r0]         \n\t"   /* CR1 업데이트 → STOP 생성       */
        :
        : "i"(I2C1_CR1), "i"(CR1_STOP)
        : "r0", "r1", "memory"
    );
}

/* =========================================================================
 * I2C_SendAddr_ASM
 *   7비트 주소 + W(0) → DR 쓰기 → ADDR 플래그 대기 → SR2 읽어 클리어
 *
 *   ADDR 플래그 클리어 방법:
 *     SR1 읽기 후 SR2 읽기 → 자동으로 ADDR 클리어
 * =========================================================================*/
static void I2C_SendAddr_ASM(uint8_t addr)
{
    __asm volatile (
        /* DR = addr (주소 + W비트) */
        "LDR  r0, =%1          \n\t"   /* r0 = &I2C1_DR                  */
        "STRB %0, [r0]         \n\t"   /* DR = addr (byte 쓰기)          */
        /* SR1:ADDR 폴링 */
        "LDR  r0, =%2          \n\t"   /* r0 = &I2C1_SR1                 */
        "1:                    \n\t"
        "LDR  r1, [r0]         \n\t"   /* r1 = SR1 읽기                  */
        "TST  r1, %3           \n\t"   /* SR1 & SR1_ADDR (bit1) 확인     */
        "BEQ  1b               \n\t"   /* ADDR=0 이면 계속 대기          */
        /* ADDR 플래그 클리어: SR1 읽기(이미 완료) → SR2 읽기 */
        "LDR  r0, =%4          \n\t"   /* r0 = &I2C1_SR2                 */
        "LDR  r1, [r0]         \n\t"   /* SR2 읽기 → ADDR 플래그 클리어 */
        :
        : "r"((uint32_t)addr),
          "i"(I2C1_DR),
          "i"(I2C1_SR1), "i"(SR1_ADDR),
          "i"(I2C1_SR2)
        : "r0", "r1", "memory"
    );
}

/* =========================================================================
 * I2C_SendData_ASM
 *   TxE(송신 버퍼 비어있음) 대기 → DR = data → BTF 대기
 *
 *   TxE=1: DR이 비어 있어 새 데이터 쓰기 가능
 *   BTF=1: 현재 전송 바이트가 완전히 시프트 아웃됨
 * =========================================================================*/
static void I2C_SendData_ASM(uint8_t data)
{
    __asm volatile (
        /* SR1:TxE 폴링 */
        "LDR  r0, =%1          \n\t"   /* r0 = &I2C1_SR1                 */
        "1:                    \n\t"
        "LDR  r1, [r0]         \n\t"   /* r1 = SR1 읽기                  */
        "TST  r1, %2           \n\t"   /* SR1 & SR1_TXE (bit7) 확인      */
        "BEQ  1b               \n\t"   /* TxE=0 이면 계속 대기           */
        /* DR = data */
        "LDR  r0, =%3          \n\t"   /* r0 = &I2C1_DR                  */
        "STRB %0, [r0]         \n\t"   /* DR = data (byte 쓰기)          */
        /* SR1:BTF 폴링 */
        "LDR  r0, =%1          \n\t"   /* r0 = &I2C1_SR1 (다시 로드)     */
        "2:                    \n\t"
        "LDR  r1, [r0]         \n\t"   /* r1 = SR1 읽기                  */
        "TST  r1, %4           \n\t"   /* SR1 & SR1_BTF (bit2) 확인      */
        "BEQ  2b               \n\t"   /* BTF=0 이면 계속 대기           */
        :
        : "r"((uint32_t)data),
          "i"(I2C1_SR1), "i"(SR1_TXE),
          "i"(I2C1_DR),
          "i"(SR1_BTF)
        : "r0", "r1", "memory"
    );
}

/* =========================================================================
 * I2C_Write_ASM
 *   START → 주소 전송 → 데이터 전송 → STOP
 *   HAL_I2C_Master_Transmit() 1바이트 전송과 동일한 동작
 * =========================================================================*/
static void I2C_Write_ASM(uint8_t addr, uint8_t data)
{
    I2C_Start_ASM();
    I2C_SendAddr_ASM(addr);
    I2C_SendData_ASM(data);
    I2C_Stop_ASM();
}

/* =========================================================================
 * LCD_CMD_4bit  (ASM I2C 사용)
 * =========================================================================*/
void LCD_CMD_4bit(uint8_t cmd)
{
    uint8_t temp;

    temp = ((cmd << 4) & 0xF0) | RS0_EN1 | BackLight;
    I2C_Write_ASM(ADDRESS, temp);

    temp = ((cmd << 4) & 0xF0) | RS0_EN0 | BackLight;
    I2C_Write_ASM(ADDRESS, temp);

    delay_us_asm(50);
}

/* =========================================================================
 * LCD_CMD  (ASM I2C 사용)
 *   상위 니블 EN 펄스 → 하위 니블 EN 펄스
 * =========================================================================*/
void LCD_CMD(uint8_t cmd)
{
    uint8_t temp;

    temp = (cmd & 0xF0) | RS0_EN1 | BackLight;
    I2C_Write_ASM(ADDRESS, temp);
    temp = (cmd & 0xF0) | RS0_EN0 | BackLight;
    I2C_Write_ASM(ADDRESS, temp);
    delay_us_asm(4);

    temp = ((cmd << 4) & 0xF0) | RS0_EN1 | BackLight;
    I2C_Write_ASM(ADDRESS, temp);
    temp = ((cmd << 4) & 0xF0) | RS0_EN0 | BackLight;
    I2C_Write_ASM(ADDRESS, temp);
    delay_us_asm(50);
}

/* =========================================================================
 * LCD_DATA  (ASM I2C 사용)
 * =========================================================================*/
void LCD_DATA(uint8_t data)
{
    uint8_t temp;

    temp = (data & 0xF0) | RS1_EN1 | BackLight;
    I2C_Write_ASM(ADDRESS, temp);
    temp = (data & 0xF0) | RS1_EN0 | BackLight;
    I2C_Write_ASM(ADDRESS, temp);
    delay_us_asm(4);

    temp = ((data << 4) & 0xF0) | RS1_EN1 | BackLight;
    I2C_Write_ASM(ADDRESS, temp);
    temp = ((data << 4) & 0xF0) | RS1_EN0 | BackLight;
    I2C_Write_ASM(ADDRESS, temp);
    delay_us_asm(50);
}

/* =========================================================================
 * LCD_INIT / LCD_XY / LCD_CLEAR / LCD_PUTS
 * =========================================================================*/
void LCD_INIT(void)
{
    delay_ms_asm(100);

    LCD_CMD_4bit(0x03); delay_ms_asm(5);
    LCD_CMD_4bit(0x03); delay_us_asm(100);
    LCD_CMD_4bit(0x03); delay_us_asm(100);
    LCD_CMD_4bit(0x02); delay_us_asm(100);

    LCD_CMD(0x28);
    LCD_CMD(0x08);
    LCD_CMD(0x01);
    delay_ms_asm(3);
    LCD_CMD(0x06);
    LCD_CMD(0x0C);
}

void LCD_XY(uint8_t x, uint8_t y)
{
    const uint8_t row[] = {0x80, 0xC0, 0x94, 0xD4};
    if (y < 4) LCD_CMD(row[y] + x);
}

void LCD_CLEAR(void)
{
    LCD_CMD(0x01);
    delay_ms_asm(2);
}

void LCD_PUTS(char *str)
{
    while (*str) LCD_DATA((uint8_t)*str++);
}

/* =========================================================================
 * 주변장치 초기화  (클럭/핀 설정은 HAL 유지, 데이터 전송만 ASM)
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
