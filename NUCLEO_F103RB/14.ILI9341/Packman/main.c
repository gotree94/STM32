/* ============================================================================
 * STM32F103 + ILI9341 LCD (240x320) - PAC-MAN Game
 * ============================================================================
 */

#include "main.h"
#include "packman.h"
#include <string.h>
#include <stdlib.h>

// ============================================================================
// Fast GPIO macros (register direct access)
// ============================================================================
#define LCD_CS_LOW()    GPIOB->BRR = GPIO_PIN_0
#define LCD_CS_HIGH()   GPIOB->BSRR = GPIO_PIN_0
#define LCD_RS_LOW()    GPIOA->BRR = GPIO_PIN_4
#define LCD_RS_HIGH()   GPIOA->BSRR = GPIO_PIN_4
#define LCD_WR_LOW()    GPIOA->BRR = GPIO_PIN_1
#define LCD_WR_HIGH()   GPIOA->BSRR = GPIO_PIN_1
#define LCD_RD_HIGH()   GPIOA->BSRR = GPIO_PIN_0
#define LCD_RST_LOW()   GPIOC->BRR = GPIO_PIN_1
#define LCD_RST_HIGH()  GPIOC->BSRR = GPIO_PIN_1

static inline void LCD_Write8Fast(uint8_t data) {
    uint32_t pa_set = 0, pa_clr = 0;
    if(data & 0x01) pa_set |= GPIO_PIN_9;  else pa_clr |= GPIO_PIN_9;
    if(data & 0x04) pa_set |= GPIO_PIN_10; else pa_clr |= GPIO_PIN_10;
    if(data & 0x80) pa_set |= GPIO_PIN_8;  else pa_clr |= GPIO_PIN_8;

    uint32_t pb_set = 0, pb_clr = 0;
    if(data & 0x08) pb_set |= GPIO_PIN_3;  else pb_clr |= GPIO_PIN_3;
    if(data & 0x10) pb_set |= GPIO_PIN_5;  else pb_clr |= GPIO_PIN_5;
    if(data & 0x20) pb_set |= GPIO_PIN_4;  else pb_clr |= GPIO_PIN_4;
    if(data & 0x40) pb_set |= GPIO_PIN_10; else pb_clr |= GPIO_PIN_10;

    uint32_t pc_set = 0, pc_clr = 0;
    if(data & 0x02) pc_set |= GPIO_PIN_7;  else pc_clr |= GPIO_PIN_7;

    GPIOA->BSRR = pa_set | (pa_clr << 16);
    GPIOB->BSRR = pb_set | (pb_clr << 16);
    GPIOC->BSRR = pc_set | (pc_clr << 16);

    LCD_WR_LOW();
    __NOP();
    LCD_WR_HIGH();
}

static inline void LCD_Write16Fast(uint16_t color) {
    LCD_Write8Fast(color >> 8);
    LCD_Write8Fast(color & 0xFF);
}

static void LCD_WriteCommand(uint8_t cmd) {
    LCD_CS_LOW(); LCD_RS_LOW(); LCD_Write8Fast(cmd); LCD_CS_HIGH();
}

static void LCD_WriteData(uint8_t data) {
    LCD_CS_LOW(); LCD_RS_HIGH(); LCD_Write8Fast(data); LCD_CS_HIGH();
}

static void LCD_SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    LCD_WriteCommand(0x2A);
    LCD_CS_LOW(); LCD_RS_HIGH();
    LCD_Write8Fast(x1 >> 8); LCD_Write8Fast(x1 & 0xFF);
    LCD_Write8Fast(x2 >> 8); LCD_Write8Fast(x2 & 0xFF);
    LCD_CS_HIGH();
    LCD_WriteCommand(0x2B);
    LCD_CS_LOW(); LCD_RS_HIGH();
    LCD_Write8Fast(y1 >> 8); LCD_Write8Fast(y1 & 0xFF);
    LCD_Write8Fast(y2 >> 8); LCD_Write8Fast(y2 & 0xFF);
    LCD_CS_HIGH();
    LCD_WriteCommand(0x2C);
}

static void LCD_FillRectFast(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if(x >= 240 || y >= 320 || w == 0 || h == 0) return;
    if(x + w > 240) w = 240 - x;
    if(y + h > 320) h = 320 - y;

    LCD_SetWindow(x, y, x + w - 1, y + h - 1);
    uint32_t total = (uint32_t)w * h;
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;
    LCD_CS_LOW(); LCD_RS_HIGH();
    while(total >= 8) {
        LCD_Write8Fast(hi); LCD_Write8Fast(lo);
        LCD_Write8Fast(hi); LCD_Write8Fast(lo);
        LCD_Write8Fast(hi); LCD_Write8Fast(lo);
        LCD_Write8Fast(hi); LCD_Write8Fast(lo);
        LCD_Write8Fast(hi); LCD_Write8Fast(lo);
        LCD_Write8Fast(hi); LCD_Write8Fast(lo);
        LCD_Write8Fast(hi); LCD_Write8Fast(lo);
        LCD_Write8Fast(hi); LCD_Write8Fast(lo);
        total -= 8;
    }
    while(total--) { LCD_Write8Fast(hi); LCD_Write8Fast(lo); }
    LCD_CS_HIGH();
}

static void LCD_Fill(uint16_t color) {
    LCD_FillRectFast(0, 0, 240, 320, color);
}

// ============================================================================
// LCD Initialization
// ============================================================================
static void LCD_Init(void) {
    LCD_RD_HIGH();
    LCD_CS_HIGH();

    LCD_RST_LOW(); HAL_Delay(50);
    LCD_RST_HIGH(); HAL_Delay(50);

    LCD_WriteCommand(0x01); HAL_Delay(100);
    LCD_WriteCommand(0x11); HAL_Delay(120);

    LCD_WriteCommand(0xCF);
    LCD_WriteData(0x00); LCD_WriteData(0xC1); LCD_WriteData(0x30);
    LCD_WriteCommand(0xED);
    LCD_WriteData(0x64); LCD_WriteData(0x03); LCD_WriteData(0x12); LCD_WriteData(0x81);
    LCD_WriteCommand(0xE8);
    LCD_WriteData(0x85); LCD_WriteData(0x00); LCD_WriteData(0x78);
    LCD_WriteCommand(0xCB);
    LCD_WriteData(0x39); LCD_WriteData(0x2C); LCD_WriteData(0x00);
    LCD_WriteData(0x34); LCD_WriteData(0x02);
    LCD_WriteCommand(0xF7); LCD_WriteData(0x20);
    LCD_WriteCommand(0xEA); LCD_WriteData(0x00); LCD_WriteData(0x00);
    LCD_WriteCommand(0xC0); LCD_WriteData(0x23);
    LCD_WriteCommand(0xC1); LCD_WriteData(0x10);
    LCD_WriteCommand(0xC5); LCD_WriteData(0x3E); LCD_WriteData(0x28);
    LCD_WriteCommand(0xC7); LCD_WriteData(0x86);
    LCD_WriteCommand(0x36); LCD_WriteData(0x48);
    LCD_WriteCommand(0x3A); LCD_WriteData(0x55);
    LCD_WriteCommand(0xB1); LCD_WriteData(0x00); LCD_WriteData(0x18);
    LCD_WriteCommand(0xB6); LCD_WriteData(0x08); LCD_WriteData(0x82); LCD_WriteData(0x27);
    LCD_WriteCommand(0xF2); LCD_WriteData(0x00);
    LCD_WriteCommand(0x26); LCD_WriteData(0x01);
    LCD_WriteCommand(0xE0);
    LCD_WriteData(0x0F); LCD_WriteData(0x31); LCD_WriteData(0x2B); LCD_WriteData(0x0C);
    LCD_WriteData(0x0E); LCD_WriteData(0x08); LCD_WriteData(0x4E); LCD_WriteData(0xF1);
    LCD_WriteData(0x37); LCD_WriteData(0x07); LCD_WriteData(0x10); LCD_WriteData(0x03);
    LCD_WriteData(0x0E); LCD_WriteData(0x09); LCD_WriteData(0x00);
    LCD_WriteCommand(0xE1);
    LCD_WriteData(0x00); LCD_WriteData(0x0E); LCD_WriteData(0x14); LCD_WriteData(0x03);
    LCD_WriteData(0x11); LCD_WriteData(0x07); LCD_WriteData(0x31); LCD_WriteData(0xC1);
    LCD_WriteData(0x48); LCD_WriteData(0x08); LCD_WriteData(0x0F); LCD_WriteData(0x0C);
    LCD_WriteData(0x31); LCD_WriteData(0x36); LCD_WriteData(0x0F);

    LCD_WriteCommand(0x11); HAL_Delay(120);
    LCD_WriteCommand(0x29); HAL_Delay(50);
}

// ============================================================================
// GPIO Initialization (LCD + Button)
// ============================================================================
static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // LCD control pins initial state
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    // LCD RST (PC1)
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    // LCD Control (PA0, PA1, PA4) + Data (PA8, PA9, PA10)
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // LCD CS (PB0) + Data (PB3, PB4, PB5, PB10)
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_10;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // LCD Data D1 (PC7)
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    // Button B1 (PC13) - Input with pull-up
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

// ============================================================================
// System Clock (64MHz via HSI + PLL)
// ============================================================================
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) Error_Handler();
}

// ============================================================================
// Button state tracking
// ============================================================================
static uint8_t btn_prev = 1;
static uint32_t btn_press_ms = 0;

static void handle_button(void) {
    uint8_t btn_now = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);

    if (btn_prev == 1 && btn_now == 0) {
        // Press detected
        btn_press_ms = HAL_GetTick();
    }

    if (btn_prev == 0 && btn_now == 1) {
        // Release detected
        uint32_t hold_ms = HAL_GetTick() - btn_press_ms;
        if (hold_ms < 500) {
            // Short press
            Packman_OnButton();
        }
    }

    // Long press handling (while held)
    if (btn_now == 0 && btn_prev == 0) {
        if (HAL_GetTick() - btn_press_ms > 500) {
            // Long press - could toggle pause, but simple: do nothing
            btn_press_ms = HAL_GetTick(); // reset to avoid repeat
        }
    }

    btn_prev = btn_now;
}

// ============================================================================
// Main
// ============================================================================
int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    LCD_Init();
    LCD_Fill(0x0000);

    srand(HAL_GetTick());

    Packman_Init();

    while (1) {
        handle_button();

        uint32_t now = HAL_GetTick();
        static uint32_t last_tick = 0;
        if (now - last_tick >= TICK_MS) {
            Packman_Tick();
            last_tick = now;
        }

        HAL_Delay(10);
    }
}

void Error_Handler(void) {
    __disable_irq();
    while(1) {}
}
