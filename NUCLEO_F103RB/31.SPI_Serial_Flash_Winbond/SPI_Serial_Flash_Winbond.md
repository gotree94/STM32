# SPI_Serial_Flash_Winbond

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
