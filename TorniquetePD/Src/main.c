/**
 ******************************************************************************
 * @file           : main.c
 * @author         : julperezca.edu.co
 * @brief          : main program body
 *  ******************************************************************************
 */
#include "main.h"


fsm_states_t fsm_torniquete = {0};
char bufferData[16];        // definicion de arreglo de characteres de 16


// Definicion de las cabeceras de las funciones del main
//extern void configMagic (void);
void initProgram(void);
e_PosibleStates state_machine_action(uint8_t event);
/*
 *  The main function, where everything happens
 */
int main(void)
{
	/* Configuramos la magia! */
	configMagic();

	initProgram();

	// Ciclo principal
	printf("Hola Mundo!\n");

	while (1){
		/* Esta funcion lee la informacion que llega por el puerto serial desde el cool term */
		read_input(bufferData);

		/* Analizamos si el valor leído es 'c' de coin */
		if (bufferData[0] == 'c'){
			printf("Coin inserted\n");
			state_machine_action(bufferData[0]);           // '' para solo un caracter  "" para string
		}
		/* ANalizamos si el valor leido es 'm'  move */
		else if(bufferData[0] == 'p'){
			printf("Push entrance\n");
			state_machine_action(bufferData[0]);

		}else
		{
			printf("Wrong key...\n");
		}
	}
}





void initProgram(void){
	fsm_torniquete.state = LOCKED;
}

/**/
e_PosibleStates state_machine_action(uint8_t event){
	switch (fsm_torniquete.state) {
	case LOCKED:{
		switch (event){
		case 'c':{
			printf("it was locked -> now it is UNLOCKED\n");
			printf("state: UNLOCKED\n");
			fsm_torniquete.state = UNLOCKED;
			break;
		} //final case 'c'
		case 'p':{
			printf("you cannot cross, please insert coin!\n");
			printf("state: LOCKED\n");
			fsm_torniquete.state = LOCKED;
			break;
		} // final case 'p'

		}
		return fsm_torniquete.state;
	} //final del case UNLOCKED
	case UNLOCKED:
	{
		switch (event){
		case 'c':{
			printf("it is already UNLOCKED !\n");
			printf("state: UNLOCKED\n");
			fsm_torniquete.state = UNLOCKED;
			break;
		} // final case 'c'
		case 'p':{
			printf("You corssed, now is LOCKED!!!\n");
			printf("state: LOCKED\n");
			//fsm.torniquete.accessCounter++;
			fsm_torniquete.state = LOCKED;

			break;
		} //final case

		}
		return fsm_torniquete.state;

	}
	} // final del case UNLOCKED //final del switch-case
	return fsm_torniquete.state;
}
