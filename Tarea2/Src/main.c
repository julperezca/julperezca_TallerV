/**
 ******************************************************************************
 * @file           : main.c- Tarea 2
 * @author         : Julián Pérez Carvajal (julperezca@unal.edu.co)
 * @brief          :
 ******************************************************************************
 */
#include <stdint.h>
#include "stm32f4xx.h"
#include "gpio_driver_hal.h"
#include "stm32_assert.h"
#include "exti_driver_hal.h"
#include "timer_driver_hal.h"
//Definimos un PIN de prueba
// LED de estado
GPIO_Handler_t userLed	 	= {0}; // PinB14

// LED RGB
GPIO_Handler_t ledRed 		= {0}; // PinC9
GPIO_Handler_t ledGreen 	= {0}; // PinB8
GPIO_Handler_t ledBlue	 	= {0}; // PinC8

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
GPIO_Handler_t userClk 		= {0}; // PinB2
GPIO_Handler_t userData 	= {0}; // PinB1
GPIO_Handler_t userSw 		= {0}; // PinB15

Timer_Handler_t blinkyTimer	    = {0}; // TIM2  Timer para led de estado
Timer_Handler_t segmentsTimer	= {0}; // TIM3  Timer para control de segmentos
EXTI_Config_t extiClk	 		= {0}; // EXTI2
EXTI_Config_t extiSw 			= {0}; // EXTI15


uint8_t number = 0;
uint8_t digit = 0;
uint8_t data = 0;
uint8_t clock = 0;
uint8_t counterLeft = 0;
uint8_t counterRight = 0;
uint8_t counterSw = 0;

enum {
	ON = 0,
	OFF
};
void init_System(void);
void numberSelection(uint8_t number);
void digitSelection(uint8_t digit);
/*
 * The main function, where everything happens.
 */
int main (void){

	init_System();
	numberSelection(number);
	while(1){
		data = gpio_ReadPin(&userData);
		clock = gpio_ReadPin(&userClk);
		digitSelection(digit);

		digit++;
		for (uint32_t i = 0; i <  1600000; i++){
		}
		if (digit==5){
			digit = 0;
		}
	}
}


void init_System(void){
	//Se configura el pin y cargamos configuracion de registros que manejan el puerto
	userLed.pGPIOx							= GPIOB;
	userLed.pinConfig.GPIO_PinNumber		= PIN_14;
	userLed.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	userLed.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	userLed.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	userLed.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&userLed);


	//Se configura el pin y cargamos configuracion de registros que manejan el puerto
	ledGreen.pGPIOx							= GPIOC;
	ledGreen.pinConfig.GPIO_PinNumber		= PIN_9;
	ledGreen.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	ledGreen.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	ledGreen.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	ledGreen.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&ledGreen);

	//Se configura el pin y cargamos configuracion de registros que manejan el puerto
	ledRed.pGPIOx							= GPIOB;
	ledRed.pinConfig.GPIO_PinNumber			= PIN_8;
	ledRed.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	ledRed.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	ledRed.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	ledRed.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&ledRed);

	//Se configura el pin y cargamos configuracion de registros que manejan el puerto
	ledBlue.pGPIOx							= GPIOC;
	ledBlue.pinConfig.GPIO_PinNumber		= PIN_8;
	ledBlue.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	ledBlue.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	ledBlue.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	ledBlue.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&ledBlue);

	/////////////////************DIGITOS*******////////////////
	//Se configura el pin y cargamos configuracion de registros que manejan el puerto
	//Se configura el pin y cargamos configuracion de registros que manejan el puerto
	digito1.pGPIOx							= GPIOC;
	digito1.pinConfig.GPIO_PinNumber		= PIN_10;
	digito1.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	digito1.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	digito1.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	digito1.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&digito1);

	//Se configura el pin y cargamos configuracion de registros que manejan el puerto
	digito2.pGPIOx							= GPIOB;
	digito2.pinConfig.GPIO_PinNumber		= PIN_9;
	digito2.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	digito2.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	digito2.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	digito2.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&digito2);

	//Se configura el pin y cargamos configuracion de registros que manejan el puerto
	digito3.pGPIOx							= GPIOC;
	digito3.pinConfig.GPIO_PinNumber		= PIN_6;
	digito3.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	digito3.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	digito3.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	digito3.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&digito3);

	//Se configura el pin y cargamos configuracion de registros que manejan el puerto
	digito4.pGPIOx							= GPIOC;
	digito4.pinConfig.GPIO_PinNumber		= PIN_5;
	digito4.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	digito4.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	digito4.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	digito4.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&digito4);

	/////////////////************SIETE SEGMENTOS********////////////////
	//Se configura el pin y cargamos configuracion de registros que manejan el puerto
	segA.pGPIOx							= GPIOB;
	segA.pinConfig.GPIO_PinNumber		= PIN_12;
	segA.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segA.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segA.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segA.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&segA);

	//Se configura el pin y cargamos configuracion de registros que manejan el puerto
	segB.pGPIOx							= GPIOA;
	segB.pinConfig.GPIO_PinNumber		= PIN_12;
	segB.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segB.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segB.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segB.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&segB);

	//Se configura el pin y cargamos configuracion de registros que manejan el puerto
	segC.pGPIOx							= GPIOC;
	segC.pinConfig.GPIO_PinNumber		= PIN_13;
	segC.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segC.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segC.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segC.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&segC);

	//Se configura el pin y cargamos configuracion de registros que manejan el puerto
	segD.pGPIOx							= GPIOD;
	segD.pinConfig.GPIO_PinNumber		= PIN_2;
	segD.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segD.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segD.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segD.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&segD);

	//Se configura el pin y cargamos configuracion de registros que manejan el puerto
	segE.pGPIOx							= GPIOC;
	segE.pinConfig.GPIO_PinNumber		= PIN_11;
	segE.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segE.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segE.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segE.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&segE);

	//Se configura  el pin y cargamos configuracion de registros que manejan el puerto
	segF.pGPIOx							= GPIOA;
	segF.pinConfig.GPIO_PinNumber		= PIN_11;
	segF.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segF.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segF.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segF.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&segF);

	//Se configura  el pin y cargamos configuracion de registros que manejan el puerto
	segG.pGPIOx							= GPIOB;
	segG.pinConfig.GPIO_PinNumber		= PIN_7;
	segG.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segG.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segG.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segG.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&segG);


	//Se configura  el pin y cargamos configuracion de registros que manejan el puerto
	userClk.pGPIOx							= GPIOB;
	userClk.pinConfig.GPIO_PinNumber		= PIN_2;
	userClk.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	userClk.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&userClk);


	//Se configura  el pin y cargamos configuracion de registros que manejan el puerto
	userData.pGPIOx							= GPIOB;
	userData.pinConfig.GPIO_PinNumber		= PIN_1;
	userData.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	userData.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&userData);

	//Se configura el pin y cargamos configuracion de registros que manejan el puerto
	userSw.pGPIOx							= GPIOB;
	userSw.pinConfig.GPIO_PinNumber			= PIN_15;
	userSw.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	userSw.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&userSw);


	//Se configura el exti para el Clk del encoder con detección de flanco de subida
	extiClk.pGPIOHandler			= &userClk;
	extiClk.edgeType				= EXTERNAL_INTERRUPT_RISING_EDGE;
	exti_Config(&extiClk);

	//Se configura el exti para el Switch boton del encoder con detección de flanco de subida
//	extiSw.pGPIOHandler				= &userSw;
//	extiSw.edgeType					= EXTERNAL_INTERRUPT_RISING_EDGE;
//	exti_Config(&extiSw);

	/* Se configura el timer del led de estado */
	blinkyTimer.pTIMx								= TIM2;
	blinkyTimer.TIMx_Config.TIMx_Prescaler  		= 16000; //1ms conversion
	blinkyTimer.TIMx_Config.TIMx_Period				= 250;
	blinkyTimer.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;
	blinkyTimer.TIMx_Config.TIMx_InterruptEnable 	= TIMER_INT_ENABLE;
	timer_Config(&blinkyTimer);
	timer_SetState(&blinkyTimer, TIMER_ON);     //ENCENDEMOS EL TIMER

	/*Se configura el timer de los digitos */
	segmentsTimer.pTIMx								= TIM3;
	segmentsTimer.TIMx_Config.TIMx_Prescaler  		= 16000; //1ms conversion
	segmentsTimer.TIMx_Config.TIMx_Period				= 250;
	segmentsTimer.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;
	segmentsTimer.TIMx_Config.TIMx_InterruptEnable 	= TIMER_INT_ENABLE;
	timer_Config(&segmentsTimer);
	timer_SetState(&segmentsTimer, TIMER_ON);     //ENCENDEMOS EL TIMER


	//Inicialmente el pin de estado está encendido
	gpio_WritePin(&userLed, SET);

	// Inicia con los digitos apagados
	gpio_WritePin(&digito1, OFF);
	gpio_WritePin(&digito2, OFF);
	gpio_WritePin(&digito3, OFF);
	gpio_WritePin(&digito4, OFF);
}

// Callback del blinkytimer con funcion que alterna el estado del userLed
void Timer2_Callback(void){
	gpio_TooglePin(&userLed);
}

// Callback para la interrupcion del pin B2 que corresponde al Clk
void callback_ExtInt2(void){
	data = gpio_ReadPin(&userData);
	clock = gpio_ReadPin(&userClk);
	// Schmitd trigger niega el valor del Clk y DT del encoder.

	if (clock==1){
		if(data==0){
				counterRight++;
		}
		else if(data==1){
				counterLeft++;
		}
	}
}

/* Callback para la interrupcion del Switch del encoder que controla el Led RGB */
void callback_ExtInt15(void){
	switch (counterSw){
	// Rojo
	case 0:{
		gpio_WritePin(&ledRed,SET);
		gpio_WritePin(&ledGreen,RESET);
		gpio_WritePin(&ledBlue,RESET);
		counterSw++;
		break;
	}
	// Verde
	case 1:{
		gpio_WritePin(&ledRed,RESET);
		gpio_WritePin(&ledGreen,SET);
		gpio_WritePin(&ledBlue,RESET);
		counterSw++;
		break;
	}
	// Azul
	case 2:{
		gpio_WritePin(&ledRed,RESET);
		gpio_WritePin(&ledGreen,RESET);
		gpio_WritePin(&ledBlue,SET);
		counterSw++;
		break;
	}
	// Azul + verde
	case 3:{
		gpio_WritePin(&ledRed,RESET);
		gpio_WritePin(&ledGreen,SET);
		gpio_WritePin(&ledBlue,SET);
		counterSw++;
		break;
	}
	// Azul + rojo
	case 4:{
		gpio_WritePin(&ledRed,SET);
		gpio_WritePin(&ledGreen,RESET);
		gpio_WritePin(&ledBlue,SET);
		counterSw++;
		break;
	}
	// Rojo + verde
	case 5:{
		gpio_WritePin(&ledRed,SET);
		gpio_WritePin(&ledGreen,SET);
		gpio_WritePin(&ledBlue,RESET);
		counterSw++;
		break;
	}
	// Rojo + verde + azul = blanco
	case 6:{
		gpio_WritePin(&ledRed,SET);
		gpio_WritePin(&ledGreen,SET);
		gpio_WritePin(&ledBlue,SET);
		counterSw++;
		break;
	}
	// Apagado
	case 7:{
		gpio_WritePin(&ledRed,RESET);
		gpio_WritePin(&ledGreen,RESET);
		gpio_WritePin(&ledBlue,RESET);
		counterSw = 0;
		break;
	}
	default:{
		break;
	}
	}
}

void digitSelection(uint8_t digit){
	switch (digit) {
		case 1:{
			gpio_WritePin(&digito1, ON);
			gpio_WritePin(&digito2, OFF);
			gpio_WritePin(&digito3, OFF);
			gpio_WritePin(&digito4, OFF);
			break;
			}
		case 2:{
			gpio_WritePin(&digito1, OFF);
			gpio_WritePin(&digito2, ON);
			gpio_WritePin(&digito3, OFF);
			gpio_WritePin(&digito4, OFF);
			break;
			}
		case 3:{
			gpio_WritePin(&digito1, OFF);
			gpio_WritePin(&digito2, OFF);
			gpio_WritePin(&digito3, ON);
			gpio_WritePin(&digito4, OFF);
			break;
			}
		case 4:{
			gpio_WritePin(&digito1, OFF);
			gpio_WritePin(&digito2, OFF);
			gpio_WritePin(&digito3, OFF);
			gpio_WritePin(&digito4, ON);
			break;
			}
	}
}
// selección de numero que es mostrado en el display
void numberSelection(uint8_t number){
	switch (number) {
		case 0:{
			gpio_WritePin(&segA, ON);
			gpio_WritePin(&segB, ON);
			gpio_WritePin(&segC, ON);
			gpio_WritePin(&segD, ON);
			gpio_WritePin(&segE, ON);
			gpio_WritePin(&segF, ON);
			gpio_WritePin(&segG, OFF);
			break;
			}
		case 1:{
			gpio_WritePin(&segA, OFF);
			gpio_WritePin(&segB, ON);
			gpio_WritePin(&segC, ON);
			gpio_WritePin(&segD, OFF);
			gpio_WritePin(&segE, OFF);
			gpio_WritePin(&segF, OFF);
			gpio_WritePin(&segG, OFF);
			break;
			}
		case 2:{
			gpio_WritePin(&segA, ON);
			gpio_WritePin(&segB, ON);
			gpio_WritePin(&segC, OFF);
			gpio_WritePin(&segD, ON);
			gpio_WritePin(&segE, ON);
			gpio_WritePin(&segF, OFF);
			gpio_WritePin(&segG, ON);
			break;
			}
		case 3:{
			gpio_WritePin(&segA, ON);
			gpio_WritePin(&segB, ON);
			gpio_WritePin(&segC, ON);
			gpio_WritePin(&segD, ON);
			gpio_WritePin(&segE, OFF);
			gpio_WritePin(&segF, OFF);
			gpio_WritePin(&segG, ON);
			break;
			}
		case 4:{
			gpio_WritePin(&segA, OFF);
			gpio_WritePin(&segB, ON);
			gpio_WritePin(&segC, ON);
			gpio_WritePin(&segD, OFF);
			gpio_WritePin(&segE, OFF);
			gpio_WritePin(&segF, ON);
			gpio_WritePin(&segG, ON);
			break;
			}
		case 5:{
			gpio_WritePin(&segA, ON);
			gpio_WritePin(&segB, OFF);
			gpio_WritePin(&segC, ON);
			gpio_WritePin(&segD, ON);
			gpio_WritePin(&segE, OFF);
			gpio_WritePin(&segF, ON);
			gpio_WritePin(&segG, ON);
			break;
			}
		case 6:{
			gpio_WritePin(&segA, ON);
			gpio_WritePin(&segB, OFF);
			gpio_WritePin(&segC, ON);
			gpio_WritePin(&segD, ON);
			gpio_WritePin(&segE, ON);
			gpio_WritePin(&segF, ON);
			gpio_WritePin(&segG, ON);
			break;
			}
		case 7:{
			gpio_WritePin(&segA, ON);
			gpio_WritePin(&segB, ON);
			gpio_WritePin(&segC, ON);
			gpio_WritePin(&segD, OFF);
			gpio_WritePin(&segE, OFF);
			gpio_WritePin(&segF, OFF);
			gpio_WritePin(&segG, OFF);
			break;
			}
		case 8:{
			gpio_WritePin(&segA, ON);
			gpio_WritePin(&segB, ON);
			gpio_WritePin(&segC, ON);
			gpio_WritePin(&segD, ON);
			gpio_WritePin(&segE, ON);
			gpio_WritePin(&segF, ON);
			gpio_WritePin(&segG, ON);
			break;
			}
		case 9:{
			gpio_WritePin(&segA, ON);
			gpio_WritePin(&segB, ON);
			gpio_WritePin(&segC, ON);
			gpio_WritePin(&segD, ON);
			gpio_WritePin(&segE, OFF);
			gpio_WritePin(&segF, ON);
			gpio_WritePin(&segG, ON);
			break;
			}

		default:{
			break;
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
