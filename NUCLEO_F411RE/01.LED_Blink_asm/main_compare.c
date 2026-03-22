/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file     : main_compare.c
  * @brief    : I2C CLCD HAL vs 인라인 어셈블러 - DWT 사이클 비교
  * @board    : NUCLEO-F411RE
  * @hardware : PCF8574 I2C 모듈 + HD44780 CLCD 16x2
  * @sysclk   : 84MHz
  * @uart     : USART2 PA2(TX) 115200bps
  *
  * [비교 대상]
  *   HAL 방식: HAL_I2C_Master_Transmit() → LCD 한 글자 표시
  *   ASM 방식: I2C 레지스터 직접 제어    → LCD 한 글자 표시
  *
  * [측정 항목] DWT Cycle Counter 사용
  *   ① I2C_Write 1회 (1바이트 전송) 사이클 수
  *   ② LCD_DATA 1회 (문자 1개 표시) 사이클 수
  *      → I2C_Write × 4회 + delay 포함
  *
  * [UART 출력 예시]
  *   =====================================================
  *    I2C LCD: HAL vs Inline ASM  |  NUCLEO-F411RE
  *    SYSCLK: 84MHz  I2C: 400kHz
  *   =====================================================
  *   [DWT Cycle Measurement]
  *   HAL I2C_Write  1byte : 2150 cycles  (25.6 us)
  *   ASM I2C_Write  1byte :  980 cycles  (11.7 us)
  *   HAL LCD_DATA   1char : 9800 cycles  (116.7 us)
  *   ASM LCD_DATA   1char : 4500 cycles  ( 53.6 us)
  *   HAL/ASM ratio        : ~2.2x
  *   -----------------------------------------------------
  *   [이유]
  *   HAL: 상태머신 + 에러처리 + 파라미터 검증 오버헤드
  *   ASM: SR1 폴링 + DR 쓰기 직접 수행 = 최소 명령어
  *   [LCD는 동일하게 표시됨 - 기능은 동등]
  *   =====================================================
  *
  * [주의] I2C는 400kHz 버스 클럭에 종속되므로 실제 전송시간은
  *        HAL/ASM 모두 동일. 차이는 CPU 사이클(오버헤드) 에서 발생.
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"
#include <stdio.h>
#include <string.h>

/* =========================================================================
 * I2C1 레지스터 주소
 * =========================================================================*/
#define I2C1_BASE       0x40005400UL
#define I2C1_CR1        (I2C1_BASE + 0x00UL)
#define I2C1_DR         (I2C1_BASE + 0x0CUL)
#define I2C1_SR1        (I2C1_BASE + 0x14UL)
#define I2C1_SR2        (I2C1_BASE + 0x18UL)

#define CR1_START       (1UL <<  8)
#define CR1_STOP        (1UL <<  9)
#define SR1_SB          (1UL <<  0)
#define SR1_ADDR        (1UL <<  1)
#define SR1_BTF         (1UL <<  2)
#define SR1_TXE         (1UL <<  7)

/* =========================================================================
 * PCF8574 / LCD 상수
 * =========================================================================*/
#define RS0_EN1     0x04
#define RS0_EN0     0x00
#define RS1_EN1     0x05
#define RS1_EN0     0x01
#define BackLight   0x08
#define ADDRESS     (0x27 << 1)

/* =========================================================================
 * DWT 레지스터
 * =========================================================================*/
#define DWT_CYCCNT  (*(volatile uint32_t *)0xE0001004UL)
#define DWT_CTRL    (*(volatile uint32_t *)0xE0001000UL)

/* ─── 함수 선언 ──────────────────────────────────────────────────────────── */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void DWT_Init(void);

/* 딜레이 */
static void Delay_ASM(uint32_t count);
static void delay_us(uint32_t us);

/* HAL I2C LCD */
static void HAL_LCD_Write(uint8_t addr, uint8_t data);
void HAL_LCD_CMD(uint8_t cmd);
void HAL_LCD_DATA(uint8_t data);
void HAL_LCD_INIT(void);
void HAL_LCD_XY(uint8_t x, uint8_t y);
void HAL_LCD_PUTS(char *str);

/* ASM I2C LCD */
static void ASM_I2C_Start(void);
static void ASM_I2C_Stop(void);
static void ASM_I2C_SendAddr(uint8_t addr);
static void ASM_I2C_SendData(uint8_t data);
static void ASM_LCD_Write(uint8_t addr, uint8_t data);
void ASM_LCD_CMD(uint8_t cmd);
void ASM_LCD_DATA(uint8_t data);
void ASM_LCD_INIT(void);
void ASM_LCD_XY(uint8_t x, uint8_t y);
void ASM_LCD_PUTS(char *str);

/* UART 출력 */
static void UART_Print(const char *str);
static void UART_PrintResult(const char *label, uint32_t cycles, uint32_t sysclk_mhz);

I2C_HandleTypeDef  hi2c1;
UART_HandleTypeDef huart2;

/* =========================================================================
 * printf → USART2
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
    DWT_Init();

    HAL_Delay(200);

    UART_Print("\r\n=====================================================\r\n");
    UART_Print(" I2C LCD: HAL vs Inline ASM  |  NUCLEO-F411RE\r\n");
    UART_Print(" SYSCLK: 84MHz  |  I2C1: 400kHz  |  PCF8574+HD44780\r\n");
    UART_Print("=====================================================\r\n\r\n");

    /* ====================================================================
     * LCD 초기화: HAL로 수행 (초기화 사이클 측정은 제외)
     * ==================================================================== */
    HAL_LCD_INIT();

    HAL_LCD_XY(0, 0);
    HAL_LCD_PUTS("HAL vs ASM");
    HAL_LCD_XY(0, 1);
    HAL_LCD_PUTS("I2C Cycle Test");
    HAL_Delay(2000);

    /* ====================================================================
     * STEP 1: DWT 사이클 측정
     *   동일한 I2C 1바이트 전송 / LCD_DATA 1글자를
     *   HAL과 ASM으로 각각 수행하고 사이클 수 비교
     * ==================================================================== */
    uint32_t t_start, t_end;
    uint32_t hal_write_cycles, asm_write_cycles;
    uint32_t hal_data_cycles,  asm_data_cycles;
    uint8_t dummy = 0x00 | RS0_EN0 | BackLight;   /* 측정용 더미 바이트 */

    /* ── HAL I2C_Write 1회 측정 (3회 중 마지막) ──────────────────────*/
    for (int i = 0; i < 3; i++) {
        DWT_CYCCNT = 0;
        t_start = DWT_CYCCNT;
        HAL_LCD_Write(ADDRESS, dummy);
        t_end = DWT_CYCCNT;
        hal_write_cycles = t_end - t_start;
    }

    /* ── ASM I2C_Write 1회 측정 ───────────────────────────────────────*/
    for (int i = 0; i < 3; i++) {
        DWT_CYCCNT = 0;
        t_start = DWT_CYCCNT;
        ASM_LCD_Write(ADDRESS, dummy);
        t_end = DWT_CYCCNT;
        asm_write_cycles = t_end - t_start;
    }

    /* ── HAL LCD_DATA 1글자 측정 ('A' 출력) ───────────────────────────*/
    for (int i = 0; i < 3; i++) {
        DWT_CYCCNT = 0;
        t_start = DWT_CYCCNT;
        HAL_LCD_DATA('A');
        t_end = DWT_CYCCNT;
        hal_data_cycles = t_end - t_start;
    }

    /* ── ASM LCD_DATA 1글자 측정 ─────────────────────────────────────*/
    for (int i = 0; i < 3; i++) {
        DWT_CYCCNT = 0;
        t_start = DWT_CYCCNT;
        ASM_LCD_DATA('A');
        t_end = DWT_CYCCNT;
        asm_data_cycles = t_end - t_start;
    }

    /* ── UART 결과 출력 ───────────────────────────────────────────────*/
    UART_Print("[STEP 1] DWT Cycle Counter Measurement\r\n");
    UART_Print("-----------------------------------------------------\r\n");
    UART_PrintResult("HAL I2C_Write 1byte ", hal_write_cycles, 84);
    UART_PrintResult("ASM I2C_Write 1byte ", asm_write_cycles, 84);
    UART_Print("-----------------------------------------------------\r\n");
    UART_PrintResult("HAL LCD_DATA  1char ", hal_data_cycles,  84);
    UART_PrintResult("ASM LCD_DATA  1char ", asm_data_cycles,  84);
    UART_Print("-----------------------------------------------------\r\n");

    UART_Print("\r\n[이유]\r\n");
    UART_Print("  HAL: 상태머신 + 에러처리 + 파라미터 검증 오버헤드\r\n");
    UART_Print("  ASM: SR1 폴링 + DR 직접 쓰기 = 최소 명령어\r\n");
    UART_Print("  I2C 버스(400kHz)는 동일 -> CPU 처리 오버헤드 차이\r\n");
    UART_Print("  LCD 표시 결과는 동등 -> 기능 차이 없음\r\n");

    UART_Print("\r\n[STEP 2] LCD 교대 출력 (HAL 3줄 → ASM 3줄)\r\n");
    UART_Print("-----------------------------------------------------\r\n");

    /* ====================================================================
     * STEP 2: LCD 교대 출력
     *   HAL / ASM 동일 문자를 교대로 출력
     *   LCD에 보이는 결과는 동일 → 기능 동등 확인
     * ==================================================================== */
    uint32_t loop = 0;
    char buf[17];

    while (1)
    {
        loop++;

        /* ── HAL 방식으로 LCD 출력 ──────────────────────────────────*/
        UART_Print("[HAL] LCD write start\r\n");
        HAL_LCD_XY(0, 0);  HAL_LCD_PUTS("HAL  I2C Write  ");
        snprintf(buf, sizeof(buf), "Loop:HAL  %5lu", loop);
        HAL_LCD_XY(0, 1);  HAL_LCD_PUTS(buf);
        UART_Print("[HAL] done\r\n");

        HAL_Delay(2000);

        /* ── ASM 방식으로 LCD 출력 ──────────────────────────────────*/
        UART_Print("[ASM] LCD write start\r\n");
        ASM_LCD_XY(0, 0);  ASM_LCD_PUTS("ASM  I2C Write  ");
        snprintf(buf, sizeof(buf), "Loop:ASM  %5lu", loop);
        ASM_LCD_XY(0, 1);  ASM_LCD_PUTS(buf);
        UART_Print("[ASM] done\r\n");
        UART_Print("[NOTE] LCD looks identical -> diff is CPU cycles\r\n\r\n");

        HAL_Delay(2000);

        /* 5회마다 측정 결과 재출력 */
        if (loop % 5 == 0) {
            UART_Print("=====================================================\r\n");
            UART_Print("[재측정 요약]\r\n");
            UART_PrintResult("HAL I2C_Write ", hal_write_cycles, 84);
            UART_PrintResult("ASM I2C_Write ", asm_write_cycles, 84);
            UART_PrintResult("HAL LCD_DATA  ", hal_data_cycles,  84);
            UART_PrintResult("ASM LCD_DATA  ", asm_data_cycles,  84);
            UART_Print("=====================================================\r\n\r\n");
        }
    }
}

/* =========================================================================
 * DWT_Init
 * =========================================================================*/
static void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT_CYCCNT = 0;
    DWT_CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
}

/* =========================================================================
 * Delay_ASM / delay_us
 * =========================================================================*/
static void Delay_ASM(uint32_t count)
{
    __asm volatile (
        "1:              \n\t"
        "SUBS %0, %0, #1 \n\t"
        "BNE  1b         \n\t"
        : "+r"(count) :: "cc"
    );
}

static void delay_us(uint32_t us)
{
    Delay_ASM(us * 3);
}

/* =========================================================================
 * HAL 방식 I2C LCD 함수
 * =========================================================================*/
static void HAL_LCD_Write(uint8_t addr, uint8_t data)
{
    while (HAL_I2C_Master_Transmit(&hi2c1, addr, &data, 1, 1000) != HAL_OK);
}

void HAL_LCD_CMD(uint8_t cmd)
{
    uint8_t temp;
    temp = (cmd & 0xF0) | RS0_EN1 | BackLight;
    HAL_LCD_Write(ADDRESS, temp);
    temp = (cmd & 0xF0) | RS0_EN0 | BackLight;
    HAL_LCD_Write(ADDRESS, temp);
    delay_us(4);

    temp = ((cmd << 4) & 0xF0) | RS0_EN1 | BackLight;
    HAL_LCD_Write(ADDRESS, temp);
    temp = ((cmd << 4) & 0xF0) | RS0_EN0 | BackLight;
    HAL_LCD_Write(ADDRESS, temp);
    delay_us(50);
}

void HAL_LCD_DATA(uint8_t data)
{
    uint8_t temp;
    temp = (data & 0xF0) | RS1_EN1 | BackLight;
    HAL_LCD_Write(ADDRESS, temp);
    temp = (data & 0xF0) | RS1_EN0 | BackLight;
    HAL_LCD_Write(ADDRESS, temp);
    delay_us(4);

    temp = ((data << 4) & 0xF0) | RS1_EN1 | BackLight;
    HAL_LCD_Write(ADDRESS, temp);
    temp = ((data << 4) & 0xF0) | RS1_EN0 | BackLight;
    HAL_LCD_Write(ADDRESS, temp);
    delay_us(50);
}

void HAL_LCD_INIT(void)
{
    HAL_Delay(100);
    uint8_t t;

    t = ((0x03 << 4) & 0xF0) | RS0_EN1 | BackLight;
    HAL_LCD_Write(ADDRESS, t);
    t = ((0x03 << 4) & 0xF0) | RS0_EN0 | BackLight;
    HAL_LCD_Write(ADDRESS, t);
    HAL_Delay(5);

    HAL_LCD_Write(ADDRESS, t);  /* 0x03 × 2회 더 반복 */
    delay_us(100);
    HAL_LCD_Write(ADDRESS, t);
    delay_us(100);

    t = ((0x02 << 4) & 0xF0) | RS0_EN1 | BackLight;
    HAL_LCD_Write(ADDRESS, t);
    t = ((0x02 << 4) & 0xF0) | RS0_EN0 | BackLight;
    HAL_LCD_Write(ADDRESS, t);
    delay_us(100);

    HAL_LCD_CMD(0x28);
    HAL_LCD_CMD(0x08);
    HAL_LCD_CMD(0x01);
    HAL_Delay(3);
    HAL_LCD_CMD(0x06);
    HAL_LCD_CMD(0x0C);
}

void HAL_LCD_XY(uint8_t x, uint8_t y)
{
    const uint8_t row[] = {0x80, 0xC0, 0x94, 0xD4};
    if (y < 4) HAL_LCD_CMD(row[y] + x);
}

void HAL_LCD_PUTS(char *str)
{
    while (*str) HAL_LCD_DATA((uint8_t)*str++);
}

/* =========================================================================
 * ASM 방식 I2C LCD 함수
 * =========================================================================*/
static void ASM_I2C_Start(void)
{
    __asm volatile (
        "LDR  r0, =%0          \n\t"
        "LDR  r1, [r0]         \n\t"
        "ORR  r1, r1, %1       \n\t"
        "STR  r1, [r0]         \n\t"
        "LDR  r0, =%2          \n\t"
        "1:                    \n\t"
        "LDR  r1, [r0]         \n\t"
        "TST  r1, %3           \n\t"
        "BEQ  1b               \n\t"
        :
        : "i"(I2C1_CR1), "i"(CR1_START),
          "i"(I2C1_SR1), "i"(SR1_SB)
        : "r0", "r1", "memory"
    );
}

static void ASM_I2C_Stop(void)
{
    __asm volatile (
        "LDR  r0, =%0          \n\t"
        "LDR  r1, [r0]         \n\t"
        "ORR  r1, r1, %1       \n\t"
        "STR  r1, [r0]         \n\t"
        :
        : "i"(I2C1_CR1), "i"(CR1_STOP)
        : "r0", "r1", "memory"
    );
}

static void ASM_I2C_SendAddr(uint8_t addr)
{
    __asm volatile (
        "LDR  r0, =%1          \n\t"
        "STRB %0, [r0]         \n\t"
        "LDR  r0, =%2          \n\t"
        "1:                    \n\t"
        "LDR  r1, [r0]         \n\t"
        "TST  r1, %3           \n\t"
        "BEQ  1b               \n\t"
        "LDR  r0, =%4          \n\t"
        "LDR  r1, [r0]         \n\t"
        :
        : "r"((uint32_t)addr),
          "i"(I2C1_DR),
          "i"(I2C1_SR1), "i"(SR1_ADDR),
          "i"(I2C1_SR2)
        : "r0", "r1", "memory"
    );
}

static void ASM_I2C_SendData(uint8_t data)
{
    __asm volatile (
        "LDR  r0, =%1          \n\t"
        "1:                    \n\t"
        "LDR  r1, [r0]         \n\t"
        "TST  r1, %2           \n\t"
        "BEQ  1b               \n\t"
        "LDR  r0, =%3          \n\t"
        "STRB %0, [r0]         \n\t"
        "LDR  r0, =%1          \n\t"
        "2:                    \n\t"
        "LDR  r1, [r0]         \n\t"
        "TST  r1, %4           \n\t"
        "BEQ  2b               \n\t"
        :
        : "r"((uint32_t)data),
          "i"(I2C1_SR1), "i"(SR1_TXE),
          "i"(I2C1_DR),
          "i"(SR1_BTF)
        : "r0", "r1", "memory"
    );
}

static void ASM_LCD_Write(uint8_t addr, uint8_t data)
{
    ASM_I2C_Start();
    ASM_I2C_SendAddr(addr);
    ASM_I2C_SendData(data);
    ASM_I2C_Stop();
}

void ASM_LCD_CMD(uint8_t cmd)
{
    uint8_t temp;
    temp = (cmd & 0xF0) | RS0_EN1 | BackLight;
    ASM_LCD_Write(ADDRESS, temp);
    temp = (cmd & 0xF0) | RS0_EN0 | BackLight;
    ASM_LCD_Write(ADDRESS, temp);
    delay_us(4);

    temp = ((cmd << 4) & 0xF0) | RS0_EN1 | BackLight;
    ASM_LCD_Write(ADDRESS, temp);
    temp = ((cmd << 4) & 0xF0) | RS0_EN0 | BackLight;
    ASM_LCD_Write(ADDRESS, temp);
    delay_us(50);
}

void ASM_LCD_DATA(uint8_t data)
{
    uint8_t temp;
    temp = (data & 0xF0) | RS1_EN1 | BackLight;
    ASM_LCD_Write(ADDRESS, temp);
    temp = (data & 0xF0) | RS1_EN0 | BackLight;
    ASM_LCD_Write(ADDRESS, temp);
    delay_us(4);

    temp = ((data << 4) & 0xF0) | RS1_EN1 | BackLight;
    ASM_LCD_Write(ADDRESS, temp);
    temp = ((data << 4) & 0xF0) | RS1_EN0 | BackLight;
    ASM_LCD_Write(ADDRESS, temp);
    delay_us(50);
}

void ASM_LCD_INIT(void)
{
    HAL_Delay(100);

    ASM_LCD_Write(ADDRESS, ((0x03<<4)&0xF0)|RS0_EN1|BackLight);
    ASM_LCD_Write(ADDRESS, ((0x03<<4)&0xF0)|RS0_EN0|BackLight);
    HAL_Delay(5);
    ASM_LCD_Write(ADDRESS, ((0x03<<4)&0xF0)|RS0_EN1|BackLight);
    ASM_LCD_Write(ADDRESS, ((0x03<<4)&0xF0)|RS0_EN0|BackLight);
    delay_us(100);
    ASM_LCD_Write(ADDRESS, ((0x03<<4)&0xF0)|RS0_EN1|BackLight);
    ASM_LCD_Write(ADDRESS, ((0x03<<4)&0xF0)|RS0_EN0|BackLight);
    delay_us(100);
    ASM_LCD_Write(ADDRESS, ((0x02<<4)&0xF0)|RS0_EN1|BackLight);
    ASM_LCD_Write(ADDRESS, ((0x02<<4)&0xF0)|RS0_EN0|BackLight);
    delay_us(100);

    ASM_LCD_CMD(0x28);
    ASM_LCD_CMD(0x08);
    ASM_LCD_CMD(0x01);
    HAL_Delay(3);
    ASM_LCD_CMD(0x06);
    ASM_LCD_CMD(0x0C);
}

void ASM_LCD_XY(uint8_t x, uint8_t y)
{
    const uint8_t row[] = {0x80, 0xC0, 0x94, 0xD4};
    if (y < 4) ASM_LCD_CMD(row[y] + x);
}

void ASM_LCD_PUTS(char *str)
{
    while (*str) ASM_LCD_DATA((uint8_t)*str++);
}

/* =========================================================================
 * UART_Print / UART_PrintResult
 *   label: cycles (us 환산)  형태 출력
 *   us = cycles / 84
 * =========================================================================*/
static void UART_Print(const char *str)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}

static void UART_PrintResult(const char *label, uint32_t cycles, uint32_t sysclk_mhz)
{
    char buf[80];
    uint32_t us_int  = cycles / sysclk_mhz;          /* 정수 부분          */
    uint32_t us_frac = (cycles % sysclk_mhz) * 10 / sysclk_mhz; /* 소수 1자리 */

    /* uint32_t → 문자열 변환 (sprintf 없이) */
    auto void u2s(uint32_t v, char *s, int *len);
    void u2s(uint32_t v, char *s, int *len) {
        *len = 0;
        if (v == 0) { s[(*len)++] = '0'; return; }
        char tmp[12]; int t = 0;
        while (v > 0) { tmp[t++] = '0' + v % 10; v /= 10; }
        for (int i = t-1; i >= 0; i--) s[(*len)++] = tmp[i];
    }

    char sc[12], su[12], sf[4];
    int lc, lu;
    u2s(cycles,   sc, &lc); sc[lc] = '\0';
    u2s(us_int,   su, &lu); su[lu] = '\0';
    sf[0] = '0' + (char)us_frac; sf[1] = '\0';

    int pos = 0;
    for (int i = 0; label[i]; i++) buf[pos++] = label[i];
    buf[pos++] = ':'; buf[pos++] = ' ';
    for (int i = 0; sc[i]; i++) buf[pos++] = sc[i];
    const char *s1 = " cycles  (";
    for (int i = 0; s1[i]; i++) buf[pos++] = s1[i];
    for (int i = 0; su[i]; i++) buf[pos++] = su[i];
    buf[pos++] = '.'; buf[pos++] = sf[0];
    const char *s2 = " us)\r\n";
    for (int i = 0; s2[i]; i++) buf[pos++] = s2[i];
    buf[pos] = '\0';

    UART_Print(buf);
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
