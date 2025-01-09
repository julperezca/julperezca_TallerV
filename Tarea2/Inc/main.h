/*
 * main.h
 *
 *  Created on: January, 2025
 *  Author: julperezca
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <stdint.h>
#include <stm32f4xx.h>


/* Enumeración de estados del led RGB para Finite State Machine */
typedef enum
{
	RED_STATE,
	GREEN_STATE,
	BLUE_STATE,
	BLUE_GREEN_STATE,
	BLUE_RED_STATE,
	RED_GREEN_STATE,
	RED_GREEN_BLUE_STATE,
	DISABLE_STATE
}led_RGBState;

/* Estructura con el uso de la finite state para led RGB */
typedef struct
{
	led_RGBState stateRGB;
}fsm_RGB_t;




typedef enum
{
	UNIT,
	TENS,
	ONE_HUNDRED,
	ONE_THOUSAND,
	DISABLE_TRANSISTOR
} transistor_state;

typedef struct
{
	transistor_state transState;
}fsm_transistor_t;





typedef enum
{
	ZERO,
	ONE,
	TWO,
	THREE,
	FOUR,
	FIVE,
	SIX,
	SEVEN,
	EIGHT,
	NINE
} segments_State;

typedef struct
{
	segments_State segmentState;
}fsm_segments_t;










typedef enum
{
	CW_STATE,
	CCW_STATE
} rotation_State;

typedef struct
{
	uint16_t rotationStateCounter;
	rotation_State rotationState;
}fsm_rotation_t;





#endif /* MAIN_H_ */

