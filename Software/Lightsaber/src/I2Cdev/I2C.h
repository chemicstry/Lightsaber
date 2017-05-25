#ifndef I2C_H
#define I2C_H

#include "stm32f10x_i2c.h"

void I2C1_Initialize(void);
void I2C_StartTransmission(I2C_TypeDef* I2Cx, uint8_t transmissionDirection,  uint8_t slaveAddress);
void I2C_WriteData(I2C_TypeDef* I2Cx, uint8_t data);
uint8_t I2C_ReadData(I2C_TypeDef* I2Cx);
void I2C_Stop(I2C_TypeDef* I2Cx);

#endif
