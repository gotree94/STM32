# I2C_EEPROM_GPIO

<img width="586" height="704" alt="NUCLEO-F103RB" src="https://github.com/user-attachments/assets/e7b74c47-b97c-47a9-a429-cc13ede19b19" />

<img width="644" height="586" alt="F103RB-pin" src="https://github.com/user-attachments/assets/5e81782c-f759-483b-a4d6-4e9f04f81b83" />

<img width="1108" height="800" alt="001" src="https://github.com/user-attachments/assets/60b80dbb-c295-423e-aefa-c25769cb0abd" />

```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */
```

```c
/* USER CODE BEGIN PD */
// Software I2C GPIO 핀 정의 (원하는 핀으로 변경 가능!)
#define SW_I2C_SCL_PORT     GPIOC
#define SW_I2C_SCL_PIN      GPIO_PIN_8

#define SW_I2C_SDA_PORT     GPIOC
#define SW_I2C_SDA_PIN      GPIO_PIN_6

// I2C 타이밍 (클럭 속도 조절)
#define I2C_DELAY_US        5   // 5us = 약 100kHz

// K24C256 설정
#define K24C256_ADDR_WRITE  0xA0
#define K24C256_ADDR_READ   0xA1
#define K24C256_BLOCK1_WRITE 0xB0
#define K24C256_BLOCK1_READ  0xB1
#define EEPROM_PAGE_SIZE    64
/* USER CODE END PD */
```

```c
/* USER CODE BEGIN PFP */
// Software I2C 기본 함수
void SW_I2C_Init(void);
void SW_I2C_Delay(void);
void SW_I2C_SCL_High(void);
void SW_I2C_SCL_Low(void);
void SW_I2C_SDA_High(void);
void SW_I2C_SDA_Low(void);
uint8_t SW_I2C_SDA_Read(void);
void SW_I2C_Start(void);
void SW_I2C_Stop(void);
void SW_I2C_SendByte(uint8_t data);
uint8_t SW_I2C_ReceiveByte(uint8_t ack);
uint8_t SW_I2C_WaitAck(void);
void SW_I2C_Ack(void);
void SW_I2C_NAck(void);

// EEPROM 함수
void SW_I2C_Scan(void);
uint8_t EEPROM_WriteByte(uint8_t dev_addr, uint16_t mem_addr, uint8_t data);
uint8_t EEPROM_ReadByte(uint8_t dev_addr, uint16_t mem_addr, uint8_t *data);
uint8_t EEPROM_WriteBytes(uint8_t dev_addr, uint16_t mem_addr, uint8_t *data, uint16_t len);
uint8_t EEPROM_ReadBytes(uint8_t dev_addr, uint16_t mem_addr, uint8_t *data, uint16_t len);
void EEPROM_Test(void);
void Test_0xB0_Device(void);
/* USER CODE END PFP */
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
  * @brief  Microsecond delay
  */
void DelayUs(uint32_t us)
{
  uint32_t cycles = us * (SystemCoreClock / 1000000) / 5;
  while(cycles--) __NOP();
}

/**
  * @brief  Software I2C 초기화
  */
void SW_I2C_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // GPIO 클럭 활성화
  __HAL_RCC_GPIOA_CLK_ENABLE();

  // SCL 핀 설정 (Open-Drain Output)
  GPIO_InitStruct.Pin = SW_I2C_SCL_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SW_I2C_SCL_PORT, &GPIO_InitStruct);

  // SDA 핀 설정 (Open-Drain Output)
  GPIO_InitStruct.Pin = SW_I2C_SDA_PIN;
  HAL_GPIO_Init(SW_I2C_SDA_PORT, &GPIO_InitStruct);

  // 초기 상태: HIGH (Idle)
  SW_I2C_SCL_High();
  SW_I2C_SDA_High();

  printf("Software I2C initialized on PC8(SCL), PC6(SDA)\n");
}

/**
  * @brief  I2C 딜레이 (클럭 속도 조절)
  */
void SW_I2C_Delay(void)
{
  DelayUs(I2C_DELAY_US);
}

/**
  * @brief  SCL을 HIGH로 설정
  */
void SW_I2C_SCL_High(void)
{
  HAL_GPIO_WritePin(SW_I2C_SCL_PORT, SW_I2C_SCL_PIN, GPIO_PIN_SET);
  SW_I2C_Delay();
}

/**
  * @brief  SCL을 LOW로 설정
  */
void SW_I2C_SCL_Low(void)
{
  HAL_GPIO_WritePin(SW_I2C_SCL_PORT, SW_I2C_SCL_PIN, GPIO_PIN_RESET);
  SW_I2C_Delay();
}

/**
  * @brief  SDA를 HIGH로 설정
  */
void SW_I2C_SDA_High(void)
{
  HAL_GPIO_WritePin(SW_I2C_SDA_PORT, SW_I2C_SDA_PIN, GPIO_PIN_SET);
  SW_I2C_Delay();
}

/**
  * @brief  SDA를 LOW로 설정
  */
void SW_I2C_SDA_Low(void)
{
  HAL_GPIO_WritePin(SW_I2C_SDA_PORT, SW_I2C_SDA_PIN, GPIO_PIN_RESET);
  SW_I2C_Delay();
}

/**
  * @brief  SDA 상태 읽기
  */
uint8_t SW_I2C_SDA_Read(void)
{
  return HAL_GPIO_ReadPin(SW_I2C_SDA_PORT, SW_I2C_SDA_PIN);
}

/**
  * @brief  I2C Start 조건
  */
void SW_I2C_Start(void)
{
  SW_I2C_SDA_High();
  SW_I2C_SCL_High();
  SW_I2C_Delay();
  SW_I2C_SDA_Low();   // SDA: HIGH -> LOW while SCL is HIGH
  SW_I2C_Delay();
  SW_I2C_SCL_Low();
}

/**
  * @brief  I2C Stop 조건
  */
void SW_I2C_Stop(void)
{
  SW_I2C_SDA_Low();
  SW_I2C_SCL_High();
  SW_I2C_Delay();
  SW_I2C_SDA_High();  // SDA: LOW -> HIGH while SCL is HIGH
  SW_I2C_Delay();
}

/**
  * @brief  1 바이트 전송
  */
void SW_I2C_SendByte(uint8_t data)
{
  for(int i = 0; i < 8; i++)
  {
    SW_I2C_SCL_Low();

    if(data & 0x80)
      SW_I2C_SDA_High();
    else
      SW_I2C_SDA_Low();

    data <<= 1;
    SW_I2C_Delay();
    SW_I2C_SCL_High();
    SW_I2C_Delay();
  }
  SW_I2C_SCL_Low();
}

/**
  * @brief  1 바이트 수신
  */
uint8_t SW_I2C_ReceiveByte(uint8_t ack)
{
  uint8_t data = 0;

  SW_I2C_SDA_High();  // Release SDA for reading

  for(int i = 0; i < 8; i++)
  {
    data <<= 1;
    SW_I2C_SCL_Low();
    SW_I2C_Delay();
    SW_I2C_SCL_High();

    if(SW_I2C_SDA_Read())
      data |= 0x01;

    SW_I2C_Delay();
  }

  SW_I2C_SCL_Low();

  // Send ACK or NACK
  if(ack)
    SW_I2C_Ack();
  else
    SW_I2C_NAck();

  return data;
}

/**
  * @brief  ACK 대기
  */
uint8_t SW_I2C_WaitAck(void)
{
  uint8_t ack;

  SW_I2C_SDA_High();  // Release SDA
  SW_I2C_Delay();
  SW_I2C_SCL_High();
  SW_I2C_Delay();

  ack = SW_I2C_SDA_Read();

  SW_I2C_SCL_Low();
  SW_I2C_Delay();

  return ack;  // 0 = ACK, 1 = NACK
}

/**
  * @brief  ACK 전송
  */
void SW_I2C_Ack(void)
{
  SW_I2C_SCL_Low();
  SW_I2C_SDA_Low();
  SW_I2C_Delay();
  SW_I2C_SCL_High();
  SW_I2C_Delay();
  SW_I2C_SCL_Low();
  SW_I2C_SDA_High();  // Release SDA
}

/**
  * @brief  NACK 전송
  */
void SW_I2C_NAck(void)
{
  SW_I2C_SCL_Low();
  SW_I2C_SDA_High();
  SW_I2C_Delay();
  SW_I2C_SCL_High();
  SW_I2C_Delay();
  SW_I2C_SCL_Low();
}

/**
  * @brief  I2C 주소 스캔
  */
void SW_I2C_Scan(void)
{
  printf("\n=== Software I2C Address Scan ===\n");
  printf("Scanning I2C bus...\n\n");

  uint8_t count = 0;

  for(uint8_t addr = 0; addr < 128; addr++)
  {
    SW_I2C_Start();
    SW_I2C_SendByte(addr << 1);  // Write address

    if(SW_I2C_WaitAck() == 0)  // ACK received
    {
      printf("Found I2C device at address: 0x%02X (7-bit: 0x%02X)\n", addr << 1, addr);
      count++;

      if((addr << 1) >= 0xA0 && (addr << 1) <= 0xAE)
      {
        printf("  -> K24C256 EEPROM detected\n");
      }
      else if((addr << 1) == 0xB0)
      {
        printf("  -> Possible Block 1 or other device\n");
      }
    }

    SW_I2C_Stop();
    HAL_Delay(1);
  }

  printf("\nTotal %d I2C device(s) found.\n", count);
  printf("=================================\n\n");
}

/**
  * @brief  EEPROM 1바이트 쓰기
  */
uint8_t EEPROM_WriteByte(uint8_t dev_addr, uint16_t mem_addr, uint8_t data)
{
  SW_I2C_Start();

  // Device address + Write
  SW_I2C_SendByte(dev_addr);
  if(SW_I2C_WaitAck() != 0) {
    SW_I2C_Stop();
    return 1;  // Error
  }

  // Memory address (High byte)
  SW_I2C_SendByte((mem_addr >> 8) & 0xFF);
  if(SW_I2C_WaitAck() != 0) {
    SW_I2C_Stop();
    return 1;
  }

  // Memory address (Low byte)
  SW_I2C_SendByte(mem_addr & 0xFF);
  if(SW_I2C_WaitAck() != 0) {
    SW_I2C_Stop();
    return 1;
  }

  // Data
  SW_I2C_SendByte(data);
  if(SW_I2C_WaitAck() != 0) {
    SW_I2C_Stop();
    return 1;
  }

  SW_I2C_Stop();
  HAL_Delay(5);  // Write cycle time

  return 0;  // Success
}

/**
  * @brief  EEPROM 1바이트 읽기
  */
uint8_t EEPROM_ReadByte(uint8_t dev_addr, uint16_t mem_addr, uint8_t *data)
{
  SW_I2C_Start();

  // Device address + Write (to set memory address)
  SW_I2C_SendByte(dev_addr & 0xFE);
  if(SW_I2C_WaitAck() != 0) {
    SW_I2C_Stop();
    return 1;
  }

  // Memory address (High byte)
  SW_I2C_SendByte((mem_addr >> 8) & 0xFF);
  if(SW_I2C_WaitAck() != 0) {
    SW_I2C_Stop();
    return 1;
  }

  // Memory address (Low byte)
  SW_I2C_SendByte(mem_addr & 0xFF);
  if(SW_I2C_WaitAck() != 0) {
    SW_I2C_Stop();
    return 1;
  }

  // Repeated Start
  SW_I2C_Start();

  // Device address + Read
  SW_I2C_SendByte(dev_addr | 0x01);
  if(SW_I2C_WaitAck() != 0) {
    SW_I2C_Stop();
    return 1;
  }

  // Read data
  *data = SW_I2C_ReceiveByte(0);  // NACK

  SW_I2C_Stop();

  return 0;  // Success
}

/**
  * @brief  EEPROM 여러 바이트 쓰기 (페이지 쓰기 지원)
  */
uint8_t EEPROM_WriteBytes(uint8_t dev_addr, uint16_t mem_addr, uint8_t *data, uint16_t len)
{
  uint16_t written = 0;

  while(written < len)
  {
    uint16_t page_offset = mem_addr % EEPROM_PAGE_SIZE;
    uint16_t bytes_to_write = EEPROM_PAGE_SIZE - page_offset;

    if(bytes_to_write > (len - written))
      bytes_to_write = len - written;

    // Page write
    SW_I2C_Start();
    SW_I2C_SendByte(dev_addr);
    if(SW_I2C_WaitAck() != 0) {
      SW_I2C_Stop();
      return 1;
    }

    SW_I2C_SendByte((mem_addr >> 8) & 0xFF);
    if(SW_I2C_WaitAck() != 0) {
      SW_I2C_Stop();
      return 1;
    }

    SW_I2C_SendByte(mem_addr & 0xFF);
    if(SW_I2C_WaitAck() != 0) {
      SW_I2C_Stop();
      return 1;
    }

    for(uint16_t i = 0; i < bytes_to_write; i++)
    {
      SW_I2C_SendByte(data[written + i]);
      if(SW_I2C_WaitAck() != 0) {
        SW_I2C_Stop();
        return 1;
      }
    }

    SW_I2C_Stop();
    HAL_Delay(5);  // Write cycle time

    written += bytes_to_write;
    mem_addr += bytes_to_write;
  }

  return 0;
}

/**
  * @brief  EEPROM 여러 바이트 읽기
  */
uint8_t EEPROM_ReadBytes(uint8_t dev_addr, uint16_t mem_addr, uint8_t *data, uint16_t len)
{
  SW_I2C_Start();

  // Set memory address
  SW_I2C_SendByte(dev_addr & 0xFE);
  if(SW_I2C_WaitAck() != 0) {
    SW_I2C_Stop();
    return 1;
  }

  SW_I2C_SendByte((mem_addr >> 8) & 0xFF);
  if(SW_I2C_WaitAck() != 0) {
    SW_I2C_Stop();
    return 1;
  }

  SW_I2C_SendByte(mem_addr & 0xFF);
  if(SW_I2C_WaitAck() != 0) {
    SW_I2C_Stop();
    return 1;
  }

  // Repeated Start
  SW_I2C_Start();

  // Read mode
  SW_I2C_SendByte(dev_addr | 0x01);
  if(SW_I2C_WaitAck() != 0) {
    SW_I2C_Stop();
    return 1;
  }

  // Read data
  for(uint16_t i = 0; i < len; i++)
  {
    data[i] = SW_I2C_ReceiveByte(i < (len - 1) ? 1 : 0);  // ACK except last byte
  }

  SW_I2C_Stop();

  return 0;
}

/**
  * @brief  EEPROM 테스트
  */
void EEPROM_Test(void)
{
  printf("=== EEPROM Test (0xA0) ===\n");

  char write_str[] = "Hello Software I2C!";
  uint8_t read_buf[50] = {0};
  uint16_t len = strlen(write_str);

  printf("Writing: \"%s\" (%d bytes)\n", write_str, len);

  if(EEPROM_WriteBytes(K24C256_ADDR_WRITE, 0x0000, (uint8_t*)write_str, len) == 0)
  {
    printf("Write: SUCCESS\n");
    HAL_Delay(10);

    if(EEPROM_ReadBytes(K24C256_ADDR_READ, 0x0000, read_buf, len) == 0)
    {
      printf("Read: SUCCESS\n");
      printf("Data: \"%s\"\n", read_buf);

      if(memcmp(write_str, read_buf, len) == 0)
        printf("** Verification PASSED! **\n");
      else
        printf("** Verification FAILED! **\n");
    }
    else
      printf("Read: FAILED\n");
  }
  else
    printf("Write: FAILED\n");

  printf("===========================\n\n");
}

/**
  * @brief  0xB0 블럭 테스트
  */
void Test_0xB0_Device(void)
{
  printf("=== Testing 0xB0 Device ===\n");

  char write_str[] = "Block1!";
  uint8_t read_buf[20] = {0};
  uint16_t len = strlen(write_str);

  printf("Writing to 0xB0: \"%s\"\n", write_str);

  if(EEPROM_WriteBytes(K24C256_BLOCK1_WRITE, 0x0100, (uint8_t*)write_str, len) == 0)
  {
    printf("Write to 0xB0: SUCCESS\n");
    HAL_Delay(10);

    if(EEPROM_ReadBytes(K24C256_BLOCK1_READ, 0x0100, read_buf, len) == 0)
    {
      printf("Read from 0xB0: SUCCESS\n");
      printf("Data: \"%s\"\n", read_buf);

      if(memcmp(write_str, read_buf, len) == 0)
        printf("** 0xB0 is valid EEPROM block! **\n");
    }
    else
      printf("Read from 0xB0: FAILED\n");
  }
  else
    printf("Write to 0xB0: FAILED\n");

  printf("===========================\n\n");
}

/* USER CODE END 0 */
```

```c
  /* USER CODE BEGIN 2 */
  printf("\n\n");
  printf("==========================================\n");
  printf("  STM32F103 Software I2C EEPROM Test     \n");
  printf("  System Clock: 64MHz                    \n");
  printf("  SW I2C: PC8(SCL), PC6(SDA)             \n");
  printf("==========================================\n\n");

  // Software I2C 초기화
  SW_I2C_Init();

  // I2C 주소 스캔
  SW_I2C_Scan();

  // EEPROM 테스트
  EEPROM_Test();

  // 0xB0 테스트
  Test_0xB0_Device();

  printf("All tests completed!\n\n");

  /* USER CODE END 2 */
```

```c
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  HAL_Delay(1000);
  }
  /* USER CODE END 3 */
```



