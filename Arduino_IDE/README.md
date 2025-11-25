# STM32F411 iwth Arduino IDE

* https://www.arduino.cc/en/software/#ide

## Arduino IDE에서 STM32 코어 업데이트
### 1. Board Manager를 통한 업데이트
   * Arduino IDE 2.x 기준:
1. Tools → Board → Boards Manager 열기
2. 검색창에 STM32 입력
3. STM32 MCU based boards (STMicroelectronics 공식) 또는 STM32duino 패키지 찾기
4. 새 버전이 있으면 Update 버튼 클릭

### 2. 수동으로 보드 매니저 URL 확인
File → Preferences에서 Additional Boards Manager URLs에 아래 URL이 있는지 확인:

```
https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
```

### 3. 캐시 문제 해결 (업데이트가 안 보일 때)
```bash
# Windows
%LOCALAPPDATA%\Arduino15\staging\packages

# Linux
~/.arduino15/staging/packages

# macOS
~/Library/Arduino15/staging/packages
```

* 해당 폴더의 STM32 관련 파일들을 삭제 후 Arduino IDE 재시작하면 최신 버전을 다시 받아올 수 있어요.

### 4. 부트로더 업데이트 (필요시)
STM32duino 부트로더도 업데이트가 필요하다면:

```bash
# DFU 모드 진입 (BOOT0 = HIGH 상태에서 리셋)
# STM32CubeProgrammer 또는 dfu-util 사용

dfu-util -a 0 -s 0x08000000 -D bootloader_f411.bin
```

NUCLEO-F411RE 보드는 Arduino IDE에서 공식적으로 지원됩니다.
STM32 공식 코어에서 NUCLEO-F411RE 선택
1. 보드 매니저 설치/업데이트
File → Preferences에서 Additional Boards Manager URLs에 추가:
https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
2. 보드 선택 경로
Tools → Board → STM32 MCU based boards 에서:
Board: "Nucleo-64"
Board part number: "Nucleo F411RE"
3. 업로드 설정
항목설정값Upload methodSTM32CubeProgrammer (SWD)USB supportCDC (generic Serial)U(S)ART supportEnabled (generic Serial)USB speedLow/Full Speed
NUCLEO 보드는 ST-Link가 내장되어 있어서 SWD 방식이 가장 안정적이에요.
4. 필요한 드라이버
STM32CubeProgrammer를 설치하면 ST-Link 드라이버도 함께 설치됩니다:

STM32CubeProgrammer 다운로드
