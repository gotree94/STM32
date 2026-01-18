/* ============================================================================
 * Vector Robot Eye Animation - ST7735S SPI Version (0.96" 160x80 LCD)
 * ★ SSD1306과 동일한 눈동자 표현 버전 ★
 * ============================================================================
 *
 * 하드웨어 연결:
 *   - SPI1: PA5(SCK), PA7(MOSI)
 *   - GPIO: PA1(RES), PA6(DC), PB6(CS)
 *   - 전원: 3.3V, GND
 *
 * LCD 배치: 핀이 위쪽, 가로로 보기
 *
 * CubeMX 설정:
 *   - SPI1: Mode=Transmit Only Master, Prescaler=4
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

// ★ 오프셋 설정 (0.96" 160x80 LCD, 핀 위쪽) ★
#define X_OFFSET   0
#define Y_OFFSET   26

// ST7735 Commands
#define ST7735_SWRESET 0x01
#define ST7735_SLPOUT  0x11
#define ST7735_NORON   0x13
#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
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

// Colors (RGB565)
#define BLACK       0x0000
#define WHITE       0xFFFF
#define EYE_COLOR   0x07E0   // Green (눈 외곽)
#define EYE_PUPIL   0x0320   // Dark Green (눈동자)
#define EYE_BRIGHT  0xAFE5   // Light Green (하이라이트)

// 눈 위치 (화면 좌표)
#define LX              40      // 왼쪽 눈 중심 X
#define RX              120     // 오른쪽 눈 중심 X
#define CY              40      // 눈 중심 Y (화면 중앙)

// 눈 크기
#define EYE_W           30
#define EYE_H           50
#define EYE_R           10

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

static void LCD_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    LCD_Cmd(ST7735_CASET);
    LCD_Data(0); LCD_Data(x0 + X_OFFSET);
    LCD_Data(0); LCD_Data(x1 + X_OFFSET);

    LCD_Cmd(ST7735_RASET);
    LCD_Data(0); LCD_Data(y0 + Y_OFFSET);
    LCD_Data(0); LCD_Data(y1 + Y_OFFSET);

    LCD_Cmd(ST7735_RAMWR);
}

static void LCD_WriteColorFast(uint16_t color, uint32_t count) {
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;

    LCD_DC_HIGH();
    LCD_CS_LOW();
    while(count--) {
        HAL_SPI_Transmit(&hspi1, &hi, 1, HAL_MAX_DELAY);
        HAL_SPI_Transmit(&hspi1, &lo, 1, HAL_MAX_DELAY);
    }
    LCD_CS_HIGH();
}

static void LCD_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if(x >= LCD_WIDTH || y >= LCD_HEIGHT || w <= 0 || h <= 0) return;
    if(x < 0) { w += x; x = 0; }
    if(y < 0) { h += y; y = 0; }
    if(x + w > LCD_WIDTH) w = LCD_WIDTH - x;
    if(y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;
    if(w <= 0 || h <= 0) return;

    LCD_SetWindow(x, y, x + w - 1, y + h - 1);
    LCD_WriteColorFast(color, (uint32_t)w * h);
}

static void LCD_HLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    if(y < 0 || y >= LCD_HEIGHT || w <= 0) return;
    if(x < 0) { w += x; x = 0; }
    if(x + w > LCD_WIDTH) w = LCD_WIDTH - x;
    if(w <= 0) return;

    LCD_SetWindow(x, y, x + w - 1, y);
    LCD_WriteColorFast(color, w);
}

static void LCD_FillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    int16_t x = r, y = 0;
    int16_t err = 1 - r;

    while(x >= y) {
        LCD_HLine(x0 - x, y0 + y, x * 2 + 1, color);
        LCD_HLine(x0 - x, y0 - y, x * 2 + 1, color);
        LCD_HLine(x0 - y, y0 + x, y * 2 + 1, color);
        LCD_HLine(x0 - y, y0 - x, y * 2 + 1, color);

        y++;
        if(err < 0) err += 2 * y + 1;
        else { x--; err += 2 * (y - x + 1); }
    }
}

static void LCD_RoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    if(w > 2*r) LCD_FillRect(x + r, y, w - 2*r, h, color);
    if(h > 2*r) {
        LCD_FillRect(x, y + r, r, h - 2*r, color);
        LCD_FillRect(x + w - r, y + r, r, h - 2*r, color);
    }
    LCD_FillCircle(x + r, y + r, r, color);
    LCD_FillCircle(x + w - r - 1, y + r, r, color);
    LCD_FillCircle(x + r, y + h - r - 1, r, color);
    LCD_FillCircle(x + w - r - 1, y + h - r - 1, r, color);
}

static void LCD_ThickLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t t, uint16_t color) {
    int16_t dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int16_t dy = (y1 > y0) ? (y1 - y0) : (y0 - y1);

    if(dy <= 2) {
        int16_t minX = (x0 < x1) ? x0 : x1;
        int16_t maxX = (x0 > x1) ? x0 : x1;
        LCD_FillRect(minX, (y0 + y1)/2 - t/2, maxX - minX + 1, t, color);
        return;
    }
    if(dx <= 2) {
        int16_t minY = (y0 < y1) ? y0 : y1;
        int16_t maxY = (y0 > y1) ? y0 : y1;
        LCD_FillRect((x0 + x1)/2 - t/2, minY, t, maxY - minY + 1, color);
        return;
    }

    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;

    while(1) {
        LCD_FillRect(x0 - t/2, y0 - t/2, t, t, color);
        if(x0 == x1 && y0 == y1) break;
        int16_t e2 = 2 * err;
        if(e2 > -dy) { err -= dy; x0 += sx; }
        if(e2 < dx) { err += dx; y0 += sy; }
    }
}

// ============================================================================
// LCD 초기화
// ============================================================================

static void LCD_Init(void) {
    LCD_RES_LOW(); HAL_Delay(50);
    LCD_RES_HIGH(); HAL_Delay(50);

    LCD_Cmd(ST7735_SWRESET); HAL_Delay(150);
    LCD_Cmd(ST7735_SLPOUT); HAL_Delay(150);

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

    LCD_Cmd(ST7735_GMCTRP1);
    uint8_t gp[] = {0x02,0x1c,0x07,0x12,0x37,0x32,0x29,0x2d,0x29,0x25,0x2B,0x39,0x00,0x01,0x03,0x10};
    for(int i=0;i<16;i++) LCD_Data(gp[i]);
    LCD_Cmd(ST7735_GMCTRN1);
    uint8_t gn[] = {0x03,0x1d,0x07,0x06,0x2E,0x2C,0x29,0x2D,0x2E,0x2E,0x37,0x3F,0x00,0x00,0x02,0x10};
    for(int i=0;i<16;i++) LCD_Data(gn[i]);

    LCD_Cmd(ST7735_NORON); HAL_Delay(10);
    LCD_Cmd(ST7735_DISPON); HAL_Delay(100);
}

static void LCD_Clear(uint16_t color) {
    LCD_FillRect(0, 0, LCD_WIDTH, LCD_HEIGHT, color);
}

// ============================================================================
// ★ 눈 그리기 함수 (SSD1306과 동일한 표현) ★
// ============================================================================

// ★ 일반 눈: 외곽 + 눈동자 + 하이라이트 ★
static void Eye_Normal(int16_t cx, int16_t ox, int16_t oy) {
    // 1. 눈 외곽 (초록색)
    LCD_RoundRect(cx - EYE_W/2, CY - EYE_H/2, EYE_W, EYE_H, EYE_R, EYE_COLOR);
    
    // 2. 눈동자 (어두운 초록) - 시선 방향에 따라 이동
    int16_t pupil_x = cx + ox;
    int16_t pupil_y = CY + oy;
    LCD_FillCircle(pupil_x, pupil_y, 8, EYE_PUPIL);
    
    // 3. 하이라이트 (밝은 초록)
    LCD_FillCircle(pupil_x - 3, pupil_y - 4, 3, EYE_BRIGHT);
}

// 감은 눈
static void Eye_Closed(int16_t cx) {
    LCD_FillRect(cx - EYE_W/2 + 2, CY - 3, EYE_W - 4, 6, EYE_COLOR);
}

// 반쯤 감은 눈
static void Eye_Half(int16_t cx, uint8_t pct) {
    int16_t h = (EYE_H * pct) / 100;
    if(h < 10) { Eye_Closed(cx); return; }
    int16_t top = CY + EYE_H/2 - h;
    LCD_RoundRect(cx - EYE_W/2, top, EYE_W, h, EYE_R/2, EYE_COLOR);
    
    // 눈동자 (반쯤 보이게)
    if(pct > 40) {
        LCD_FillCircle(cx, CY + 3, 7, EYE_PUPIL);
        LCD_FillCircle(cx - 2, CY, 2, EYE_BRIGHT);
    }
}

// ★ 행복한 눈: ^ ^ 모양 ★
static void Eye_Happy(int16_t cx) {
    for(int16_t i = -EYE_W/2 + 2; i <= EYE_W/2 - 2; i++) {
        int32_t n = (int32_t)i * i * 100 / ((EYE_W/2) * (EYE_W/2));
        int16_t y = CY + 5 - (15 * (100 - n) / 100);
        LCD_FillRect(cx + i, y - 3, 2, 6, EYE_COLOR);
    }
}

// ★ 슬픈 눈: 처진 눈썹 + 눈동자 ★
static void Eye_Sad(int16_t cx) {
    // 눈
    LCD_RoundRect(cx - EYE_W/2, CY - EYE_H/2 + 6, EYE_W, EYE_H - 6, EYE_R, EYE_COLOR);
    // 눈동자
    LCD_FillCircle(cx, CY + 4, 7, EYE_PUPIL);
    LCD_FillCircle(cx - 2, CY + 1, 2, EYE_BRIGHT);
    // 슬픈 눈썹
    LCD_ThickLine(cx - EYE_W/2, CY - EYE_H/2 - 2, 
                  cx + EYE_W/2, CY - EYE_H/2 + 10, 4, EYE_COLOR);
}

// ★ 화난 눈: 찡그린 눈썹 + 눈동자 ★
static void Eye_Angry(int16_t cx, uint8_t is_left) {
    // 눈 (좀 더 작게)
    LCD_RoundRect(cx - EYE_W/2, CY - EYE_H/2 + 8, EYE_W, EYE_H - 12, EYE_R - 2, EYE_COLOR);
    // 눈동자
    LCD_FillCircle(cx, CY + 2, 6, EYE_PUPIL);
    LCD_FillCircle(cx - 2, CY, 2, EYE_BRIGHT);
    // 화난 눈썹
    if(is_left) {
        LCD_ThickLine(cx - EYE_W/2 - 3, CY - EYE_H/2 + 6, 
                      cx + EYE_W/2 + 3, CY - EYE_H/2 - 6, 5, EYE_COLOR);
    } else {
        LCD_ThickLine(cx - EYE_W/2 - 3, CY - EYE_H/2 - 6, 
                      cx + EYE_W/2 + 3, CY - EYE_H/2 + 6, 5, EYE_COLOR);
    }
}

// ★ 놀란 눈: 큰 원 + 눈동자 + 하이라이트 ★
static void Eye_Surprised(int16_t cx) {
    // 큰 원 (외곽)
    LCD_FillCircle(cx, CY, EYE_H/2 + 3, EYE_COLOR);
    // 눈동자 (어두운 색, 크게)
    LCD_FillCircle(cx, CY, EYE_H/2 - 6, EYE_PUPIL);
    // 하이라이트 2개
    LCD_FillCircle(cx - 5, CY - 6, 5, EYE_BRIGHT);
    LCD_FillCircle(cx + 4, CY + 4, 3, EYE_BRIGHT);
}

// ★ 졸린 눈: 반달 모양 ★
static void Eye_Sleepy(int16_t cx) {
    // 반달 눈 (아래쪽 반원만)
    for(int16_t y = 0; y <= EYE_H/3; y++) {
        int16_t w = EYE_W/2 - (y * EYE_W / EYE_H);
        if(w > 0) {
            LCD_FillRect(cx - w, CY + y, w * 2, 1, EYE_COLOR);
        }
    }
    // 눈 라인
    LCD_FillRect(cx - EYE_W/2 + 2, CY - 2, EYE_W - 4, 4, EYE_COLOR);
}

// ★ 하트 눈 ★
static void Eye_Heart(int16_t cx) {
    int16_t s = 16;
    // 하트 상단 두 원
    LCD_FillCircle(cx - s/2, CY - s/3, s/2, EYE_COLOR);
    LCD_FillCircle(cx + s/2, CY - s/3, s/2, EYE_COLOR);
    // 하트 하단 삼각형
    for(int16_t r = 0; r < s + 3; r++) {
        int16_t w = s + 3 - r;
        LCD_FillRect(cx - w, CY - s/3 + r, w * 2 + 1, 1, EYE_COLOR);
    }
}

// ★ X 눈 (어지러움) ★
static void Eye_X(int16_t cx) {
    int16_t s = EYE_H/2 - 4;
    LCD_ThickLine(cx - s, CY - s, cx + s, CY + s, 6, EYE_COLOR);
    LCD_ThickLine(cx + s, CY - s, cx - s, CY + s, 6, EYE_COLOR);
}

// ============================================================================
// 표정 설정
// ============================================================================

static void Draw_Expression(Expression_t expr, int16_t ox, int16_t oy) {
    LCD_Clear(BLACK);

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
            Eye_Sleepy(LX);
            Eye_Sleepy(RX);
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
            Eye_Normal(LX, -6, 0);
            Eye_Normal(RX, -6, 0);
            break;
        case EXPR_LOOK_RIGHT:
            Eye_Normal(LX, 6, 0);
            Eye_Normal(RX, 6, 0);
            break;
        case EXPR_LOOK_UP:
            Eye_Normal(LX, 0, -6);
            Eye_Normal(RX, 0, -6);
            break;
        case EXPR_LOOK_DOWN:
            Eye_Normal(LX, 0, 6);
            Eye_Normal(RX, 0, 6);
            break;
    }
}

static void Anim_SetExpr(Expression_t expr) {
    current_expr = expr;
    Draw_Expression(expr, 0, 0);
}

// ============================================================================
// 애니메이션
// ============================================================================

static void Anim_Blink(void) {
    // 70% 감기
    LCD_Clear(BLACK);
    Eye_Half(LX, 70);
    Eye_Half(RX, 70);
    
    // 30% 감기
    LCD_Clear(BLACK);
    Eye_Half(LX, 30);
    Eye_Half(RX, 30);

    // 완전히 감기
    LCD_Clear(BLACK);
    Eye_Closed(LX);
    Eye_Closed(RX);
    HAL_Delay(50);

    // 30% 뜨기
    LCD_Clear(BLACK);
    Eye_Half(LX, 30);
    Eye_Half(RX, 30);
    
    // 70% 뜨기
    LCD_Clear(BLACK);
    Eye_Half(LX, 70);
    Eye_Half(RX, 70);

    Draw_Expression(current_expr, 0, 0);
}

static void Anim_WinkL(void) {
    LCD_Clear(BLACK);
    Eye_Closed(LX);
    Eye_Normal(RX, 0, 0);
    HAL_Delay(200);
    Draw_Expression(EXPR_NORMAL, 0, 0);
}

static void Anim_WinkR(void) {
    LCD_Clear(BLACK);
    Eye_Normal(LX, 0, 0);
    Eye_Closed(RX);
    HAL_Delay(200);
    Draw_Expression(EXPR_NORMAL, 0, 0);
}

static void Anim_LookAround(void) {
    Anim_SetExpr(EXPR_LOOK_LEFT);
    HAL_Delay(300);
    Anim_SetExpr(EXPR_NORMAL);
    HAL_Delay(100);
    Anim_SetExpr(EXPR_LOOK_RIGHT);
    HAL_Delay(300);
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
            case 0: Anim_SetExpr(EXPR_LOOK_LEFT); HAL_Delay(350); break;
            case 1: Anim_SetExpr(EXPR_LOOK_RIGHT); HAL_Delay(350); break;
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
    LCD_Clear(BLACK);

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
