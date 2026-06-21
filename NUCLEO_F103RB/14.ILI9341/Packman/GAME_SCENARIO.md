# PAC-MAN Game Scenario — STM32F103 + ILI9341

## 1. 전원 온 → 게임 시작 시퀀스

```
[전원 ON]
    │
    ▼
HAL_Init / SystemClock_Config (64MHz)
    │
    ▼
MX_GPIO_Init (LCD 핀 + 버튼 PC13)
    │
    ▼
LCD_Init (ILI9341 초기화, 화면 전체 검정)
    │
    ▼
Packman_Init()
    │
    ├── maze[][] → maze_state[][] 복사
    ├── dots_total 계산 (dot + power pellet 개수)
    ├── 팩맨 초기화  → col=11, row=23, dir=DIR_L (왼쪽 진행, 미로 하단)
    ├── 유령 4마리 초기화
    ├── 유령 4마리 초기화
    │   ├── Blinky: in_house=0, col=14, row=11, dir=L  (이미 밖)
    │   ├── Pinky:  in_house=1, col=12, row=13, dir=NONE
    │   ├── Inky:   in_house=1, col=13, row=13, dir=NONE
    │   └── Clyde:  in_house=1, col=14, row=13, dir=NONE
    ├── score=0, lives=3, power=0
    ├── mode_phase=0 (첫 Scatter), mode_phase_start=now
    ├── 전체 미로 그리기
    ├── HUD 그리기 (score=0, lives=3)
    ├── 팩맨 그리기 (col=12, row=23, 닫힌 입)
    ├── Blinky 그리기 (col=14, row=11)
    ├── "READY!" 텍스트 표시 (col=70, row=142)
    │
    ▼  (2초 대기)
    │
"READY!" 텍스트 제거 → ready_to_play=1
    │
    ▼
```

게임 시작 — 플레이어 입력 대기

## 2. 정규 플레이 (GS_PLAY)

### 2.1 타이밍

| 항목 | 주기 | 설명 |
|------|------|------|
| 게임 틱 | 160ms | Packman_Tick() 호출 주기 (TIM3 ISR) |
| 유령 이동 | 320ms (2틱) | move_ghost()는 2틱마다 한 번씩 실행 |
| 팩맨 이동 | 160ms (매 틱) | move_pacman()은 매 틱 실행 |
| 입 애니메이션 | 이동 시 토글 | 이동 성공할 때마다 mouth_frame 반전 (열림/닫힘) |

### 2.2 팩맨 이동

1. **방향 입력**: 버튼 짧은 누름 → `next_dir_cycle()`로 다음 방향 순환
   - 순서: NONE → R → D → L → U → R → ...
   - 설정된 `next_dir`는 즉시 적용되지 않고, **다음 틱의 이동 시도**에 사용됨
2. **매 틱**:
   - `next_dir`가 설정되어 있고, 그 방향으로 이동 가능하면 `dir = next_dir`
   - `dir` 방향으로 1셀 이동 시도
   - 통로면 이동, 벽이면 정지
   - **터널 처리**: col < 0 → col = 23, col ≥ 24 → col = 0 (화면 반대편으로 이동)
3. **이동 성공 시**:
   - 이전 셀 다시 그리기 (draw_maze_cell)
   - 새 위치에 팩맨 그리기 (draw_pacman, mouth_frame 토글)
   - dot/power pellet 먹기 처리

### 2.3 점수 체계

| 액션 | 점수 |
|------|------|
| DOT (작은 점) | 10 |
| POWER PELLET (큰 점) | 50 |
| 유령 1마리 (GM_FRIGHT 상태) | 200 × 2^combo |
| combo = 연속으로 먹은 유령 수 (첫 번째=200, 두 번째=400, 세 번째=800, 네 번째=1600) |

### 2.4 유령 행동

#### 2.4.1 모드 개요

| 모드 | 설명 |
|------|------|
| GM_CHASE | 각 유령이 개별 타겟을 추적 |
| GM_SCATTER | 각 유령이 자기 코너로 산란 (Blinky: 우상, Pinky: 좌상, Inky: 우하, Clyde: 좌하) |
| GM_FRIGHT | 파란색, 랜덤 방향 이동, 팩맨과 충돌 시 먹힘 |
| GM_EATEN | 눈만 표시, 집으로 복귀 후 리스폰 |

#### 2.4.2 Chase 타겟팅 (개별 AI)

| 유령 | 타겟 계산 |
|------|-----------|
| Blinky (0) | 팩맨의 현재 위치 (col, row) 직접 추적 |
| Pinky (1) | 팩맨 위치에서 **4칸 앞** (팩맨의 진행 방향 기준) |
| Inky (2) | 팩맨 위치에서 **2칸 앞** 점을 기준으로, Blinky 위치와의 벡터를 **2배**로 늘린 점 |
| Clyde (3) | 팩맨과의 거리 제곱 > 64이면 Blinky처럼 추적, ≤ 64이면 Scatter 모드처럼 코너로 후퇴 |

#### 2.4.3 Chase/Scatter 모드 전환 사이클

| 단계 | 모드 | 지속시간 |
|------|------|----------|
| 0 | SCATTER | 7초 |
| 1 | CHASE | 20초 |
| 2 | SCATTER | 7초 |
| 3 | CHASE | 20초 |
| 4 | SCATTER | 5초 |
| 5 | CHASE | 20초 |
| 6 | SCATTER | 5초 |
| 7+ | CHASE (영구) | — |

- 전환 시점에 모든 활성 유령(GM_FRIGHT/GM_EATEN 제외)은 **방향 반전**
- `mode_phase` 인덱스로 위 테이블을 순회, phase 7 이상이면 영구 CHASE

#### 2.4.4 Frightened 모드

1. **발생 조건**: 팩맨이 POWER PELLET(CELL_POWER, 값 3)을 먹음
2. **효과**: 모든 GM_CHASE/GM_SCATTER 유령 → GM_FRIGHT, 색상 파란색
3. **지속시간**: 6000ms (FRIGHT_MS)
4. **종료 임박**(마지막 2000ms): 유령이 파란색과 흰색을 번갈아 깜빡임 (매 틱 토글)
5. **종료**: 6000ms 경과 → GM_CHASE로 복귀

#### 2.4.5 방향 선택 알고리즘

모든 유령(집 안 제외)은 **교차로**(2개 이상 선택지)에 도착했을 때만 방향 결정:

1. 현재 진행 방향의 반대 방향은 제외 (후진 금지)
2. 남은 방향 중 `ghost_can_move()`로 이동 가능한 방향만 필터
3. 이동 가능 방향이 0개면 강제 반전 (막다른 골목)
4. 이동 가능 방향이 1개면 그 방향으로 진행
5. 이동 가능 방향이 2개 이상이면 타겟까지의 유클리드 거리 제곱이 최소인 방향 선택

**주의**: Frightened 모드에서는 타겟 거리 계산 대신 랜덤 선택.

#### 2.4.6 유령 집 출입

- **초기 상태**: Blinky는 집 밖(14,11), Pinky/Inky/Clyde는 집 안
- **출소 조건**: 게임 시작 후 4초 + 인덱스×3초 간격으로 한 마리씩 출소
  - Pinky: 4초, Inky: 7초, Clyde: 10초 후
- **출소 경로**: (12,13) [유령 집 내부] → (12,12) [지붕 출입문, CELL_GDOOR] → (12,11) [복도]
- **출소 후 모드**: GM_CHASE로 시작 (Scatter가 아닌 Chase부터)

#### 2.4.7 리스폰 (Eaten → 복귀)

1. 유령이 GM_FRIGHT 상태에서 팩맨과 충돌 → GM_EATEN
2. Eaten 상태에서는 움직이지 않음 (눈만 그림)
3. 2초 후 집 내부(12,13)로 텔레포트, in_house=1
4. 다음 release_ghosts()에서 정상 출소

### 2.5 충돌 검사

매 틱, 팩맨과 각 유령의 위치(col, row)가 같으면:

| 유령 모드 | 결과 |
|-----------|------|
| GM_CHASE / GM_SCATTER | 팩맨 사망 |
| GM_FRIGHT | 유령이 먹힘 (GM_EATEN) |
| GM_EATEN | 무시 (눈만 있는 상태) |

**팩맨 사망 시**:
- 목숨 감소, 목숨이 0이면 GS_GAMEOVER
- 목숨이 남았으면 1초 후 Packman_Init() 수준의 리셋 (미로 재생성 + READY)

### 2.6 승리 조건

`dots_eaten >= dots_total` → GS_WIN

현재는 승리 시 "YOU WIN" / "PRESS KEY" 표시, 버튼 누르면 Packman_Init() (재시작).
추후 레벨 전환 시 난이도 증가 후 새 라운드 진입 예정.

## 3. 버튼 입력 상세

### 3.1 PC13 (USER Button, Active Low, Pull-Up)

| 액션 | 지속시간 | 반응 |
|------|----------|------|
| 짧게 누름 | < 500ms | 방향 순환 (PLAY 상태) |
| 길게 누름 | ≥ 500ms | 일시정지 토글 (PLAY ↔ PAUSE) |
| 게임오버/승리 시 | 아무 누름 | Packman_Init() (재시작) |

### 3.2 디바운스

- Packman_OnButton() 진입 시 200ms 이내 재호출 무시
- long press는 Tick에서 500ms 임계값으로 검출

### 3.3 일시정지 (PAUSE)

- GS_PAUSE 상태에서는 Tick이 게임 로직을 실행하지 않음
- PAUSE 텍스트 표시
- 다시 길게 누르면 GS_PLAY로 복귀
- 복귀 시 mode_phase_start를 현재 시각으로 리셋 (Chase/Scatter 타이밍 점프 방지)

## 4. 화면 구성

```
HUD: Y=0~19    점수 표시 (왼쪽 정렬)
─────────────────────────────────
게임: Y=20~299  미로 24×28 셀, 각 10px
                팩맨, 유령, 점, 벽
─────────────────────────────────
HUD: Y=300~319 목숨 표시 (작은 팩맨 아이콘, 오른쪽 정렬)
```

### 4.1 오버레이 메시지

| 메시지 | 위치 | 표시 조건 | 색상 |
|--------|------|-----------|------|
| READY! | (70, 142) | Packman_Init 후 2초간 | 노랑(COL_POWER) |
| GAME OVER | (70, 100) | 목숨 0 | 빨강(0xF800) |
| YOU WIN | (70, 100) | 모든 점 소진 | 노랑 |
| PRESS KEY | (70, 120) | YOU WIN 아래 | 흰색 |
| PAUSE | (70, 142) | 길게 누름 | 흰색 |

## 5. 미로 데이터

```
cells: 0=빈공간, 1=벽(파랑), 2=점(노랑), 3=파워펠렛(큰노랑)
       4=유령집바닥, 5=출입문(분홍)

가로 24셀, 세로 28셀.
셀 크기 10px → 게임영역 240×280px

터널: row=14(col 0~5, col 18~23) — 화면 좌우 가장자리 연결
      row=11(left 5셀), row=17(left 5셀) — 추가 통로

유령집: cols 11~14, rows 13~15 (CELL_GHOUSE=4)
출입문: row=12, col=12 (CELL_GDOOR=5) — 지붕 위
이전 출입문(row=16, cols 11~14)은 제거 → 현재는 일반 통로
```

## 6. 레벨 전환 (예정)

현재는 승리 시 재시작만 가능. 추후 구현 예정:

| 난이도 요소 | 변화 방향 |
|-------------|-----------|
| 유령 이동 속도 | 증가 (틱 단축) |
| Chase/Scatter 시간 | Scatter 감소, Chase 증가 |
| FRIGHT 지속시간 | 감소 |
| 유령 출소 간격 | 단축 |
| 점수 보너스 | 동일 |

## 7. 현재 구현 상태 요약

- [x] 미로 렌더링 (벽, 점, 파워펠렛, 유령집, 출입문, 외곽선만)
- [x] 팩맨 이동 + 방향 전환 (시작위치/방향 보정 완료)
- [x] 점/파워펠렛 먹기 + 점수
- [x] 유령 AI 개별화 (Blinky/Pinky/Inky/Clyde)
- [x] Chase/Scatter 모드 전환 사이클
- [x] Frightened 모드 + 종료 임박 깜빡임
- [x] 유령 집 출입문 통과 (12,13 → 12,12 → 12,11)
- [x] 팩맨 입 삼각형 마스킹 (wedge shape)
- [x] HUD (점수/목숨)
- [x] READY! 메시지 (+ 배경 복원)
- [x] GAME OVER / YOU WIN 메시지
- [x] 롱프레스 일시정지
- [x] 고스트 드로잉 (cy 보정, cell 내에 그림)
- [x] DOT/POWER 셀 배경 채움 (고스트 잔상 제거)
- [x] 폰트 데이터 4비트 정합 (숫자/알파벳 왼쪽 열 누락 수정)
- [ ] 유령 출소 전 집 안 대기 애니메이션 (상하로 깜빡임)
- [ ] 사운드 출력 (버저/PWM)
- [ ] 레벨 전환 (난이도 증가)
- [ ] 프레임 속도와 게임 로직 분리 (variable tick)
- [ ] 타이머 인터럽트 기반 Tick (현재는 main loop delay)
