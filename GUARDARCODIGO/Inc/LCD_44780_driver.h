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

//#define LCD_I2C_ADDR 			0x21  //  pantalla 18
#define INITIALIZATION_VALUE 	0x03 // valor de inicialización  que se realizatres veces
#define FOUR_BITS_MODE 			0x02	// MODO DE 4 BITS
#define ROW_COLUMNS 			0x28 // FILAS Y COLUMNAS
#define DISPLAY_ON 				0x0E  // Display ON, sin cursor
#define MODE_INCREMENT 			0x06  // modo de incremento
#define CLEAN_LCD	 			0x01  // Limpiar pantalla

#define FIRST_ROW				0x80
#define SECOND_ROW				0xC0
#define THIRD_ROW				0x94
#define FOURTH_ROW				0xD4
#define BLINKY_CURSOR_ON		0x0F
#define BLINKY_CURSOR_OFF		0x0E


/* Funciones públicas*/

void LCD_Init(I2C_Handler_t *pHandlerI2C);
void LCD_setCursor(I2C_Handler_t *pHandlerI2C, uint8_t row, uint8_t col);
void LCD_writeString(I2C_Handler_t *pHandlerI2C, char *msg, uint8_t row, uint8_t col);
void LCD_sendByte(I2C_Handler_t *pHandlerI2C, uint8_t rs, uint8_t data);
void clean_display_lcd(I2C_Handler_t *pHandlerI2C);
void clean_row(I2C_Handler_t *pHandlerI2C, uint8_t row_to_clean);
void LCD_cursor_blinky(I2C_Handler_t *pHandlerI2C,uint8_t cursorBlinky);




#endif /* LCD_HD44780_DRIVER_H_ */
