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


	/* GPIO handler y TIMER para el led de estado */
GPIO_Handler_t ledState	 	= {0}; 		// PinB14
Timer_Handler_t blinkyTimer	 = {0}; 	// TIM2 Led de estado

	/* GPIO handler para led RGB*/
GPIO_Handler_t ledRed 		= {0}; 		// PinC9
GPIO_Handler_t ledGreen 	= {0}; 		// PinB8
GPIO_Handler_t ledBlue	 	= {0}; 		// PinC8

	/* GPIO handler para los siete segmentos */
GPIO_Handler_t segmentA 		= {0}; 		// PinB12 segmento a
GPIO_Handler_t segmentB  		= {0}; 		// PinA12 segmento b
GPIO_Handler_t segmentC 	 	= {0}; 		// PinC13 segmento c
GPIO_Handler_t segmentD  		= {0}; 		// PinC12 segmento d
GPIO_Handler_t segmentE 		= {0}; 		// PinC11 segmento e
GPIO_Handler_t segmentF 	 	= {0}; 		// PinA11 segmento f
GPIO_Handler_t segmentG 	 	= {0}; 		// PinB7  segmento g


	/* GPIO handler y TIMER para los transistores  */
GPIO_Handler_t digitoUnidad 	 	= {0}; 		// PinC5
GPIO_Handler_t digitoDecena			= {0}; 		// PinC6
GPIO_Handler_t digitoCentena 		= {0}; 		// PinC10
GPIO_Handler_t digitoUnMillar 		= {0}; 		// PinD2
Timer_Handler_t transistorsTimer	= {0}; 		// TIM3 para los transistores
	/* GPIO y EXTI para el CLK del encoder*/
GPIO_Handler_t userClock 			= {0}; 		// PinB2
EXTI_Config_t extiClock	 			= {0}; 		// EXTI2

	/* GPIO handler y EXTI para el SW del encoder*/
GPIO_Handler_t userSwitch 			= {0}; 		// PinB15
EXTI_Config_t extiSwitch 			= {0}; 		// EXTI15

	/* GPIO handler para el DT del encoder*/
GPIO_Handler_t userData 			= {0}; 		// PinB1

/* Variables globales */
uint8_t digit = 1;  			// Variable que indica qué transistor está activo
uint8_t data = 0;				// Variable que almacena el estado del DT del encoder
uint8_t clock = 0;				// Variable que almacena el estado el CLK del encoder
uint8_t counterSwitching = 0;	// Variable que almacena el valor del SW del encoder
int16_t rotationCounter = 0;	// Variable que es mostrada en el display (giros del encoder)
uint8_t rgbFlag = 0;			// Bandera de la interrupción del led RGB
uint8_t digitFlag = 0;			// Bandera del timer3 para cambiar de transistor activo
uint8_t rotationFlag = 0;		// Bandera  indicativa de una rotation del encoder
uint8_t ledStateFlag = 0;		// Bandera del led de estado



/* Estado de los transistores y segmentos */
enum {
	ON = 0,
	OFF
};

/*  */
extern void configMagic(void);  		// Función del magic para
void rgbModeSelection(void);			// Función que selecciona el modo del led RGB
void init_Config(void);					// Función que inicia la config. de los pines, timers y EXTI
void numberSelection(uint8_t number);	// Función que selecciona los segmentos encendidos
void digitSelection(uint8_t digit);		// Función que elige que transistor encender
void dirOfRotation(void); 				// Función encargada de el sentido de rotation y el valor de la misma


/*
 * The main function, where everything happens.
 */
int main (void){
	configMagic();  // Se inicia la configuracion de Magic
	init_Config();	// Se inicia la configuracion del sistema


	while(1){

		// Toogle del led de estado
		if (ledStateFlag){
			gpio_TooglePin(&ledState);
			ledStateFlag = 0;
		}
		// Modificación del modo del LED RGB si se alza la bandera
		if (rgbFlag){
			rgbModeSelection();
			rgbFlag = 0;
		}
		// Switching de los transistores
		if (digitFlag){
			numberSelection(10); 	//
			digitSelection(5);		// Se apagan los transistores
			digitFlag = 0;

			// Se alza la bandera de rotacoion y se determina el valor + sentido de giro
			if (rotationFlag){
				dirOfRotation();
				rotationFlag = 0;
			}

			// Casos donde se descompone el valor de rotationCounter y se asigna a cada transistor
			switch (digit){
			case 1:{
				numberSelection(rotationCounter%10); // Unidades del valor mostrado
			break;
			}
			case 2:{
				numberSelection((rotationCounter/10)%10); // Decenas del valor mostrado
			break;
			}
			case 3:{
				numberSelection((rotationCounter/100)%10); // Centenas del valor mostrado
			break;
			}
			case 4:{
				numberSelection((rotationCounter/1000)%10); // Un millar del valor mostrado
			break;
			}
			default:
				break;
			}

			digitSelection(digit);  // Se enciende el digito con el numero seleccionado
			digit++;

			// El desborde del digito lo reinicia
			if (digit == 5){
				digit = 1;
			}
		}
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
	ledState.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	ledState.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&ledState);

	// Config para el timer del led de estado
	blinkyTimer.pTIMx								= TIM2;
	blinkyTimer.TIMx_Config.TIMx_Prescaler  		= 16000; //1ms conversion
	blinkyTimer.TIMx_Config.TIMx_Period				= 100;
	blinkyTimer.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;
	blinkyTimer.TIMx_Config.TIMx_InterruptEnable 	= TIMER_INT_ENABLE;
	timer_Config(&blinkyTimer);
	timer_SetState(&blinkyTimer, TIMER_ON);

		/* FIN de configuración de Led de estado y su timer */



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
	digitoUnidad.pGPIOx								= GPIOC;
	digitoUnidad.pinConfig.GPIO_PinNumber			= PIN_10;
	digitoUnidad.pinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	digitoUnidad.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	digitoUnidad.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	digitoUnidad.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	gpio_Config(&digitoUnidad);

	// Transistor que maneja el digito de las decenas
	digitoDecena.pGPIOx								= GPIOB;
	digitoDecena.pinConfig.GPIO_PinNumber			= PIN_9;
	digitoDecena.pinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	digitoDecena.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	digitoDecena.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	digitoDecena.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	gpio_Config(&digitoDecena);

	// Transistor que maneja el digito de las centenas
	digitoCentena.pGPIOx							= GPIOC;
	digitoCentena.pinConfig.GPIO_PinNumber			= PIN_6;
	digitoCentena.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	digitoCentena.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	digitoCentena.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	digitoCentena.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	gpio_Config(&digitoCentena);

	// Transistor que maneja el digito de un millar
	digitoUnMillar.pGPIOx							= GPIOC;
	digitoUnMillar.pinConfig.GPIO_PinNumber			= PIN_5;
	digitoUnMillar.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	digitoUnMillar.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	digitoUnMillar.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	digitoUnMillar.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&digitoUnMillar);

	/*Se configura el timer de los digitos */
	transistorsTimer.pTIMx								= TIM3;
	transistorsTimer.TIMx_Config.TIMx_Prescaler  		= 16000; //1ms conversion
	transistorsTimer.TIMx_Config.TIMx_Period			= 4;
	transistorsTimer.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;
	transistorsTimer.TIMx_Config.TIMx_InterruptEnable 	= TIMER_INT_ENABLE;
	timer_Config(&transistorsTimer);
	timer_SetState(&transistorsTimer, TIMER_ON);

			/* FIN de configuración de transistores y su timer */


		/* Se configuran los pines que manejan los siete segmentos */

	//Segmento a
	segmentA.pGPIOx							= GPIOB;
	segmentA.pinConfig.GPIO_PinNumber		= PIN_12;
	segmentA.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segmentA.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segmentA.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segmentA.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&segmentA);

	//Segmento b
	segmentB.pGPIOx							= GPIOA;
	segmentB.pinConfig.GPIO_PinNumber		= PIN_12;
	segmentB.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segmentB.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segmentB.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segmentB.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&segmentB);

	//Segmento c
	segmentC.pGPIOx							= GPIOC;
	segmentC.pinConfig.GPIO_PinNumber		= PIN_13;
	segmentC.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segmentC.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segmentC.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segmentC.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&segmentC);

	//Segmento d
	segmentD.pGPIOx							= GPIOD;
	segmentD.pinConfig.GPIO_PinNumber		= PIN_2;
	segmentD.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segmentD.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segmentD.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segmentD.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&segmentD);

	//Segmento e
	segmentE.pGPIOx							= GPIOC;
	segmentE.pinConfig.GPIO_PinNumber		= PIN_11;
	segmentE.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segmentE.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segmentE.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segmentE.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&segmentE);

	//Segmento f
	segmentF.pGPIOx							= GPIOA;
	segmentF.pinConfig.GPIO_PinNumber		= PIN_11;
	segmentF.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segmentF.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segmentF.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segmentF.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&segmentF);

	//Segmento g
	segmentG.pGPIOx							= GPIOB;
	segmentG.pinConfig.GPIO_PinNumber		= PIN_7;
	segmentG.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segmentG.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	segmentG.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	segmentG.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&segmentG);

			/* FIN de la configuración de los segmentos */

		/* Se configura GPIO con su EXTI excepto para el userData*/

	// GPIO mode in para el CLK
	userClock.pGPIOx							= GPIOB;
	userClock.pinConfig.GPIO_PinNumber			= PIN_2;
	userClock.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	userClock.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	gpio_Config(&userClock);

	// Configuración EXTI para el CLK
	extiClock.pGPIOHandler						= &userClock;
	extiClock.edgeType							= EXTERNAL_INTERRUPT_RISING_EDGE;
	exti_Config(&extiClock);

	// GPIO mode in para el Switch button del encoder
	userSwitch.pGPIOx							= GPIOB;
	userSwitch.pinConfig.GPIO_PinNumber			= PIN_15;
	userSwitch.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	userSwitch.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&userSwitch);

	// Configuración del Exti para el SW
	extiSwitch.pGPIOHandler						= &userSwitch;
	extiSwitch.edgeType							= EXTERNAL_INTERRUPT_RISING_EDGE;
	exti_Config(&extiSwitch);

	// GPIO config para el DT del encoder
	userData.pGPIOx								= GPIOB;
	userData.pinConfig.GPIO_PinNumber			= PIN_1;
	userData.pinConfig.GPIO_PinMode				= GPIO_MODE_IN;
	userData.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	gpio_Config(&userData);

			/* FIN de GPIO and EXTI config */


	// Inicialmente el pin de estado está encendido
	gpio_WritePin(&ledState, SET);

	// Inicia con los digitos apagados
	gpio_WritePin(&digitoUnidad, OFF);
	gpio_WritePin(&digitoDecena, OFF);
	gpio_WritePin(&digitoCentena, OFF);
	gpio_WritePin(&digitoUnMillar, OFF);
}

/* Callback del blinkytimer alterna el estado del ledState */
void Timer2_Callback(void){
	ledStateFlag = 1;
}

/* Callback del timer que enciende y apaga los transistores */
void Timer3_Callback(void){
	digitFlag = 1;
}

/* Callback de la interrupcion del pin B2 que corresponde al Clk */
void callback_ExtInt2(void){
	rotationFlag = 1;
	data = gpio_ReadPin(&userData);
	clock = gpio_ReadPin(&userClock);
	// Schmitd trigger niega el valor del CLK y DT del encoder.
}

/* Callback de la interrupcion del Switch SW del encoder que controla el Led RGB */
void callback_ExtInt15(void){
	rgbFlag = 1;
}

/* Funcion que determina el sentido de giro y aumenta o disminuye el valor */
void dirOfRotation(void){
	// si data es cero, se gira en sentido CW y aumenta el valor del contador hasta 4095 y luego 0
	if(data == 0){
		printf("Giro CW\n");
		rotationCounter++;
			if (rotationCounter == 4096){
				rotationCounter = 0;
			}
		}
	// si data es uno, se gira en sentido CCW y disminuye el valor del contador hasta 0 y luego 4095
	else if(data == 1){
		printf("Giro CCW\n");
		rotationCounter--;
		if (rotationCounter == -1){
			rotationCounter = 4095;
		}
	}
}

/* Funcion para  seleccionar los modos del Led RGB */
void rgbModeSelection(void){
	// Cada que ingresa en un case se aumenta el valor del counter siguiendo la secuencia indicada
	switch (counterSwitching){
		// Rojo
		case 0:{
			gpio_WritePin(&ledRed,SET);
			gpio_WritePin(&ledGreen,RESET);
			gpio_WritePin(&ledBlue,RESET);
			counterSwitching++;
			break;
		}
		// Verde
		case 1:{
			gpio_WritePin(&ledRed,RESET);
			gpio_WritePin(&ledGreen,SET);
			gpio_WritePin(&ledBlue,RESET);
			counterSwitching++;
			break;
		}
		// Azul
		case 2:{
			gpio_WritePin(&ledRed,RESET);
			gpio_WritePin(&ledGreen,RESET);
			gpio_WritePin(&ledBlue,SET);
			counterSwitching++;
			break;
		}
		// Azul + verde
		case 3:{
			gpio_WritePin(&ledRed,RESET);
			gpio_WritePin(&ledGreen,SET);
			gpio_WritePin(&ledBlue,SET);
			counterSwitching++;
			break;
		}
		// Azul + rojo
		case 4:{
			gpio_WritePin(&ledRed,SET);
			gpio_WritePin(&ledGreen,RESET);
			gpio_WritePin(&ledBlue,SET);
			counterSwitching++;
			break;
		}
		// Rojo + verde
		case 5:{
			gpio_WritePin(&ledRed,SET);
			gpio_WritePin(&ledGreen,SET);
			gpio_WritePin(&ledBlue,RESET);
			counterSwitching++;
			break;
		}
		// Rojo + verde + azul = blanco
		case 6:{
			gpio_WritePin(&ledRed,SET);
			gpio_WritePin(&ledGreen,SET);
			gpio_WritePin(&ledBlue,SET);
			counterSwitching++;
			break;
		}
		// Apagado
		case 7:{
			gpio_WritePin(&ledRed,RESET);
			gpio_WritePin(&ledGreen,RESET);
			gpio_WritePin(&ledBlue,RESET);
			counterSwitching = 0;
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
			gpio_WritePin(&digitoUnidad, ON);
			gpio_WritePin(&digitoDecena, OFF);
			gpio_WritePin(&digitoCentena, OFF);
			gpio_WritePin(&digitoUnMillar, OFF);
			break;
			}
		case 2:{
			gpio_WritePin(&digitoUnidad, OFF);
			gpio_WritePin(&digitoDecena, ON);
			gpio_WritePin(&digitoCentena, OFF);
			gpio_WritePin(&digitoUnMillar, OFF);
			break;
			}
		case 3:{
			gpio_WritePin(&digitoUnidad, OFF);
			gpio_WritePin(&digitoDecena, OFF);
			gpio_WritePin(&digitoCentena, ON);
			gpio_WritePin(&digitoUnMillar, OFF);
			break;
			}
		case 4:{
			gpio_WritePin(&digitoUnidad, OFF);
			gpio_WritePin(&digitoDecena, OFF);
			gpio_WritePin(&digitoCentena, OFF);
			gpio_WritePin(&digitoUnMillar, ON);
			break;
			}
		case 5:{
			gpio_WritePin(&digitoUnidad, OFF);
			gpio_WritePin(&digitoDecena, OFF);
			gpio_WritePin(&digitoCentena, OFF);
			gpio_WritePin(&digitoUnMillar, OFF);
			break;
			}
	}
}

/* Funcion que selecciona los segmentos para asignar un numero en el display */
void numberSelection(uint8_t number){
	switch (number) {
		case 0:{
			gpio_WritePin(&segmentA, ON);
			gpio_WritePin(&segmentB, ON);
			gpio_WritePin(&segmentC, ON);
			gpio_WritePin(&segmentD, ON);
			gpio_WritePin(&segmentE, ON);
			gpio_WritePin(&segmentF, ON);
			gpio_WritePin(&segmentG, OFF);
			break;
			}
		case 1:{
			gpio_WritePin(&segmentA, OFF);
			gpio_WritePin(&segmentB, ON);
			gpio_WritePin(&segmentC, ON);
			gpio_WritePin(&segmentD, OFF);
			gpio_WritePin(&segmentE, OFF);
			gpio_WritePin(&segmentF, OFF);
			gpio_WritePin(&segmentG, OFF);
			break;
			}
		case 2:{
			gpio_WritePin(&segmentA, ON);
			gpio_WritePin(&segmentB, ON);
			gpio_WritePin(&segmentC, OFF);
			gpio_WritePin(&segmentD, ON);
			gpio_WritePin(&segmentE, ON);
			gpio_WritePin(&segmentF, OFF);
			gpio_WritePin(&segmentG, ON);
			break;
			}
		case 3:{
			gpio_WritePin(&segmentA, ON);
			gpio_WritePin(&segmentB, ON);
			gpio_WritePin(&segmentC, ON);
			gpio_WritePin(&segmentD, ON);
			gpio_WritePin(&segmentE, OFF);
			gpio_WritePin(&segmentF, OFF);
			gpio_WritePin(&segmentG, ON);
			break;
			}
		case 4:{
			gpio_WritePin(&segmentA, OFF);
			gpio_WritePin(&segmentB, ON);
			gpio_WritePin(&segmentC, ON);
			gpio_WritePin(&segmentD, OFF);
			gpio_WritePin(&segmentE, OFF);
			gpio_WritePin(&segmentF, ON);
			gpio_WritePin(&segmentG, ON);
			break;
			}
		case 5:{
			gpio_WritePin(&segmentA, ON);
			gpio_WritePin(&segmentB, OFF);
			gpio_WritePin(&segmentC, ON);
			gpio_WritePin(&segmentD, ON);
			gpio_WritePin(&segmentE, OFF);
			gpio_WritePin(&segmentF, ON);
			gpio_WritePin(&segmentG, ON);
			break;
			}
		case 6:{
			gpio_WritePin(&segmentA, ON);
			gpio_WritePin(&segmentB, OFF);
			gpio_WritePin(&segmentC, ON);
			gpio_WritePin(&segmentD, ON);
			gpio_WritePin(&segmentE, ON);
			gpio_WritePin(&segmentF, ON);
			gpio_WritePin(&segmentG, ON);
			break;
			}
		case 7:{
			gpio_WritePin(&segmentA, ON);
			gpio_WritePin(&segmentB, ON);
			gpio_WritePin(&segmentC, ON);
			gpio_WritePin(&segmentD, OFF);
			gpio_WritePin(&segmentE, OFF);
			gpio_WritePin(&segmentF, OFF);
			gpio_WritePin(&segmentG, OFF);
			break;
			}
		case 8:{
			gpio_WritePin(&segmentA, ON);
			gpio_WritePin(&segmentB, ON);
			gpio_WritePin(&segmentC, ON);
			gpio_WritePin(&segmentD, ON);
			gpio_WritePin(&segmentE, ON);
			gpio_WritePin(&segmentF, ON);
			gpio_WritePin(&segmentG, ON);
			break;
			}
		case 9:{
			gpio_WritePin(&segmentA, ON);
			gpio_WritePin(&segmentB, ON);
			gpio_WritePin(&segmentC, ON);
			gpio_WritePin(&segmentD, ON);
			gpio_WritePin(&segmentE, OFF);
			gpio_WritePin(&segmentF, ON);
			gpio_WritePin(&segmentG, ON);
			break;
			}
		case 10:{
			gpio_WritePin(&segmentA, OFF);
			gpio_WritePin(&segmentB, OFF);
			gpio_WritePin(&segmentC, OFF);
			gpio_WritePin(&segmentD, OFF);
			gpio_WritePin(&segmentE, OFF);
			gpio_WritePin(&segmentF, OFF);
			gpio_WritePin(&segmentG, OFF);
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
