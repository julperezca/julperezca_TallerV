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

#define BUFFER_SIZE 64




	/* Enumeración de los estados de la FSM*/
typedef enum
{
	STANDBY_STATE,
	CHAR_RECEIVED_STATE,
	CMD_COMPLETE
} fsm_State;

	/* Estructura  que maneja de la fsm del loop principal */
typedef struct
{
	fsm_State fsmState;
}fsm_t;


/* Enumeración de los estados de la FSM*/
typedef enum
{
	NO_STATE,
	CW_STATE,
	CCW_STATE
} fsm_State_cw;

/* Estructura  que maneja de la fsm del loop principal */
typedef struct
{
fsm_State_cw fsmState_cw;
}fsm_cw_t;


		/* ******************* */



#endif /* MAIN_H_ */

