/*
 * lidarvl53.h
 * CREATED ON : 04/03/25
 * AUTHOR: Julián Pérez Carvajal (julperezca@unal.edu.co)
 *
 */


#ifndef LIDAR_H_
#define LIDAR_H_
#define VL53L3CX_I2C_ADDR  0x29  // Dirección I2C del sensor
#define lidar_max_mm 2000
#define VL53L3CX_MODEL_ID_REG 0xC0
#define VL53L3CX_MODULE_TYPE_REG 0xC1
#define dir_s1 0x29  // dirección de 7 bits

#include "vl53l0x_api.h"
#include "main.h"


void lidar_init(uint8_t dir);
uint16_t lidar_lee_mm(uint8_t dir);
float lidar_lee_cm(uint8_t dir);
uint8_t lidar_set_dir(uint8_t dir);

#endif /* LIDAR_H_ */
