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

/* return un valor 1 o 0, no se necesita una variable de 32 bits para almacenar ese valor, utilizar un
 entero sin signo de 8 o 16 bits*/
//  si es mayor a 45 inmediatamente es mayor a 25, sobra un if.
// nunca llega a cumplir la condición de rotation<23 pues para llegar ahí rotation debe
// ser mayor a 45, pues para llegar a este
uint32_t checkRotation(void){


		if(rotation > 45){
			if(rotation < 23){ 		 // nunca llega a cumplir esta condición, pues para llegar a este
				return 0;			 // punto se debió pasa por rotation>45
			}
		}
		return 1;

	return 1;
}



/* Describir cuales son los problemas que hay en esta función y como se pueden corregir */
/*
   Función que entra en un ciclo for si getData==1. El ciclo for va desde un indice i=0 hasta 9
   este ciclo, el indice se va sumando, pero luego de multiplicar el indice por 35 que es almacenado
   y se le resta 1 a el valor del índice, por lo que i=-1, luego entra al for y i=1-1=0, siempre
   se queda con el valor de 0.

   */

/*

 */
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

/*
 Se tiene a[10] de diez elementos de 0 hasta 9. Se está evaluando en a[10].
 */
/* Describir cuales son los problemas que hay en esta función y como se pueden corregir */
uint8_t leaking(uint16_t const range){
	char a[10];

	/* Utilice una linea de codigo del ciclo FOR y luego la otra, ¿que observa en la salida del cppcheck?*/
	for(uint8_t size = 0; size < range; size++){
	//for(uint8_t size = 0; size < 350; size++){
		a[size] = size*2;
	}

	/* Utilice una linea de codigo del ciclo FOR y luego la otra, ¿que observa en la salida del cppcheck?*/
	return a[9];
	//return a[10];
}

//misma salida para ambas condiciones
/* Describir cuales son los problemas que hay en esta función y como se pueden corregir */
uint8_t positiveFunction(uint8_t data){
	if(data == 1){
		return 1;
	}else{
		return 0;
	}
}
'a'

//return 0 return 0, mismo return
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
