/**
 ******************************************************************************
 * @file           : main.c
 * @author         : julperezca
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>



#include "gpio_driver_hal.h"
#include "stm32f4xx.h"
#include "stm32_assert.h"
#include "timer_driver_hal.h"
#include "usart_driver_hal.h"
#include "i2c_driver_hal.h"




//def de los headers necesarios
GPIO_Handler_t	userLed = {0};

GPIO_Handler_t handlerLedRed   = {0};
GPIO_Handler_t handlerLedGreen = {0};
GPIO_Handler_t handlerLedBlue  = {0};

//comunicacion rs 232 con el pc
USART_Handler_t commSerial ={0};

GPIO_Handler_t pinTx = {0};
GPIO_Handler_t pinRx = {0};

/*TIMERS*/
Timer_Handler_t blinkTimer 			= {0};
Timer_Handler_t updateDisplayTimer  = {0};
Timer_Handler_t updateCounter 		= {0};

/*pines para el i2c */
GPIO_Handler_t pinSCL = {0};
GPIO_Handler_t pinSDA = {0};

I2C_Handler_t accelSensor = {0};
uint8_t i2c_AuxBuffer = 0;
uint8_t flag = 0;


/*registros y valores relacionados con el mpu*/
#define ACCEL_ADDRES 0b1101000; //0xD0 Direcion del accel con logic_0
#define ACCEL_XOUT_H 	 59
#define ACCEL_XOUT_L 	 60
#define ACCEL_YOUT_H 	 61
#define ACCEL_YOUT_L 	 62
#define ACCEL_ZOUT_H	 63
#define ACCEL_ZOUT_L	 64
#define PWR_MGMT_1 		 107
#define WHO_AM_I   		 117

uint8_t receivedChar = '\0';
uint8_t sendMsg = 0;

//mensaje que se imprime
char greetingMsg[]= "Taller v Rocks!\n";
char bufferData[64] = "Accel MPU-6850 testing \n";

//defde las cabeceras de las funciones del main
void initSystem(void);
void config_LedRGB(void);
void config_I2C(void);

/*1
 *The main function
 **/
int main(void){

	initSystem();
	config_LedRGB();
	config_I2C();
	usart_writeMsg(&commSerial, greetingMsg);





    while(1){
    	if(flag == 1){

    		if(receivedChar != '\0'){
    			if(receivedChar == 'm'){
    				usart_writeMsg(&commSerial, bufferData);

    			}
    			if(receivedChar == 'w'){
    				sprintf(bufferData, "WHO_AM_I? (r)\n");
    				usart_writeMsg(&commSerial, bufferData);

    				i2c_AuxBuffer = i2c_ReadSingleRegister(&accelSensor, WHO_AM_I);
    				sprintf(bufferData, "dataRead = 0x%x \n",(unsigned int) i2c_AuxBuffer);
    				usart_writeMsg(&commSerial, bufferData);

    			}
    			else if(receivedChar == 'p'){
    				sprintf(bufferData, "PWR_MGMT_1 state (r)\n ");
    				usart_writeMsg(&commSerial, bufferData);


    				i2c_AuxBuffer = i2c_ReadSingleRegister(&accelSensor, PWR_MGMT_1);
    				sprintf(bufferData, "dataRead = 0x%x \n", (unsigned int) i2c_AuxBuffer);
    				usart_writeMsg(&commSerial, bufferData);

    			}
    			else if(receivedChar == 'r'){
    				sprintf(bufferData, "PWR_MGMT_1 RESET (w)\n ");
    				usart_writeMsg(&commSerial, bufferData);


    				i2c_WriteSingleRegister(&accelSensor, PWR_MGMT_1, 0x00);



    			}

    			else if(receivedChar == 'x'){
    				sprintf(bufferData, "Axis x data  (r)\n");
    				usart_writeMsg(&commSerial, bufferData);

    				uint8_t AccelLX_low  = i2c_ReadSingleRegister(&accelSensor, ACCEL_XOUT_L);
    				uint8_t AccelLX_high = i2c_ReadSingleRegister(&accelSensor, ACCEL_XOUT_H);
    				int16_t AccelX = AccelLX_high << 8 |  AccelLX_low;
    				sprintf(bufferData, "Accelx = %d \n",  (int) AccelX);
    				usart_writeMsg(&commSerial, bufferData);



    			}
				else if(receivedChar == 'y'){
					sprintf(bufferData, "Axis y data  (r)\n ");
					usart_writeMsg(&commSerial, bufferData);

					uint8_t AccelLY_low  = i2c_ReadSingleRegister(&accelSensor, ACCEL_YOUT_L);
					uint8_t AccelLY_high = i2c_ReadSingleRegister(&accelSensor, ACCEL_YOUT_H);
					int16_t AccelY = AccelLY_high << 8 |  AccelLY_low;
					sprintf(bufferData, "AccelY = %d \n",  (int) AccelY);
					usart_writeMsg(&commSerial, bufferData);

				}
				else if(receivedChar == 'z'){
					sprintf(bufferData, "Axis z data  (r)\n ");
					usart_writeMsg(&commSerial, bufferData);

					uint8_t AccelLZ_low  = i2c_ReadSingleRegister(&accelSensor, ACCEL_ZOUT_L);
					uint8_t AccelLZ_high = i2c_ReadSingleRegister(&accelSensor, ACCEL_ZOUT_H);
					int16_t AccelZ = AccelLZ_high << 8 |  AccelLZ_low;
					sprintf(bufferData, "AccelZ = %d \n",  (int) AccelZ);
					usart_writeMsg(&commSerial, bufferData);



				}
				else if(receivedChar == 'q'){
					sprintf(bufferData, "All 3 axis  (r)\n");
					usart_writeMsg(&commSerial, bufferData);

					uint8_t AccelData[6] = {0};
//					i2c_ReadManyRegister(&accelSensor, ACCEL_XOUT_H, AccelData, 6);
					int16_t AccelX = AccelData[0] << 8 | AccelData[1];
					int16_t AccelY = AccelData[2] << 8 | AccelData[3];
					int16_t AccelZ = AccelData[4] << 8 | AccelData[5];
					sprintf(bufferData, "Accel x,y,z -> %d;  %d; %d \n", (int) AccelX, (int) AccelY, (int) AccelZ);
					usart_writeMsg(&commSerial, bufferData);


				}
				receivedChar = '\0';
    		}





    			flag = 0;
			}


		}
		return 0;
}

void initSystem(void){
	userLed.pGPIOx                          = GPIOA;
	userLed.pinConfig.GPIO_PinNumber        = PIN_5;
	userLed.pinConfig.GPIO_PinMode          = GPIO_MODE_OUT;
	userLed.pinConfig.GPIO_PinOutputType    = GPIO_OTYPE_PUSHPULL;
	userLed.pinConfig.GPIO_PinOutputSpeed   	  = GPIO_OSPEED_MEDIUM;
	userLed.pinConfig.GPIO_PinPuPdControl   = GPIO_PUPDR_NOTHING;

    /* Cargamos config en los registros*/
    gpio_Config(&userLed);

    //timer blink
    blinkTimer.pTIMx							= TIM2;
    blinkTimer.TIMx_Config.TIMx_Prescaler		= 16000; //genera incremento  de 1 ms
	blinkTimer.TIMx_Config.TIMx_Period 			= 500; 	//de la mano con el prescaler
	blinkTimer.TIMx_Config.TIMx_mode 			=TIMER_UP_COUNTER;
	blinkTimer.TIMx_Config.TIMx_InterruptEnable =TIMER_INT_ENABLE;

	/*CONFIG DEL TIMER*/
	timer_Config(&blinkTimer);

    //timer blink
    updateDisplayTimer.pTIMx							= TIM3;
    updateDisplayTimer.TIMx_Config.TIMx_Prescaler		= 16000; //genera incremento  de 1 ms
    updateDisplayTimer.TIMx_Config.TIMx_Period 			= 60; 	//de la mano con el prescaler
    updateDisplayTimer.TIMx_Config.TIMx_mode 			=TIMER_UP_COUNTER;
    updateDisplayTimer.TIMx_Config.TIMx_InterruptEnable =TIMER_INT_ENABLE;

	/*CONFIG DEL TIMER*/
	timer_Config(&updateDisplayTimer);

    //timer blink
    updateCounter.pTIMx							= TIM4;
    updateCounter.TIMx_Config.TIMx_Prescaler		= 16000; //genera incremento  de 1 ms
    updateCounter.TIMx_Config.TIMx_Period 			= 100; 	//de la mano con el prescaler
    updateCounter.TIMx_Config.TIMx_mode 			=TIMER_UP_COUNTER;
    updateCounter.TIMx_Config.TIMx_InterruptEnable =TIMER_INT_ENABLE;

	/*CONFIG DEL TIMER*/
	timer_Config(&updateCounter);

	/*pintx usart 2 TX*/
	pinTx.pGPIOx                          = GPIOA;
	pinTx.pinConfig.GPIO_PinNumber        = PIN_2;
	pinTx.pinConfig.GPIO_PinMode          = GPIO_MODE_ALTFN;
	pinTx.pinConfig.GPIO_PinAltFunMode 	  = AF7;
	pinTx.pinConfig.GPIO_PinOutputSpeed  	  = GPIO_OSPEED_FAST;
	pinTx.pinConfig.GPIO_PinPuPdControl   = GPIO_PUPDR_NOTHING;
	gpio_Config(&pinTx);


	/*pintx usart 2 RX*/
	pinRx.pGPIOx                          = GPIOA;
	pinRx.pinConfig.GPIO_PinNumber        = PIN_3;
	pinRx.pinConfig.GPIO_PinMode          = GPIO_MODE_ALTFN;
	pinRx.pinConfig.GPIO_PinAltFunMode 	  = AF7;
	pinRx.pinConfig.GPIO_PinOutputSpeed   	  = GPIO_OSPEED_FAST;
	pinRx.pinConfig.GPIO_PinPuPdControl   = GPIO_PUPDR_NOTHING;
	gpio_Config(&pinRx);



	//CONFIG EL PUERTO SERIAL (USART2)
	commSerial.ptrUSARTx				= USART2;
	commSerial.USART_Config.baudrate    = USART_BAUDRATE_115200;
	commSerial.USART_Config.datasize	= USART_DATASIZE_8BIT;
	commSerial.USART_Config.parity		= USART_PARITY_NONE;
	commSerial.USART_Config.stopbits	= USART_STOPBIT_1;
	commSerial.USART_Config.mode 		= USART_MODE_RXTX;
	commSerial.USART_Config.enableIntRX = USART_RX_INTERRUP_ENABLE;
	usart_Config(&commSerial);
	usart_WriteChar(&commSerial, '\0');

	/*encendiendo los timers */
	timer_SetState(&blinkTimer, TIMER_ON);
	timer_SetState(&updateDisplayTimer, TIMER_ON);
	timer_SetState(&updateCounter, TIMER_ON);
}


void config_I2C(void){
	pinSCL.pGPIOx 								= GPIOB;
	pinSCL.pinConfig.GPIO_PinNumber        	  	= PIN_8;
	pinSCL.pinConfig.GPIO_PinMode     		 	= GPIO_MODE_ALTFN;
	pinSCL.pinConfig.GPIO_PinOutputType    		= GPIO_OTYPE_OPENDRAIN;
	pinSCL.pinConfig.GPIO_PinPuPdControl   		= GPIO_PUPDR_NOTHING;
	pinSCL.pinConfig.GPIO_PinOutputSpeed  	  	= GPIO_OSPEED_FAST;
	pinSCL.pinConfig.GPIO_PinAltFunMode	  		= AF4;
	gpio_Config(&pinSCL);

	pinSDA.pGPIOx 							= GPIOB;
	pinSDA.pinConfig.GPIO_PinNumber         = PIN_9;
	pinSDA.pinConfig.GPIO_PinMode           = GPIO_MODE_ALTFN;
	pinSDA.pinConfig.GPIO_PinOutputType     = GPIO_OTYPE_OPENDRAIN;
	pinSDA.pinConfig.GPIO_PinPuPdControl    = GPIO_PUPDR_NOTHING;
	pinSDA.pinConfig.GPIO_PinOutputSpeed   	= GPIO_OSPEED_FAST;
	pinSDA.pinConfig.GPIO_PinAltFunMode	    = AF4;

	gpio_Config(&pinSDA);

	accelSensor.pI2Cx  			= I2C1;
	accelSensor.i2c_mainClock   = I2C_MAIN_CLOCK_16_MHz;
	accelSensor.i2c_mode		= eI2C_MODE_SM;
	accelSensor.slaveAddress    = ACCEL_ADDRES;
	i2c_Config(&accelSensor);
}


void config_LedRGB(void){
	/*handlerLedRed.pGPIOx 					= GPIOC;
	handlerLedRed.pinConfig.GPIO_PinNumber        = PIN_7;
	handlerLedRed.pinConfig.GPIO_PinMode          = GPIO_MODE_OUT;
	handlerLedRed.pinConfig.GPIO_PinOutputType    = GPIO_OTYPE_PUSHPULL;
	handlerLedRed.pinConfig.GPIO_PinPuPdControl   = GPIO_PUPDR_NOTHING;
	handlerLedRed.pinConfig.GPIO_PinoutSpeed   	  = GPIO_OSPEED_FAST;
	gpio_Config(&handlerLedRed);
	gpio_WritePin(&handlerLedRed, RESET); // comienza apagado el led
*/
	handlerLedGreen.pGPIOx 					= GPIOC;
	handlerLedGreen.pinConfig.GPIO_PinNumber        = PIN_9;
	handlerLedGreen.pinConfig.GPIO_PinMode          = GPIO_MODE_OUT;
	handlerLedGreen.pinConfig.GPIO_PinOutputType    = GPIO_OTYPE_PUSHPULL;
	handlerLedGreen.pinConfig.GPIO_PinPuPdControl   = GPIO_PUPDR_NOTHING;
	handlerLedGreen.pinConfig.GPIO_PinOutputSpeed   = GPIO_OSPEED_FAST;
	gpio_Config(&handlerLedGreen);
	gpio_WritePin(&handlerLedGreen, RESET); // comienza apagado el led

	handlerLedBlue.pGPIOx 							= GPIOC;
	handlerLedBlue.pinConfig.GPIO_PinNumber        = PIN_8;
	handlerLedBlue.pinConfig.GPIO_PinMode          = GPIO_MODE_OUT;
	handlerLedBlue.pinConfig.GPIO_PinOutputType    = GPIO_OTYPE_PUSHPULL;
	handlerLedBlue.pinConfig.GPIO_PinPuPdControl   = GPIO_PUPDR_NOTHING;
	handlerLedBlue.pinConfig.GPIO_PinOutputSpeed   = GPIO_OSPEED_FAST;
	gpio_Config(&handlerLedBlue);
	gpio_WritePin(&handlerLedBlue, RESET); // comienza apagado el led
}

/*
 * Overwrite Function
 **/
void Timer2_Callback(void){
	gpio_TooglePin(&userLed);
	sendMsg = 1;
	flag = 1;
}

void Timer3_Callback(void){
	//este callback permitirá el switch de los digitos
	__NOP();
}
void Timer4_Callback(void){
	//callback que incrementara el numero con el timer
	__NOP();
}
void usart2_RxCallback(void){

	receivedChar = usart_getRxData(&commSerial);

	sprintf(bufferData, "Dato recibido: %c\n\r", receivedChar);
	usart_writeMsg(&commSerial, bufferData);

}



/*
 *Esta funcion sirve para detectar problemas de parametros
 *incorrectos
 **/
void assert_failed(uint8_t* file, uint32_t line){
    while(1){
        // problems
    }
}
