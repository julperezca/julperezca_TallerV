/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Julián Pérez Carvajal (julperezca@unal.edu.co)
 * @brief          : Examen. LCD-> commands -> RTC
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
#include "pll_driver_hal.h"
#include "i2c_driver_hal.h"

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

GPIO_Handler_t sclPin = {0};
GPIO_Handler_t sdaPin = {0};
I2C_Handler_t LCD_handler = {0};

#define LCD_ADDRESS 0b0100100

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
void init_I2C(void);
/*
 * The main function, where everything happens.
 */
int main (void){
	configMagic();  // Se inicia la configuracion de Magic
	init_config();	// Se inicia la configuracion del sistema
	init_I2C();		// Se inicia la comunicación I2C para los pines B8,B9-> SCL, SDA



	/* Loop infinito */

	while(1){

		/* Condicional para el alza de la bandera del Led de estado */
		if (blinkyFlag){
			gpio_TooglePin(&ledState);		// Alterna estado del led
			blinkyFlag = 0;					// Se limpia la bandera del parpadeo del led

		}
	}
	return 0;
}

void init_I2C(void){

	sclPin.pGPIOx 								= GPIOB;
	sclPin.pinConfig.GPIO_PinNumber        	  	= PIN_8;
	sclPin.pinConfig.GPIO_PinMode     		 	= GPIO_MODE_ALTFN;
	sclPin.pinConfig.GPIO_PinOutputType    		= GPIO_OTYPE_OPENDRAIN;
	sclPin.pinConfig.GPIO_PinPuPdControl   		= GPIO_PUPDR_NOTHING;
	sclPin.pinConfig.GPIO_PinOutputSpeed  	  	= GPIO_OSPEED_FAST;
	sclPin.pinConfig.GPIO_PinAltFunMode	  		= AF4;
	gpio_Config(&sclPin);

	sdaPin.pGPIOx 							= GPIOB;
	sdaPin.pinConfig.GPIO_PinNumber         = PIN_9;
	sdaPin.pinConfig.GPIO_PinMode           = GPIO_MODE_ALTFN;
	sdaPin.pinConfig.GPIO_PinOutputType     = GPIO_OTYPE_OPENDRAIN;
	sdaPin.pinConfig.GPIO_PinPuPdControl    = GPIO_PUPDR_NOTHING;
	sdaPin.pinConfig.GPIO_PinOutputSpeed   	= GPIO_OSPEED_FAST;
	sdaPin.pinConfig.GPIO_PinAltFunMode	    = AF4;
	gpio_Config(&sdaPin);

	LCD_handler.pI2Cx  			= I2C1;
	LCD_handler.i2c_mainClock   = I2C_MAIN_CLOCK_16_MHz;
	LCD_handler.i2c_mode		= eI2C_MODE_SM;
	LCD_handler.slaveAddress    = LCD_ADDRESS;
	i2c_Config(&LCD_handler);
}


/* Funcion encargada de la configuración del GPIO, TIMERS y EXTIs */
void init_config(void){
//	change_to_100MHz();
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
	blinkyTimer.TIMx_Config.TIMx_Prescaler  		= 16000; //100us conversion
	blinkyTimer.TIMx_Config.TIMx_Period				= 200;
	blinkyTimer.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;
	blinkyTimer.TIMx_Config.TIMx_InterruptEnable 	= TIMER_INT_ENABLE;
	timer_Config(&blinkyTimer);
	timer_SetState(&blinkyTimer, TIMER_ON);

		/* FIN de configuración de Led de estado y su timer */

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
		usart_writeMsg(&hCmdTerminal,"12) IB-VBE #     -- Create a Ib vs Vbe table and select the collector voltage around from 200mV to 3300 mV. Set period from 20 us in PWMFilter to\n");
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

	/*The inserted msg is not in the list*/
	else{
		usart_writeMsg(&hCmdTerminal,"Wrong CMD\n");
	}

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
