/**
  ******************************************************************************
  * @file    vl53l0x_platform.h
  * @brief   VL53L0X Platform Layer Header
  ******************************************************************************
  */

#ifndef __VL53L0X_PLATFORM_H
#define __VL53L0X_PLATFORM_H

#include "main.h"

/* VL53L0X I2C Address */
#define VL53L0X_I2C_ADDR    0x52  // 7-bit address shifted left

/* VL53L0X Registers */
#define VL53L0X_REG_IDENTIFICATION_MODEL_ID         0xC0
#define VL53L0X_REG_SYSRANGE_START                  0x00
#define VL53L0X_REG_RESULT_RANGE_STATUS             0x14

/* Function Prototypes */
uint8_t VL53L0X_Init(I2C_HandleTypeDef *hi2c);
uint8_t VL53L0X_ReadID(I2C_HandleTypeDef *hi2c, uint8_t *id);
uint8_t VL53L0X_StartMeasurement(I2C_HandleTypeDef *hi2c);
uint8_t VL53L0X_ReadDistance(I2C_HandleTypeDef *hi2c, uint16_t *distance);
uint8_t VL53L0X_WriteReg(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t data);
uint8_t VL53L0X_ReadReg(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t *data);
uint8_t VL53L0X_ReadMulti(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t *data, uint8_t len);

#endif /* __VL53L0X_PLATFORM_H */