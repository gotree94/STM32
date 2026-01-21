# I2C CLCD

<img width="706" height="170" alt="129" src="https://github.com/user-attachments/assets/8fdba439-49cd-47d9-8069-223c4ea9305b" />

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

<img width="800" height="600" alt="CLCD" src="https://github.com/user-attachments/assets/a590d783-9ded-40ba-b23d-ce6f93a5b430" />

```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */
```

```c
/* USER CODE BEGIN PD */
#define delay_ms HAL_Delay

#define ADDRESS   0x3F << 1
//#define ADDRESS   0x27 << 1

#define RS1_EN1   0x05
#define RS1_EN0   0x01
#define RS0_EN1   0x04
#define RS0_EN0   0x00
#define BackLight 0x08
/* USER CODE END PD */
```

```c
/* USER CODE BEGIN PV */
int delay = 0;
int value = 0;
/* USER CODE END PV */
```

```c
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
void I2C_ScanAddresses(void);

void delay_us(int us);
void LCD_DATA(uint8_t data);
void LCD_CMD(uint8_t cmd);
void LCD_CMD_4bit(uint8_t cmd);
void LCD_INIT(void);
void LCD_XY(char x, char y);
void LCD_CLEAR(void);
void LCD_PUTS(char *str);
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

void delay_us(int us){
	value = 3;
	delay = us * value;
	for(int i=0;i < delay;i++);
}

void LCD_DATA(uint8_t data) {
	uint8_t temp=(data & 0xF0)|RS1_EN1|BackLight;

	while(HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000)!=HAL_OK);
	temp=(data & 0xF0)|RS1_EN0|BackLight;
	while(HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000)!=HAL_OK);
	delay_us(4);

	temp=((data << 4) & 0xF0)|RS1_EN1|BackLight;
	while(HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000)!=HAL_OK);
	temp = ((data << 4) & 0xF0)|RS1_EN0|BackLight;
	while(HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000)!=HAL_OK);
	delay_us(50);
}

void LCD_CMD(uint8_t cmd) {
	uint8_t temp=(cmd & 0xF0)|RS0_EN1|BackLight;
	while(HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000)!=HAL_OK);
	temp=(cmd & 0xF0)|RS0_EN0|BackLight;
	while(HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000)!=HAL_OK);
	delay_us(4);

	temp=((cmd << 4) & 0xF0)|RS0_EN1|BackLight;
	while(HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000)!=HAL_OK);
	temp=((cmd << 4) & 0xF0)|RS0_EN0|BackLight;
	while(HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000)!=HAL_OK);
	delay_us(50);
}

void LCD_CMD_4bit(uint8_t cmd) {
	uint8_t temp=((cmd << 4) & 0xF0)|RS0_EN1|BackLight;
	while(HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000)!=HAL_OK);
	temp=((cmd << 4) & 0xF0)|RS0_EN0|BackLight;
	while(HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000)!=HAL_OK);
	delay_us(50);
}

void LCD_INIT(void) {

	delay_ms(100);

	LCD_CMD_4bit(0x03); delay_ms(5);
	LCD_CMD_4bit(0x03); delay_us(100);
	LCD_CMD_4bit(0x03); delay_us(100);
	LCD_CMD_4bit(0x02); delay_us(100);
	LCD_CMD(0x28);  // 4 bits, 2 line, 5x8 font
	LCD_CMD(0x08);  // display off, cursor off, blink off
	LCD_CMD(0x01);  // clear display
	delay_ms(3);
	LCD_CMD(0x06);  // cursor movint direction
	LCD_CMD(0x0C);  // display on, cursor off, blink off
}

void LCD_XY(char x, char y) {
	if      (y == 0) LCD_CMD(0x80 + x);
	else if (y == 1) LCD_CMD(0xC0 + x);
	else if (y == 2) LCD_CMD(0x94 + x);
	else if (y == 3) LCD_CMD(0xD4 + x);
}

void LCD_CLEAR(void) {
	LCD_CMD(0x01);
	delay_ms(2);
}

void LCD_PUTS(char *str) {
	while (*str) LCD_DATA(*str++);
}
/* USER CODE END 0 */
```

```c
  /* USER CODE BEGIN 2 */
  I2C_ScanAddresses();

  LCD_INIT();
  LCD_XY(0, 0) ; LCD_PUTS((char *)"LCD Display test");
  LCD_XY(0, 1) ; LCD_PUTS((char *)"Hello World.....");

  /* USER CODE END 2 */
```

---

# 사용자 문자 

![CLCD](https://github.com/user-attachments/assets/1f1e5482-9e28-4fdb-aeee-41385e28646c)



* /* USER CODE BEGIN 0 */ 섹션에 아래 함수를 추가ㅊ

```c
// 커스텀 문자 패턴 정의 (5x8 도트, 8바이트)
// 하트 모양
const uint8_t heart[8] = {
    0b00000,
    0b01010,
    0b11111,
    0b11111,
    0b11111,
    0b01110,
    0b00100,
    0b00000
};

// 스마일 모양
const uint8_t smiley[8] = {
    0b00000,
    0b00000,
    0b01010,
    0b00000,
    0b10001,
    0b01110,
    0b00000,
    0b00000
};

// 화살표 위
const uint8_t arrow_up[8] = {
    0b00100,
    0b01110,
    0b11111,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00000
};

// 화살표 아래
const uint8_t arrow_down[8] = {
    0b00000,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b11111,
    0b01110,
    0b00100
};

// 온도계 모양
const uint8_t thermometer[8] = {
    0b00100,
    0b01010,
    0b01010,
    0b01010,
    0b01110,
    0b11111,
    0b11111,
    0b01110
};

// 종 모양
const uint8_t bell[8] = {
    0b00100,
    0b01110,
    0b01110,
    0b01110,
    0b11111,
    0b00000,
    0b00100,
    0b00000
};

// 배터리 모양
const uint8_t battery[8] = {
    0b01110,
    0b11011,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b11111
};

// 스피커 모양
const uint8_t speaker[8] = {
    0b00001,
    0b00011,
    0b01111,
    0b01111,
    0b01111,
    0b00011,
    0b00001,
    0b00000
};

/**
 * @brief CGRAM에 커스텀 문자 등록
 * @param location: 문자 번호 (0~7)
 * @param pattern: 8바이트 패턴 배열
 */
void LCD_CreateChar(uint8_t location, const uint8_t *pattern) {
    if (location > 7) return;  // 최대 8개 문자만 가능
    
    // CGRAM 주소 설정: 0x40 + (location * 8)
    LCD_CMD(0x40 | (location << 3));
    
    // 8바이트 패턴 데이터 쓰기
    for (int i = 0; i < 8; i++) {
        LCD_DATA(pattern[i]);
    }
    
    // DDRAM 모드로 복귀 (커서를 홈 위치로)
    LCD_CMD(0x80);
}

/**
 * @brief 커스텀 문자 출력
 * @param location: 등록된 문자 번호 (0~7)
 */
void LCD_PutCustomChar(uint8_t location) {
    if (location > 7) return;
    LCD_DATA(location);
}

/**
 * @brief 모든 커스텀 문자 등록
 */
void LCD_CreateAllCustomChars(void) {
    LCD_CreateChar(0, heart);
    LCD_CreateChar(1, smiley);
    LCD_CreateChar(2, arrow_up);
    LCD_CreateChar(3, arrow_down);
    LCD_CreateChar(4, thermometer);
    LCD_CreateChar(5, bell);
    LCD_CreateChar(6, battery);
    LCD_CreateChar(7, speaker);
}
```

* main() 함수 수정
* /* USER CODE BEGIN 2 */ 섹션을 다음과 같이 수정:

```c
/* USER CODE BEGIN 2 */
I2C_ScanAddresses();

LCD_INIT();

// 커스텀 문자 등록
LCD_CreateAllCustomChars();

// 예제 1: 커스텀 문자 표시
LCD_XY(0, 0);
LCD_PUTS("Custom Chars:");

LCD_XY(0, 1);
LCD_PutCustomChar(0);  // 하트
LCD_DATA(' ');
LCD_PutCustomChar(1);  // 스마일
LCD_DATA(' ');
LCD_PutCustomChar(2);  // 화살표 위
LCD_DATA(' ');
LCD_PutCustomChar(3);  // 화살표 아래
LCD_DATA(' ');
LCD_PutCustomChar(4);  // 온도계
LCD_DATA(' ');
LCD_PutCustomChar(5);  // 종
LCD_DATA(' ');
LCD_PutCustomChar(6);  // 배터리
LCD_DATA(' ');
LCD_PutCustomChar(7);  // 스피커

/* USER CODE END 2 */
```

## 실행 결과
```
Line 0: Custom Chars:
Line 1: ♥ ☺ ↑ ↓ 🌡 🔔 🔋 🔊
```

* 응용 예제: 온도 표시

```c
// while 루프 안에서 사용
LCD_CLEAR();
LCD_XY(0, 0);
LCD_PUTS("Temperature:");

LCD_XY(0, 1);
LCD_PutCustomChar(4);  // 온도계 아이콘
LCD_PUTS(" 25.5");
LCD_DATA(0xDF);        // 도(°) 기호 (내장 문자)
LCD_PUTS("C ");
LCD_PutCustomChar(2);  // 상승 화살표
```


