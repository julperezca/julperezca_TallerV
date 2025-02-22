/*
 * LCD_44780_driver.c
 *
 *  Created on: 22 Feb 22, 2025
 *      Author: julperezca
 */

#include "i2c_driver_hal.h"
#include "systick_driver.h"
#include "LCD_44780_driver.h"


/**/
void LCD_SendNibble(I2C_Handler_t *pHandlerI2C, uint8_t nibble, uint8_t rs);
void LCD_SendByte(I2C_Handler_t *pHandlerI2C, uint8_t rs, uint8_t data);

void LCD_Init(I2C_Handler_t *pHandlerI2C) {
    msDelay(50);  // Esperar encendido del LCD

    // se envía el nibble de inicialización tres veces
    LCD_SendNibble(pHandlerI2C, INITIALIZATION_VALUE, 0);
    msDelay(5);
    LCD_SendNibble(pHandlerI2C, INITIALIZATION_VALUE, 0);
    msDelay(1);
    LCD_SendNibble(pHandlerI2C, INITIALIZATION_VALUE, 0);
    msDelay(1);


    // se cambia de modo a 4 bits
    LCD_SendNibble(pHandlerI2C, FOUR_BITS_MODE , 0);

    /* el orden va así en los bits:
     D7 D6 D5 D4 BL E RW RS
	*/

    /* dado que ya estamos en formato 4 bits se deben
     * enviar los ocho bits en dos tandascada uno con su enable 1 --> 0
    */


    /*
     * Byte a mandar = 7,6,5,4,3,2,1,0
     *  7 6 5 4 BL E RW RS  EN 1 --> 0
     *  3 2 1 0 BL E RW RS  EN 1 --> 0
     *
    */

    /* Orden tal cual  está en la figura 24. 4-bit interface*/
    LCD_SendByte(pHandlerI2C, 0, ROW_COLUMNS); 	    // Modo 4 bits, 2 líneas, 5x8
    LCD_SendByte(pHandlerI2C, 0, DISPLAY_ON);  		// Display ON, sin cursor
    LCD_SendByte(pHandlerI2C, 0, MODE_INCREMENT);   // Escribir de izquierda a derecha
    LCD_SendByte(pHandlerI2C, 1, 0x48);  			// escribir letra H para verificar funcionamiento
    LCD_SendByte(pHandlerI2C, 0, CLEAN_LCD);  		// Limpiar pantalla
    msDelay(2);

    /* FIN  de la inicialización */
}

/* Función que envía cuatro bits*/
/* Recibe:El handler del I2C,
 * el valor de 4 bits,
 * y RS(Select regiter) si es comando 0, o dato 1*/
void LCD_SendNibble(I2C_Handler_t *pHandlerI2C, uint8_t nibble, uint8_t rs){
    uint8_t data = 0;
    data |= (nibble << 4); // D4, D5, D6, D7  cuatro bits mas significativos
    data |= (rs << 0); // comando o dato bit 0
    data |= (0x08);  // backlight P3

    // enable (E = 1 --> E = 0)

    i2c_WriteSingleRegister(pHandlerI2C, 0, data | 0x04);  // E = 1
    msDelay(1);
    i2c_WriteSingleRegister(pHandlerI2C, 0, data);         // E = 0
    msDelay(1);
}


/* Función que recibe byte u 8 bits y los descompone para enviar
 * el más significativo primero y luego el menos significativo
 * */
/* Recibe:El handler del I2C,
 * el valor de 8 bits,
 * y RS(Select regiter) si es comando 0, o dato 1*/
void LCD_SendByte(I2C_Handler_t *pHandlerI2C, uint8_t rs, uint8_t data){

	// bits más significativos
    LCD_SendNibble(pHandlerI2C, data >> 4, rs);  // Enviar nibble alto
    msDelay(2);

    // bits menos significativos
    LCD_SendNibble(pHandlerI2C, data & 0x0F, rs);  // Enviar nibble bajo
    msDelay(2);
}
