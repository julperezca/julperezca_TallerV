/*
 * pll_driver_hal.c
 *
 *  Created on: Feb 2025
 *      Author: julperezca
 */

#include "stm32f4xx.h"
#include "pll_driver_hal.h"
#include "gpio_driver_hal.h"

GPIO_Handler_t MCO1_Pin = {0};
GPIO_Handler_t MCO2_Pin = {0};


void pll_calibration(uint8_t trimm);


/*Función de config de PLL a 100 MHz*/
void pll_Config_100MHz(void){


	// Seleccionar HSI como la señal de entrada al PLL         HSI = 0
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLSRC);


	FLASH->ACR |= FLASH_ACR_PRFTEN;
	FLASH->ACR |= FLASH_ACR_ICEN;
	FLASH->ACR |= FLASH_ACR_DCEN;

	// Configurar latencia para leer memoria flash
	FLASH->ACR &= ~FLASH_ACR_LATENCY;	//Limpiar
	FLASH->ACR |= FLASH_ACR_LATENCY_3WS;


	// Dividir el reloj de entrada usnado PLLM hasta 2MHz, recomendado por el manual para reducir jitter en el PLL

	RCC->PLLCFGR &= ~0xFF;
	//Dividir entre 8
	RCC->PLLCFGR |= 0x08;


	// Multiplicar por el valor PLLN Para conseguir 200MHz y luego dividir entre 2 por el PLLP

	//Limpiar registro
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN;

	//Multiplicar por 100 /*100MHz*/
	RCC->PLLCFGR |= 0x64<< RCC_PLLCFGR_PLLN_Pos;


	// Dividir entre 2
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP;

	// Encender PLL
	RCC->CR |= RCC_CR_PLLON;

	// Esperar a que se estabilice el PLL
	while (!(RCC->CR & RCC_CR_PLLRDY)){
		__NOP();
	}

	// Cnfigurar prescalers del hardware para que funcionen a su velocidad maxima permitida.

	//Prescaler del procesador (AHB1) . 0 = sin prescaler
	RCC->CFGR &= ~RCC_CFGR_HPRE;

	//Prescaler del APB1 (Low speed) Maxima frecuencia 50MHz preescaler 2:1   = 100
	RCC->CFGR &= ~RCC_CFGR_PPRE1;	//Limpiar
	RCC->CFGR |= RCC_CFGR_PPRE1_2;



	//Prescaler del APB2 (High speed)  0 = sin prescaler
	RCC->CFGR &= ~RCC_CFGR_PPRE2;



	// Configurar POWER
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	PWR->CR |= PWR_CR_VOS;


	// Configurar PLL como reloj del sistema							SW   = 10   PLL
	RCC->CFGR &= ~RCC_CFGR_SW;	//Limpiar
	RCC->CFGR |= RCC_CFGR_SW_1;

	//Esperar hasta que se estabilice el sistema
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL){
		__NOP();
	}
	pll_calibration(13);

}

void config_MC01_pin(void){
	//Configuración de GPIO
	MCO1_Pin.pGPIOx 						= GPIOA;
	MCO1_Pin.pinConfig.GPIO_PinNumber		= PIN_8;
	MCO1_Pin.pinConfig.GPIO_PinMode			= GPIO_MODE_ALTFN;
	MCO1_Pin.pinConfig.GPIO_PinAltFunMode	= AF0;
	gpio_Config(&MCO1_Pin);

}

void signal_selection_MC01(uint8_t clock_signal, uint8_t prescalerMCO){

	/*configuración del pin*/
	config_MC01_pin();

	//Configurar canal del MC01--> señal que saldrá por él
	RCC->CFGR &= ~(RCC_CFGR_MCO1);
	RCC->CFGR |= (clock_signal << RCC_CFGR_MCO1_Pos);

	//Configurar prescaler para observar señal
	RCC->CFGR &= ~(RCC_CFGR_MCO1PRE);
	RCC->CFGR |= (prescalerMCO << RCC_CFGR_MCO1PRE_Pos);
}


/*Calibración con el TRIM del HSI para mejorar variaciones en la freq
 *  se escoge un valor de trim menor que 16 debido a que se encontró
 *  que la freq era mayor. un valor de Trim mayor implica que
 *  disminuye el valor neto de freq que se calibra respecto a "ideal"
 *  y viceversa.
 * */


/*
 * Por defecto el valor del registro es de 16 = 0b10000
 * */
void pll_calibration(uint8_t trimm){
	if((trimm<31) && (trimm>0)){
	RCC->CR &= ~RCC_CR_HSITRIM;  // Borrar bits de ajuste
	RCC->CR |= (trimm << RCC_CR_HSITRIM_Pos);  // Usar el valor de fábrica
	}
	while(!(RCC->CR & RCC_CR_HSIRDY)){
		__NOP();
	}
}





