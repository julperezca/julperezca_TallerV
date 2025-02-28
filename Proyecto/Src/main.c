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
#include "i2c_driver_hal.h"


	/* GPIO handler y TIMER para el led de estado */
GPIO_Handler_t  ledState    	= {0}; 		// PinH1
Timer_Handler_t blinkyTimer		= {0}; 		// TIM2 Led de estado


	/* GPIO handler para el data rdy del ToF
	 *  EXTI config para lectura y nueva toma de dato
	 *
	 * */
GPIO_Handler_t gpio_ToF_pin 		 = {0}; 		// PinB15
EXTI_Config_t exti_handler_data_rdy	 = {0}; 		// EXTI15




/* I2C handler y pines de GPIO para la pantalla LCD*/
I2C_Handler_t i2c1_handler = {0};
GPIO_Handler_t pinSCL = {0};
GPIO_Handler_t pinSDA = {0};


/* Variables globales */
uint8_t blinkyFlag 		 = 0;		// Flag para el parpadeo del led
uint16_t blinkyPeriod = 0;			// blinky del led de estado


/* Declaración o prototipo de funciones */
extern void configMagic(void);  				// Config del Magic para comunicación serial
void init_config(void);							// Función que inicia la config. de los pines, timers y EXTI
void state_machine_action(void);				// Maquina de estados en accción
void reference_registers_ToF(void);

#define VL53L3CX_I2C_ADDR        0x29    // Dirección I2C del sensor de base
#define VL53L3CX_MODEL_ID_REG    0x010F  // Registro de identificación (16 bits)
#define VL53L3CX_MODULE_TYPE_REG 0x0110  // Registro Module_Type (16 bits)




int main(void) {
    // Inicializar el I2C con tu driver
	configMagic();  // Se inicia la configuracion de Magic
    init_config();

    reference_registers_ToF();

    while (1) {
        // Bucle infinito
    }
}



void reference_registers_ToF(void){
    uint8_t modelID = 0;
    uint8_t moduleType = 0;

    modelID = i2c_Read8BitsFrom16BitReg(&i2c1_handler, VL53L3CX_MODEL_ID_REG);
    moduleType = i2c_Read8BitsFrom16BitReg(&i2c1_handler, VL53L3CX_MODULE_TYPE_REG);

    // verifficación de respuesta del sensor
    if (modelID == 0xEA && moduleType == 0xAA) {
        printf("Sensor VL53L3CX detectado correctamente.\n");
        printf("Model_ID: 0x%02X\n", modelID);
        printf("Module_Type: 0x%02X\n", moduleType);
    } else {
        printf("Error: No se pudo detectar el sensor VL53L3CX.\n");
        printf("Model_ID: 0x%02X\n", modelID);
        printf("Module_Type: 0x%02X\n", moduleType);
    }
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

void exti_encoder_config(){
	/* Se configura GPIO con su EXTI excepto para el userData*/



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


			/* FIN de GPIO and EXTI config */
}

/* I2C config handler*/
void i2c_config(void){
    pinSCL.pGPIOx = GPIOA;
    pinSCL.pinConfig.GPIO_PinNumber = PIN_8;
    pinSCL.pinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    pinSCL.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_OPENDRAIN;
    pinSCL.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_FAST;
    pinSCL.pinConfig.GPIO_PinAltFunMode = AF4;
    gpio_Config(&pinSCL);

    pinSDA.pGPIOx = GPIOC;
    pinSDA.pinConfig.GPIO_PinNumber = PIN_9;
    pinSDA.pinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    pinSDA.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_OPENDRAIN;
    pinSDA.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_FAST;
    pinSDA.pinConfig.GPIO_PinAltFunMode = AF4;
    gpio_Config(&pinSDA);


    i2c1_handler.pI2Cx = I2C3;
    i2c1_handler.i2c_mainClock = I2C_MAIN_CLOCK_16_MHz;
    i2c1_handler.i2c_mode = eI2C_MODE_FM;
    i2c1_handler.slaveAddress = VL53L3CX_I2C_ADDR;
    i2c_Config(&i2c1_handler);
}


/* Funcion encargada de la configuración del GPIO, TIMERS y EXTIs
 *
*/
void init_config(void){


	/*Configuración del led de estado o blinky*/
	led_state_config();


	/* Configuración de los pines GPIO y exti para el encoder*/
	exti_encoder_config();


	/*I2C initialization for LCD screen*/
	i2c_config();

}





/****************************** CALLBACKS ***************************/



/* Callback del blinkytimer alterna el estado del ledState */
void Timer2_Callback(void){
	gpio_TooglePin(&ledState);
}

/* Callback del timer que enciende y apaga los transistores */
void Timer4_Callback(void){

}

/* Callback de la interrupcion del pin B2 que corresponde al Clk */
void callback_ExtInt2(void){
}

/* Callback de la interrupcion del Switch SW del encoder que controla el Led RGB */
void callback_ExtInt15(void){

}

/* Call back de recepción de char*/
void usart6_RxCallback(void){
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

