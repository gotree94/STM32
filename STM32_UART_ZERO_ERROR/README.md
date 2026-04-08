# STM32 UART Zero Error Project

## Overview
This project demonstrates how to achieve **0% UART baud rate error** on STM32F103.

## Features
- Optimized clock tree (7.3728 MHz → 73.728 MHz)
- HAL & LL UART examples
- Python baud error calculator
- Logic analyzer validation guide

## Structure
```
STM32_UART_ZERO_ERROR/
├── README.md
├── Core/
├── Drivers/
├── python_tool/
├── logic_capture/
```

# UART Baud Rate Error 0% Design Guide (STM32F103)

## 📌 Overview
This document explains how to design UART communication with **0% baud rate error** using proper clock selection, specifically for STM32F103.

---

## ✅ 1. UART Baud Rate Formula

For STM32 (Oversampling = 16):

```
USARTDIV = fCK / (16 × Baud)
```

To achieve **0% error**:

```
fCK = Baud × 16 × N   (N = integer)
```

---

## ✅ 2. Standard UART-Friendly Clock

A well-known UART clock:

```
14.7456 MHz
```

This satisfies:

```
14.7456 MHz = 9600 × 1536
```

---

## ✅ 3. Recommended STM32F103 Clock Design

### 🎯 Goal
- Maintain 0% UART error
- Maximize CPU performance

### ✔ Configuration

| Parameter | Value |
|----------|------|
| HSE | 7.3728 MHz |
| PLL MUL | x10 |
| SYSCLK | 73.728 MHz |
| APB1 | 36.864 MHz |
| APB2 | 73.728 MHz |

---

## ✅ 4. Baud Rate Error Table (0% Case)

### Using fCK = 14.7456 MHz

| Baud | USARTDIV | Error |
|------|----------|------|
| 9600 | 96 | 0% |
| 19200 | 48 | 0% |
| 38400 | 24 | 0% |
| 57600 | 16 | 0% |
| 115200 | 8 | 0% |
| 230400 | 4 | 0% |
| 460800 | 2 | 0% |
| 921600 | 1 | 0% |

---

## ✅ 5. STM32F103 Practical Example

### ✔ APB1 = 36.864 MHz

| Baud | USARTDIV | Error |
|------|----------|------|
| 38400 | 60 | 0% |
| 57600 | 40 | 0% |
| 115200 | 20 | 0% |
| 230400 | 10 | 0% |

---

## ✅ 6. Error Calculation Formula

```
Baud_actual = fCK / (16 × USARTDIV)
Error (%) = (Baud_actual - Baud_target) / Baud_target × 100
```

---

## ✅ 7. Design Guidelines

### ✔ When to use 0% design
- Long distance communication (RS-485 / RS-422)
- Multi-device systems with independent clocks
- High-reliability systems (industrial / defense)

### ✔ When not required
- Short distance UART
- Stable clock on both sides
- Error tolerance within ±2%

---

## ✅ 8. Recommended Strategy

```
HSE = 7.3728 MHz
→ PLL ×10
→ SYSCLK = 73.728 MHz
→ APB1 = 36.864 MHz
```

✔ Supports multiple baud rates with 0% error  
✔ Maintains high CPU performance  

---

## 🚀 Conclusion

Using UART-friendly clocks like **7.3728 MHz / 14.7456 MHz** allows:

- Perfect baud alignment
- Zero communication error
- Robust system design

