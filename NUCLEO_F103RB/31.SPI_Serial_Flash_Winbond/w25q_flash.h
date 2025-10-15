/* ========================================================================== */
/* w25q_flash.h - 프로젝트에 추가할 헤더 파일 */
/* ========================================================================== */

#ifndef W25Q_FLASH_H
#define W25Q_FLASH_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

// W25Q Commands
#define W25Q_CMD_WRITE_ENABLE       0x06
#define W25Q_CMD_WRITE_DISABLE      0x04
#define W25Q_CMD_READ_STATUS_REG1   0x05
#define W25Q_CMD_READ_STATUS_REG2   0x35
#define W25Q_CMD_WRITE_STATUS_REG   0x01
#define W25Q_CMD_PAGE_PROGRAM       0x02
#define W25Q_CMD_QUAD_PAGE_PROGRAM  0x32
#define W25Q_CMD_SECTOR_ERASE       0x20
#define W25Q_CMD_BLOCK_ERASE_32K    0x52
#define W25Q_CMD_BLOCK_ERASE_64K    0xD8
#define W25Q_CMD_CHIP_ERASE         0xC7
#define W25Q_CMD_READ_DATA          0x03
#define W25Q_CMD_FAST_READ          0x0B
#define W25Q_CMD_POWER_DOWN         0xB9
#define W25Q_CMD_RELEASE_POWER_DOWN 0xAB
#define W25Q_CMD_DEVICE_ID          0x90
#define W25Q_CMD_JEDEC_ID           0x9F
#define W25Q_CMD_READ_UNIQUE_ID     0x4B

// Status Register Bits
#define W25Q_STATUS_BUSY            0x01
#define W25Q_STATUS_WEL             0x02

// Device Parameters
#define W25Q32_PAGE_SIZE            256
#define W25Q32_SECTOR_SIZE          4096
#define W25Q32_BLOCK_SIZE           65536
#define W25Q32_TOTAL_SIZE           (4 * 1024 * 1024)  // 4MB

#define W25Q64_PAGE_SIZE            256
#define W25Q64_SECTOR_SIZE          4096
#define W25Q64_BLOCK_SIZE           65536
#define W25Q64_TOTAL_SIZE           (8 * 1024 * 1024)  // 8MB

// Timeout
#define W25Q_TIMEOUT_MS             5000

// Flash Info Structure
typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
    uint32_t page_size;
    uint32_t sector_size;
    uint32_t block_size;
    uint32_t total_size;
} W25Q_HandleTypeDef;

// Function Prototypes
void W25Q_Init(W25Q_HandleTypeDef *hflash, SPI_HandleTypeDef *hspi, 
               GPIO_TypeDef *cs_port, uint16_t cs_pin);
bool W25Q_ReadID(W25Q_HandleTypeDef *hflash, uint16_t *manufacturer_id, uint16_t *device_id);
bool W25Q_ReadJEDECID(W25Q_HandleTypeDef *hflash, uint8_t *manufacturer, 
                       uint8_t *memory_type, uint8_t *capacity);
bool W25Q_IsBusy(W25Q_HandleTypeDef *hflash);
void W25Q_WaitBusy(W25Q_HandleTypeDef *hflash);
bool W25Q_WriteEnable(W25Q_HandleTypeDef *hflash);
bool W25Q_WriteDisable(W25Q_HandleTypeDef *hflash);
bool W25Q_ReadData(W25Q_HandleTypeDef *hflash, uint32_t address, 
                   uint8_t *buffer, uint32_t length);
bool W25Q_FastRead(W25Q_HandleTypeDef *hflash, uint32_t address, 
                   uint8_t *buffer, uint32_t length);
bool W25Q_PageProgram(W25Q_HandleTypeDef *hflash, uint32_t address, 
                      uint8_t *buffer, uint32_t length);
bool W25Q_SectorErase(W25Q_HandleTypeDef *hflash, uint32_t sector_address);
bool W25Q_BlockErase32K(W25Q_HandleTypeDef *hflash, uint32_t block_address);
bool W25Q_BlockErase64K(W25Q_HandleTypeDef *hflash, uint32_t block_address);
bool W25Q_ChipErase(W25Q_HandleTypeDef *hflash);
bool W25Q_WritePage(W25Q_HandleTypeDef *hflash, uint32_t address, 
                    uint8_t *buffer, uint32_t length);
bool W25Q_WriteData(W25Q_HandleTypeDef *hflash, uint32_t address, 
                    uint8_t *buffer, uint32_t length);

#endif // W25Q_FLASH_H
