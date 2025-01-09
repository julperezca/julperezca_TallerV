/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Julián Pérez Carvajal (julperezca@unal.edu.co)
 * @brief          : Tarea 2. Drivers GPIO, EXTI, TIMERS, magicProject.
 ******************************************************************************
 */
#include <stdio.h>
#include <stdint.h>
#include "stm32f4xx.h"
#include "gpio_driver_hal.h"
#include "stm32_assert.h"
#include "exti_driver_hal.h"
#include "timer_driver_hal.h"
#include "systick_driver.h"



GPIO_Handler_t  ledState     = {0};

extern void configMagic(void);
void init_Config(void);					// Función que inicia la config. de los pines, timers y EXTI
int main (void){
	init_Config();
	configMagic();
//	printf("value: %u\n", currentValue);

	while(1){
		uint32_t currentValue = ticksNumber();
		printf("current value: %lu\n", currentValue);
		msDelay(1000);
		gpio_TooglePin(&ledState);

	}
}

/* Funcion encargada de la configuración del GPIO, TIMERS y EXTIs */
void init_Config(void){

			/* Configuración de LED de estado y su respectivo timer */

	// GPIO config para Led de estado
	ledState.pGPIOx							= GPIOH;
	ledState.pinConfig.GPIO_PinNumber		= PIN_1;
	ledState.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	ledState.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	ledState.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_FAST;
	ledState.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&ledState);

	systickConfig();    // 16, 80, 100. Si se selecciona ExternalClock debe seleccionarse 80 ó 100

	// Inicialmente el pin de estado está encendido
	gpio_WritePin(&ledState, SET);
}


void assert_failed(uint8_t* file, uint32_t line){
	while(1){
		//Problems
	}
}
