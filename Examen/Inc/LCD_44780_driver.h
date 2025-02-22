/*
 * LCD_44780_driver.h
 *
 *  Created on: 22 Feb 22, 2025
 *      Author: julperezca
 */

#ifndef LCD_HD44780_DRIVER_H_
#define LCD_HD44780_DRIVER_H_

#include "stm32f4xx.h"

/* Puedo definir todos los def de las direcciones */


#define LCD_I2C_ADDR 			0x21  //  pantalla 18
#define INITIALIZATION_VALUE 	0x03 // valor de inicialización  que se realizatres veces
#define FOUR_BITS_MODE 			0x02	// MODO DE 4 BITS
#define ROW_COLUMNS 			0x28 // FILAS Y COLUMNAS
#define DISPLAY_ON 				0x0E  // Display ON, sin cursor
#define MODE_INCREMENT 			0x06  // modo de incremento
#define CLEAN_LCD	 			0x01  // Limpiar pantalla


#endif /* LCD_HD44780_DRIVER_H_ */
