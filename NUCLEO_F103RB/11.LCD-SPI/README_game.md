# LCD-SPI-Game

<img width="306" height="233" alt="132" src="https://github.com/user-attachments/assets/7f6424ec-fb5b-441d-ab0e-0a9dc575bf6c" />
<br>

| 쉴드 보드 |  쉴드 보드 - STM32F103 NUCLEO Pinmap | 
|:-------:|:-------:|
| <img width="600" height="400" alt="Sheild-002" src="https://github.com/user-attachments/assets/29bdd8e8-3e94-45da-87f4-426694f32622" /> | <img width="600" height="400" alt="Sheild-001" src="https://github.com/user-attachments/assets/9df5b8c3-d81a-4026-9f86-67fa4dde1e38" /> | 


<img width="600" height="600" alt="F103RB-pin" src="https://github.com/user-attachments/assets/45bb557f-9517-419d-b45c-81a92869bac0" />
<br>

---

## 전체 프로젝트 구성:

### 1. main.c (위 코드)
STM32 CubeIDE에서 프로젝트의 main.c 파일을 위 코드로 교체하세요.

### 2. 추가로 필요한 설정:
 * CubeMX에서 설정할 사항:
 * System Clock: HSE 8MHz → PLL × 8 = 64MHz
    * SPI1 활성화:
       * PA5 (SPI1_SCK)
       * PA7 (SPI1_MOSI)
   * GPIO 출력핀 설정:
      * PA1 (LCD_RES)
      * PA6 (LCD_DC)
      * PB6 (LCD_CS)

### 3. 하드웨어 연결:
   * LCD 핀    →  STM32F103 핀
   * BLK      →  NC (연결 안함)
   * CS       →  PB6
   * DC       →  PA6
   * RES      →  PA1
   * SDA(MOSI)→  PA7
   * SCL(SCK) →  PA5
   * VCC      →  3.3V
   * GND      →  GND

### 4. 코드 특징:
   * 완전한 ST7735S 초기화 시퀀스
   * 8x8 폰트 포함 (ASCII 32-126)
   * RGB565 컬러 지원
   * 텍스트 출력 기능
   * 64MHz 시스템 클럭 설정

### 5. 실행 결과:
   * LCD에 다음과 같이 표시됩니다:
      * "Hello" (흰색)
      * "World!" (흰색)
      * "STM32F103" (녹색)
      * "ST7735S" (시안색)
      * "80x160" (노란색)

---

## ioc

```
#MicroXplorer Configuration settings - do not modify
CAD.formats=
CAD.pinconfig=
CAD.provider=
Dma.Request0=SPI1_TX
Dma.RequestsNb=1
Dma.SPI1_TX.0.Direction=DMA_MEMORY_TO_PERIPH
Dma.SPI1_TX.0.Instance=DMA1_Channel3
Dma.SPI1_TX.0.MemDataAlignment=DMA_MDATAALIGN_HALFWORD
Dma.SPI1_TX.0.MemInc=DMA_MINC_ENABLE
Dma.SPI1_TX.0.Mode=DMA_NORMAL
Dma.SPI1_TX.0.PeriphDataAlignment=DMA_PDATAALIGN_HALFWORD
Dma.SPI1_TX.0.PeriphInc=DMA_PINC_DISABLE
Dma.SPI1_TX.0.Priority=DMA_PRIORITY_LOW
Dma.SPI1_TX.0.RequestParameters=Instance,Direction,PeriphInc,MemInc,PeriphDataAlignment,MemDataAlignment,Mode,Priority
File.Version=6
KeepUserPlacement=false
Mcu.CPN=STM32F103RBT6
Mcu.Family=STM32F1
Mcu.IP0=DMA
Mcu.IP1=NVIC
Mcu.IP2=RCC
Mcu.IP3=SPI1
Mcu.IP4=SYS
Mcu.IP5=USART2
Mcu.IPNb=6
Mcu.Name=STM32F103R(8-B)Tx
Mcu.Package=LQFP64
Mcu.Pin0=PC13-TAMPER-RTC
Mcu.Pin1=PC14-OSC32_IN
Mcu.Pin10=PA7
Mcu.Pin11=PA13
Mcu.Pin12=PA14
Mcu.Pin13=PB3
Mcu.Pin14=PB6
Mcu.Pin15=VP_SYS_VS_Systick
Mcu.Pin2=PC15-OSC32_OUT
Mcu.Pin3=PD0-OSC_IN
Mcu.Pin4=PD1-OSC_OUT
Mcu.Pin5=PA1
Mcu.Pin6=PA2
Mcu.Pin7=PA3
Mcu.Pin8=PA5
Mcu.Pin9=PA6
Mcu.PinsNb=16
Mcu.ThirdPartyNb=0
Mcu.UserConstants=
Mcu.UserName=STM32F103RBTx
MxCube.Version=6.14.1
MxDb.Version=DB.6.0.141
NVIC.BusFault_IRQn=true\:0\:0\:false\:false\:true\:true\:false\:false
NVIC.DMA1_Channel3_IRQn=true\:0\:0\:false\:false\:true\:false\:true\:true
NVIC.DebugMonitor_IRQn=true\:0\:0\:false\:false\:true\:true\:false\:false
NVIC.EXTI15_10_IRQn=true\:0\:0\:false\:false\:true\:true\:true\:true
NVIC.ForceEnableDMAVector=true
NVIC.HardFault_IRQn=true\:0\:0\:false\:false\:true\:true\:false\:false
NVIC.MemoryManagement_IRQn=true\:0\:0\:false\:false\:true\:true\:false\:false
NVIC.NonMaskableInt_IRQn=true\:0\:0\:false\:false\:true\:true\:false\:false
NVIC.PendSV_IRQn=true\:0\:0\:false\:false\:true\:true\:false\:false
NVIC.PriorityGroup=NVIC_PRIORITYGROUP_4
NVIC.SVCall_IRQn=true\:0\:0\:false\:false\:true\:true\:false\:false
NVIC.SysTick_IRQn=true\:0\:0\:false\:false\:true\:true\:true\:false
NVIC.UsageFault_IRQn=true\:0\:0\:false\:false\:true\:true\:false\:false
PA1.GPIOParameters=GPIO_Label
PA1.GPIO_Label=LCD_RES
PA1.Locked=true
PA1.Signal=GPIO_Output
PA13.GPIOParameters=GPIO_Label
PA13.GPIO_Label=TMS
PA13.Locked=true
PA13.Mode=Serial_Wire
PA13.Signal=SYS_JTMS-SWDIO
PA14.GPIOParameters=GPIO_Label
PA14.GPIO_Label=TCK
PA14.Locked=true
PA14.Mode=Serial_Wire
PA14.Signal=SYS_JTCK-SWCLK
PA2.GPIOParameters=GPIO_Speed,GPIO_PuPd,GPIO_Label,GPIO_Mode
PA2.GPIO_Label=USART_TX
PA2.GPIO_Mode=GPIO_MODE_AF_PP
PA2.GPIO_PuPd=GPIO_NOPULL
PA2.GPIO_Speed=GPIO_SPEED_FREQ_LOW
PA2.Locked=true
PA2.Mode=Asynchronous
PA2.Signal=USART2_TX
PA3.GPIOParameters=GPIO_Speed,GPIO_PuPd,GPIO_Label,GPIO_Mode
PA3.GPIO_Label=USART_RX
PA3.GPIO_Mode=GPIO_MODE_AF_PP
PA3.GPIO_PuPd=GPIO_NOPULL
PA3.GPIO_Speed=GPIO_SPEED_FREQ_LOW
PA3.Locked=true
PA3.Mode=Asynchronous
PA3.Signal=USART2_RX
PA5.Mode=Simplex_Bidirectional_Master
PA5.Signal=SPI1_SCK
PA6.GPIOParameters=GPIO_Label
PA6.GPIO_Label=LCD_DC
PA6.Locked=true
PA6.Signal=GPIO_Output
PA7.Mode=Simplex_Bidirectional_Master
PA7.Signal=SPI1_MOSI
PB3.GPIOParameters=GPIO_Label
PB3.GPIO_Label=SWO
PB3.Locked=true
PB3.Signal=SYS_JTDO-TRACESWO
PB6.GPIOParameters=GPIO_Label
PB6.GPIO_Label=LCD_CS
PB6.Locked=true
PB6.Signal=GPIO_Output
PC13-TAMPER-RTC.GPIOParameters=GPIO_PuPd,GPIO_Label
PC13-TAMPER-RTC.GPIO_Label=B1 [Blue PushButton]
PC13-TAMPER-RTC.GPIO_PuPd=GPIO_NOPULL
PC13-TAMPER-RTC.Locked=true
PC13-TAMPER-RTC.Signal=GPXTI13
PC14-OSC32_IN.Locked=true
PC14-OSC32_IN.Signal=RCC_OSC32_IN
PC15-OSC32_OUT.Locked=true
PC15-OSC32_OUT.Signal=RCC_OSC32_OUT
PD0-OSC_IN.Locked=true
PD0-OSC_IN.Signal=RCC_OSC_IN
PD1-OSC_OUT.Locked=true
PD1-OSC_OUT.Signal=RCC_OSC_OUT
PinOutPanel.RotationAngle=0
ProjectManager.AskForMigrate=true
ProjectManager.BackupPrevious=false
ProjectManager.CompilerLinker=GCC
ProjectManager.CompilerOptimize=6
ProjectManager.ComputerToolchain=false
ProjectManager.CoupleFile=false
ProjectManager.CustomerFirmwarePackage=
ProjectManager.DefaultFWLocation=true
ProjectManager.DeletePrevious=true
ProjectManager.DeviceId=STM32F103RBTx
ProjectManager.FirmwarePackage=STM32Cube FW_F1 V1.8.7
ProjectManager.FreePins=false
ProjectManager.HalAssertFull=false
ProjectManager.HeapSize=0x200
ProjectManager.KeepUserCode=true
ProjectManager.LastFirmware=true
ProjectManager.LibraryCopy=1
ProjectManager.MainLocation=Core/Src
ProjectManager.NoMain=false
ProjectManager.PreviousToolchain=
ProjectManager.ProjectBuild=false
ProjectManager.ProjectFileName=LCD-SPI-Game.ioc
ProjectManager.ProjectName=LCD-SPI-Game
ProjectManager.ProjectStructure=
ProjectManager.RegisterCallBack=
ProjectManager.StackSize=0x400
ProjectManager.TargetToolchain=STM32CubeIDE
ProjectManager.ToolChainLocation=
ProjectManager.UAScriptAfterPath=
ProjectManager.UAScriptBeforePath=
ProjectManager.UnderRoot=true
ProjectManager.functionlistsort=1-SystemClock_Config-RCC-false-HAL-false,2-MX_GPIO_Init-GPIO-false-HAL-true,3-MX_DMA_Init-DMA-false-HAL-true,4-MX_USART2_UART_Init-USART2-false-HAL-true,5-MX_SPI1_Init-SPI1-false-HAL-true
RCC.ADCFreqValue=32000000
RCC.AHBFreq_Value=64000000
RCC.APB1CLKDivider=RCC_HCLK_DIV2
RCC.APB1Freq_Value=32000000
RCC.APB1TimFreq_Value=64000000
RCC.APB2Freq_Value=64000000
RCC.APB2TimFreq_Value=64000000
RCC.FCLKCortexFreq_Value=64000000
RCC.FamilyName=M
RCC.HCLKFreq_Value=64000000
RCC.IPParameters=ADCFreqValue,AHBFreq_Value,APB1CLKDivider,APB1Freq_Value,APB1TimFreq_Value,APB2Freq_Value,APB2TimFreq_Value,FCLKCortexFreq_Value,FamilyName,HCLKFreq_Value,MCOFreq_Value,PLLCLKFreq_Value,PLLMCOFreq_Value,PLLMUL,RTCClockSelection,RTCFreq_Value,SYSCLKFreq_VALUE,SYSCLKSource,TimSysFreq_Value,USBFreq_Value,VCOOutput2Freq_Value
RCC.MCOFreq_Value=64000000
RCC.PLLCLKFreq_Value=64000000
RCC.PLLMCOFreq_Value=32000000
RCC.PLLMUL=RCC_PLL_MUL16
RCC.RTCClockSelection=RCC_RTCCLKSOURCE_LSE
RCC.RTCFreq_Value=32768
RCC.SYSCLKFreq_VALUE=64000000
RCC.SYSCLKSource=RCC_SYSCLKSOURCE_PLLCLK
RCC.TimSysFreq_Value=64000000
RCC.USBFreq_Value=64000000
RCC.VCOOutput2Freq_Value=4000000
SH.GPXTI13.0=GPIO_EXTI13
SH.GPXTI13.ConfNb=1
SPI1.BaudRatePrescaler=SPI_BAUDRATEPRESCALER_4
SPI1.CalculateBaudRate=16.0 MBits/s
SPI1.Direction=SPI_DIRECTION_1LINE
SPI1.IPParameters=VirtualType,Mode,Direction,BaudRatePrescaler,CalculateBaudRate
SPI1.Mode=SPI_MODE_MASTER
SPI1.VirtualType=VM_MASTER
USART2.IPParameters=VirtualMode
USART2.VirtualMode=VM_ASYNC
VP_SYS_VS_Systick.Mode=SysTick
VP_SYS_VS_Systick.Signal=SYS_VS_Systick
board=NUCLEO-F103RB
boardIOC=true
isbadioc=false
```

---

## stm32f1xx_it.c

```
void DMA1_Channel3_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel3_IRQn 0 */

  /* USER CODE END DMA1_Channel3_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_spi1_tx);
  /* USER CODE BEGIN DMA1_Channel3_IRQn 1 */

  /* USER CODE END DMA1_Channel3_IRQn 1 */
}
```


---

## main.c

```c
/* USER CODE BEGIN Includes */
#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
/* USER CODE END Includes */
```

```c
/* USER CODE BEGIN PD */

// ST7735S Commands
#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09
#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13
#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E
#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36
#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6
#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5
#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD
#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

// LCD dimensions (80x160 portrait)

// Colors (RGB565)
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0


/* 디스플레이 해상도 규격 */
#define LCD_WIDTH 80
#define LCD_HEIGHT 160
/* 16비트 모드 다이렉트 RGB565 컬러 매크로 (바이트 스왑 필요 없음) */
#define RGB565(r, g, b) (uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))
#define COLOR_ROAD RGB565(45, 45, 45)
#define COLOR_GRASS_1 RGB565(20, 140, 20)
#define COLOR_GRASS_2 RGB565(10, 100, 10)
#define COLOR_WHITE RGB565(240, 240, 240)
#define COLOR_MYCAR RGB565(255, 10, 10)
#define COLOR_ENEMY RGB565(10, 50, 255)
/* DMA 동기화 플래그 변수 */
volatile uint8_t dma_tx_complete = 1;
/* USER CODE END PD */
```

```c
/* USER CODE BEGIN PM */

// Pin control macros
#define LCD_CS_LOW()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET)
#define LCD_CS_HIGH()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET)
#define LCD_DC_LOW()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET)
#define LCD_DC_HIGH()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET)
#define LCD_RES_LOW()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET)
#define LCD_RES_HIGH() HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET)
/* USER CODE END PM */
```

```c
/* USER CODE BEGIN PV */

// Simple 8x8 font (ASCII 32-127) - subset for demonstration
static const uint8_t font8x8[][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // ' ' (Space)
    {0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00}, // '!'
    {0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // '"'
    {0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00}, // '#'
    {0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00}, // '$'
    {0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00}, // '%'
    {0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00}, // '&'
    {0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00}, // '''
    {0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00}, // '('
    {0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00}, // ')'
    {0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00}, // '*'
    {0x00, 0x0C, 0x0C, 0x3F, 0x0C, 0x0C, 0x00, 0x00}, // '+'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x06, 0x00}, // ','
    {0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00}, // '-'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00}, // '.'
    {0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00}, // '/'
    {0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00}, // '0'
    {0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00}, // '1'
    {0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00}, // '2'
    {0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00}, // '3'
    {0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00}, // '4'
    {0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00}, // '5'
    {0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00}, // '6'
    {0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00}, // '7'
    {0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00}, // '8'
    {0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00}, // '9'
    {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00}, // ':'
    {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x06, 0x00}, // ';'
    {0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00}, // '<'
    {0x00, 0x00, 0x3F, 0x00, 0x00, 0x3F, 0x00, 0x00}, // '='
    {0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00}, // '>'
    {0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00}, // '?'
    {0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00}, // '@'
    {0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00}, // 'A'
    {0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00}, // 'B'
    {0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00}, // 'C'
    {0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00}, // 'D'
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00}, // 'E'
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00}, // 'F'
    {0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00}, // 'G'
    {0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00}, // 'H'
    {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // 'I'
    {0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00}, // 'J'
    {0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00}, // 'K'
    {0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00}, // 'L'
    {0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00}, // 'M'
    {0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00}, // 'N'
    {0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00}, // 'O'
    {0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00}, // 'P'
    {0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00}, // 'Q'
    {0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00}, // 'R'
    {0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00}, // 'S'
    {0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // 'T'
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00}, // 'U'
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00}, // 'V'
    {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00}, // 'W'
    {0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00}, // 'X'
    {0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00}, // 'Y'
    {0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00}, // 'Z'
    {0x1E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1E, 0x00}, // '['
    {0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00}, // '\'
    {0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x00}, // ']'
    {0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00}, // '^'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF}, // '_'
    {0x0C, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00}, // '`'
    {0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00}, // 'a'
    {0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00}, // 'b'
    {0x00, 0x00, 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x00}, // 'c'
    {0x38, 0x30, 0x30, 0x3e, 0x33, 0x33, 0x6E, 0x00}, // 'd'
    {0x00, 0x00, 0x1E, 0x33, 0x3f, 0x03, 0x1E, 0x00}, // 'e'
    {0x1C, 0x36, 0x06, 0x0f, 0x06, 0x06, 0x0F, 0x00}, // 'f'
    {0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x1F}, // 'g'
    {0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x67, 0x00}, // 'h'
    {0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // 'i'
    {0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E}, // 'j'
    {0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x67, 0x00}, // 'k'
    {0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // 'l'
    {0x00, 0x00, 0x33, 0x7F, 0x7F, 0x6B, 0x63, 0x00}, // 'm'
    {0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x00}, // 'n'
    {0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00}, // 'o'
    {0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x0F}, // 'p'
    {0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x78}, // 'q'
    {0x00, 0x00, 0x3B, 0x6E, 0x66, 0x06, 0x0F, 0x00}, // 'r'
    {0x00, 0x00, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x00}, // 's'
    {0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x2C, 0x18, 0x00}, // 't'
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00}, // 'u'
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00}, // 'v'
    {0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00}, // 'w'
    {0x00, 0x00, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x00}, // 'x'
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x1F}, // 'y'
    {0x00, 0x00, 0x3F, 0x19, 0x0C, 0x26, 0x3F, 0x00}, // 'z'
    {0x38, 0x0C, 0x0C, 0x07, 0x0C, 0x0C, 0x38, 0x00}, // '{'
    {0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00}, // '|'
    {0x07, 0x0C, 0x0C, 0x38, 0x0C, 0x0C, 0x07, 0x00}, // '}'
    {0x6E, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // '~'
};

/* USER CODE END PV */
```

```c
/* USER CODE BEGIN 0 */
void LCD_WriteCommand(uint8_t cmd) {
    LCD_CS_LOW();
    LCD_DC_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    LCD_CS_HIGH();
}

void LCD_WriteData(uint8_t data) {
    LCD_CS_LOW();
    LCD_DC_HIGH();
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
    LCD_CS_HIGH();
}

void LCD_WriteData16(uint16_t data) {
    uint8_t buffer[2];
    buffer[0] = (data >> 8) & 0xFF;
    buffer[1] = data & 0xFF;

    LCD_CS_LOW();
    LCD_DC_HIGH();
    HAL_SPI_Transmit(&hspi1, buffer, 2, HAL_MAX_DELAY);
    LCD_CS_HIGH();
}



void LCD_SetWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    // ★ 80x160 (90도 회전 모드 0x60) 전용 하드웨어 물리 오프셋 고정 계산 ★
    uint16_t real_x0 = x0 + 26;
    uint16_t real_x1 = x1 + 26;
    uint16_t real_y0 = y0 + 1;
    uint16_t real_y1 = y1 + 1;

    // Column address set (X축 스캔 영역 지정)
    LCD_WriteCommand(ST7735_CASET);
    LCD_WriteData((real_x0 >> 8) & 0xFF);
    LCD_WriteData(real_x0 & 0xFF);
    LCD_WriteData((real_x1 >> 8) & 0xFF);
    LCD_WriteData(real_x1 & 0xFF);

    // Row address set (Y축 스캔 영역 지정)
    LCD_WriteCommand(ST7735_RASET);
    LCD_WriteData((real_y0 >> 8) & 0xFF);
    LCD_WriteData(real_y0 & 0xFF);
    LCD_WriteData((real_y1 >> 8) & 0xFF);
    LCD_WriteData(real_y1 & 0xFF);

    // Write to RAM 명령어를 날려 이후 들어오는 SPI 데이터가 픽셀로 채워지도록 유도
    LCD_WriteCommand(ST7735_RAMWR);
}

void LCD_DrawPixel(uint8_t x, uint8_t y, uint16_t color) {
    if(x >= LCD_WIDTH || y >= LCD_HEIGHT) return;

    LCD_SetWindow(x, y, x, y);
    LCD_WriteData16(color);
}

/* 초고속 화면 채우기 함수 (80픽셀 단위 묶음 전송 방식으로 오버헤드 제거) */
void LCD_Fill(uint16_t color) {
    LCD_SetWindow(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

    uint8_t line_buffer[LCD_WIDTH * 2];
    uint8_t hi = (color >> 8) & 0xFF;
    uint8_t lo = color & 0xFF;

    // 1라인 분량의 버퍼를 미리 채워둠
    for (uint16_t i = 0; i < LCD_WIDTH; i++) {
        line_buffer[i * 2]     = hi;
        line_buffer[i * 2 + 1] = lo;
    }

    LCD_CS_LOW();
    LCD_DC_HIGH();

    // 세로 한 줄 한 줄씩 통째로 고속 스트리밍 송신
    for (uint16_t y = 0; y < LCD_HEIGHT; y++) {
        HAL_SPI_Transmit(&hspi1, line_buffer, LCD_WIDTH * 2, HAL_MAX_DELAY);
    }

    LCD_CS_HIGH();
}

void LCD_DrawChar(uint8_t x, uint8_t y, char ch, uint16_t color, uint16_t bg_color) {
    if(ch < 32 || ch > 126) ch = 32;

    const uint8_t* font_char = font8x8[ch - 32];

    for(uint8_t i = 0; i < 8; i++) {
        uint8_t line = font_char[i];
        for(uint8_t j = 0; j < 8; j++) {
            if(line & (0x01 << j)) {
                LCD_DrawPixel(x + j, y + i, color);
            } else {
                LCD_DrawPixel(x + j, y + i, bg_color);
            }
        }
    }
}

void LCD_DrawString(uint8_t x, uint8_t y, const char* str, uint16_t color, uint16_t bg_color) {
    uint8_t orig_x = x;

    while(*str) {
        if(*str == '\n') {
            y += 8;
            x = orig_x;
        } else if(*str == '\r') {
            x = orig_x;
        } else {
            if(x + 8 > LCD_WIDTH) {
                x = orig_x;
                y += 8;
            }
            if(y + 8 > LCD_HEIGHT) {
                break;
            }

            LCD_DrawChar(x, y, *str, color, bg_color);
            x += 8;
        }
        str++;
    }
}

/* SPI 제어 모드 동적 전환 함수 */
void ST7735_SetSPI_BitMode(uint32_t datasize) {
	hspi1.Instance->CR1 &= ~SPI_CR1_SPE; // SPI 비활성화
	hspi1.Init.DataSize = datasize; // SPI_DATASIZE_8BIT 또는 SPI_DATASIZE_16BIT
	HAL_SPI_Init(&hspi1);
	hspi1.Instance->CR1 |= SPI_CR1_SPE; // SPI 재활성화
}
void ST7735_WriteCommand(uint8_t cmd) {
	ST7735_SetSPI_BitMode(SPI_DATASIZE_8BIT);
	LCD_DC_LOW();
	LCD_CS_LOW();
	HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
	LCD_CS_HIGH();
}
void ST7735_WriteData8(uint8_t data) {
	ST7735_SetSPI_BitMode(SPI_DATASIZE_8BIT);
	LCD_DC_HIGH();
	LCD_CS_LOW();
	HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
	LCD_CS_HIGH();
}
void ST7735_SetAddressWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
	uint8_t offset_x = 26; // 80x160 화면 보정용 패널 오프셋 값
	uint8_t offset_y = 1;
	x0 += offset_x; x1 += offset_x;
	y0 += offset_y; y1 += offset_y;
	ST7735_WriteCommand(0x2A); // Column Address Set
	ST7735_WriteData8(0x00); ST7735_WriteData8(x0);
	ST7735_WriteData8(0x00); ST7735_WriteData8(x1);
	ST7735_WriteCommand(0x2B); // Row Address Set
	ST7735_WriteData8(0x00); ST7735_WriteData8(y0);
	ST7735_WriteData8(0x00); ST7735_WriteData8(y1);
	ST7735_WriteCommand(0x2C); // Memory Write Command
}

void ST7735_Init(void) {
    // 1. 하드웨어 리셋 확실히 수행
    LCD_CS_HIGH();
    LCD_RES_LOW();
    HAL_Delay(50);
    LCD_RES_HIGH();
    HAL_Delay(50);

    ST7735_WriteCommand(0x01); // Software Reset
    HAL_Delay(120);

    ST7735_WriteCommand(0x11); // Sleep Out
    HAL_Delay(120);

    // 2. 프레임 레이트 및 디스플레이 구동 속도 안정화 설정
    ST7735_WriteCommand(0xB1); // Frame Rate Control (In normal mode)
    ST7735_WriteData8(0x01); ST7735_WriteData8(0x2C); ST7735_WriteData8(0x2D);

    ST7735_WriteCommand(0xB2); // Frame Rate Control (In Idle mode)
    ST7735_WriteData8(0x01); ST7735_WriteData8(0x2C); ST7735_WriteData8(0x2D);

    ST7735_WriteCommand(0xB3); // Frame Rate Control (In Partial mode)
    ST7735_WriteData8(0x01); ST7735_WriteData8(0x2C); ST7735_WriteData8(0x2D);
    ST7735_WriteData8(0x01); ST7735_WriteData8(0x2C); ST7735_WriteData8(0x2D);

    ST7735_WriteCommand(0xB4); // Display Inversion Control
    ST7735_WriteData8(0x07);   // No Inversion

    // 3. 파워 컨트롤 내장 전압 설정 (안정적 구동 목적)
    ST7735_WriteCommand(0xC0); // Power Control 1
    ST7735_WriteData8(0xA2); ST7735_WriteData8(0x02); ST7735_WriteData8(0x84);
    ST7735_WriteCommand(0xC1); // Power Control 2
    ST7735_WriteData8(0xC5);
    ST7735_WriteCommand(0xC2); // Power Control 3
    ST7735_WriteData8(0x0A); ST7735_WriteData8(0x00);
    ST7735_WriteCommand(0xC3); // Power Control 4
    ST7735_WriteData8(0x8A); ST7735_WriteData8(0x2A);
    ST7735_WriteCommand(0xC4); // Power Control 5
    ST7735_WriteData8(0x8A); ST7735_WriteData8(0xEE);

    ST7735_WriteCommand(0xC5); // VCOM Control 1
    ST7735_WriteData8(0x0E);

    // 4. 픽셀 포맷 및 화면 방향 결정
    ST7735_WriteCommand(0x3A); // Interface Pixel Format
    ST7735_WriteData8(0x05);   // 16-bit/pixel (RGB565)

    ST7735_WriteCommand(0x36); // MADCTL (Memory Data Access Control)
    ST7735_WriteData8(0xC8);   // 일반적인 80x160 모듈의 상하 반전 및 RGB 정렬 보정값 (필요시 0x00 등으로 변경)

    ST7735_WriteCommand(0x29); // Display ON
    HAL_Delay(100);
}

void LCD_Init(void) {
    // 1. 하드웨어 리셋 (확실하게 파워 온 딜레이 부여)
    LCD_CS_HIGH();
    LCD_RES_LOW();
    HAL_Delay(50);
    LCD_RES_HIGH();
    HAL_Delay(50);

    // Software reset
    LCD_WriteCommand(ST7735_SWRESET);
    HAL_Delay(120);

    // Out of sleep mode
    LCD_WriteCommand(ST7735_SLPOUT);
    HAL_Delay(120);

    // Frame rate control - normal mode
    LCD_WriteCommand(ST7735_FRMCTR1);
    LCD_WriteData(0x01); LCD_WriteData(0x2C); LCD_WriteData(0x2D);

    // Frame rate control - idle mode
    LCD_WriteCommand(ST7735_FRMCTR2);
    LCD_WriteData(0x01); LCD_WriteData(0x2C); LCD_WriteData(0x2D);

    // Frame rate control - partial mode
    LCD_WriteCommand(ST7735_FRMCTR3);
    LCD_WriteData(0x01); LCD_WriteData(0x2C); LCD_WriteData(0x2D);
    LCD_WriteData(0x01); LCD_WriteData(0x2C); LCD_WriteData(0x2D);

    // Display inversion control
    LCD_WriteCommand(ST7735_INVCTR);
    LCD_WriteData(0x07);

    // Power control (패널 내 구동 전압 안정화)
    LCD_WriteCommand(ST7735_PWCTR1);
    LCD_WriteData(0xA2); LCD_WriteData(0x02); LCD_WriteData(0x84);

    LCD_WriteCommand(ST7735_PWCTR2);
    LCD_WriteData(0xC5);

    LCD_WriteCommand(ST7735_PWCTR3);
    LCD_WriteData(0x0A); LCD_WriteData(0x00);

    LCD_WriteCommand(ST7735_PWCTR4);
    LCD_WriteData(0x8A); LCD_WriteData(0x2A);

    LCD_WriteCommand(ST7735_PWCTR5);
    LCD_WriteData(0x8A); LCD_WriteData(0xEE);

    // VCOM control
    LCD_WriteCommand(ST7735_VMCTR1);
    LCD_WriteData(0x0E);

    // 디스플레이 컬러 반전 제어 (화면 색상이 반전되어 나오면 ST7735_INVON으로 변경하세요)
    LCD_WriteCommand(ST7735_INVOFF);

    // Memory access control (가로/세로 화면 회정 레이아웃 설정)
    LCD_WriteCommand(ST7735_MADCTL);
    LCD_WriteData(0x60); // 90도 회전 + X축 미러링 (가로 해상도 160, 세로 해상도 80 기준 최적)

    // Color mode: 16-bit color (RGB565 포맷 강제 지정)
    LCD_WriteCommand(ST7735_COLMOD);
    LCD_WriteData(0x05);

    // 전체 가로 세로 하드웨어 초기 윈도우 한계 지정
    LCD_SetWindow(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

    // Normal display on
    LCD_WriteCommand(ST7735_NORON);
    HAL_Delay(10);

    // Main screen turn on
    LCD_WriteCommand(ST7735_DISPON);
    HAL_Delay(100);
}

/* DMA 송신 완료 인터럽트 핸들러 콜백 백엔드 */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
	if (hspi->Instance == SPI1) {
		dma_tx_complete = 1;
	}
}

/* 엔티티 상태 변수 */
int16_t car_player_x = 40;
int16_t car_enemy_x = 35;
int16_t car_enemy_y = -15;
uint16_t track_scroll_y = 0;

void Reset_Enemy_Car(void) {
	car_enemy_y = -15;
	car_enemy_x = 20 + (rand() % 40);
}

void Process_Game_Physics(void) {
	track_scroll_y++;
	car_enemy_y += 3;

	if (car_enemy_y > LCD_HEIGHT) {
		Reset_Enemy_Car();
	}

	static int8_t steer_dir = 1;
	car_player_x += steer_dir;
	if (car_player_x > 62) steer_dir = -1;
	if (car_player_x < 18) steer_dir = 1;
}

static int sin_lookup(int t) {
	t = t & 63;
	if (t < 16) return t;
	if (t < 32) return 16 - (t - 16);
	if (t < 48) return -(t - 32);
	return -(16 - (t - 48));
}

void Render_Racing_Frame(void) {
	uint8_t line_buffer[LCD_WIDTH * 2];

	LCD_SetWindow(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

	LCD_CS_LOW();
	LCD_DC_HIGH();

	for (int y = 0; y < LCD_HEIGHT; y++) {
		int road_width = 24 + (y / 3);
		int road_center = 40 + sin_lookup((y + track_scroll_y) * 2);

		for (int x = 0; x < LCD_WIDTH; x++) {
			uint16_t color;

			if (x >= (road_center - road_width / 2) && x <= (road_center + road_width / 2)) {
				if (x >= (road_center - 1) && x <= (road_center + 1) && ((y + track_scroll_y) % 24 < 12)) {
					color = COLOR_WHITE;
				} else {
					color = COLOR_ROAD;
				}
			} else {
				if (((x + y + (track_scroll_y >> 1)) % 16) < 8) {
					color = COLOR_GRASS_1;
				} else {
					color = COLOR_GRASS_2;
				}
			}

			if (y >= car_enemy_y && y < (car_enemy_y + 12)) {
				if (x >= (car_enemy_x - 4) && x < (car_enemy_x + 4)) {
					color = COLOR_ENEMY;
				}
			}
			if (y >= 132 && y < 146) {
				if (x >= (car_player_x - 5) && x < (car_player_x + 5)) {
					color = COLOR_MYCAR;
				}
			}

			line_buffer[x * 2] = (color >> 8) & 0xFF;
			line_buffer[x * 2 + 1] = color & 0xFF;
		}

		HAL_SPI_Transmit(&hspi1, line_buffer, LCD_WIDTH * 2, HAL_MAX_DELAY);
	}

	LCD_CS_HIGH();
}

/* USER CODE END 0 */
```

```c
  /* USER CODE BEGIN 2 */
  srand(HAL_GetTick());

  LCD_Init();

  LCD_Fill(BLUE);
  HAL_Delay(500);

  LCD_DrawString(10, 30, "System Ready!", WHITE, BLUE);
  LCD_DrawString(10, 45, "STM32F103 OK", YELLOW, BLUE);
  HAL_Delay(5000);
  /* USER CODE END 2 */
```

```c
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  LCD_Fill(RED);
  LCD_DrawString(10, 70, "Game Start!", WHITE, RED);
  HAL_Delay(500);

  uint32_t frame_count = 0;
  while (1)
  {
	  Process_Game_Physics();
	  Render_Racing_Frame();

	  frame_count++;
	  if (frame_count % 60 == 0) {
		  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
	  }

	  HAL_Delay(16);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
```




