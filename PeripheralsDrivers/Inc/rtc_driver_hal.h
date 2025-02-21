/*
 * rtc_driver_hal.h
 *
 *  Created on: Feb 2025
 *      Author: julperezca
 */

#ifndef RTC_DRIVER_HAL_H_
#define RTC_DRIVER_HAL_H_


#include "stm32f4xx.h"
#include "stm32_assert.h"

#define WPR_ENABLE_1	0xCA
#define WPR_ENABLE_2	0x53
#define WPR_DISABLE 	0xFF
#define FORMAT_24H		0
#define FORMAT_AMPM		1


typedef struct
{
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minutes;
	uint8_t seconds;
	uint8_t formato;

}RTC_Handler_t;


/*Funciones publicas*/

void RTC_config(RTC_Handler_t *pRTC_handler);
uint8_t BCD_to_Dec(uint8_t bcd);
void RTC_Read(uint8_t *storeDate, uint8_t *storeTime);


#endif /* RTC_DRIVER_HAL_H_ */
