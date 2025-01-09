/*
 * systick_driver.c
 *
 *  Created on:
 *  		Author: Julian
 */
#include "stm32f4xx.h"
#include "stm32_assert.h"
#include "systick_driver.h"

/* Variable que guardad la referencia del perfiérico que se está utilizando */

/* === Headeers for private functions === */
void systick_CTRL(void);
void systick_RVR(void);
void systick_CVR(void);



 /* luego se puede agregar  una entrada en sistick_Config para seleccionar la señal de reloj externa */

uint32_t counterTicks = 0;
uint32_t initTicks = 0;
uint32_t ticks = 0;



void systickConfig(void){

	//ticks = 0;

	// se carga la config del Reload
	systick_RVR();

	// se pone el valor actual en 0
	systick_CVR ();

	/* Lo anterior tal cual dice en la guía de usuario */

	/* 0. Desactivamos las interrupciones globales mientras configuramos el sistema.*/
	__disable_irq();

	/* Matriculamos en el vector de interrupciones  */
	NVIC_EnableIRQ(SysTick_IRQn);

	// Se habilita el conteo hasta cero con system exception register
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;

	// Se habilita el conteo y la opción de reloj externo
	systick_CTRL();

	/*. Volvemos a activar las interrupciones del sistema */
	__enable_irq();

}

// para esta función faltaría habilitar la función de reloj externo.
void systick_CTRL(void){

	// Se habilita el contador
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

	// Opcion de reloj interno para el Systick
	SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;
}

// Reload Value Register
void systick_RVR(void){
	// Se carga el valor de la señal de reloj
	SysTick->LOAD = SYSTICKLOAD_16MHz-1;
}

// Current Value Register
void systick_CVR (void){

	// Inicialización de el valor actual en cero
	SysTick->VAL = 0;
}

// devuelve el numero de ticks que se está actualizando luego de cargar la configuración systickConfig
uint32_t ticksNumber(void){
	return ticks;
}

void msDelay(uint32_t delay){

	initTicks = ticksNumber(); // initTicks, los ticks que hay inicialmente

	counterTicks = ticksNumber(); // counterTicks cambiará conforme aumenta ticks, este tomará su valor.

	// Se hará el ciclo hasta que counterTicks-initTicks <= delay
	while(counterTicks < delay + initTicks){
		counterTicks = ticksNumber();				// counterTicks irá tomando el valor de ticks que se está actualizando luego
	}												// de cargar la configuración.
}


// Manejador del Systick. Aumentarán los ticks cuando se cuente hasta cero. lo cual corresponde a que
// la máscara de COUNTFLAG en operación bitwise en el registro CTRL en el bit 16 sea igual a 1. (se levante la bandera)
void SysTick_Handler(void){
	if(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk){
		// Limpieza de la bandera
		SysTick->CTRL &= ~SysTick_CTRL_COUNTFLAG_Msk;
		// Incremento en el contador
		ticks++;
	}
}







