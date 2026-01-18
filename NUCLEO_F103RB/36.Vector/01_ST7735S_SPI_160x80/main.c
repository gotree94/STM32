/* ============================================================================
 * Vector Robot Eye Animation - ST7735S SPI Version (0.96" 160x80 LCD)
 * ============================================================================
 * 
 * 하드웨어 연결:
 *   - SPI1: PA5(SCK), PA7(MOSI)
 *   - GPIO: PA1(RES), PA6(DC), PB6(CS)
 *   - 전원: 3.3V, GND
 * 
 * CubeMX 설정:
 *   - SPI1: Mode=Transmit Only Master, Prescaler=4 (16MHz @ 64MHz clock)
 *   - GPIO Output: PA1, PA6, PB6
 *   - System Clock: 64MHz
 * 
 * ============================================================================
 */

#include "main.h"
#include <string.h>
#include <stdlib.h>

SPI_HandleTypeDef hspi1;

// ============================================================================
// ST7735S LCD 설정
// ============================================================================

#define LCD_WIDTH  160
#define LCD_HEIGHT 80

// ST7735 Commands
#define ST7735_SWRESET 0x01
#define ST7735_SLPOUT  0x11
#define ST7735_NORON   0x13
#define ST7735_INVOFF  0x20
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36
#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5
#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

// Colors
#define BLACK       0x0000
#define EYE_COLOR   0x07E0   // Green
#define EYE_BRIGHT  0xAFE5
#define EYE_DIM     0x0320

// 눈 영역 정의
#define EYE_AREA_X      10
#define EYE_AREA_Y      20      // 위치 조정 가능
#define EYE_AREA_W      140
#define EYE_AREA_H      50

// 눈 위치 (버퍼 내 좌표)
#define LX              40
#define RX              100
#define CY              25

// 눈 크기
#define EYE_W           32
#define EYE_H           40
#define EYE_R           10

// 프레임버퍼
static uint8_t eye_buffer[EYE_AREA_W * EYE_AREA_H * 2];

// GPIO 매크로
#define LCD_CS_LOW()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET)
#define LCD_CS_HIGH()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET)
#define LCD_DC_LOW()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET)
#define LCD_DC_HIGH()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET)
#define LCD_RES_LOW()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET)
#define LCD_RES_HIGH() HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET)

// Expression 타입
typedef enum {
    EXPR_NORMAL, EXPR_HAPPY, EXPR_SAD, EXPR_ANGRY,
    EXPR_SURPRISED, EXPR_SLEEPY, EXPR_WINK_LEFT, EXPR_WINK_RIGHT,
    EXPR_BLINK, EXPR_LOVE, EXPR_DIZZY,
    EXPR_LOOK_LEFT, EXPR_LOOK_RIGHT, EXPR_LOOK_UP, EXPR_LOOK_DOWN
} Expression_t;

static Expression_t current_expr = EXPR_NORMAL;
static uint32_t last_blink = 0;
static uint32_t last_action = 0;

// ============================================================================
// LCD 기본 함수
// ============================================================================

static void LCD_Cmd(uint8_t cmd) {
    LCD_DC_LOW();
    LCD_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    LCD_CS_HIGH();
}

static void LCD_Data(uint8_t data) {
    LCD_DC_HIGH();
    LCD_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
    LCD_CS_HIGH();
}

static void LCD_SetWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    LCD_Cmd(ST7735_CASET);
    LCD_Data(0); LCD_Data(x0);
    LCD_Data(0); LCD_Data(x1);
    LCD_Cmd(ST7735_RASET);
    LCD_Data(0); LCD_Data(y0);
    LCD_Data(0); LCD_Data(y1);
    LCD_Cmd(ST7735_RAMWR);
}

static void LCD_SendBuffer(uint8_t* buf, uint32_t len) {
    LCD_DC_HIGH();
    LCD_CS_LOW();
    HAL_SPI_Transmit(&hspi1, buf, len, HAL_MAX_DELAY);
    LCD_CS_HIGH();
}

static void LCD_Init(void) {
    LCD_RES_LOW(); HAL_Delay(50);
    LCD_RES_HIGH(); HAL_Delay(50);
    
    LCD_Cmd(ST7735_SWRESET); HAL_Delay(120);
    LCD_Cmd(ST7735_SLPOUT); HAL_Delay(120);
    
    LCD_Cmd(ST7735_FRMCTR1);
    LCD_Data(0x01); LCD_Data(0x2C); LCD_Data(0x2D);
    LCD_Cmd(ST7735_FRMCTR2);
    LCD_Data(0x01); LCD_Data(0x2C); LCD_Data(0x2D);
    LCD_Cmd(ST7735_FRMCTR3);
    LCD_Data(0x01); LCD_Data(0x2C); LCD_Data(0x2D);
    LCD_Data(0x01); LCD_Data(0x2C); LCD_Data(0x2D);
    
    LCD_Cmd(ST7735_INVCTR); LCD_Data(0x07);
    LCD_Cmd(ST7735_PWCTR1); LCD_Data(0xA2); LCD_Data(0x02); LCD_Data(0x84);
    LCD_Cmd(ST7735_PWCTR2); LCD_Data(0xC5);
    LCD_Cmd(ST7735_PWCTR3); LCD_Data(0x0A); LCD_Data(0x00);
    LCD_Cmd(ST7735_PWCTR4); LCD_Data(0x8A); LCD_Data(0x2A);
    LCD_Cmd(ST7735_PWCTR5); LCD_Data(0x8A); LCD_Data(0xEE);
    LCD_Cmd(ST7735_VMCTR1); LCD_Data(0x0E);
    
    LCD_Cmd(ST7735_INVOFF);
    LCD_Cmd(ST7735_MADCTL); LCD_Data(0x60);
    LCD_Cmd(ST7735_COLMOD); LCD_Data(0x05);
    
    LCD_Cmd(ST7735_CASET);
    LCD_Data(0); LCD_Data(0); LCD_Data(0); LCD_Data(159);
    LCD_Cmd(ST7735_RASET);
    LCD_Data(0); LCD_Data(0); LCD_Data(0); LCD_Data(79);
    
    LCD_Cmd(ST7735_GMCTRP1);
    uint8_t gp[] = {0x0f,0x1a,0x0f,0x18,0x2f,0x28,0x20,0x22,0x1f,0x1b,0x23,0x37,0x00,0x07,0x02,0x10};
    for(int i=0;i<16;i++) LCD_Data(gp[i]);
    LCD_Cmd(ST7735_GMCTRN1);
    uint8_t gn[] = {0x0f,0x1b,0x0f,0x17,0x33,0x2c,0x29,0x2e,0x30,0x30,0x39,0x3f,0x00,0x07,0x03,0x10};
    for(int i=0;i<16;i++) LCD_Data(gn[i]);
    
    LCD_Cmd(ST7735_NORON); HAL_Delay(10);
    LCD_Cmd(ST7735_DISPON); HAL_Delay(50);
}

static void LCD_ClearScreen(void) {
    LCD_SetWindow(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    LCD_DC_HIGH();
    LCD_CS_LOW();
    uint8_t black[2] = {0, 0};
    for(uint32_t i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        HAL_SPI_Transmit(&hspi1, black, 2, HAL_MAX_DELAY);
    }
    LCD_CS_HIGH();
}

// ============================================================================
// 버퍼 그리기 함수
// ============================================================================

static void Buf_Clear(void) {
    memset(eye_buffer, 0, sizeof(eye_buffer));
}

static inline void Buf_SetPixel(int16_t x, int16_t y, uint16_t color) {
    if(x < 0 || x >= EYE_AREA_W || y < 0 || y >= EYE_AREA_H) return;
    uint32_t idx = (y * EYE_AREA_W + x) * 2;
    eye_buffer[idx] = color >> 8;
    eye_buffer[idx + 1] = color & 0xFF;
}

static void Buf_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;
    for(int16_t j = y; j < y + h; j++) {
        if(j < 0 || j >= EYE_AREA_H) continue;
        for(int16_t i = x; i < x + w; i++) {
            if(i < 0 || i >= EYE_AREA_W) continue;
            uint32_t idx = (j * EYE_AREA_W + i) * 2;
            eye_buffer[idx] = hi;
            eye_buffer[idx + 1] = lo;
        }
    }
}

static void Buf_FillCircle(int16_t cx, int16_t cy, int16_t r, uint16_t color) {
    int16_t x = r, y = 0;
    int16_t err = 1 - r;
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;
    
    while(x >= y) {
        for(int16_t i = cx - x; i <= cx + x; i++) {
            if(i >= 0 && i < EYE_AREA_W) {
                if(cy + y >= 0 && cy + y < EYE_AREA_H) {
                    uint32_t idx = ((cy + y) * EYE_AREA_W + i) * 2;
                    eye_buffer[idx] = hi; eye_buffer[idx + 1] = lo;
                }
                if(cy - y >= 0 && cy - y < EYE_AREA_H) {
                    uint32_t idx = ((cy - y) * EYE_AREA_W + i) * 2;
                    eye_buffer[idx] = hi; eye_buffer[idx + 1] = lo;
                }
            }
        }
        for(int16_t i = cx - y; i <= cx + y; i++) {
            if(i >= 0 && i < EYE_AREA_W) {
                if(cy + x >= 0 && cy + x < EYE_AREA_H) {
                    uint32_t idx = ((cy + x) * EYE_AREA_W + i) * 2;
                    eye_buffer[idx] = hi; eye_buffer[idx + 1] = lo;
                }
                if(cy - x >= 0 && cy - x < EYE_AREA_H) {
                    uint32_t idx = ((cy - x) * EYE_AREA_W + i) * 2;
                    eye_buffer[idx] = hi; eye_buffer[idx + 1] = lo;
                }
            }
        }
        y++;
        if(err < 0) err += 2 * y + 1;
        else { x--; err += 2 * (y - x + 1); }
    }
}

static void Buf_RoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    Buf_FillRect(x + r, y, w - 2*r, h, color);
    Buf_FillRect(x, y + r, r, h - 2*r, color);
    Buf_FillRect(x + w - r, y + r, r, h - 2*r, color);
    Buf_FillCircle(x + r, y + r, r, color);
    Buf_FillCircle(x + w - r - 1, y + r, r, color);
    Buf_FillCircle(x + r, y + h - r - 1, r, color);
    Buf_FillCircle(x + w - r - 1, y + h - r - 1, r, color);
}

static void Buf_HLine(int16_t x, int16_t y, int16_t w, int16_t thick, uint16_t color) {
    Buf_FillRect(x, y - thick/2, w, thick, color);
}

static void Buf_ThickLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t t, uint16_t color) {
    int16_t dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int16_t dy = (y1 > y0) ? (y1 - y0) : (y0 - y1);
    
    if(dy <= 2) {
        int16_t minX = (x0 < x1) ? x0 : x1;
        int16_t maxX = (x0 > x1) ? x0 : x1;
        Buf_FillRect(minX, (y0 + y1)/2 - t/2, maxX - minX + 1, t, color);
        return;
    }
    
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;
    
    while(1) {
        Buf_FillRect(x0 - t/2, y0 - t/2, t, t, color);
        if(x0 == x1 && y0 == y1) break;
        int16_t e2 = 2 * err;
        if(e2 > -dy) { err -= dy; x0 += sx; }
        if(e2 < dx) { err += dx; y0 += sy; }
    }
}

static void Buf_Flush(void) {
    LCD_SetWindow(EYE_AREA_X, EYE_AREA_Y, 
                  EYE_AREA_X + EYE_AREA_W - 1, 
                  EYE_AREA_Y + EYE_AREA_H - 1);
    LCD_SendBuffer(eye_buffer, sizeof(eye_buffer));
}

// ============================================================================
// 눈 그리기 함수
// ============================================================================

static void Eye_Normal(int16_t cx, int16_t ox, int16_t oy) {
    Buf_RoundRect(cx - EYE_W/2, CY - EYE_H/2, EYE_W, EYE_H, EYE_R, EYE_COLOR);
    Buf_FillCircle(cx - 3 + ox, CY - 5 + oy, 3, EYE_BRIGHT);
}

static void Eye_Closed(int16_t cx) {
    Buf_HLine(cx - EYE_W/2 + 3, CY, EYE_W - 6, 5, EYE_COLOR);
}

static void Eye_Half(int16_t cx, uint8_t pct) {
    int16_t h = (EYE_H * pct) / 100;
    if(h < 8) { Eye_Closed(cx); return; }
    int16_t top = CY + EYE_H/2 - h;
    Buf_RoundRect(cx - EYE_W/2, top, EYE_W, h, EYE_R/2, EYE_COLOR);
}

static void Eye_Happy(int16_t cx) {
    for(int16_t i = -EYE_W/2 + 2; i <= EYE_W/2 - 2; i++) {
        int32_t n = (int32_t)i * i * 100 / ((EYE_W/2) * (EYE_W/2));
        int16_t y = CY + 3 - (8 * (100 - n) / 100);
        Buf_FillRect(cx + i, y - 3, 1, 4, EYE_COLOR);
    }
}

static void Eye_Sad(int16_t cx) {
    Buf_RoundRect(cx - EYE_W/2, CY - EYE_H/2 + 5, EYE_W, EYE_H - 5, EYE_R, EYE_COLOR);
    Buf_ThickLine(cx - EYE_W/2, CY - EYE_H/2 - 2, cx + EYE_W/2, CY - EYE_H/2 + 6, 3, EYE_COLOR);
}

static void Eye_Angry(int16_t cx, uint8_t is_left) {
    Buf_RoundRect(cx - EYE_W/2, CY - EYE_H/2 + 6, EYE_W, EYE_H - 10, EYE_R - 2, EYE_COLOR);
    if(is_left)
        Buf_ThickLine(cx - EYE_W/2 - 2, CY - EYE_H/2 + 4, cx + EYE_W/2 + 2, CY - EYE_H/2 - 4, 4, EYE_COLOR);
    else
        Buf_ThickLine(cx - EYE_W/2 - 2, CY - EYE_H/2 - 4, cx + EYE_W/2 + 2, CY - EYE_H/2 + 4, 4, EYE_COLOR);
}

static void Eye_Surprised(int16_t cx) {
    Buf_FillCircle(cx, CY, EYE_H/2, EYE_COLOR);
    Buf_FillCircle(cx, CY, EYE_H/2 - 6, EYE_DIM);
    Buf_FillCircle(cx - 3, CY - 4, 4, EYE_BRIGHT);
}

static void Eye_Heart(int16_t cx) {
    int16_t s = 10;
    Buf_FillCircle(cx - s/2, CY - s/3, s/2, EYE_COLOR);
    Buf_FillCircle(cx + s/2, CY - s/3, s/2, EYE_COLOR);
    for(int16_t r = 0; r < s; r++) {
        Buf_FillRect(cx - (s - r), CY - s/3 + r, (s - r) * 2 + 1, 1, EYE_COLOR);
    }
}

static void Eye_X(int16_t cx) {
    int16_t s = EYE_H/2 - 5;
    Buf_ThickLine(cx - s, CY - s, cx + s, CY + s, 4, EYE_COLOR);
    Buf_ThickLine(cx + s, CY - s, cx - s, CY + s, 4, EYE_COLOR);
}

// ============================================================================
// 표정 설정
// ============================================================================

static void Draw_Expression(Expression_t expr, int16_t ox, int16_t oy) {
    Buf_Clear();
    
    switch(expr) {
        case EXPR_NORMAL:
            Eye_Normal(LX, ox, oy);
            Eye_Normal(RX, ox, oy);
            break;
        case EXPR_HAPPY:
            Eye_Happy(LX);
            Eye_Happy(RX);
            break;
        case EXPR_SAD:
            Eye_Sad(LX);
            Eye_Sad(RX);
            break;
        case EXPR_ANGRY:
            Eye_Angry(LX, 1);
            Eye_Angry(RX, 0);
            break;
        case EXPR_SURPRISED:
            Eye_Surprised(LX);
            Eye_Surprised(RX);
            break;
        case EXPR_SLEEPY:
            Eye_Half(LX, 30);
            Eye_Half(RX, 30);
            break;
        case EXPR_WINK_LEFT:
            Eye_Closed(LX);
            Eye_Normal(RX, 0, 0);
            break;
        case EXPR_WINK_RIGHT:
            Eye_Normal(LX, 0, 0);
            Eye_Closed(RX);
            break;
        case EXPR_BLINK:
            Eye_Closed(LX);
            Eye_Closed(RX);
            break;
        case EXPR_LOVE:
            Eye_Heart(LX);
            Eye_Heart(RX);
            break;
        case EXPR_DIZZY:
            Eye_X(LX);
            Eye_X(RX);
            break;
        case EXPR_LOOK_LEFT:
            Eye_Normal(LX, -5, 0);
            Eye_Normal(RX, -5, 0);
            break;
        case EXPR_LOOK_RIGHT:
            Eye_Normal(LX, 5, 0);
            Eye_Normal(RX, 5, 0);
            break;
        case EXPR_LOOK_UP:
            Eye_Normal(LX, 0, -5);
            Eye_Normal(RX, 0, -5);
            break;
        case EXPR_LOOK_DOWN:
            Eye_Normal(LX, 0, 5);
            Eye_Normal(RX, 0, 5);
            break;
    }
    
    Buf_Flush();
}

static void Anim_SetExpr(Expression_t expr) {
    current_expr = expr;
    Draw_Expression(expr, 0, 0);
}

// ============================================================================
// 애니메이션
// ============================================================================

static void Anim_Blink(void) {
    Buf_Clear();
    Eye_Half(LX, 50);
    Eye_Half(RX, 50);
    Buf_Flush();
    
    Buf_Clear();
    Eye_Closed(LX);
    Eye_Closed(RX);
    Buf_Flush();
    HAL_Delay(40);
    
    Buf_Clear();
    Eye_Half(LX, 50);
    Eye_Half(RX, 50);
    Buf_Flush();
    
    Draw_Expression(current_expr, 0, 0);
}

static void Anim_WinkL(void) {
    Buf_Clear();
    Eye_Closed(LX);
    Eye_Normal(RX, 0, 0);
    Buf_Flush();
    HAL_Delay(180);
    Draw_Expression(EXPR_NORMAL, 0, 0);
}

static void Anim_WinkR(void) {
    Buf_Clear();
    Eye_Normal(LX, 0, 0);
    Eye_Closed(RX);
    Buf_Flush();
    HAL_Delay(180);
    Draw_Expression(EXPR_NORMAL, 0, 0);
}

static void Anim_LookAround(void) {
    Anim_SetExpr(EXPR_LOOK_LEFT);
    HAL_Delay(250);
    Anim_SetExpr(EXPR_NORMAL);
    HAL_Delay(100);
    Anim_SetExpr(EXPR_LOOK_RIGHT);
    HAL_Delay(250);
    Anim_SetExpr(EXPR_NORMAL);
}

static void Anim_Idle(void) {
    uint32_t t = HAL_GetTick();
    
    if(t - last_blink > 2500 + (rand() % 2000)) {
        Anim_Blink();
        last_blink = t;
    }
    
    if(t - last_action > 6000 + (rand() % 4000)) {
        switch(rand() % 5) {
            case 0: Anim_SetExpr(EXPR_LOOK_LEFT); HAL_Delay(300); break;
            case 1: Anim_SetExpr(EXPR_LOOK_RIGHT); HAL_Delay(300); break;
            case 2: Anim_WinkL(); break;
            case 3: Anim_WinkR(); break;
            case 4: Anim_LookAround(); break;
        }
        Anim_SetExpr(EXPR_NORMAL);
        last_action = t;
    }
}

static void Anim_Demo(void) {
    Anim_SetExpr(EXPR_NORMAL);    HAL_Delay(1000);
    Anim_Blink();                  HAL_Delay(500);
    Anim_SetExpr(EXPR_HAPPY);     HAL_Delay(1000);
    Anim_SetExpr(EXPR_SAD);       HAL_Delay(1000);
    Anim_SetExpr(EXPR_ANGRY);     HAL_Delay(1000);
    Anim_SetExpr(EXPR_SURPRISED); HAL_Delay(1000);
    Anim_WinkL();                  HAL_Delay(400);
    Anim_WinkR();                  HAL_Delay(400);
    Anim_SetExpr(EXPR_LOVE);      HAL_Delay(1000);
    Anim_SetExpr(EXPR_SLEEPY);    HAL_Delay(1000);
    Anim_SetExpr(EXPR_DIZZY);     HAL_Delay(1000);
    Anim_LookAround();             HAL_Delay(500);
}

// ============================================================================
// Main
// ============================================================================

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_SPI1_Init();
    
    LCD_Init();
    LCD_ClearScreen();
    
    srand(HAL_GetTick());
    
    Anim_SetExpr(EXPR_NORMAL);
    HAL_Delay(500);
    
    while(1) {
        // 데모 모드
        Anim_Demo();
        
        // 또는 Idle 모드
        // Anim_Idle();
        // HAL_Delay(30);
    }
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL8;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

static void MX_SPI1_Init(void) {
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;
    if (HAL_SPI_Init(&hspi1) != HAL_OK) {
        Error_Handler();
    }
}

static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1|GPIO_PIN_6, GPIO_PIN_SET);
    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void Error_Handler(void) {
    __disable_irq();
    while(1) {}
}
