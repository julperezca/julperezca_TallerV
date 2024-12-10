/*
 * timer_driver_hal.c
 *
 *  Created on:
 *  		Author: Julian
 */
#include "stm32f4xx.h"
#include "stm32_assert.h"

#include "timer_driver_hal.h"

/* Variable que guardad la referencia del perfiérico que se está utilizando */
TIM_TypeDef *ptrTimerUsed;

/* === Headeers for private functions === */
static void timer_enable_clock_peripheral(Timer_Handler_t *pTimerHandler);
static void timer_set_prescaler(Timer_Handler_t *pTimerHandler);
static void timer_set_period(Timer_Handler_t *pTimerHandler);
static void timer_set_mode(Timer_Handler_t *pTimerHandler);
static void timer_config_interrupt(Timer_Handler_t *pTimerHandler);

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
void timer_Config(Timer_Handler_t *pTimerHandler){
	//Guardamos la referencia al periferico que estamos utilizando...
	ptrTimerUsed = pTimerHandler->pTIMx;

	/* 0. Desactivamos las interrupciones globales mientras configuramos el sistema.*/
	__disable_irq();
	/* 1. Activar la señal de reloj del periférico requerico */
	timer_enable_clock_peripheral(pTimerHandler);

	/* 2. Configuramos el prescaler*/
	timer_set_prescaler(pTimerHandler);

	/* 3. Configuramos si UP_COUNT o DOWN_COUNT (mode) */
	timer_set_mode(pTimerHandler);

	/*4 Configuramos el auto-reaload */
	timer_set_period(pTimerHandler);

	/* 5. Configuramos la interrupcion */
	timer_config_interrupt(pTimerHandler);

	/* x. Volvemos a activar las interrupciones del sistema */
	__enable_irq();

	/* El timer inicia apagado */
	timer_SetState(pTimerHandler, TIMER_OFF);
}

/**
 *
 */
void timer_enable_clock_peripheral(Timer_Handler_t *pTimerHandler){

	//verificamos que es un timer permitido
	assert_param(IS_TIM_INSTANCE(pTimerHandler->pTIMx));

	if (pTimerHandler->pTIMx == TIM2){
		RCC->APB1ENR |=RCC_APB1ENR_TIM2EN;
	}
	else if(pTimerHandler->pTIMx == TIM3){
		RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	}
	else{
		__NOP();
	}
}

/**
 * eL PRESCALER NOS CONFIGURA LA VELOCIDAD A LA QUE INCREMENTA EL REGISTRO
 * el CNT del timer.
 */
void timer_set_prescaler(Timer_Handler_t *pTimerHandler){

	//Verificamos que el valor del prescaler el valido
	assert_param(IS_TIMER_PRESC(pTimerHandler->TIMx_Config.TIMx_Prescaler));

	//configuramos el valor del prescaler
	pTimerHandler->pTIMx->PSC= pTimerHandler-> TIMx_Config.TIMx_Prescaler -1; //(16000000)
}

/**
 * esta función configura el limite hasta donde cuenta el timer para generar
 * un evento "update" (cuando esta contando de forma ascendente), o configura
 * el valor desde donde comienza a contar, cuando el sistema funciona de forma
 * descendente
 */
void timer_set_period(Timer_Handler_t *pTimerHandler){

	// Verificamos que el valor que genera el periodo es valido
	assert_param(IS_TIMER_PERIOD(pTimerHandler->TIMx_Config.TIMx_Prescaler));

	//acá hace falta algo...

	// configuramos el valor del autoreload
	pTimerHandler->pTIMx->ARR = pTimerHandler->TIMx_Config.TIMx_Period - 1;
}

/**
 *  UPcounter or DownCounter
 */
void timer_set_mode(Timer_Handler_t *pTimerHandler){

	//verificamos que el modo de funcionamiento es correcto
	assert_param(IS_TIMER_mode(pTimerHandler->TIMx_Config.TIMx_mode));

	//verificamos cual es el modo que se desea configurar
	if (pTimerHandler->TIMx_Config.TIMx_mode == TIMER_UP_COUNTER){
		//configuramosen modo upcounter DIR=0
		pTimerHandler->pTIMx->CR1 &= ~TIM_CR1_DIR;
	}
	else{
		//configuramos en modod down counter DIR=1
		pTimerHandler->pTIMx->CR1 |= TIM_CR1_DIR;
	}
}

/**
 *
 */
void timer_config_interrupt(Timer_Handler_t *pTimerHandler){

	//verificamos el posible valor configurado
	assert_param(IS_TIMER_INTERRUP(pTimerHandler->TIMx_Config.TIMx_InterruptEnable));

	if (pTimerHandler->TIMx_Config.TIMx_InterruptEnable == TIMER_INT_ENABLE){
		/* Activamos la interrupcion debida al timerx utilizado */
		pTimerHandler->pTIMx->DIER |= TIM_DIER_UIE;


		/* Activamos el canal del sistema NVIC para que lea la inrerrupcion*/
		if(pTimerHandler->pTIMx==TIM2){
			NVIC_EnableIRQ(TIM2_IRQn);
		}
		else if (pTimerHandler->pTIMx == TIM3){
			NVIC_EnableIRQ(TIM3_IRQn);
		}
		else{
			__NOP();
		}
	}
	else{
		/*desactivamos la interrupcion debida al timerx utilizado */
		pTimerHandler->pTIMx->DIER &= ~TIM_DIER_UIE;

		/* Desactivamos el canal del sistema NVIC para que lea la interrupcion */
		if (pTimerHandler->pTIMx ==TIM2){
			NVIC_DisableIRQ(TIM2_IRQn);
		}
		else if (pTimerHandler->pTIMx ==TIM3){
			NVIC_DisableIRQ(TIM3_IRQn);
		}
		else{
			__NOP();
		}
	}
}

/*
 *
 */
void timer_SetState(Timer_Handler_t *pTimerHandler, uint8_t newState){

	//Verificamos que el estado ingresado es adecuado
	assert_param(IS_TIMER_STATE(newState));

	/* 4. Reiniciamos el registro counter */
	pTimerHandler->pTIMx->CNT = 0;


	if (newState ==TIMER_ON){
		/* 5a. Activamos el timer (el CNTdebe comenzar a contar) */
		pTimerHandler->pTIMx->CR1 |= TIM_CR1_CEN;
	}
	else{
		/* 5b Desactivamos el timer (el CNT debe detenerse) */
		pTimerHandler->pTIMx->CR1 &= ~TIM_CR1_CEN;
	}

}


/**/
__attribute__ ((weak)) void Timer2_Callback(void){
	__NOP();
}


/* Esta es la funcion a la que apunta el sistema en el vector de interrupciones
 * se debe utilizar usando exactamente el mismo nombre definido en el vector de interrupciones
 * Al hacerlo correctamente el sistema apunta a esta funcion cuando la interrupcion se la
 * el sistema inmediatamente salta a este lugar en la memoria */
void TIM2_IRQHandler(void){
	/* limpiamos la bandera que indica que la interrupcion se ha generado */
	TIM2->SR &= ~TIM_SR_UIF;

	/*LLAMAMOS A LA FUNCION QUE SE DEBE ENCARargar de hacer algo con esta interrupcion */
	Timer2_Callback();

}




