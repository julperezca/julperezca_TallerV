/*
 * systick_driver.h
 *
 * Created on:
 *  	Author: julperezca
 */

#ifndef SYSTICK_DRIVER_H_
#define SYSTICK_DRIVER_H_

#include "stm32f4xx.h"


#define SYSTICKLOAD_16MHz  16000     // CONVERSION A ms
#define SYSTICKLOAD_80MHz  80000
#define SYSTICKLOAD_100MHz  100000

enum
{
	CLOCK_SOURCE_16MHz,
	CLOCK_SOURCE_80MHz,
	CLOCK_SOURCE_100MHz
};

void systickConfig(uint8_t clock_source);
void msDelay(uint32_t delay);
uint32_t ticksNumber(void);




#endif /* SYSTICK_DRIVER_H_ */
