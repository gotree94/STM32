# 21.OV7670 - 103

<img width="406" height="379" alt="121" src="https://github.com/user-attachments/assets/97af8cfd-befb-4292-ab40-6b207d98cd10" />


## 1. 클럭 소스
* F411: HSI로 84MHz 가능
* F103: 일반적으로 외부 HSE 사용 (8MHz 크리스탈)

##  OV7670 XCLK 주파수 문제
* OV7670 지원 범위: 10~48MHz
* F411 출력: 21MHz ✅ 최적
* F103 출력: 32MHz ✅ 가능하지만 다소 높음

* F103에서 32MHz도 OV7670 스펙 내이므로 동작은 하지만, 21MHz가 더 안정적입니다.

<img width="453" height="451" alt="stm32f103" src="https://github.com/user-attachments/assets/c8c2d4af-5ba4-43ff-8ec5-fe5e54d08675" />
<br>
<img width="467" height="1174" alt="026" src="https://github.com/user-attachments/assets/8a4270a1-701d-4723-ba62-e8ebf5dae437" />
<br>
<img width="800" height="600" alt="001" src="https://github.com/user-attachments/assets/13cccac6-2bfa-44bc-a752-0504a97dd8f8" />
<br>
<img width="800" height="600" alt="002" src="https://github.com/user-attachments/assets/a1f7adc1-817d-453b-a14c-458a923ee2b8" />
<br>
<img width="800" height="600" alt="003" src="https://github.com/user-attachments/assets/a6d0cb28-707e-43b7-b554-bbfe80143b32" />
<br>
<img width="800" height="600" alt="004" src="https://github.com/user-attachments/assets/74ce7870-6438-4e65-bb8e-77bca29499f5" />
<br>
<img width="800" height="600" alt="005" src="https://github.com/user-attachments/assets/ba34d749-5219-44f7-a96f-a3bc84e43c48" />
<br>
<img width="800" height="600" alt="006" src="https://github.com/user-attachments/assets/70697c52-b2e6-4004-9935-0901b7ed0f6b" />
<br>

```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */
```

```c
/* USER CODE BEGIN PD */
// OV7670 I2C Address
#define OV7670_I2C_ADDR         0x42

// Key OV7670 Registers
#define OV7670_REG_PID          0x0A
#define OV7670_REG_VER          0x0B
#define OV7670_REG_MIDH         0x1C
#define OV7670_REG_MIDL         0x1D
#define OV7670_REG_COM1         0x04
#define OV7670_REG_COM7         0x12
#define OV7670_REG_COM10        0x15
#define OV7670_REG_CLKRC        0x11

// Pin definitions
#define I2C_SCL_PIN             GPIO_PIN_8
#define I2C_SDA_PIN             GPIO_PIN_9
#define I2C_GPIO_PORT           GPIOB
/* USER CODE END PD */
```

```c
/* USER CODE BEGIN 0 */

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
    if (ch == '\n')
        HAL_UART_Transmit(&huart2, (uint8_t*)"\r", 1, 0xFFFF);
    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, 0xFFFF);
    return ch;
}

/**
  * @brief  MCO (PA8) 클럭 출력 확인 및 설정
  */
static void Configure_MCO_Output(void)
{
    printf("\n=== MCO (XCLK) Configuration ===\n");
    printf("PA8 (MCO) configured in SystemClock_Config()\n");
    printf("Output: PLLCLK/2 = 32MHz\n");
    printf("OV7670 supports 10~48MHz range\n");
    printf("32MHz is acceptable for OV7670\n");
    printf("================================\n\n");

    printf("Waiting for XCLK stabilization (500ms)...\n");
    HAL_Delay(500);
    printf("✓ XCLK stable\n\n");
}

/**
  * @brief  XCLK 출력 소스 변경
  */
void Change_XCLK_Source(uint32_t source)
{
    printf("\n=== Changing XCLK Source ===\n");

    uint32_t mco_source;
    const char* source_name;
    const char* output_freq;

    switch(source) {
        case 1:
            mco_source = RCC_MCO1SOURCE_HSI;
            source_name = "HSI";
            output_freq = "8MHz";
            break;
        case 2:
            mco_source = RCC_MCO1SOURCE_HSE;
            source_name = "HSE";
            output_freq = "8MHz (if crystal present)";
            break;
        case 3:
            mco_source = RCC_MCO1SOURCE_SYSCLK;
            source_name = "SYSCLK";
            output_freq = "64MHz (too high!)";
            break;
        case 4:
        default:
            mco_source = RCC_MCO1SOURCE_PLLCLK;
            source_name = "PLLCLK/2";
            output_freq = "32MHz (recommended)";
            break;
    }

    HAL_RCC_MCOConfig(RCC_MCO, mco_source, RCC_MCODIV_1);

    printf("✓ XCLK changed to %s\n", source_name);
    printf("  Output: %s\n", output_freq);
    printf("============================\n\n");

    HAL_Delay(100);
}

HAL_StatusTypeDef OV7670_ReadRegister(uint8_t reg_addr, uint8_t *data)
{
    HAL_StatusTypeDef status;
    status = HAL_I2C_Master_Transmit(&hi2c1, OV7670_I2C_ADDR, &reg_addr, 1, 1000);
    if (status != HAL_OK) return status;
    status = HAL_I2C_Master_Receive(&hi2c1, OV7670_I2C_ADDR, data, 1, 1000);
    return status;
}

HAL_StatusTypeDef OV7670_WriteRegister(uint8_t reg_addr, uint8_t data)
{
    uint8_t buf[2] = {reg_addr, data};
    return HAL_I2C_Master_Transmit(&hi2c1, OV7670_I2C_ADDR, buf, 2, 1000);
}

void Force_I2C1_GPIO_Config(void)
{
    printf("\n=== Forcing I2C1 GPIO Config ===\n");

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();

    // I2C1 Remap 활성화 (PB8/PB9)
    __HAL_AFIO_REMAP_I2C1_ENABLE();

    GPIO_InitStruct.Pin = I2C_SCL_PIN | I2C_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(I2C_GPIO_PORT, &GPIO_InitStruct);

    printf("✓ PB8/PB9 configured as I2C1\n");
    printf("✓ I2C1 Remap enabled\n");
    printf("================================\n\n");
}

void I2C_CheckPinStatus(void)
{
    printf("\n=== I2C Configuration Check ===\n");

    printf("I2C1->CR1: 0x%04X ", (uint16_t)I2C1->CR1);
    printf("(PE: %s)\n", (I2C1->CR1 & I2C_CR1_PE) ? "Enabled" : "Disabled");

    printf("I2C1->SR1: 0x%04X\n", (uint16_t)I2C1->SR1);
    printf("I2C1->SR2: 0x%04X\n", (uint16_t)I2C1->SR2);

    if (RCC->APB2ENR & RCC_APB2ENR_IOPBEN) {
        printf("✓ GPIOB clock enabled\n");
    } else {
        printf("✗ GPIOB clock disabled\n");
    }

    if (RCC->APB1ENR & RCC_APB1ENR_I2C1EN) {
        printf("✓ I2C1 clock enabled\n");
    } else {
        printf("✗ I2C1 clock disabled\n");
    }

    if (AFIO->MAPR & AFIO_MAPR_I2C1_REMAP) {
        printf("✓ I2C1 Remap enabled (PB8/PB9)\n");
    } else {
        printf("⚠ I2C1 using default pins (PB6/PB7)\n");
    }

    printf("===============================\n\n");
}

void I2C_Scanner(void)
{
    printf("\n=== I2C Bus Scanner ===\n");
    printf("Scanning addresses 0x08 to 0x77...\n");

    uint8_t found = 0;

    for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
        if (HAL_I2C_IsDeviceReady(&hi2c1, addr << 1, 1, 100) == HAL_OK) {
            printf("✓ Device at 0x%02X (7-bit: 0x%02X)\n", addr << 1, addr);
            found++;

            if (addr == 0x21 || addr == 0x42 || addr == 0x60) {
                printf("  --> Possible OV7670!\n");
            }
        }
    }

    if (found == 0) {
        printf("❌ No devices found!\n");
        printf("Check: XCLK, I2C wiring, pullups\n");
    } else {
        printf("Total: %d device(s)\n", found);
    }
    printf("=======================\n\n");
}

void OV7670_TryMultipleAddresses(void)
{
    printf("\n=== OV7670 Address Test ===\n");

    uint8_t addresses[] = {0x42, 0x43, 0x60, 0x61};

    for (uint8_t i = 0; i < 4; i++) {
        uint8_t addr = addresses[i];
        printf("Testing 0x%02X...\n", addr);

        if (HAL_I2C_IsDeviceReady(&hi2c1, addr, 3, 100) == HAL_OK) {
            printf("  ✓ ACK received\n");

            uint8_t reg = 0x0A, pid = 0;
            if (HAL_I2C_Master_Transmit(&hi2c1, addr, &reg, 1, 1000) == HAL_OK) {
                if (HAL_I2C_Master_Receive(&hi2c1, addr, &pid, 1, 1000) == HAL_OK) {
                    printf("  PID: 0x%02X", pid);
                    if (pid == 0x76) {
                        printf(" <- OV7670!\n");

                        reg = 0x0B;
                        uint8_t ver = 0;
                        HAL_I2C_Master_Transmit(&hi2c1, addr, &reg, 1, 1000);
                        HAL_I2C_Master_Receive(&hi2c1, addr, &ver, 1, 1000);
                        printf("  VER: 0x%02X", ver);

                        if (ver == 0x73) {
                            printf(" <- CONFIRMED!\n");
                        } else {
                            printf("\n");
                        }
                    } else {
                        printf("\n");
                    }
                }
            }
        } else {
            printf("  ✗ No response\n");
        }
    }
    printf("===========================\n\n");
}

uint8_t OV7670_CheckID(void)
{
    uint8_t pid, ver, midh, midl;

    printf("\n=== OV7670 ID Check ===\n");

    if (OV7670_ReadRegister(OV7670_REG_PID, &pid) != HAL_OK) {
        printf("✗ Failed to read PID\n");
        return 0;
    }

    if (OV7670_ReadRegister(OV7670_REG_VER, &ver) != HAL_OK) {
        printf("✗ Failed to read VER\n");
        return 0;
    }

    if (OV7670_ReadRegister(OV7670_REG_MIDH, &midh) != HAL_OK) {
        printf("✗ Failed to read MIDH\n");
        return 0;
    }

    if (OV7670_ReadRegister(OV7670_REG_MIDL, &midl) != HAL_OK) {
        printf("✗ Failed to read MIDL\n");
        return 0;
    }

    printf("Product ID: 0x%02X%02X (Expected: 0x7673)\n", pid, ver);
    printf("Manufacturer ID: 0x%02X%02X (Expected: 0x7FA2)\n", midh, midl);

    if (pid == 0x76 && ver == 0x73 && midh == 0x7F && midl == 0xA2) {
        printf("✓ Valid OV7670 detected!\n");
        return 1;
    } else {
        printf("✗ Invalid sensor ID\n");
        return 0;
    }
}

void OV7670_CheckRegisters(void)
{
    uint8_t val;

    printf("\n=== OV7670 Registers ===\n");

    if (OV7670_ReadRegister(OV7670_REG_CLKRC, &val) == HAL_OK)
        printf("CLKRC (0x%02X): 0x%02X ✓\n", OV7670_REG_CLKRC, val);
    else
        printf("CLKRC (0x%02X): ✗\n", OV7670_REG_CLKRC);

    if (OV7670_ReadRegister(OV7670_REG_COM1, &val) == HAL_OK)
        printf("COM1  (0x%02X): 0x%02X ✓\n", OV7670_REG_COM1, val);
    else
        printf("COM1  (0x%02X): ✗\n", OV7670_REG_COM1);

    if (OV7670_ReadRegister(OV7670_REG_COM7, &val) == HAL_OK)
        printf("COM7  (0x%02X): 0x%02X ✓\n", OV7670_REG_COM7, val);
    else
        printf("COM7  (0x%02X): ✗\n", OV7670_REG_COM7);

    if (OV7670_ReadRegister(OV7670_REG_COM10, &val) == HAL_OK)
        printf("COM10 (0x%02X): 0x%02X ✓\n", OV7670_REG_COM10, val);
    else
        printf("COM10 (0x%02X): ✗\n", OV7670_REG_COM10);

    printf("========================\n\n");
}

void Complete_I2C1_Reinit(void)
{
    printf("\n=== I2C1 Reinitialization ===\n");

    printf("Step 1: Deinit...\n");
    HAL_I2C_DeInit(&hi2c1);

    printf("Step 2: Reset clocks...\n");
    __HAL_RCC_I2C1_CLK_DISABLE();
    __HAL_RCC_I2C1_FORCE_RESET();
    HAL_Delay(10);
    __HAL_RCC_I2C1_RELEASE_RESET();
    __HAL_RCC_I2C1_CLK_ENABLE();

    printf("Step 3: GPIO config...\n");
    Force_I2C1_GPIO_Config();

    printf("Step 4: Reinit I2C1...\n");
    if (HAL_I2C_Init(&hi2c1) == HAL_OK)
        printf("✓ I2C1 reinitialized\n");
    else
        printf("✗ Reinit failed\n");

    printf("=============================\n\n");
}

void PrintSystemInfo(void)
{
    printf("\n==============================\n");
    printf("  OV7670 I2C Diagnostic Tool\n");
    printf("==============================\n");
    printf("MCU:   STM32F103 @ 64MHz\n");
    printf("I2C1:  PB8(SCL), PB9(SDA)\n");
    printf("UART2: PA2(TX), PA3(RX)\n");
    printf("XCLK:  PA8 (MCO) @ 32MHz\n");
    printf("Addr:  0x%02X (7-bit: 0x%02X)\n", OV7670_I2C_ADDR, OV7670_I2C_ADDR >> 1);
    printf("==============================\n\n");
}

void ProcessCommand(char cmd)
{
    switch (cmd) {
        case 's':
        case 'S':
            I2C_Scanner();
            break;

        case 'p':
        case 'P':
            I2C_CheckPinStatus();
            break;

        case 'f':
        case 'F':
            Force_I2C1_GPIO_Config();
            break;

        case 'x':
        case 'X':
            Complete_I2C1_Reinit();
            break;

        case 'c':
        case 'C':
            Configure_MCO_Output();
            break;

        case '1':
        case '2':
        case '3':
        case '4':
            Change_XCLK_Source(cmd - '0');
            break;

        case 'm':
        case 'M':
            OV7670_TryMultipleAddresses();
            break;

        case 'i':
        case 'I':
            OV7670_CheckID();
            break;

        case 'r':
        case 'R':
            OV7670_CheckRegisters();
            break;

        case 'a':
        case 'A':
            printf("=== Full Diagnostic ===\n\n");
            Configure_MCO_Output();
            Complete_I2C1_Reinit();
            I2C_Scanner();
            OV7670_TryMultipleAddresses();
            if (OV7670_CheckID()) {
                OV7670_CheckRegisters();
            }
            break;

        case 'h':
        case 'H':
        case '?':
            printf("\n=== Commands ===\n");
            printf("S - I2C scan\n");
            printf("P - Check pins\n");
            printf("F - Force GPIO\n");
            printf("X - Reinit I2C\n");
            printf("C - Check XCLK\n");
            printf("1-4 - Change XCLK\n");
            printf("M - Try addresses\n");
            printf("I - Check ID\n");
            printf("R - Read regs\n");
            printf("A - All tests\n");
            printf("H - Help\n");
            printf("================\n\n");
            break;

        default:
            printf("Unknown: '%c' (0x%02X)\n", cmd, cmd);
            printf("Type 'H' for help\n\n");
            break;
    }
}

void UserApp_Main(void)
{
    PrintSystemInfo();

    printf("⚠ IMPORTANT: Connect OV7670 XCLK to PA8\n");
    printf("⚠ IMPORTANT: Use 4.7k pullups on SCL/SDA\n\n");

    Configure_MCO_Output();
    Complete_I2C1_Reinit();
    I2C_Scanner();
    OV7670_TryMultipleAddresses();

    printf("Type 'H' for help\n");
    printf("Ready for commands...\n\n");
}

/* USER CODE END 0 */
```

```c
  /* USER CODE BEGIN WHILE */
   UserApp_Main();

   uint8_t rx_buffer[1];

   while (1)
   {
     /* USER CODE END WHILE */

     /* USER CODE BEGIN 3 */

     if (HAL_UART_Receive(&huart2, rx_buffer, 1, 10) == HAL_OK) {
         char cmd = (char)rx_buffer[0];

         if (cmd != '\r' && cmd != '\n') {
             printf("%c\n", cmd);
         }

         ProcessCommand(cmd);
         printf("> ");
     }

     HAL_Delay(1);

   }
   /* USER CODE END 3 */
```

