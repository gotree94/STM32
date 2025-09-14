# GPIO
- [https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf RM0008_STM32F101xx, STM32F102xx, STM32F103xx, STM32F105xx and STM32F107xx advanced Arm®-based 32-bit MCUs]

- 1.[Architecture](#Architecture)
- [GPIO 레지스터 구조](#gpio-레지스터-구조)
- 3.[Test](#Test)


## Architecture
<img width="1185" height="852" alt="GPIO_Register_010" src="https://github.com/user-attachments/assets/45a3773c-c552-4388-b9bb-01d889790867" />
<br>
<img width="1269" height="360" alt="GPIO_Register_011" src="https://github.com/user-attachments/assets/2eb0bbf5-16f7-4f15-a5b5-51e834e80615" />
<br>
   * 각 GPIO 핀은 소프트웨어로 출력(푸시풀 또는 오픈 드레인), 입력(풀업 또는 풀다운 여부 선택), 또는 주변 장치의 대체 기능으로 구성할 수 있습니다.<br>
   * 대부분의 GPIO 핀은 디지털 또는 아날로그 대체 기능과 공유됩니다. 모든 GPIO는 고전류를 지원합니다.<br>
   * I/O의 대체 기능 구성은 필요 시 특정 순서를 따라 잠글 수 있으며, 이는 I/O 레지스터에 불필요한 기록이 이루어지는 것을 방지하기 위함입니다.<br>
   * I/O는 APB2 버스에 연결되어 있으며, 최대 18 MHz의 토글 속도를 지원합니다.<br>
<br>
<img width="710" height="909" alt="GPIO_Register_012" src="https://github.com/user-attachments/assets/b48ca11a-e702-4809-b98a-dbe8e864ecfa" />
<br>

## Test
<img width="459" height="151" alt="GPIO_Register_001" src="https://github.com/user-attachments/assets/46223b7e-ec0a-41c5-86ec-ef612250b98a" />
<br>
<img width="486" height="69" alt="GPIO_Register_002" src="https://github.com/user-attachments/assets/43a3d8b7-8d07-493e-a005-b04296b30bb5" />
<br>
<img width="295" height="53" alt="GPIO_Register_003" src="https://github.com/user-attachments/assets/3919a347-9f9f-4f1a-b11f-37045c2d3514" />
<br>
<img width="875" height="296" alt="GPIO_Register_004" src="https://github.com/user-attachments/assets/5ad41fc9-1d63-43a1-a5d6-01bf8adaabff" />
<br>
<img width="1920" height="1080" alt="GPIO_Register_005" src="https://github.com/user-attachments/assets/bb5ddcaf-b887-4451-88ac-1212a51e3638" />
<br>
<img width="862" height="460" alt="GPIO_Register_006" src="https://github.com/user-attachments/assets/7c0644bb-55dc-4072-a00f-57b6e5544b2f" />
<br>
<img width="652" height="350" alt="GPIO_Register_007" src="https://github.com/user-attachments/assets/70b06a32-57ff-41a0-bfba-7a708df195d7" />
<br>
<img width="622" height="351" alt="GPIO_Register_008" src="https://github.com/user-attachments/assets/bddee6a3-5078-4ec3-966a-f55926d10c72" />
<br>
<img width="967" height="453" alt="GPIO_Register_009" src="https://github.com/user-attachments/assets/2258a120-7b56-4842-919f-4b8f4af537ad" />
<br>

