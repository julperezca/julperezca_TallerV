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
	EXTERNAL_CLOCK 	= 0,   // Elegir una señal de reloj extern
	INTERNAL_CLOCK		   // Elegir señal de reloj interna
};


/* Estructurar que contiene la contig minima nec par ael manejo del t*/
typedef struct
{
	uint32_t 	SysTick_CSR;		//UP or DOWN
	uint32_t	SysTick_RVR;		// prescaler...
	uint32_t	SysTick_CVR;		// valor en ms del periodo del Timer
	uint32_t	SysTick_CALIB;
} SysTick_Config_t;

/* Handler para el systick*/
typedef struct
{
	SysTick_Type 		*pSysTick;
	SysTick_Config_t	SysTick_Config;
	uint8_t				ExternalCLockEnable;
} Systick_Handler_t;


/*for testing asserrt parameters -checking basic configuration */
//#define IS_TIMER_INTERRUP(VALUE) 	(((VALUE) == TIMER_INT_DISABLE) ||((VALUE)==TIMER_INT_ENABLE))
//
//#define IS_TIMER_MODE(VALUE)		(((VALUE) == TIMER_UP_COUNTER) ||((VALUE)==TIMER_DOWN_COUNTER))
//
//#define IS_TIMER_STATE(VALUE)		(((VALUE) == TIMER_OFF) ||((VALUE)==TIMER_ON))
//
//#define IS_TIMER_PRESC(VALUE)		(((uint32_t)VALUE)>1 && ((uint32_t)VALUE)<0xFFFE)
//
//#define IS_TIMER_PERIOD(PERIOD)		(((uint32_t)PERIOD)>1)
//
//void timer_Config(Timer_Handler_t *pTimerHandler);
//void timer_SetState(Timer_Handler_t *pTimerHandler, uint8_t newState);
//
///* Esta funcion debe ser sobre-escrrita en el main para que el sistema funcione */
//void Timer2_Callback(void);
//void Timer3_Callback(void);
//void Timer4_Callback(void);
//void Timer5_Callback(void);



#endif /* SYSTICK_DRIVER_H_ */
