/**
 ******************************************************************************
 * @file           : main.c
 * @author         : namontoy@unal.edu.co
 * @brief          : Here starts the magic!!
 ******************************************************************************
 */

 // Directivas preprocesamiento
#include <stdint.h>
#include <stdio.h>
#include <stm32f4xx.h>


#define RCC_BASE_ADDRESS		0x40023800UL   //buscar de donde sale esta vaina
#define GPIOA_BASE_ADDRESS		0x40020000UL     //direccion base puerto A
#define GPIOC_BASE_ADDRESS		0x40020800UL  		// UL= unsigded lock

#define RCC_AHB1ENR_OFFSET 		0x30
#define GPIO_MODE_REG_OFFSET 	0x00
#define GPIO_PUPD_REG_OFFSET 	0x0C
#define GPIO_ID_REG_OFFSET 		0x10
#define GPIO_OD_REG_OFFSET  	0x14

#define RCC_AHB1ENR 			(RCC_BASE_ADDRESS + RCC_AHB1ENR_OFFSET)

#define GPIOA_MODE_REG 			(GPIOA_BASE_ADDRESS + GPIO_MODE_REG_OFFSET)
#define GPIOA_OPD_REG			(GPIOA_BASE_ADDRESS + GPIO_OD_REG_OFFSET)

#define GPIOC_MODE_REG 			(GPIOC_BASE_ADDRESS + GPIO_MODE_REG_OFFSET)
#define GPIOC_PUPD_REG 			(GPIOC_BASE_ADDRESS + GPIO_PUPD_REG_OFFSET)
#define GPIOC_ID_REG 			(GPIOC_BASE_ADDRESS + GPIO_ID_REG_OFFSET)

// Definicion de las cabeceras de las funciones del main
extern void configMagic(void);


int main(void){

	/* congigura la magia */
	configMagic();

	printf("Pruebas GPIO No HAL\n");


	uint32_t *registerAHB1enb = (uint32_t *)RCC_AHB1ENR;
	//	uint32_t *registerAHB1enb=(uin32_t *)0x40023830UL;

	/*Configuracion para el pin PA5*/

	*registerAHB1enb |=(1<<0); // activando la señal de reloj para el puerto GPIOA


	uint32_t *registerGPIOA_MODE = (uint32_t *)GPIOA_MODE_REG;
	*registerGPIOA_MODE |=(1<<10);

	uint32_t *registerGPIOA_ODR = (uint32_t *)GPIOA_OPD_REG;
	*registerGPIOA_ODR |=(1<<5);  //LED2 (green) set



	/*configuracion para el PC13*/
	*registerAHB1enb |=(1<<2);                  //activando la señal de reloj para puerto GPIOc

	uint32_t *registerGPIOC_MODE = (uint32_t *)GPIOC_MODE_REG;
	*registerGPIOC_MODE &= ~(0b11 <<26);             //configurar el PC13 como entrada digital


	uint32_t *registerGPIOC_PUPD = (uint32_t *)GPIOC_PUPD_REG;
	*registerGPIOC_PUPD &= ~(0b11<<26);      		 // desactivando las resistencias PU y PD



	uint32_t *registerGPIOC_IDR = (uint32_t *)GPIOC_ID_REG;   // input data register

	uint32_t idr_Gpioc_Value = *registerGPIOC_IDR;//crear una variable para cargar el valor
	uint32_t pinc13_Value=0;                      //
	pinc13_Value = (idr_Gpioc_Value>>13);         //  ENTENDER MUYYYYYY BIEN QUÉ HACEN ESTAS TRES LINEAS


	while(1){
		// leemos el valor de todo el puerto GPIOC
		idr_Gpioc_Value = *registerGPIOC_IDR;

		//Extraemos el valor del pinC13
		pinc13_Value = (idr_Gpioc_Value>>13);

		if (pinc13_Value==1){
			*registerGPIOA_ODR |=(1<<5);   //LED GREEN SET

		}
		else{
			*registerGPIOA_ODR &=~(1<<5);  //LED GREEN OFF
		}
	}

	return 0;
}

