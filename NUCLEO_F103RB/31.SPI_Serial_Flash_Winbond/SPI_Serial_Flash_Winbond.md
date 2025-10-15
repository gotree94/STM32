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
