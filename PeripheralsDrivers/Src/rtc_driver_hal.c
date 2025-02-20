

#include "stm32f4xx.h"
#include "stm32_assert.h"
#include "rtc_driver_hal.h"


void algo(void){
	RCC->BDCR |= RCC_BDCR_LSEON; // crear un delay para tener el LSE activo
	 // habilitar el clock del RTC

	RCC_BDCR_LSERDY; // registro que alza la bandera para saber si está bien o no el LSE
	// crear una función  para la elección del reloj: por ahora será LSE pedida en la tarea // escoger reloj NoClock, LSE, LSI, HSE
	RCC->BDCR  = RCC_BDCR_RTCEN;
	RCC->BDCR |= RCC_BDCR_RTCSEL_0;  // 01 corresponde a LSE clock



}
