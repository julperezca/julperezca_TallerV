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

#define MAX_12_BITS 0xFFF  //Numero máximo del display
#define BUFFER_SIZE 64

			/* ******************* */


	/* Enumeración de los estados de la FSM*/
typedef enum
{
	STANDBY_STATE,
	SW_BUTTON_STATE,
	DISPLAY_VALUE_STATE,
	CHAR_RECEIVED_STATE,
	CMD_COMPLETE,
	ADC_COMPLETE
} fsm_State;

	/* Estructura  que maneja de la fsm del loop principal */
typedef struct
{
	fsm_State fsmState;
}fsm_t;

			/* ******************* */


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


	/* Estructura que maneja el led RGB*/
typedef struct
{
	led_RGBState stateRGB;
}fsm_RGB_t;

			/* ******************* */


	/* Enumeración para los digitos del display*/
typedef enum
{
	UNIT,
	TENS,
	ONE_HUNDRED,
	ONE_THOUSAND,
	DISABLE_TRANSISTOR
} transistor_state;

	/* Estructura que maneja los transistores */
typedef struct
{
	transistor_state transState;
}fsm_transistor_t;

			/* ******************* */


	/* Enumeración de los estados de los segmentos*/
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

	/* Estructura que maneja los segmentos */
typedef struct
{
	segments_State segmentState;
}fsm_segments_t;

			/* ******************* */


	/* Enumeración de los estados de rotación*/
typedef enum
{
	NO_ROTATION,
	ROTATION_STATE,
} rotation_State;

 	/* Estructura que maneja los estados de rotación */
typedef struct
{
	rotation_State rotationState;
}fsm_rotation_t;

			/* ******************* */



/* Enumeración de los estados de rtc*/
typedef enum
{
	DATE_HOUR_OFF,
	DATE_HOUR_ON,
} rtc_State;

	/* Estructura que maneja los estados de rtc */
typedef struct
{
	rtc_State rtcState;
}fsm_rtc_t;

		/* ******************* */




#endif /* MAIN_H_ */
