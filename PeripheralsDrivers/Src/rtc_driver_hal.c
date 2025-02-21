/*
 * rtc_driver_hal.c
 *
 *  Created on: Feb 2025
 *      Author: julperezca
 */

#include "stm32f4xx.h"
#include "stm32_assert.h"
#include "rtc_driver_hal.h"

    uint8_t hour;
	uint8_t minutos;
	uint8_t segundos = 0;
    uint8_t dia;
	uint8_t mes;
	uint8_t anio;
	uint8_t dayOfWeek = 0;


void algo(void){


    // Habilitar acceso al BDCR
    RCC->APB1ENR |= RCC_APB1ENR_PWREN; // Habilitar módulo PWR
    PWR->CR |= PWR_CR_DBP; // Desbloquear BDCR para poder manipular el LSE



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

	RTC->WPR = WPR_ENABLE_1;
	RTC->WPR = WPR_ENABLE_2;

	// está el registro de RTC initialization and statud register RTC_ISR

	// se inicializa
	RTC->ISR |= RTC_ISR_INIT;

	// espera a que se inicialice el RTC
	while (!(RTC->ISR & RTC_ISR_INITF));


	RTC->CR &= ~RTC_CR_FMT; // 0 = formato 24h BIT 6 DEL FORMATO DE HORA


	// EL rtc_TR tiene la información de configuración de la hora actual que se hace manualmente

    RTC->TR = (12 << 16) | (34 << 8) | (56 << 0); // HH:MM:SS en BCD






    RTC->ISR &= ~RTC_ISR_INIT; // se coloca 0 en el registro para terminar la inicialización
	while (!(RTC->ISR & RTC_ISR_INITF));

    RTC->WPR = WPR_DISABLE; // se escribe cualquier comando para bloquear el registro del RTC




}




void RTC_ReadTime(void) {


    // Leer Hora
    uint32_t tiempo = RTC->TR;
    hour = BCD_to_Dec((tiempo >> 16) & 0x3F);
    minutos = BCD_to_Dec((tiempo >> 8) & 0x7F);
    segundos = BCD_to_Dec(tiempo & 0x7F);

    // Leer Fecha
    uint32_t fecha = RTC->DR;
    anio = BCD_to_Dec((fecha >> 16) & 0xFF);
    mes = BCD_to_Dec((fecha >> 8) & 0x1F);
    dia = BCD_to_Dec(fecha & 0x3F);
    dayOfWeek = (fecha >> 13) & 0x07;


}















