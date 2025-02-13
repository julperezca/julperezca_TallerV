/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Julián Pérez Carvajal (julperezca@unal.edu.co)
 * @brief          : Tarea 4. Drivers GPIO, EXTI, TIMERS, PWM, USART,ADC, magicProject. +curvas transistor.
 ******************************************************************************
 */

#include <string.h>
#include <main.h>
#include <stdio.h>
#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32_assert.h"
#include "exti_driver_hal.h"
#include "timer_driver_hal.h"
#include "systick_driver.h"
#include "pwm_driver_hal.h"
#include "usart_driver_hal.h"
#include "AdcDriver.h"

#define BUFFER_SIZE 64

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
GPIO_Handler_t segmentG 	 	= {0}; 		// PinC12  segmento g

	/* GPIO handler y TIMER para los transistores  */
GPIO_Handler_t digitoUnidad 	 	= {0}; 		// PinC10
GPIO_Handler_t digitoDecena			= {0}; 		// PinA5
GPIO_Handler_t digitoCentena 		= {0}; 		// PinB9
GPIO_Handler_t digitoUnMillar 		= {0}; 		// PinC5
Timer_Handler_t transistorsTimer	= {0}; 		// TIM4 para los transistores

	/* GPIO handler y EXTI config para el CLK del encoder*/
GPIO_Handler_t userClock 			 = {0}; 		// PinB2
EXTI_Config_t extiClock	 			 = {0}; 		// EXTI2

	/* GPIO handler y EXTI config para el SW del encoder*/
GPIO_Handler_t userSwitch 			 = {0}; 		// PinB15
EXTI_Config_t extiSwitch 			 = {0}; 		// EXTI15

	/* GPIO handler para el DT del encoder*/
GPIO_Handler_t userData 			 = {0}; 		// PinB1

	/* GPIO handler para PWM del led RGB y filtro RC*/
GPIO_Handler_t handlerPinPwmRgbLed  	 = {0};			// Pin C8
GPIO_Handler_t handlerPinPwmBase 		 = {0};			// Pin A0
GPIO_Handler_t handlerPinPwmCollector 	 = {0};			// Pin A1

	/* PWM Handler para la señal PWM: timer y canal*/
PWM_Handler_t handlerSignalPWMrgb  		 = {0};				// Timer 3, canal 3
PWM_Handler_t handlerSignalPWMBase 	 	 = {0};				// Timer 5, canal 1
PWM_Handler_t handlerSignalPWMCollector  = {0};				// Timer 5, canal 2

	/* Handler para usart6*/
USART_Handler_t hCmdTerminal 		 = {0}; 		// USART6
GPIO_Handler_t usart6Tx 			 = {0};			// Tx  pin C6
GPIO_Handler_t usart6Rx				 = {0};			// Rx  pin C7


/* ADC Handler*/
ADC_Config_t ADC_handler = {0};

/* Finite State Machine + subestados del led RGB, de los transistores y de los segmentos */
fsm_t fsm = {0};
fsm_RGB_t fsm_RGB 					= {0};
fsm_transistor_t fsm_transistor 	= {0};
fsm_segments_t fsm_segments 		= {0};
fsm_rotation_t fsm_rotation 		= {0};

/* Variables globales */
uint8_t data 			 = 0;		// Variable que almacena el estado del DT del encoder
uint8_t clock			 = 0;		// Variable que almacena el estado el CLK del encoder
uint16_t rotationCounter = 0;		// Variable que es mostrada en el display (giros del encoder)
uint8_t blinkyFlag 		 = 0;		// Flag para el parpadeo del led
uint16_t duttyValueRgb = 0;			// valor de 0 a 100
uint16_t duttyValueRC = 0;			// valor de 0 a 100
uint16_t blinkyPeriod = 0;



float data_base[100];		// A7 channel 7
float data_collector[100];	// C4 channel 14



//uint16_t ADC_value = 0;
float ADC_value = 0;
//uint16_t collectorCurrent = 0;
float collectorCurrent = 0;
//uint16_t baseCurrent = 0;
float baseCurrent = 0;
uint8_t R_emisor = 5;
uint16_t R_base = 220;
uint8_t R_colector = 220;
uint16_t Vbb = 0;
uint16_t Vcc = 0;
float VCE = 0;
uint16_t baseVoltaje = 0;
float base_voltage_buffer[100];
float base_current_buffer[100];
//uint16_t collecor_voltage_buffer[128];
float collecor_voltage_buffer[100];
//uint16_t collecor_current_buffer[128];
float collecor_current_buffer[100];
uint8_t contador_base = 0;
uint8_t contador_collector = 0;
//uint16_t base_average = 0;
float base_average = 0;
float collector_average = 0;
uint8_t duttyBase = 1;
uint8_t initial_dutty_collector = 1;
uint8_t bufferCounter = 0;
uint8_t CONTER = 0;
uint16_t counterproof = 0;
uint16_t counterproof2 = 0;
/* Estado de los transistores y segmentos */
enum {
	ON = 0,
	OFF
};



uint8_t rxData = 0;
char bufferReception[BUFFER_SIZE];
uint8_t counterReception;

char cmd[16];
char userMsg[BUFFER_SIZE] = {0};
char bufferData[BUFFER_SIZE] = {0};
char clearBuffer[16] = {0};
unsigned int  firstParameter;
unsigned int  secondParameter;

/* Declaración o prototipo de funciones */
extern void configMagic(void);  				// Config del Magic para comunicación serial
void fsm_rgb_modeSelection(void);				// Función que selecciona el color del led RGB
void init_config(void);							// Función que inicia la config. de los pines, timers y EXTI
void numberSelection(uint8_t displayNumber);	// Función que selecciona los segmentos encendidos, ingresa el valor a mostrar
void fsm_rotation_handler(void); 				// Función encargada del sentido de rotation y el valor de la misma
void disableTransistors(void);					// Función encargada de apagar los transistores para evitar el "fantasma"
void fsm_display_handler(void);					// Función encargada de manejar el los transistores y cada segmento
void state_machine_action(void);
float average(float *databuffer);
/*
 * The main function, where everything happens.
 */
int main (void){
	configMagic();  // Se inicia la configuracion de Magic
	init_config();	// Se inicia la configuracion del sistema


	clearBuffer[0] = 0x1B;
	clearBuffer[1] = 0x5B;
	clearBuffer[2] = 0x32;
	clearBuffer[3] = 0x4A;
	/* Se limpia la terminal*/
	usart_writeMsg(&hCmdTerminal,clearBuffer);

	usart_writeMsg(&hCmdTerminal,"Escriba help @ para desplegar el manual de comandos a utilizar\n");
	/* Loop infinito */

	while(1){

/***********************************/
		if (counterproof2){
			if(CONTER){

				ADC_value = (ADC_handler.adcData) * 3300.0 / 4095.0;
				data_collector[contador_collector] = ADC_value;
				contador_collector++;

				if (contador_collector == 100){
					collector_average = average(data_collector);
					collectorCurrent = (((handlerSignalPWMCollector.config.duttyCicle)*3300.0/100.0) - collector_average)/R_colector;
					collector_average = average(data_collector) - collectorCurrent*R_emisor;
					initial_dutty_collector++;

					collecor_voltage_buffer[bufferCounter] = collector_average;
					collecor_current_buffer[bufferCounter] = collectorCurrent;
					bufferCounter++;


					pwm_Update_DuttyCycle(&handlerSignalPWMCollector,initial_dutty_collector);
					contador_collector = 0;

				}

				if(bufferCounter == 100){
					counterproof2 = 0;
					initial_dutty_collector = 1;
					for (uint8_t i = 0; i < sizeof(bufferReception); i++){
						bufferReception[i] = 0;
					}
					bufferCounter = 0;
					for(uint16_t i = 7; i<100;i++){
						sprintf(bufferData,"%.2f-%.2f \n",collecor_voltage_buffer[i],collecor_current_buffer[i]);
						usart_writeMsg(&hCmdTerminal,bufferData);
					}
					for (uint8_t i = 0; i < 100; i++){
						collecor_voltage_buffer[i] = 0;
						collecor_current_buffer[i] = 0;
					}

				 }
				adc_Config(&ADC_handler);
				startSingleADC();
				CONTER = 0;
			}
		}



		else if(counterproof == 1){
			if(CONTER){
				if(ADC_handler.channel == ADC_CHANNEL_7){
					ADC_value = (ADC_handler.adcData) * 3300.0 / 4095.0;
					data_base[contador_base] = ADC_value;
					ADC_handler.channel = ADC_CHANNEL_14; // PARA colector
					adc_Config(&ADC_handler);
					contador_base++;

				}
				else if(ADC_handler.channel == ADC_CHANNEL_14){
					ADC_value = (ADC_handler.adcData) * 3300.0 / 4095.0;
					data_collector[contador_collector] = ADC_value;
					ADC_handler.channel = ADC_CHANNEL_7; // PARA BASE
					adc_Config(&ADC_handler);
					contador_collector++;
				}


				if (contador_base == 100){
					base_average = average(data_base);
					baseCurrent = (((handlerSignalPWMBase.config.duttyCicle)*3300.0/100.0) - base_average)/R_base;
					duttyBase ++;

					base_voltage_buffer[bufferCounter] = base_average;
					base_current_buffer[bufferCounter] = baseCurrent;
					bufferCounter++;
					pwm_Update_DuttyCycle(&handlerSignalPWMBase,duttyBase);
					contador_base = 0;

				}
				if (contador_collector == 100){
					collector_average = average(data_collector);

					contador_collector = 0;

				}
				if(bufferCounter == 100){
					for (uint8_t i = 0; i < sizeof(bufferReception); i++){
						bufferReception[i] = 0;
					}
					bufferCounter = 0;
					for(uint16_t i = 7; i<100;i++){
						sprintf(bufferData,"%.2f-%.2f \n",base_voltage_buffer[i],base_current_buffer[i]);
						usart_writeMsg(&hCmdTerminal,bufferData);
					}
					for (uint8_t i = 0; i < 100; i++){
						base_voltage_buffer[i] = 0;
						base_current_buffer[i] = 0;
					}
					counterproof = 0;
					duttyBase = 1;
				 }
				adc_Config(&ADC_handler);
				startSingleADC();
				CONTER = 0;
			}
		}

 /*****************************************/


		/* Condicional para el alza de la bandera del Led de estado */
		if (blinkyFlag){
			gpio_TooglePin(&ledState);		// Alterna estado del led
			blinkyFlag = 0;					// Se limpia la bandera del parpadeo del led

		}


		if(fsm.fsmState != STANDBY_STATE){
			state_machine_action();
		}
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
	digitoDecena.pGPIOx								= GPIOA;
	digitoDecena.pinConfig.GPIO_PinNumber			= PIN_5;
	digitoDecena.pinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	digitoDecena.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	digitoDecena.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	digitoDecena.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	gpio_Config(&digitoDecena);

	// Transistor que maneja el digito de las centenas
	digitoCentena.pGPIOx							= GPIOB;
	digitoCentena.pinConfig.GPIO_PinNumber			= PIN_9;
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
	transistorsTimer.pTIMx								= TIM4;
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
	segmentG.pGPIOx							= GPIOC;
	segmentG.pinConfig.GPIO_PinNumber		= PIN_12;
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

	/* config del PWM para el led RGB*/
	handlerPinPwmRgbLed.pGPIOx 						= GPIOC;
	handlerPinPwmRgbLed.pinConfig.GPIO_PinNumber 	= PIN_8;
	handlerPinPwmRgbLed.pinConfig.GPIO_PinMode 		= GPIO_MODE_ALTFN;
	handlerPinPwmRgbLed.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	handlerPinPwmRgbLed.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	handlerPinPwmRgbLed.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_FAST;
	handlerPinPwmRgbLed.pinConfig.GPIO_PinAltFunMode 	= AF2;
	gpio_Config(&handlerPinPwmRgbLed);


	handlerSignalPWMrgb.ptrTIMx = TIM3;
	handlerSignalPWMrgb.config.channel = PWM_CHANNEL_3;
	handlerSignalPWMrgb.config.duttyCicle = 50;
	handlerSignalPWMrgb.config.periodo = 20;
	handlerSignalPWMrgb.config.prescaler = 16;
	pwm_Config(&handlerSignalPWMrgb);




		/*FIN del config del PWM para led RGB*/



	/* config del PWM para la salida del filtro RC*/

	handlerPinPwmBase.pGPIOx = GPIOA;
	handlerPinPwmBase.pinConfig.GPIO_PinNumber = PIN_0;
	handlerPinPwmBase.pinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	handlerPinPwmBase.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	handlerPinPwmBase.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	handlerPinPwmBase.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_FAST;
	handlerPinPwmBase.pinConfig.GPIO_PinAltFunMode = AF2;
	gpio_Config(&handlerPinPwmBase);

	handlerSignalPWMBase.ptrTIMx = TIM5;
	handlerSignalPWMBase.config.channel = PWM_CHANNEL_1;
	handlerSignalPWMBase.config.duttyCicle = 50;
	handlerSignalPWMBase.config.periodo = 1000;		// 1kHz de freq para la señal de reloj 16MHz
	handlerSignalPWMBase.config.prescaler = 16;
	pwm_Config(&handlerSignalPWMBase);

	pwm_Enable_Output(&handlerSignalPWMBase);
	pwm_Start_Signal(&handlerSignalPWMBase);



	handlerPinPwmCollector.pGPIOx = GPIOA;
	handlerPinPwmCollector.pinConfig.GPIO_PinNumber = PIN_1;
	handlerPinPwmCollector.pinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	handlerPinPwmCollector.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	handlerPinPwmCollector.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	handlerPinPwmCollector.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_FAST;
	handlerPinPwmCollector.pinConfig.GPIO_PinAltFunMode = AF2;
	gpio_Config(&handlerPinPwmCollector);

	handlerSignalPWMCollector.ptrTIMx = TIM5;
	handlerSignalPWMCollector.config.channel = PWM_CHANNEL_2;
	handlerSignalPWMCollector.config.duttyCicle = 50;
	handlerSignalPWMCollector.config.periodo = 1000;		// 1kHz de freq para la señal de reloj 16MHz
	handlerSignalPWMCollector.config.prescaler = 16;
	pwm_Config(&handlerSignalPWMCollector);

	pwm_Enable_Output(&handlerSignalPWMCollector);
	pwm_Start_Signal(&handlerSignalPWMCollector);

		/*FIN del config del PWM para salida del filtro RC*/


				/* Configuración para USART6 */
	//  GPIO Rx, Tx config
	usart6Tx.pGPIOx = GPIOC;
	usart6Tx.pinConfig.GPIO_PinNumber = PIN_6;
	usart6Tx.pinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	usart6Tx.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	usart6Tx.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_MEDIUM;
	usart6Tx.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	usart6Tx.pinConfig.GPIO_PinAltFunMode = AF8;
	gpio_Config(&usart6Tx);

	usart6Rx.pGPIOx = GPIOC;
	usart6Rx.pinConfig.GPIO_PinNumber = PIN_7;
	usart6Rx.pinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	usart6Rx.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	usart6Rx.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_MEDIUM;
	usart6Rx.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	usart6Rx.pinConfig.GPIO_PinAltFunMode = AF8;
	gpio_Config(&usart6Rx);

	// USART 6 CONFIG
	hCmdTerminal.ptrUSARTx = USART6;
	hCmdTerminal.USART_Config.baudrate = USART_BAUDRATE_115200;
	hCmdTerminal.USART_Config.datasize = USART_DATASIZE_8BIT;
	hCmdTerminal.USART_Config.parity = USART_PARITY_NONE;
	hCmdTerminal.USART_Config.stopbits = USART_STOPBIT_1;
	hCmdTerminal.USART_Config.mode = USART_MODE_RXTX;
	hCmdTerminal.USART_Config.enableIntRX = USART_RX_INTERRUP_ENABLE;
	hCmdTerminal.USART_Config.enableIntTX = USART_TX_INTERRUP_DISABLE;
	usart_Config(&hCmdTerminal);





	ADC_handler.channel = ADC_CHANNEL_7;
	ADC_handler.resolution = ADC_RESOLUTION_12_BIT;
	ADC_handler.samplingPeriod = ADC_SAMPLING_PERIOD_56_CYCLES;
	ADC_handler.dataAlignment = ADC_ALIGNMENT_RIGHT;


	 	 	 /* Fin de la config del USART6 */

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

void ReceivedChar(void){
	if (hCmdTerminal.receivedChar != '\0'){
		bufferReception[counterReception] = hCmdTerminal.receivedChar;
		counterReception++;

		if (hCmdTerminal.receivedChar =='@'){
			bufferReception[counterReception] = '\0';
			counterReception = 0;

			fsm.fsmState = CMD_COMPLETE;
		}
	}
}

void parseCommands(char *ptrBufferReception){


	sscanf(ptrBufferReception,"%s %u %u %s",cmd,&firstParameter,&secondParameter, userMsg);

	/* */
	if ((strcmp(cmd,"help")) == 0){
		usart_writeMsg(&hCmdTerminal,"Help Menu CMDs. CMD_Structure: cmd # # @\n");
		usart_writeMsg(&hCmdTerminal,"1) help          -- Print this menu\n");
		usart_writeMsg(&hCmdTerminal,"2) dummy #A #B   -- Dummy cmd, #A and #B are uint32_t\n");
		usart_writeMsg(&hCmdTerminal,"3) setDisplay #  -- Change the display value. The max value is 12 bit \n");
		usart_writeMsg(&hCmdTerminal,"4) setPeriod #   -- Change the led_state period (ms)\n");
		usart_writeMsg(&hCmdTerminal,"5) setFreq #A #B -- Select PWM: A=0 -> PWMrgb, A=1 -> PWMrcFilter. Select period: B=pwm period in \n");
		usart_writeMsg(&hCmdTerminal,"6) setDutty #    -- Select PWM: A=0 -> PWMrgb, A=1 -> PWMrcFilter. Select dutty B=dutty cycle from 0 to 100\n");
		usart_writeMsg(&hCmdTerminal,"7) setVoltB #    -- Select the DAC in transistor base a value from 100 mV to 3300 mV. Set period from 20 us in PWMFilter to\n");
		usart_writeMsg(&hCmdTerminal,"8) setVoltc #    -- Select the DAC in transistor collector a value from 100 mV to 3300 mV. Set period from 20 us in PWMFilter to\n");
		usart_writeMsg(&hCmdTerminal,"9) readVoltB     -- Read the voltage on Base emitter after a setVolt in Base\n");
		usart_writeMsg(&hCmdTerminal,"10) readVoltC    -- Read the voltage on Collector emitter after an appropriate setVolt on Base and Collector to\n");
		usart_writeMsg(&hCmdTerminal,"11) IC-VCE #     -- Create a Ic vs Vce table and select the base voltage around 200 mV . Set period from 20 us in PWMFilter to\n");
		usart_writeMsg(&hCmdTerminal,"12) IB-VBE #     -- Create a Ib vs Vbe table and select the collector voltage around from 100mV to 3300 mV. Set period from 20 us in PWMFilter to\n");
	}

	/* Command dummy*/
	else if(strcmp(cmd,"dummy") == 0){
		usart_writeMsg(&hCmdTerminal,"CMD: dummy\n");
		sprintf(bufferData,"number A = %u \n",firstParameter);
		usart_writeMsg(&hCmdTerminal,bufferData);

		sprintf(bufferData,"number B = %u \n", secondParameter);
		usart_writeMsg(&hCmdTerminal, bufferData);
	}



	/* setDisplay command to change the display valuu*/
	else if(strcmp(cmd,"setDisplay") == 0){
		usart_writeMsg(&hCmdTerminal,"CMD: setDisplay\n");

		// Se garantiza que el valor ingresado sea menor a 12 bits
		if ((firstParameter<= MAX_12_BITS) & (firstParameter>=0)){
			sprintf(bufferData,"Display updated to: %u \n",firstParameter);
			rotationCounter = firstParameter; 			// se actualiza el valor global para mostrar en display
			usart_writeMsg(&hCmdTerminal,bufferData);
		}
		// se imprime una instrucción para el usuario
		else{
			sprintf(bufferData,"Insert a 12 bits value\n");
			usart_writeMsg(&hCmdTerminal,bufferData);
		}
	}

	/*This elseif modifies the period of the blinky led */
	else if(strcmp(cmd,"setPeriod") == 0){

		usart_writeMsg(&hCmdTerminal,"CMD: setPeriod\n");

		// se garantiza que el valor de ms ingresado este entre 100 y 1500
		if((firstParameter>=100) & (firstParameter<=1500) ){

			// se apaga el timer y se reconfigura
			timer_SetState(&blinkyTimer, TIMER_OFF);
			blinkyTimer.TIMx_Config.TIMx_Period	= firstParameter;
			timer_Config(&blinkyTimer);
			timer_SetState(&blinkyTimer, TIMER_ON);
			sprintf(bufferData,"Blinky period updated: %u \n",firstParameter);
			usart_writeMsg(&hCmdTerminal,bufferData);
		}
		// se da una instruccion al usuario
		else{
			sprintf(bufferData,"Insert a value between 100 and 1500 ms\n");
			usart_writeMsg(&hCmdTerminal,bufferData);
		}
	}

	/* this elseif modifies the frequency of the PWMs*/
	else if(strcmp(cmd,"setFreq") == 0){
		usart_writeMsg(&hCmdTerminal,"CMD: setFreq\n");

		// A = 0 corresponde al PWM del led RGB, led azul
		if (firstParameter == 0){
			// se ponen los otros pines en bajo por si se tiene un estado del led RGB
			gpio_WritePin(&ledRed, RESET);
			gpio_WritePin(&ledGreen, RESET);

			// se cambia la config para modo PWM del pin
			gpio_Config(&handlerPinPwmRgbLed);

			// se enciende y configura el PWM  para los led RGB
			pwm_Enable_Output(&handlerSignalPWMrgb);
			pwm_Start_Signal(&handlerSignalPWMrgb);
			pwm_Update_Frequency(&handlerSignalPWMrgb, secondParameter);
			pwm_Config(&handlerSignalPWMrgb);


			sprintf(bufferData,"Periodo (um) PWM led rgb: %u\n",secondParameter);
			usart_writeMsg(&hCmdTerminal,bufferData);

		}
		 // A = 1 corresponde al PWM del ledRGB, led azul
		else if(firstParameter == 1){
			pwm_Update_Frequency(&handlerSignalPWMBase, secondParameter);
			pwm_Config(&handlerSignalPWMBase);
			sprintf(bufferData,"Periodo (um) del PWM filtro RC: %u\n",secondParameter);
			usart_writeMsg(&hCmdTerminal,bufferData);
		}
	}

	/* This elseif modifies the dutty cycle of the PWMs*/
	else if(strcmp(cmd,"setDutty") == 0){
		usart_writeMsg(&hCmdTerminal,"CMD: setDutty\n");

		// A = 0 corresponde al PWM del led RGB, led azul
		if (firstParameter == 0){

			// se ponen los otros pines en bajo por si se tiene un estado del led RGB
			gpio_WritePin(&ledRed, RESET);
			gpio_WritePin(&ledGreen, RESET);
			gpio_Config(&handlerPinPwmRgbLed);
			pwm_Enable_Output(&handlerSignalPWMrgb);
			pwm_Start_Signal(&handlerSignalPWMrgb);
			pwm_Update_DuttyCycle(&handlerSignalPWMrgb, secondParameter);
			sprintf(bufferData,"Modificación de dutty cycle del led rgb percentage: %u\n",secondParameter);
			usart_writeMsg(&hCmdTerminal,bufferData);
		}

		 // A = 1 corresponde al PWM del ledRGB, led azul
		else if(firstParameter == 1){

			gpio_Config(&handlerPinPwmRgbLed);
			pwm_Enable_Output(&handlerSignalPWMrgb);
			pwm_Start_Signal(&handlerSignalPWMrgb);
			pwm_Update_DuttyCycle(&handlerSignalPWMBase, secondParameter);
			sprintf(bufferData,"Modificación de dutty cycle del filtro RC percentage: %u\n",secondParameter);
			usart_writeMsg(&hCmdTerminal,bufferData);

		}
	}
	/*This modifies the voltaje of the output in the RC filter */
	else if(strcmp(cmd,"setVoltB") == 0){
		if ((firstParameter>=1) & (firstParameter<=3300)){
		pwm_Update_DuttyCycle(&handlerSignalPWMBase, firstParameter*100/3300);
		sprintf(bufferData,"Voltaje actual: %u mV \n",firstParameter);
		usart_writeMsg(&hCmdTerminal,bufferData);
		}
		else{
		sprintf(bufferData,"Inserte un valor de voltaje entre 1 mV y 3300 mV");
		usart_writeMsg(&hCmdTerminal,bufferData);
		}
	}

	/*This modifies the voltaje of the output in the RC filter */
	else if(strcmp(cmd,"setVoltC") == 0){
		if ((firstParameter>=1) & (firstParameter<=3300)){
		pwm_Update_DuttyCycle(&handlerSignalPWMCollector, firstParameter*100/3300);
		sprintf(bufferData,"Voltaje actual: %u mV \n",firstParameter);
		usart_writeMsg(&hCmdTerminal,bufferData);
		}
		else{
		sprintf(bufferData,"Inserte un valor de voltaje entre 1 mV y 3300 mV");
		usart_writeMsg(&hCmdTerminal,bufferData);
		}
	}




	/*This read the voltaje on the transistor base */
	else if(strcmp(cmd,"readVoltB") == 0){

		ADC_handler.channel = ADC_CHANNEL_7; // base
		adc_Config(&ADC_handler);
		startSingleADC();

		while (!(ADC1->SR & ADC_SR_EOC));
		Vbb = ADC_handler.adcData*3300/4095;
		baseCurrent = ((handlerSignalPWMBase.config.duttyCicle)*3300/100 - Vbb)/R_base;

		ADC_handler.channel = ADC_CHANNEL_14; // colector
		adc_Config(&ADC_handler);
		startSingleADC();
		while (!(ADC1->SR & ADC_SR_EOC));

		collectorCurrent = ((ADC_handler.adcData)*3300/4095 - (handlerSignalPWMCollector.config.duttyCicle)*3300/100)/R_colector;
		baseVoltaje = ( Vbb - collectorCurrent*R_emisor );

		sprintf(bufferData,"Voltaje leido en BE: %u mV \n",baseVoltaje);
		usart_writeMsg(&hCmdTerminal,bufferData);

	}

	/*This read the voltaje on the transistor collector */
	else if(strcmp(cmd,"readVoltC") == 0){

		ADC_handler.channel = ADC_CHANNEL_14; // colector
		adc_Config(&ADC_handler);
		startSingleADC();
		while (!(ADC1->SR & ADC_SR_EOC));
		collector_average = (ADC_handler.adcData )*3300/4095;
		collectorCurrent = (((handlerSignalPWMCollector.config.duttyCicle)*3300.0/100.0) - collector_average)/R_colector;

		VCE = collector_average - collectorCurrent*R_emisor;
		sprintf(bufferData,"Voltaje leído en el CE: %.2f mV \n",VCE);
		usart_writeMsg(&hCmdTerminal,bufferData);


	}

	else if (strcmp(cmd,"IC-VCE") == 0){

		// limpiamos el array por si hubo datos de la otra grafica

		for (uint16_t i = 0; i < 100; i++) {
			data_collector[i] = 0;
		}

		counterproof = 0;
		counterproof2 = 1;
		pwm_Update_DuttyCycle(&handlerSignalPWMBase, firstParameter*100/3300);
		pwm_Update_DuttyCycle(&handlerSignalPWMCollector, initial_dutty_collector);

		sprintf(bufferData,"Tabla IC-VCE generandose. Voltaje base seleccionado %u\n",firstParameter);
		usart_writeMsg(&hCmdTerminal,bufferData);
		ADC_handler.channel = ADC_CHANNEL_14;
		adc_Config(&ADC_handler);
		startSingleADC();
	}





	/* Para esta curva es necesario setear un valor de voltaje en BE*/
	// 					IB-VBE #A #B, donde #A es el voltaje de colector con el que se generan la curva.
	else if (strcmp(cmd,"IB-VBE") == 0){
		bufferCounter = 0;
		for (uint16_t i = 0; i < 100; i++) {
			data_collector[i] = 0;
		}
		counterproof2 = 0;
		counterproof = 1;
		pwm_Update_DuttyCycle(&handlerSignalPWMCollector, firstParameter*100/3300);
		pwm_Update_DuttyCycle(&handlerSignalPWMBase,duttyBase);
		Vcc = firstParameter;

		adc_Config(&ADC_handler);
		startSingleADC();

		sprintf(bufferData,"Tabla IB-VBE generandose \n");
		usart_writeMsg(&hCmdTerminal,bufferData);
	}


	/*The inserted msg is not in the list*/
	else{
		usart_writeMsg(&hCmdTerminal,"Wrong CMD\n");
	}

}

float average(float *databuffer){
	float Average = 0;
	uint8_t counter = 0;
	for (uint16_t i = 0; i<100; i++){
		Average += databuffer[i];
		counter++;
	}
	Average = Average/counter;
	return Average;
}



/* Función de la Finite State Machine  */
void state_machine_action(void){

	uint32_t currentTime = 0; 		// variable local que almacena el tiempo desde que inicia el código

	/* Switch case que evalua cada uno los estados de la FSM */
	switch (fsm.fsmState){

	case SW_BUTTON_STATE:
		pwm_Disable_Output(&handlerSignalPWMrgb);
		pwm_Stop_Signal(&handlerSignalPWMrgb);
		gpio_Config(&ledBlue);
		currentTime = ticksNumber();
		printf("Current time: %lu \n",currentTime);
		// Se guarda el valor de los ticks
		fsm_rgb_modeSelection();							// Se cambia el estado del Led RGB

		break;

	case DISPLAY_VALUE_STATE:

		disableTransistors();		         	 // Se apagan los transistores

		/* Condicional para el alza de la bandera dada por el extiCLK */
		if (fsm_rotation.rotationState == ROTATION_STATE){
			// Actualiza rotationCounter para mostrar en el display
			fsm_rotation_handler();
			fsm_rotation.rotationState = NO_ROTATION;	// Se actualiza la fsmRotation
		}
		fsm_display_handler(); 			    	 // Función que enciende los segmentos y el transistor
//		voltage_base_collector_sampling();
		break;

	case CHAR_RECEIVED_STATE:
		ReceivedChar();
		if(fsm.fsmState == CMD_COMPLETE){
			parseCommands(bufferReception);
			for (uint8_t i = 0; i < sizeof(bufferReception); i++){
				bufferReception[i] = 0;
			}
		}
		break;



//	case ADC_COMPLETE:
//
//		break;



	default:
		fsm.fsmState = STANDBY_STATE;			// Estado de espera
		break;
	}
	fsm.fsmState = STANDBY_STATE;
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



/* Callback del blinkytimer alterna el estado del ledState */
void Timer2_Callback(void){
	blinkyFlag = 1;				// Se sube la bandera al led de estado

}

/* Callback del timer que enciende y apaga los transistores */
void Timer4_Callback(void){
	fsm.fsmState = DISPLAY_VALUE_STATE;				// Se actualiza el estado para la fsm

}

/* Callback de la interrupcion del pin B2 que corresponde al Clk */
void callback_ExtInt2(void){
	fsm_rotation.rotationState = ROTATION_STATE;	// Se actualiza el estado para la fsm
	// Se lee el valor del data y clock para determinar el giro en sentido CW o CCW
	data = gpio_ReadPin(&userData);
	clock = gpio_ReadPin(&userClock);
}

/* Callback de la interrupcion del Switch SW del encoder que controla el Led RGB */
void callback_ExtInt15(void){
	fsm.fsmState = SW_BUTTON_STATE;				 // Se define el estado para la fsm

}

void usart6_RxCallback(void){
	rxData = usart_getRxData(&hCmdTerminal);
	fsm.fsmState = CHAR_RECEIVED_STATE;
}

void adcComplete_Callback(void){
	CONTER = 1;
	ADC_handler.adcData = getADC();
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
