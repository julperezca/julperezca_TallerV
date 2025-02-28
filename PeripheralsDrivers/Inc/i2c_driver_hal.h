/*
 * i2c_driver_hal.h
 *
 *  Created on: 
 *      Author: julperezca
 */

#ifndef I2C_DRIVER_HAL_H2_
#define I2C_DRIVER_HAL_H2_

#include <stdint.h>
#include "stm32f4xx.h"

enum{
	eI2C_WRITE_DATA = 0,
	eI2C_READ_DATA
};

enum{
	eI2C_MODE_SM = 0,
	eI2C_MODE_FM
};

#define I2C_MAIN_CLOCK_4_MHz	4
#define I2C_MAIN_CLOCK_16_MHz	16
#define I2C_MAIN_CLOCK_20_MHz	20

#define I2C_MODE_SM_SPEED	88	// 100KHz clock signal
#define I2C_MODE_FM_SPEED	13  // 400KHz clock signal

#define I2C_MAX_RISE_TIME_SM	17
#define I2C_MAX_RISE_TIME_FM	5


typedef struct{
	I2C_TypeDef		*pI2Cx;
	uint8_t			slaveAddress;
	uint8_t			i2c_mode;
	uint8_t			i2c_mainClock;
	uint8_t			i2c_data;
}I2C_Handler_t;



/* prototipos de las funciones publicas */
void i2c_Config(I2C_Handler_t *pHandlerI2C);
uint8_t i2c_ReadSingleRegister(I2C_Handler_t *pHandlerI2C, uint8_t regToRead);
uint8_t i2c_ReadManyRegister(I2C_Handler_t *pHandlerI2C, uint8_t regToRead, uint8_t *bufferRxData, uint8_t numerOfBytes);
void i2c_WriteSingleRegister(I2C_Handler_t *pHandlerI2C, uint8_t regToWrite, uint8_t newValue);
void i2c_WriteManyRegisters(I2C_Handler_t *pHandlerI2C, uint8_t regToWrite, uint8_t *bufferRxData, uint8_t numerOfBytes);
void i2c_WriteSingleRegisterLCD(I2C_Handler_t *pHandlerI2C, uint8_t newValue);
void i2c_send_slave_address_rw(I2C_Handler_t *pHandlerI2C, uint8_t rw);
uint8_t i2c_Read8BitsFrom16BitReg(I2C_Handler_t *pHandlerI2C, uint16_t regAddress);
#endif /* I2C_DRIVER_HAL_H_ */
