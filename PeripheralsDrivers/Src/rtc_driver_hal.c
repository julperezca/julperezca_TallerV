/*
 * rtc_driver_hal.c
 *
 *  Created on: Feb 2025
 *      Author: julperezca
 */

#include "stm32f4xx.h"
#include "stm32_assert.h"
#include "rtc_driver_hal.h"

uint8_t Dec_to_BCD(uint8_t dec);


void RTC_config(RTC_Handler_t *pRTC_handler){


    // Habilitar acceso al BDCR
    RCC->APB1ENR |= RCC_APB1ENR_PWREN; // Habilitar módulo PWR
    PWR->CR |= PWR_CR_DBP; // Desbloquear BDCR para poder manipular el LSE



	RCC->BDCR |= RCC_BDCR_LSEON;

	// crear un delay para tener el LSE activo
	// registro que alza la bandera para saber si está listo el LSE
	while (!(RCC->BDCR & RCC_BDCR_LSERDY)){
		__NOP();
	}


	 // habilitar el clock del RTC

	// crear una función  para la elección del reloj: por ahora será LSE pedida en la tarea // escoger reloj NoClock, LSE, LSI, HSE

	RCC->BDCR &= ~RCC_BDCR_RTCSEL; // limpia registro
	RCC->BDCR |= RCC_BDCR_RTCSEL_0;  // 01 corresponde a LSE clock

	 // habilitar el clock del RTC
	RCC->BDCR  |= RCC_BDCR_RTCEN;

	// pg 438 registro de protección de escritura del RTC-> se desactiva
	// dice que se debe escribir :

	//	Write ‘0xCA’ into the RTC_WPR register.
	// Write ‘0x53’ into the RTC_WPR register. Si se aplica uno diferente se activa la protección de escritura

	RTC->WPR |= ((WPR_ENABLE_1) << RTC_WPR_KEY_Pos);
	RTC->WPR |= ((WPR_ENABLE_2) << RTC_WPR_KEY_Pos);

	// está el registro de RTC initialization and statud register RTC_ISR

	// se inicializa
	RTC->ISR |= RTC_ISR_INIT;

	// espera a que se inicialice el RTC
	while (!(RTC->ISR & RTC_ISR_INITF)){
    	__NOP();
    }

	RTC->CR |= RTC_CR_BYPSHAD;

    RTC->DR = 0; // Date register inicia en cero
    RTC->TR = 0; // Time register inicia en cero


    // EL TR tiene la información de configuración de la hora actual que se hace manualmente

	RTC->TR |= ((Dec_to_BCD(pRTC_handler->hour)) << RTC_TR_HU_Pos);
	RTC->TR |= ((Dec_to_BCD(pRTC_handler->minutes)) << RTC_TR_MNU_Pos);
	RTC->TR |= ((Dec_to_BCD(pRTC_handler->seconds)) << RTC_TR_SU_Pos); // HH:MM:SS en BCD

	// El CR contiene la forma del formato
	// dado que se creó el formato en el handler se puede funcionalizar
	if (pRTC_handler->formato == FORMAT_24H){
		RTC->CR &= ~RTC_CR_FMT; // 0 = formato 24h BIT 6 DEL FORMATO DE HORA
	}
	else{
		RTC->CR &= ~RTC_CR_FMT;
		RTC->CR |= RTC_CR_FMT;  // formato AM/PM
	}

	//Date register DR selecciona la fecha, año, mes y día

	RTC->DR |= ((Dec_to_BCD(pRTC_handler->day)) << RTC_DR_DU_Pos);
	RTC->DR |= ((Dec_to_BCD(pRTC_handler->month)) << RTC_DR_MU_Pos);
	RTC->DR |= ((Dec_to_BCD(pRTC_handler->year)) << RTC_DR_YU_Pos);


    RTC->ISR &= ~RTC_ISR_INIT; // se coloca 0 en el registro para terminar la inicialización

    while (!(RTC->ISR & RTC_ISR_INITS)){
    	__NOP();
    }

    RTC->WPR = WPR_DISABLE; // se escribe cualquier comando para bloquear el registro del RTC

}



/* conversion de binario a decimal*/
uint8_t BCD_to_Dec(uint8_t bcd){
	// ingresa un binario bcd y se separa en dos
	// cuatro bits mas significativos y menos significativos
	// se realiza una operación bitwise and para separarlos
    return ((bcd/16)*10) + (bcd%16);
}

/* conversion de binario a decimal*/
uint8_t Dec_to_BCD(uint8_t dec){

	return  ((dec/10)*16) + (dec%10);
}



/* lectura de fecha y tiempo*/
void RTC_Read(uint8_t *storeDate, uint8_t *storeTime){
    // Leer Hora
    uint32_t tiempo = RTC->TR;
    storeTime[0] = BCD_to_Dec((tiempo >> RTC_TR_HU_Pos) & 0x3F); // hora
    storeTime[1] = BCD_to_Dec((tiempo >> RTC_TR_MNU_Pos) & 0x7F);  //
    storeTime[2] = BCD_to_Dec((tiempo >> RTC_TR_SU_Pos) & 0x7F);		  //

    // Leer Fecha
    uint32_t fecha = RTC->DR;
    storeDate[0] = BCD_to_Dec((fecha >> RTC_DR_YU_Pos) & 0xFF); // primer dato: año
    storeDate[1] = BCD_to_Dec((fecha >> RTC_DR_MU_Pos) & 0x1F);	 // segundo dato: mes
    storeDate[2] = BCD_to_Dec((fecha >> RTC_DR_DU_Pos) & 0x3f);		 // tercer dato : día
}
















