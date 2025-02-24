/*
 * pll_driver_hal.h
 *
 *  Created on:
 *      Author: julperezca
 */

#ifndef PLL_DRIVER_HAL_H_
#define PLL_DRIVER_HAL_H_


/*cinco diferetes prescaler para las señales*/
enum{
	PRESCALER_DIV_1 = 0,
	PRESCALER_DIV_2 = 0b100,
	PRESCALER_DIV_3 = 0b101,
	PRESCALER_DIV_4 = 0b110,
	PRESCALER_DIV_5 = 0b111
};



 // Registro RCC_CFGR BIT 21-22 señala la salida de
// la señal de reloj escogida en la enumeracion MC01
enum{
	MC01_HSI_CHANNEL = 0,
	MC01_LSE_CHANNEL,
	MC01_HSE_CHANNEL,
	MC01_PLL_CHANNEL
};


/* funciones publicas*/

void pll_Config_100MHz(void);
void signal_selection_MC01(uint8_t clock_signal, uint8_t prescalerMCO);



#endif /* PLL_DRIVER_HAL_H_ */
