/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    user_diskio.c
  * @brief   SD Card driver implementation for FatFs
  ******************************************************************************
  */
/* USER CODE END Header */

#include "ff_gen_drv.h"
#include "user_diskio.h"
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN 0 */
extern SPI_HandleTypeDef hspi1;

/* SD Card Commands */
#define CMD0    (0)         /* GO_IDLE_STATE */
#define CMD1    (1)         /* SEND_OP_COND */
#define CMD8    (8)         /* SEND_IF_COND */
#define CMD9    (9)         /* SEND_CSD */
#define CMD10   (10)        /* SEND_CID */
#define CMD12   (12)        /* STOP_TRANSMISSION */
#define CMD16   (16)        /* SET_BLOCKLEN */
#define CMD17   (17)        /* READ_SINGLE_BLOCK */
#define CMD18   (18)        /* READ_MULTIPLE_BLOCK */
#define CMD23   (23)        /* SET_BLOCK_COUNT */
#define CMD24   (24)        /* WRITE_BLOCK */
#define CMD25   (25)        /* WRITE_MULTIPLE_BLOCK */
#define CMD55   (55)        /* APP_CMD */
#define CMD58   (58)        /* READ_OCR */
#define ACMD41  (0x80+41)   /* SEND_OP_COND (ACMD) */

/* SD Card Type */
#define CT_MMC      0x01
#define CT_SD1      0x02
#define CT_SD2      0x04
#define CT_BLOCK    0x08

static volatile DSTATUS Stat = STA_NOINIT;
static uint8_t CardType;

/* CS Pin Control */
#define SD_CS_LOW()   HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET)
#define SD_CS_HIGH()  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET)

/* SPI Communication Functions */
static void SPI_TxByte(uint8_t data)
{
    HAL_SPI_Transmit(&hspi1, &data, 1, 100);
}

static uint8_t SPI_RxByte(void)
{
    uint8_t dummy = 0xFF, data;
    HAL_SPI_TransmitReceive(&hspi1, &dummy, &data, 1, 100);
    return data;
}

static void SPI_RxBytePtr(uint8_t *buff)
{
    *buff = SPI_RxByte();
}

/* Wait for card ready */
static uint8_t SD_ReadyWait(void)
{
    uint8_t res;
    uint32_t timeout = 500;
    
    SPI_RxByte();
    do {
        res = SPI_RxByte();
    } while ((res != 0xFF) && --timeout);
    
    return res;
}

/* Deselect card and release SPI */
static void SD_Deselect(void)
{
    SD_CS_HIGH();
    SPI_RxByte();
}

/* Select card and wait for ready */
static int SD_Select(void)
{
    SD_CS_LOW();
    SPI_RxByte();
    
    if (SD_ReadyWait() != 0xFF) {
        SD_Deselect();
        return 0;
    }
    return 1;
}

/* Send command packet to SD card */
static uint8_t SD_SendCmd(uint8_t cmd, uint32_t arg)
{
    uint8_t n, res;
    
    if (cmd & 0x80) {
        cmd &= 0x7F;
        res = SD_SendCmd(CMD55, 0);
        if (res > 1) return res;
    }
    
    /* Select card */
    SD_Deselect();
    if (!SD_Select()) return 0xFF;
    
    /* Send command packet */
    SPI_TxByte(0x40 | cmd);
    SPI_TxByte((uint8_t)(arg >> 24));
    SPI_TxByte((uint8_t)(arg >> 16));
    SPI_TxByte((uint8_t)(arg >> 8));
    SPI_TxByte((uint8_t)arg);
    
    /* CRC */
    n = 0x01;
    if (cmd == CMD0) n = 0x95;
    if (cmd == CMD8) n = 0x87;
    SPI_TxByte(n);
    
    /* Wait for response */
    if (cmd == CMD12) SPI_RxByte();
    
    n = 10;
    do {
        res = SPI_RxByte();
    } while ((res & 0x80) && --n);
    
    return res;
}

/* USER CODE END 0 */

/* Private variables ---------------------------------------------------------*/
DSTATUS USER_initialize(BYTE pdrv)
{
    uint8_t n, cmd, ty, ocr[4];
    uint16_t timeout;
    
    SD_CS_HIGH();
    for (n = 10; n; n--) SPI_RxByte();
    
    ty = 0;
    if (SD_SendCmd(CMD0, 0) == 1) {
        timeout = 1000;
        
        if (SD_SendCmd(CMD8, 0x1AA) == 1) {
            /* SDv2 */
            for (n = 0; n < 4; n++) ocr[n] = SPI_RxByte();
            
            if (ocr[2] == 0x01 && ocr[3] == 0xAA) {
                while (timeout-- && SD_SendCmd(ACMD41, 1UL << 30));
                
                if (timeout && SD_SendCmd(CMD58, 0) == 0) {
                    for (n = 0; n < 4; n++) ocr[n] = SPI_RxByte();
                    ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
                }
            }
        } else {
            /* SDv1 or MMC */
            if (SD_SendCmd(ACMD41, 0) <= 1) {
                ty = CT_SD1;
                cmd = ACMD41;
            } else {
                ty = CT_MMC;
                cmd = CMD1;
            }
            
            while (timeout-- && SD_SendCmd(cmd, 0));
            
            if (!timeout || SD_SendCmd(CMD16, 512) != 0)
                ty = 0;
        }
    }
    
    CardType = ty;
    SD_Deselect();
    
    if (ty) {
        Stat &= ~STA_NOINIT;
    } else {
        Stat = STA_NOINIT;
    }
    
    return Stat;
}

DSTATUS USER_status(BYTE pdrv)
{
    return Stat;
}

DRESULT USER_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
    if (!count) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;
    
    if (!(CardType & CT_BLOCK)) sector *= 512;
    
    if (count == 1) {
        if ((SD_SendCmd(CMD17, sector) == 0) && SD_RxDataBlock(buff, 512))
            count = 0;
    } else {
        if (SD_SendCmd(CMD18, sector) == 0) {
            do {
                if (!SD_RxDataBlock(buff, 512)) break;
                buff += 512;
            } while (--count);
            SD_SendCmd(CMD12, 0);
        }
    }
    SD_Deselect();
    
    return count ? RES_ERROR : RES_OK;
}

DRESULT USER_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
    if (!count) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;
    
    if (!(CardType & CT_BLOCK)) sector *= 512;
    
    if (count == 1) {
        if ((SD_SendCmd(CMD24, sector) == 0) && SD_TxDataBlock(buff, 0xFE))
            count = 0;
    } else {
        if (CardType & CT_SD1) {
            SD_SendCmd(ACMD23, count);
        }
        if (SD_SendCmd(CMD25, sector) == 0) {
            do {
                if (!SD_TxDataBlock(buff, 0xFC)) break;
                buff += 512;
            } while (--count);
            if (!SD_TxDataBlock(0, 0xFD)) count = 1;
        }
    }
    SD_Deselect();
    
    return count ? RES_ERROR : RES_OK;
}

DRESULT USER_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    DRESULT res = RES_ERROR;
    
    if (Stat & STA_NOINIT) return RES_NOTRDY;
    
    switch (cmd) {
        case CTRL_SYNC:
            if (SD_Select()) res = RES_OK;
            SD_Deselect();
            break;
        case GET_SECTOR_COUNT:
            *(DWORD*)buff = 0;
            res = RES_OK;
            break;
        case GET_BLOCK_SIZE:
            *(WORD*)buff = 512;
            res = RES_OK;
            break;
        default:
            res = RES_PARERR;
    }
    
    return res;
}

/* Receive data block */
static int SD_RxDataBlock(BYTE *buff, UINT btr)
{
    uint8_t token;
    uint16_t timeout = 200;
    
    do {
        token = SPI_RxByte();
    } while ((token == 0xFF) && --timeout);
    
    if (token != 0xFE) return 0;
    
    do {
        SPI_RxBytePtr(buff++);
    } while (--btr);
    
    SPI_RxByte();
    SPI_RxByte();
    
    return 1;
}

/* Send data block */
static int SD_TxDataBlock(const BYTE *buff, BYTE token)
{
    uint8_t resp;
    uint16_t bc = 512;
    
    if (SD_ReadyWait() != 0xFF) return 0;
    
    SPI_TxByte(token);
    if (token != 0xFD) {
        do {
            SPI_TxByte(*buff++);
        } while (--bc);
        
        SPI_RxByte();
        SPI_RxByte();
        
        resp = SPI_RxByte();
        if ((resp & 0x1F) != 0x05) return 0;
    }
    
    return 1;
}