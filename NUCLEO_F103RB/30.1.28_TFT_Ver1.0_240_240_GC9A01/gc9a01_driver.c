/*
 * gc9a01_driver.c
 * GC9A01 Round LCD Driver Implementation
 */

#include "gc9a01_driver.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static SPI_HandleTypeDef *spi_handle;

// 5x7 폰트 (ASCII 숫자와 콜론)
static const uint8_t font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // ' ' (space) - index 0
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // '0' - index 1
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // '1' - index 2
    {0x42, 0x61, 0x51, 0x49, 0x46}, // '2' - index 3
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // '3' - index 4
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // '4' - index 5
    {0x27, 0x45, 0x45, 0x45, 0x39}, // '5' - index 6
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // '6' - index 7
    {0x01, 0x71, 0x09, 0x05, 0x03}, // '7' - index 8
    {0x36, 0x49, 0x49, 0x49, 0x36}, // '8' - index 9
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // '9' - index 10
    {0x00, 0x36, 0x36, 0x00, 0x00}, // ':' - index 11
};

// GPIO 제어 매크로
#define CS_LOW()  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET)
#define CS_HIGH() HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET)
#define DC_LOW()  HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET)
#define DC_HIGH() HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET)
#define RST_LOW() HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_RESET)
#define RST_HIGH() HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET)

void GC9A01_WriteCommand(uint8_t cmd) {
    DC_LOW();
    CS_LOW();
    HAL_SPI_Transmit(spi_handle, &cmd, 1, HAL_MAX_DELAY);
    CS_HIGH();
}

void GC9A01_WriteData(uint8_t data) {
    DC_HIGH();
    CS_LOW();
    HAL_SPI_Transmit(spi_handle, &data, 1, HAL_MAX_DELAY);
    CS_HIGH();
}

void GC9A01_WriteData16(uint16_t data) {
    uint8_t buffer[2] = {data >> 8, data & 0xFF};
    DC_HIGH();
    CS_LOW();
    HAL_SPI_Transmit(spi_handle, buffer, 2, HAL_MAX_DELAY);
    CS_HIGH();
}

void GC9A01_Init(SPI_HandleTypeDef *hspi) {
    spi_handle = hspi;

    // 하드웨어 리셋
    CS_HIGH();
    RST_HIGH();
    HAL_Delay(10);
    RST_LOW();
    HAL_Delay(10);
    RST_HIGH();
    HAL_Delay(120);

    // 초기화 명령어 시퀀스
    GC9A01_WriteCommand(0xEF);

    GC9A01_WriteCommand(0xEB);
    GC9A01_WriteData(0x14);

    GC9A01_WriteCommand(0xFE);
    GC9A01_WriteCommand(0xEF);

    GC9A01_WriteCommand(0xEB);
    GC9A01_WriteData(0x14);

    GC9A01_WriteCommand(0x84);
    GC9A01_WriteData(0x40);

    GC9A01_WriteCommand(0x85);
    GC9A01_WriteData(0xFF);

    GC9A01_WriteCommand(0x86);
    GC9A01_WriteData(0xFF);

    GC9A01_WriteCommand(0x87);
    GC9A01_WriteData(0xFF);

    GC9A01_WriteCommand(0x88);
    GC9A01_WriteData(0x0A);

    GC9A01_WriteCommand(0x89);
    GC9A01_WriteData(0x21);

    GC9A01_WriteCommand(0x8A);
    GC9A01_WriteData(0x00);

    GC9A01_WriteCommand(0x8B);
    GC9A01_WriteData(0x80);

    GC9A01_WriteCommand(0x8C);
    GC9A01_WriteData(0x01);

    GC9A01_WriteCommand(0x8D);
    GC9A01_WriteData(0x01);

    GC9A01_WriteCommand(0x8E);
    GC9A01_WriteData(0xFF);

    GC9A01_WriteCommand(0x8F);
    GC9A01_WriteData(0xFF);

    GC9A01_WriteCommand(0xB6);
    GC9A01_WriteData(0x00);
    GC9A01_WriteData(0x00);

//    GC9A01_WriteCommand(GC9A01_MADCTL);
//    GC9A01_WriteData(0x08);
    GC9A01_WriteCommand(GC9A01_MADCTL);
    GC9A01_WriteData(0x48);  // ★ 0x08에서 0x48로 변경

    GC9A01_WriteCommand(GC9A01_COLMOD);
    GC9A01_WriteData(0x05); // 16-bit color

    GC9A01_WriteCommand(0x90);
    GC9A01_WriteData(0x08);
    GC9A01_WriteData(0x08);
    GC9A01_WriteData(0x08);
    GC9A01_WriteData(0x08);

    GC9A01_WriteCommand(0xBD);
    GC9A01_WriteData(0x06);

    GC9A01_WriteCommand(0xBC);
    GC9A01_WriteData(0x00);

    GC9A01_WriteCommand(0xFF);
    GC9A01_WriteData(0x60);
    GC9A01_WriteData(0x01);
    GC9A01_WriteData(0x04);

    GC9A01_WriteCommand(0xC3);
    GC9A01_WriteData(0x13);
    GC9A01_WriteCommand(0xC4);
    GC9A01_WriteData(0x13);

    GC9A01_WriteCommand(0xC9);
    GC9A01_WriteData(0x22);

    GC9A01_WriteCommand(0xBE);
    GC9A01_WriteData(0x11);

    GC9A01_WriteCommand(0xE1);
    GC9A01_WriteData(0x10);
    GC9A01_WriteData(0x0E);

    GC9A01_WriteCommand(0xDF);
    GC9A01_WriteData(0x21);
    GC9A01_WriteData(0x0C);
    GC9A01_WriteData(0x02);

    GC9A01_WriteCommand(0xF0);
    GC9A01_WriteData(0x45);
    GC9A01_WriteData(0x09);
    GC9A01_WriteData(0x08);
    GC9A01_WriteData(0x08);
    GC9A01_WriteData(0x26);
    GC9A01_WriteData(0x2A);

    GC9A01_WriteCommand(0xF1);
    GC9A01_WriteData(0x43);
    GC9A01_WriteData(0x70);
    GC9A01_WriteData(0x72);
    GC9A01_WriteData(0x36);
    GC9A01_WriteData(0x37);
    GC9A01_WriteData(0x6F);

    GC9A01_WriteCommand(0xF2);
    GC9A01_WriteData(0x45);
    GC9A01_WriteData(0x09);
    GC9A01_WriteData(0x08);
    GC9A01_WriteData(0x08);
    GC9A01_WriteData(0x26);
    GC9A01_WriteData(0x2A);

    GC9A01_WriteCommand(0xF3);
    GC9A01_WriteData(0x43);
    GC9A01_WriteData(0x70);
    GC9A01_WriteData(0x72);
    GC9A01_WriteData(0x36);
    GC9A01_WriteData(0x37);
    GC9A01_WriteData(0x6F);

    GC9A01_WriteCommand(0xED);
    GC9A01_WriteData(0x1B);
    GC9A01_WriteData(0x0B);

    GC9A01_WriteCommand(0xAE);
    GC9A01_WriteData(0x77);

    GC9A01_WriteCommand(0xCD);
    GC9A01_WriteData(0x63);

    GC9A01_WriteCommand(0x70);
    GC9A01_WriteData(0x07);
    GC9A01_WriteData(0x07);
    GC9A01_WriteData(0x04);
    GC9A01_WriteData(0x0E);
    GC9A01_WriteData(0x0F);
    GC9A01_WriteData(0x09);
    GC9A01_WriteData(0x07);
    GC9A01_WriteData(0x08);
    GC9A01_WriteData(0x03);

    GC9A01_WriteCommand(0xE8);
    GC9A01_WriteData(0x34);

    GC9A01_WriteCommand(0x62);
    GC9A01_WriteData(0x18);
    GC9A01_WriteData(0x0D);
    GC9A01_WriteData(0x71);
    GC9A01_WriteData(0xED);
    GC9A01_WriteData(0x70);
    GC9A01_WriteData(0x70);
    GC9A01_WriteData(0x18);
    GC9A01_WriteData(0x0F);
    GC9A01_WriteData(0x71);
    GC9A01_WriteData(0xEF);
    GC9A01_WriteData(0x70);
    GC9A01_WriteData(0x70);

    GC9A01_WriteCommand(0x63);
    GC9A01_WriteData(0x18);
    GC9A01_WriteData(0x11);
    GC9A01_WriteData(0x71);
    GC9A01_WriteData(0xF1);
    GC9A01_WriteData(0x70);
    GC9A01_WriteData(0x70);
    GC9A01_WriteData(0x18);
    GC9A01_WriteData(0x13);
    GC9A01_WriteData(0x71);
    GC9A01_WriteData(0xF3);
    GC9A01_WriteData(0x70);
    GC9A01_WriteData(0x70);

    GC9A01_WriteCommand(0x64);
    GC9A01_WriteData(0x28);
    GC9A01_WriteData(0x29);
    GC9A01_WriteData(0xF1);
    GC9A01_WriteData(0x01);
    GC9A01_WriteData(0xF1);
    GC9A01_WriteData(0x00);
    GC9A01_WriteData(0x07);

    GC9A01_WriteCommand(0x66);
    GC9A01_WriteData(0x3C);
    GC9A01_WriteData(0x00);
    GC9A01_WriteData(0xCD);
    GC9A01_WriteData(0x67);
    GC9A01_WriteData(0x45);
    GC9A01_WriteData(0x45);
    GC9A01_WriteData(0x10);
    GC9A01_WriteData(0x00);
    GC9A01_WriteData(0x00);
    GC9A01_WriteData(0x00);

    GC9A01_WriteCommand(0x67);
    GC9A01_WriteData(0x00);
    GC9A01_WriteData(0x3C);
    GC9A01_WriteData(0x00);
    GC9A01_WriteData(0x00);
    GC9A01_WriteData(0x00);
    GC9A01_WriteData(0x01);
    GC9A01_WriteData(0x54);
    GC9A01_WriteData(0x10);
    GC9A01_WriteData(0x32);
    GC9A01_WriteData(0x98);

    GC9A01_WriteCommand(0x74);
    GC9A01_WriteData(0x10);
    GC9A01_WriteData(0x85);
    GC9A01_WriteData(0x80);
    GC9A01_WriteData(0x00);
    GC9A01_WriteData(0x00);
    GC9A01_WriteData(0x4E);
    GC9A01_WriteData(0x00);

    GC9A01_WriteCommand(0x98);
    GC9A01_WriteData(0x3E);
    GC9A01_WriteData(0x07);

    GC9A01_WriteCommand(0x35);
    GC9A01_WriteCommand(GC9A01_SLPOUT);
    HAL_Delay(120);

    GC9A01_WriteCommand(GC9A01_INVON);
    GC9A01_WriteCommand(GC9A01_DISPON);
    HAL_Delay(20);
}

void GC9A01_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    GC9A01_WriteCommand(GC9A01_CASET);
    GC9A01_WriteData16(x0);
    GC9A01_WriteData16(x1);

    GC9A01_WriteCommand(GC9A01_RASET);
    GC9A01_WriteData16(y0);
    GC9A01_WriteData16(y1);

    GC9A01_WriteCommand(GC9A01_RAMWR);
}

void GC9A01_FillScreen(uint16_t color) {
    GC9A01_FillRect(0, 0, LCD_WIDTH, LCD_HEIGHT, color);
}

void GC9A01_DrawPixel(int16_t x, int16_t y, uint16_t color) {
    if (x < 0 || x >= LCD_WIDTH || y < 0 || y >= LCD_HEIGHT) return;

    GC9A01_SetWindow(x, y, x, y);
    GC9A01_WriteData16(color);
}

void GC9A01_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    if (x + w > LCD_WIDTH) w = LCD_WIDTH - x;
    if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (w <= 0 || h <= 0) return;

    GC9A01_SetWindow(x, y, x + w - 1, y + h - 1);

    uint8_t colorH = color >> 8;
    uint8_t colorL = color & 0xFF;

    DC_HIGH();
    CS_LOW();

    for (uint32_t i = 0; i < (uint32_t)w * h; i++) {
        uint8_t buffer[2] = {colorH, colorL};
        HAL_SPI_Transmit(spi_handle, buffer, 2, HAL_MAX_DELAY);
    }

    CS_HIGH();
}

void GC9A01_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;

    while (1) {
        GC9A01_DrawPixel(x0, y0, color);

        if (x0 == x1 && y0 == y1) break;

        int16_t e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void GC9A01_DrawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    GC9A01_DrawPixel(x0, y0 + r, color);
    GC9A01_DrawPixel(x0, y0 - r, color);
    GC9A01_DrawPixel(x0 + r, y0, color);
    GC9A01_DrawPixel(x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        GC9A01_DrawPixel(x0 + x, y0 + y, color);
        GC9A01_DrawPixel(x0 - x, y0 + y, color);
        GC9A01_DrawPixel(x0 + x, y0 - y, color);
        GC9A01_DrawPixel(x0 - x, y0 - y, color);
        GC9A01_DrawPixel(x0 + y, y0 + x, color);
        GC9A01_DrawPixel(x0 - y, y0 + x, color);
        GC9A01_DrawPixel(x0 + y, y0 - x, color);
        GC9A01_DrawPixel(x0 - y, y0 - x, color);
    }
}

void GC9A01_FillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    for (int16_t y = -r; y <= r; y++) {
        for (int16_t x = -r; x <= r; x++) {
            if (x * x + y * y <= r * r) {
                GC9A01_DrawPixel(x0 + x, y0 + y, color);
            }
        }
    }
}

void GC9A01_DrawChar(int16_t x, int16_t y, char c, uint16_t color, uint16_t bg, uint8_t size) {
    uint8_t index;

    if (c == ' ') {
        index = 0;
    } else if (c >= '0' && c <= '9') {
        index = c - '0' + 1;
    } else if (c == ':') {
        index = 11;
    } else {
        return; // 지원하지 않는 문자
    }

    for (uint8_t i = 0; i < 5; i++) {
        uint8_t line = font5x7[index][i];
        for (uint8_t j = 0; j < 8; j++) {
            if (line & 0x01) {
                if (size == 1) {
                    GC9A01_DrawPixel(x + i, y + j, color);
                } else {
                    GC9A01_FillRect(x + i * size, y + j * size, size, size, color);
                }
            } else if (bg != color) {
                if (size == 1) {
                    GC9A01_DrawPixel(x + i, y + j, bg);
                } else {
                    GC9A01_FillRect(x + i * size, y + j * size, size, size, bg);
                }
            }
            line >>= 1;
        }
    }
}

void GC9A01_DrawString(int16_t x, int16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size) {
    while (*str) {
        GC9A01_DrawChar(x, y, *str++, color, bg, size);
        x += 6 * size;
    }
}

void GC9A01_DrawNumber(int16_t x, int16_t y, int32_t num, uint16_t color, uint16_t bg, uint8_t size) {
    char buffer[12];
    sprintf(buffer, "%d", (int)num);
    GC9A01_DrawString(x, y, buffer, color, bg, size);
}

void GC9A01_SetRotation(uint8_t rotation) {
    GC9A01_WriteCommand(GC9A01_MADCTL);
    switch (rotation % 4) {
        case 0:
            GC9A01_WriteData(0x08);
            break;
        case 1:
            GC9A01_WriteData(0x68);
            break;
        case 2:
            GC9A01_WriteData(0xC8);
            break;
        case 3:
            GC9A01_WriteData(0xA8);
            break;
    }
}
