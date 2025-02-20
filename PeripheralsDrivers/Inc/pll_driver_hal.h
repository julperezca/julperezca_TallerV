/*
 * pll_driver_hal.h
 *
 *  Created on:
 *      Author: julperezca
 */

#ifndef PLL_DRIVER_HAL_H_
#define PLL_DRIVER_HAL_H_



enum{
	MC01_PRESCALER_DIV_2 = 0b100,
	MC01_PRESCALER_DIV_3 = 0b101,
	MC01_PRESCALER_DIV_4 = 0b110,
	MC01_PRESCALER_DIV_5 = 0b111
};




enum{
	MC01_HSI_CHANNEL = 0,
	MC01_LSE_CHANNEL,
	MC01_HSE_CHANNEL,
	MC01_PLL_CHANNEL
};



enum{
	HSI_CLOCK_CONFIGURED = 0,
	HSE_CLOCK_CONFIGURED,
	PLL_CLOCK_CONFIGURED
};



/* Cabeceras de funciones*/

//void pll_Config_100MHz(void);
//void pll_Config_MC01(uint8_t prescalerMCO, uint8_t channelMCO);
//uint8_t pll_Get_MainClock(void);


#endif /* PLL_DRIVER_HAL_H_ */
