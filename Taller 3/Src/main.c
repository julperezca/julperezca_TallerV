/**
 ******************************************************************************
 * @file           : main.c
 * @author         : namontoy@unal.edu.co
 * @brief          : Here starts the magic!!
 ******************************************************************************
 */
#include <stdint.h>
#include <stdio.h>
#include <stm32f4xx.h>

// Definicion de las cabeceras de las funciones del main
extern void configMagic(void);

uint8_t contador 	= 0;
uint8_t bandera 	= 0;


/*
 * The main function, where everything happens.
 */
//uint8_t isPar(uint8_t numero);
//void isPar(int numero);
//void isImpar(int numero);
void isPar(void);
//uint8_t isPar(void);
int main(void)
{

	contador = 59;
//	bandera = isPar(contador);
	isPar();

	while (1) {
	}
	return 0;
}

////Esta funcion retorna 1 si es impar
//uint8_t isPar(uint8_t numero){
//
//	uint8_t condicion = 0;
//
//	condicion = numero%2;
//
//	return condicion;
//}

//void isPar(int numero){
//
//	numero = numero%2;
//	//si numero es impar entra a la condicion de if
//	if (numero){
//		// si el numero es impar la bandera tomara el valor de 0
//		bandera = 0;
//	}
//	else{
//		// si el numero es par entrar a esta conficion y bandera es igual a 1
//		bandera = 1;
//	}
//
//}


//void isImpar(int numero){
//                                   //funcion que no retorna nada, solo cambia la variable global bandera
//	numero = numero%2;
//
//	if (numero){                   // sería equivalente a !numero con el contrario de bandera
//
//		bandera = 1;
//	}
//	else{
//
//		bandera = 0;
//	}
//}




//uint8_t isPar(void){              // cuando no se recibe parametros se debe trabajar con variable global
//	contador=contador%2;
//	uint8_t condicion = 0;       // debe retornar algo
//
//
//	if (contador){
//		condicion = 0;
//	}
//	else{
//		condicion = 1;
//	}
//	return condicion;
//}



void isPar(void){                 // todo lo que se utiliza dentro de la void void vacía vacía deben ser globales
	contador=contador%2;
	if (contador){
		bandera=0;
	}
	else{
		bandera=1;
	}
}










