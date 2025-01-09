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


void systickConfig();
void msDelay(uint32_t delay);
uint32_t ticksNumber(void);




#endif /* SYSTICK_DRIVER_H_ */
