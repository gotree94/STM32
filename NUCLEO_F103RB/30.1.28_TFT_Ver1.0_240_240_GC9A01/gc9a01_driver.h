/*
 * gc9a01_driver.h
 * GC9A01 Round LCD Driver for STM32
 */

#ifndef GC9A01_DRIVER_H
#define GC9A01_DRIVER_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

// LCD 설정
#define LCD_WIDTH  240
#define LCD_HEIGHT 240

// GPIO 핀 정의 (필요에 따라 수정)
#define LCD_CS_PIN   GPIO_PIN_4
#define LCD_CS_PORT  GPIOA
#define LCD_RST_PIN  GPIO_PIN_8
#define LCD_RST_PORT GPIOA
#define LCD_DC_PIN   GPIO_PIN_9
#define LCD_DC_PORT  GPIOA

// 색상 정의 (RGB565)
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0
#define COLOR_CYAN    0x07FF
#define COLOR_MAGENTA 0xF81F
#define COLOR_GRAY    0x8410
#define COLOR_ORANGE  0xFD20

// GC9A01 명령어
#define GC9A01_SLPOUT   0x11
#define GC9A01_INVON    0x21
#define GC9A01_DISPON   0x29
#define GC9A01_CASET    0x2A
#define GC9A01_RASET    0x2B
#define GC9A01_RAMWR    0x2C
#define GC9A01_MADCTL   0x36
#define GC9A01_COLMOD   0x3A

// 함수 선언
void GC9A01_Init(SPI_HandleTypeDef *hspi);
void GC9A01_SetRotation(uint8_t rotation);
void GC9A01_FillScreen(uint16_t color);
void GC9A01_DrawPixel(int16_t x, int16_t y, uint16_t color);
void GC9A01_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void GC9A01_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void GC9A01_DrawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
void GC9A01_FillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
void GC9A01_DrawChar(int16_t x, int16_t y, char c, uint16_t color, uint16_t bg, uint8_t size);
void GC9A01_DrawString(int16_t x, int16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size);
void GC9A01_DrawNumber(int16_t x, int16_t y, int32_t num, uint16_t color, uint16_t bg, uint8_t size);

// 내부 함수
void GC9A01_WriteCommand(uint8_t cmd);
void GC9A01_WriteData(uint8_t data);
void GC9A01_WriteData16(uint16_t data);
void GC9A01_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

#endif // GC9A01_DRIVER_H