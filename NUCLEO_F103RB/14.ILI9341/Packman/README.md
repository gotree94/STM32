# PAC-MAN Game — STM32F103 + ILI9341 LCD (240x320)

## 개요

STM32F103 (Blue Pill / NUCLEO-F103RB) 에 8비트 병렬 인터페이스로 연결된 ILI9341 TFT LCD (240x320) 에서 동작하는 PAC-MAN 게임.

원래는 로봇 눈 표정 애니메이션(눈 깜빡임, 감정 표현)을 출력하던 프로젝트를 게임으로 전환.

## 하드웨어 구성

| 핀 | 포트 | 기능 |
|---|---|---|
| PA0 | GPIOA | LCD RD (Read) |
| PA1 | GPIOA | LCD WR (Write) |
| PA4 | GPIOA | LCD RS (Command/Data) |
| PB0 | GPIOB | LCD CS (Chip Select) |
| PC1 | GPIOC | LCD RST (Reset) |
| PA8 | GPIOA | LCD D7 (Data bit 7) |
| PA9 | GPIOA | LCD D0 (Data bit 0) |
| PA10 | GPIOA | LCD D2 (Data bit 2) |
| PB3 | GPIOB | LCD D3 (Data bit 3) |
| PB4 | GPIOB | LCD D5 (Data bit 5) |
| PB5 | GPIOB | LCD D4 (Data bit 4) |
| PB10 | GPIOB | LCD D6 (Data bit 6) |
| PC7 | GPIOC | LCD D1 (Data bit 1) |
| PC13 | GPIOC | Button B1 (USER button, Pull-Up) |

- LCD: ILI9341 240x320, 8비트 8080 병렬 인터페이스
- 클록: HSI 8MHz → PLL x16 → 64MHz SYSCLK

## 게임 구조

### 파일 구성

```
Core/
├── Inc/
│   ├── main.h          (변경 없음)
│   ├── Ili9341.h       (변경 없음)
│   └── packman.h       (신규) 게임 상수/타입/함수 선언
└── Src/
    ├── main.c          (수정) 게임 루프 + 버튼 입력
    ├── ili9341.c       (변경 없음)
    └── packman.c       (신규) 전체 게임 로직 및 고속 LCD 드로잉
```

### 게임 데이터

- **그리드**: 24열 × 28행, 셀 크기 10px
- **게임 영역**: (0, 20) ~ (239, 299) — 240×280px
- **HUD**: 상단 20px (점수), 하단 20px (목숨)
- **미로 데이터**: 정적 2차원 배열 (`static const int8_t maze[28][24]`), 실행 중에는 `maze_state`로 복사하여 변경

### 미로 셀 타입

| 값 | 타입 | 설명 |
|---|---|---|
| 0 | CELL_EMPTY | 빈 통로 |
| 1 | CELL_WALL | 벽 (파란색) |
| 2 | CELL_DOT | 점 (노란색, 10점) |
| 3 | CELL_POWER | 파워 펠렛 (노란색 원, 50점) |
| 4 | CELL_GHOUSE | 유령의 집 내부 바닥 |
| 5 | CELL_GDOOR | 유령의 집 출입문 (분홍색) |

### 게임 객체

**Pacman (`Pacman_t`)**
- 위치 (col, row): 그리드 좌표
- 현재 방향, 다음 방향 (큐)
- 입 애니메이션 프레임

**Ghost (`Ghost_t`)**
- 4마리: Blinky(빨강), Pinky(분홍), Inky(청록), Clyde(주황)
- 모드: Chase / Scatter / Frightened / Eaten
- 집 안/밖 플래그
- 산란 목표 좌표 (모드별 타겟)

### 입력 처리

- **PC13 버튼** (Pull-Up, Active Low)
- **짧은 누름 ( < 500ms)**: 방향 순환 (없음 → 오른쪽 → 아래 → 왼쪽 → 위 → 반복)
- **게임 오버/승리 상태**: 버튼 누르면 재시작
- 디바운스: 200ms

### 게임 로직 (Packman_Tick)

매 틱(160ms) 마다 실행:

1. `move_pacman()` — 다음 방향 시도 → 현재 방향 이동 → 점/파워 먹기
2. `move_ghost()` — 유령 방향 선택 (타겟 기반) → 이동
3. `draw_pacman()` / `draw_ghost()` — 화면 갱신
4. `check_collisions()` — 팩맨/유령 충돌 검사
5. 파워 모드 타이머 체크 (6초)
6. `respawn_ghosts()` / `release_ghosts()` — 리스폰/출소
7. `check_win()` — 승리 조건 검사
8. `draw_hud()` — 점수/목숨 갱신

### 유령 AI

- 교차로에서 타겟까지의 유클리드 거리 제곱이 가장 작은 방향 선택
- 후진 불가 (단, Frightened 진입 시 강제 반전)
- Frightened 모드: 랜덤 방향 선택
- Eaten 모드: 움직이지 않음 → 2초 후 집으로 리스폰

### 고속 LCD 드로잉

GPIO 레지스터 (BSRR/BRR) 직접 접근 방식:

```c
GPIOA->BSRR = pa_set | (pa_clr << 16);
GPIOB->BSRR = pb_set | (pb_clr << 16);
GPIOC->BSRR = pc_set | (pc_clr << 16);
```

- FillRect: 8회 언롤링 루프로 고속 전송
- FillCircle: 수평선 스캔라인 방식
- 동일한 드로잉 코드가 `main.c` (LCD 초기화)와 `packman.c` (게임 렌더링)에 각각 static 으로 존재

## 빌드

STM32CubeIDE 에서 Debug/Release 구성:

1. 프로젝트 열기
2. `Core/Inc/packman.h` 및 `Core/Src/packman.c` 자동 인식 (sourceEntries 에 `Core` 포함)
3. 빌드 (Ctrl+B)

## 알려진 문제 / Todo

- [ ] 유령 AI 개선 — Pinky/Inky/Clyde 각각 다른 타겟팅
- [ ] 유령 집 출입 애니메이션 (문 통과)
- [ ] 팩맨 입 애니메이션 개선 (삼각형 마스크)
- [ ] 게임오버/승리 메시지 텍스트 출력
- [ ] 파워 모드 종료 임박 시 유령 깜빡임
- [ ] 난이도 증가 (레벨 전환)
- [ ] 사운드 출력 (버저)

## 이력

- 2025-09-17: 프로젝트 생성 (CubeMX, ILI9341 초기화)
- 2025-xx: 로봇 눈 표정 애니메이션 구현
- 2026-06-22: PAC-MAN 게임으로 전환 (packman.h/c 추가, main.c 수정)
