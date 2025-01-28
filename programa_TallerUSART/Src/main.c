/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Julián Pérez Carvajal (julperezca@unal.edu.co)
 * @brief          : Tarea 2. Drivers GPIO, EXTI, TIMERS, magicProject.
 ******************************************************************************
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "stm32f4xx.h"
#include "exti_driver_hal.h"
#include "timer_driver_hal.h"
#include "systick_driver.h"
#include "usart_driver_hal.h"

//Parametro puntero a un arreglo donde se almacena la info
//segundo param formado de string
//tercer parametro valores que se desean observar




GPIO_Handler_t handlerBlinky = {0};
GPIO_Handler_t handlerUserButton = {0};
EXTI_Config_t handlerUserButtonExti = {0};
Timer_Handler_t handlerBlinkyTimer = {0};


GPIO_Handler_t handlerPinTX 	= {0};
GPIO_Handler_t handlerPinRX 	= {0};
USART_Handler_t usart2Comm		= {0};

uint8_t sendMsg = 0;


uint8_t usart2DataReceiver = 0;

char bufferMsg[64] = {0};

void init_Config(void);
/*
 * The main function, where everything happens.
 */
int main (void){
	init_Config();	// Se inicia la configuracion del sistema

	/* Loop infinito */

	while(1){
		if (sendMsg > 4){

//			sprintf(bufferMsg, "Recibido Char : \n");
//			usart_writeMsg(&usart2Comm, bufferMsg);
			sendMsg = 0;
		}
    }
	return 0;
}

/* Funcion encargada de la configuración del GPIO, TIMERS y EXTIs */
void init_Config(void){

			/* Configuración de LED de estado y su respectivo timer */

	// GPIO config para Led de estado
	handlerBlinky.pGPIOx							= GPIOA;
	handlerBlinky.pinConfig.GPIO_PinNumber			= PIN_5;
	handlerBlinky.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerBlinky.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	handlerBlinky.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_FAST;
	handlerBlinky.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	gpio_Config(&handlerBlinky);

	// Config para el timer del led de estado
	handlerBlinkyTimer.pTIMx								= TIM2;
	handlerBlinkyTimer.TIMx_Config.TIMx_Prescaler  			= 16000; //1ms conversion
	handlerBlinkyTimer.TIMx_Config.TIMx_Period				= 500;
	handlerBlinkyTimer.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;
	handlerBlinkyTimer.TIMx_Config.TIMx_InterruptEnable 	= TIMER_INT_ENABLE;
	timer_Config(&handlerBlinkyTimer);
	timer_SetState(&handlerBlinkyTimer, TIMER_ON);

		/* FIN de configuración de Led de estado y su timer */


			/* Se configuran los pines para el led RGB */

	//Led rojo
	handlerUserButton.pGPIOx						= GPIOC;
	handlerUserButton.pinConfig.GPIO_PinNumber		= PIN_13;
	handlerUserButton.pinConfig.GPIO_PinMode		= GPIO_MODE_IN;
	handlerUserButton.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&handlerUserButton);


	// Configuración EXTI para el CLK
	handlerUserButtonExti.pGPIOHandler				= &handlerUserButton;
	handlerUserButtonExti.edgeType					= EXTERNAL_INTERRUPT_RISING_EDGE;
	exti_Config(&handlerUserButtonExti);

	handlerPinTX.pGPIOx								= GPIOA;
	handlerPinTX.pinConfig.GPIO_PinNumber			= PIN_2;
	handlerPinTX.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
	handlerPinTX.pinConfig.GPIO_PinAltFunMode		= AF7;
	gpio_Config(&handlerPinTX);


	handlerPinRX.pGPIOx								= GPIOA;
	handlerPinRX.pinConfig.GPIO_PinNumber			= PIN_3;
	handlerPinRX.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
	handlerPinRX.pinConfig.GPIO_PinAltFunMode		= AF7;
	gpio_Config(&handlerPinRX);

	usart2Comm.ptrUSARTx 							= USART2;
	usart2Comm.USART_Config.baudrate 				= USART_BAUDRATE_19200;
	usart2Comm.USART_Config.datasize 				= USART_DATASIZE_8BIT;
	usart2Comm.USART_Config.parity 					= USART_PARITY_NONE;
	usart2Comm.USART_Config.stopbits 				= USART_STOPBIT_1;
	usart2Comm.USART_Config.mode 					= USART_MODE_RXTX;
	usart2Comm.USART_Config.enableIntRX 			= USART_RX_INTERRUP_ENABLE;
	usart2Comm.USART_Config.enableIntTX 			= USART_TX_INTERRUP_DISABLE;

	usart_Config(&usart2Comm);
}


void callback_ExtInt13(void){
	__NOP();
}

void Timer2_Callback(void){
	gpio_TooglePin(&handlerBlinky);
	sendMsg++;
}

/* Funcion que se ejecuta cada vez que un caracter es recibido
 * por el puerto USART2
 */
void usart2_RxCallback(void){
	usart2DataReceiver = usart_getRxData();
	sprintf(bufferMsg,"recibido %c \n", usart2DataReceiver);
	usart_writeMsg(&usart2Comm, bufferMsg);
}

