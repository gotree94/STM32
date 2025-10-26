# Waveshare 1.3" OLED (SH1106/SSD1306 컨트롤러 기반)


<img width="247" height="215" alt="142" src="https://github.com/user-attachments/assets/3d1fcb2e-397c-47b4-8e6d-eb650978567f" />
<img width="355" height="287" alt="141" src="https://github.com/user-attachments/assets/a4e033d5-f69f-4450-a0c1-f8bab06d4dde" />
<br>

<img width="644" height="586" alt="F103RB-pin" src="https://github.com/user-attachments/assets/1d9ceb52-875a-458b-834b-cf41825aa8c2" />
<br>

<img width="1108" height="800" alt="SH1106_001" src="https://github.com/user-attachments/assets/4526503d-ba0c-4584-8d2a-9edeeafb28bb" />
<br>
<img width="1108" height="800" alt="SH1106_002" src="https://github.com/user-attachments/assets/99e4128f-5650-4538-ad65-d99ecdbcde88" />
<br>
<img width="1108" height="800" alt="SH1106_003" src="https://github.com/user-attachments/assets/0e523ec6-26b3-42d0-9349-215cb4ed3b7a" />
<br>
```c
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
/* USER CODE END Includes */
```

```c
/* OLED Control Macros -------------------------------------------------------*/
#define OLED_CS_LOW()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
#define OLED_CS_HIGH()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)
#define OLED_RST_LOW()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET)
#define OLED_RST_HIGH() HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET)
#define OLED_DC_LOW()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET)
#define OLED_DC_HIGH()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET)

/* OLED Parameters -----------------------------------------------------------*/
#define OLED_WIDTH  128
#define OLED_HEIGHT 64
#define OLED_PAGES  8   // 64/8 = 8 pages

/* Display Buffer */
static uint8_t oled_buffer[OLED_PAGES][OLED_WIDTH];
```

```c
/* USER CODE BEGIN 0 */
/* OLED Functions ------------------------------------------------------------*/

/**
  * @brief  Write command to OLED
  * @param  cmd: Command byte
  */
void OLED_WriteCmd(uint8_t cmd)
{
    OLED_DC_LOW();   // Command mode
    OLED_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    OLED_CS_HIGH();
}

/**
  * @brief  Write data to OLED
  * @param  data: Data byte
  */
void OLED_WriteData(uint8_t data)
{
    OLED_DC_HIGH();  // Data mode
    OLED_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
    OLED_CS_HIGH();
}

/**
  * @brief  Hardware reset OLED
  */
void OLED_Reset(void)
{
    OLED_RST_HIGH();
    HAL_Delay(10);
    OLED_RST_LOW();
    HAL_Delay(10);
    OLED_RST_HIGH();
    HAL_Delay(10);
}

/**
  * @brief  Initialize OLED display (SH1106)
  */
void OLED_Init(void)
{
    OLED_Reset();

    // Initialization sequence for SH1106
    OLED_WriteCmd(0xAE); // Display OFF
    OLED_WriteCmd(0x02); // Set lower column address
    OLED_WriteCmd(0x10); // Set higher column address
    OLED_WriteCmd(0x40); // Set display start line
    OLED_WriteCmd(0xB0); // Set page address

    OLED_WriteCmd(0x81); // Contract control
    OLED_WriteCmd(0xCF); // 128

    OLED_WriteCmd(0xA1); // Set segment remap (A0/A1)
    OLED_WriteCmd(0xA6); // Normal display (A6/A7)

    OLED_WriteCmd(0xA8); // Multiplex ratio
    OLED_WriteCmd(0x3F); // 1/64 duty

    OLED_WriteCmd(0xAD); // Set DC-DC
    OLED_WriteCmd(0x8B); // Enable DC-DC

    OLED_WriteCmd(0x30); // Set VPP 9V

    OLED_WriteCmd(0xC8); // Com scan direction (C0/C8)

    OLED_WriteCmd(0xD3); // Set display offset
    OLED_WriteCmd(0x00); // 0

    OLED_WriteCmd(0xD5); // Set clock divide
    OLED_WriteCmd(0x80); // 100Hz

    OLED_WriteCmd(0xD9); // Set pre-charge period
    OLED_WriteCmd(0x1F); //

    OLED_WriteCmd(0xDA); // Set COM pins
    OLED_WriteCmd(0x12); //

    OLED_WriteCmd(0xDB); // Set vcomh
    OLED_WriteCmd(0x40); //

    OLED_WriteCmd(0xAF); // Display ON
}

/**
  * @brief  Clear display buffer
  */
void OLED_Clear(void)
{
    memset(oled_buffer, 0x00, sizeof(oled_buffer));
}

/**
  * @brief  Fill display buffer
  */
void OLED_Fill(uint8_t pattern)
{
    memset(oled_buffer, pattern, sizeof(oled_buffer));
}

/**
  * @brief  Set pixel in buffer
  * @param  x: X coordinate (0-127)
  * @param  y: Y coordinate (0-63)
  * @param  color: 1=white, 0=black
  */
void OLED_SetPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;

    uint8_t page = y / 8;
    uint8_t bit = y % 8;

    if (color)
        oled_buffer[page][x] |= (1 << bit);
    else
        oled_buffer[page][x] &= ~(1 << bit);
}

/**
  * @brief  Draw a line
  */
void OLED_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        OLED_SetPixel(x0, y0, 1);

        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
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

/**
  * @brief  Draw rectangle
  */
void OLED_DrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    OLED_DrawLine(x, y, x + w - 1, y);
    OLED_DrawLine(x + w - 1, y, x + w - 1, y + h - 1);
    OLED_DrawLine(x + w - 1, y + h - 1, x, y + h - 1);
    OLED_DrawLine(x, y + h - 1, x, y);
}

/**
  * @brief  Update display from buffer
  */
void OLED_Display(void)
{
    for (uint8_t page = 0; page < OLED_PAGES; page++) {
        OLED_WriteCmd(0xB0 + page);  // Set page address
        OLED_WriteCmd(0x02);         // Set lower column (SH1106: starts at column 2)
        OLED_WriteCmd(0x10);         // Set higher column

        OLED_DC_HIGH();  // Data mode
        OLED_CS_LOW();
        HAL_SPI_Transmit(&hspi1, oled_buffer[page], OLED_WIDTH, HAL_MAX_DELAY);
        OLED_CS_HIGH();
    }
}

/**
  * @brief  Test patterns
  */
void OLED_TestPattern(void)
{
    // Test 1: Checkerboard
    OLED_Clear();
    for (int y = 0; y < OLED_HEIGHT; y++) {
        for (int x = 0; x < OLED_WIDTH; x++) {
            if ((x / 8 + y / 8) % 2 == 0)
                OLED_SetPixel(x, y, 1);
        }
    }
    OLED_Display();
    HAL_Delay(2000);

    // Test 2: Lines
    OLED_Clear();
    OLED_DrawLine(0, 0, 127, 63);
    OLED_DrawLine(127, 0, 0, 63);
    OLED_DrawLine(0, 31, 127, 31);
    OLED_DrawLine(63, 0, 63, 63);
    OLED_Display();
    HAL_Delay(2000);

    // Test 3: Rectangles
    OLED_Clear();
    OLED_DrawRect(0, 0, 128, 64);
    OLED_DrawRect(10, 10, 108, 44);
    OLED_DrawRect(20, 20, 88, 24);
    OLED_Display();
    HAL_Delay(2000);

    // Test 4: Animation - moving pixel
    for (int i = 0; i < 128; i++) {
        OLED_Clear();
        OLED_SetPixel(i, 31, 1);
        OLED_DrawRect(i - 2, 29, 5, 5);
        OLED_Display();
        HAL_Delay(20);
    }
}
/* USER CODE END 0 */
```

```c
  /* USER CODE BEGIN 2 */
  /* Initialize OLED */
  OLED_Init();
  HAL_Delay(100);
  /* USER CODE END 2 */
```

```c
    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */    /* USER CODE BEGIN 3 */
	  OLED_TestPattern();
  }
  /* USER CODE END 3 */
```




