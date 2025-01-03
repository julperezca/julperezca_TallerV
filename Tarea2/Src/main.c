/**
 ******************************************************************************
 * @file           : main.c- Prueba de EXTI
 * @author         : Jesús, modificado por Julián
 * @brief          : Main program body
 ******************************************************************************
 */
#include <stdint.h>
#include "stm32f4xx.h"
#include "gpio_driver_hal.h"
#include "stm32_assert.h"
#include "exti_driver_hal.h"
#include "timer_driver_hal.h"

// LED de estado
GPIO_Handler_t userLed	 	= {0}; // PinB14

// LED RGB
GPIO_Handler_t ledRed 		= {0}; // PinC9
GPIO_Handler_t ledGreen 	= {0}; // PinB8
GPIO_Handler_t ledBlue	 	= {0}; // PinC8

//Definimos un PINES para siete-segmentos
GPIO_Handler_t segA 		= {0}; // PinB12 segmento a
GPIO_Handler_t segB  		= {0}; // PinA12 segmento b
GPIO_Handler_t segC 	 	= {0}; // PinC13 segmento c
GPIO_Handler_t segD  		= {0}; // PinC12 segmento d
GPIO_Handler_t segE 		= {0}; // PinC11 segmento e
GPIO_Handler_t segF 	 	= {0}; // PinA11 segmento f
GPIO_Handler_t segG 	 	= {0}; // PinB7  segmento g

//Digitos del siete segmentos
GPIO_Handler_t digito1  	= {0}; // PinC5 D1
GPIO_Handler_t digito2		= {0}; // PinC6 D2
GPIO_Handler_t digito3 	 	= {0}; // PinC10 D3
GPIO_Handler_t digito4	 	= {0}; // PinD2  D4



// Pines de encoder
GPIO_Handler_t userSw 	= {0}; // PinB15
GPIO_Handler_t userData 	= {0}; // PinB1
GPIO_Handler_t userClk 		= {0}; // PinB2



Timer_Handler_t blinkyTimer = {0}; // TIM2  Timer para led de estado
Timer_Handler_t segTimer 	= {0}; // 		Timer para control de segmentos
EXTI_Config_t extiClk	 	= {0}; //
EXTI_Config_t extiSw 		= {0}; //

enum{
	silencio = 0,
	volumen_medio,
	volumen_alto,
	volumen_muy_alto
};
uint8_t estado = 0;
uint8_t btnSw = 0;
uint8_t data = 0;
uint8_t clock = 0;
uint8_t counterLeft = 0;
uint8_t counterRight = 0;

void init_System(void);
/*
 * The main function, where everything happens.
 */
int main (void){

	init_System();

	while(1){
		if(data == 0){

			if(estado == 3){
				estado = 0;
			}else{
				estado++;
			}
		}
		else{

			if(estado == 0){
			estado = 3;
			}
			else{
				estado--;
			}
		}


		switch(estado){
		case silencio:{
			gpio_WritePin(&ledBlue, SET);
			gpio_WritePin(&ledRed, RESET);
			gpio_WritePin(&ledGreen, RESET);
			for (uint32_t i = 0; i <  1600000; i++) {
			    }
			break;
		}
		case volumen_medio:{
			gpio_WritePin(&ledBlue, RESET);
			gpio_WritePin(&ledRed, SET);
			gpio_WritePin(&ledGreen, RESET);
			for (uint32_t i = 0; i <  1600000; i++) {
			   }
			break;
		}
		case volumen_alto:{
			gpio_WritePin(&ledBlue, RESET);
			gpio_WritePin(&ledRed, RESET);
			gpio_WritePin(&ledGreen, SET);
			for (uint32_t i = 0; i <  1600000; i++) {
			    }
			break;
		}
		case volumen_muy_alto:{
			gpio_WritePin(&ledBlue, SET);
			gpio_WritePin(&ledRed, SET);
			gpio_WritePin(&ledGreen, SET);
			for (uint32_t i = 0; i <  1600000; i++) {
			    }
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
	userLed.pGPIOx							= GPIOB;
	userLed.pinConfig.GPIO_PinNumber		= PIN_14;
	userLed.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	userLed.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	userLed.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	userLed.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&userLed);


	//Configuramos el pin
	ledRed.pGPIOx							= GPIOC;
	ledRed.pinConfig.GPIO_PinNumber			= PIN_9;
	ledRed.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	ledRed.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	ledRed.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	ledRed.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&ledRed);


	//Configuramos el pin
	ledGreen.pGPIOx							= GPIOB;
	ledGreen.pinConfig.GPIO_PinNumber		= PIN_8;
	ledGreen.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	ledGreen.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	ledGreen.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	ledGreen.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&ledGreen);


	//Configuramos el pin
	ledBlue.pGPIOx							= GPIOC;
	ledBlue.pinConfig.GPIO_PinNumber		= PIN_8;
	ledBlue.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	ledBlue.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	ledBlue.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	ledBlue.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&ledBlue);

	/////////////////************DIGITOS*******////////////////

	//Configuramos el pin
	digito1.pGPIOx							= GPIOC;
	digito1.pinConfig.GPIO_PinNumber		= PIN_5;
	digito1.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	digito1.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	digito1.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	digito1.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&digito1);

	//Configuramos el pin
	digito2.pGPIOx							= GPIOC;
	digito2.pinConfig.GPIO_PinNumber		= PIN_6;
	digito2.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	digito2.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	digito2.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	digito2.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&digito2);

	digito3.pGPIOx							= GPIOC;
	digito3.pinConfig.GPIO_PinNumber		= PIN_10;
	digito3.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	digito3.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	digito3.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	digito3.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&digito3);

	//Configuramos el pin
	digito4.pGPIOx							= GPIOD;
	digito4.pinConfig.GPIO_PinNumber		= PIN_2;
	digito4.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	digito4.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	digito4.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	digito4.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&digito4);

	/////////////////************SIETE SEGMENTOS********////////////////
	//Configuramos el pin
	segA.pGPIOx							= GPIOB;
	segA.pinConfig.GPIO_PinNumber		= PIN_12;
	segA.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segA.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segA.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segA.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&segA);

	//Configuramos el pin
	segB.pGPIOx							= GPIOA;
	segB.pinConfig.GPIO_PinNumber		= PIN_12;
	segB.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segB.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segB.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segB.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&segB);

	//Configuramos el pin
	segC.pGPIOx							= GPIOC;
	segC.pinConfig.GPIO_PinNumber		= PIN_13;
	segC.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segC.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segC.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segC.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&segC);

	//Configuramos el pin
	segD.pGPIOx							= GPIOC;
	segD.pinConfig.GPIO_PinNumber		= PIN_12;
	segD.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segD.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segD.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segD.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&segD);

	//Configuramos el pin
	segE.pGPIOx							= GPIOC;
	segE.pinConfig.GPIO_PinNumber		= PIN_11;
	segE.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segE.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segE.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segE.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&segE);

	//Configuramos el pin
	segF.pGPIOx							= GPIOA;
	segF.pinConfig.GPIO_PinNumber		= PIN_11;
	segF.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segF.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segF.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segF.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&segF);

	//Configuramos el pin
	segG.pGPIOx							= GPIOB;
	segG.pinConfig.GPIO_PinNumber		= PIN_7;
	segG.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segG.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segG.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segG.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&segG);

	//Configuramos el pin
	userSw.pGPIOx							= GPIOB;
	userSw.pinConfig.GPIO_PinNumber			= PIN_15;
	userSw.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	userSw.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	//Cargamos la configuracion  en los registros que gobiernan el puerto.
	gpio_Config(&userSw);

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

	// control del led de estado
	blinkyTimer.pTIMx								= TIM1;
	blinkyTimer.TIMx_Config.TIMx_Prescaler  		= 16000; //1ms conversion
	blinkyTimer.TIMx_Config.TIMx_Period				= 250;
	blinkyTimer.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;
	blinkyTimer.TIMx_Config.TIMx_InterruptEnable 	= TIMER_INT_ENABLE;
	timer_Config(&blinkyTimer);			     	//CARGAMOS CONFIGURACIÓN TIMER
	timer_SetState(&blinkyTimer, TIMER_ON);     //ENCENDEMOS EL TIMER

	// control de siete segmentos
	segTimer.pTIMx								= TIM2;
	segTimer.TIMx_Config.TIMx_Prescaler  		= 16000; //1ms conversion
	segTimer.TIMx_Config.TIMx_Period			= 8;
	segTimer.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;
	segTimer.TIMx_Config.TIMx_InterruptEnable 	= TIMER_INT_ENABLE;
	timer_Config(&segTimer);			     	//CARGAMOS CONFIGURACIÓN TIMER
	timer_SetState(&segTimer, TIMER_ON);     //ENCENDEMOS EL TIMER

	// RISING EDGE para el clock, se carga la config del exti para el clk
	exti.pGPIOHandler		= &userClk;
	extiClk.edgeType		= EXTERNAL_INTERRUPT_RISING_EDGE;
	exti_Config(&extiClk);

	// RISING EDGE para el boton
	extiSw.pGPIOHandler		= &userSw;
	extiSw.edgeType			= EXTERNAL_INTERRUPT_FALLING_EDGE;
	exti_Config(&extiSw);


	gpio_WritePin(&userLed, SET);
}


void Timer1_Callback(void){
	gpio_TooglePin(&userLed);
}
void Timer2_Callback(void){
	//Segmentos
}


// Callback para boton Sw del encoder
void callback_ExtInt8(void){
}


// Callback para giros del encoder
void callback_ExtInt9(void){
	data = gpio_ReadPin(&userData);
	clock = gpio_ReadPin(&userClk);
	// dato que se está dando la interrupción por flanco de subida
	// clock siempre será 1 al momento de realizar la interrupción
	// NOTA: si se utiliza el schmit trigger, se debe intercambiar
	// los valores, pues esta es una compuerta negadora.
	if (clock==1){
		if(data==0){
				counterRight++;
		}
		else if(data==1){
				counterLeft++;
		}
	}
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
