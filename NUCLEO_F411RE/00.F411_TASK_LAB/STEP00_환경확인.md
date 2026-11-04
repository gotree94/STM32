# STEP00 - 환경 확인

> 이 단계는 한 번만 하면 된다. 이미 프로젝트가 열리고 빌드된다면 STEP01로 넘어가도 된다.

---

## 1. 보드와 기본 핀

| 기능 | 핀 | 비고 |
|---|---|---|
| LD2 (Green LED) | PA5 | High = ON |
| B1 (Blue Button) | PC13 | 누르면 Low, EXTI Falling |
| USART2 TX | PA2 | ST-LINK VCP, 115200 8N1 |
| USART2 RX | PA3 | ST-LINK VCP |
| SWDIO / SWCLK | PA13 / PA14 | 다운로드·디버그 |
| SWO | PB3 | 선택 |

현재 클럭: HSI 16 MHz → PLL → **SYSCLK 84 MHz**
(AHB 84 MHz, APB1 42 MHz, APB2 84 MHz, Flash latency 2 WS)

---

## 2. 프로젝트 열기

1. STM32CubeIDE 실행
2. `File` → `Open Projects from File System...`
3. `Directory`에 아래 경로 입력
   ```text
   C:\Users\Administrator\STM32CubeIDE\workspace_2.2.0\F411_TASK_LAB
   ```
4. `Finish`
5. Project Explorer에서 `F411_TASK_LAB`가 보이는지 확인

> 이미 워크스페이스에 `F411_TASK_LAB`이 있으면 그대로 열면 된다.

---

## 3. 빌드

1. `Project` → `Build Project` (또는 `Ctrl+B`)
2. Console 마지막에 아래가 보이면 성공
   ```text
   Build Finished. 0 errors, 0 warnings.
   ```
3. 산출물: `Debug\F411_TASK_LAB.elf`

빌드가 안 되면:

- `Project` → `Clean...` 후 다시 Build
- 좌측 Project Explorer에서 프로젝트가 선택되어 있는지 확인
- `Properties` → `C/C++ Build` → `Tool Chain Editor`에서 `MCU ARM GCC` 확인

---

## 4. 플래시

1. ST-LINK USB를 PC와 보드에 연결
2. 보드 전원 LED가 켜지는지 확인
3. `Run` → `Run` (또는 `Ctrl+F11`)
4. 처음이면 ST-LINK 드라이버/펌웨어 업데이트 안내가 나올 수 있다

> 디버그가 필요하면 `Run` → `Debug` (F11)를 사용한다.

---

## 5. 시리얼 모니터

1. Windows `장치 관리자` → `포트 (COM & LPT)`
2. `STMicroelectronics STLink Virtual COM Port (COMx)` 확인
3. Tera Term / PuTTY / VS Code Serial Monitor 실행
4. 설정
   - Baud rate: **115200**
   - Data: 8 bit
   - Parity: None
   - Stop: 1 bit
   - Flow control: None
5. 보드의 검은 `RESET` 버튼을 누른다

> COM 포트가 안 보이면 ST-LINK 드라이버 설치 여부와 USB 케이블(데이터 지원)을 확인한다.
> USB 케이블 중에는 전원만 되는 케이블도 있다.

---

## 6. 자주 막히는 곳

| 증상 | 확인 |
|---|---|
| 빌드는 되는데 플래시 실패 | ST-LINK 연결, 보드 전원, 다른 디버거 사용 중인지 |
| COM 포트가 안 보임 | ST-LINK 드라이버, USB 케이블, 장치 관리자 |
| 시리얼이 깨짐 | Baud rate 115200, 8N1, 잘못된 COM 포트 |
| 보드 리셋 후 출력 없음 | 시리얼 프로그램에서 포트를 먼저 연 뒤 리셋 |
| LD2가 안 켜짐 | PA5 설정, 코드, 보드의 LD2 위치 확인 |

---

## 7. 다음

`STEP01_LED.md`로 이동한다.
