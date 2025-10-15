# SPI_Serial_Flash_Winbond

<img width="644" height="586" alt="F103RB-pin" src="https://github.com/user-attachments/assets/5071fc8b-00bf-4fcf-9c83-39f2225a3e25" />



**1.SPI1 설정:**
```
- Mode: Full-Duplex Master
- Hardware NSS Signal: Disable (CS를 GPIO로 수동 제어)
- Data Size: 8 Bits
- First Bit: MSB First
- Prescaler: 2 (64MHz / 2 = 32MHz) 또는 4 (16MHz)
  * W25Q는 최대 80-104MHz 지원하지만, 안정성을 위해 32MHz 이하 권장
- Clock Polarity (CPOL): Low
- Clock Phase (CPHA): 1 Edge
- CRC Calculation: Disabled
```

**2. GPIO 설정:**
```
CS 핀 (예: PA4):
- GPIO Output Level: High
- GPIO mode: Output Push Pull
- GPIO Pull-up/Pull-down: No pull-up and no pull-down
- Maximum output speed: High
```

**3. 핀 연결:**
```
STM32F103     →     W25Q32/64
---------------------------------
PA5 (SPI1_SCK) →    CLK
PA6 (SPI1_MISO) →   D0 (DO)
PA7 (SPI1_MOSI) →   D1 (DI)
PA4 (GPIO)      →   CS
3.3V            →   VCC
GND             →   GND
```


```c
/* USER CODE BEGIN Includes */
#include "w25q_flash.h"
/* USER CODE END Includes */
```

```c
/* USER CODE BEGIN PV */
W25Q_HandleTypeDef hflash;
char uart_buffer[100];
/* USER CODE END PV */
```

```c
/* USER CODE BEGIN 0 */
void UART_Print(char *msg) {
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 1000);
}

void W25Q_Test(void) {
    uint16_t manufacturer_id, device_id;
    uint8_t manufacturer, memory_type, capacity;

    sprintf(uart_buffer, "\r\n=== W25Q Flash Test ===\r\n");
    UART_Print(uart_buffer);

    // Read Device ID
    if (W25Q_ReadID(&hflash, &manufacturer_id, &device_id)) {
        sprintf(uart_buffer, "Device ID Read Success!\r\n");
        UART_Print(uart_buffer);
        sprintf(uart_buffer, "Manufacturer ID: 0x%02X\r\n", manufacturer_id);
        UART_Print(uart_buffer);
        sprintf(uart_buffer, "Device ID: 0x%02X\r\n", device_id);
        UART_Print(uart_buffer);

        if (manufacturer_id == 0xEF) {
            sprintf(uart_buffer, "Manufacturer: Winbond\r\n");
            UART_Print(uart_buffer);
        }

        if (device_id == 0x15) {
            sprintf(uart_buffer, "Device: W25Q32 (4MB)\r\n");
            UART_Print(uart_buffer);
        } else if (device_id == 0x16) {
            sprintf(uart_buffer, "Device: W25Q64 (8MB)\r\n");
            UART_Print(uart_buffer);
        }
    } else {
        sprintf(uart_buffer, "Device ID Read Failed!\r\n");
        UART_Print(uart_buffer);
        return;
    }

    // Read JEDEC ID
    if (W25Q_ReadJEDECID(&hflash, &manufacturer, &memory_type, &capacity)) {
        sprintf(uart_buffer, "JEDEC ID: 0x%02X%02X%02X\r\n",
                manufacturer, memory_type, capacity);
        UART_Print(uart_buffer);
    }

    // Test Write and Read
    sprintf(uart_buffer, "\r\n--- Write/Read Test ---\r\n");
    UART_Print(uart_buffer);

    uint32_t test_address = 0x0000;
    uint8_t write_data[256];
    uint8_t read_data[256];

    // Prepare test data
    for (int i = 0; i < 256; i++) {
        write_data[i] = i;
    }

    // Erase sector before writing
    sprintf(uart_buffer, "Erasing sector at 0x%06lX...\r\n", test_address);
    UART_Print(uart_buffer);

    if (W25Q_SectorErase(&hflash, test_address)) {
        sprintf(uart_buffer, "Sector erase success!\r\n");
        UART_Print(uart_buffer);
    } else {
        sprintf(uart_buffer, "Sector erase failed!\r\n");
        UART_Print(uart_buffer);
        return;
    }

    // Write data
    sprintf(uart_buffer, "Writing 256 bytes to 0x%06lX...\r\n", test_address);
    UART_Print(uart_buffer);

    if (W25Q_WriteData(&hflash, test_address, write_data, 256)) {
        sprintf(uart_buffer, "Write success!\r\n");
        UART_Print(uart_buffer);
    } else {
        sprintf(uart_buffer, "Write failed!\r\n");
        UART_Print(uart_buffer);
        return;
    }

    // Read data
    sprintf(uart_buffer, "Reading 256 bytes from 0x%06lX...\r\n", test_address);
    UART_Print(uart_buffer);

    if (W25Q_ReadData(&hflash, test_address, read_data, 256)) {
        sprintf(uart_buffer, "Read success!\r\n");
        UART_Print(uart_buffer);
    } else {
        sprintf(uart_buffer, "Read failed!\r\n");
        UART_Print(uart_buffer);
        return;
    }

    // Verify data
    bool verify_ok = true;
    for (int i = 0; i < 256; i++) {
        if (write_data[i] != read_data[i]) {
            verify_ok = false;
            sprintf(uart_buffer, "Verify failed at byte %d: wrote 0x%02X, read 0x%02X\r\n",
                    i, write_data[i], read_data[i]);
            UART_Print(uart_buffer);
            break;
        }
    }

    if (verify_ok) {
        sprintf(uart_buffer, "Data verification SUCCESS! All 256 bytes match.\r\n");
        UART_Print(uart_buffer);
    }

    sprintf(uart_buffer, "\r\n=== Test Complete ===\r\n\r\n");
    UART_Print(uart_buffer);
}
/* USER CODE END 0 */
```

```c
  /* USER CODE BEGIN 2 */
  // Initialize W25Q Flash with SPI1_CS pin
  W25Q_Init(&hflash, &hspi1, SPI1_CS_GPIO_Port, SPI1_CS_Pin);

  sprintf(uart_buffer, "W25Q Flash Driver Initialized\r\n");
  UART_Print(uart_buffer);

  // Run flash test
  HAL_Delay(100);
  W25Q_Test();
  /* USER CODE END 2 */
```

```c
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	// 플래시 메모리 읽기 예제
	// uint8_t buffer[256];
	// W25Q_ReadData(&hflash, 0x0000, buffer, 256);

	HAL_Delay(1000);
  }
  /* USER CODE END 3 */
```

---

```
W25Q Flash Driver Initialized

=== W25Q Flash Test ===
Device ID Read Success!
Manufacturer ID: 0xEF
Device ID: 0x15
Manufacturer: Winbond
Device: W25Q32 (4MB)
JEDEC ID: 0xEF4016

--- Write/Read Test ---
Erasing sector at 0x000000...
Sector erase success!
Writing 256 bytes to 0x000000...
Write success!
Reading 256 bytes from 0x000000...
Read success!
Data verification SUCCESS! All 256 bytes match.

=== Test Complete ===

W25Q Flash Driver Initialized

=== W25Q Flash Test ===
Device ID Read Success!
Manufacturer ID: 0xEF
Device ID: 0x16
Manufacturer: Winbond
Device: W25Q64 (8MB)
JEDEC ID: 0xEF4017

--- Write/Read Test ---
Erasing sector at 0x000000...
Sector erase success!
Writing 256 bytes to 0x000000...
Write success!
Reading 256 bytes from 0x000000...
Read success!
Data verification SUCCESS! All 256 bytes match.
```
