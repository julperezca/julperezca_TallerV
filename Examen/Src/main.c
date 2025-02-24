/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Julián Pérez Carvajal (julperezca@unal.edu.co)
 * @brief          : Examen. LCD Hitachi-> commands -> RTC + Clock Freq.
 ******************************************************************************
 */

#include <string.h>
#include <main.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include "stm32f4xx.h"
#include "stm32_assert.h"
#include "exti_driver_hal.h"
#include "timer_driver_hal.h"
#include "systick_driver.h"
#include "pwm_driver_hal.h"
#include "usart_driver_hal.h"
#include "AdcDriver.h"
#include "pll_driver_hal.h"
#include "i2c_driver_hal.h"
#include "rtc_driver_hal.h"
#include "LCD_44780_driver.h"

#define ADC_BUFFER_SIZE 100 		 // Tamaño del buffer

	/* GPIO handler y TIMER para el led de estado */
GPIO_Handler_t  ledState    	= {0}; 		// PinH1
Timer_Handler_t blinkyTimer		= {0}; 		// TIM2 Led de estado

	/* GPIO handler para led RGB*/
GPIO_Handler_t ledRed 			= {0}; 		// PinA7
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
GPIO_Handler_t digitoCentena 		= {0}; 		// PinA6
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
GPIO_Handler_t handlerPinPwmRgbLed   = {0};			// Pin C8
GPIO_Handler_t handlerPinPwmRCfilter = {0};			// Pin A1

	/* PWM Handler para la señal PWM: timer y canal*/
PWM_Handler_t handlerSignalPWMrgb  	 = {0};			// Timer 3, canal 3
PWM_Handler_t handlerSignalPWMfilter = {0};			// Timer 5, canal 2

	/* Handler para usart6*/
USART_Handler_t hCmdTerminal 		 = {0}; 		// USART6
GPIO_Handler_t usart6Tx 			 = {0};			// Tx  pin C6
GPIO_Handler_t usart6Rx				 = {0};			// Rx  pin C7

/* Finite State Machine + subestados del led RGB, de los transistores y de los segmentos */
fsm_t fsm = {0};
fsm_RGB_t fsm_RGB 					= {0};
fsm_transistor_t fsm_transistor 	= {0};
fsm_segments_t fsm_segments 		= {0};
fsm_rotation_t fsm_rotation 		= {0};
fsm_rtc_t 	   fsm_rtc  			= {0};
fsm_adc_t 	   fsm_adc 				= {0};

	/* RTC  handler y buffer de tiempo y date*/
RTC_Handler_t rtc_handler = {0};
uint8_t dateBuffer[3] = {0}; 	// buffer de tres elementos que guarda: año, mes, día
uint8_t timeBuffer[3] = {0};	// buffer de tres ekementos que guarda: hora, minutos, segundos

/* I2C handler y pines de GPIO para la pantalla LCD*/
I2C_Handler_t i2cLCD_handler ={0};
GPIO_Handler_t pinSCL = {0};
GPIO_Handler_t pinSDA = {0};



/* Estructura de configuración del ADC */
ADC_Config_t ADC_handler = {0};   	 // PA0

float adcBuffer[ADC_BUFFER_SIZE]; 	 // Buffer de almacenamiento
uint8_t bufferIndex = 0;  			 // Índice del buffer
float voltaje_adc_value = 0;		 // Valor de voltaje promediado ADC



/* Variables globales */
uint8_t data 			 = 0;		// Variable que almacena el estado del DT del encoder
uint8_t clock			 = 0;		// Variable que almacena el estado el CLK del encoder
uint16_t rotationCounter = 0;		// Variable que es mostrada en el display (giros del encoder)
uint8_t blinkyFlag 		 = 0;		// Flag para el parpadeo del led
uint16_t blinkyPeriod = 0;			// blinky del led de estado
uint8_t cursorx  = 0;				// valor del cursor en x(columna)
uint8_t cursory  = 0;				// valor de cursor en y(fila)


/*analisis de USART*/
uint8_t rxData = 0;
char bufferReception[BUFFER_SIZE];
uint8_t counterReception;

/*buffer para parsecommands*/
char cmd[16];
char write[80];
char userMsg[BUFFER_SIZE] = {0};
char endCommand[BUFFER_SIZE] = {0};
char bufferData[BUFFER_SIZE] = {0};
char clearBuffer[16] = {0};

/*parametros para el parsecommands*/
unsigned int  firstParameter;
unsigned int  secondParameter;
unsigned int  thirdParameter;
unsigned int  fourthParameter;
unsigned int  fifthParameter;
unsigned int  sixthParameter;
unsigned int sevenParameter;

/* Declaración o prototipo de funciones */
extern void configMagic(void);  				// Config del Magic para comunicación serial
void fsm_rgb_modeSelection(void);				// Función que selecciona el color del led RGB
void init_config(void);							// Función que inicia la config. de los pines, timers y EXTI
void numberSelection(uint8_t displayNumber);	// Función que selecciona los segmentos encendidos, ingresa el valor a mostrar
void fsm_rotation_handler(void); 				// Función encargada del sentido de rotation y el valor de la misma
void disableTransistors(void);					// Función encargada de apagar los transistores para evitar el "fantasma"
void fsm_display_handler(void);					// Función encargada de manejar el los transistores y cada segmento
void state_machine_action(void);				// Maquina de estados en accción
void array_to_string_hour(uint8_t array[3], char *str);
void array_to_string_date(uint8_t array[3], char *str);
/*
 * The main function, where everything happens.
 */
int main (void){
	configMagic();  		   // Se inicia la configuracion de Magic
	init_config();			   // Se inicia la configuracion del sistema

	/*buffer para limpieza de terminal*/
	clearBuffer[0] = 0x1B;
	clearBuffer[1] = 0x5B;
	clearBuffer[2] = 0x32;
	clearBuffer[3] = 0x4A;

	/* Se limpia la terminal*/
	usart_writeMsg(&hCmdTerminal,clearBuffer);
	usart_writeMsg(&hCmdTerminal,"Escriba help @ para desplegar el manual de comandos a utilizar\n");

	/* Loop infinito */
	while(1){

		/* Condicional para el alza de la bandera del Led de estado */
		if (blinkyFlag){
			gpio_TooglePin(&ledState);				// Alterna estado del led
			blinkyFlag = 0;							// Se limpia la bandera del parpadeo del led
		}

		if(fsm.fsmState != STANDBY_STATE){
			state_machine_action();
		}
	}
	return 0;
}

/*initial func configs*/
void led_state_config(void){
	/* Configuración de LED de estado y su respectivo timer */

	// GPIO config para Led de estado
	ledState.pGPIOx							= GPIOH;
	ledState.pinConfig.GPIO_PinNumber		= PIN_1;
	ledState.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	ledState.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	ledState.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	ledState.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&ledState);
	// Inicialmente led de estado está encendido
	gpio_WritePin(&ledState, SET);

	// Config para el timer del led de estado
	blinkyTimer.pTIMx								= TIM2;
	blinkyTimer.TIMx_Config.TIMx_Prescaler  		= 10000; //100us conversion
	blinkyTimer.TIMx_Config.TIMx_Period				= 2500;  // 250ms
	blinkyTimer.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;
	blinkyTimer.TIMx_Config.TIMx_InterruptEnable 	= TIMER_INT_ENABLE;
	timer_Config(&blinkyTimer);
	timer_SetState(&blinkyTimer, TIMER_ON);

		/* FIN de configuración de Led de estado y su timer */
}

void config_rgb_segmentos(void){

	/* Se configuran los pines para el led RGB */

	//Led rojo
	ledRed.pGPIOx							= GPIOA;
	ledRed.pinConfig.GPIO_PinNumber			= PIN_7;
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
	digitoCentena.pGPIOx							= GPIOA;
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
	transistorsTimer.pTIMx								= TIM4;
	transistorsTimer.TIMx_Config.TIMx_Prescaler  		= 50000; //500us conversion
	transistorsTimer.TIMx_Config.TIMx_Period			= 5;	// si se coloca 5->> 2.5ms
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

	// Inicia con los digitos apagados
	gpio_WritePin(&digitoUnidad, OFF);
	gpio_WritePin(&digitoDecena, OFF);
	gpio_WritePin(&digitoCentena, OFF);
	gpio_WritePin(&digitoUnMillar, OFF);

		/* FIN de la configuración de los segmentos */

}

void usart_init_config(void){
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
	hCmdTerminal.USART_Config.baudrate = USART_BAUDRATE_57600;
	hCmdTerminal.USART_Config.datasize = USART_DATASIZE_8BIT;
	hCmdTerminal.USART_Config.parity = USART_PARITY_ODD;
	hCmdTerminal.USART_Config.stopbits = USART_STOPBIT_1;
	hCmdTerminal.USART_Config.mode = USART_MODE_RXTX;
	hCmdTerminal.USART_Config.enableIntRX = USART_RX_INTERRUP_ENABLE;
	hCmdTerminal.USART_Config.enableIntTX = USART_TX_INTERRUP_DISABLE;
	usart_Config(&hCmdTerminal);

		 /* Fin de la config del USART6 */
}

void exti_encoder_config(){
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
}

void pwm_initial_config(){

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
	handlerSignalPWMrgb.config.periodo = 10;   // conversión a 1ms
	handlerSignalPWMrgb.config.prescaler = 10000;
	pwm_Config(&handlerSignalPWMrgb);

	/*FIN del config del PWM para led RGB*/

	/* COnfig Pin PWM RC filter*/
	handlerPinPwmRCfilter.pGPIOx = GPIOA;
	handlerPinPwmRCfilter.pinConfig.GPIO_PinNumber = PIN_1;
	handlerPinPwmRCfilter.pinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	handlerPinPwmRCfilter.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	handlerPinPwmRCfilter.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	handlerPinPwmRCfilter.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_FAST;
	handlerPinPwmRCfilter.pinConfig.GPIO_PinAltFunMode = AF2;
	gpio_Config(&handlerPinPwmRCfilter);

	handlerSignalPWMfilter.ptrTIMx = TIM5;
	handlerSignalPWMfilter.config.channel = PWM_CHANNEL_2;
	handlerSignalPWMfilter.config.duttyCicle = 50;
	handlerSignalPWMfilter.config.periodo = 10;		// 1kHz de freq para la señal de reloj 16MHz
	handlerSignalPWMfilter.config.prescaler = 10000;
	pwm_Config(&handlerSignalPWMfilter);

	pwm_Enable_Output(&handlerSignalPWMfilter);
	pwm_Start_Signal(&handlerSignalPWMfilter);

		/*FIN del config del PWM para salida del filtro RC*/
}

/*RTC initial config */
void rtc_Config(void){

	rtc_handler.day = 24;
	rtc_handler.month = 2;
	rtc_handler.year = 25;
	rtc_handler.hour = 10;
	rtc_handler.minutes = 0;
	rtc_handler.seconds = 0;
	rtc_handler.formato = FORMAT_24H;
	RTC_config(&rtc_handler);
}

/* I2C config handler*/
void i2c_config(void){
    pinSCL.pGPIOx = GPIOB;
    pinSCL.pinConfig.GPIO_PinNumber = PIN_8;
    pinSCL.pinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    pinSCL.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_OPENDRAIN;
    pinSCL.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_FAST;
    pinSCL.pinConfig.GPIO_PinAltFunMode = AF4;
    gpio_Config(&pinSCL);

    pinSDA.pGPIOx = GPIOB;
    pinSDA.pinConfig.GPIO_PinNumber = PIN_9;
    pinSDA.pinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    pinSDA.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_OPENDRAIN;
    pinSDA.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_FAST;
    pinSDA.pinConfig.GPIO_PinAltFunMode = AF4;
    gpio_Config(&pinSDA);

    i2cLCD_handler.pI2Cx = I2C1;
    i2cLCD_handler.i2c_mainClock = I2C_MAIN_CLOCK_16_MHz;
    i2cLCD_handler.i2c_mode = eI2C_MODE_SM;
    i2cLCD_handler.slaveAddress = LCD_I2C_ADDR;
    i2c_Config(&i2cLCD_handler);
}


// Inicialización del ADC en el pin A0 (canal0)
void initADC_C0(void) {
    ADC_handler.channel = ADC_CHANNEL_0;
    ADC_handler.resolution = ADC_RESOLUTION_12_BIT;
    ADC_handler.samplingPeriod = ADC_SAMPLING_PERIOD_112_CYCLES;
    ADC_handler.dataAlignment = ADC_ALIGNMENT_RIGHT;
    adc_Config(&ADC_handler);
}

/* Funcion encargada de la configuración del GPIO, TIMERS y EXTIs */
void init_config(void){
	/*Configuración del sistema con la señal de 100MHz*/
	pll_Config_100MHz();

	/*configuración del MC01 para PLL inicialmente*/
	signal_selection_MC01(MC01_PLL_CHANNEL,PRESCALER_DIV_5);
	/*Configuración del led de estado o blinky*/
	led_state_config();


	/* Configuración de pines de led rgb y 7 segmentos + display timer*/
	config_rgb_segmentos();


	/* Configuración de los pines GPIO y exti para el encoder*/
	exti_encoder_config();


	/* Configuración de timer y pines para PWM*/
	pwm_initial_config();


	/*Se configura el USART 6*/
	usart_init_config();


	/* Se configura el SysTick con la señal de reloj de 100MHz	*/
	systickConfig(CLOCK_SOURCE_100MHz);


	/*RTC initialization*/
	rtc_Config();


	/*I2C initialization for LCD screen*/
	i2c_config();


	/*HITACHI-Configuración inicial de la pantalla*/
	LCD_Init(&i2cLCD_handler);

	// Se muestra la inicialización
	LCD_writeString(&i2cLCD_handler, "=== SISTEMA ===", 3, 1);
	LCD_writeString(&i2cLCD_handler, "CARGADO", 7, 2);


	/*config chanel 0 ADC  pin A0*/
	initADC_C0();
	startSingleADC();

}

/*average buffer adc*/
float average(float *databuffer){
	float Average = 0;
	uint8_t counter = 0;
	for (uint8_t i = 0; i<100; i++){
		Average += databuffer[i];
		counter++;
	}
	Average = Average/counter;
	return Average;
}

/*función para hacer sampling del adc*/
void adc_samplin(void){
	if(fsm_adc.adcState == ADC_RDY){
		adcBuffer[bufferIndex]= (ADC_handler.adcData)*3300.0/4095;
		bufferIndex++;
		if(bufferIndex>=ADC_BUFFER_SIZE){
			bufferIndex = 0;
			voltaje_adc_value = average(adcBuffer);
		}
		startSingleADC();
	}
	fsm_adc.adcState = ADC_NRDY;
}

/*recepción de carácter*/
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

/* analisis de comando*/
void parseCommands(char *ptrBufferReception){

	/***Analisis para determinar escritura en LCD**/
	sscanf(ptrBufferReception, "%s %s %u %u %u %u %u %u %s",
	       cmd, write, &firstParameter, &secondParameter, &thirdParameter,
	       &fourthParameter, &fifthParameter, &sixthParameter, userMsg);
	if (write[0] >= '0' && write[0] <= '9'){
		sscanf(ptrBufferReception, "%s %u %u %u %u %u %u %s",
		   cmd, &firstParameter, &secondParameter, &thirdParameter,
		   &fourthParameter, &fifthParameter, &sixthParameter, userMsg);
	}
	/***FIN De analisis***/


	if ((strcmp(cmd,"help")) == 0){
		usart_writeMsg(&hCmdTerminal,"Help Menu CMDs. CMD_Structure: cmd str # # # # # # @\n");
		usart_writeMsg(&hCmdTerminal,"1)  help            -- Print this menu\n");
		usart_writeMsg(&hCmdTerminal,"2)  dummy #A #B     -- Dummy cmd, #A and #B are uint32_t\n");
		usart_writeMsg(&hCmdTerminal,"3)  setDisplay #    -- Change the display value. The max value is 12 bit \n");
		usart_writeMsg(&hCmdTerminal,"4)  setPeriod #     -- Change the led_state period (ms)\n");
		usart_writeMsg(&hCmdTerminal,"5)  setDutty #A	  -- A=dutty cycle from 0 to 100 in led blue RGB.\n");
		usart_writeMsg(&hCmdTerminal,"6)  setVolt #   	  -- PWM-DAC output in mV from 100 mV to 3300 mV.\n");
		usart_writeMsg(&hCmdTerminal,"7)  readVolt #   	  -- ADC reading the PWM-DAC mV in the Hitachi LCD screen.\n");
		usart_writeMsg(&hCmdTerminal,"8)  lcdClear #      -- Clear the screen and set the cursor in pos (0,0)\n");
		usart_writeMsg(&hCmdTerminal,"9)  blinkCursor #A  -- Set the blink(A=1) or not blinky(A=0) in cursor screen. \n");
		usart_writeMsg(&hCmdTerminal,"10) clearRow   #A   -- Clear the row A=0,1,2,3\n");
		usart_writeMsg(&hCmdTerminal,"11) cursorPos #A #B -- Set the cursor position (A=row=0,1,2,3,col=0,1,..,19)\n");
		usart_writeMsg(&hCmdTerminal,"12) writeLCD (str)  -- Write in LCD in the cursor position(instead of using space use hyphen).\n");
		usart_writeMsg(&hCmdTerminal,"13) date_hour       -- Shows the hour and date.\n");
		usart_writeMsg(&hCmdTerminal,"14) setRTC          -- #d #m #y #h #min #sec set the RTC values (y>=2000).\n");
		usart_writeMsg(&hCmdTerminal,"15) date_hour       -- Shows the hour and date\n");
		usart_writeMsg(&hCmdTerminal,"16) hsiClock   #A   -- 16MHz HSI clock in MC01. Where A=1,2,3,4,5 the prescaler.\n");
		usart_writeMsg(&hCmdTerminal,"17) lseClock   #A   -- 32kHz LSE clock in MC01. Where A=1,2,3,4,5 the prescaler.\n");
		usart_writeMsg(&hCmdTerminal,"18) pllClock   #A   -- 100MHz PLL clock in MC01. Where A=1,2,3,4,5 the prescaler.\n");

	}

	/* 1) Command dummy*/
	else if(strcmp(cmd,"dummy") == 0){
		usart_writeMsg(&hCmdTerminal,"CMD: dummy\n");
		sprintf(bufferData,"number A = %u \n",firstParameter);
		usart_writeMsg(&hCmdTerminal,bufferData);

		sprintf(bufferData,"number B = %u \n", secondParameter);
		usart_writeMsg(&hCmdTerminal, bufferData);
	}



	/* 3) setDisplay command to change the display valuu*/
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

	/* 4)here the period of the blinky led is modified */
	else if(strcmp(cmd,"setPeriod") == 0){

		usart_writeMsg(&hCmdTerminal,"CMD: setPeriod\n");

		// se garantiza que el valor de ms ingresado este entre 100 y 1500
		if((firstParameter>=100) & (firstParameter<=1500) ){

			// se apaga el timer y se reconfigura
			timer_SetState(&blinkyTimer, TIMER_OFF);
			blinkyTimer.TIMx_Config.TIMx_Period		= firstParameter*10;  // 250ms

			timer_Config(&blinkyTimer);
			timer_SetState(&blinkyTimer, TIMER_ON);
			sprintf(bufferData,"Blinky period updated: %u ms\n",firstParameter);
			usart_writeMsg(&hCmdTerminal,bufferData);
		}
		// se da una instruccion al usuario
		else{
			sprintf(bufferData,"Insert a value between 100 and 1500 ms\n");
			usart_writeMsg(&hCmdTerminal,bufferData);
		}
	}


	/*5)*/

	else if(strcmp(cmd,"setDutty") == 0){
		usart_writeMsg(&hCmdTerminal,"CMD: setDutty\n");

		// se ponen los otros pines en bajo por si se tiene un estado del led RGB
		gpio_WritePin(&ledRed, RESET);
		gpio_WritePin(&ledGreen, RESET);
		gpio_Config(&handlerPinPwmRgbLed);
		pwm_Enable_Output(&handlerSignalPWMrgb);
		pwm_Start_Signal(&handlerSignalPWMrgb);
		pwm_Update_DuttyCycle(&handlerSignalPWMrgb, firstParameter);
		sprintf(bufferData,"Modificación de dutty cycle del led rgb percentage: %u\n",firstParameter);
		usart_writeMsg(&hCmdTerminal,bufferData);
	}


	/* 6) This modifies the voltaje of the output in the RC filter */
	else if(strcmp(cmd,"setVolt") == 0){
		if ((firstParameter>=500) & (firstParameter<=3300)){
		pwm_Update_DuttyCycle(&handlerSignalPWMfilter, (uint16_t)firstParameter*100/3300);
		sprintf(bufferData,"Voltaje actual: %u mV \n",firstParameter);
		usart_writeMsg(&hCmdTerminal,bufferData);
		}
		else{
		sprintf(bufferData,"Inserte un valor de voltaje entre 500 mV y 3300 mV");
		usart_writeMsg(&hCmdTerminal,bufferData);
		}
	}

	/* 7) This read the output voltage in the RC filter */
	else if(strcmp(cmd,"readVolt") == 0){
		clean_display_lcd(&i2cLCD_handler);
		char buffer_voltage[40];
		sprintf(buffer_voltage,"ADC volt: %.2f mV",voltaje_adc_value);
		LCD_writeString(&i2cLCD_handler, "ADC COMPLETE", 4, 0);
		LCD_writeString(&i2cLCD_handler, buffer_voltage, 0, 2);
		sprintf(bufferData,"ADC voltage: %.2f mV\n",voltaje_adc_value);
		usart_writeMsg(&hCmdTerminal,bufferData);
	}



	/* 7) Clear de hitachi LCD*/
	else if(strcmp(cmd,"lcdClear") == 0){
		clean_display_lcd(&i2cLCD_handler);
		sprintf(bufferData,"lcdCleared\n");
		usart_writeMsg(&hCmdTerminal,bufferData);
	}

	/* 8) Setea el blink cursor*/
	else if(strcmp(cmd,"blinkCursor") == 0){
		LCD_cursor_blinky(&i2cLCD_handler, firstParameter);
		sprintf(bufferData,"blinkCursor\n");
		usart_writeMsg(&hCmdTerminal,bufferData);
	}

	/* 9) Clean de row A=0,1,2,3*/
	else if(strcmp(cmd,"clearRow") == 0){
		clean_row(&i2cLCD_handler, firstParameter);
		sprintf(bufferData,"rowCleared: %u\n",firstParameter);
		usart_writeMsg(&hCmdTerminal,bufferData);
	}

	/* 10) Select the position of the cursor*/
	else if(strcmp(cmd,"cursorPos") == 0){
		cursorx = secondParameter;
		cursory = firstParameter;
		LCD_setCursor(&i2cLCD_handler, firstParameter, secondParameter);
		LCD_cursor_blinky(&i2cLCD_handler, 1);
		sprintf(bufferData,"Cursor position: %u,%u\n",firstParameter,secondParameter);
		usart_writeMsg(&hCmdTerminal,bufferData);
	}

	/* 11) write a message in the hitachi LCD*/
	else if(strcmp(cmd, "writeLCD") == 0) {
		clean_display_lcd(&i2cLCD_handler);
	    LCD_writeString(&i2cLCD_handler, write,cursorx,cursory);  // Escribe en LCD
	    snprintf(bufferData, sizeof(bufferData), "LCD Message: %.48s\n", write);
	    usart_writeMsg(&hCmdTerminal, bufferData);
	}


	/* 12)* 	Set the cursor position A=row=0,1,2,3,col=0,1,..,19	*/
	else if(strcmp(cmd,"setRTC") == 0){
		rtc_handler.day = firstParameter;
		rtc_handler.month = secondParameter;
		rtc_handler.year = thirdParameter;
		rtc_handler.hour = fourthParameter;
		rtc_handler.minutes = fifthParameter;
		rtc_handler.seconds = sixthParameter;
		rtc_handler.formato = FORMAT_24H;
		RTC_config(&rtc_handler);

		sprintf(bufferData,"Date and hour updated\n");
		usart_writeMsg(&hCmdTerminal,bufferData);
	}

	/* 13) ut the date and hour in the screen LCD hitachi*/
	else if(strcmp(cmd,"date_hour") == 0){
		/* dos ideas:
		 * 1) se tiene la lectura del RTC en el while
		 * de manera que se muestre o imprima la hora en la LCD en refresh display
		 * crear un nuevo estado el cual indique si se quiere o no
		 * mostrar la hora y fecha en el display
		 *
		 * */
		clean_display_lcd(&i2cLCD_handler);
		LCD_writeString(&i2cLCD_handler, "Hour-->", 0, 0);
		LCD_writeString(&i2cLCD_handler, "Date-->", 0, 2);
		RTC_Read(dateBuffer, timeBuffer); 	// lee y asigna los datos a los buffer
		char str_hour[9];
		char str_date[9];

		array_to_string_hour(timeBuffer, str_hour);
		LCD_writeString(&i2cLCD_handler, str_hour, 8, 0); //(x,y)

		array_to_string_date(dateBuffer, str_date);
		LCD_writeString(&i2cLCD_handler, str_date, 8, 2); //(x,y)

		sprintf(bufferData,"dateHourOn\n");
		usart_writeMsg(&hCmdTerminal,bufferData);
	}

	/* 14) HSI clock*/
	else if(strcmp(cmd,"hsiClock") == 0){
		switch (firstParameter) {
			case 1:signal_selection_MC01(MC01_HSI_CHANNEL, PRESCALER_DIV_1);break;
			case 2:signal_selection_MC01(MC01_HSI_CHANNEL, PRESCALER_DIV_2);break;
			case 3:signal_selection_MC01(MC01_HSI_CHANNEL, PRESCALER_DIV_3);break;
			case 4:signal_selection_MC01(MC01_HSI_CHANNEL, PRESCALER_DIV_4);break;
			case 5:signal_selection_MC01(MC01_HSI_CHANNEL, PRESCALER_DIV_5);break;
			default:
				break;
		}
		sprintf(bufferData,"Freq in MC01->HSI: %.3f MHz \n",16.0/firstParameter);
		usart_writeMsg(&hCmdTerminal,bufferData);
	}

	/* 15) LSE clock*/
	else if(strcmp(cmd,"lseClock") == 0){
		switch (firstParameter) {
			case 1:signal_selection_MC01(MC01_LSE_CHANNEL, PRESCALER_DIV_1);break;
			case 2:signal_selection_MC01(MC01_LSE_CHANNEL, PRESCALER_DIV_2);break;
			case 3:signal_selection_MC01(MC01_LSE_CHANNEL, PRESCALER_DIV_3);break;
			case 4:signal_selection_MC01(MC01_LSE_CHANNEL, PRESCALER_DIV_4);break;
			case 5:signal_selection_MC01(MC01_LSE_CHANNEL, PRESCALER_DIV_5);break;
			default:
				break;
		}
		sprintf(bufferData,"Freq in MC01->LSE: %.3f kHz \n",32.768/firstParameter);
		usart_writeMsg(&hCmdTerminal,bufferData);
	}

	/* 16) PLL clock*/
	else if(strcmp(cmd,"pllClock") == 0){
		switch (firstParameter) {
			case 1:signal_selection_MC01(MC01_PLL_CHANNEL, PRESCALER_DIV_1);break;
			case 2:signal_selection_MC01(MC01_PLL_CHANNEL, PRESCALER_DIV_2);break;
			case 3:signal_selection_MC01(MC01_PLL_CHANNEL, PRESCALER_DIV_3);break;
			case 4:signal_selection_MC01(MC01_PLL_CHANNEL, PRESCALER_DIV_4);break;
			case 5:signal_selection_MC01(MC01_PLL_CHANNEL, PRESCALER_DIV_5);break;
			default:
				break;
		}
		sprintf(bufferData,"Freq in MC01->PLL: %.3f MHz \n",100.0/firstParameter);
		usart_writeMsg(&hCmdTerminal,bufferData);
	}

	/*The inserted msg is not in the list*/
	else{
		usart_writeMsg(&hCmdTerminal,"Wrong CMD\n");
	}
}





/* Función de la Finite State Machine  */
void state_machine_action(void){

	uint32_t currentTime = 1; 		// variable local que almacena el tiempo desde que inicia el código

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

		/*muestreo del ADC y adicion a un bufferdata*/
		adc_samplin();

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

	default:
		fsm.fsmState = STANDBY_STATE;			// Estado de espera
		break;
	}
	fsm.fsmState = STANDBY_STATE;
}

/*funciones para convertir los arrays en strings*/
void array_to_string_hour(uint8_t array[3], char *str) {
    snprintf(str, 9, "%02d:%02d:%02d", array[0], array[1], array[2]);
}
void array_to_string_date(uint8_t array[3], char *str) {
    snprintf(str, 9, "%02d/%02d/%02d", array[0], array[1], array[2]);
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







/****************************** CALLBACKS ***************************/



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

/* Call back de recepción de char*/
void usart6_RxCallback(void){
	rxData = usart_getRxData(&hCmdTerminal);
	fsm.fsmState = CHAR_RECEIVED_STATE;
}


void adcComplete_Callback(void){
	fsm_adc.adcState = ADC_RDY;
	ADC_handler.adcData = getADC();
}

/****************************** FIN CALLBACKS ***************************/




/*
 * Esta función sirve para detectar problemas de parametros
 * incorrectos al momento de ejecutar un programa.
 */
void assert_failed(uint8_t* file, uint32_t line){
	while(1){
		//Problems
	}
}

