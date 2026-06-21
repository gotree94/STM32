#ifndef INC_PACKMAN_H_
#define INC_PACKMAN_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdlib.h>

#define MAZE_COLS   24
#define MAZE_ROWS   28
#define CELL_SZ     10

#define GAME_LEFT   0
#define GAME_TOP    20
#define GAME_W      (MAZE_COLS * CELL_SZ)
#define GAME_H      (MAZE_ROWS * CELL_SZ)

#define TICK_MS         160
#define FRIGHT_MS       6000

#define CELL_EMPTY   0
#define CELL_WALL    1
#define CELL_DOT     2
#define CELL_POWER   3
#define CELL_GHOUSE  4
#define CELL_GDOOR   5

#define DOT_SCORE    10
#define POWER_SCORE  50
#define GHOST_SCORE_BASE 200

typedef enum { DIR_NONE = 0, DIR_R = 1, DIR_D = 2, DIR_L = 3, DIR_U = 4 } Dir_t;
typedef enum { GS_INIT, GS_PLAY, GS_PAUSE, GS_GAMEOVER, GS_WIN } GameState_t;
typedef enum { GM_CHASE, GM_SCATTER, GM_FRIGHT, GM_EATEN } GhostMode_t;

typedef struct {
    int8_t col, row;
    Dir_t dir;
    Dir_t next_dir;
    uint8_t mouth_frame;
    uint8_t move_counter;
} Pacman_t;

typedef struct {
    int8_t col, row;
    Dir_t dir;
    uint8_t in_house;
    GhostMode_t mode;
    uint16_t color;
    int32_t fright_remain_ms;
    int8_t scatter_col, scatter_row;
} Ghost_t;

extern volatile GameState_t g_game_state;
extern uint16_t g_score;
extern uint8_t g_lives;

void Packman_Init(void);
void Packman_Tick(void);
void Packman_OnButton(void);

#endif
