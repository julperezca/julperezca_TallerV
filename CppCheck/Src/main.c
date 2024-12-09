/**
 ******************************************************************************
 * @file           : main.c
 * @author         : namontoy@unal.edu.co
 * @brief          : Here starts the magic!!
 ******************************************************************************
 */

 // Directivas preprocesamiento
#include <stdint.h>
#include <stdio.h>
#include <stm32f4xx.h>

// Definicion de las cabeceras de las funciones del main
extern void configMagic(void);

   // Definicion de constante
     // declarar que esta funcuion existe para tener un debido orden

 // Definicion de variables

uint8_t diasSemana;           //
int32_t miliseconsPerYear;     //
int huevos;                  //
double presupuesto;
char letra;                  //  no usar las palabras protegidas del lenguaje C

 // definiciones de las cabeceras

uint8_t contador=0;
uint8_t segundos= 0;
uint8_t bandera =0 ;
uint32_t ejecuciones=0;
/*
 * The main function, where everything happens.
 */
uint8_t isPar(int numero);



int main(void)
{

	contador=59;
	bandera=isPar(contador);

	/* Configuramos la magia! */
//	configMagic();
}


 // Función que determina si un numero es par o impar
uint8_t isPar(int numero){
	numero=numero%2;
	if (numero){
		bandera = 0;

	}
	else{
		bandera = 1;
	}
}






//
//	while(1){
//		while(ejecuciones<1230000){
//			ejecuciones++;
//		}
//		ejecuciones=0;
//		segundos++;
//		for (uint32_t i=0;i<1230000;i++){
//		}
//		segundos++;
//		printf("%d segundos\n",segundos);

//	return 0;



//



//	segundos=1;
//	while(1){

//	}
		// variables locales   (solamente porque estas variables existen dentro  de la function)
//	uint16_t presupuesto = 10000;
//	uint16_t compras = 5000;
//	uint16_t operacion;
//	operacion = operacionClase(presupuesto,compras);        // sentencia
//	printf("Presupuesto: %d\n",presupuesto);    // Ciclo principal
//	printf("Precio: %d\n",compras);
//	printf("sobrante: %d\n",operacion);
//	printf("%p\n",&operacion);
//		printf("Hola Mundo!\n");
//		printf("Characters: %c %c\n", 'a', 65);
//		printf("Decimals: %d %ld\n", 1977, 650000L);
//		printf("Preceding with blanks: %10d\n", 1977);
//		printf("Preceding with zeros: %010d\n", 1977);
//		printf("Some different radices: %d %x %o %#x %#o\n", 100, 100, 100, 100,
//				100);
//		printf("floats: %4.2f %+.0e %E\n", 3.1416, 3.1416, 3.1416);
//		printf("Width trick: %*d\n", 5, 10);
//		printf("%s\n", "A string");



/**/
//uint16_t operacionClase(uint16_t budget, uint16_t cost){
//	uint16_t auxOperation;
//	auxOperation = budget - cost;
////	printf("\d",auxOperation)
//	return auxOperation;
//}

