/**
 ******************************************************************************
 * @file           : main.c
 * @author         : namontoy@unal.edu.co
 * @brief          : Here starts the magic!!
 ******************************************************************************
 */
#include <stdint.h>
#include <stdio.h>
#include <stm32f4xx.h>

// Definicion de las cabeceras de las funciones del main
extern void configMagic(void);


uint8_t Mascara1 = 0;
uint16_t Mascara2 = 0;
uint32_t Mascara3 = 0;

uint8_t Mascara4;
uint16_t Mascara5;
uint32_t Mascara6;

uint32_t Mascara20;


int main(void)
{
	Mascara1 &= 0b0<<1;
	Mascara2 |= 0xFA;
	Mascara3 =(Mascara1|Mascara2) |(1<<31);

	Mascara20=Mascara3 |(Mascara2)<<18;


  /*
   *
   * Crear variables para mover la mascara dos al centro de la mascara 3
   * Crear una  variable que almacene un and de la mascara 3 con la mascara 4
   * Crear una variable y guardar el bit 31 de la mascara 3
   *
   */


	while (1) {
	}
	return 0;
}












