/**
 ******************************************************************************
 * @file           : main.c
 * @author         : julperezca.edu.co
 * @brief          : main program body
 *  ******************************************************************************
 */












#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "stm32_assert.h"
#include "gpio_driver_hal.h"

// headers definition
int add(int x, int y);

// Definimos un Pin de prueba
GPIO_Handler_t userLed = {0}; // PinA5

/*
 * the main funct, where eveerything happens
 * */
int main (void)
{
	/* configuramos el pin */
	userLed.pGPIOx							= GPIOA;
	userLed.pinConfig.GPIO_PinNumber		= PIN_5;
	userLed.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	userLed.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	userLed.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	userLed.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;

	/* Cargamos la configuración en los registros que gobiernan el puerto */
	gpio_Config(&userLed);

	gpio_WritePin(&userLed,SET);

	while(1){

	}
}


/*
 * Esta funcion sirve para detectar problemas de parametors
 * incorrectos al momento de ejecutar el programa
 * */
void assert_failed(uint8_t* file, uint32_t line){
	while(1){
		//problems
	}
}

