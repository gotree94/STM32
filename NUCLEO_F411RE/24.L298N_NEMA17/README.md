# 21.OV7670

<img width="671" height="569" alt="nucleo-f411re-pinout" src="https://github.com/user-attachments/assets/c2b1cc58-ef59-46ea-8436-30d41cc2303a" />

<img width="659" height="1894" alt="005" src="https://github.com/user-attachments/assets/4364051b-172c-437a-b1c3-0c20cb9f7c8a" />

```
=== System Information ===
MCU: STM32F411
Clock Frequency: 84 MHz
I2C Interface: I2C1 (PB8=SCL, PB9=SDA)
UART Interface: UART2 (PA2=TX, PA3=RX)
XCLK Output: PA8 (MCO1) @ 21MHz
OV7670 I2C Address: 0x42 (7-bit: 0x21)
=========================

⚠️  IMPORTANT: Connect OV7670 XCLK to PA8!

Starting OV7670 diagnostic with XCLK generation...

=== Configuring MCO1 for XCLK ===
✓ MCO1 configured: PA8 output = 21MHz
  Source: PLLCLK (84MHz)
  Divider: /4
  Output: 21MHz (suitable for OV7670)
=================================

Waiting for XCLK stabilization (500ms)...
✓ XCLK should be stable now


=== Complete I2C1 Reinitialization ===
Step 1: Deinitializing I2C1...
Step 2: Resetting clocks...
Step 3: Forcing GPIO configuration...

=== Forcing I2C1 GPIO Configuration (PB8/PB9) ===
PB8 (SCL) Mode: 2 (should be 2), AF: 4 (should be 4)
PB9 (SDA) Mode: 2 (should be 2), AF: 4 (should be 4)
✓ GPIO configuration successful!
==============================================

Step 4: Reinitializing I2C1...
✓ I2C1 initialized successfully!
Step 5: Verifying configuration...

=== I2C Configuration Check (PB8/PB9) ===
I2C1->CR1: 0x0001 (PE: Enabled)
I2C1->SR1: 0x0000
I2C1->SR2: 0x0000
✓ GPIOB clock enabled
✓ I2C1 clock enabled
PB8 (SCL) Mode: Alternate Function (AF4)
PB9 (SDA) Mode: Alternate Function (AF4)
✓ PB8/PB9 correctly configured for I2C1 (AF4)
======================================

=====================================


=== I2C Bus Scanner ===
Scanning I2C addresses from 0x08 to 0x77...
✓ Device found at 8-bit addr: 0x42 (7-bit: 0x21)
  --> This could be OV7670!
Total devices found: 1
======================


=== OV7670 Multi-Address Test ===
Trying address 0x42 (7-bit: 0x21)...
  ✓ Device responds at this address
  ✓ PID Register (0x0A): 0x76
  🎯 FOUND OV7670! Address: 0x42
  ✓ VER Register (0x0B): 0x73
  🎉 CONFIRMED: Valid OV7670 at address 0x42!

Trying address 0x43 (7-bit: 0x21)...
  ✓ Device responds at this address
  ✓ PID Register (0x0A): 0x76
  🎯 FOUND OV7670! Address: 0x43
  ✓ VER Register (0x0B): 0x73
  🎉 CONFIRMED: Valid OV7670 at address 0x43!

Trying address 0x60 (7-bit: 0x30)...
  ✗ No response at this address

Trying address 0x61 (7-bit: 0x30)...
  ✗ No response at this address

===============================


=== OV7670 Diagnostic Commands ===
S - I2C bus scan
P - Check I2C pin configuration
F - Force GPIO configuration
X - Complete I2C reinitialization
C - Configure/Restart XCLK output
1-5 - Change XCLK frequency
M - Try multiple OV7670 addresses
I - Check OV7670 ID
R - Read key registers
T - Basic configuration test
A - Run ALL diagnostic tests
H - Show this help
===================================
Ready for commands...

```


<img width="800" height="600" alt="004" src="https://github.com/user-attachments/assets/0a885f7e-a56b-4021-8872-d20865bf42d1" />

<img width="800" height="600" alt="001" src="https://github.com/user-attachments/assets/b87e60e8-5abf-45fb-903e-bb6ffe89947d" />

<img width="800" height="600" alt="002" src="https://github.com/user-attachments/assets/4a17ebab-30bd-4512-909c-12dfe973a138" />

<img width="800" height="600" alt="003" src="https://github.com/user-attachments/assets/b5e4039a-3781-4653-b37b-aaf956a0b1fe" />

```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */
```

```c
/* USER CODE BEGIN PD */
// OV7670 I2C Address
#define OV7670_I2C_ADDR         0x42  // 7-bit address shifted left (0x21 << 1)

// Key OV7670 Registers for status check
#define OV7670_REG_PID          0x0A  // Product ID MSB (should be 0x76)
#define OV7670_REG_VER          0x0B  // Product ID LSB (should be 0x73)
#define OV7670_REG_MIDH         0x1C  // Manufacturer ID MSB (should be 0x7F)
#define OV7670_REG_MIDL         0x1D  // Manufacturer ID LSB (should be 0xA2)
#define OV7670_REG_COM1         0x04  // Common control 1
#define OV7670_REG_COM7         0x12  // Common control 7
#define OV7670_REG_COM10        0x15  // Common control 10
#define OV7670_REG_CLKRC        0x11  // Clock control

// I2C Pin definitions for PB8/PB9
#define I2C_SCL_PIN             GPIO_PIN_8  // PB8
#define I2C_SDA_PIN             GPIO_PIN_9  // PB9
#define I2C_GPIO_PORT           GPIOB

// XCLK Pin (MCO1 output)
#define XCLK_PIN                GPIO_PIN_8  // PA8
#define XCLK_GPIO_PORT          GPIOA
/* USER CODE END PD */
```

```c
/* USER CODE BEGIN PFP */
static void Configure_MCO1_Output(void);
/* USER CODE END PFP */
```

```c
/* USER CODE BEGIN 0 */

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/**
  * @brief  Retargets the C library printf function to the USART.
  */
PUTCHAR_PROTOTYPE
{
    if (ch == '\n')
        HAL_UART_Transmit(&huart2, (uint8_t*)"\r", 1, 0xFFFF);
    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, 0xFFFF);
    return ch;
}

/**
  * @brief  MCO1 (PA8) 클럭 출력 설정 - OV7670 XCLK용
  * @note   HSI/2 = 16MHz/2 = 8MHz 또는 PLLCLK/2 = 84MHz/2 = 42MHz
  *         추가 분주기로 최종 클럭 조절 가능
  */
static void Configure_MCO1_Output(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    printf("\n=== Configuring MCO1 for XCLK ===\n");

    // GPIOA 클럭 활성화
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // PA8을 MCO1 Alternate Function으로 설정
    GPIO_InitStruct.Pin = XCLK_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF0_MCO;  // MCO는 AF0
    HAL_GPIO_Init(XCLK_GPIO_PORT, &GPIO_InitStruct);

    // MCO1 소스 및 분주 설정
    // Option 1: HSI (16MHz) / 2 = 8MHz (안정적이지만 낮음)
    // Option 2: PLLCLK (84MHz) / 4 = 21MHz (권장)
    // Option 3: PLLCLK (84MHz) / 2 = 42MHz (너무 높을 수 있음)

    // PLLCLK / 4 = 21MHz 설정 (OV7670에 적합)
    HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_PLLCLK, RCC_MCODIV_4);

    printf("✓ MCO1 configured: PA8 output = 21MHz\n");
    printf("  Source: PLLCLK (84MHz)\n");
    printf("  Divider: /4\n");
    printf("  Output: 21MHz (suitable for OV7670)\n");
    printf("=================================\n\n");

    // XCLK 안정화 대기
    printf("Waiting for XCLK stabilization (500ms)...\n");
    HAL_Delay(500);
    printf("✓ XCLK should be stable now\n\n");
}

/**
  * @brief  XCLK 주파수 변경
  * @param  divider: 1=84MHz, 2=42MHz, 3=28MHz, 4=21MHz, 5=16.8MHz
  */
void Change_XCLK_Frequency(uint32_t divider)
{
    printf("\n=== Changing XCLK Frequency ===\n");

    uint32_t mcodiv;
    float output_freq;

    switch(divider) {
        case 1:
            mcodiv = RCC_MCODIV_1;
            output_freq = 84.0f;
            break;
        case 2:
            mcodiv = RCC_MCODIV_2;
            output_freq = 42.0f;
            break;
        case 3:
            mcodiv = RCC_MCODIV_3;
            output_freq = 28.0f;
            break;
        case 4:
            mcodiv = RCC_MCODIV_4;
            output_freq = 21.0f;
            break;
        case 5:
            mcodiv = RCC_MCODIV_5;
            output_freq = 16.8f;
            break;
        default:
            printf("Invalid divider! Using /4 (21MHz)\n");
            mcodiv = RCC_MCODIV_4;
            output_freq = 21.0f;
            break;
    }

    HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_PLLCLK, mcodiv);

    printf("✓ XCLK frequency changed to %.1f MHz\n", output_freq);
    printf("  Divider: /%lu\n", divider);
    printf("==============================\n\n");

    HAL_Delay(100);  // 안정화 대기
}

/**
  * @brief  OV7670 레지스터 읽기
  */
HAL_StatusTypeDef OV7670_ReadRegister(uint8_t reg_addr, uint8_t *data)
{
    HAL_StatusTypeDef status;
    status = HAL_I2C_Master_Transmit(&hi2c1, OV7670_I2C_ADDR, &reg_addr, 1, 1000);
    if (status != HAL_OK) return status;
    status = HAL_I2C_Master_Receive(&hi2c1, OV7670_I2C_ADDR, data, 1, 1000);
    return status;
}

/**
  * @brief  OV7670 레지스터 쓰기
  */
HAL_StatusTypeDef OV7670_WriteRegister(uint8_t reg_addr, uint8_t data)
{
    uint8_t buf[2] = {reg_addr, data};
    return HAL_I2C_Master_Transmit(&hi2c1, OV7670_I2C_ADDR, buf, 2, 1000);
}

/**
  * @brief  I2C 핀을 강제로 Alternate Function으로 설정 (PB8/PB9)
  */
void Force_I2C1_GPIO_Config(void)
{
    printf("\n=== Forcing I2C1 GPIO Configuration (PB8/PB9) ===\n");

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = I2C_SCL_PIN | I2C_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;

    HAL_GPIO_Init(I2C_GPIO_PORT, &GPIO_InitStruct);

    uint32_t moder_val = GPIOB->MODER;
    uint32_t afr_val = GPIOB->AFR[1];

    uint32_t pb8_mode = (moder_val >> (8 * 2)) & 0x3;
    uint32_t pb9_mode = (moder_val >> (9 * 2)) & 0x3;
    uint32_t pb8_af = (afr_val >> ((8-8) * 4)) & 0xF;
    uint32_t pb9_af = (afr_val >> ((9-8) * 4)) & 0xF;

    printf("PB8 (SCL) Mode: %lu (should be 2), AF: %lu (should be 4)\n", pb8_mode, pb8_af);
    printf("PB9 (SDA) Mode: %lu (should be 2), AF: %lu (should be 4)\n", pb9_mode, pb9_af);

    if (pb8_mode == 2 && pb9_mode == 2 && pb8_af == 4 && pb9_af == 4) {
        printf("✓ GPIO configuration successful!\n");
    } else {
        printf("✗ GPIO configuration failed!\n");
    }
    printf("==============================================\n\n");
}

/**
  * @brief  I2C 핀 설정 및 상태 확인
  */
void I2C_CheckPinStatus(void)
{
    printf("\n=== I2C Configuration Check (PB8/PB9) ===\n");

    printf("I2C1->CR1: 0x%04X ", (uint16_t)I2C1->CR1);
    printf("(PE: %s)\n", (I2C1->CR1 & I2C_CR1_PE) ? "Enabled" : "Disabled");

    printf("I2C1->SR1: 0x%04X\n", (uint16_t)I2C1->SR1);
    printf("I2C1->SR2: 0x%04X\n", (uint16_t)I2C1->SR2);

    if (RCC->AHB1ENR & RCC_AHB1ENR_GPIOBEN) {
        printf("✓ GPIOB clock enabled\n");
    } else {
        printf("✗ GPIOB clock disabled\n");
    }

    if (RCC->APB1ENR & RCC_APB1ENR_I2C1EN) {
        printf("✓ I2C1 clock enabled\n");
    } else {
        printf("✗ I2C1 clock disabled\n");
    }

    uint32_t moder_val = GPIOB->MODER;
    uint32_t afr_val = GPIOB->AFR[1];

    uint32_t pb8_mode = (moder_val >> (8 * 2)) & 0x3;
    uint32_t pb9_mode = (moder_val >> (9 * 2)) & 0x3;
    uint32_t pb8_af = (afr_val >> ((8-8) * 4)) & 0xF;
    uint32_t pb9_af = (afr_val >> ((9-8) * 4)) & 0xF;

    printf("PB8 (SCL) Mode: ");
    switch (pb8_mode) {
        case 0: printf("Input"); break;
        case 1: printf("Output"); break;
        case 2: printf("Alternate Function (AF%lu)", pb8_af); break;
        case 3: printf("Analog"); break;
    }
    printf("\n");

    printf("PB9 (SDA) Mode: ");
    switch (pb9_mode) {
        case 0: printf("Input"); break;
        case 1: printf("Output"); break;
        case 2: printf("Alternate Function (AF%lu)", pb9_af); break;
        case 3: printf("Analog"); break;
    }
    printf("\n");

    if (pb8_mode == 2 && pb9_mode == 2 && pb8_af == 4 && pb9_af == 4) {
        printf("✓ PB8/PB9 correctly configured for I2C1 (AF4)\n");
    } else {
        printf("✗ GPIO pins not properly configured for I2C\n");
    }

    printf("======================================\n\n");
}

/**
  * @brief  I2C 버스 스캔
  */
void I2C_Scanner(void)
{
    printf("\n=== I2C Bus Scanner ===\n");
    printf("Scanning I2C addresses from 0x08 to 0x77...\n");

    uint8_t found_devices = 0;

    for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
        HAL_StatusTypeDef result = HAL_I2C_IsDeviceReady(&hi2c1, addr << 1, 1, 100);

        if (result == HAL_OK) {
            printf("✓ Device found at 8-bit addr: 0x%02X (7-bit: 0x%02X)\n", addr << 1, addr);
            found_devices++;

            if (addr == 0x21 || addr == 0x42 || addr == 0x60) {
                printf("  --> This could be OV7670!\n");
            }
        }
    }

    if (found_devices == 0) {
        printf("❌ No I2C devices found!\n");
        printf("Make sure XCLK is connected and running!\n");
    } else {
        printf("Total devices found: %d\n", found_devices);
    }
    printf("======================\n\n");
}

/**
  * @brief  다양한 OV7670 주소로 ID 읽기 시도
  */
void OV7670_TryMultipleAddresses(void)
{
    printf("\n=== OV7670 Multi-Address Test ===\n");

    uint8_t possible_addresses[] = {0x42, 0x43, 0x60, 0x61};
    uint8_t num_addresses = sizeof(possible_addresses) / sizeof(possible_addresses[0]);

    for (uint8_t i = 0; i < num_addresses; i++) {
        uint8_t addr = possible_addresses[i];
        printf("Trying address 0x%02X (7-bit: 0x%02X)...\n", addr, addr >> 1);

        HAL_StatusTypeDef ready_result = HAL_I2C_IsDeviceReady(&hi2c1, addr, 3, 100);
        if (ready_result == HAL_OK) {
            printf("  ✓ Device responds at this address\n");

            uint8_t reg_addr = 0x0A;
            uint8_t pid_value = 0;

            HAL_StatusTypeDef tx_result = HAL_I2C_Master_Transmit(&hi2c1, addr, &reg_addr, 1, 1000);
            if (tx_result == HAL_OK) {
                HAL_StatusTypeDef rx_result = HAL_I2C_Master_Receive(&hi2c1, addr, &pid_value, 1, 1000);
                if (rx_result == HAL_OK) {
                    printf("  ✓ PID Register (0x0A): 0x%02X\n", pid_value);
                    if (pid_value == 0x76) {
                        printf("  🎯 FOUND OV7670! Address: 0x%02X\n", addr);

                        uint8_t ver_addr = 0x0B;
                        uint8_t ver_value = 0;
                        HAL_I2C_Master_Transmit(&hi2c1, addr, &ver_addr, 1, 1000);
                        HAL_I2C_Master_Receive(&hi2c1, addr, &ver_value, 1, 1000);
                        printf("  ✓ VER Register (0x0B): 0x%02X\n", ver_value);

                        if (ver_value == 0x73) {
                            printf("  🎉 CONFIRMED: Valid OV7670 at address 0x%02X!\n", addr);
                        }
                    }
                }
            }
        } else {
            printf("  ✗ No response at this address\n");
        }
        printf("\n");
    }
    printf("===============================\n\n");
}

/**
  * @brief  OV7670 센서 ID 확인
  */
uint8_t OV7670_CheckID(void)
{
    uint8_t pid, ver, midh, midl;
    HAL_StatusTypeDef status;

    printf("\n=== OV7670 ID Check ===\n");

    status = OV7670_ReadRegister(OV7670_REG_PID, &pid);
    if (status != HAL_OK) {
        printf("Failed to read PID register (0x%02X)\n", OV7670_REG_PID);
        return 0;
    }

    status = OV7670_ReadRegister(OV7670_REG_VER, &ver);
    if (status != HAL_OK) {
        printf("Failed to read VER register (0x%02X)\n", OV7670_REG_VER);
        return 0;
    }

    status = OV7670_ReadRegister(OV7670_REG_MIDH, &midh);
    if (status != HAL_OK) {
        printf("Failed to read MIDH register (0x%02X)\n", OV7670_REG_MIDH);
        return 0;
    }

    status = OV7670_ReadRegister(OV7670_REG_MIDL, &midl);
    if (status != HAL_OK) {
        printf("Failed to read MIDL register (0x%02X)\n", OV7670_REG_MIDL);
        return 0;
    }

    printf("Product ID: 0x%02X%02X (Expected: 0x7673)\n", pid, ver);
    printf("Manufacturer ID: 0x%02X%02X (Expected: 0x7FA2)\n", midh, midl);

    if (pid == 0x76 && ver == 0x73 && midh == 0x7F && midl == 0xA2) {
        printf("✓ OV7670 sensor detected successfully!\n");
        return 1;
    } else {
        printf("✗ Invalid sensor ID - may be faulty or not OV7670\n");
        return 0;
    }
}

/**
  * @brief  OV7670 주요 레지스터 상태 확인
  */
void OV7670_CheckRegisters(void)
{
    uint8_t reg_value;
    HAL_StatusTypeDef status;

    printf("\n=== OV7670 Register Status ===\n");

    status = OV7670_ReadRegister(OV7670_REG_CLKRC, &reg_value);
    printf("CLKRC (0x%02X): 0x%02X %s\n",
           OV7670_REG_CLKRC, reg_value,
           (status == HAL_OK) ? "✓" : "✗");

    status = OV7670_ReadRegister(OV7670_REG_COM1, &reg_value);
    printf("COM1  (0x%02X): 0x%02X %s\n",
           OV7670_REG_COM1, reg_value,
           (status == HAL_OK) ? "✓" : "✗");

    status = OV7670_ReadRegister(OV7670_REG_COM7, &reg_value);
    printf("COM7  (0x%02X): 0x%02X %s",
           OV7670_REG_COM7, reg_value,
           (status == HAL_OK) ? "✓" : "✗");
    if (status == HAL_OK) {
        printf(" (Reset: %s, Format: %s)",
               (reg_value & 0x80) ? "Active" : "Normal",
               (reg_value & 0x04) ? "RGB" : "YUV");
    }
    printf("\n");

    status = OV7670_ReadRegister(OV7670_REG_COM10, &reg_value);
    printf("COM10 (0x%02X): 0x%02X %s",
           OV7670_REG_COM10, reg_value,
           (status == HAL_OK) ? "✓" : "✗");
    if (status == HAL_OK) {
        printf(" (HSYNC: %s, VSYNC: %s)",
               (reg_value & 0x40) ? "Active Low" : "Active High",
               (reg_value & 0x20) ? "Active Low" : "Active High");
    }
    printf("\n");

    printf("============================\n\n");
}

/**
  * @brief  OV7670 기본 설정 테스트
  */
void OV7670_BasicConfigTest(void)
{
    printf("\n=== OV7670 Basic Config Test ===\n");

    printf("Testing software reset...\n");
    HAL_StatusTypeDef status = OV7670_WriteRegister(OV7670_REG_COM7, 0x80);
    if (status == HAL_OK) {
        printf("✓ Software reset command sent\n");
        HAL_Delay(100);

        uint8_t pid;
        status = OV7670_ReadRegister(OV7670_REG_PID, &pid);
        if (status == HAL_OK && pid == 0x76) {
            printf("✓ Sensor responsive after reset\n");
        } else {
            printf("✗ Sensor not responsive after reset\n");
        }
    } else {
        printf("✗ Failed to send software reset\n");
    }

    printf("Testing clock divider setting...\n");
    status = OV7670_WriteRegister(OV7670_REG_CLKRC, 0x00);
    if (status == HAL_OK) {
        uint8_t clkrc_read;
        status = OV7670_ReadRegister(OV7670_REG_CLKRC, &clkrc_read);
        if (status == HAL_OK) {
            printf("✓ CLKRC write/read test: wrote 0x00, read 0x%02X\n", clkrc_read);
        } else {
            printf("✗ Failed to read back CLKRC\n");
        }
    } else {
        printf("✗ Failed to write CLKRC\n");
    }

    printf("===============================\n\n");
}

/**
  * @brief  완전한 I2C1 재초기화
  */
void Complete_I2C1_Reinit(void)
{
    printf("\n=== Complete I2C1 Reinitialization ===\n");

    printf("Step 1: Deinitializing I2C1...\n");
    HAL_I2C_DeInit(&hi2c1);

    printf("Step 2: Resetting clocks...\n");
    __HAL_RCC_I2C1_CLK_DISABLE();
    __HAL_RCC_I2C1_FORCE_RESET();
    HAL_Delay(10);
    __HAL_RCC_I2C1_RELEASE_RESET();
    __HAL_RCC_I2C1_CLK_ENABLE();

    printf("Step 3: Forcing GPIO configuration...\n");
    Force_I2C1_GPIO_Config();

    printf("Step 4: Reinitializing I2C1...\n");
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        printf("✗ I2C1 initialization failed!\n");
    } else {
        printf("✓ I2C1 initialized successfully!\n");
    }

    printf("Step 5: Verifying configuration...\n");
    I2C_CheckPinStatus();

    printf("=====================================\n\n");
}

/**
  * @brief  시스템 정보 출력
  */
void PrintSystemInfo(void)
{
    printf("\n=== System Information ===\n");
    printf("MCU: STM32F411\n");
    printf("Clock Frequency: 84 MHz\n");
    printf("I2C Interface: I2C1 (PB8=SCL, PB9=SDA)\n");
    printf("UART Interface: UART2 (PA2=TX, PA3=RX)\n");
    printf("XCLK Output: PA8 (MCO1) @ 21MHz\n");
    printf("OV7670 I2C Address: 0x%02X (7-bit: 0x%02X)\n", OV7670_I2C_ADDR, OV7670_I2C_ADDR >> 1);
    printf("=========================\n\n");
}

/**
  * @brief  명령어 처리
  */
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
            Configure_MCO1_Output();
            break;

        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
            Change_XCLK_Frequency(cmd - '0');
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

        case 't':
        case 'T':
            OV7670_BasicConfigTest();
            break;

        case 'a':
        case 'A':
            printf("Running COMPLETE diagnostic with XCLK...\n\n");
            Configure_MCO1_Output();
            Complete_I2C1_Reinit();
            I2C_Scanner();
            OV7670_TryMultipleAddresses();
            if (OV7670_CheckID()) {
                OV7670_CheckRegisters();
                OV7670_BasicConfigTest();
            }
            break;

        case 'h':
        case 'H':
        case '?':
            printf("\n=== OV7670 Diagnostic Commands ===\n");
            printf("S - I2C bus scan\n");
            printf("P - Check I2C pin configuration\n");
            printf("F - Force GPIO configuration\n");
            printf("X - Complete I2C reinitialization\n");
            printf("C - Configure/Restart XCLK output\n");
            printf("1-5 - Change XCLK frequency\n");
            printf("      1=84MHz, 2=42MHz, 3=28MHz, 4=21MHz(default), 5=16.8MHz\n");
            printf("M - Try multiple OV7670 addresses\n");
            printf("I - Check OV7670 ID\n");
            printf("R - Read key registers\n");
            printf("T - Basic configuration test\n");
            printf("A - Run ALL diagnostic tests\n");
            printf("H - Show this help\n");
            printf("===================================\n\n");
            break;

        default:
            printf("Unknown command: %c\n", cmd);
            printf("Type 'H' for help\n\n");
            break;
    }
}

/**
  * @brief  Main application entry point
  */
void UserApp_Main(void)
{
    PrintSystemInfo();

    printf("⚠️  IMPORTANT: Connect OV7670 XCLK to PA8!\n\n");

    printf("Starting OV7670 diagnostic with XCLK generation...\n");

    // XCLK 출력 설정 (가장 먼저!)
    Configure_MCO1_Output();

    // I2C 초기화
    Complete_I2C1_Reinit();

    // 진단 시작
    I2C_Scanner();
    OV7670_TryMultipleAddresses();

    printf("\n=== OV7670 Diagnostic Commands ===\n");
    printf("S - I2C bus scan\n");
    printf("P - Check I2C pin configuration\n");
    printf("F - Force GPIO configuration\n");
    printf("X - Complete I2C reinitialization\n");
    printf("C - Configure/Restart XCLK output\n");
    printf("1-5 - Change XCLK frequency\n");
    printf("M - Try multiple OV7670 addresses\n");
    printf("I - Check OV7670 ID\n");
    printf("R - Read key registers\n");
    printf("T - Basic configuration test\n");
    printf("A - Run ALL diagnostic tests\n");
    printf("H - Show this help\n");
    printf("===================================\n");
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
          char received_char = (char)rx_buffer[0];

          if (received_char != '\r' && received_char != '\n') {
              printf("%c\n", received_char);
          }

          ProcessCommand(received_char);
          printf("> ");
      }

      HAL_Delay(1);

    }
  /* USER CODE END 3 */
```

