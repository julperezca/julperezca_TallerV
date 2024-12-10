/*
 * timer_driver_hal.h
 *
 * Created on:
 *  	Author: Julián
 */


#ifndef TIMER_DRIVER_HAL_H_
#define TIMER_DRIVER_HAL_H_

#include "stm32f4xx.h"

enum
{
	TIMER_INT_DISABLE = 0,
	TIMER_INT_ENABLE
};

enum
{
	TIMER_UP_COUNTER = 0,
	TIMER_DOWN_COUNTER
};

enum
{
	TIMER_OFF= 0,
	TIMER_ON
};

/* Estructurar que contiene la contig minima nec par ael manejo del t*/
typedef struct
{
	uint8_t 	TIMx_mode;			//UP or DOWN
	uint16_t	TIMx_Prescaler;		// prescaler...
	uint32_t	TIMx_Period;		// valor en ms del periodo del Timer
	uint8_t		TIMx_InterruptEnable; //activa o desactiva el modo interrup
} Timer_BasicConfig_t;

/* Handler para el timer */
typedef struct
{
	TIM_TypeDef 		*pTIMx;
	Timer_BasicConfig_t	TIMx_Config;
} Timer_Handler_t;


/*for testing asserrt parameters -checking basic configuration */
#define IS_TIMER_INTERRUP(VALUE) 	(((VALUE) == TIMER_INT_DISABLE) ||((VALUE)==TIMER_INT_ENABLE))

#define IS_TIMER_MODE(VALUE)		(((VALUE) == TIMER_UP_COUNTER) ||((VALUE)==TIMER_DOWN_COUNTER))

#define IS_TIMER_STATE(VALUE)		(((VALUE) == TIMER_OFF) ||((VALUE)==TIMER_ON))

#define IS_TIMER_PRESC(VALUE)		(((uint32_t)VALUE)>1 && ((uint32_t)VALUE)<0xFFFE)

#define IS_TIMER_PERIOD(PERIOD)		(((uint32_t)PERIOD)>1)

void timer_Config(Timer_Handler_t *pTimerHandler);
void timer_SetState(Timer_Handler_t *pTimerHandler, uint8_t newState);

/* Esta funcion debe ser sobre-escrrita en el main para que el sistema funcione */
void timer2_Callback(void);

#endif /* TIMER_DRIVER_HAL_H_ */
