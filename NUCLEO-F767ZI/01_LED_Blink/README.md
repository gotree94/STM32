# STM32
## NUCLEO-F767ZI

## NUCLEO-F767ZI LED Blink 프로젝트

<img width="1193" height="896" alt="001" src="https://github.com/user-attachments/assets/25ca614b-4ec5-422f-8330-fd55f057d2a8" />

## 1. 하드웨어 확인
NUCLEO-F767ZI 보드의 User LED 핀 매핑:
  * LD1 (Green): PB0
  * LD2 (Blue): PB7
  * LD3 (Red): PB14

## 2. STM32CubeIDE 프로젝트 생성
### 2.1 새 프로젝트 시작

1. File → New → STM32 Project
2. Board Selector 탭에서 NUCLEO-F767ZI 검색 후 선택
3. 프로젝트 이름 입력 (예: LED_Blink_LD1_LD3)
4. Targeted Language: C
5. Targeted Binary Type: Executable
6. Finish 클릭

### 2.2 CubeMX 핀 설정 (.ioc 파일)
프로젝트 생성 시 보드를 선택했다면 LD1, LD2, LD3가 이미 GPIO_Output으로 설정되어 있을 것. 확인 및 수정 방법:

1. .ioc 파일 더블클릭하여 CubeMX 열기
2. Pinout & Configuration 탭에서 확인:
  * PB0 → GPIO_Output (LD1)
  * PB14 → GPIO_Output (LD3)
3. System Core → GPIO 클릭 후 각 핀 설정:

| 핀 |  GPIO output level | GPIO mode | User Label| 
|:----:| :----:| :----:| :----:| 
| PB0 | Low | Output Push Pull | LD1 | 
| PB14 | Low | Output Push Pull | LD3 | 

4. Project → Generate Code (또는 Ctrl+S)

## 3. 코드 작성
 
 * Core/Src/main.c 파일의 while(1) 루프 수정:

```c
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, 1);
	  HAL_Delay(50);
	  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 1);
	  HAL_Delay(50);
	  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, 1);
	  HAL_Delay(50);
	  HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, 0);
	  HAL_Delay(50);
	  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);
	  HAL_Delay(50);
	  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, 0);
	  HAL_Delay(50);

    /* USER CODE END WHILE */
```

또는 Toggle 방식으로 더 간단하게:

```c
while (1)
{
    HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
    HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
    HAL_Delay(500);
}
```

## 4. 빌드 및 다운로드
### 4.1 빌드
1. Project → Build Project (또는 Ctrl+B)
2. Console 창에서 에러 없이 완료 확인

### 4.2 디버그/다운로드
1. USB 케이블로 NUCLEO 보드 연결 (ST-LINK USB 포트 사용)
2. Run → Debug As → STM32 C/C++ Application
3. 처음 실행 시 Debug Configuration 창이 뜨면 기본 설정 유지 후 OK
4. Debugger로 진입 후 Resume (F8) 클릭하여 실행

* 또는 디버그 없이 바로 실행:

   * Run → Run As → STM32 C/C++ Application

## 5. 동작 확인

* LD1(초록)과 LD3(빨강)이 500ms 간격으로 번갈아 점멸
* Toggle 방식 사용 시 두 LED가 동시에 토글됨

## 6. 트러블슈팅

* LED가 안 켜지는 경우:
  * User Label이 제대로 설정되었는지 확인 (main.h에서 LD1_Pin, LD1_GPIO_Port 매크로 확인)
  * 코드 생성이 제대로 되었는지 확인 (Ctrl+S로 재생성)

* ST-LINK 연결 안 되는 경우:
  * ST-LINK 드라이버 설치 확인
  * USB 케이블이 데이터 전송 가능한 케이블인지 확인
  * Device Manager에서 ST-LINK 인식 확인
