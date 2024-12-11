/*
 * CheckSourcesMain.c
 *
 *  Created on: Dec 10, 2024
 *      Author: Julián Pérez Carvajal
 */

#include <stdint.h>
#include <stdio.h>
#include <stm32f4xx.h>

/* Definición de variables */
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

/* 	 return un valor 1 o 0, no se necesita una variable de 32 bits para almacenar ese valor, utilizar un
 	 entero sin signo de 8 o 16 bits
 	 si es mayor a 45 inmediatamente es mayor a 25, sobra un if.
 	 nunca llega a cumplir la condición de rotation<23 pues para llegar ahí rotation debe
 	 ser mayor a 45, pues para llegar a este  */
// Para tener en cuenta: rotation es una variable global que no tiene definido ningun valor, por lo que
// se evaluará la función en un dato desconocido previamente al debugin.

uint32_t checkRotation(void){
	if(rotation > 45){
		return 1;
	}
	else if(rotation < 23){
		return 0;
		}
	else{
		return 1;
	}
}



/* Describir cuales son los problemas que hay en esta función y como se pueden corregir */
/*
   Función que entra en un ciclo for si getData==1. El ciclo for va desde un indice i=0 hasta 9.
   El indice se va sumando, pero luego de multiplicar el indice por 35 que es almacenado
   y se le resta 1 a el valor del índice, por lo que i=-1, luego entra al for y i=1-1=0, siempre
   se queda con el valor de 0.
   Además si se corrige que el for esté indefinidamente, el valor que siempre devolverá
   la función será un 0, pues al salir del condicional se o no se cumpla este de igual manera
   retornará 0
   */

/* Se realiza la corrección de quitar el i-- para que no esté indefinidamente tomando valores de cero */
/* Se añade un else para que no sea cero el valor que tome siempre el return al final del llamado de la
 * funcioón getTemperature.
 */

int16_t getTemperature(uint8_t getData){
	if(getData == 1){
		for(int i = 0; i < 10; i++){
			temperatureValue = i*35;
		}
		return temperatureValue;
	}
	return 0;
}


/* Describir cuales son los problemas que hay en esta función y como se pueden corregir */


 /*
  *  Se tiene a[10] de diez elementos de 0 hasta 9. Se está evaluando en a[10]
  * Se tiene una variable de uint8, toma valor máximo 255. Por lo que 350 no corresponde a un
  * valor definido para esa cantidad de bits.
  *  */
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



/* Describir cuales son los problemas que hay en esta función y como se pueden corregir */

/* Misma salida para ambas condiciones, se retornaría 1 independientemente del valor data*/
uint8_t positiveFunction(uint8_t data){
	if(data == 1){
		return 1;
	}
	else{
		return 0;
	}
}



/* Describir cuales son los problemas que hay en esta función y como se pueden corregir */

/*
 * Return 0, y return 0, no se importa que se haga en la función, retornará 0.
 * No se tiene funcionalidad de werid, pues solo se necesita, pues si data es igual a 25, weird tomará
 * el valor de 25 el condicional se cumple. Pero si data es diferente de 25, no se cumplirá el condicional
 * */
uint8_t weirdFunction(uint8_t data){
	if(data == 25){
		return 0;
	}
	else{
		return 1;
	}
}


/* Describir cuales son los problemas que hay en esta función y como se pueden corregir */


/* se intenta acceder fuera de los limites de data pues solo tiene "abc"
 * - la variable dummy no está haciendo nada en la función ni tampoco el argumento value
 * - no *data tiene característica de puntero, por lo que se le está intentando ingresar a
 *  un valor de una posición de mem. hacia donde apunta.
 * */

int dostuff(char *data, int value){
	data = data;
	dummy = value + 1;

	return 0;
}







