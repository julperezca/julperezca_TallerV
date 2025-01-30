/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Julián Pérez Carvajal (julperezca@unal.edu.co)
 * @brief          : Taller USART.
 ******************************************************************************
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "stm32f4xx.h"
#include "gpio_driver_hal.h"
#include "timer_driver_hal.h"
#include "exti_driver_hal.h"
#include "usart_driver_hal.h"
#include "string.h"

//Parametro puntero a un arreglo donde se almacena la info
//segundo param formado de string
//tercer parametro valores que se desean observar




Timer_Handler_t blinkTimer = { 0 };
Timer_Handler_t usartRefresh = { 0 };


GPIO_Handler_t userLed = { 0 };
GPIO_Handler_t userLed1 = { 0 };
GPIO_Handler_t userLed2 = { 0 };
GPIO_Handler_t userLed3 = { 0 };


EXTI_Config_t imprimir = {0};
GPIO_Handler_t user13 = {0};

USART_Handler_t usart2 = { 0 };
GPIO_Handler_t usart2T = { 0 };
GPIO_Handler_t usart2R = {0};

char bufferMsg[128] = {0};
char bufferMsgVar[128] = {0};


uint8_t bandera = 0;
uint8_t sendMsg = 0;
uint8_t receivedChar = 0;
uint8_t posicionSafe = 0;
uint8_t msglisto = 0;
uint8_t conteo = 0;
int periodoTimer = 0;


void init_Config(void);
void analizeCommand(char *buffer);
/*
 * The main function, where everything happens.
 */
int main (void){
	init_Config();	// Se inicia la configuracion del sistema

	/* Loop infinito */

	bufferMsg[0] = 0x1B;
	bufferMsg[1] = 0x5B;
	bufferMsg[2] = 0x32;
	bufferMsg[3] = 0x4A;
	usart_writeMsg(&usart2, bufferMsg);
	usart_writeMsg(&usart2, "Escribe help para abrir el manual de instr \n\n");
	while(1){

//		if (sendMsg){
//			usart_writeMsg(&usart2, "Hola mundo\n");
//			sprintf(bufferMsg,"has hecho blin %d \n\n",conteo);
//			usart_writeMsg(&usart2, bufferMsg);
//			sendMsg = 0;
//		}
		if (receivedChar){
			if(receivedChar == ' '){
				msglisto = 1;
			}
			else{
				bufferMsg[posicionSafe] = receivedChar;
				posicionSafe ++;
			}
			receivedChar = 0;
			}

		if (msglisto){

			gpio_WritePin(&userLed, SET);
			analizeCommand(bufferMsg);
			for (uint8_t i = 0; i < sizeof(bufferMsg); i++){
				bufferMsg[i] = 0;
			}
			posicionSafe = 0;
			msglisto = 0;
		}
	}


}


/* Funcion encargada de la configuración del GPIO, TIMERS y EXTIs */
void init_Config(void){

	/* Configuramos el timer del blink (TIM2) */
		blinkTimer.pTIMx = TIM2;
		blinkTimer.TIMx_Config.TIMx_Prescaler = 16000;
		blinkTimer.TIMx_Config.TIMx_Period = 250;
		blinkTimer.TIMx_Config.TIMx_mode = TIMER_UP_COUNTER;
		blinkTimer.TIMx_Config.TIMx_InterruptEnable = TIMER_INT_ENABLE;

		timer_Config(&blinkTimer);
		timer_SetState(&blinkTimer, SET);

		/* Configuramos el timer del blink (TIM2) */
		usartRefresh.pTIMx = TIM3;
		usartRefresh.TIMx_Config.TIMx_Prescaler = 16000;
		usartRefresh.TIMx_Config.TIMx_Period = 1000;
		usartRefresh.TIMx_Config.TIMx_mode = TIMER_UP_COUNTER;
		usartRefresh.TIMx_Config.TIMx_InterruptEnable = TIMER_INT_ENABLE;

		timer_Config(&usartRefresh);
		timer_SetState(&usartRefresh, SET);

		//Configuramos los pines que se van a utilizar

		/* Configuramos el PinA5 */
		userLed.pGPIOx = GPIOA;
		userLed.pinConfig.GPIO_PinNumber = PIN_5;
		userLed.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
		userLed.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
		userLed.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_MEDIUM;
		userLed.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;

		gpio_Config(&userLed);

		/* Configuramos el PinA9 */
		userLed3.pGPIOx = GPIOA;
		userLed3.pinConfig.GPIO_PinNumber = PIN_9;
		userLed3.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
		userLed3.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
		userLed3.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_MEDIUM;
		userLed3.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;

		gpio_Config(&userLed3);

		/* Configuramos el PinA8 */
		userLed2.pGPIOx = GPIOA;
		userLed2.pinConfig.GPIO_PinNumber = PIN_8;
		userLed2.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
		userLed2.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
		userLed2.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_MEDIUM;
		userLed2.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;

		gpio_Config(&userLed2);

		/* Configuramos el PinA9 */
		userLed1.pGPIOx = GPIOA;
		userLed1.pinConfig.GPIO_PinNumber = PIN_7;
		userLed1.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
		userLed1.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
		userLed1.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_MEDIUM;
		userLed1.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;

		gpio_Config(&userLed1);

		usart2.ptrUSARTx = USART2;
		usart2.USART_Config.baudrate = USART_BAUDRATE_115200;
		usart2.USART_Config.datasize = USART_DATASIZE_8BIT;
		usart2.USART_Config.mode = USART_MODE_RXTX;
		usart2.USART_Config.parity = USART_PARITY_NONE;
		usart2.USART_Config.stopbits = USART_STOPBIT_1;
		usart2.USART_Config.enableIntTX = USART_TX_INTERRUP_DISABLE;
		usart2.USART_Config.enableIntRX = USART_RX_INTERRUP_ENABLE;

		usart_Config(&usart2);

		usart2T.pGPIOx = GPIOA;
		usart2T.pinConfig.GPIO_PinNumber = PIN_2;
		usart2T.pinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
		usart2T.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
		usart2T.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_MEDIUM;
		usart2T.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		usart2T.pinConfig.GPIO_PinAltFunMode = AF7;

		gpio_Config(&usart2T);

		usart2R.pGPIOx = GPIOA;
		usart2R.pinConfig.GPIO_PinNumber = PIN_3;
		usart2R.pinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
		usart2R.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
		usart2R.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_MEDIUM;
		usart2R.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		usart2R.pinConfig.GPIO_PinAltFunMode = AF7;

		gpio_Config(&usart2R);

		user13.pGPIOx = GPIOC;
		user13.pinConfig.GPIO_PinNumber = PIN_13;
		user13.pinConfig.GPIO_PinMode = GPIO_MODE_IN;

		gpio_Config(&user13);

		imprimir.pGPIOHandler = &user13;
		imprimir.edgeType = EXTERNAL_INTERRUPT_FALLING_EDGE;

		exti_Config(&imprimir);
}

void analizeCommand (char * buffer){
	if(strcmp(buffer, "help") == 0){
		usart_writeMsg(&usart2, "1. Escribe RedON para pinA9 \n\n");
		usart_writeMsg(&usart2, "2. Escribe GreenON para pinA9 \n\n");
		usart_writeMsg(&usart2, "3. Escribe BlueON para pinA9 \n\n");
		usart_writeMsg(&usart2, "4. Limpiar el terminal \n\n");
		usart_writeMsg(&usart2, "5. E7S escribir un numero en el display \n\n");
		usart_writeMsg(&usart2, "6. Escribe CPT cambia el periodo del timer\n\n");
		usart_writeMsg(&usart2, "7. Escribe help\n\n");
	}
	else if(strcmp(buffer, "CRON") == 0){
		gpio_WritePin(&userLed1, SET);
		usart_writeMsg(&usart2, "led encendido\n");
	}
	else if(strcmp(buffer, "CGON") == 0){
		gpio_WritePin(&userLed2, SET);
		usart_writeMsg(&usart2, "led encendido\n");
	}
	else if(strcmp(buffer, "CBGON") == 0){
		gpio_WritePin(&userLed3, SET);
		usart_writeMsg(&usart2, "led encendido\n");
	}
	else if(strcmp(buffer, "clear") == 0){
		bufferMsgVar[0] = 0x1B;
		bufferMsgVar[1] = 0x5B;
		bufferMsgVar[2] = 0x32;
		bufferMsgVar[3] = 0x4A;
		usart_writeMsg(&usart2, bufferMsgVar);
	}
	else if(strcmp(buffer, "CPT") == 0){
		timer_SetState(&blinkTimer, TIMER_OFF);
		usart_writeMsg(&usart2, "Escriba el periodo en ms\n");
		scanf(periodoTimer);
		blinkTimer.TIMx_Config.TIMx_Period = periodoTimer;
		timer_Config(&blinkTimer);
		timer_SetState(&blinkTimer,TIMER_ON);
		sprintf(buffer,"cambio%d\n",periodoTimer);
		usart_writeMsg(&usart2, "led encendido\n");
	}
}

void Timer2_Callback(void) {
	gpio_TooglePin(&userLed);
	conteo++;
}
void Timer3_Callback(void) {
	sendMsg = 1;
}

void callback_ExtInt13(void){
	bandera = 1;
}
void usart2_RxCallback(void){
	receivedChar = usart_getRxData(&usart2);
}

