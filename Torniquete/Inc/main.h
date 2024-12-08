



#ifndef MAIN_H_
#define MAIN_H



#include <stdint.h>
#include <stdio.h>
#include <stm32f4xx.h>

typedef enum
{
	LOCKED,
	UNLOCKED
} e_PosibleStates;


typedef struct
{
	uint32_t accessCounter;
	e_PosibleStates state;
} fsm_states_t;

//definición de las cabeceras de las funciones del main
extern void configMagic(void);
extern void clear_Scanf (void);
extern void read_input(char *data);

#endif /* MAIN_H_ */
