/*
 * timer_driver_hal.c
 *
 *  Created on:
 *  		Author: Julian
 */
#include "stm32f4xx.h"
#include "stm32_assert.h"
#include "systick_driver.h"

/* Variable que guardad la referencia del perfiérico que se está utilizando */

/* === Headeers for private functions === */


/* Funcion en la que cargamos la config del timer
 * Recordar que siempre se debe comenzar con activar la señal de reloj
 * del periferico que se está utilizando.
 * Además en este caso debemos ser cuidadosos al momento de utilizar interrupciones
 * Los Timers están conectados directamente al elemento NVIC del cortex-Mx
 * debemos configurar y/o utilizar:
 * 		-TIMx_CR1 (control Register 1)
 * 		-TIMx_SMCR (slave mode contron register ) -> mantener en 0 para modo Timer básico
 * 		-TIMx_DIER (DMA and interrupt enable register)
 * 		-TIM_SR (status register)
 * 		-TIMx_CNT (counter)
 * 		-TIMx_PSC (Pre-scaler)
 * 		-TIMx_ARR (Auto-reload register)
 *
 * 	Como vamos a trabajar con interruptions antes de config una nueva debemos desacti
 * 	el sistema global de interrupciones, activar la IRQ especifica y luego volver a
 * 	encencer el sistema
 */

/**
 *
 */
uint32_t countertick = 0;
uint32_t scaledTicks = 0;


void systick_CTRL(Systick_Handler_t *pSysTickHandler);
void systick_RVR(Systick_Handler_t *pSysTickHandler);
void systick_CVR (Systick_Handler_t *pSysTickHandler);




void sistick_Config(Systick_Handler_t *pSysTickHandler){

	// se carga la config del Reload
	systick_RVR(Systick_Handler_t *pSysTickHandler);

	// se pone el valor actual en 0
	systick_CVR (Systick_Handler_t *pSysTickHandler);


	/* 0. Desactivamos las interrupciones globales mientras configuramos el sistema.*/
	__disable_irq();

	NVIC_EnableIRQ(SysTick_IRQn);
	// Se habilita el conteo hasta cero con system exception register
	pSysTickHandler->pSysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;

	systick_CTRL(Systick_Handler_t *pSysTickHandler);

	/*. Volvemos a activar las interrupciones del sistema */
	__enable_irq();



}




void systick_CTRL(Systick_Handler_t *pSysTickHandler){

	// Se habilita el contador
	pSysTickHandler->pSysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;



	// Opcion de reloj Externo para el Systick
	if (pSysTickHandler->ExternalCLockEnable == EXTERNAL_CLOCK){

		pSysTickHandler->pSysTick->CTRL &= ~SysTick_CTRL_CLKSOURCE_Msk;
	}
	else{
		pSysTickHandler->pSysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;
	}

}

// Reload Value Register
void systick_RVR(Systick_Handler_t *pSysTickHandler){
	// el reloj es de 16MHz hasta ahora
	pSysTickHandler->pSysTick->LOAD = SYSTICKLOAD_16MHz;

}

// Current Value Register
void systick_CVR (Systick_Handler_t *pSysTickHandler){
	// inicialización de  el valor actual en cero
	pSysTickHandler->pSysTick->VAL = 0;
}






void ticksFunction(Systick_Handler_t *pSysTickHandler){
// si se alza la bandera de COUNTFLAG al llegar a cero la cuenta
	if (pSysTickHandler->pSysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk){
		// aumentar los ticks de llegada a cero
		countertick++;
		// se limpia la bandera
		pSysTickHandler->pSysTick->CTRL &= ~SysTick_CTRL_COUNTFLAG_Msk;
	}
}


void msConversion(uint32_t delay){
	while(countertick < delay){
		scaledTicks++;
	}

}







