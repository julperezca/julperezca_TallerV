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


#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define led_Pin GPIO_PIN_1
#define led_GPIO_Port GPIOH
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB


#endif /* MAIN_H_ */

