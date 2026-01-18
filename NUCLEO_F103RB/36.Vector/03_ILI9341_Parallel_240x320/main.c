/* ============================================================================
 * STM32F103 + ILI9341 LCD (240x320) - Vector Robot Eye Animation
 * ★★★ 초고속 버전 - GPIO 레지스터 직접 접근 ★★★
 * ============================================================================
 */

#include "main.h"
#include <string.h>
#include <stdlib.h>

// ============================================================================
// ★ 초고속 GPIO 매크로 (레지스터 직접 접근) ★
// ============================================================================

// Control pins (BSRR: set, BRR: reset)
#define LCD_CS_LOW()    GPIOB->BRR = GPIO_PIN_0
#define LCD_CS_HIGH()   GPIOB->BSRR = GPIO_PIN_0
#define LCD_RS_LOW()    GPIOA->BRR = GPIO_PIN_4      // Command
#define LCD_RS_HIGH()   GPIOA->BSRR = GPIO_PIN_4     // Data
#define LCD_WR_LOW()    GPIOA->BRR = GPIO_PIN_1
#define LCD_WR_HIGH()   GPIOA->BSRR = GPIO_PIN_1
#define LCD_RD_HIGH()   GPIOA->BSRR = GPIO_PIN_0
#define LCD_RST_LOW()   GPIOC->BRR = GPIO_PIN_1
#define LCD_RST_HIGH()  GPIOC->BSRR = GPIO_PIN_1

// ★ 8-bit 데이터 고속 출력 ★
// D0=PA9, D1=PC7, D2=PA10, D3=PB3, D4=PB5, D5=PB4, D6=PB10, D7=PA8
static inline void LCD_Write8Fast(uint8_t data) {
    // GPIOA: D0(9), D2(10), D7(8)
    uint32_t pa_set = 0, pa_clr = 0;
    if(data & 0x01) pa_set |= GPIO_PIN_9;  else pa_clr |= GPIO_PIN_9;   // D0
    if(data & 0x04) pa_set |= GPIO_PIN_10; else pa_clr |= GPIO_PIN_10;  // D2
    if(data & 0x80) pa_set |= GPIO_PIN_8;  else pa_clr |= GPIO_PIN_8;   // D7
    
    // GPIOB: D3(3), D4(5), D5(4), D6(10)
    uint32_t pb_set = 0, pb_clr = 0;
    if(data & 0x08) pb_set |= GPIO_PIN_3;  else pb_clr |= GPIO_PIN_3;   // D3
    if(data & 0x10) pb_set |= GPIO_PIN_5;  else pb_clr |= GPIO_PIN_5;   // D4
    if(data & 0x20) pb_set |= GPIO_PIN_4;  else pb_clr |= GPIO_PIN_4;   // D5
    if(data & 0x40) pb_set |= GPIO_PIN_10; else pb_clr |= GPIO_PIN_10;  // D6
    
    // GPIOC: D1(7)
    uint32_t pc_set = 0, pc_clr = 0;
    if(data & 0x02) pc_set |= GPIO_PIN_7;  else pc_clr |= GPIO_PIN_7;   // D1
    
    // 동시에 설정 (BSRR 상위 16bit = reset, 하위 16bit = set)
    GPIOA->BSRR = pa_set | (pa_clr << 16);
    GPIOB->BSRR = pb_set | (pb_clr << 16);
    GPIOC->BSRR = pc_set | (pc_clr << 16);
    
    // WR 스트로브
    LCD_WR_LOW();
    __NOP();
    LCD_WR_HIGH();
}

// ★ 16-bit 컬러 고속 출력 (연속 전송) ★
static inline void LCD_Write16Fast(uint16_t color) {
    LCD_Write8Fast(color >> 8);
    LCD_Write8Fast(color & 0xFF);
}

// ============================================================================
// LCD 기본 함수 (고속 버전)
// ============================================================================

static void LCD_WriteCommand(uint8_t cmd) {
    LCD_CS_LOW();
    LCD_RS_LOW();
    LCD_Write8Fast(cmd);
    LCD_CS_HIGH();
}

static void LCD_WriteData(uint8_t data) {
    LCD_CS_LOW();
    LCD_RS_HIGH();
    LCD_Write8Fast(data);
    LCD_CS_HIGH();
}

static void LCD_SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    LCD_WriteCommand(0x2A);  // CASET
    LCD_CS_LOW();
    LCD_RS_HIGH();
    LCD_Write8Fast(x1 >> 8);
    LCD_Write8Fast(x1 & 0xFF);
    LCD_Write8Fast(x2 >> 8);
    LCD_Write8Fast(x2 & 0xFF);
    LCD_CS_HIGH();
    
    LCD_WriteCommand(0x2B);  // PASET
    LCD_CS_LOW();
    LCD_RS_HIGH();
    LCD_Write8Fast(y1 >> 8);
    LCD_Write8Fast(y1 & 0xFF);
    LCD_Write8Fast(y2 >> 8);
    LCD_Write8Fast(y2 & 0xFF);
    LCD_CS_HIGH();
    
    LCD_WriteCommand(0x2C);  // RAMWR
}

// ★ 초고속 사각형 채우기 ★
static void LCD_FillRectFast(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if(x >= 240 || y >= 320 || w == 0 || h == 0) return;
    if(x + w > 240) w = 240 - x;
    if(y + h > 320) h = 320 - y;
    
    LCD_SetWindow(x, y, x + w - 1, y + h - 1);
    
    uint32_t total = (uint32_t)w * h;
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;
    
    LCD_CS_LOW();
    LCD_RS_HIGH();
    
    // ★ 언롤링으로 더 빠르게 ★
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
    while(total--) {
        LCD_Write8Fast(hi);
        LCD_Write8Fast(lo);
    }
    
    LCD_CS_HIGH();
}

// ★ 수평선 (가장 빠른 요소) ★
static inline void LCD_HLineFast(int16_t x, int16_t y, int16_t w, uint16_t color) {
    if(y < 0 || y >= 320 || w <= 0) return;
    if(x < 0) { w += x; x = 0; }
    if(x + w > 240) w = 240 - x;
    if(w <= 0) return;
    
    LCD_SetWindow(x, y, x + w - 1, y);
    
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;
    
    LCD_CS_LOW();
    LCD_RS_HIGH();
    while(w--) {
        LCD_Write8Fast(hi);
        LCD_Write8Fast(lo);
    }
    LCD_CS_HIGH();
}

// ============================================================================
// 전체 화면 채우기
// ============================================================================

static void LCD_Fill(uint16_t color) {
    LCD_FillRectFast(0, 0, 240, 320, color);
}

// ============================================================================
// LCD 초기화
// ============================================================================

static void LCD_Init(void) {
    LCD_RD_HIGH();
    LCD_CS_HIGH();
    
    LCD_RST_LOW();
    HAL_Delay(50);
    LCD_RST_HIGH();
    HAL_Delay(50);
    
    LCD_WriteCommand(0x01);  // Software Reset
    HAL_Delay(100);
    
    LCD_WriteCommand(0x11);  // Sleep Out
    HAL_Delay(120);
    
    LCD_WriteCommand(0xCF);
    LCD_WriteData(0x00); LCD_WriteData(0xC1); LCD_WriteData(0x30);
    
    LCD_WriteCommand(0xED);
    LCD_WriteData(0x64); LCD_WriteData(0x03); LCD_WriteData(0x12); LCD_WriteData(0x81);
    
    LCD_WriteCommand(0xE8);
    LCD_WriteData(0x85); LCD_WriteData(0x00); LCD_WriteData(0x78);
    
    LCD_WriteCommand(0xCB);
    LCD_WriteData(0x39); LCD_WriteData(0x2C); LCD_WriteData(0x00);
    LCD_WriteData(0x34); LCD_WriteData(0x02);
    
    LCD_WriteCommand(0xF7);
    LCD_WriteData(0x20);
    
    LCD_WriteCommand(0xEA);
    LCD_WriteData(0x00); LCD_WriteData(0x00);
    
    LCD_WriteCommand(0xC0);  // Power Control 1
    LCD_WriteData(0x23);
    
    LCD_WriteCommand(0xC1);  // Power Control 2
    LCD_WriteData(0x10);
    
    LCD_WriteCommand(0xC5);  // VCOM Control 1
    LCD_WriteData(0x3E); LCD_WriteData(0x28);
    
    LCD_WriteCommand(0xC7);  // VCOM Control 2
    LCD_WriteData(0x86);
    
    LCD_WriteCommand(0x36);  // Memory Access Control
    LCD_WriteData(0x48);
    
    LCD_WriteCommand(0x3A);  // Pixel Format
    LCD_WriteData(0x55);     // 16-bit
    
    LCD_WriteCommand(0xB1);  // Frame Rate Control
    LCD_WriteData(0x00); LCD_WriteData(0x18);
    
    LCD_WriteCommand(0xB6);  // Display Function Control
    LCD_WriteData(0x08); LCD_WriteData(0x82); LCD_WriteData(0x27);
    
    LCD_WriteCommand(0xF2);  // Gamma Function Disable
    LCD_WriteData(0x00);
    
    LCD_WriteCommand(0x26);  // Gamma Curve
    LCD_WriteData(0x01);
    
    LCD_WriteCommand(0xE0);  // Positive Gamma
    LCD_WriteData(0x0F); LCD_WriteData(0x31); LCD_WriteData(0x2B); LCD_WriteData(0x0C);
    LCD_WriteData(0x0E); LCD_WriteData(0x08); LCD_WriteData(0x4E); LCD_WriteData(0xF1);
    LCD_WriteData(0x37); LCD_WriteData(0x07); LCD_WriteData(0x10); LCD_WriteData(0x03);
    LCD_WriteData(0x0E); LCD_WriteData(0x09); LCD_WriteData(0x00);
    
    LCD_WriteCommand(0xE1);  // Negative Gamma
    LCD_WriteData(0x00); LCD_WriteData(0x0E); LCD_WriteData(0x14); LCD_WriteData(0x03);
    LCD_WriteData(0x11); LCD_WriteData(0x07); LCD_WriteData(0x31); LCD_WriteData(0xC1);
    LCD_WriteData(0x48); LCD_WriteData(0x08); LCD_WriteData(0x0F); LCD_WriteData(0x0C);
    LCD_WriteData(0x31); LCD_WriteData(0x36); LCD_WriteData(0x0F);
    
    LCD_WriteCommand(0x11);  // Sleep Out
    HAL_Delay(120);
    
    LCD_WriteCommand(0x29);  // Display On
    HAL_Delay(50);
}

// ============================================================================
// 눈 설정
// ============================================================================

#define EYE_AREA_X      10
#define EYE_AREA_Y      80
#define EYE_AREA_W      220
#define EYE_AREA_H      160

#define LX              55
#define RX              165
#define CY              80

#define EYE_W           50
#define EYE_H           70
#define EYE_R           18

#define EYE_COLOR       0x07E0   // GREEN
#define EYE_BRIGHT      0xAFE0
#define EYE_DIM         0x0320
#define EYE_BG          0x0000   // BLACK

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
// 그리기 함수 (고속)
// ============================================================================

// 채워진 원 (수평선 기반)
static void LCD_FillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    int16_t x = r, y = 0;
    int16_t err = 1 - r;
    
    while(x >= y) {
        LCD_HLineFast(x0 - x, y0 + y, x * 2 + 1, color);
        LCD_HLineFast(x0 - x, y0 - y, x * 2 + 1, color);
        LCD_HLineFast(x0 - y, y0 + x, y * 2 + 1, color);
        LCD_HLineFast(x0 - y, y0 - x, y * 2 + 1, color);
        
        y++;
        if(err < 0) err += 2 * y + 1;
        else { x--; err += 2 * (y - x + 1); }
    }
}

// 둥근 사각형
static void LCD_RoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    if(w > 2*r) LCD_FillRectFast(x + r, y, w - 2*r, h, color);
    if(h > 2*r) {
        LCD_FillRectFast(x, y + r, r, h - 2*r, color);
        LCD_FillRectFast(x + w - r, y + r, r, h - 2*r, color);
    }
    LCD_FillCircle(x + r, y + r, r, color);
    LCD_FillCircle(x + w - r - 1, y + r, r, color);
    LCD_FillCircle(x + r, y + h - r - 1, r, color);
    LCD_FillCircle(x + w - r - 1, y + h - r - 1, r, color);
}

// 두꺼운 선
static void LCD_ThickLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t t, uint16_t color) {
    int16_t dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int16_t dy = (y1 > y0) ? (y1 - y0) : (y0 - y1);
    
    if(dy <= 2) {
        int16_t minX = (x0 < x1) ? x0 : x1;
        int16_t maxX = (x0 > x1) ? x0 : x1;
        LCD_FillRectFast(minX, (y0 + y1)/2 - t/2, maxX - minX + 1, t, color);
        return;
    }
    if(dx <= 2) {
        int16_t minY = (y0 < y1) ? y0 : y1;
        int16_t maxY = (y0 > y1) ? y0 : y1;
        LCD_FillRectFast((x0 + x1)/2 - t/2, minY, t, maxY - minY + 1, color);
        return;
    }
    
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;
    
    while(1) {
        LCD_FillRectFast(x0 - t/2, y0 - t/2, t, t, color);
        if(x0 == x1 && y0 == y1) break;
        int16_t e2 = 2 * err;
        if(e2 > -dy) { err -= dy; x0 += sx; }
        if(e2 < dx) { err += dx; y0 += sy; }
    }
}

// ============================================================================
// 눈 그리기
// ============================================================================

static void Eye_Clear(void) {
    LCD_FillRectFast(EYE_AREA_X, EYE_AREA_Y, EYE_AREA_W, EYE_AREA_H, EYE_BG);
}

static void Eye_Normal(int16_t cx, int16_t ox, int16_t oy) {
    int16_t sx = EYE_AREA_X + cx - EYE_W/2;
    int16_t sy = EYE_AREA_Y + CY - EYE_H/2;
    LCD_RoundRect(sx, sy, EYE_W, EYE_H, EYE_R, EYE_COLOR);
    LCD_FillCircle(sx + 8 + ox, sy + 10 + oy, 5, EYE_BRIGHT);
}

static void Eye_Closed(int16_t cx) {
    int16_t sx = EYE_AREA_X + cx - EYE_W/2 + 5;
    int16_t sy = EYE_AREA_Y + CY;
    LCD_FillRectFast(sx, sy - 3, EYE_W - 10, 7, EYE_COLOR);
}

static void Eye_Half(int16_t cx, uint8_t pct) {
    int16_t h = (EYE_H * pct) / 100;
    if(h < 10) { Eye_Closed(cx); return; }
    int16_t sx = EYE_AREA_X + cx - EYE_W/2;
    int16_t sy = EYE_AREA_Y + CY + EYE_H/2 - h;
    LCD_RoundRect(sx, sy, EYE_W, h, EYE_R/2, EYE_COLOR);
}

static void Eye_Happy(int16_t cx) {
    int16_t bx = EYE_AREA_X + cx;
    int16_t by = EYE_AREA_Y + CY;
    for(int16_t i = -EYE_W/2 + 3; i <= EYE_W/2 - 3; i++) {
        int32_t n = (int32_t)i * i * 100 / ((EYE_W/2) * (EYE_W/2));
        int16_t y = by + 5 - (15 * (100 - n) / 100);
        LCD_FillRectFast(bx + i, y - 4, 2, 6, EYE_COLOR);
    }
}

static void Eye_Sad(int16_t cx) {
    int16_t sx = EYE_AREA_X + cx - EYE_W/2;
    int16_t sy = EYE_AREA_Y + CY - EYE_H/2 + 8;
    LCD_RoundRect(sx, sy, EYE_W, EYE_H - 8, EYE_R, EYE_COLOR);
    LCD_ThickLine(sx - 3, sy - 3, sx + EYE_W + 3, sy + 12, 5, EYE_COLOR);
}

static void Eye_Angry(int16_t cx, uint8_t is_left) {
    int16_t sx = EYE_AREA_X + cx - EYE_W/2;
    int16_t sy = EYE_AREA_Y + CY - EYE_H/2 + 10;
    LCD_RoundRect(sx, sy, EYE_W, EYE_H - 15, EYE_R - 3, EYE_COLOR);
    if(is_left)
        LCD_ThickLine(sx - 5, sy + 8, sx + EYE_W + 5, sy - 10, 6, EYE_COLOR);
    else
        LCD_ThickLine(sx - 5, sy - 10, sx + EYE_W + 5, sy + 8, 6, EYE_COLOR);
}

static void Eye_Surprised(int16_t cx) {
    int16_t x = EYE_AREA_X + cx;
    int16_t y = EYE_AREA_Y + CY;
    LCD_FillCircle(x, y, EYE_H/2 + 5, EYE_COLOR);
    LCD_FillCircle(x, y, EYE_H/2 - 8, EYE_DIM);
    LCD_FillCircle(x - 8, y - 8, 7, EYE_BRIGHT);
    LCD_FillCircle(x + 4, y + 4, 4, EYE_BRIGHT);
}

static void Eye_Heart(int16_t cx) {
    int16_t x = EYE_AREA_X + cx;
    int16_t y = EYE_AREA_Y + CY;
    int16_t s = 18;
    LCD_FillCircle(x - s/2 - 2, y - s/3, s/2 + 2, EYE_COLOR);
    LCD_FillCircle(x + s/2 + 2, y - s/3, s/2 + 2, EYE_COLOR);
    for(int16_t r = 0; r < s + 5; r++) {
        int16_t w = s + 5 - r;
        LCD_FillRectFast(x - w, y - s/3 + r, w * 2 + 1, 1, EYE_COLOR);
    }
}

static void Eye_X(int16_t cx) {
    int16_t x = EYE_AREA_X + cx;
    int16_t y = EYE_AREA_Y + CY;
    int16_t s = EYE_H/2 - 8;
    LCD_ThickLine(x - s, y - s, x + s, y + s, 6, EYE_COLOR);
    LCD_ThickLine(x + s, y - s, x - s, y + s, 6, EYE_COLOR);
}

// ============================================================================
// 표정 & 애니메이션
// ============================================================================

static void Draw_Expression(Expression_t expr, int16_t ox, int16_t oy) {
    Eye_Clear();
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
            Eye_Normal(LX, -8, 0);
            Eye_Normal(RX, -8, 0);
            break;
        case EXPR_LOOK_RIGHT:
            Eye_Normal(LX, 8, 0);
            Eye_Normal(RX, 8, 0);
            break;
        case EXPR_LOOK_UP:
            Eye_Normal(LX, 0, -8);
            Eye_Normal(RX, 0, -8);
            break;
        case EXPR_LOOK_DOWN:
            Eye_Normal(LX, 0, 8);
            Eye_Normal(RX, 0, 8);
            break;
    }
}

static void Anim_SetExpr(Expression_t expr) {
    current_expr = expr;
    Draw_Expression(expr, 0, 0);
}

static void Anim_Blink(void) {
    Eye_Clear();
    Eye_Half(LX, 50);
    Eye_Half(RX, 50);
    
    Eye_Clear();
    Eye_Closed(LX);
    Eye_Closed(RX);
    HAL_Delay(40);
    
    Eye_Clear();
    Eye_Half(LX, 50);
    Eye_Half(RX, 50);
    
    Draw_Expression(current_expr, 0, 0);
}

static void Anim_WinkL(void) {
    Eye_Clear();
    Eye_Closed(LX);
    Eye_Normal(RX, 0, 0);
    HAL_Delay(180);
    Draw_Expression(EXPR_NORMAL, 0, 0);
}

static void Anim_WinkR(void) {
    Eye_Clear();
    Eye_Normal(LX, 0, 0);
    Eye_Closed(RX);
    HAL_Delay(180);
    Draw_Expression(EXPR_NORMAL, 0, 0);
}

static void Anim_LookAround(void) {
    Anim_SetExpr(EXPR_LOOK_LEFT);
    HAL_Delay(280);
    Anim_SetExpr(EXPR_NORMAL);
    HAL_Delay(80);
    Anim_SetExpr(EXPR_LOOK_RIGHT);
    HAL_Delay(280);
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
// GPIO 초기화
// ============================================================================

static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // 초기 상태
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

    // ★ 고속 모드로 설정 ★
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
}

// ============================================================================
// System Clock (64MHz)
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

// ============================================================================
// Main
// ============================================================================

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    
    LCD_Init();
    LCD_Fill(0x0000);  // Black
    
    srand(HAL_GetTick());
    
    Anim_SetExpr(EXPR_NORMAL);
    HAL_Delay(500);
    
    while(1) {
        // 데모 모드
        Anim_Demo();
        
        // 또는 Idle 모드
        // Anim_Idle();
        // HAL_Delay(20);
    }
}

void Error_Handler(void) {
    __disable_irq();
    while(1) {}
}
