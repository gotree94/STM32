/* ========================================================================== */

/* w25q_flash.c */
#include "w25q_flash.h"

// CS Pin Control
static inline void W25Q_CS_Low(W25Q_HandleTypeDef *hflash) {
    HAL_GPIO_WritePin(hflash->cs_port, hflash->cs_pin, GPIO_PIN_RESET);
}

static inline void W25Q_CS_High(W25Q_HandleTypeDef *hflash) {
    HAL_GPIO_WritePin(hflash->cs_port, hflash->cs_pin, GPIO_PIN_SET);
}

// SPI Transmit/Receive
static HAL_StatusTypeDef W25Q_SPI_Transmit(W25Q_HandleTypeDef *hflash, 
                                            uint8_t *data, uint16_t size) {
    return HAL_SPI_Transmit(hflash->hspi, data, size, W25Q_TIMEOUT_MS);
}

static HAL_StatusTypeDef W25Q_SPI_Receive(W25Q_HandleTypeDef *hflash, 
                                           uint8_t *data, uint16_t size) {
    return HAL_SPI_Receive(hflash->hspi, data, size, W25Q_TIMEOUT_MS);
}

// Initialize Flash
void W25Q_Init(W25Q_HandleTypeDef *hflash, SPI_HandleTypeDef *hspi, 
               GPIO_TypeDef *cs_port, uint16_t cs_pin) {
    hflash->hspi = hspi;
    hflash->cs_port = cs_port;
    hflash->cs_pin = cs_pin;
    
    // Default to W25Q32 parameters (will be updated after reading ID)
    hflash->page_size = W25Q32_PAGE_SIZE;
    hflash->sector_size = W25Q32_SECTOR_SIZE;
    hflash->block_size = W25Q32_BLOCK_SIZE;
    hflash->total_size = W25Q32_TOTAL_SIZE;
    
    W25Q_CS_High(hflash);
    HAL_Delay(10);
}

// Read Status Register
static uint8_t W25Q_ReadStatusReg(W25Q_HandleTypeDef *hflash, uint8_t reg_num) {
    uint8_t cmd = (reg_num == 1) ? W25Q_CMD_READ_STATUS_REG1 : W25Q_CMD_READ_STATUS_REG2;
    uint8_t status = 0;
    
    W25Q_CS_Low(hflash);
    W25Q_SPI_Transmit(hflash, &cmd, 1);
    W25Q_SPI_Receive(hflash, &status, 1);
    W25Q_CS_High(hflash);
    
    return status;
}

// Check if flash is busy
bool W25Q_IsBusy(W25Q_HandleTypeDef *hflash) {
    uint8_t status = W25Q_ReadStatusReg(hflash, 1);
    return (status & W25Q_STATUS_BUSY) != 0;
}

// Wait until flash is not busy
void W25Q_WaitBusy(W25Q_HandleTypeDef *hflash) {
    uint32_t timeout = HAL_GetTick() + W25Q_TIMEOUT_MS;
    while (W25Q_IsBusy(hflash)) {
        if (HAL_GetTick() > timeout) {
            break;
        }
    }
}

// Write Enable
bool W25Q_WriteEnable(W25Q_HandleTypeDef *hflash) {
    uint8_t cmd = W25Q_CMD_WRITE_ENABLE;
    
    W25Q_CS_Low(hflash);
    HAL_StatusTypeDef status = W25Q_SPI_Transmit(hflash, &cmd, 1);
    W25Q_CS_High(hflash);
    
    return status == HAL_OK;
}

// Write Disable
bool W25Q_WriteDisable(W25Q_HandleTypeDef *hflash) {
    uint8_t cmd = W25Q_CMD_WRITE_DISABLE;
    
    W25Q_CS_Low(hflash);
    HAL_StatusTypeDef status = W25Q_SPI_Transmit(hflash, &cmd, 1);
    W25Q_CS_High(hflash);
    
    return status == HAL_OK;
}

// Read Device ID
bool W25Q_ReadID(W25Q_HandleTypeDef *hflash, uint16_t *manufacturer_id, uint16_t *device_id) {
    uint8_t cmd[4] = {W25Q_CMD_DEVICE_ID, 0x00, 0x00, 0x00};
    uint8_t data[2];
    
    W25Q_CS_Low(hflash);
    W25Q_SPI_Transmit(hflash, cmd, 4);
    HAL_StatusTypeDef status = W25Q_SPI_Receive(hflash, data, 2);
    W25Q_CS_High(hflash);
    
    if (status == HAL_OK) {
        *manufacturer_id = data[0];
        *device_id = data[1];
        
        // Update flash parameters based on device ID
        if (*device_id == 0x15) {  // W25Q32
            hflash->total_size = W25Q32_TOTAL_SIZE;
        } else if (*device_id == 0x16 || *device_id == 0x17) {  // W25Q64
            hflash->total_size = W25Q64_TOTAL_SIZE;
        }
    }
    
    return status == HAL_OK;
}

// Read JEDEC ID
bool W25Q_ReadJEDECID(W25Q_HandleTypeDef *hflash, uint8_t *manufacturer, 
                       uint8_t *memory_type, uint8_t *capacity) {
    uint8_t cmd = W25Q_CMD_JEDEC_ID;
    uint8_t data[3];
    
    W25Q_CS_Low(hflash);
    W25Q_SPI_Transmit(hflash, &cmd, 1);
    HAL_StatusTypeDef status = W25Q_SPI_Receive(hflash, data, 3);
    W25Q_CS_High(hflash);
    
    if (status == HAL_OK) {
        *manufacturer = data[0];
        *memory_type = data[1];
        *capacity = data[2];
    }
    
    return status == HAL_OK;
}

// Read Data (Standard Read)
bool W25Q_ReadData(W25Q_HandleTypeDef *hflash, uint32_t address, 
                   uint8_t *buffer, uint32_t length) {
    uint8_t cmd[4] = {
        W25Q_CMD_READ_DATA,
        (address >> 16) & 0xFF,
        (address >> 8) & 0xFF,
        address & 0xFF
    };
    
    W25Q_CS_Low(hflash);
    W25Q_SPI_Transmit(hflash, cmd, 4);
    HAL_StatusTypeDef status = W25Q_SPI_Receive(hflash, buffer, length);
    W25Q_CS_High(hflash);
    
    return status == HAL_OK;
}

// Fast Read
bool W25Q_FastRead(W25Q_HandleTypeDef *hflash, uint32_t address, 
                   uint8_t *buffer, uint32_t length) {
    uint8_t cmd[5] = {
        W25Q_CMD_FAST_READ,
        (address >> 16) & 0xFF,
        (address >> 8) & 0xFF,
        address & 0xFF,
        0xFF  // Dummy byte
    };
    
    W25Q_CS_Low(hflash);
    W25Q_SPI_Transmit(hflash, cmd, 5);
    HAL_StatusTypeDef status = W25Q_SPI_Receive(hflash, buffer, length);
    W25Q_CS_High(hflash);
    
    return status == HAL_OK;
}

// Page Program (up to 256 bytes)
bool W25Q_PageProgram(W25Q_HandleTypeDef *hflash, uint32_t address, 
                      uint8_t *buffer, uint32_t length) {
    if (length > hflash->page_size) {
        return false;
    }
    
    uint8_t cmd[4] = {
        W25Q_CMD_PAGE_PROGRAM,
        (address >> 16) & 0xFF,
        (address >> 8) & 0xFF,
        address & 0xFF
    };
    
    W25Q_WriteEnable(hflash);
    
    W25Q_CS_Low(hflash);
    W25Q_SPI_Transmit(hflash, cmd, 4);
    HAL_StatusTypeDef status = W25Q_SPI_Transmit(hflash, buffer, length);
    W25Q_CS_High(hflash);
    
    W25Q_WaitBusy(hflash);
    
    return status == HAL_OK;
}

// Sector Erase (4KB)
bool W25Q_SectorErase(W25Q_HandleTypeDef *hflash, uint32_t sector_address) {
    uint8_t cmd[4] = {
        W25Q_CMD_SECTOR_ERASE,
        (sector_address >> 16) & 0xFF,
        (sector_address >> 8) & 0xFF,
        sector_address & 0xFF
    };
    
    W25Q_WriteEnable(hflash);
    
    W25Q_CS_Low(hflash);
    HAL_StatusTypeDef status = W25Q_SPI_Transmit(hflash, cmd, 4);
    W25Q_CS_High(hflash);
    
    W25Q_WaitBusy(hflash);
    
    return status == HAL_OK;
}

// Block Erase 32KB
bool W25Q_BlockErase32K(W25Q_HandleTypeDef *hflash, uint32_t block_address) {
    uint8_t cmd[4] = {
        W25Q_CMD_BLOCK_ERASE_32K,
        (block_address >> 16) & 0xFF,
        (block_address >> 8) & 0xFF,
        block_address & 0xFF
    };
    
    W25Q_WriteEnable(hflash);
    
    W25Q_CS_Low(hflash);
    HAL_StatusTypeDef status = W25Q_SPI_Transmit(hflash, cmd, 4);
    W25Q_CS_High(hflash);
    
    W25Q_WaitBusy(hflash);
    
    return status == HAL_OK;
}

// Block Erase 64KB
bool W25Q_BlockErase64K(W25Q_HandleTypeDef *hflash, uint32_t block_address) {
    uint8_t cmd[4] = {
        W25Q_CMD_BLOCK_ERASE_64K,
        (block_address >> 16) & 0xFF,
        (block_address >> 8) & 0xFF,
        block_address & 0xFF
    };
    
    W25Q_WriteEnable(hflash);
    
    W25Q_CS_Low(hflash);
    HAL_StatusTypeDef status = W25Q_SPI_Transmit(hflash, cmd, 4);
    W25Q_CS_High(hflash);
    
    W25Q_WaitBusy(hflash);
    
    return status == HAL_OK;
}

// Chip Erase
bool W25Q_ChipErase(W25Q_HandleTypeDef *hflash) {
    uint8_t cmd = W25Q_CMD_CHIP_ERASE;
    
    W25Q_WriteEnable(hflash);
    
    W25Q_CS_Low(hflash);
    HAL_StatusTypeDef status = W25Q_SPI_Transmit(hflash, &cmd, 1);
    W25Q_CS_High(hflash);
    
    W25Q_WaitBusy(hflash);
    
    return status == HAL_OK;
}

// Write Data (handles page boundary crossing)
bool W25Q_WriteData(W25Q_HandleTypeDef *hflash, uint32_t address, 
                    uint8_t *buffer, uint32_t length) {
    uint32_t remaining = length;
    uint32_t offset = 0;
    
    while (remaining > 0) {
        uint32_t page_offset = address % hflash->page_size;
        uint32_t write_size = hflash->page_size - page_offset;
        
        if (write_size > remaining) {
            write_size = remaining;
        }
        
        if (!W25Q_PageProgram(hflash, address, buffer + offset, write_size)) {
            return false;
        }
        
        address += write_size;
        offset += write_size;
        remaining -= write_size;
    }
    
    return true;
}

