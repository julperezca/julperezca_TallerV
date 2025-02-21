

#include "stm32f4xx.h"
#include "stm32_assert.h"
#include "rtc_driver_hal.h"


void algo(void){

	RCC->BDCR |= RCC_BDCR_LSEON;

	// crear un delay para tener el LSE activo
	if (!(RCC->BDCR & RCC_BDCR_LSERDY)) {
	        RCC->BDCR |= RCC_BDCR_LSEON;
	        // registro que alza la bandera para saber si está listo el LSE
	        while (!(RCC->BDCR & RCC_BDCR_LSERDY));
	 }

	 // habilitar el clock del RTC

	// crear una función  para la elección del reloj: por ahora será LSE pedida en la tarea // escoger reloj NoClock, LSE, LSI, HSE

	RCC->BDCR &= ~RCC_BDCR_RTCSEL_0; // limpia registro
	RCC->BDCR |= RCC_BDCR_RTCSEL_0;  // 01 corresponde a LSE clock

	 // habilitar el clock del RTC
	RCC->BDCR  = RCC_BDCR_RTCEN;

	// pg 438 registro de protección de escritura del RTC-> se desactiva
	// dice que se debe escribir :

	//	Write ‘0xCA’ into the RTC_WPR register.
	// Write ‘0x53’ into the RTC_WPR register. Si se aplica uno diferente se activa la protección de escritura

	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;

	// está el registro de RTC initialization and statud register RTC_ISR

	// se inicializa
	RTC->ISR |= RTC_ISR_INIT;
	while (!(RTC->ISR & RTC_ISR_INITF));





}















