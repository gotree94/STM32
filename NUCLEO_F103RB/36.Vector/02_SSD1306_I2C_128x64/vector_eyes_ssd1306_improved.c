/* ============================================================================
 * Vector Robot Eye Animation - SSD1306 I2C OLED Version (0.96" 128x64)
 * ★ 단색 눈동자/하이라이트 표현 개선 버전 ★
 * ============================================================================
 *
 * 하드웨어 연결:
 *   - I2C1: PB6(SCL), PB7(SDA)
 *   - 전원: 3.3V, GND
 *
 * CubeMX 설정:
 *   - I2C1: Fast Mode 400kHz
 *   - System Clock: 64MHz
 *
 * I2C 주소:
 *   - 기본: 0x3C (코드에서는 0x78 = 0x3C << 1)
 *   - 대안: 0x3D (코드에서는 0x7A = 0x3D << 1)
 *
 * ============================================================================
 */

#include "main.h"
#include <string.h>
#include <stdlib.h>

I2C_HandleTypeDef hi2c1;

// ============================================================================
// SSD1306 설정
// ============================================================================

#define SSD1306_I2C_ADDR        0x78    // 0x3C << 1 (안되면 0x7A 시도)
#define SSD1306_WIDTH           128
#define SSD1306_HEIGHT          64

// SSD1306 Commands
#define SSD1306_CMD             0x00
#define SSD1306_DATA            0x40
#define SSD1306_DISPLAY_OFF     0xAE
#define SSD1306_DISPLAY_ON      0xAF
#define SSD1306_SET_DISPLAY_CLOCK_DIV 0xD5
#define SSD1306_SET_MULTIPLEX   0xA8
#define SSD1306_SET_DISPLAY_OFFSET 0xD3
#define SSD1306_SET_START_LINE  0x40
#define SSD1306_CHARGE_PUMP     0x8D
#define SSD1306_MEMORY_MODE     0x20
#define SSD1306_SEG_REMAP       0xA0
#define SSD1306_COM_SCAN_DEC    0xC8
#define SSD1306_SET_COM_PINS    0xDA
#define SSD1306_SET_CONTRAST    0x81
#define SSD1306_SET_PRECHARGE   0xD9
#define SSD1306_SET_VCOM_DETECT 0xDB
#define SSD1306_DISPLAY_ALL_ON_RESUME 0xA4
#define SSD1306_NORMAL_DISPLAY  0xA6
#define SSD1306_COLUMN_ADDR     0x21
#define SSD1306_PAGE_ADDR       0x22

// 프레임버퍼 (128 x 64 / 8 = 1024 bytes)
static uint8_t frame_buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

// 눈 위치 (화면 좌표)
#define LX              32      // 왼쪽 눈 중심 X
#define RX              96      // 오른쪽 눈 중심 X
#define CY              32      // 눈 중심 Y

// 눈 크기
#define EYE_W           26
#define EYE_H           34
#define EYE_R           9

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
// SSD1306 I2C 함수
// ============================================================================

static void SSD1306_WriteCmd(uint8_t cmd) {
    uint8_t data[2] = {SSD1306_CMD, cmd};
    HAL_I2C_Master_Transmit(&hi2c1, SSD1306_I2C_ADDR, data, 2, HAL_MAX_DELAY);
}

static void SSD1306_Init(void) {
    HAL_Delay(100);

    SSD1306_WriteCmd(SSD1306_DISPLAY_OFF);
    SSD1306_WriteCmd(SSD1306_SET_DISPLAY_CLOCK_DIV);
    SSD1306_WriteCmd(0x80);
    SSD1306_WriteCmd(SSD1306_SET_MULTIPLEX);
    SSD1306_WriteCmd(SSD1306_HEIGHT - 1);
    SSD1306_WriteCmd(SSD1306_SET_DISPLAY_OFFSET);
    SSD1306_WriteCmd(0x00);
    SSD1306_WriteCmd(SSD1306_SET_START_LINE | 0x00);
    SSD1306_WriteCmd(SSD1306_CHARGE_PUMP);
    SSD1306_WriteCmd(0x14);
    SSD1306_WriteCmd(SSD1306_MEMORY_MODE);
    SSD1306_WriteCmd(0x00);
    SSD1306_WriteCmd(SSD1306_SEG_REMAP | 0x01);
    SSD1306_WriteCmd(SSD1306_COM_SCAN_DEC);
    SSD1306_WriteCmd(SSD1306_SET_COM_PINS);
    SSD1306_WriteCmd(0x12);
    SSD1306_WriteCmd(SSD1306_SET_CONTRAST);
    SSD1306_WriteCmd(0xCF);
    SSD1306_WriteCmd(SSD1306_SET_PRECHARGE);
    SSD1306_WriteCmd(0xF1);
    SSD1306_WriteCmd(SSD1306_SET_VCOM_DETECT);
    SSD1306_WriteCmd(0x40);
    SSD1306_WriteCmd(SSD1306_DISPLAY_ALL_ON_RESUME);
    SSD1306_WriteCmd(SSD1306_NORMAL_DISPLAY);
    SSD1306_WriteCmd(SSD1306_DISPLAY_ON);
}

static void SSD1306_Update(void) {
    SSD1306_WriteCmd(SSD1306_COLUMN_ADDR);
    SSD1306_WriteCmd(0);
    SSD1306_WriteCmd(SSD1306_WIDTH - 1);

    SSD1306_WriteCmd(SSD1306_PAGE_ADDR);
    SSD1306_WriteCmd(0);
    SSD1306_WriteCmd(7);

    uint8_t buffer[1025];
    buffer[0] = SSD1306_DATA;
    memcpy(&buffer[1], frame_buffer, sizeof(frame_buffer));

    HAL_I2C_Master_Transmit(&hi2c1, SSD1306_I2C_ADDR, buffer, sizeof(buffer), HAL_MAX_DELAY);
}

// ============================================================================
// 프레임버퍼 그리기 함수
// ============================================================================

static void Buf_Clear(void) {
    memset(frame_buffer, 0, sizeof(frame_buffer));
}

static inline void Buf_SetPixel(int16_t x, int16_t y, uint8_t color) {
    if(x < 0 || x >= SSD1306_WIDTH || y < 0 || y >= SSD1306_HEIGHT) return;

    if(color) {
        frame_buffer[x + (y / 8) * SSD1306_WIDTH] |= (1 << (y & 7));
    } else {
        frame_buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y & 7));
    }
}

static void Buf_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color) {
    for(int16_t j = y; j < y + h; j++) {
        for(int16_t i = x; i < x + w; i++) {
            Buf_SetPixel(i, j, color);
        }
    }
}

static void Buf_FillCircle(int16_t cx, int16_t cy, int16_t r, uint8_t color) {
    int16_t x = r, y = 0;
    int16_t err = 1 - r;

    while(x >= y) {
        for(int16_t i = cx - x; i <= cx + x; i++) {
            Buf_SetPixel(i, cy + y, color);
            Buf_SetPixel(i, cy - y, color);
        }
        for(int16_t i = cx - y; i <= cx + y; i++) {
            Buf_SetPixel(i, cy + x, color);
            Buf_SetPixel(i, cy - x, color);
        }
        y++;
        if(err < 0) err += 2 * y + 1;
        else { x--; err += 2 * (y - x + 1); }
    }
}

// ★ 원 외곽선만 그리기 ★
static void Buf_DrawCircle(int16_t cx, int16_t cy, int16_t r, uint8_t color) {
    int16_t x = r, y = 0;
    int16_t err = 1 - r;

    while(x >= y) {
        Buf_SetPixel(cx + x, cy + y, color);
        Buf_SetPixel(cx - x, cy + y, color);
        Buf_SetPixel(cx + x, cy - y, color);
        Buf_SetPixel(cx - x, cy - y, color);
        Buf_SetPixel(cx + y, cy + x, color);
        Buf_SetPixel(cx - y, cy + x, color);
        Buf_SetPixel(cx + y, cy - x, color);
        Buf_SetPixel(cx - y, cy - x, color);
        
        y++;
        if(err < 0) err += 2 * y + 1;
        else { x--; err += 2 * (y - x + 1); }
    }
}

static void Buf_RoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint8_t color) {
    Buf_FillRect(x + r, y, w - 2*r, h, color);
    Buf_FillRect(x, y + r, r, h - 2*r, color);
    Buf_FillRect(x + w - r, y + r, r, h - 2*r, color);
    Buf_FillCircle(x + r, y + r, r, color);
    Buf_FillCircle(x + w - r - 1, y + r, r, color);
    Buf_FillCircle(x + r, y + h - r - 1, r, color);
    Buf_FillCircle(x + w - r - 1, y + h - r - 1, r, color);
}

static void Buf_HLine(int16_t x, int16_t y, int16_t w, int16_t thick, uint8_t color) {
    Buf_FillRect(x, y - thick/2, w, thick, color);
}

static void Buf_ThickLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t t, uint8_t color) {
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

// ============================================================================
// 눈 그리기 함수 (단색 표현 개선)
// ============================================================================

// ★ 일반 눈: 외곽 + 눈동자(검은 구멍) + 하이라이트(흰 점) ★
static void Eye_Normal(int16_t cx, int16_t ox, int16_t oy) {
    // 1. 눈 외곽 (흰색 채움)
    Buf_RoundRect(cx - EYE_W/2, CY - EYE_H/2, EYE_W, EYE_H, EYE_R, 1);
    
    // 2. 눈동자 (검은 원) - 시선 방향에 따라 이동
    int16_t pupil_x = cx + ox;
    int16_t pupil_y = CY + oy;
    Buf_FillCircle(pupil_x, pupil_y, 6, 0);  // 검은색 눈동자
    
    // 3. 하이라이트 (흰 점) - 눈동자 위쪽에
    Buf_FillCircle(pupil_x - 2, pupil_y - 3, 2, 1);  // 작은 흰 점
}

// 감은 눈
static void Eye_Closed(int16_t cx) {
    Buf_HLine(cx - EYE_W/2 + 2, CY, EYE_W - 4, 4, 1);
}

// 반쯤 감은 눈
static void Eye_Half(int16_t cx, uint8_t pct) {
    int16_t h = (EYE_H * pct) / 100;
    if(h < 8) { Eye_Closed(cx); return; }
    int16_t top = CY + EYE_H/2 - h;
    Buf_RoundRect(cx - EYE_W/2, top, EYE_W, h, EYE_R/2, 1);
    
    // 눈동자 (반쯤 보이게)
    if(pct > 40) {
        Buf_FillCircle(cx, CY + 2, 5, 0);
        Buf_FillCircle(cx - 2, CY - 1, 2, 1);
    }
}

// ★ 행복한 눈: ^ ^ 모양 (두껍게) ★
static void Eye_Happy(int16_t cx) {
    for(int16_t i = -EYE_W/2 + 2; i <= EYE_W/2 - 2; i++) {
        int32_t n = (int32_t)i * i * 100 / ((EYE_W/2) * (EYE_W/2));
        int16_t y = CY + 3 - (10 * (100 - n) / 100);
        // 더 두껍게
        Buf_FillRect(cx + i, y - 2, 2, 4, 1);
    }
}

// ★ 슬픈 눈: 처진 눈썹 + 눈동자 ★
static void Eye_Sad(int16_t cx) {
    // 눈
    Buf_RoundRect(cx - EYE_W/2, CY - EYE_H/2 + 5, EYE_W, EYE_H - 5, EYE_R, 1);
    // 눈동자
    Buf_FillCircle(cx, CY + 3, 5, 0);
    Buf_FillCircle(cx - 2, CY, 2, 1);
    // 슬픈 눈썹
    Buf_ThickLine(cx - EYE_W/2, CY - EYE_H/2 - 1, 
                  cx + EYE_W/2, CY - EYE_H/2 + 7, 3, 1);
}

// ★ 화난 눈: 찡그린 눈썹 + 눈동자 ★
static void Eye_Angry(int16_t cx, uint8_t is_left) {
    // 눈 (좀 더 작게)
    Buf_RoundRect(cx - EYE_W/2, CY - EYE_H/2 + 6, EYE_W, EYE_H - 10, EYE_R - 2, 1);
    // 눈동자
    Buf_FillCircle(cx, CY + 1, 4, 0);
    Buf_FillCircle(cx - 1, CY - 1, 1, 1);
    // 화난 눈썹
    if(is_left) {
        Buf_ThickLine(cx - EYE_W/2 - 2, CY - EYE_H/2 + 5, 
                      cx + EYE_W/2 + 2, CY - EYE_H/2 - 5, 4, 1);
    } else {
        Buf_ThickLine(cx - EYE_W/2 - 2, CY - EYE_H/2 - 5, 
                      cx + EYE_W/2 + 2, CY - EYE_H/2 + 5, 4, 1);
    }
}

// ★ 놀란 눈: 큰 원 + 작은 눈동자 + 하이라이트 ★
static void Eye_Surprised(int16_t cx) {
    // 큰 원 (외곽)
    Buf_FillCircle(cx, CY, EYE_H/2 + 2, 1);
    // 눈동자 (검은색, 크게)
    Buf_FillCircle(cx, CY, EYE_H/2 - 5, 0);
    // 하이라이트 2개
    Buf_FillCircle(cx - 4, CY - 5, 3, 1);  // 큰 하이라이트
    Buf_FillCircle(cx + 3, CY + 3, 2, 1);  // 작은 하이라이트
}

// ★ 하트 눈 ★
static void Eye_Heart(int16_t cx) {
    int16_t s = 10;
    // 하트 상단 두 원
    Buf_FillCircle(cx - s/2, CY - s/3, s/2, 1);
    Buf_FillCircle(cx + s/2, CY - s/3, s/2, 1);
    // 하트 하단 삼각형
    for(int16_t r = 0; r < s + 2; r++) {
        int16_t w = s + 2 - r;
        Buf_FillRect(cx - w, CY - s/3 + r, w * 2 + 1, 1, 1);
    }
}

// ★ X 눈 (어지러움) ★
static void Eye_X(int16_t cx) {
    int16_t s = EYE_H/2 - 3;
    Buf_ThickLine(cx - s, CY - s, cx + s, CY + s, 4, 1);
    Buf_ThickLine(cx + s, CY - s, cx - s, CY + s, 4, 1);
}

// ★ 졸린 눈: 반달 모양 ★
static void Eye_Sleepy(int16_t cx) {
    // 반달 눈 (아래쪽 반원만)
    for(int16_t y = 0; y <= EYE_H/3; y++) {
        int16_t w = EYE_W/2 - (y * EYE_W / EYE_H);
        if(w > 0) {
            Buf_FillRect(cx - w, CY + y, w * 2, 1, 1);
        }
    }
    // 눈 라인
    Buf_HLine(cx - EYE_W/2 + 2, CY - 1, EYE_W - 4, 3, 1);
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

    SSD1306_Update();
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
    Buf_Clear();
    Eye_Half(LX, 70);
    Eye_Half(RX, 70);
    SSD1306_Update();
    
    // 30% 감기
    Buf_Clear();
    Eye_Half(LX, 30);
    Eye_Half(RX, 30);
    SSD1306_Update();

    // 완전히 감기
    Buf_Clear();
    Eye_Closed(LX);
    Eye_Closed(RX);
    SSD1306_Update();
    HAL_Delay(50);

    // 30% 뜨기
    Buf_Clear();
    Eye_Half(LX, 30);
    Eye_Half(RX, 30);
    SSD1306_Update();
    
    // 70% 뜨기
    Buf_Clear();
    Eye_Half(LX, 70);
    Eye_Half(RX, 70);
    SSD1306_Update();

    Draw_Expression(current_expr, 0, 0);
}

static void Anim_WinkL(void) {
    Buf_Clear();
    Eye_Closed(LX);
    Eye_Normal(RX, 0, 0);
    SSD1306_Update();
    HAL_Delay(200);
    Draw_Expression(EXPR_NORMAL, 0, 0);
}

static void Anim_WinkR(void) {
    Buf_Clear();
    Eye_Normal(LX, 0, 0);
    Eye_Closed(RX);
    SSD1306_Update();
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
static void MX_I2C1_Init(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();

    SSD1306_Init();
    Buf_Clear();
    SSD1306_Update();

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

static void MX_I2C1_Init(void) {
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 400000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        Error_Handler();
    }
}

static void MX_GPIO_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
}

void Error_Handler(void) {
    __disable_irq();
    while(1) {}
}
