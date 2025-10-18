/**
  ******************************************************************************
  * @file    vl53l0x_platform.c
  * @brief   VL53L0X Platform Layer Implementation
  * @author  Your Name
  * @date    2024
  ******************************************************************************
  */

#include "vl53l0x_platform.h"

#define I2C_TIMEOUT 1000

/**
  * @brief  Write single byte to VL53L0X register
  * @param  hi2c: pointer to I2C handle
  * @param  reg: register address
  * @param  data: data to write
  * @retval 0: success, 1: error
  */
uint8_t VL53L0X_WriteReg(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t data)
{
    uint8_t buf[2] = {reg, data};
    if(HAL_I2C_Master_Transmit(hi2c, VL53L0X_I2C_ADDR, buf, 2, I2C_TIMEOUT) != HAL_OK)
        return 1;
    return 0;
}

/**
  * @brief  Read single byte from VL53L0X register
  * @param  hi2c: pointer to I2C handle
  * @param  reg: register address
  * @param  data: pointer to store read data
  * @retval 0: success, 1: error
  */
uint8_t VL53L0X_ReadReg(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t *data)
{
    if(HAL_I2C_Master_Transmit(hi2c, VL53L0X_I2C_ADDR, &reg, 1, I2C_TIMEOUT) != HAL_OK)
        return 1;
    if(HAL_I2C_Master_Receive(hi2c, VL53L0X_I2C_ADDR, data, 1, I2C_TIMEOUT) != HAL_OK)
        return 1;
    return 0;
}

/**
  * @brief  Read multiple bytes from VL53L0X register
  * @param  hi2c: pointer to I2C handle
  * @param  reg: register address
  * @param  data: pointer to store read data
  * @param  len: number of bytes to read
  * @retval 0: success, 1: error
  */
uint8_t VL53L0X_ReadMulti(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t *data, uint8_t len)
{
    if(HAL_I2C_Master_Transmit(hi2c, VL53L0X_I2C_ADDR, &reg, 1, I2C_TIMEOUT) != HAL_OK)
        return 1;
    if(HAL_I2C_Master_Receive(hi2c, VL53L0X_I2C_ADDR, data, len, I2C_TIMEOUT) != HAL_OK)
        return 1;
    return 0;
}

/**
  * @brief  Read VL53L0X Model ID
  * @param  hi2c: pointer to I2C handle
  * @param  id: pointer to store model ID
  * @retval 0: success, 1: error
  */
uint8_t VL53L0X_ReadID(I2C_HandleTypeDef *hi2c, uint8_t *id)
{
    return VL53L0X_ReadReg(hi2c, VL53L0X_REG_IDENTIFICATION_MODEL_ID, id);
}

/**
  * @brief  Initialize VL53L0X sensor with basic configuration
  * @param  hi2c: pointer to I2C handle
  * @retval 0: success, 1: error
  */
uint8_t VL53L0X_Init(I2C_HandleTypeDef *hi2c)
{
    uint8_t id;
    uint8_t temp;

    // Read device ID
    if(VL53L0X_ReadID(hi2c, &id) != 0)
        return 1;

    // Check if ID is correct (should be 0xEE)
    if(id != 0xEE)
        return 1;

    // VL53L0X initialization sequence based on VL53L0X API
    VL53L0X_WriteReg(hi2c, 0x88, 0x00);
    VL53L0X_WriteReg(hi2c, 0x80, 0x01);
    VL53L0X_WriteReg(hi2c, 0xFF, 0x01);
    VL53L0X_WriteReg(hi2c, 0x00, 0x00);
    VL53L0X_ReadReg(hi2c, 0x91, &temp);
    VL53L0X_WriteReg(hi2c, 0x00, 0x01);
    VL53L0X_WriteReg(hi2c, 0xFF, 0x00);
    VL53L0X_WriteReg(hi2c, 0x80, 0x00);

    // Set I2C standard mode
    VL53L0X_WriteReg(hi2c, 0x60, 0x00);

    // Set signal rate limit to 0.25 MCPS
    VL53L0X_WriteReg(hi2c, 0x44, 0x00);
    VL53L0X_WriteReg(hi2c, 0x45, 0x20);
    VL53L0X_WriteReg(hi2c, 0x46, 0x00);
    VL53L0X_WriteReg(hi2c, 0x47, 0x00);

    // Set measurement timing budget
    VL53L0X_WriteReg(hi2c, 0x01, 0xFF);
    VL53L0X_WriteReg(hi2c, 0x02, 0x00);

    // Set Sigma limit
    VL53L0X_WriteReg(hi2c, 0x60, 0x0C);

    // Set Vcsel pulse period
    VL53L0X_WriteReg(hi2c, 0x50, 0x08);
    VL53L0X_WriteReg(hi2c, 0x70, 0x0A);

    HAL_Delay(10);

    return 0;
}

/**
  * @brief  Start single range measurement
  * @param  hi2c: pointer to I2C handle
  * @retval 0: success, 1: error
  */
uint8_t VL53L0X_StartMeasurement(I2C_HandleTypeDef *hi2c)
{
    return VL53L0X_WriteReg(hi2c, VL53L0X_REG_SYSRANGE_START, 0x01);
}

/**
  * @brief  Read distance measurement result
  * @param  hi2c: pointer to I2C handle
  * @param  distance: pointer to store distance in mm
  * @retval 0: success, 1: error
  */
uint8_t VL53L0X_ReadDistance(I2C_HandleTypeDef *hi2c, uint16_t *distance)
{
    uint8_t data[12];
    uint8_t status;
    uint32_t timeout = 0;

    // Wait for measurement ready (bit 0 of status register)
    do {
        if(VL53L0X_ReadReg(hi2c, VL53L0X_REG_RESULT_RANGE_STATUS, &status) != 0)
            return 1;

        if(timeout++ > 5000)
            return 1;

        HAL_Delay(1);
    } while((status & 0x01) == 0);

    // Read range status and range value
    if(VL53L0X_ReadMulti(hi2c, VL53L0X_REG_RESULT_RANGE_STATUS, data, 12) != 0)
        return 1;

    // Extract distance (bytes 10 and 11)
    *distance = (uint16_t)((data[10] << 8) | data[11]);

    return 0;
}
