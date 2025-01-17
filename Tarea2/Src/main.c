/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Julián Pérez Carvajal (julperezca@unal.edu.co)
 * @brief          : Tarea 2. Drivers GPIO, EXTI, TIMERS, magicProject.
 ******************************************************************************
 */
#include <main.h>
#include <stdio.h>
#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32_assert.h"
#include "exti_driver_hal.h"
#include "timer_driver_hal.h"
#include "systick_driver.h"



	/* GPIO handler y TIMER para el led de estado */
GPIO_Handler_t  ledState    	= {0}; 		// PinH1
Timer_Handler_t blinkyTimer		= {0}; 		// TIM2 Led de estado

	/* GPIO handler para led RGB*/
GPIO_Handler_t ledRed 			= {0}; 		// PinB8
GPIO_Handler_t ledGreen 		= {0}; 		// PinC9
GPIO_Handler_t ledBlue	 		= {0}; 		// PinC8

	/* GPIO handler para los siete segmentos */
GPIO_Handler_t segmentA 		= {0}; 		// PinB12 segmento a
GPIO_Handler_t segmentB  		= {0}; 		// PinA12 segmento b
GPIO_Handler_t segmentC 	 	= {0}; 		// PinC13 segmento c
GPIO_Handler_t segmentD  		= {0}; 		// PinD2  segmento d
GPIO_Handler_t segmentE 		= {0}; 		// PinC11 segmento e
GPIO_Handler_t segmentF 	 	= {0}; 		// PinA11 segmento f
GPIO_Handler_t segmentG 	 	= {0}; 		// PinB7  segmento g

	/* GPIO handler y TIMER para los transistores  */
GPIO_Handler_t digitoUnidad 	 	= {0}; 		// PinC10
GPIO_Handler_t digitoDecena			= {0}; 		// PinB9
GPIO_Handler_t digitoCentena 		= {0}; 		// PinC6
GPIO_Handler_t digitoUnMillar 		= {0}; 		// PinC5
Timer_Handler_t transistorsTimer	= {0}; 		// TIM3 para los transistores

	/* GPIO handler y EXTI config para el CLK del encoder*/
GPIO_Handler_t userClock 			= {0}; 		// PinB2
EXTI_Config_t extiClock	 			= {0}; 		// EXTI2

	/* GPIO handler y EXTI config para el SW del encoder*/
GPIO_Handler_t userSwitch 			= {0}; 		// PinB15
EXTI_Config_t extiSwitch 			= {0}; 		// EXTI15

	/* GPIO handler para el DT del encoder*/
GPIO_Handler_t userData 			= {0}; 		// PinB1

/* Finite State Machine + subestados del led RGB, de los transistores y de los segmentos */
fsm_t fsm = {0};
fsm_RGB_t fsm_RGB 					= {0};
fsm_transistor_t fsm_transistor 	= {0};
fsm_segments_t fsm_segments 		= {0};


/* Variables globales */
uint8_t data 			 = 0;		// Variable que almacena el estado del DT del encoder
uint8_t clock			 = 0;		// Variable que almacena el estado el CLK del encoder
uint16_t rotationCounter = 0;		// Variable que es mostrada en el display (giros del encoder)

uint8_t transistorFlag	 = 0;		// Flag para switching de transistores
uint8_t blinkyFlag 		 = 0;		// Flag para el parpadeo del led
uint8_t rotationFlag 	 = 0;		// Flag de rotación del encoder
uint8_t swFlag 			 = 0;		// Flag para el SW del encoder

/* Estado de los transistores y segmentos */
enum {
	ON = 0,
	OFF
};

/* Declaración o prototipo de funciones */
extern void configMagic(void);  				// Config del Magic para comunicación serial
void fsm_rgb_modeSelection(void);				// Función que selecciona el color del led RGB
void init_config(void);							// Función que inicia la config. de los pines, timers y EXTI
void numberSelection(uint8_t displayNumber);	// Función que selecciona los segmentos encendidos, ingresa el valor a mostrar
void fsm_rotation_handler(void); 				// Función encargada del sentido de rotation y el valor de la misma
void disableTransistors(void);					// Función encargada de apagar los transistores para evitar el "fantasma"
void fsm_display_handler(void);					// Función encargada de manejar el los transistores y cada segmento
void state_machine_action(void);
/*
 * The main function, where everything happens.
 */
int main (void){
	configMagic();  // Se inicia la configuracion de Magic
	init_config();	// Se inicia la configuracion del sistema

	/* Loop infinito */
	while(1){

		/* Condicional para el alza de la bandera del Led de estado */
		if (blinkyFlag){
			gpio_TooglePin(&ledState);		// Alterna estado del led
			blinkyFlag = 0;					// Se limpia la bandera del parpadeo del led
		}


		/* Condicional para el Switching de los transistores */
		if(transistorFlag){
			fsm.fsmState = DISPLAY_VALUE_STATE;					// Se define el estado para la fsm
			transistorFlag = 0;					     			// Se limpia la bandera
		}

		/* Condicional para el alza de la bandera debido al SW del encoder */
		else if (swFlag){
			fsm.fsmState = SW_BUTTON_STATE;						// Se define el estado para la fsm
			swFlag = 0;											// Se baja la bandera
		}
		else{
			fsm.fsmState = STANDBY_STATE;						// Estado de espera
		}

		/* Se aplica el estado actual a la fsm*/
		state_machine_action();


	}
	return 0;
}


/* Funcion encargada de la configuración del GPIO, TIMERS y EXTIs */
void init_config(void){

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
	blinkyTimer.TIMx_Config.TIMx_Period				= 200;
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
	transistorsTimer.TIMx_Config.TIMx_Period			= 2;
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


			/* Se configura el SysTick con la señal de reloj de 16 MHz	*/

	systickConfig();

			/* FIN del SysTick config*/


	// Inicialmente led de estado está encendido
	gpio_WritePin(&ledState, SET);

	// Inicia con los digitos apagados
	gpio_WritePin(&digitoUnidad, OFF);
	gpio_WritePin(&digitoDecena, OFF);
	gpio_WritePin(&digitoCentena, OFF);
	gpio_WritePin(&digitoUnMillar, OFF);

}

/* Callback del blinkytimer alterna el estado del ledState */
void Timer2_Callback(void){
	blinkyFlag = 1;				// Se sube la bandera al led de estado
}

/* Callback del timer que enciende y apaga los transistores */
void Timer3_Callback(void){
	transistorFlag = 1;				// Se sube la bandera del switcheo de transistores
}

/* Callback de la interrupcion del pin B2 que corresponde al Clk */
void callback_ExtInt2(void){
	rotationFlag = 1;				// Se sube la bandera de rotación

	// Se lee el valor del data y clock para determinar el giro en sentido CW o CCW
	data = gpio_ReadPin(&userData);
	clock = gpio_ReadPin(&userClock);
}

/* Callback de la interrupcion del Switch SW del encoder que controla el Led RGB */
void callback_ExtInt15(void){
	swFlag = 1;				// Se sube la bandera correspondiente al bit pos 4
}



/* Función de la Finite State Machine  */
void state_machine_action(void){

	uint32_t currentTime = 0; 		// variable local que almacena el tiempo desde que inicia el código

	/* Switch case que evalua cada uno los estados de la FSM */
	switch (fsm.fsmState){

	case SW_BUTTON_STATE:
		currentTime = ticksNumber(); 						// Se guarda el valor de los ticks
		printf("Current time: %lu ms\n", currentTime);		// Se imprime el tiempo de ejecución del programa
		fsm_rgb_modeSelection();							// Se cambia el estado del Led RGB
		break;

	case DISPLAY_VALUE_STATE:

		disableTransistors();		         	 // Se apagan los transistores

		/* Condicional para el alza de la bandera dada por el extiCLK */
		if (rotationFlag){
			fsm_rotation_handler();	 		 	 // Actualiza rotationCounter para mostrar en el display
			rotationFlag = 0;				     // Se baja la bandera
		}
		fsm_display_handler(); 			    	 // Función que enciende los segmentos y el transistor

		break;

	default:
		fsm.fsmState = STANDBY_STATE;			// Estado de espera
		break;
	}
}



/* Función que maneja los estados de cadad digito y segmento */
void fsm_display_handler(void){
    switch (fsm_transistor.transState) {
        case UNIT:
        	// Se selecciona el valor numerico de la unidad
        	// Se habilita el transistor y se apagan los otros digitos
            numberSelection(rotationCounter % 10);
        	gpio_WritePin(&digitoUnidad, ON);
			gpio_WritePin(&digitoDecena, OFF);
			gpio_WritePin(&digitoCentena, OFF);
			gpio_WritePin(&digitoUnMillar, OFF);

			fsm_transistor.transState = TENS;			// Se selecciona el siguiente estado

            break;
        case TENS:
        	// Se selecciona el valor numerico de la decena
        	// Se habilita el transistor y se apagan los otros digitos
            numberSelection((rotationCounter / 10) % 10);
			gpio_WritePin(&digitoUnidad, OFF);
			gpio_WritePin(&digitoDecena, ON);
			gpio_WritePin(&digitoCentena, OFF);
			gpio_WritePin(&digitoUnMillar, OFF);

			fsm_transistor.transState = ONE_HUNDRED;	// Se selecciona el siguiente estado

            break;
        case ONE_HUNDRED:
        	// Se selecciona el valor numerico de la centena
        	// Se habilita el transistor y se apagan los otros digitos
        	numberSelection((rotationCounter / 100) % 10);
			gpio_WritePin(&digitoUnidad, OFF);
			gpio_WritePin(&digitoDecena, OFF);
			gpio_WritePin(&digitoCentena, ON);
			gpio_WritePin(&digitoUnMillar, OFF);

			fsm_transistor.transState = ONE_THOUSAND;	// Se selecciona el siguiente estado
            break;
        case ONE_THOUSAND:
        	// Se selecciona el valor numerico del millar
        	// Se habilita el transistor y se apagan los otros digitos
            numberSelection((rotationCounter / 1000) % 10);
			gpio_WritePin(&digitoUnidad, OFF);
			gpio_WritePin(&digitoDecena, OFF);
			gpio_WritePin(&digitoCentena, OFF);
			gpio_WritePin(&digitoUnMillar, ON);

			fsm_transistor.transState = UNIT;			// Se selecciona el estado inicial
            break;
        default:
            break;
    }
}



/* Funcion que determina el sentido de giro y aumenta o disminuye el valor rotationCounter*/
void fsm_rotation_handler(void){
	// MAX_12_BITS  en operación bitwise & con rotationCounter.
	// Si, rotationCounter toma un valor mayor a 12bits, la operación lo devolverá a cero
	// Si rotationCounter = 0 y se gira CCW pasa -1 & MAX_12_BITS, que será cierta para los 12 bits

	switch (data) {
	// Si data es 1, se gira en sentido CW y aumenta el valor del contador hasta 4095 y luego se desborda
	case 0:

		rotationCounter = (rotationCounter  - 1) & MAX_12_BITS;
		printf("Giro CCW\n");
		break;

	// Si data es cero, se gira en sentido CCW y disminuye el valor del contador hasta 0 y luego 4095
	case 1:
		rotationCounter = (rotationCounter + 1) & MAX_12_BITS;
		printf("Giro CW\n");
		break;

	default:
		break;
	}
}



/* Funcion para  seleccionar los modos del Led RGB */
void fsm_rgb_modeSelection(void){

	// Cada que ingresa en un case se selecciona el siguiente estado del led rgb
	switch (fsm_RGB.stateRGB){
		// Rojo
		case RED_STATE:{
			gpio_WritePin(&ledRed,SET);
			gpio_WritePin(&ledGreen,RESET);
			gpio_WritePin(&ledBlue,RESET);

			// Cambio al siguiente estado RGB
			fsm_RGB.stateRGB = GREEN_STATE;

			// Imprimo para la fsm el color actual
			printf("Current RGB color: Red\n");
			break;
		}
		// Verde
		case GREEN_STATE:{
			gpio_WritePin(&ledRed,RESET);
			gpio_WritePin(&ledGreen,SET);
			gpio_WritePin(&ledBlue,RESET);

			// Cambio al siguiente estado RGB
			fsm_RGB.stateRGB = BLUE_STATE;

			// Imprimo para la fsm el color actual
			printf("Current RGB color: Green\n");

			break;
		}
		// Azul
		case BLUE_STATE:{
			gpio_WritePin(&ledRed,RESET);
			gpio_WritePin(&ledGreen,RESET);
			gpio_WritePin(&ledBlue,SET);

			// Cambio al siguiente estado RGB
			fsm_RGB.stateRGB = BLUE_GREEN_STATE;

			// Imprimo para la fsm el color actual
			printf("Current RGB color: Blue\n");
			break;
		}
		// Azul + verde
		case BLUE_GREEN_STATE:{
			gpio_WritePin(&ledRed,RESET);
			gpio_WritePin(&ledGreen,SET);
			gpio_WritePin(&ledBlue,SET);

			// Cambio al siguiente estado RGB
			fsm_RGB.stateRGB = BLUE_RED_STATE;

			// Imprimo para la fsm el color actual
			printf("Current RGB color: BLUE + GREEN\n");
			break;
		}
		// Azul + rojo
		case BLUE_RED_STATE:{
			gpio_WritePin(&ledRed,SET);
			gpio_WritePin(&ledGreen,RESET);
			gpio_WritePin(&ledBlue,SET);

			// Cambio al siguiente estado RGB
			fsm_RGB.stateRGB = RED_GREEN_STATE;

			// Imprimo para la fsm el color actual
			printf("Current RGB color: BLUE + RED\n");
			break;
		}
		// Rojo + verde
		case RED_GREEN_STATE:{
			gpio_WritePin(&ledRed,SET);
			gpio_WritePin(&ledGreen,SET);
			gpio_WritePin(&ledBlue,RESET);

			// Cambio al siguiente estado RGB
			fsm_RGB.stateRGB = RED_GREEN_BLUE_STATE;

			// Imprimo para la fsm el color actual
			printf("Current RGB color: RED + GREEN\n");
			break;
		}
		// Rojo + verde + azul = blanco
		case RED_GREEN_BLUE_STATE:{
			gpio_WritePin(&ledRed,SET);
			gpio_WritePin(&ledGreen,SET);
			gpio_WritePin(&ledBlue,SET);

			// Cambio al siguiente estado RGB
			fsm_RGB.stateRGB = DISABLE_STATE;

			// Imprimo para la fsm el color actual
			printf("Current RGB color: RGB-White\n");
			break;
		}
		// Apagado
		case DISABLE_STATE:{
			gpio_WritePin(&ledRed,RESET);
			gpio_WritePin(&ledGreen,RESET);
			gpio_WritePin(&ledBlue,RESET);

			// Cambio al estado RGB inicial
			fsm_RGB.stateRGB = RED_STATE;

			// Imprimo para la fsm el color actual
			printf("Current RGB color: None\n");
			break;
		}
		default:{
			break;
		}
		}
}



/* Función que apaga los transistores para evitar el ghosting */
void disableTransistors(void){
	gpio_WritePin(&digitoUnidad, OFF);
	gpio_WritePin(&digitoDecena, OFF);
	gpio_WritePin(&digitoCentena, OFF);
	gpio_WritePin(&digitoUnMillar, OFF);
}



/* Funcion que selecciona los segmentos para asignar un numero en el display */
void numberSelection(uint8_t displayNumber){

	/* Switch case, cada caso corresponde al valor numerico seleccionado */
	switch (displayNumber) {

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
