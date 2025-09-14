# I2C CLCD

  * PB8 - SCL
  * PB9 - SDA

<img width="644" height="586" alt="F103RB-pin" src="https://github.com/user-attachments/assets/5a174c00-4edc-4481-a59f-00297ecf229d" />
<br><br>

<img width="800" height="600" alt="I2C_001" src="https://github.com/user-attachments/assets/6ad1eefb-17c6-4073-8355-276e65266cdb" />
<br>
<img width="800" height="600" alt="I2C_002" src="https://github.com/user-attachments/assets/4da3d974-64f7-48d9-8080-a8792f981041" />
<br>
<img width="800" height="600" alt="I2C_003" src="https://github.com/user-attachments/assets/06a7ed9a-ca43-4011-9b45-76428a27ada8" />
<br>

```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */
```

```c
/* USER CODE BEGIN PFP */
void I2C_ScanAddresses(void);
/* USER CODE END PFP */
```

```c
/* USER CODE BEGIN 0 */
#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART1 and Loop until the end of transmission */
  if (ch == '\n')
    HAL_UART_Transmit (&huart2, (uint8_t*) "\r", 1, 0xFFFF);
  HAL_UART_Transmit (&huart2, (uint8_t*) &ch, 1, 0xFFFF);

  return ch;
}

void I2C_ScanAddresses(void) {
    HAL_StatusTypeDef result;
    uint8_t i;


    printf("Scanning I2C addresses...\r\n");


    for (i = 1; i < 128; i++) {
        /*
         * HAL_I2C_IsDeviceReady: If a device at the specified address exists return HAL_OK.
         * Since I2C devices must have an 8-bit address, the 7-bit address is shifted left by 1 bit.
         */
        result = HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(i << 1), 1, 10);
        if (result == HAL_OK) {
            printf("I2C device found at address 0x%02X\r\n", i);
        }
    }


    printf("Scan complete.\r\n");
}

/* USER CODE END 0 */
```

```c
  /* USER CODE BEGIN 2 */
  I2C_ScanAddresses();
  /* USER CODE END 2 */
```


---
CLCD "Hello World!"
---


```c

```

```c

```
```c

```

```c

```

```c

```
