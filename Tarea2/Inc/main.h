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


typedef struct
{
	uint16_t stateRGB_Counter;
	led_RGBState stateRGB;
}fsm_RGB_t;





typedef enum
{
	ON_STATE,
	OFF_STATE
} led_blinkState;

typedef struct
{
	uint16_t stateBlinkyCounter;
	led_blinkState blinkState;
}fsm_blink_t;







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

