/**
 ******************************************************************************
 * @file           : main.c
 * @author         : ImJeviz
 * @brief          : Main program body
 ******************************************************************************
 */
#include <stdint.h>
#include "stm32f4xx.h"
#include "gpio_driver_hal.h"
#include "stm32_assert.h"
#include "exti_driver_hal.h"
#include "timer_driver_hal.h"

//Definimos un PIN de prueba
GPIO_Handler_t userLed	 	= {0}; // PinA5
GPIO_Handler_t userLed1 	= {0}; // PinC9
GPIO_Handler_t userLed2 	= {0}; // PinB8
GPIO_Handler_t userLed3 	= {0}; // PinC8
GPIO_Handler_t userLed4 	= {0}; // PinC6
GPIO_Handler_t userClk 		= {0}; // PinB9
GPIO_Handler_t userData 	= {0}; // PinC5

Timer_Handler_t blinkyTimer = {0}; //TIM2

EXTI_Config_t extiClk	 	= {0};


enum{
	silencio = 0,
	volumen_medio,
	volumen_alto,
	volumen_muy_alto
};

uint8_t estado = 0;
uint8_t data = 0;
uint8_t clock = 0;
uint8_t flag_enconder = 0;

void init_System(void);
/*
 * The main function, where everything happens.
 */
int main (void){

	init_System();

	while(1){
		if(flag_enconder==1){
			if(data == 0){
				if(estado == 3){
					estado = 0;
				}else{
					estado++;
				}
			}else{
				if(estado == 0){
				estado = 3;
			}else{
				estado--;
			}
			}
			flag_enconder = 0;
		}


		switch(estado){
		case silencio:{
			gpio_WritePin(&userLed1, RESET);
			gpio_WritePin(&userLed2, RESET);
			gpio_WritePin(&userLed3, RESET);
			gpio_WritePin(&userLed4, RESET);
			break;
		}
		case volumen_medio:{
			gpio_WritePin(&userLed1, SET);
			gpio_WritePin(&userLed2, RESET);
			gpio_WritePin(&userLed3, RESET);
			gpio_WritePin(&userLed4, RESET);
			break;
		}
		case volumen_alto:{
			gpio_WritePin(&userLed1, SET);
			gpio_WritePin(&userLed2, SET);
			gpio_WritePin(&userLed3, RESET);
			gpio_WritePin(&userLed4, RESET);
			break;
		}
		case volumen_muy_alto:{
			gpio_WritePin(&userLed1, SET);
			gpio_WritePin(&userLed2, SET);
			gpio_WritePin(&userLed3, SET);
			gpio_WritePin(&userLed4, RESET);
			break;
		}
		default:{
			//No se hace nada
			break;
		}
		}

	}
}



void init_System(void){

	//Configuramos el pin
	userLed.pGPIOx							= GPIOA;
	userLed.pinConfig.GPIO_PinNumber		= PIN_5;
	userLed.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	userLed.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	userLed.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	userLed.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;

	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&userLed);

	//Configuramos el pin
	userLed1.pGPIOx							= GPIOC;
	userLed1.pinConfig.GPIO_PinNumber		= PIN_9;
	userLed1.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	userLed1.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	userLed1.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	userLed1.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;

	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&userLed1);

	//Configuramos el pin
	userLed2.pGPIOx							= GPIOB;
	userLed2.pinConfig.GPIO_PinNumber		= PIN_8;
	userLed2.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	userLed2.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	userLed2.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	userLed2.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;

	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&userLed2);

	//Configuramos el pin
	userLed3.pGPIOx							= GPIOC;
	userLed3.pinConfig.GPIO_PinNumber		= PIN_8;
	userLed3.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	userLed3.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	userLed3.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	userLed3.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;

	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&userLed3);


	//Configuramos el pin
	userLed4.pGPIOx							= GPIOC;
	userLed4.pinConfig.GPIO_PinNumber		= PIN_6;
	userLed4.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	userLed4.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	userLed4.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	userLed4.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;

	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&userLed4);

	//Configuramos el pin
	userClk.pGPIOx							= GPIOB;
	userClk.pinConfig.GPIO_PinNumber		= PIN_9;
	userClk.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	userClk.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;

	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&userClk);

	//Configuramos el pin
	userData.pGPIOx							= GPIOC;
	userData.pinConfig.GPIO_PinNumber		= PIN_5;
	userData.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	userData.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;

	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&userData);

	blinkyTimer.pTIMx								= TIM2;
	blinkyTimer.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;
	blinkyTimer.TIMx_Config.TIMx_Prescaler  		= 16000; //1ms conversion
	blinkyTimer.TIMx_Config.TIMx_Period				= 250;
	blinkyTimer.TIMx_Config.TIMx_InterruptEnable 	= TIMER_INT_ENABLE;

	timer_Config(&blinkyTimer);

	extiClk.pGPIOHandler					= &userClk;
	extiClk.edgeType						= EXTERNAL_INTERRUPT_RISING_EDGE;

	exti_Config(&extiClk);

	gpio_WritePin(&userLed, SET);
}

/*
 * Overwrite function
 */
void Timer2_Callback(void){
	gpio_TooglePin(&userLed);
}
void callback_ExtInt9(void){
	data = gpio_ReadPin(&userData);
	clock = gpio_ReadPin(&userClk);
	flag_enconder = 1;
}

/*
 * Esta función sirve para detectar problemas de parametros
 * incorrectos al momento de ejecutar un programa.
 */
void assert_failed(uint8_t* file, uint32_t line){
	while(1){
		//Problems
	}
}
