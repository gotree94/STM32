# NUCLEO-F411RE 단계별 학습 노트

> 보드: NUCLEO-F411RE (STM32F411RET6, Cortex-M4F, 512 KB Flash, 128 KB SRAM) <br>
> 프로젝트: `F411_TASK_LAB` (`C:\Users\Administrator\STM32CubeIDE\workspace_2.2.0\F411_TASK_LAB`) <br>
> 도구: STM32CubeIDE 2.2.0 / STM32Cube FW_F4 V1.28.3 <br>
> Zephyr: v4.5.0 / SDK 1.0.1 / workspace `D:\zephyrproject` <br>
> 최종 업데이트: 2026-11-18

---

## 진행 방식

- **한 번에 한 단계만** 진행한다.
- 각 단계는 `목표 → 준비물/배선 → 설정 → 코드 → 빌드/플래시 → 보드 확인 → 체크리스트` 순서로 읽는다.
- **보드에서 확인이 끝난 뒤에만** 다음 단계 파일을 연다.
- 센서 모델, 핀, 주기, 정확도 같은 결정은 해당 단계에서 함께 정한다.
  미리 확정하지 않은 가정은 문서에 적지 않는다.

## 지금 읽을 파일

1. `STEP00_환경확인.md` (처음 한 번만)
2. `STEP01_LED.md` (LED 점멸 구조 이해)
3. `STEP02_거리측정.md` (초음파 측정 구조 이해)
4. `STEP03_온습도_보정.md` ← 완료 (보드 동작 확인 완료)
5. `STEP04_FreeRTOS.md` (코드/빌드 완료, 보드 확인 대기)
6. **`STEP05_Zephyr.md`** ← 현재 단계 (코드/빌드 완료, 보드 확인 대기)

## 진행 현황

| 단계 | 내용 | 상태 |
|---|---|---|
| STEP00 | 보드/툴체인/시리얼/빌드·플래시 확인 | 정리됨 |
| STEP01 | LED 점멸 | 코드/빌드 완료, STEP02에서 UART 로그 제거 |
| STEP02 | 초음파 거리 측정 | **보드 동작 확인 완료** (`D,...` CSV 정상) |
| STEP03 | 온습도 센서 + 음속 보정 | **보드 동작 확인 완료** (`D/E` CSV, `delta` 10~12 mm) |
| STEP04 | FreeRTOS 태스크 분리 | **코드/빌드 완료, 보드 확인 대기** |
| STEP05 | Zephyr 구현 | **코드/빌드 완료, 보드 확인 대기** |
| STEP06 | MNIST PC 학습 → ST Edge AI | 예정 |

## 다음 행동

1. `STEP05_Zephyr.md`를 읽고 `west build` / `west flash`한다.
2. Zephyr 배너(`=== F411_TASK_LAB Step 5: Zephyr ===`)가 나오는지 확인한다.
3. LD2가 0.5초마다 점멸하는지 확인한다.
4. 시리얼에서 `D,...` 1 Hz, `E,...` 0.5 Hz가 계속 출력되는지 확인한다.
5. `E`의 `valid=1`, `D`의 `delta_mm`이 10~12 mm로 유지되는지 확인한다.
6. STEP04 보드 확인도 아직 미완료이므로 같은 배선으로 함께 확인한다.
7. 결과를 알려주면 STEP06(MNIST → ST Edge AI)으로 진행한다.

> STEP02부터 LED는 무음으로 점멸한다. `LED ON` / `LED OFF` UART 출력은
> 거리 CSV를 깨끗하게 유지하려고 제거했다.

## 파일 목록

| 파일 | 내용 |
|---|---|
| `README.md` | 진행 방식, 현황, 다음 행동 |
| `STEP00_환경확인.md` | 보드 정보, 프로젝트 열기, 빌드/플래시, 시리얼 |
| `STEP01_LED.md` | LED 점멸 구현과 비차단 구조 |
| `STEP02_거리측정.md` | HC-SR04 배선, TIM2/EXTI 측정, 필터, CSV 검증 |
| `STEP03_온습도_보정.md` | DHT11/DHT22 배선, 음속 보정, 환경 CSV 검증 |
| `STEP04_FreeRTOS.md` | FreeRTOS 태스크 분리, 큐 로거, TIM6 HAL tick |
| `STEP05_Zephyr.md` | Zephyr 스레드, 디바이스 트리, message queue 로거, 빌드/플래시 |
| `STEP05_Zephyr/` | Zephyr 앱 소스 (`CMakeLists.txt`, `prj.conf`, overlay, binding, `src/`) |

> STEP06부터는 실제로 시작할 때 파일을 새로 만든다.
> 각 단계는 보드에서 확인한 뒤 다음 단계로 넘어간다.

> 이전 `START_PRJ`는 백업으로 남겨 두었다.
> 앞으로의 작업은 `F411_TASK_LAB`에서 진행한다.
