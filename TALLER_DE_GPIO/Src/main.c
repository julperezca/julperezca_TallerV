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
GPIO_Handler_t pinLed5 = {0}; // PinA5
GPIO_Handler_t pinLed6 = {0}; // PinC6
GPIO_Handler_t pinLed7 = {0}; // PinA7
GPIO_Handler_t pinLed8 = {0}; // PinB8
GPIO_Handler_t userBtn = {0}; // PinC13

uint8_t contador = 0;
uint8_t flag	 = 0;

uint8_t Velitas(void);
/*
 * the main funct, where eveerything happens
 * */
int main (void)
{
	/* configuramos el pin */
	pinLed5.pGPIOx							= GPIOA;
	pinLed5.pinConfig.GPIO_PinNumber		= PIN_5;
	pinLed5.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	pinLed5.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	pinLed5.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	pinLed5.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;

	/* Cargamos la configuración en los registros que gobiernan el puerto */
	gpio_Config(&pinLed5);




	/* configuramos el pin */
	pinLed6.pGPIOx							= GPIOC;
	pinLed6.pinConfig.GPIO_PinNumber		= PIN_6;
	pinLed6.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	pinLed6.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	pinLed6.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	pinLed6.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;

	/* Cargamos la configuración en los registros que gobiernan el puerto */
	gpio_Config(&pinLed6);





	/* configuramos el pin */
	pinLed7.pGPIOx							= GPIOA;
	pinLed7.pinConfig.GPIO_PinNumber		= PIN_7;
	pinLed7.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	pinLed7.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	pinLed7.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	pinLed7.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;

	/* Cargamos la configuración en los registros que gobiernan el puerto */
	gpio_Config(&pinLed7);





	/* configuramos el pin */
	pinLed8.pGPIOx							= GPIOB;
	pinLed8.pinConfig.GPIO_PinNumber		= PIN_8;
	pinLed8.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	pinLed8.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	pinLed8.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	pinLed8.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;

	/* Cargamos la configuración en los registros que gobiernan el puerto */
	gpio_Config(&pinLed8);




	userBtn.pGPIOx							= GPIOC;
	userBtn.pinConfig.GPIO_PinNumber		= PIN_13;
	userBtn.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	userBtn.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;

	/* Cargamos la configuración en los registros que gobiernan el puerto */
	gpio_Config(&userBtn);


	gpio_WritePin(&pinLed5, SET);


	contador = 1;

	while(1){
			flag = Velitas();
			flag = 0;
	}
}


uint8_t Velitas(void){

	switch(contador){

	case 0:{
		gpio_WritePin(&pinLed5, 1);
		contador++;
		break;
	}
	case 1:{
		gpio_WritePin(&pinLed6, 1);
		gpio_WritePin(&pinLed5, 0);
		contador++;
		break;
	}
	case 2:{
		gpio_WritePin(&pinLed7, 1);
		gpio_WritePin(&pinLed6, 0);
		contador++;
		break;
	}
	case 3:{
		gpio_WritePin(&pinLed8, 1);
		gpio_WritePin(&pinLed7, 0);
		contador++;
		break;
	}
	default:{
		gpio_WritePin(&pinLed5, 1);
		gpio_WritePin(&pinLed6, 1);
		gpio_WritePin(&pinLed7, 1);
		gpio_WritePin(&pinLed8, 1);
		for(uint32_t i = 0; i<1250000;i++);
		gpio_WritePin(&pinLed5, 0);
		gpio_WritePin(&pinLed6, 0);
		gpio_WritePin(&pinLed7, 0);
		gpio_WritePin(&pinLed8, 0);
		contador = 0;
		break;
	}

	}



	return 1;
}



