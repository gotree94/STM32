#include "packman.h"

// ============================================================================
// Fast LCD drawing (direct GPIO register access)
// ============================================================================
#define LCD_CS_LOW()    GPIOB->BRR = GPIO_PIN_0
#define LCD_CS_HIGH()   GPIOB->BSRR = GPIO_PIN_0
#define LCD_RS_LOW()    GPIOA->BRR = GPIO_PIN_4
#define LCD_RS_HIGH()   GPIOA->BSRR = GPIO_PIN_4
#define LCD_WR_LOW()    GPIOA->BRR = GPIO_PIN_1
#define LCD_WR_HIGH()   GPIOA->BSRR = GPIO_PIN_1

static inline void LCD_Write8(uint8_t data) {
    uint32_t pa_set = 0, pa_clr = 0;
    if (data & 0x01) pa_set |= GPIO_PIN_9;  else pa_clr |= GPIO_PIN_9;
    if (data & 0x04) pa_set |= GPIO_PIN_10; else pa_clr |= GPIO_PIN_10;
    if (data & 0x80) pa_set |= GPIO_PIN_8;  else pa_clr |= GPIO_PIN_8;

    uint32_t pb_set = 0, pb_clr = 0;
    if (data & 0x08) pb_set |= GPIO_PIN_3;  else pb_clr |= GPIO_PIN_3;
    if (data & 0x10) pb_set |= GPIO_PIN_5;  else pb_clr |= GPIO_PIN_5;
    if (data & 0x20) pb_set |= GPIO_PIN_4;  else pb_clr |= GPIO_PIN_4;
    if (data & 0x40) pb_set |= GPIO_PIN_10; else pb_clr |= GPIO_PIN_10;

    uint32_t pc_set = 0, pc_clr = 0;
    if (data & 0x02) pc_set |= GPIO_PIN_7;  else pc_clr |= GPIO_PIN_7;

    GPIOA->BSRR = pa_set | (pa_clr << 16);
    GPIOB->BSRR = pb_set | (pb_clr << 16);
    GPIOC->BSRR = pc_set | (pc_clr << 16);

    LCD_WR_LOW();
    __NOP();
    LCD_WR_HIGH();
}

static inline void LCD_Write16(uint16_t color) {
    LCD_Write8(color >> 8);
    LCD_Write8(color & 0xFF);
}

static void LCD_WriteCmd(uint8_t cmd) {
    LCD_CS_LOW();
    LCD_RS_LOW();
    LCD_Write8(cmd);
    LCD_CS_HIGH();
}

static void LCD_WriteData(uint8_t data) {
    LCD_CS_LOW();
    LCD_RS_HIGH();
    LCD_Write8(data);
    LCD_CS_HIGH();
}

static void LCD_SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    LCD_WriteCmd(0x2A);
    LCD_CS_LOW();
    LCD_RS_HIGH();
    LCD_Write8(x1 >> 8); LCD_Write8(x1 & 0xFF);
    LCD_Write8(x2 >> 8); LCD_Write8(x2 & 0xFF);
    LCD_CS_HIGH();

    LCD_WriteCmd(0x2B);
    LCD_CS_LOW();
    LCD_RS_HIGH();
    LCD_Write8(y1 >> 8); LCD_Write8(y1 & 0xFF);
    LCD_Write8(y2 >> 8); LCD_Write8(y2 & 0xFF);
    LCD_CS_HIGH();

    LCD_WriteCmd(0x2C);
}

static void LCD_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (x >= 240 || y >= 320 || w == 0 || h == 0) return;
    if (x + w > 240) w = 240 - x;
    if (y + h > 320) h = 320 - y;

    LCD_SetWindow(x, y, x + w - 1, y + h - 1);

    uint32_t total = (uint32_t)w * h;
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;

    LCD_CS_LOW();
    LCD_RS_HIGH();

    while (total >= 8) {
        LCD_Write8(hi); LCD_Write8(lo);
        LCD_Write8(hi); LCD_Write8(lo);
        LCD_Write8(hi); LCD_Write8(lo);
        LCD_Write8(hi); LCD_Write8(lo);
        LCD_Write8(hi); LCD_Write8(lo);
        LCD_Write8(hi); LCD_Write8(lo);
        LCD_Write8(hi); LCD_Write8(lo);
        LCD_Write8(hi); LCD_Write8(lo);
        total -= 8;
    }
    while (total--) {
        LCD_Write8(hi);
        LCD_Write8(lo);
    }
    LCD_CS_HIGH();
}

static void LCD_HLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    if (y < 0 || y >= 320 || w <= 0) return;
    if (x < 0) { w += x; x = 0; }
    if (x + w > 240) w = 240 - x;
    if (w <= 0) return;

    LCD_SetWindow(x, y, x + w - 1, y);
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;
    LCD_CS_LOW();
    LCD_RS_HIGH();
    while (w--) { LCD_Write8(hi); LCD_Write8(lo); }
    LCD_CS_HIGH();
}

static void LCD_FillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    int16_t x = r, y = 0, err = 1 - r;
    while (x >= y) {
        LCD_HLine(x0 - x, y0 + y, x * 2 + 1, color);
        LCD_HLine(x0 - x, y0 - y, x * 2 + 1, color);
        LCD_HLine(x0 - y, y0 + x, y * 2 + 1, color);
        LCD_HLine(x0 - y, y0 - x, y * 2 + 1, color);
        y++;
        if (err < 0) err += 2 * y + 1;
        else { x--; err += 2 * (y - x + 1); }
    }
}

// ============================================================================
// Colors
// ============================================================================
#define COL_BG      0x0000
#define COL_WALL    0x001F
#define COL_DOT     0xFFE0
#define COL_POWER   0xFFE0
#define COL_PACMAN  0xFFE0
#define COL_GHOST_R 0xF800
#define COL_GHOST_P 0xF81F
#define COL_GHOST_C 0x07FF
#define COL_GHOST_O 0xFD20
#define COL_FRIGHT  0x001F
#define COL_FWHITE  0xFFFF
#define COL_EYE     0xFFFF
#define COL_PUPIL   0x0000
#define COL_SCR     0xFFFF
#define COL_LIFE    0xFFE0

// ============================================================================
// Maze data (24 x 28)
// ============================================================================
static const int8_t maze[MAZE_ROWS][MAZE_COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,1,1,2,1,1,1,1,2,1,1,2,1,1,1,1,2,1,1,2,1},
    {1,3,1,1,1,1,2,1,1,1,1,2,1,1,2,1,1,1,1,2,1,1,3,1},
    {1,2,1,1,1,1,2,1,1,1,1,2,1,1,2,1,1,1,1,2,1,1,2,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,1,1,2,1,1,2,1,1,1,1,1,1,2,1,1,2,1,1,2,1},
    {1,2,1,1,1,1,2,1,1,2,1,1,1,1,1,1,2,1,1,2,1,1,2,1},
    {1,2,2,2,2,2,2,1,1,2,2,2,1,1,2,2,2,1,1,2,2,2,2,1},
    {1,1,1,1,1,1,2,1,1,1,1,2,1,1,2,1,1,1,1,2,1,1,1,1},
    {0,0,0,0,0,1,2,1,1,1,1,2,1,1,2,1,1,1,1,2,1,0,0,0},
    {0,0,0,0,0,1,2,1,1,0,0,0,0,0,0,0,0,1,1,2,1,0,0,0},
    {0,0,0,0,0,1,2,1,1,0,1,1,5,1,1,1,0,1,1,2,1,0,0,0},
    {1,1,1,1,1,1,2,1,1,0,1,4,4,4,4,1,0,1,1,2,1,1,1,1},
    {2,2,2,2,2,2,2,2,2,0,1,4,4,4,4,1,0,2,2,2,2,2,2,2},
    {1,1,1,1,1,1,2,1,1,0,1,4,4,4,4,1,0,1,1,2,1,1,1,1},
    {0,0,0,0,0,1,2,1,1,0,0,0,0,0,0,0,0,1,1,2,1,0,0,0},
    {0,0,0,0,0,1,2,1,1,0,0,0,0,0,0,0,0,1,1,2,1,0,0,0},
    {0,0,0,0,0,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,0,0,0},
    {1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,1,1,2,1,1,1,1,2,1,1,2,1,1,1,1,2,1,1,2,1},
    {1,2,1,1,1,1,2,1,1,1,1,2,1,1,2,1,1,1,1,2,1,1,2,1},
    {1,3,2,2,1,1,2,2,2,2,2,0,0,0,0,2,2,2,2,2,1,1,3,1},
    {1,1,1,2,1,1,2,1,1,2,1,1,1,1,1,1,2,1,1,2,1,1,1,1},
    {1,1,1,2,1,1,2,1,1,2,1,1,1,1,1,1,2,1,1,2,1,1,1,1},
    {1,2,2,2,2,2,2,1,1,2,2,2,1,1,2,2,2,1,1,2,2,2,2,1},
    {1,2,1,1,1,1,1,1,1,1,1,2,1,1,2,1,1,1,1,1,1,1,2,1},
};

// ============================================================================
// Game state
// ============================================================================
volatile GameState_t g_game_state = GS_INIT;
uint16_t g_score = 0;
uint8_t g_lives = 3;

static Pacman_t pac;
static Ghost_t ghosts[4];
static uint32_t last_tick_ms = 0;
static uint32_t ghost_spawn_ms = 0;
static int8_t maze_state[MAZE_ROWS][MAZE_COLS];
static uint32_t power_start_ms = 0;
static int8_t power_active = 0;
static int32_t dots_total = 0;
static int32_t dots_eaten = 0;
static uint8_t ghost_eat_combo = 0;
static uint32_t last_button_ms = 0;
static uint32_t button_press_ms = 0;
static uint8_t long_press_handled = 0;
#define LONG_PRESS_MS 500
static uint8_t fright_blink_on = 0;
#define FRIGHT_BLINK_MS 2000

// Chase/Scatter mode cycling
static const uint32_t mode_durations_ms[] = {7000, 20000, 7000, 20000, 5000, 20000, 5000};
#define MODE_PHASES 7
static uint8_t mode_phase = 0;
static uint32_t mode_phase_start = 0;
static uint8_t ready_to_play = 0;
static uint32_t ready_start_ms = 0;

// ============================================================================
// Ghost scatter targets (corners)
// ============================================================================
// Blinky: top-right, Pinky: top-left, Inky: bottom-right, Clyde: bottom-left

// ============================================================================
// Helper: is walkable for pacman
// ============================================================================
static int8_t is_walkable(int8_t col, int8_t row) {
    if (col < 0 || col >= MAZE_COLS || row < 0 || row >= MAZE_ROWS) return 0;
    int8_t c = maze_state[row][col];
    return (c == CELL_EMPTY || c == CELL_DOT || c == CELL_POWER);
}

static int8_t is_walkable_ghost(int8_t col, int8_t row) {
    if (col < 0 || col >= MAZE_COLS || row < 0 || row >= MAZE_ROWS) return 0;
    int8_t c = maze_state[row][col];
    return (c != CELL_WALL);
}

// ============================================================================
// Drawing helpers
// ============================================================================
static void cell_rect(int8_t col, int8_t row, uint16_t *x, uint16_t *y) {
    *x = GAME_LEFT + col * CELL_SZ;
    *y = GAME_TOP + row * CELL_SZ;
}

static void draw_wall(int8_t col, int8_t row) {
    uint16_t x, y;
    cell_rect(col, row, &x, &y);
    LCD_FillRect(x, y, CELL_SZ, CELL_SZ, COL_WALL);
}

static void draw_dot(int8_t col, int8_t row) {
    uint16_t x, y;
    cell_rect(col, row, &x, &y);
    LCD_FillRect(x, y, CELL_SZ, CELL_SZ, COL_BG);
    LCD_FillRect(x + 4, y + 4, 2, 2, COL_DOT);
}

static void draw_power(int8_t col, int8_t row) {
    uint16_t x, y;
    cell_rect(col, row, &x, &y);
    LCD_FillRect(x, y, CELL_SZ, CELL_SZ, COL_BG);
    LCD_FillCircle(x + 5, y + 5, 4, COL_POWER);
}

static void draw_empty(int8_t col, int8_t row) {
    uint16_t x, y;
    cell_rect(col, row, &x, &y);
    LCD_FillRect(x, y, CELL_SZ, CELL_SZ, COL_BG);
}

static void draw_ghouse(int8_t col, int8_t row) {
    uint16_t x, y;
    cell_rect(col, row, &x, &y);
    LCD_FillRect(x, y, CELL_SZ, CELL_SZ, COL_BG);
    // Only draw outer borders (skip if neighbor is also ghost house)
    if (row == 0 || maze_state[row-1][col] != CELL_GHOUSE)
        LCD_FillRect(x + 1, y + 1, CELL_SZ - 2, 1, COL_WALL); // top
    if (row == MAZE_ROWS-1 || maze_state[row+1][col] != CELL_GHOUSE)
        LCD_FillRect(x + 1, y + CELL_SZ - 2, CELL_SZ - 2, 1, COL_WALL); // bottom
    if (col == 0 || maze_state[row][col-1] != CELL_GHOUSE)
        LCD_FillRect(x + 1, y + 1, 1, CELL_SZ - 2, COL_WALL); // left
    if (col == MAZE_COLS-1 || maze_state[row][col+1] != CELL_GHOUSE)
        LCD_FillRect(x + CELL_SZ - 2, y + 1, 1, CELL_SZ - 2, COL_WALL); // right
}

static void draw_gdoor(int8_t col, int8_t row) {
    uint16_t x, y;
    cell_rect(col, row, &x, &y);
    LCD_FillRect(x, y, CELL_SZ, CELL_SZ, COL_BG);
    LCD_FillRect(x + 1, y + 4, CELL_SZ - 2, 2, 0xF81F); // pink door
}

// ============================================================================
// Draw Pacman
// ============================================================================
static void draw_pacman(int8_t col, int8_t row, Dir_t dir, uint8_t mouth) {
    uint16_t x, y;
    cell_rect(col, row, &x, &y);
    int16_t cx = x + 5;
    int16_t cy = y + 5;

    LCD_FillRect(x, y, CELL_SZ, CELL_SZ, COL_BG);

    // Body circle
    LCD_FillCircle(cx, cy, 4, COL_PACMAN);

    if (mouth) {
        // Mouth direction vector
        int8_t sx = 0, sy = 0;
        switch (dir) {
            case DIR_R: sx = 1; break;
            case DIR_L: sx = -1; break;
            case DIR_U: sy = -1; break;
            case DIR_D: sy = 1; break;
            default: break;
        }
        // Erase triangular wedge from center to edge
        // half-width increases with distance from center
        for (int8_t d = 0; d <= 4; d++) {
            int nx = cx + sx * d;
            int ny = cy + sy * d;
            int hw = d * 3 / 5;  // wedge half-width
            int px = -sy * hw;
            int py = sx * hw;
            LCD_FillRect(nx - px, ny - py, 2 * abs(px) + 1, 2 * abs(py) + 1, COL_BG);
        }
    }

    // Eye (opposite mouth direction)
    if (dir == DIR_NONE) {
        LCD_FillCircle(cx + 1, cy - 2, 1, COL_PUPIL);
    } else {
        int8_t ex = 0, ey = 0;
        switch (dir) {
            case DIR_R: ex = 1; ey = -2; break;
            case DIR_L: ex = -1; ey = -2; break;
            case DIR_U: ex = 1; ey = -1; break;
            case DIR_D: ex = 1; ey = 1; break;
            default: break;
        }
        LCD_FillCircle(cx + ex, cy + ey, 1, COL_PUPIL);
    }
}

// ============================================================================
// Draw Ghost
// ============================================================================
static void draw_ghost(int8_t col, int8_t row, uint16_t color, Dir_t dir, GhostMode_t mode) {
    uint16_t x, y;
    cell_rect(col, row, &x, &y);
    int16_t cx = x + 5;
    int16_t cy = y + 5;

    // Clear cell
    LCD_FillRect(x, y, CELL_SZ, CELL_SZ, COL_BG);

    if (mode == GM_EATEN) {
        // Just draw eyes floating back to house
        LCD_FillCircle(cx - 2, cy - 2, 1, COL_EYE);
        LCD_FillCircle(cx + 2, cy - 2, 1, COL_EYE);
        return;
    }

    uint16_t body_color = color;
    if (mode == GM_FRIGHT) {
        body_color = fright_blink_on ? COL_FWHITE : COL_FRIGHT;
    }

    // Ghost body (rounded top + body) — stays within cell (y to y+9)
    LCD_FillCircle(cx, cy - 1, 4, body_color);  // center (cx, y+4), radius 4 → top y, bottom y+8
    LCD_FillRect(cx - 4, cy - 1, 8, 4, body_color); // y+4 to y+7

    // Wavy bottom skirt
    LCD_FillRect(cx - 4, cy + 3, 2, 2, body_color); // y+8
    LCD_FillRect(cx - 1, cy + 3, 2, 2, body_color); // y+8
    LCD_FillRect(cx + 2, cy + 3, 2, 2, body_color); // y+8

    if (mode == GM_FRIGHT) {
        // Frightened face
        LCD_FillCircle(cx - 2, cy - 2, 1, COL_FWHITE);
        LCD_FillCircle(cx + 2, cy - 2, 1, COL_FWHITE);
        LCD_FillCircle(cx - 2, cy - 2, 1, COL_PUPIL);
        LCD_FillCircle(cx + 2, cy - 2, 1, COL_PUPIL);
        // Wavy mouth
        LCD_FillRect(cx - 2, cy + 1, 1, 1, COL_FWHITE);
        LCD_FillRect(cx, cy, 1, 1, COL_FWHITE);
        LCD_FillRect(cx + 2, cy + 1, 1, 1, COL_FWHITE);
    } else {
        // Normal eyes
        LCD_FillCircle(cx - 2, cy - 2, 2, COL_EYE);
        LCD_FillCircle(cx + 2, cy - 2, 2, COL_EYE);
        // Pupils look in direction
        int8_t px = 0, py = 0;
        switch (dir) {
            case DIR_R: px = 1; break;
            case DIR_L: px = -1; break;
            case DIR_U: py = -1; break;
            case DIR_D: py = 1; break;
            default: break;
        }
        LCD_FillCircle(cx - 2 + px, cy - 2 + py, 1, COL_PUPIL);
        LCD_FillCircle(cx + 2 + px, cy - 2 + py, 1, COL_PUPIL);
    }
}

// ============================================================================
// Draw maze
// ============================================================================
static void draw_maze_cell(int8_t col, int8_t row) {
    int8_t c = maze_state[row][col];
    switch (c) {
        case CELL_WALL:   draw_wall(col, row); break;
        case CELL_DOT:    draw_dot(col, row); break;
        case CELL_POWER:  draw_power(col, row); break;
        case CELL_EMPTY:  draw_empty(col, row); break;
        case CELL_GHOUSE: draw_ghouse(col, row); break;
        case CELL_GDOOR:  draw_gdoor(col, row); break;
        default: break;
    }
}

static void draw_maze_full(void) {
    for (int8_t r = 0; r < MAZE_ROWS; r++)
        for (int8_t c = 0; c < MAZE_COLS; c++)
            draw_maze_cell(c, r);
}

// ============================================================================
// HUD
// ============================================================================
static void draw_hud(void) {
    LCD_FillRect(0, 0, 240, 20, COL_BG);
    LCD_FillRect(0, 300, 240, 20, COL_BG);

    // Score
    char buf[12];
    int s = g_score;
    uint8_t len = 0;
    if (s == 0) { buf[0] = '0'; len = 1; }
    else {
        uint8_t i = 0;
        while (s > 0 && i < 10) {
            buf[i++] = '0' + (s % 10);
            s /= 10;
        }
        len = i;
        // Reverse
        for (uint8_t j = 0; j < len / 2; j++) {
            char t = buf[j];
            buf[j] = buf[len - 1 - j];
            buf[len - 1 - j] = t;
        }
    }
    buf[len] = '\0';

    // Simple draw score digits
    int16_t sx = 10;
    for (uint8_t i = 0; i < len; i++) {
        // Draw digit using rectangles (simplified 3x5 font)
        uint8_t ch = buf[i] - '0';
        static const uint8_t digits[10][5] = {
            {0x06,0x09,0x09,0x09,0x06}, // 0
            {0x02,0x06,0x02,0x02,0x07}, // 1
            {0x06,0x09,0x02,0x04,0x0F}, // 2
            {0x06,0x09,0x02,0x09,0x06}, // 3
            {0x02,0x06,0x0A,0x0F,0x02}, // 4
            {0x0F,0x08,0x0E,0x01,0x0E}, // 5
            {0x06,0x08,0x0E,0x09,0x06}, // 6
            {0x0F,0x01,0x02,0x04,0x04}, // 7
            {0x06,0x09,0x06,0x09,0x06}, // 8
            {0x06,0x09,0x07,0x01,0x06}, // 9
        };
        for (uint8_t row = 0; row < 5; row++) {
            for (uint8_t col = 0; col < 4; col++) {
                if (digits[ch][row] & (0x08 >> col)) {
                    LCD_FillRect(sx + col * 2, 5 + row * 3, 2, 2, COL_SCR);
                }
            }
        }
        sx += 14;
    }

    // Lives (small Pacmans)
    for (uint8_t i = 0; i < g_lives; i++) {
        LCD_FillCircle(200 + i * 14, 310, 5, COL_LIFE);
    }
}

// Draw text string using 4x5 pixel font
static void draw_text(int16_t x, int16_t y, const char *str, uint16_t color) {
    static const uint8_t chars[26][5] = {
        {0x06,0x09,0x0F,0x09,0x09}, // A
        {0x0E,0x09,0x0E,0x09,0x0E}, // B
        {0x06,0x09,0x08,0x09,0x06}, // C
        {0x0E,0x09,0x09,0x09,0x0E}, // D
        {0x0F,0x08,0x0E,0x08,0x0F}, // E
        {0x0F,0x08,0x0E,0x08,0x08}, // F
        {0x06,0x09,0x0B,0x09,0x06}, // G
        {0x09,0x09,0x0F,0x09,0x09}, // H
        {0x07,0x02,0x02,0x02,0x07}, // I
        {0x01,0x01,0x01,0x09,0x06}, // J
        {0x09,0x0A,0x0C,0x0A,0x09}, // K
        {0x08,0x08,0x08,0x08,0x0F}, // L
        {0x09,0x0F,0x0F,0x09,0x09}, // M
        {0x09,0x0D,0x0B,0x09,0x09}, // N
        {0x06,0x09,0x09,0x09,0x06}, // O
        {0x0E,0x09,0x0E,0x08,0x08}, // P
        {0x06,0x09,0x09,0x09,0x06}, // Q (same as O)
        {0x0E,0x09,0x0E,0x0A,0x09}, // R
        {0x07,0x08,0x07,0x01,0x0E}, // S
        {0x0F,0x02,0x02,0x02,0x02}, // T
        {0x09,0x09,0x09,0x09,0x06}, // U
        {0x09,0x05,0x05,0x02,0x02}, // V
        {0x09,0x09,0x0B,0x0D,0x09}, // W
        {0x09,0x09,0x06,0x09,0x09}, // X
        {0x09,0x09,0x06,0x02,0x02}, // Y
        {0x0F,0x01,0x02,0x04,0x0F}, // Z
    };
    while (*str) {
        uint8_t ch = *str - 'A';
        if (*str == ' ') {
            x += 16;
            str++;
            continue;
        }
        if (*str == '!') {
            // Exclamation mark
            static const uint8_t excl[5] = {0x04,0x04,0x04,0x00,0x04};
            for (uint8_t row = 0; row < 5; row++) {
                for (uint8_t col = 0; col < 4; col++) {
                    if (excl[row] & (0x08 >> col))
                        LCD_FillRect(x + col * 2, y + row * 3, 2, 2, color);
                }
            }
            x += 12;
            str++;
            continue;
        }
        if (ch <= 25) {
            for (uint8_t row = 0; row < 5; row++) {
                for (uint8_t col = 0; col < 4; col++) {
                    if (chars[ch][row] & (0x08 >> col))
                        LCD_FillRect(x + col * 2, y + row * 3, 2, 2, color);
                }
            }
        }
        x += 14;
        str++;
    }
}
static Dir_t next_dir_cycle(Dir_t current) {
    switch (current) {
        case DIR_NONE: return DIR_R;
        case DIR_R: return DIR_D;
        case DIR_D: return DIR_L;
        case DIR_L: return DIR_U;
        case DIR_U: return DIR_R;
        default: return DIR_R;
    }
}

// ============================================================================
// Tunnel wrap
// ============================================================================
static int8_t is_tunnel_row(int8_t row) {
    return (row >= 10 && row <= 18);
}

static void handle_tunnel(int8_t *col, int8_t row) {
    if (!is_tunnel_row(row)) return;
    // Left tunnel: col 5 with wall at col 4, wrap from col 0-4 to col 23-19
    // Right tunnel: col 18 with wall at col 19, wrap from col 20-23 to col 3-0
    // Check maze data for tunnel openings
    if (*col < 0 && maze_state[row][0] == CELL_EMPTY) {
        // Find rightmost open cell
        for (int8_t c = MAZE_COLS - 1; c >= 0; c--) {
            if (maze_state[row][c] == CELL_EMPTY || maze_state[row][c] == CELL_DOT || maze_state[row][c] == CELL_POWER) {
                *col = c;
                break;
            }
        }
    } else if (*col >= MAZE_COLS && maze_state[row][MAZE_COLS - 1] == CELL_EMPTY) {
        for (int8_t c = 0; c < MAZE_COLS; c++) {
            if (maze_state[row][c] == CELL_EMPTY || maze_state[row][c] == CELL_DOT || maze_state[row][c] == CELL_POWER) {
                *col = c;
                break;
            }
        }
    }
}

// ============================================================================
// Pacman movement
// ============================================================================
static void move_pacman(void) {
    if (pac.move_counter > 0) { pac.move_counter--; return; }
    pac.move_counter = 0;

    // Try queued direction first
    if (pac.next_dir != DIR_NONE && pac.next_dir != pac.dir) {
        int8_t nc = pac.col, nr = pac.row;
        switch (pac.next_dir) {
            case DIR_R: nc++; break;
            case DIR_L: nc--; break;
            case DIR_U: nr--; break;
            case DIR_D: nr++; break;
            default: break;
        }
        handle_tunnel(&nc, pac.row);
        if (is_walkable(nc, nr)) {
            pac.dir = pac.next_dir;
            pac.next_dir = DIR_NONE;
        }
    }

    // Move in current direction
    int8_t nc = pac.col, nr = pac.row;
    switch (pac.dir) {
        case DIR_R: nc++; break;
        case DIR_L: nc--; break;
        case DIR_U: nr--; break;
        case DIR_D: nr++; break;
        default: return;
    }

    handle_tunnel(&nc, pac.row);

    if (is_walkable(nc, nr)) {
        // Clear old cell
        draw_maze_cell(pac.col, pac.row);
        pac.col = nc;
        pac.row = nr;
        pac.mouth_frame = !pac.mouth_frame;

        // Eat dot
        if (maze_state[pac.row][pac.col] == CELL_DOT) {
            maze_state[pac.row][pac.col] = CELL_EMPTY;
            g_score += DOT_SCORE;
            dots_eaten++;
        } else if (maze_state[pac.row][pac.col] == CELL_POWER) {
            maze_state[pac.row][pac.col] = CELL_EMPTY;
            g_score += POWER_SCORE;
            dots_eaten++;
            // Frighten ghosts
            power_active = 1;
            power_start_ms = HAL_GetTick();
            ghost_eat_combo = 0;
            for (int i = 0; i < 4; i++) {
                if (ghosts[i].mode == GM_CHASE || ghosts[i].mode == GM_SCATTER) {
                    ghosts[i].mode = GM_FRIGHT;
                    ghosts[i].fright_remain_ms = FRIGHT_MS;
                    // Reverse direction
                    switch (ghosts[i].dir) {
                        case DIR_R: ghosts[i].dir = DIR_L; break;
                        case DIR_L: ghosts[i].dir = DIR_R; break;
                        case DIR_U: ghosts[i].dir = DIR_D; break;
                        case DIR_D: ghosts[i].dir = DIR_U; break;
                        default: break;
                    }
                }
            }
        }
    } else {
        // Can't move - stop
        pac.dir = DIR_NONE;
    }
}

// ============================================================================
// Ghost AI helpers
// ============================================================================
static Dir_t opposite_dir(Dir_t d) {
    switch (d) {
        case DIR_R: return DIR_L;
        case DIR_L: return DIR_R;
        case DIR_U: return DIR_D;
        case DIR_D: return DIR_U;
        default: return DIR_NONE;
    }
}

static int8_t ghost_can_move(int8_t col, int8_t row, Dir_t dir) {
    int8_t nc = col, nr = row;
    switch (dir) {
        case DIR_R: nc++; break;
        case DIR_L: nc--; break;
        case DIR_U: nr--; break;
        case DIR_D: nr++; break;
        default: return 0;
    }
    handle_tunnel(&nc, nr);
    if (nc < 0 || nc >= MAZE_COLS || nr < 0 || nr >= MAZE_ROWS) return 0;
    return is_walkable_ghost(nc, nr);
}

// ============================================================================
// Ghost movement
// ============================================================================
static void move_ghost(Ghost_t *g) {
    // Don't move if eaten (wait to respawn)
    if (g->mode == GM_EATEN) return;

    // In house: wait then leave
    if (g->in_house) {
        // Ghosts leave one by one
        return;
    }

    // Choose direction at cell center
    if (g->col < 0 || g->col >= MAZE_COLS || g->row < 0 || g->row >= MAZE_ROWS) return;

    // Count available directions
    Dir_t choices[4];
    uint8_t n = 0;
    for (Dir_t d = DIR_R; d <= DIR_U; d++) {
        if (d == opposite_dir(g->dir)) continue; // Can't reverse
        if (ghost_can_move(g->col, g->row, d)) {
            choices[n++] = d;
        }
    }

    if (n == 0) {
        // Dead end, reverse
        g->dir = opposite_dir(g->dir);
        return;
    }

    // Pick best direction based on mode
    Dir_t best_dir = choices[0];
    if (g->mode == GM_FRIGHT) {
        // Random direction
        best_dir = choices[rand() % n];
    } else {
        // Chase or scatter: pick direction closest to target
        int8_t target_col = g->scatter_col;
        int8_t target_row = g->scatter_row;

        if (g->mode == GM_CHASE) {
            int8_t idx = (int8_t)(g - ghosts);
            switch (idx) {
                case 0: // Blinky: direct chase
                    target_col = pac.col;
                    target_row = pac.row;
                    break;
                case 1: // Pinky: 4 cells ahead of Pacman
                    target_col = pac.col;
                    target_row = pac.row;
                    switch (pac.dir) {
                        case DIR_R: target_col += 4; break;
                        case DIR_L: target_col -= 4; break;
                        case DIR_U: target_row -= 4; break;
                        case DIR_D: target_row += 4; break;
                        default: break;
                    }
                    break;
                case 2: // Inky: double vector from Blinky to 2 cells ahead
                {
                    int8_t ac = pac.col, ar = pac.row;
                    switch (pac.dir) {
                        case DIR_R: ac += 2; break;
                        case DIR_L: ac -= 2; break;
                        case DIR_U: ar -= 2; break;
                        case DIR_D: ar += 2; break;
                        default: break;
                    }
                    target_col = 2 * ac - ghosts[0].col;
                    target_row = 2 * ar - ghosts[0].row;
                    break;
                }
                case 3: // Clyde: chase if far, scatter if close
                {
                    int32_t d2 = (g->col - pac.col) * (g->col - pac.col)
                               + (g->row - pac.row) * (g->row - pac.row);
                    if (d2 > 64) {
                        target_col = pac.col;
                        target_row = pac.row;
                    } else {
                        target_col = g->scatter_col;
                        target_row = g->scatter_row;
                    }
                    break;
                }
            }
        }

        int32_t best_dist = 99999;
        for (uint8_t i = 0; i < n; i++) {
            int8_t nc = g->col, nr = g->row;
            switch (choices[i]) {
                case DIR_R: nc++; break;
                case DIR_L: nc--; break;
                case DIR_U: nr--; break;
                case DIR_D: nr++; break;
                default: break;
            }
            int32_t d = (nc - target_col) * (nc - target_col) + (nr - target_row) * (nr - target_row);
            if (d < best_dist) {
                best_dist = d;
                best_dir = choices[i];
            }
        }
    }

    // Save old position for redraw
    int8_t old_col = g->col;
    int8_t old_row = g->row;

    g->dir = best_dir;
    switch (g->dir) {
        case DIR_R: g->col++; break;
        case DIR_L: g->col--; break;
        case DIR_U: g->row--; break;
        case DIR_D: g->row++; break;
        default: break;
    }
    handle_tunnel(&g->col, g->row);

    // Redraw old cell
    draw_maze_cell(old_col, old_row);
}

// ============================================================================
// Collision detection
// ============================================================================
static void check_collisions(void) {
    for (int i = 0; i < 4; i++) {
        if (ghosts[i].in_house || ghosts[i].mode == GM_EATEN) continue;
        if (ghosts[i].col == pac.col && ghosts[i].row == pac.row) {
            if (ghosts[i].mode == GM_FRIGHT) {
                // Eat ghost
                ghosts[i].mode = GM_EATEN;
                ghost_eat_combo++;
                g_score += GHOST_SCORE_BASE * ghost_eat_combo;
                // Ghost will respawn at house later
            } else {
                // Pacman dies
                g_lives--;
                draw_hud();
                if (g_lives == 0) {
                    g_game_state = GS_GAMEOVER;
                } else {
                    // Reset positions
                    pac.col = 12;
                    pac.row = 23;
                    pac.dir = DIR_NONE;
                    pac.next_dir = DIR_NONE;
                    HAL_Delay(1000);
                    // Reset ghosts
                    for (int j = 0; j < 4; j++) {
                        ghosts[j].mode = GM_CHASE;
                    }
                    ghosts[0].col = 14; ghosts[0].row = 11;
                    ghosts[1].col = 12; ghosts[1].row = 13;
                    ghosts[2].col = 13; ghosts[2].row = 13;
                    ghosts[3].col = 14; ghosts[3].row = 13;
                    ghosts[0].dir = DIR_L; ghosts[1].dir = DIR_NONE;
                    ghosts[2].dir = DIR_NONE; ghosts[3].dir = DIR_NONE;

                    // Redraw
                    draw_maze_full();
                    draw_pacman(pac.col, pac.row, pac.dir, 1);
                    for (int j = 0; j < 4; j++) {
                        if (!ghosts[j].in_house)
                            draw_ghost(ghosts[j].col, ghosts[j].row, ghosts[j].color, ghosts[j].dir, ghosts[j].mode);
                    }
                }
            }
        }
    }
}

// ============================================================================
// Respawn eaten ghost
// ============================================================================
static void respawn_ghosts(void) {
    static uint32_t respawn_timer = 0;
    if (HAL_GetTick() - respawn_timer < 2000) return;

    for (int i = 0; i < 4; i++) {
        if (ghosts[i].mode == GM_EATEN) {
            ghosts[i].col = 11 + i;
            ghosts[i].row = 13;
            ghosts[i].mode = GM_CHASE;
            ghosts[i].dir = DIR_NONE;
            ghosts[i].in_house = 1;
            respawn_timer = HAL_GetTick();
            break;
        }
    }
}

// ============================================================================
// Release ghosts from house
// ============================================================================
static void release_ghosts(void) {
    static uint32_t last_release = 0;
    uint32_t now = HAL_GetTick();

    for (int i = 0; i < 4; i++) {
        if (ghosts[i].in_house && now - last_release > 4000 + i * 3000) {
        // Place ghost at door position; it will exit via door (12,12) → corridor
        ghosts[i].col = 12;
        ghosts[i].row = 13;  // inside ghost house, below door
        ghosts[i].dir = DIR_U;
        ghosts[i].in_house = 0;
            ghosts[i].mode = GM_CHASE;
            last_release = now;
            break;
        }
    }
}

// ============================================================================
// Check win
// ============================================================================
static void check_win(void) {
    if (dots_eaten >= dots_total) {
        g_game_state = GS_WIN;
    }
}

// ============================================================================
// Public: Init
// ============================================================================
void Packman_Init(void) {
    // Copy maze
    for (int r = 0; r < MAZE_ROWS; r++)
        for (int c = 0; c < MAZE_COLS; c++)
            maze_state[r][c] = maze[r][c];

    // Count dots
    dots_total = 0;
    dots_eaten = 0;
    for (int r = 0; r < MAZE_ROWS; r++)
        for (int c = 0; c < MAZE_COLS; c++)
            if (maze_state[r][c] == CELL_DOT || maze_state[r][c] == CELL_POWER)
                dots_total++;

    // Init Pacman
    pac.col = 11;
    pac.row = 23;
    pac.dir = DIR_R;
    pac.next_dir = DIR_NONE;
    pac.mouth_frame = 0;
    pac.move_counter = 0;

    // Init ghosts
    ghosts[0].color = COL_GHOST_R;  // Blinky - red
    ghosts[1].color = COL_GHOST_P;  // Pinky - pink
    ghosts[2].color = COL_GHOST_C;  // Inky - cyan
    ghosts[3].color = COL_GHOST_O;  // Clyde - orange

    // Ghost house occupies cols 11-14, rows 13-15 (CELL_GHOUSE)
    for (int i = 0; i < 4; i++) {
        ghosts[i].col = 11 + i;
        ghosts[i].row = 13;
        ghosts[i].dir = DIR_NONE;
        ghosts[i].in_house = 1;
        ghosts[i].mode = GM_CHASE;
        ghosts[i].fright_remain_ms = 0;
        ghosts[i].scatter_col = (i == 0 || i == 2) ? MAZE_COLS - 2 : 1;
        ghosts[i].scatter_row = (i == 0 || i == 1) ? 1 : MAZE_ROWS - 2;
    }
    // Blinky starts outside in corridor above ghost house
    ghosts[0].in_house = 0;
    ghosts[0].col = 14;
    ghosts[0].row = 11;
    ghosts[0].dir = DIR_L;

    g_score = 0;
    g_lives = 3;
    power_active = 0;

    // Clear screen
    LCD_FillRect(0, 0, 240, 320, COL_BG);

    // Draw maze
    draw_maze_full();

    // Draw HUD
    draw_hud();

    // Draw initial positions
    draw_pacman(pac.col, pac.row, pac.dir, 1);
    for (int i = 0; i < 4; i++) {
        if (!ghosts[i].in_house)
            draw_ghost(ghosts[i].col, ghosts[i].row, ghosts[i].color, ghosts[i].dir, ghosts[i].mode);
    }

    mode_phase = 0;
    mode_phase_start = HAL_GetTick();

    last_tick_ms = HAL_GetTick();
    ghost_spawn_ms = HAL_GetTick();
    last_button_ms = HAL_GetTick();

    g_game_state = GS_PLAY;
    ready_to_play = 0;
    ready_start_ms = HAL_GetTick();
    draw_text(70, 142, "READY", COL_POWER);
}

// ============================================================================
// Public: Tick (call every ~TICK_MS)
// ============================================================================
void Packman_Tick(void) {
    if (g_game_state == GS_GAMEOVER) {
        draw_text(70, 100, "GAME OVER", 0xF800);
        return;
    }
    if (g_game_state == GS_WIN) {
        draw_text(70, 100, "YOU WIN", COL_POWER);
        draw_text(70, 120, "PRESS KEY", COL_SCR);
        return;
    }

    uint32_t now = HAL_GetTick();

    // Long press detection (works in both PLAY and PAUSE)
    if (button_press_ms && !long_press_handled && now - button_press_ms > LONG_PRESS_MS) {
        long_press_handled = 1;
        if (g_game_state == GS_PLAY) {
            draw_text(70, 142, "PAUSE", COL_SCR);
            g_game_state = GS_PAUSE;
            return;
        } else if (g_game_state == GS_PAUSE) {
            LCD_FillRect(70, 140, 80, 20, COL_BG);
            g_game_state = GS_PLAY;
            // Reset mode phase timer to avoid time jump
            mode_phase_start = now;
            return;
        }
    }

    if (g_game_state == GS_PAUSE) return;
    if (g_game_state != GS_PLAY) return;

    // Show READY! message for 2 seconds before starting play
    if (!ready_to_play) {
        if (HAL_GetTick() - ready_start_ms > 2000) {
            // Restore maze cells covered by READY text (rows 12-13, cols 0-23)
            for (int c = 0; c < MAZE_COLS; c++) {
                draw_maze_cell(c, 12);
                draw_maze_cell(c, 13);
            }
            // Restore ghosts in those rows too
            for (int i = 0; i < 4; i++) {
                if (!ghosts[i].in_house)
                    draw_ghost(ghosts[i].col, ghosts[i].row, ghosts[i].color, ghosts[i].dir, ghosts[i].mode);
            }
            ready_to_play = 1;
        }
        return;
    }

    // Move pacman
    move_pacman();

    // Move ghosts (every other tick for slower speed)
    static uint8_t ghost_tick = 0;
    ghost_tick = !ghost_tick;
    if (ghost_tick == 0) {
        for (int i = 0; i < 4; i++) {
            move_ghost(&ghosts[i]);
        }
    }

    // Redraw pacman
    draw_pacman(pac.col, pac.row, pac.dir, pac.mouth_frame);

    // Redraw ghosts
    for (int i = 0; i < 4; i++) {
        if (!ghosts[i].in_house)
            draw_ghost(ghosts[i].col, ghosts[i].row, ghosts[i].color, ghosts[i].dir, ghosts[i].mode);
    }

    // Check collisions
    check_collisions();

    // Check fright timer
    if (power_active) {
        uint32_t fright_elapsed = now - power_start_ms;
        if (fright_elapsed > FRIGHT_MS) {
            power_active = 0;
            for (int i = 0; i < 4; i++) {
                if (ghosts[i].mode == GM_FRIGHT) {
                    ghosts[i].mode = GM_CHASE;
                }
            }
        } else if (fright_elapsed > FRIGHT_MS - FRIGHT_BLINK_MS) {
            // Blink every tick (160ms) during last 2 seconds
            fright_blink_on = !fright_blink_on;
        } else {
            fright_blink_on = 0;
        }
    }

    // Respawn eaten ghosts
    respawn_ghosts();

    // Release ghosts from house
    release_ghosts();

    // Update Chase/Scatter mode cycling
    {
        uint32_t elapsed = now - mode_phase_start;
        uint32_t dur = (mode_phase < MODE_PHASES) ? mode_durations_ms[mode_phase] : 20000;
        if (elapsed >= dur) {
            mode_phase++;
            mode_phase_start = now;
            uint8_t new_scatter = (mode_phase % 2 == 0);
            for (int i = 0; i < 4; i++) {
                if (ghosts[i].in_house || ghosts[i].mode == GM_FRIGHT || ghosts[i].mode == GM_EATEN) continue;
                ghosts[i].mode = new_scatter ? GM_SCATTER : GM_CHASE;
                // Reverse direction
                switch (ghosts[i].dir) {
                    case DIR_R: ghosts[i].dir = DIR_L; break;
                    case DIR_L: ghosts[i].dir = DIR_R; break;
                    case DIR_U: ghosts[i].dir = DIR_D; break;
                    case DIR_D: ghosts[i].dir = DIR_U; break;
                    default: break;
                }
            }
        }
    }

    // Check win
    check_win();

    // Update HUD
    draw_hud();

    last_tick_ms = now;
}

// ============================================================================
// Public: Button press (short = cycle direction, long = pause)
// ============================================================================
void Packman_OnButton(void) {
    uint32_t now = HAL_GetTick();
    if (now - last_button_ms < 200) return; // debounce
    last_button_ms = now;

    if (g_game_state == GS_GAMEOVER || g_game_state == GS_WIN) {
        Packman_Init();
        return;
    }

    if (g_game_state == GS_PLAY || g_game_state == GS_PAUSE) {
        button_press_ms = now;
        long_press_handled = 0;
    }

    if (g_game_state == GS_PLAY) {
        // Short press: cycle direction
        if (pac.dir == DIR_NONE) {
    pac.dir = DIR_L;
        } else {
            pac.next_dir = next_dir_cycle(pac.dir);
        }
    }
}
