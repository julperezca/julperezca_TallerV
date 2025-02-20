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




void pll_Config_100MHz(void){

	/*Se configura el PLL para poder ser medido en salida MCO1 sin necesidad de estar activado como reloj principal */

	//1. Seleccionar HSI como la señal de entrada al PLL         HSI = 0   , HSE = 1

	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLSRC);

	//2. Dividir el reloj de entrada usnado PLLM hasta 2MHz, recomendado por el manual para reducir jitter en el PLL

	//Limpiar registro
	RCC->PLLCFGR &= ~0b11111111;

	//Dividir entre 8
	RCC->PLLCFGR |= 0b00001000;


	//3. Multiplicar por el valor PLLN Para conseguir 200MHz y luego dividir entre 2 por el PLLP

	//Limpiar registro
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN;

	//Multiplicar por 100
	RCC->PLLCFGR |= 0b001100100<< RCC_PLLCFGR_PLLN_Pos;

	//4. Dividir entre 2
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP;

	//5. Encender PLL
	RCC->CR |= RCC_CR_PLLON;

	//6. Esperar a que se estabilice el PLL
	while (!(RCC->CR & RCC_CR_PLLRDY)){
		__NOP();
	}

	//7. Configurar prescalers del hardware para que funcionen a su velocidad maxima permitida.

	//Prescaler del procesador (AHB1) . 0 = sin prescaler
	RCC->CFGR &= ~RCC_CFGR_HPRE;

	//Prescaler del APB1 (Low speed) Maxima frecuencia 50MHz preescaler 2:1   = 100
	RCC->CFGR &= ~RCC_CFGR_PPRE1;	//Limpiar
	RCC->CFGR |= RCC_CFGR_PPRE1_2;

	//Prescaler del APB2 (High speed)  0 = sin prescaler
	RCC->CFGR &= RCC_CFGR_PPRE2;


	//8. Configurar latencia para leer memoria flash
	FLASH->ACR &= ~FLASH_ACR_LATENCY;	//Limpiar
	FLASH->ACR |= FLASH_ACR_LATENCY_3WS;

	//9. Configurar POWER
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	PWR->CR |= PWR_CR_VOS;


	//10. Configurar PLL como reloj del sistema							SW   = 10   PLL
	RCC->CFGR &= ~RCC_CFGR_SW;	//Limpiar
	RCC->CFGR |= RCC_CFGR_SW_1;

	//Esperar hasta que se estabilice el sistema
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL){
		__NOP();
	}
}


 // configuración de los pines para obtener la señal de 100MHz
void pll_Config_MC01(uint8_t prescalerMCO, uint8_t channelMCO){

	//Configurar GPIO
	MCO1_Pin.pGPIOx 						= GPIOA;
	MCO1_Pin.pinConfig.GPIO_PinNumber		= PIN_8;
	MCO1_Pin.pinConfig.GPIO_PinMode			= GPIO_MODE_ALTFN;
	MCO1_Pin.pinConfig.GPIO_PinAltFunMode	= AF0;
	gpio_Config(&MCO1_Pin);

	//Configurar canal del MC01
	RCC->CFGR &= ~RCC_CFGR_MCO1;
	RCC->CFGR |= channelMCO<<RCC_CFGR_MCO1_Pos;

	//Configurar prescaler para observar señal
	RCC->CFGR &= ~RCC_CFGR_MCO1PRE;
	RCC->CFGR |= prescalerMCO<<RCC_CFGR_MCO1PRE_Pos;
}


uint8_t pll_Get_MainClock(void){


    uint8_t auxClock = 0;
    uint32_t pll_m, pll_n, pll_p, pll_src;
    uint32_t sysclk_source = (RCC->CFGR & RCC_CFGR_SWS) >> 2;

    if (sysclk_source == 0)      // HSI seleccionado
        auxClock = 16;
    else if (sysclk_source == 1) // HSE seleccionado
        auxClock = 8;            // Suponiendo un HSE de 8 MHz (puede cambiar)
    else if (sysclk_source == 2) // PLL seleccionado
    {
        /* Leer valores del PLL */
        pll_src = (RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC) >> 22;  // 0 = HSI, 1 = HSE
        pll_m = (RCC->PLLCFGR & RCC_PLLCFGR_PLLM);            // PLLM [5:0]
        pll_n = (RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6;        // PLLN [14:6]
        pll_p = ((RCC->PLLCFGR & RCC_PLLCFGR_PLLP) >> 16)*2 + 2; // PLLP es 2, 4, 6 u 8

        /* Determinar la frecuencia de entrada del PLL */
        uint32_t pll_input_freq;
        if (!pll_src){

        	pll_input_freq = 16;
        }
        else{
        	pll_input_freq = 8;
        }

        /* Calcular SYSCLK */
        auxClock = (pll_input_freq * pll_n) / (pll_m * pll_p);
    }
	return auxClock;
}



