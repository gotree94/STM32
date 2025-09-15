#include "ili9341.h"
#include <math.h>

static SPI_HandleTypeDef *ili9341_spi;

// 5x7 font data
const uint8_t font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // space
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    // Add more characters as needed...
};

// Control pin macros
#define CS_LOW()    HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_RESET)
#define CS_HIGH()   HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_SET)
#define DC_LOW()    HAL_GPIO_WritePin(ILI9341_DC_PORT, ILI9341_DC_PIN, GPIO_PIN_RESET)
#define DC_HIGH()   HAL_GPIO_WritePin(ILI9341_DC_PORT, ILI9341_DC_PIN, GPIO_PIN_SET)
#define RST_LOW()   HAL_GPIO_WritePin(ILI9341_RST_PORT, ILI9341_RST_PIN, GPIO_PIN_RESET)
#define RST_HIGH()  HAL_GPIO_WritePin(ILI9341_RST_PORT, ILI9341_RST_PIN, GPIO_PIN_SET)

void ILI9341_WriteCommand(uint8_t cmd) {
    DC_LOW();
    CS_LOW();
    HAL_SPI_Transmit(ili9341_spi, &cmd, 1, HAL_MAX_DELAY);
    CS_HIGH();
}

void ILI9341_WriteData(uint8_t data) {
    DC_HIGH();
    CS_LOW();
    HAL_SPI_Transmit(ili9341_spi, &data, 1, HAL_MAX_DELAY);
    CS_HIGH();
}

void ILI9341_WriteData16(uint16_t data) {
    uint8_t buf[2];
    buf[0] = data >> 8;
    buf[1] = data & 0xFF;

    DC_HIGH();
    CS_LOW();
    HAL_SPI_Transmit(ili9341_spi, buf, 2, HAL_MAX_DELAY);
    CS_HIGH();
}

void ILI9341_Init(SPI_HandleTypeDef *hspi) {
    ili9341_spi = hspi;

    // Hardware reset
    RST_HIGH();
    HAL_Delay(10);
    RST_LOW();
    HAL_Delay(10);
    RST_HIGH();
    HAL_Delay(120);

    // Software reset
    ILI9341_WriteCommand(ILI9341_SWRESET);
    HAL_Delay(150);

    // Power control A
    ILI9341_WriteCommand(0xCB);
    ILI9341_WriteData(0x39);
    ILI9341_WriteData(0x2C);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x34);
    ILI9341_WriteData(0x02);

    // Power control B
    ILI9341_WriteCommand(0xCF);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0xC1);
    ILI9341_WriteData(0x30);

    // Driver timing control A
    ILI9341_WriteCommand(0xE8);
    ILI9341_WriteData(0x85);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x78);

    // Driver timing control B
    ILI9341_WriteCommand(0xEA);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x00);

    // Power on sequence control
    ILI9341_WriteCommand(0xED);
    ILI9341_WriteData(0x64);
    ILI9341_WriteData(0x03);
    ILI9341_WriteData(0x12);
    ILI9341_WriteData(0x81);

    // Pump ratio control
    ILI9341_WriteCommand(0xF7);
    ILI9341_WriteData(0x20);

    // Power control 1
    ILI9341_WriteCommand(ILI9341_PWCTR1);
    ILI9341_WriteData(0x23);

    // Power control 2
    ILI9341_WriteCommand(ILI9341_PWCTR2);
    ILI9341_WriteData(0x10);

    // VCOM control 1
    ILI9341_WriteCommand(ILI9341_VMCTR1);
    ILI9341_WriteData(0x3E);
    ILI9341_WriteData(0x28);

    // VCOM control 2
    ILI9341_WriteCommand(ILI9341_VMCTR2);
    ILI9341_WriteData(0x86);

    // Memory access control
    ILI9341_WriteCommand(ILI9341_MADCTL);
    ILI9341_WriteData(0x48);

    // Pixel format
    ILI9341_WriteCommand(ILI9341_PIXFMT);
    ILI9341_WriteData(0x55);

    // Frame rate control
    ILI9341_WriteCommand(ILI9341_FRMCTR1);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x18);

    // Display function control
    ILI9341_WriteCommand(ILI9341_DFUNCTR);
    ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x82);
    ILI9341_WriteData(0x27);

    // Gamma function disable
    ILI9341_WriteCommand(0xF2);
    ILI9341_WriteData(0x00);

    // Gamma curve
    ILI9341_WriteCommand(ILI9341_GAMMASET);
    ILI9341_WriteData(0x01);

    // Positive gamma correction
    ILI9341_WriteCommand(ILI9341_GMCTRP1);
    ILI9341_WriteData(0x0F);
    ILI9341_WriteData(0x31);
    ILI9341_WriteData(0x2B);
    ILI9341_WriteData(0x0C);
    ILI9341_WriteData(0x0E);
    ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x4E);
    ILI9341_WriteData(0xF1);
    ILI9341_WriteData(0x37);
    ILI9341_WriteData(0x07);
    ILI9341_WriteData(0x10);
    ILI9341_WriteData(0x03);
    ILI9341_WriteData(0x0E);
    ILI9341_WriteData(0x09);
    ILI9341_WriteData(0x00);

    // Negative gamma correction
    ILI9341_WriteCommand(ILI9341_GMCTRN1);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x0E);
    ILI9341_WriteData(0x14);
    ILI9341_WriteData(0x03);
    ILI9341_WriteData(0x11);
    ILI9341_WriteData(0x07);
    ILI9341_WriteData(0x31);
    ILI9341_WriteData(0xC1);
    ILI9341_WriteData(0x48);
    ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x0F);
    ILI9341_WriteData(0x0C);
    ILI9341_WriteData(0x31);
    ILI9341_WriteData(0x36);
    ILI9341_WriteData(0x0F);

    // Exit sleep mode
    ILI9341_WriteCommand(ILI9341_SLPOUT);
    HAL_Delay(120);

    // Display on
    ILI9341_WriteCommand(ILI9341_DISPON);
    HAL_Delay(50);

    // Fill screen with black
    ILI9341_Fill(BLACK);
}

void ILI9341_SetAddress(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    ILI9341_WriteCommand(ILI9341_CASET);
    ILI9341_WriteData16(x1);
    ILI9341_WriteData16(x2);

    ILI9341_WriteCommand(ILI9341_PASET);
    ILI9341_WriteData16(y1);
    ILI9341_WriteData16(y2);

    ILI9341_WriteCommand(ILI9341_RAMWR);
}

void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= ILI9341_WIDTH || y >= ILI9341_HEIGHT) return;

    ILI9341_SetAddress(x, y, x, y);
    ILI9341_WriteData16(color);
}

void ILI9341_Fill(uint16_t color) {
    ILI9341_FillRect(0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, color);
}

void ILI9341_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (x >= ILI9341_WIDTH || y >= ILI9341_HEIGHT) return;
    if ((x + w - 1) >= ILI9341_WIDTH) w = ILI9341_WIDTH - x;
    if ((y + h - 1) >= ILI9341_HEIGHT) h = ILI9341_HEIGHT - y;

    ILI9341_SetAddress(x, y, x + w - 1, y + h - 1);

    DC_HIGH();
    CS_LOW();

    uint8_t buf[2];
    buf[0] = color >> 8;
    buf[1] = color & 0xFF;

    for (uint32_t i = 0; i < w * h; i++) {
        HAL_SPI_Transmit(ili9341_spi, buf, 2, HAL_MAX_DELAY);
    }

    CS_HIGH();
}

void ILI9341_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;

    while (1) {
        ILI9341_DrawPixel(x0, y0, color);

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

void ILI9341_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    ILI9341_DrawLine(x, y, x + w - 1, y, color);
    ILI9341_DrawLine(x + w - 1, y, x + w - 1, y + h - 1, color);
    ILI9341_DrawLine(x + w - 1, y + h - 1, x, y + h - 1, color);
    ILI9341_DrawLine(x, y + h - 1, x, y, color);
}

void ILI9341_DrawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    ILI9341_DrawPixel(x0, y0 + r, color);
    ILI9341_DrawPixel(x0, y0 - r, color);
    ILI9341_DrawPixel(x0 + r, y0, color);
    ILI9341_DrawPixel(x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        ILI9341_DrawPixel(x0 + x, y0 + y, color);
        ILI9341_DrawPixel(x0 - x, y0 + y, color);
        ILI9341_DrawPixel(x0 + x, y0 - y, color);
        ILI9341_DrawPixel(x0 - x, y0 - y, color);
        ILI9341_DrawPixel(x0 + y, y0 + x, color);
        ILI9341_DrawPixel(x0 - y, y0 + x, color);
        ILI9341_DrawPixel(x0 + y, y0 - x, color);
        ILI9341_DrawPixel(x0 - y, y0 - x, color);
    }
}

void ILI9341_DrawChar(uint16_t x, uint16_t y, char ch, uint16_t color, uint16_t bgcolor) {
    if (ch < 32 || ch > 126) ch = 32; // Space for unsupported characters

    const uint8_t *font_data = font5x7[ch - 32];

    for (uint8_t col = 0; col < 5; col++) {
        uint8_t line = font_data[col];
        for (uint8_t row = 0; row < 7; row++) {
            if (line & 0x01) {
                ILI9341_DrawPixel(x + col, y + row, color);
            } else if (bgcolor != color) {
                ILI9341_DrawPixel(x + col, y + row, bgcolor);
            }
            line >>= 1;
        }
    }
}

void ILI9341_DrawString(uint16_t x, uint16_t y, char* str, uint16_t color, uint16_t bgcolor) {
    uint16_t start_x = x;

    while (*str) {
        if (*str == '\n') {
            y += 8;
            x = start_x;
        } else {
            ILI9341_DrawChar(x, y, *str, color, bgcolor);
            x += 6;
        }
        str++;
    }
}
