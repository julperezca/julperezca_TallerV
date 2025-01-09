/*
 * magic.c
 *
 *  Created on: Oct 31, 2024
 *      Author: namontoy
 */
#include <stdint.h>
#include <stdio.h>
#include <stm32f4xx.h>


void configMagic(void)
{
	/* desactivamos el buffer para el scanf */
	setvbuf(stdin, NULL, _IONBF, 0);

	/* Activamos el coprocesador Matematico - FPU */
	SCB->CPACR |= (0xF << 20);

	// Activamos el reloj para el GPIOA
	RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOAEN);
	// Configuramos MODER
	GPIOA->MODER &= ~GPIO_MODER_MODER2;
	GPIOA->MODER |= (0x2 << GPIO_MODER_MODER2_Pos);
	GPIOA->MODER &= ~GPIO_MODER_MODER3;
	GPIOA->MODER |= (0x2 << GPIO_MODER_MODER3_Pos);
	//Configuramos OTYPER
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT2;
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT3;
	// Configuramos OSPEEDR
	GPIOA->OSPEEDR &= ~GPIO_OSPEEDER_OSPEEDR2;
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR2;
	GPIOA->OSPEEDR &= ~GPIO_OSPEEDER_OSPEEDR3;
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR3;
	// Configuramos PUPDR
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD2;
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD3;
	// Configuramos AFRL
	GPIOA->AFR[0] &= ~(0xF << 8);
	GPIOA->AFR[0] |= (0x7 << 8);
	GPIOA->AFR[0] &= ~(0xF << 12);
	GPIOA->AFR[0] |= (0x7 << 12);

	// Configuramos ahora el USART2 (Debug) -> 115200-8N1
	// Activamos el reloj para el USART2
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	// Configuramos el parity (none)
	USART2->CR1 &= ~USART_CR1_PCE;
	// Configuramos data-size (8bit)
	USART2->CR1 &= ~USART_CR1_M;
	// Configuramos stop bits (1 bit)
	USART2->CR2 &= ~USART_CR2_STOP;
	// Configuramos baudrate (115200)
	USART2->BRR = 0x008B;
	// Rx enable
	USART2->CR1 &= ~USART_CR1_RE;
	USART2->CR1 |= USART_CR1_RE;
	// Tx enable
	USART2->CR1 &= ~USART_CR1_TE;
	USART2->CR1 |= USART_CR1_TE;
	// Activamos el USART2
	USART2->CR1 |= USART_CR1_UE;


	for(int i = 0; i < 30000; i++){
		__NOP();
	}
	char clear[5] = { 0 };
	clear[0] = 0x1B;
	clear[1] = 0x5B;
	clear[2] = 0x32;
	clear[3] = 0x4A;
	printf(&clear[0]);
	printf(&clear[0]);

}


/**/
void clear_Scanf (void)
{
	printf("clear_Scanf\n");

	char c = getchar();

    while ( (c != 0x3) ) {
    	printf("c = 0x%X \n", c);
    	c = getchar();
    }
}

/**/
void read_input(char *data)
{

	char c = (char)getchar();

    while (1) {
    	*data = c;
    	data++;
    	//printf("c = 0x%X \n", c);
    	c = (char) getchar();

    	if( (c == 0x3) || (c == '\n') ){
    		*data = '\0';
    		return;
    	}
    }
}

/**
  * @brief  Retargets the C library printf function to the USART.
  *   None
  * @retval None
  */
int __io_putchar(int ch)
{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART1 and Loop until the end of transmission */
	while (!(USART2->SR & USART_SR_TXE)) {
		__NOP();
	}

	USART2->DR = ch;

	return ch;
}

/**
  * @brief  Retargets the C library scanf function to the USART.
  *   None
  * @retval None
  */
int __io_getchar(void){
	uint8_t ch = 0;

	/* Clear the Overrun flag just before receiving the first character */
	USART2->SR &= ~USART_SR_ORE_Msk;

	/* Wait for reception of a character on the USART RX line and echo this
	 * character on console */
	while (!(USART2->SR & USART_SR_RXNE)) {
		__NOP();
	}

	ch = USART2->DR;

	return ch;
}
