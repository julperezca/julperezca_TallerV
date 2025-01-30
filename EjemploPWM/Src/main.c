/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Julián Pérez Carvajal (julperezca@unal.edu.co)
 * @brief          : Ejemplo PWM.
 ******************************************************************************
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx.h"
#include "gpio_driver_hal.h"
#include "timer_driver_hal.h"
#include "exti_driver_hal.h"
#include "usart_driver_hal.h"
#include "pwm_driver_hal.h"

// definicion de los handlers
GPIO_Handler_t handlerBlinkyPin  = {0};

GPIO_Handler_t handlerUserButton  = {0};
EXTI_Config_t handlerUserButtonExti  = {0};

Timer_Handler_t handlerBlinkyTimer = {0};

/* elemento para hacer comunicación serial */
GPIO_Handler_t handlerPinTX  = {0};
GPIO_Handler_t handlerPinRX  = {0};
USART_Handler_t usart2Comm  = {0};
uint8_t sendMsg  = 0;
uint8_t usart2DataReceived = 0;

/* Elementos para el PWM*/
GPIO_Handler_t handlerPinPwmChannel  = {0};
PWM_Handler_t handlerSignalPWM  = {0};

uint16_t duttyValue = 10;

char bufferMsg[64] = {0};

void init_Config(void);
void analizeCommand(char *buffer);
/*
 * The main function, where everything happens.
 */
int main (void){
	init_Config();	// Se inicia la configuracion del sistema
//	SCB->CPACR |= (0xF << 20);

	bufferMsg[0] = 0x1B;
	bufferMsg[1] = 0x5B;
	bufferMsg[2] = 0x32;
	bufferMsg[3] = 0x4A;


	while(1){


		if (usart2DataReceived != '\0'){
			if(usart2DataReceived == 'D'){
				if(duttyValue == 0){
					duttyValue = 110;
				}
				duttyValue -= 10;
				pwm_Update_DuttyCycle(&handlerSignalPWM, duttyValue);
			}

			if (usart2DataReceived == 'U'){
				if(duttyValue == 100){
					duttyValue = -10;
				}
				duttyValue +=10;
				pwm_Update_DuttyCycle(&handlerSignalPWM, duttyValue);

			}

			sprintf(bufferMsg, "dutty = %u \n", (unsigned int)duttyValue);
			usart_writeMsg(&usart2Comm, bufferMsg);
			usart2DataReceived = '\0';

		}
	}

	return 0;


}


/* Funcion encargada de la configuración del GPIO, TIMERS y EXTIs */
void init_Config(void){

		handlerBlinkyPin.pGPIOx = GPIOH;
		handlerBlinkyPin.pinConfig.GPIO_PinNumber = PIN_1;
		handlerBlinkyPin.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
		handlerBlinkyPin.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
		handlerBlinkyPin.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_FAST;
		handlerBlinkyPin.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		gpio_Config(&handlerBlinkyPin);



	/* Configuramos el timer del blink (TIM2) */
		handlerBlinkyTimer.pTIMx = TIM2;
		handlerBlinkyTimer.TIMx_Config.TIMx_Prescaler = 16000;
		handlerBlinkyTimer.TIMx_Config.TIMx_Period = 250;
		handlerBlinkyTimer.TIMx_Config.TIMx_mode = TIMER_UP_COUNTER;
		handlerBlinkyTimer.TIMx_Config.TIMx_InterruptEnable = TIMER_INT_ENABLE;

		timer_Config(&handlerBlinkyTimer);
		timer_SetState(&handlerBlinkyTimer, SET);

		handlerUserButton.pGPIOx = GPIOC;
		handlerUserButton.pinConfig.GPIO_PinNumber = PIN_13;
		handlerUserButton.pinConfig.GPIO_PinMode = GPIO_MODE_IN;
		handlerUserButton.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		gpio_Config(&handlerUserButton);

		handlerUserButtonExti.pGPIOHandler = &handlerUserButton;
		handlerUserButtonExti.edgeType = EXTERNAL_INTERRUPT_RISING_EDGE;
		exti_Config(&handlerUserButtonExti);


		handlerPinTX.pGPIOx = GPIOA;
		handlerPinTX.pinConfig.GPIO_PinNumber = PIN_2;
		handlerPinTX.pinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
		handlerPinTX.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
		handlerPinTX.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_MEDIUM;
		handlerPinTX.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		handlerPinTX.pinConfig.GPIO_PinAltFunMode = AF7;
		gpio_Config(&handlerPinTX);

		handlerPinRX.pGPIOx = GPIOA;
		handlerPinRX.pinConfig.GPIO_PinNumber = PIN_3;
		handlerPinRX.pinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
		handlerPinRX.pinConfig.GPIO_PinAltFunMode = AF7;
		gpio_Config(&handlerPinRX);

		usart2Comm.ptrUSARTx = USART2;
		usart2Comm.USART_Config.baudrate = USART_BAUDRATE_19200;
		usart2Comm.USART_Config.datasize = USART_DATASIZE_8BIT;
		usart2Comm.USART_Config.parity = USART_PARITY_NONE;
		usart2Comm.USART_Config.stopbits = USART_STOPBIT_1;
		usart2Comm.USART_Config.mode = USART_MODE_RXTX;
		usart2Comm.USART_Config.enableIntRX = USART_RX_INTERRUP_ENABLE;
		usart2Comm.USART_Config.enableIntTX = USART_TX_INTERRUP_DISABLE;
		usart_Config(&usart2Comm);





		/*PWM config */
		handlerPinPwmChannel.pGPIOx = GPIOC;
		handlerPinPwmChannel.pinConfig.GPIO_PinNumber = PIN_7;
		handlerPinPwmChannel.pinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
		handlerPinPwmChannel.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
		handlerPinPwmChannel.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		handlerPinPwmChannel.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_FAST;
		handlerPinPwmChannel.pinConfig.GPIO_PinAltFunMode = AF2;
		gpio_Config(&handlerPinPwmChannel);

		handlerSignalPWM.ptrTIMx = TIM3;
		handlerSignalPWM.config.channel = PWM_CHANNEL_2;
		handlerSignalPWM.config.duttyCicle = duttyValue;
		handlerSignalPWM.config.periodo = 100;
		handlerSignalPWM.config.prescaler = 16000;
		pwm_Config(&handlerSignalPWM);

		pwm_Enable_Output(&handlerSignalPWM);
		pwm_Start_Signal(&handlerSignalPWM);



		GPIO_Handler_t PINC12 = {0};
		PINC12.pGPIOx = GPIOC;
		PINC12.pinConfig.GPIO_PinNumber = PIN_12;
		PINC12.pinConfig.GPIO_PinMode = GPIO_MODE_OUT;
		PINC12.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
		PINC12.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_FAST;
		PINC12.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		gpio_Config(&PINC12);
		gpio_WritePin(&PINC12, SET);
}



void Timer2_Callback(void) {
	gpio_TooglePin(&handlerBlinkyPin);
	sendMsg++;
}


void callback_ExtInt13(void){
	__NOP();
}
//void usart2_RxCallback(void){
//	usart2DataReceived = usart_getRxData();
//}

