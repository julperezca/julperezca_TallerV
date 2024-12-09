/*
 * CheckSourcesMain.c
 *
 *  Created on: Dec 2, 2024
 *      Author: namontoy
 */

#include <stdint.h>
#include <stdio.h>
#include <stm32f4xx.h>

/* Definiciond de variables */
uint32_t rotation;
int16_t temperatureValue;
int dummy;

// Definicion de las cabeceras de las funciones del main
extern void configMagic(void);
int16_t getTemperature(uint8_t getData);
uint32_t checkRotation(void);
uint8_t leaking(uint16_t const range);
uint8_t positiveFunction(uint8_t data);
int dostuff(char *data, int value);
uint8_t weirdFunction(uint8_t data);



/*
 * The main function, where everything happens.
 */
int main(void)
{
	/* Configuramos la magia! */
	configMagic();

	// Ciclo principal
	printf("Hola Mundo!\n");

	/* Probando diferentes funciones */
	checkRotation();
	getTemperature(23);
	leaking(350);
	positiveFunction(2);
	dostuff("abc", 1024);
	weirdFunction(26);

	while (1) {
	}

	return 0;
}

/* Describir cuales son los problemas que hay en esta función y como se pueden corregir */
uint32_t checkRotation(void){

	if(rotation > 25){
		if(rotation > 45){
			if(rotation < 23){
				return 0;
			}
		}
		return 1;
	}
	return 1;
}

/* Describir cuales son los problemas que hay en esta función y como se pueden corregir */
int16_t getTemperature(uint8_t getData){
	if(getData == 1){
		for(int i = 0; i < 10; i++){
			temperatureValue = i*35;
			i--;
		}
		return temperatureValue;
	}
	return 0;
}

/* Describir cuales son los problemas que hay en esta función y como se pueden corregir */
uint8_t leaking(uint16_t const range){
	char a[10];

	/* Utilice una linea de codigo del ciclo FOR y luego la otra, ¿que observa en la salida del cppcheck?*/
	for(uint8_t size = 0; size < range; size++){
	//for(uint8_t size = 0; size < 350; size++){
		a[size] = size*2;
	}

	/* Utilice una linea de codigo del ciclo FOR y luego la otra, ¿que observa en la salida del cppcheck?*/
	//return a[9];
	return a[10];
}

/* Describir cuales son los problemas que hay en esta función y como se pueden corregir */
uint8_t positiveFunction(uint8_t data){
	if(data == 1){
		return 1;
	}else{
		return 1;
	}
}

/* Describir cuales son los problemas que hay en esta función y como se pueden corregir */
uint8_t weirdFunction(uint8_t data){
	uint8_t weird = 10;
	if((weird = data) == 25){
		return 0;
	}else{
		return 0;
	}
}

/* Describir cuales son los problemas que hay en esta función y como se pueden corregir */
int dostuff(char *data, int value){
	data[125] = 200;
	dummy = value + 1;

	return 0;
}
