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



uint8_t digit = 1;
uint8_t data = 0;
uint8_t clock = 0;
uint8_t counterSw = 0;
int16_t counterRight = 0;
uint8_t rgbFlag = 0;
uint8_t digitFlag = 0;
uint8_t rotationFlag = 0;



/* Estado de los transistores y segmentos */
enum {
	ON = 0,
	OFF
};
/*  */
void rgbModeSelection(void);
void init_System(void);
void numberSelection(uint8_t number);
void digitSelection(uint8_t digit);
void dirOfRotation(void);
/*
 * The main function, where everything happens.
 */
int main (void){

	init_System();

	while(1){


		if (rgbFlag){
			rgbFlag = 0;
			rgbModeSelection();
		}

		if (digitFlag){
			numberSelection(10);
			digitSelection(5);
			digitFlag = 0;
			if (rotationFlag){
				rotationFlag = 0;
				dirOfRotation();
			}
			switch (digit){
			case 1:{
				numberSelection(counterRight%10); // Unidades del valor mostrado
			break;
			}
			case 2:{
				numberSelection((counterRight/10)%10); // Decenas del valor mostrado
			break;
			}
			case 3:{
				numberSelection((counterRight/100)%10); // Centenas del valor mostrado
			break;
			}
			case 4:{
				numberSelection((counterRight/1000)%10); // Un millar del valor mostrado
			break;
			}
			default:
				break;
			}

			digitSelection(digit);  // Se enciende el digito con el numero seleccionado
			digit++;
			if (digit == 5){
				digit = 1;
			}
		}
    }
}
void init_System(void){

			/* Configuración de LED de estado y su respectivo timer */

	// GPIO config para Led de estado
	userLed.pGPIOx							= GPIOB;
	userLed.pinConfig.GPIO_PinNumber		= PIN_14;
	userLed.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	userLed.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	userLed.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	userLed.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&userLed);

	// Config para el timer del led de estado
	blinkyTimer.pTIMx								= TIM2;
	blinkyTimer.TIMx_Config.TIMx_Prescaler  		= 16000; //1ms conversion
	blinkyTimer.TIMx_Config.TIMx_Period				= 250;
	blinkyTimer.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;
	blinkyTimer.TIMx_Config.TIMx_InterruptEnable 	= TIMER_INT_ENABLE;
	timer_Config(&blinkyTimer);
	timer_SetState(&blinkyTimer, TIMER_ON);

	/* FIN de configuración de Led de estado y timer */



			/* Se configuran los pines para el led RGB */

	//Led rojo
	ledRed.pGPIOx							= GPIOB;
	ledRed.pinConfig.GPIO_PinNumber			= PIN_8;
	ledRed.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	ledRed.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	ledRed.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	ledRed.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&ledRed);

	//Led verde
	ledGreen.pGPIOx							= GPIOC;
	ledGreen.pinConfig.GPIO_PinNumber		= PIN_9;
	ledGreen.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	ledGreen.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	ledGreen.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	ledGreen.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&ledGreen);

	//Led azul
	ledBlue.pGPIOx							= GPIOC;
	ledBlue.pinConfig.GPIO_PinNumber		= PIN_8;
	ledBlue.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	ledBlue.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	ledBlue.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	ledBlue.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&ledBlue);

	/* FIN de la config de Led RGB */


			/* Se configura GPIO y Timer para los transistores  */

	// Transistor que maneja el digito de las unidades
	digito1.pGPIOx							= GPIOC;
	digito1.pinConfig.GPIO_PinNumber		= PIN_10;
	digito1.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	digito1.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	digito1.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	digito1.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&digito1);

	// Transistor que maneja el digito de las decenas
	digito2.pGPIOx							= GPIOB;
	digito2.pinConfig.GPIO_PinNumber		= PIN_9;
	digito2.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	digito2.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	digito2.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	digito2.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&digito2);

	// Transistor que maneja el digito de las centenas
	digito3.pGPIOx							= GPIOC;
	digito3.pinConfig.GPIO_PinNumber		= PIN_6;
	digito3.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	digito3.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	digito3.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	digito3.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&digito3);

	// Transistor que maneja el digito de un millar
	digito4.pGPIOx							= GPIOC;
	digito4.pinConfig.GPIO_PinNumber		= PIN_5;
	digito4.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	digito4.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	digito4.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	digito4.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&digito4);

	/*Se configura el timer de los digitos */
	segmentsTimer.pTIMx								= TIM3;
	segmentsTimer.TIMx_Config.TIMx_Prescaler  		= 16000; //1ms conversion
	segmentsTimer.TIMx_Config.TIMx_Period			= 2;
	segmentsTimer.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;
	segmentsTimer.TIMx_Config.TIMx_InterruptEnable 	= TIMER_INT_ENABLE;
	timer_Config(&segmentsTimer);
	timer_SetState(&segmentsTimer, TIMER_ON);

	/* FIN de configuración de transistores y timer */


		/* Se configuran los pines que manejan los siete segmentos */

	//Segmento a
	segA.pGPIOx							= GPIOB;
	segA.pinConfig.GPIO_PinNumber		= PIN_12;
	segA.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segA.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segA.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segA.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&segA);

	//Segmento b
	segB.pGPIOx							= GPIOA;
	segB.pinConfig.GPIO_PinNumber		= PIN_12;
	segB.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segB.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segB.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segB.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&segB);

	//Segmento c
	segC.pGPIOx							= GPIOC;
	segC.pinConfig.GPIO_PinNumber		= PIN_13;
	segC.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segC.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segC.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segC.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&segC);

	//Segmento d
	segD.pGPIOx							= GPIOD;
	segD.pinConfig.GPIO_PinNumber		= PIN_2;
	segD.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segD.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segD.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segD.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&segD);

	//Segmento e
	segE.pGPIOx							= GPIOC;
	segE.pinConfig.GPIO_PinNumber		= PIN_11;
	segE.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segE.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segE.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segE.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&segE);

	//Segmento f
	segF.pGPIOx							= GPIOA;
	segF.pinConfig.GPIO_PinNumber		= PIN_11;
	segF.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segF.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segF.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segF.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&segF);

	//Segmento g
	segG.pGPIOx							= GPIOB;
	segG.pinConfig.GPIO_PinNumber		= PIN_7;
	segG.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segG.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segG.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segG.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&segG);

		/* Se configura GPIO con su EXTI excepto para el userData*/

	// GPIO mode in para el CLK
	userClk.pGPIOx							= GPIOB;
	userClk.pinConfig.GPIO_PinNumber		= PIN_2;
	userClk.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	userClk.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&userClk);
	//configuración EXTI para el CLK
	extiClk.pGPIOHandler			= &userClk;
	extiClk.edgeType				= EXTERNAL_INTERRUPT_RISING_EDGE;
	exti_Config(&extiClk);

	userData.pGPIOx							= GPIOB;
	userData.pinConfig.GPIO_PinNumber		= PIN_1;
	userData.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	userData.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&userData);

	// GPIO mode in para el Switch button del encoder
	userSw.pGPIOx							= GPIOB;
	userSw.pinConfig.GPIO_PinNumber			= PIN_15;
	userSw.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	userSw.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&userSw);
	//Configuración del Exti para el SW
	extiSw.pGPIOHandler				= &userSw;
	extiSw.edgeType					= EXTERNAL_INTERRUPT_RISING_EDGE;
	exti_Config(&extiSw);

	/* FIN de GPIO and EXTI config */


	//Inicialmente el pin de estado está encendido
	gpio_WritePin(&userLed, SET);

	// Inicia con los digitos apagados
	gpio_WritePin(&digito1, OFF);
	gpio_WritePin(&digito2, OFF);
	gpio_WritePin(&digito3, OFF);
	gpio_WritePin(&digito4, OFF);
}

/* Callback del blinkytimer con funcion que alterna el estado del userLed */
void Timer2_Callback(void){
	gpio_TooglePin(&userLed);
}
/* Callback del timer que enciende y apaga los transistores */
void Timer3_Callback(void){
	digitFlag = 1;
}

/* Callback para la interrupcion del pin B2 que corresponde al Clk */
void callback_ExtInt2(void){
	rotationFlag = 1;
	data = gpio_ReadPin(&userData);
	clock = gpio_ReadPin(&userClk);
	// Schmitd trigger niega el valor del CLK y DT del encoder.
}

/* Callback para la interrupcion del Switch del encoder que controla el Led RGB */
void callback_ExtInt15(void){
	rgbFlag = 1;
}

/* Funcion que determina el sentido de giro y aumenta o disminuye el valor */
void dirOfRotation(void){
	if(data == 0){
		counterRight++;
			if (counterRight == 4096){
				counterRight = 0;
			}
		}
	else if(data == 1){
		counterRight--;
		if (counterRight == -1){
			counterRight = 4095;
		}
	}
}



/* Funcion para  seleccionar los modos del Led RGB */
void rgbModeSelection(void){
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

/* Funcion para seleccionar el digito en uso */
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
		case 5:{
			gpio_WritePin(&digito1, OFF);
			gpio_WritePin(&digito2, OFF);
			gpio_WritePin(&digito3, OFF);
			gpio_WritePin(&digito4, OFF);
			break;
			}
	}
}

/* Funcion que selecciona los segmentos para asignar un numero en el display */
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
		case 10:{
			gpio_WritePin(&segA, OFF);
			gpio_WritePin(&segB, OFF);
			gpio_WritePin(&segC, OFF);
			gpio_WritePin(&segD, OFF);
			gpio_WritePin(&segE, OFF);
			gpio_WritePin(&segF, OFF);
			gpio_WritePin(&segG, OFF);
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
