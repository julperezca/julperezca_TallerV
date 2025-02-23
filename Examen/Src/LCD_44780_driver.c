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
void LCD_sendNibble(I2C_Handler_t *pHandlerI2C, uint8_t nibble, uint8_t rs);
void LCD_sendByte(I2C_Handler_t *pHandlerI2C, uint8_t rs, uint8_t data);
void Crystalfontz_LCD(I2C_Handler_t *pHandlerI2C , uint8_t x, uint8_t y);

uint8_t address_code = 0;


void LCD_Init(I2C_Handler_t *pHandlerI2C){
    msDelay(50);  // Esperar encendido del LCD

    // se envía el nibble de inicialización tres veces
    LCD_sendNibble(pHandlerI2C, INITIALIZATION_VALUE, 0);
    msDelay(5);
    LCD_sendNibble(pHandlerI2C, INITIALIZATION_VALUE, 0);
    msDelay(1);
    LCD_sendNibble(pHandlerI2C, INITIALIZATION_VALUE, 0);
    msDelay(1);


    // se cambia de modo a 4 bits
    LCD_sendNibble(pHandlerI2C, FOUR_BITS_MODE , 0);

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
    LCD_sendByte(pHandlerI2C, 0, ROW_COLUMNS); 	    // Modo 4 bits, 2 líneas, 5x8
    LCD_sendByte(pHandlerI2C, 0, DISPLAY_ON);  		// Display ON, sin cursor
    LCD_sendByte(pHandlerI2C, 0, MODE_INCREMENT);   // Escribir de izquierda a derecha
    LCD_sendByte(pHandlerI2C, 0, CLEAN_LCD);  		// Limpiar pantalla
    LCD_sendByte(pHandlerI2C, 1, 0x48);  			// escribir letra H para verificar funcionamiento
    msDelay(2);

    /* FIN  de la inicialización */
}

/* Función que envía cuatro bits*/
/* Recibe:El handler del I2C,
 * el valor de 4 bits,
 * y RS(Select regiter) si es comando 0, o dato 1*/
void LCD_sendNibble(I2C_Handler_t *pHandlerI2C, uint8_t nibble, uint8_t rs){
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
void LCD_sendByte(I2C_Handler_t *pHandlerI2C, uint8_t rs, uint8_t data){

	// bits más significativos
    LCD_sendNibble(pHandlerI2C, data >> 4, rs);  // Enviar nibble alto
    msDelay(2);

    // bits menos significativos
    LCD_sendNibble(pHandlerI2C, data & 0x0F, rs);  // Enviar nibble bajo
    msDelay(2);
}

/* setea fila y columna del cursor*/
void LCD_setCursor(I2C_Handler_t *pHandlerI2C, uint8_t row, uint8_t col){
    uint8_t pos;
    /*
     * se selecciona la fila(row) a la cual se
     * tine una dirección asignada.
     * Ej: fila 3-> 0x94 ->> para elegir col se aumenta
     * este valor de hexadecimal
     * son 4 filas: de 0 a 3:  20 columnas: de 0 a 19
     */
    switch(row) {
	case 0:{
		pos = FIRST_ROW + col;
		break;  // Primera fila
	}
	case 1:{
		pos = SECOND_ROW + col;
		break;  // Segunda fila
	}
	case 2:{
		pos = THIRD_ROW + col;
		break;  // Tercera fila
	}
	case 3:{
		pos = FOURTH_ROW + col;
		break;  // Cuarta fila
	}
	default:{
		break;
	}
    }
    // RS 0 de comando
    LCD_sendByte(pHandlerI2C, 0, pos);  // Enviar comando de dirección
}


/*Cursor a posición 0,0*/
void LCD_cursorHome(I2C_Handler_t *pHandlerI2C){
    LCD_sendByte(pHandlerI2C, 0, 0x02);  //homing (0,0) cursor
}


/*
 * Función que recibe un string + handler de I2C
 * envía carácter por carácter a la LCD
 * */
void LCD_writeString(I2C_Handler_t *pHandlerI2C, char *msg, uint8_t row, uint8_t col){
	/*
	 * SD == 1, se envía dato
	 * se avanza en cada elemento del mensaje
	 * y se envía individualmente
	 * */
	while (*msg != '\0') {
		if (col<20){
		Crystalfontz_LCD(pHandlerI2C,row, col);
        LCD_sendByte(pHandlerI2C, 1, *msg++);
        col++;
		}
		else{
			if(row<3){
				row++;
				col = 0;
				Crystalfontz_LCD(pHandlerI2C,row, col);
		        LCD_sendByte(pHandlerI2C, 1, *msg++);
			}
			else if(row ==3){
				row = 0;
				col = 0;
				Crystalfontz_LCD(pHandlerI2C,row, col);
		        LCD_sendByte(pHandlerI2C, 1, *msg++);
			}
		}
    }
}

/*Limpieza de pantalla*/
void clean_display_lcd(I2C_Handler_t *pHandlerI2C){
	LCD_sendByte(pHandlerI2C, 0, CLEAN_LCD);  // Limpiar pantalla

	LCD_writeString(pHandlerI2C, "Screen cleared",2,3);
	LCD_sendByte(pHandlerI2C, 0, CLEAN_LCD);  // Limpiar pantalla
}


/* Limpieza de fila*/
void clean_row(I2C_Handler_t *pHandlerI2C, uint8_t row_to_clean){
	char clear[1] = " ";
	for(uint8_t i = 0; i < 20 ; i++){
		LCD_setCursor(pHandlerI2C, row_to_clean, i);
		LCD_sendByte(pHandlerI2C, 1, clear[0]);  // Limpiar de fila
	}
}

//Parpadeo de cursor
void LCD_cursor_blinky(I2C_Handler_t *pHandlerI2C,uint8_t cursorBlinky){
	if(cursorBlinky == 1){
		LCD_sendByte(pHandlerI2C, 0, BLINKY_CURSOR_ON);
	}
	else{
	LCD_sendByte(pHandlerI2C, 0, BLINKY_CURSOR_OFF);
	}
}

void Crystalfontz_LCD(I2C_Handler_t *pHandlerI2C , uint8_t x, uint8_t y){

	if ((x<20)&&(y<4)){

		switch (y) {
			case 0:	address_code = 0x80|0x00;break;
			case 1: address_code = 0x80|0x40;break;
			case 2: address_code = 0x80|0x14;break;
			case 3: address_code = 0x80|0x54;break;
			default:break;
		}
		LCD_setCursor(pHandlerI2C, x, y);
	}
}


