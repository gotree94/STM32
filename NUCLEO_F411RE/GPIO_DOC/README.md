# GPIO
   - [Datasheet - STM32F411xC STM32F411xE](https://www.st.com/resource/en/datasheet/stm32f411ce.pdf)
   - [RM0383 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0383-stm32f411xce-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)

## Table of Contents
- [1. Architecture](#1-architecture)
- [2. MemoryMap](#2-memorymap)
- [3. Register](#3-register)
- [4. Test](#4-test)

---
## STM32F103

<img src="./img/arm-arch.png"> <br>

* 각 GPIO 핀은 소프트웨어로 출력(푸시풀 또는 오픈 드레인), 입력(풀업 또는 풀다운 여부 선택), 또는 주변 장치의 대체 기능으로 구성할 수 있습니다.<br>
* 대부분의 GPIO 핀은 디지털 또는 아날로그 대체 기능과 공유됩니다. 모든 GPIO는 고전류를 지원합니다.<br>
* I/O의 대체 기능 구성은 필요 시 특정 순서를 따라 잠글 수 있으며, 이는 I/O 레지스터에 불필요한 기록이 이루어지는 것을 방지하기 위함입니다.<br>
* I/O는 APB2 버스에 연결되어 있으며, 최대 18 MHz의 토글 속도를 지원합니다.<br>

---
## STM32F411

### 메모리 및 버스 아키텍처

#### 시스템 아키텍처

* STM32F411xC/E의 메인 시스템은 32비트 다중 계층(multilayer) AHB 버스 매트릭스로 구성되어 있으며, 이를 통해 다음 요소들이 상호 연결됩니다. <br>
* 6개의 마스터(Master):
   * FPU를 포함한 Cortex-M4 코어의 I-버스(I-bus), D-버스(D-bus) 및 S-버스(S-bus)
   * DMA1 메모리 버스
   * DMA2 메모리 버스
   * DMA2 주변장치(peripheral) 버스
* 5개의 슬레이브(Slave):
   * 내부 플래시 메모리 ICode 버스
   * 내부 플래시 메모리 DCode 버스
   * 메인 내부 SRAM
   * AHB-APB 브리지 및 APB 주변장치를 포함한 AHB1 주변장치
   * AHB2 주변장치

버스 매트릭스는 마스터가 슬레이브에 접근할 수 있도록 경로를 제공하며, 여러 고속 주변장치가 동시에 작동할 때에도 동시 접근을 가능하게 하여 효율적인 동작을 지원합니다. 이러한 아키텍처는 그림 1에 나타나 있습니다.

<img src="./img/arm-arch-2.png"> <br>

#### 2.1.1 I-bus(명령 버스)
이 버스는 FPU가 포함된 Cortex®-M4 코어의 명령 버스(Instruction bus)를 BusMatrix에 연결합니다. 코어는 이 버스를 사용하여 명령을 가져옵니다(fetch). 이 버스의 대상은 코드가 저장된 메모리(내부 플래시 메모리/SRAM)입니다.

#### 2.1.2 D-bus(데이터 버스)
이 버스는 FPU가 포함된 Cortex®-M4의 데이터 버스를 BusMatrix에 연결합니다. 코어는 이 버스를 사용하여 리터럴(상수) 값을 로드하거나 디버깅을 위한 접근을 수행합니다. 이 버스의 대상은 코드나 데이터가 저장된 메모리(내부 플래시 메모리/SRAM)입니다.

#### 2.1.3 S-bus(시스템 버스)
이 버스는 FPU가 포함된 Cortex®-M4 코어의 시스템 버스를 BusMatrix에 연결합니다. 이 버스는 주변 장치(peripheral)나 SRAM에 위치한 데이터에 접근하는 데 사용됩니다. 명령을 가져오는(fetch) 용도로도 사용할 수 있지만, ICode 버스를 사용하는 것보다 효율성은 떨어집니다. 이 버스의 대상은 내부 SRAM1, AHB1 주변 장치(APB 주변 장치 포함), 그리고 AHB2 주변 장치입니다.

#### 2.1.4 
DMA 메모리 버스
이 버스는 DMA 메모리 버스 마스터 인터페이스를 BusMatrix에 연결합니다. DMA가 메모리와 데이터를 주고받는(전송하는) 작업을 수행할 때 사용됩니다. 이 버스의 대상은 데이터 메모리(내부 플래시 메모리, 내부 SRAM)이며, S4의 경우 AHB1/AHB2 주변 장치(APB 주변 장치 포함)도 대상에 포함됩니다.

#### 2.1.5 
DMA 주변 장치 버스
이 버스는 DMA 주변 장치 마스터 버스 인터페이스를 BusMatrix에 연결합니다. DMA가 AHB 주변 장치에 접근하거나 메모리 간(memory-to-memory) 데이터 전송을 수행할 때 사용됩니다. 이 버스의 대상은 AHB 및 APB 주변 장치와 데이터 메모리(플래시 메모리 및 내부 SRAM)입니다.

#### 2.1.6 BusMatrix(버스 매트릭스)
BusMatrix는 마스터 간의 접근 중재(arbitration)를 관리합니다. 중재 방식으로는 라운드 로빈(round-robin) 알고리즘이 사용됩니다. 

#### 2.1.7 AHB/APB 브리지(APB)
두 개의 AHB/APB 브리지인 APB1과 APB2는 AHB 버스와 두 개의 APB 버스 간에 완전 동기식 연결을 제공하며, 이를 통해 주변 장치(peripheral)의 동작 주파수를 유연하게 선택할 수 있습니다.
APB1 및 APB2의 최대 동작 주파수에 대한 자세한 내용은 해당 디바이스의 데이터시트를 참조하고, AHB 및 APB 주변 장치의 주소 매핑에 대해서는 표 1을 참조하십시오.
디바이스가 리셋될 때마다 모든 주변 장치 클록은 비활성화됩니다(SRAM 및 플래시 메모리 인터페이스는 예외). 주변 장치를 사용하려면 먼저 RCC_AHBxENR 또는 RCC_APBxENR 레지스터를 통해 해당 주변 장치의 클록을 활성화해야 합니다.

참고: APB 레지스터에 대해 16비트 또는 8비트 액세스를 수행하면, 해당 액세스는 32비트 액세스로 변환됩니다. 즉, 브리지가 16비트 또는 8비트 데이터를 복제하여 32비트 벡터를 구성합니다.


   
<img src="./img/arm-arch-1.png"> <br>


## 1.Architecture

<details>
<summary>Click to collapse</summary>
   
## **2.3.21 GPIOs (general-purpose inputs/outputs)**

* Each of the GPIO pins can be configured by software as output (push-pull or open-drain), as input (with or without pull-up or pull-down) or as peripheral alternate function.
* Most of the GPIO pins are shared with digital or analog alternate functions. All GPIOs are high current- capable.
* The I/Os alternate function configuration can be locked if needed following a specific sequence in order to avoid spurious writing to the I/Os registers.
* I/Os on APB2 with up to 18 MHz toggling speed.

<img src="./img/003.png"> <br>
<img src="./img/004.png"> <br>
<img src="./img/005.png"> <br>
<img src="./img/006.png"> <br>
</details>

## 2.MemoryMap

<details>
<summary>Click to collapse</summary>
<img src="./img/007.png"> <br>
   
<img src="./img/008.png" width="85%"> <br>
<img src="./img/009.png" width="85%"> <br>
<img src="./img/010.png" width="85%"> <br>
<img src="./img/011.png" width="85%"> <br>
<img src="./img/012.png" width="85%"> <br>
<img src="./img/013.png" width="85%"> <br>
<img src="./img/014.png" width="85%"> <br>
<img src="./img/015.png" width="85%"> <br>
<img src="./img/016.png" width="85%"> <br>
<img src="./img/017.png" width="85%"> <br>
<img src="./img/017-1.png" width="85%"> <br>
</details>


<details>
<summary>Click to collapse</summary>
<img src="./img/007-1.png" width="120%"> <br>
   
<img src="./img/010-1.png" width="85%"> <br>
<img src="./img/011-1.png" width="85%"> <br>
</details>

## 3.Register

<details>
<summary>Click to collapse</summary>
<img src="./img/018.png"> <br>
<img src="./img/019.png"> <br>
<img src="./img/020.png" width="85%"> <br>
<img src="./img/021.png" width="85%"> <br>
</details>

## 4.Test

<details>
<summary>Click to collapse</summary>

#### 1.	LED test 프로그램을 작성합니다.
<img src="./img/1001.png"> <br>

#### 2.	디버그를 시작합니다.
<img src="./img/1002.png"> <br>

#### 3.	디버그 메뉴에서 실행을 시작합니다.
<img src="./img/1003.png"> <br>

#### 4.	LD2_GPIO_Port 위에 마우스를 올리면 관련 정보들이 나옵니다.
<img src="./img/1004.png"> <br>
<img src="./img/1005.png"> <br>

#### 5.	GPIO A 관련 정보를 위에서 확인하면 관련 레지스터 및 주소 옵셋을 확인할 수 있습니다.<br>
<img src="./img/1006.png"> <br>

#### 6.	메모리 값을 확인 및 접근하기 위해서 아래에서 Memory 탭을 누릅니다.<br>
<img src="./img/1007.png"> <br>

#### 7.	플러스 버튼을 눌러서 관련 번지를 입력합니다. 값을 0x40010800을 입력합니다.<br>
<img src="./img/1008.png"> <br>

#### 8.	오른쪽에 관련 메모리 범위와 값이 표현됩니다.<br>
<img src="./img/1009.png"> <br>

#### 9.	우선은 값들의 변화를 확인하기 위해서 브레이크 포인트 부터 한단계씩 실행하면서 값의 변화를 확인합니다.<br>
<img src="./img/1010.png"> <br>

#### 10.	우선은 값들의 변화를 확인하기 위해서 브레이크 포인트 부터 한단계씩 실행하면서 값의 변화를 확인합니다. <br>
#### (HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 1 실행전);<br>
<img src="./img/1011.png"> <br>

#### 11.	우선은 값들의 변화를 확인하기 위해서 브레이크 포인트 부터 한단계씩 실행하면서 값의 변화를 확인합니다. <br>
#### (HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);<br>
<img src="./img/1012.png"> <br>

#### 12.	우선은 값들의 변화를 확인하기 위해서 브레이크 포인트 부터 한단계씩 실행하면서 값의 변화를 확인합니다. <br>
#### (HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);<br>   
<img src="./img/1013.png"> <br>

#### 13.	C-F 위치의 레지스터에서 값을 직접 입력하면서 LED의 상태를 확인하고, 레지스터의 위치와 비교해보면서 동작시켜 봅니다.<br>
<img src="./img/1014.png"> <br>

</details>
