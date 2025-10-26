# GM009605 LCD 

<img width="417" height="394" alt="131" src="https://github.com/user-attachments/assets/6d05b431-3f45-44a3-b1b3-e4dd7362c97f" />
<br>

<img width="644" height="586" alt="F103RB-pin" src="https://github.com/user-attachments/assets/a54e4c3e-b7c0-4c67-ba45-32d441bbbdcb" />
<br>

| NUCLEO-F103RB |  GM009605 LCD | 
|-----:|:-------------|
| PB8 | SCL | 
| PB9 | SDA | 
| 3.3V | VCC | 
| GND | GND | 

![001](https://github.com/user-attachments/assets/ec58574f-4af1-436c-ae7c-b817fd7d347f)

![002](https://github.com/user-attachments/assets/db0940ba-6be6-4544-b788-84fa8ee87a82)

![003](https://github.com/user-attachments/assets/14a8e3e0-f90c-4065-a51a-fc6476d07a9b)


```c
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */
```


```c
/* USER CODE BEGIN PD */
/* GM009605 LCD I2C 주소 (7비트 주소) */
#define GM009605_I2C_ADDR 0x78  // 일반적으로 0x3C 또는 0x3D

/* LCD 해상도 */
#define LCD_WIDTH  128
#define LCD_HEIGHT 64

/* 명령/데이터 제어 바이트 */
#define LCD_CMD_MODE  0x00
#define LCD_DATA_MODE 0x40

/* SSD1306 명령어 (GM009605가 SSD1306 호환 컨트롤러 사용) */
#define SSD1306_DISPLAYOFF          0xAE
#define SSD1306_SETDISPLAYCLOCKDIV  0xD5
#define SSD1306_SETMULTIPLEX        0xA8
#define SSD1306_SETDISPLAYOFFSET    0xD3
#define SSD1306_SETSTARTLINE        0x40
#define SSD1306_CHARGEPUMP          0x8D
#define SSD1306_MEMORYMODE          0x20
#define SSD1306_SEGREMAP            0xA1
#define SSD1306_COMSCANDEC          0xC8
#define SSD1306_SETCOMPINS          0xDA
#define SSD1306_SETCONTRAST         0x81
#define SSD1306_SETPRECHARGE        0xD9
#define SSD1306_SETVCOMDETECT       0xDB
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_NORMALDISPLAY       0xA6
#define SSD1306_INVERSEDISPLAY      0xA7
#define SSD1306_DISPLAYON           0xAF
#define SSD1306_COLUMNADDR          0x21
#define SSD1306_PAGEADDR            0x22
/* USER CODE END PD */
```

```c
/* USER CODE BEGIN PM */
/* 프레임버퍼 */
static uint8_t lcd_buffer[LCD_WIDTH * LCD_HEIGHT / 8];

/* 5x7 ASCII 폰트 (간단한 폰트) */
/* 5x7 ASCII 폰트 (ASCII 32-122: 스페이스 ~ z) */
const uint8_t font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // 32: 스페이스
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // 33: !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // 34: "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // 35: #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // 36: $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // 37: %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // 38: &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // 39: '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // 40: (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // 41: )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // 42: *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // 43: +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // 44: ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // 45: -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // 46: .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // 47: /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 48: 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 49: 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 50: 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 51: 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 52: 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 53: 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 54: 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 55: 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 56: 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 57: 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // 58: :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // 59: ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // 60: <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // 61: =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // 62: >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // 63: ?
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // 64: @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // 65: A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // 66: B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // 67: C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // 68: D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // 69: E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // 70: F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // 71: G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // 72: H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // 73: I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // 74: J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // 75: K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // 76: L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // 77: M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // 78: N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // 79: O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // 80: P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // 81: Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // 82: R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // 83: S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // 84: T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // 85: U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // 86: V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // 87: W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // 88: X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // 89: Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // 90: Z
    {0x00, 0x7F, 0x41, 0x41, 0x00}, // 91: [
    {0x02, 0x04, 0x08, 0x10, 0x20}, // 92: backslash
    {0x00, 0x41, 0x41, 0x7F, 0x00}, // 93: ]
    {0x04, 0x02, 0x01, 0x02, 0x04}, // 94: ^
    {0x40, 0x40, 0x40, 0x40, 0x40}, // 95: _
    {0x00, 0x01, 0x02, 0x04, 0x00}, // 96: `
    {0x20, 0x54, 0x54, 0x54, 0x78}, // 97: a
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // 98: b
    {0x38, 0x44, 0x44, 0x44, 0x20}, // 99: c
    {0x38, 0x44, 0x44, 0x48, 0x7F}, // 100: d
    {0x38, 0x54, 0x54, 0x54, 0x18}, // 101: e
    {0x08, 0x7E, 0x09, 0x01, 0x02}, // 102: f
    {0x0C, 0x52, 0x52, 0x52, 0x3E}, // 103: g
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // 104: h
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // 105: i
    {0x20, 0x40, 0x44, 0x3D, 0x00}, // 106: j
    {0x7F, 0x10, 0x28, 0x44, 0x00}, // 107: k
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // 108: l
    {0x7C, 0x04, 0x18, 0x04, 0x78}, // 109: m
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // 110: n
    {0x38, 0x44, 0x44, 0x44, 0x38}, // 111: o
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // 112: p
    {0x08, 0x14, 0x14, 0x18, 0x7C}, // 113: q
    {0x7C, 0x08, 0x04, 0x04, 0x08}, // 114: r
    {0x48, 0x54, 0x54, 0x54, 0x20}, // 115: s
    {0x04, 0x3F, 0x44, 0x40, 0x20}, // 116: t
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // 117: u
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // 118: v
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // 119: w
    {0x44, 0x28, 0x10, 0x28, 0x44}, // 120: x
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, // 121: y
    {0x44, 0x64, 0x54, 0x4C, 0x44}, // 122: z
};
/* USER CODE END PM */
```

```c
/* USER CODE BEGIN PFP */
/* 함수 프로토타입 */
void LCD_WriteCommand(uint8_t cmd);
void LCD_WriteData(uint8_t *data, uint16_t len);
void LCD_Init(void);
void LCD_Clear(void);
void LCD_Display(void);
void LCD_SetPixel(uint8_t x, uint8_t y, uint8_t color);
void LCD_DrawChar(uint8_t x, uint8_t y, char ch);
void LCD_DrawString(uint8_t x, uint8_t y, const char *str);
void LCD_Fill(uint8_t color);
/* USER CODE END PFP */
```

```c
/* USER CODE BEGIN 0 */
/* LCD 명령 전송 */
void LCD_WriteCommand(uint8_t cmd)
{
    uint8_t data[2] = {LCD_CMD_MODE, cmd};
    HAL_I2C_Master_Transmit(&hi2c1, GM009605_I2C_ADDR, data, 2, 100);
}

/* LCD 데이터 전송 */
void LCD_WriteData(uint8_t *data, uint16_t len)
{
    uint8_t buffer[len + 1];
    buffer[0] = LCD_DATA_MODE;
    memcpy(buffer + 1, data, len);
    HAL_I2C_Master_Transmit(&hi2c1, GM009605_I2C_ADDR, buffer, len + 1, 1000);
}

/* LCD 초기화 */
void LCD_Init(void)
{
    HAL_Delay(100);

    LCD_WriteCommand(SSD1306_DISPLAYOFF);
    LCD_WriteCommand(SSD1306_SETDISPLAYCLOCKDIV);
    LCD_WriteCommand(0x80);
    LCD_WriteCommand(SSD1306_SETMULTIPLEX);
    LCD_WriteCommand(LCD_HEIGHT - 1);
    LCD_WriteCommand(SSD1306_SETDISPLAYOFFSET);
    LCD_WriteCommand(0x00);
    LCD_WriteCommand(SSD1306_SETSTARTLINE | 0x00);
    LCD_WriteCommand(SSD1306_CHARGEPUMP);
    LCD_WriteCommand(0x14);
    LCD_WriteCommand(SSD1306_MEMORYMODE);
    LCD_WriteCommand(0x00);
    LCD_WriteCommand(SSD1306_SEGREMAP | 0x01);
    LCD_WriteCommand(SSD1306_COMSCANDEC);
    LCD_WriteCommand(SSD1306_SETCOMPINS);
    LCD_WriteCommand(0x12);
    LCD_WriteCommand(SSD1306_SETCONTRAST);
    LCD_WriteCommand(0xCF);
    LCD_WriteCommand(SSD1306_SETPRECHARGE);
    LCD_WriteCommand(0xF1);
    LCD_WriteCommand(SSD1306_SETVCOMDETECT);
    LCD_WriteCommand(0x40);
    LCD_WriteCommand(SSD1306_DISPLAYALLON_RESUME);
    LCD_WriteCommand(SSD1306_NORMALDISPLAY);
    LCD_WriteCommand(SSD1306_DISPLAYON);

    LCD_Clear();
}

/* 화면 지우기 */
void LCD_Clear(void)
{
    memset(lcd_buffer, 0, sizeof(lcd_buffer));
}

/* 프레임버퍼를 LCD에 표시 */
void LCD_Display(void)
{
    LCD_WriteCommand(SSD1306_COLUMNADDR);
    LCD_WriteCommand(0);
    LCD_WriteCommand(LCD_WIDTH - 1);
    LCD_WriteCommand(SSD1306_PAGEADDR);
    LCD_WriteCommand(0);
    LCD_WriteCommand((LCD_HEIGHT / 8) - 1);

    LCD_WriteData(lcd_buffer, sizeof(lcd_buffer));
}

/* 픽셀 설정 */
void LCD_SetPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;

    if (color) {
        lcd_buffer[x + (y / 8) * LCD_WIDTH] |= (1 << (y % 8));
    } else {
        lcd_buffer[x + (y / 8) * LCD_WIDTH] &= ~(1 << (y % 8));
    }
}

/* 문자 그리기 */
void LCD_DrawChar(uint8_t x, uint8_t y, char ch)
{
    // ASCII 32-122 범위 체크 (스페이스 ~ z)
    if (ch < 32 || ch > 122) ch = 32;

    const uint8_t *glyph = font5x7[ch - 32];

    // 문자 배경 먼저 지우기 (6x8 영역)
    for (uint8_t i = 0; i < 6; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            LCD_SetPixel(x + i, y * 8 + j, 0);
        }
    }

    // 문자 그리기
    for (uint8_t i = 0; i < 5; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            if (glyph[i] & (1 << j)) {
                LCD_SetPixel(x + i, y * 8 + j, 1);
            }
        }
    }
}

/* 문자열 그리기 */
void LCD_DrawString(uint8_t x, uint8_t y, const char *str)
{
    while (*str) {
        LCD_DrawChar(x, y, *str);
        x += 6;
        if (x > LCD_WIDTH - 6) break;
        str++;
    }
}

/* 화면 채우기 */
void LCD_Fill(uint8_t color)
{
    memset(lcd_buffer, color ? 0xFF : 0x00, sizeof(lcd_buffer));
}
/* USER CODE END 0 */
```

```c
  /* LCD 초기화 */
  LCD_Init();
  HAL_Delay(100);
  /* USER CODE END 2 */
```

```c
    /* USER CODE BEGIN 3 */
      /* 1. 검은색 배경 + 흰색 글씨 (정상 모드) */
      LCD_WriteCommand(SSD1306_NORMALDISPLAY);
      LCD_Fill(0);
      LCD_DrawString(0, 0, "Hello World!");
      LCD_DrawString(0, 1, "Line 2: Black BG");
      LCD_DrawString(0, 2, "NUCLEO-F103RB");
      LCD_DrawString(0, 3, "64MHz Clock");
      LCD_DrawString(0, 4, "I2C GM009605");
      LCD_DrawString(0, 5, "HAL Driver");
      LCD_DrawString(0, 6, "Test Display");
      LCD_DrawString(0, 7, "================");
      LCD_Display();
      HAL_Delay(3000);

      /* 2. 흰색 배경 + 검은색 글씨 (반전 모드) */
      LCD_WriteCommand(SSD1306_INVERSEDISPLAY);
      LCD_Fill(0);
      LCD_DrawString(0, 0, "Hello World!");
      LCD_DrawString(0, 1, "Line 2: White BG");
      LCD_DrawString(0, 2, "NUCLEO-F103RB");
      LCD_DrawString(0, 3, "64MHz Clock");
      LCD_DrawString(0, 4, "I2C GM009605");
      LCD_DrawString(0, 5, "HAL Driver");
      LCD_DrawString(0, 6, "Test Display");
      LCD_DrawString(0, 7, "================");
      LCD_Display();
      HAL_Delay(3000);

      /* 3. 체크 패턴 (8x8 블록) */
      LCD_WriteCommand(SSD1306_NORMALDISPLAY);
      LCD_Fill(0);
      for (uint8_t y = 0; y < LCD_HEIGHT; y++) {
          for (uint8_t x = 0; x < LCD_WIDTH; x++) {
              // 8x8 체크 패턴
              if (((x / 8) + (y / 8)) % 2 == 0) {
                  LCD_SetPixel(x, y, 1);
              }
          }
      }
      LCD_Display();
      HAL_Delay(2000);
  }
  /* USER CODE END 3 */
```

