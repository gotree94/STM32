# GPIO
- [RM0008 Reference Manual (STM32F101/2/3/5/7 Series)](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [DS5319 Datasheet (STM32F101/2/3/5/7 Series)](https://www.st.com/resource/en/datasheet/stm32f103rb.pdf)

- 1.[Architecture](#Architecture)
- 2.[MemoryMap](#MemoryMap)
- 3.[Register](#Register)
- 4.[Test](#Test)


## Architecture
<img width="700" height="852" alt="GPIO_Register_010" src="https://github.com/user-attachments/assets/45a3773c-c552-4388-b9bb-01d889790867" />
<br>
<img width="700" height="360" alt="GPIO_Register_011" src="https://github.com/user-attachments/assets/2eb0bbf5-16f7-4f15-a5b5-51e834e80615" />
<br>
   * 각 GPIO 핀은 소프트웨어로 출력(푸시풀 또는 오픈 드레인), 입력(풀업 또는 풀다운 여부 선택), 또는 주변 장치의 대체 기능으로 구성할 수 있습니다.<br>
   * 대부분의 GPIO 핀은 디지털 또는 아날로그 대체 기능과 공유됩니다. 모든 GPIO는 고전류를 지원합니다.<br>
   * I/O의 대체 기능 구성은 필요 시 특정 순서를 따라 잠글 수 있으며, 이는 I/O 레지스터에 불필요한 기록이 이루어지는 것을 방지하기 위함입니다.<br>
   * I/O는 APB2 버스에 연결되어 있으며, 최대 18 MHz의 토글 속도를 지원합니다.<br>
<br>
<img width="700" height="909" alt="GPIO_Register_012" src="https://github.com/user-attachments/assets/b48ca11a-e702-4809-b98a-dbe8e864ecfa" />
<br>
<img width="700" height="685" alt="clock_tree" src="https://github.com/user-attachments/assets/416aed77-6d31-471f-b18d-dee1bc57e469" />
<br>
<img width="700" height="541" alt="gpio_001" src="https://github.com/user-attachments/assets/1140a763-b73a-469a-b128-425ddf8dbb6c" />
<br>
<img width="700" height="572" alt="gpio_002" src="https://github.com/user-attachments/assets/15d26c36-b388-4465-9624-5942e86d6738" />
<br>

## MemoryMap
<br>
<img width="700" height="841" alt="memory_map_000" src="https://github.com/user-attachments/assets/61b183dc-5844-4e6e-af76-f233ff1be6e5" />
<br>
<img width="700" height="554" alt="memory_map_001" src="https://github.com/user-attachments/assets/484f6555-89aa-4b1e-8b21-b2d93045dc71" />
<br>
<img width="700" height="638" alt="memory_map_002" src="https://github.com/user-attachments/assets/72cf784d-3cc2-4f82-af47-cb56706edd09" />
<br>
<img width="700" height="599" alt="memory_map_003" src="https://github.com/user-attachments/assets/058abfec-9e1a-47d2-8387-a5e3f275a2ba" />
<br>
<img width="700" height="681" alt="memory_map_004" src="https://github.com/user-attachments/assets/9ead6b79-6af2-4b08-8dd5-e46634ac0314" />
<br>
<img width="700" height="897" alt="memory_map_005" src="https://github.com/user-attachments/assets/02d6abe9-3942-4416-9c25-0f624d0352b0" />
<br>
<img width="700" height="889" alt="memory_map_006" src="https://github.com/user-attachments/assets/595e60d5-ad9e-4091-b8a6-6aecc00cfdc4" />
<br>
<img width="700" height="1034" alt="memory_map_007" src="https://github.com/user-attachments/assets/95f81606-7960-477c-ac24-2c0f2a77c976" />
<br>
<img width="700" height="870" alt="memory_map_008" src="https://github.com/user-attachments/assets/7d815b4c-e36b-4469-9396-7dadfe013d79" />
<br>
<img width="700" height="831" alt="memory_map_009" src="https://github.com/user-attachments/assets/ff78b8af-c6df-44b6-8b46-f83db18ba049" />
<br>
<img width="700" height="695" alt="memory_map_010" src="https://github.com/user-attachments/assets/e2354241-30d5-4466-9b4a-d8c7a74369fc" />
<br>
<img width="700" height="537" alt="memory_map_011" src="https://github.com/user-attachments/assets/1705425a-930b-49b5-9602-48708ed5360e" />
<br>

## Register
<img width="700" height="789" alt="GPIO_Register_001" src="https://github.com/user-attachments/assets/8bdea1c9-775c-49a1-9edc-1561dbcb336a" />
<br>
<img width="700" height="707" alt="GPIO_Register_002" src="https://github.com/user-attachments/assets/4f3f19c2-9a94-42b4-a0b5-114d87a409a4" />
<br>
<img width="700" height="368" alt="GPIO_Register_003" src="https://github.com/user-attachments/assets/45f106f2-e432-4dcb-bf74-c4dd08d2fef7" />
<br>
<img width="700" height="376" alt="GPIO_Register_004" src="https://github.com/user-attachments/assets/6c4b1f8f-dbe2-4374-8908-ee492f53d153" />
<br>
<img width="700" height="481" alt="GPIO_Register_005" src="https://github.com/user-attachments/assets/07bf0f6b-c3d9-465c-a002-17d2ac9a41b5" />
<br>
<img width="700" height="376" alt="GPIO_Register_006" src="https://github.com/user-attachments/assets/48daf395-3145-41ca-b3a3-8a4fb0201697" />
<br>
<img width="700" height="813" alt="GPIO_Register_007" src="https://github.com/user-attachments/assets/01aab0d6-81dd-4590-b46d-19f6201a8980" />
<br>


## Test

#### 1.	LED test 프로그램을 작성합니다.<br>
<img width="459" height="151" alt="GPIO_Register_001" src="https://github.com/user-attachments/assets/46223b7e-ec0a-41c5-86ec-ef612250b98a" />
<br>

#### 2.	디버그를 시작합니다.<br>
<img width="486" height="69" alt="GPIO_Register_002" src="https://github.com/user-attachments/assets/43a3d8b7-8d07-493e-a005-b04296b30bb5" />
<br>

#### 3.	디버그 메뉴에서 실행을 시작합니다.<br>
<img width="295" height="53" alt="GPIO_Register_003" src="https://github.com/user-attachments/assets/3919a347-9f9f-4f1a-b11f-37045c2d3514" />
<br>

#### 4.	LD2_GPIO_Port 위에 마우스를 올리면 관련 정보들이 나옵니다.<br>
<img width="875" height="296" alt="GPIO_Register_004" src="https://github.com/user-attachments/assets/5ad41fc9-1d63-43a1-a5d6-01bf8adaabff" />
<br>

<img width="1920" height="1080" alt="GPIO_Register_005" src="https://github.com/user-attachments/assets/bb5ddcaf-b887-4451-88ac-1212a51e3638" />
<br>

#### 5.	GPIO A 관련 정보를 위에서 확인하면 관련 레지스터 및 주소 옵셋을 확인할 수 있습니다.<br>
<img width="862" height="460" alt="GPIO_Register_006" src="https://github.com/user-attachments/assets/7c0644bb-55dc-4072-a00f-57b6e5544b2f" />
<br>

#### 6.	메모리 값을 확인 및 접근하기 위해서 아래에서 Memory 탭을 누릅니다.<br>
<img width="652" height="350" alt="GPIO_Register_007" src="https://github.com/user-attachments/assets/70b06a32-57ff-41a0-bfba-7a708df195d7" />
<br>

#### 7.	플러스 버튼을 눌러서 관련 번지를 입력합니다. 값을 0x40010800을 입력합니다.<br>
<img width="622" height="351" alt="GPIO_Register_008" src="https://github.com/user-attachments/assets/bddee6a3-5078-4ec3-966a-f55926d10c72" />
<br>

#### 8.	오른쪽에 관련 메모리 범위와 값이 표현됩니다.<br>
<img width="967" height="453" alt="GPIO_Register_009" src="https://github.com/user-attachments/assets/2258a120-7b56-4842-919f-4b8f4af537ad" />
<br>

#### 9.	우선은 값들의 변화를 확인하기 위해서 브레이크 포인트 부터 한단계씩 실행하면서 값의 변화를 확인합니다.<br>
<img width="901" height="415" alt="GPIO_Setting_001" src="https://github.com/user-attachments/assets/f5b0951f-8288-4978-8622-63b2873af74c" />
<br>

#### 10.	우선은 값들의 변화를 확인하기 위해서 브레이크 포인트 부터 한단계씩 실행하면서 값의 변화를 확인합니다. <br>
#### (HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 1 실행전);<br>
<img width="854" height="358" alt="GPIO_Setting_002" src="https://github.com/user-attachments/assets/aea09dcf-1953-46b5-97f3-3e0d9f0572d5" />
<br>

#### 11.	우선은 값들의 변화를 확인하기 위해서 브레이크 포인트 부터 한단계씩 실행하면서 값의 변화를 확인합니다. <br>
#### (HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);<br>
<img width="862" height="403" alt="GPIO_Setting_003" src="https://github.com/user-attachments/assets/7c4257d6-a1ca-4d63-b56d-22b2201cd2ea" />
<br>

#### 12.	우선은 값들의 변화를 확인하기 위해서 브레이크 포인트 부터 한단계씩 실행하면서 값의 변화를 확인합니다. <br>
#### (HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);<br>   
<img width="960" height="480" alt="GPIO_Setting_004" src="https://github.com/user-attachments/assets/e06ae1a8-5137-46f3-adfe-0fb37fa52d46" />
<br>

#### 13.	C-F 위치의 레지스터에서 값을 직접 입력하면서 LED의 상태를 확인하고, 레지스터의 위치와 비교해보면서 동작시켜 봅니다.<br>
<img width="804" height="590" alt="GPIO_Register_008" src="https://github.com/user-attachments/assets/85cb1220-d9d6-44d6-9d2a-4e46640ce6ca" />
