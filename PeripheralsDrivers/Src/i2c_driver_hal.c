/*
 * i2c_driver_hal2.c
 *
 *  Created on: 
 *      Author: julperezca
 */
#include <stdint.h>
#include "i2c_driver_hal.h"
#include "gpio_driver_hal.h"

/*headers de las funciones privadas*/

static void i2c_enable_clock_peripheral(I2C_Handler_t *pHandlerI2C);
static void i2c_soft_reset(I2C_Handler_t *pHandlerI2C);
static void i2c_set_main_clock(I2C_Handler_t *pHandlerI2C);
static void i2c_set_mode(I2C_Handler_t *pHandlerI2C);
static void i2c_enable_port(I2C_Handler_t *pHandlerI2C);
static void i2c_disable_port(I2C_Handler_t *pHandlerI2C);
static void i2c_stop_signal(I2C_Handler_t *pHandlerI2C);
static void i2c_start_signal(I2C_Handler_t *pHandlerI2C);
static void i2c_restart_signal(I2C_Handler_t *pHandlerI2C);
static void i2c_send_no_ack(I2C_Handler_t *pHandlerI2C);
static void i2c_send_ack(I2C_Handler_t *pHandlerI2C);
static void i2c_send_slave_address_rw(I2C_Handler_t *pHandlerI2C, uint8_t rw);
static void i2c_send_memory_address(I2C_Handler_t *pHandlerI2C, uint8_t memAddr);
static void i2c_send_close_comm(I2C_Handler_t *pHandlerI2C);
static void i2c_send_byte(I2C_Handler_t *pHandlerI2C, uint8_t dataToWrite);
static uint8_t i2c_read_byte(I2C_Handler_t *pHandlerI2C);
void i2c_WriteSingleRegister(I2C_Handler_t *pHandlerI2C, uint8_t regToWrite, uint8_t newValue);
void i2c_WriteSingleRegisterLCD(I2C_Handler_t *pHandlerI2C, uint8_t newValue);
void i2c_Config(I2C_Handler_t *pHandlerI2C){
	//1. activamos la senal de reloj del periferico
	i2c_enable_clock_peripheral(pHandlerI2C);

	//disable i2c port
	i2c_disable_port(pHandlerI2C);

	//2. reiniciamos el periferico para comenzar desde un estado conocido
	i2c_soft_reset(pHandlerI2C);

	//3. configuramos la senal de reloj principal, la cual es usada porr el periferico para generar la senal del bus del i2c
	i2c_set_main_clock(pHandlerI2C);

	//4. configuramos el modo i2c en el cual el sistema funciona, aca tambien se incluye la velocidad de reloj y el tiempo maximo para el cambio de senal (tiempo de subida)
	i2c_set_mode(pHandlerI2C);

	//5. activamos el modulo de i2c
	i2c_enable_port(pHandlerI2C);
}

/* ACTIVACION DE LA SENAL DE RELOJ PARA LOS I2C */
static void i2c_enable_clock_peripheral(I2C_Handler_t *pHandlerI2C){

	if (pHandlerI2C->pI2Cx == I2C1){
		RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	}
	else if (pHandlerI2C->pI2Cx == I2C2){
		RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
	}
	else if (pHandlerI2C->pI2Cx == I2C3){
		RCC->APB1ENR |= RCC_APB1ENR_I2C3EN;
	}
}

static void i2c_set_main_clock(I2C_Handler_t *pHandlerI2C){
	pHandlerI2C->pI2Cx->CR2 &= ~(0b111111 << I2C_CR2_FREQ_Pos); //limpiamos la configuracion previa
	pHandlerI2C->pI2Cx->CR2 |= (pHandlerI2C->i2c_mainClock << I2C_CR2_FREQ_Pos);
}

static void i2c_set_mode(I2C_Handler_t *pHandlerI2C){
	pHandlerI2C->pI2Cx->CCR = 0; //borramos la informacion de ambos registtros de configuracion
	pHandlerI2C->pI2Cx->TRISE = 0;

	if (pHandlerI2C->i2c_mode == eI2C_MODE_SM){ //evaluamos el modo de funcionamiento que se quiere configurar
		pHandlerI2C->pI2Cx->CCR &= ~I2C_CCR_FS; //para elegir el modo estadar hay que escribir 0
		pHandlerI2C->pI2Cx->CCR |= (I2C_MODE_SM_SPEED << I2C_CCR_CCR_Pos); //configuramos el registro encargado de seleccionar la senal de reloj
		pHandlerI2C->pI2Cx->TRISE |= I2C_MAX_RISE_TIME_SM; //configuramos el rise time
	}else{ //en caso contrario no estamos en modo standar, sino en modo fast
		pHandlerI2C->pI2Cx->CCR |= I2C_CCR_FS; //para seleccionar el modo fast hay que escribir 1
		pHandlerI2C->pI2Cx->CCR |= (I2C_MODE_FM_SPEED << I2C_CCR_CCR_Pos); //configuramos el registro que gobierna la senal de reloj
		pHandlerI2C->pI2Cx->TRISE |= (I2C_MAX_RISE_TIME_FM); //configuramos el registro del tiempo de subida en el fast mode

	}

}

/*
 * activa el puerto, es importante notar que cuando el periferico esta activo no es posible configurarlo, hay que deshabilitarlo si se quiere cambiar
 * la configuracion, se debe apagar el periferico, luego cambiar la configuracion, y luego volver a escribirlo
 */

static void i2c_enable_port(I2C_Handler_t *pHandlerI2C){
	pHandlerI2C->pI2Cx->CR1 |= I2C_CR1_PE; //activamos el periferico i2c
}



/*
 * desactiva el puerto para poder configurarlo, si se intenta configurar cuando esta activo se genera un error
 */

static void i2c_disable_port(I2C_Handler_t *pHandlerI2C){
	pHandlerI2C->pI2Cx->CR1 &= ~I2C_CR1_PE; //PARA DESACTIVAR ESCRIBIMOS 0
}

static void i2c_soft_reset(I2C_Handler_t *pHandlerI2C){
	pHandlerI2C->pI2Cx->CR1 |= I2C_CR1_SWRST; //desactivamos escribiendo 1
	__NOP(); //esperamos 2 ciclos de reloj para que las cosas se estabilicen
	__NOP();
	pHandlerI2C->pI2Cx->CR1 &= ~I2C_CR1_SWRST; //volvemos a activar escribiendo 1
}

//funcion para generar la condicion de stop
static void i2c_stop_signal(I2C_Handler_t *pHandlerI2C){
	pHandlerI2C->pI2Cx->CR1 |= I2C_CR1_STOP; //generamos la condicion de stop
}

/*
 * funcion para generar la condicion de start de un ciclo de comunicacion i2c. Consultar la figura 164 de la pagina 481 del reference manual
 *
 * 1. se debe confgurar el bit pos del cr1
 * 2. generamos la senal de start
 * 2.1. esperamos que la bandera del evento start se levante
 * 3. leemos el registro sr1
 *
 * ESTOS SON LOS PASOS ESPECIFICADOS EN EL EVENTO EV5 DE LA FIGURA 164
 */
static void i2c_start_signal(I2C_Handler_t *pHandlerI2C){
	uint8_t auxByte = 0; //definimos una variable auxiliar
	(void) auxByte;

	/*
	 * CONFIGURAMOS EL BIT ACK
	 * este registro controla si eel ack se genera con el byte que se esta leyendo actualmente o con el byte siguiente
	 * lo mas logico y normal es trabajar con el byte actual, es decir que dejaremos este bit en 0 permanentemente
	 */
	pHandlerI2C->pI2Cx->CR1 &= ~I2C_CR1_POS;

	//2. generamos la senal de start
	pHandlerI2C->pI2Cx->CR1 |= I2C_CR1_START;

	/* 2.1. esperamos que la bandera del evento start se levante. este bit se pone en 1 solo si la senal de start se genera satisfactoriamente.
	 * mientras esperamos el valor del SB es 0, por ende la negacion ! es un 1
	 */
	while(!(pHandlerI2C->pI2Cx->SR1 & I2C_SR1_SB)){ //mientras el bit no este seteado
		__NOP(); //no haga nada, espere que se setee
	}

	/* EL SISTEMA ESPERA QUE EL REGISTRO SR1 SEA LEIDO, PARA CONTINUAR SE DEBE ENVIAR UNA SENAL AL ESCLAVO. CONSULTE LA PAGINA 479 DEL USER MANUAL
	 * LA CONDICION DE LEER EL REGISTRO ES NECESARIA Y SIEMPRE DEBE HACERSE
	 */
	auxByte = pHandlerI2C->pI2Cx->SR1; //leemos el registro usando la variable auxiliar que creamos
}

/* segun el manual de referencia, con hacer una senal de start luego de la transferencia de un byte nos genera una senal de restart, por lo
 * tanto es suficiente con hacer otro start
 */

static void i2c_restart_signal(I2C_Handler_t *pHandlerI2C){
	i2c_start_signal(pHandlerI2C); //generamos una senal de start
}

/* activamos la indicacion para no hacer ack para que el esclavo deje de tranmitir datos */
static void i2c_send_no_ack(I2C_Handler_t *pHandlerI2C){
	pHandlerI2C->pI2Cx->CR1 &= ~I2C_CR1_ACK; //no hacemos ack, por lo que escribimos 0
}

/* activamos la indicacion ack, la cual ordena al esclavo que siga enviando informacion, es decir que envie el siguiente byte en memoria */
static void i2c_send_ack(I2C_Handler_t *pHandlerI2C){
	pHandlerI2C->pI2Cx->CR1 |= I2C_CR1_ACK;
}

/* despues de enviar la senal de start o de restart se debe enviar la direccion al esclavo e indicar si se desea leer (1) o escribir (0)
 * la direccion del esclavo se movera una posicion hacia la derecha para generar la indicacion de lectura, escritura en el bit menos significativo
 *
 * ESTO ESTA INDICADO EN EL EVENTO EV6 DE LA FIGURA 164 DEL USER MANUAL
 */
static void i2c_send_slave_address_rw(I2C_Handler_t *pHandlerI2C, uint8_t rw){
	uint8_t auxByte = 0; //definimos una variable auxiliar
	(void) auxByte;

	/* 3. enviamos la direccion del esclavo y el bit de lectura/escritura
	 * (en el paso siguiente se envia el byte que se desea escribir
	 */
	pHandlerI2C->pI2Cx->DR = (pHandlerI2C->slaveAddress << 1) | rw; //se corre hacia la derecha y se introduce la instruccion de lectura/escritura en el bit 0

	//3.1. esperamos que la bandera del evento addr se levante para indicar que la direccion fue enviada satisfacoriamente
	while (!(pHandlerI2C->pI2Cx->SR1 & I2C_SR1_ADDR)){
		__NOP();
	}

	/* hay que limpiar las banderas de recepcion ack de la addr par lo cual leemos primero el bit i2c sr1 y luego el i2c sr2
	 * los leeremos usando la variable auxiliar que creamos al inicio de ela funcion
	 */
	auxByte = pHandlerI2C->pI2Cx->SR1;
	auxByte = pHandlerI2C->pI2Cx->SR2;
	/* El bit TRA del SR2 indica si el equipo quedo en modo recepcion o en modo transmision, lo cual esta definido por la seleccion de lectura
	 * escritura que fue enviado. Consultar la pagina 480 del user manual
	 */
}

static void i2c_send_byte(I2C_Handler_t *pHandlerI2C, uint8_t dataToWrite){
	pHandlerI2C->pI2Cx->DR = dataToWrite; //5. montamos el dato que queremos enviar usando el dataregister

	//5.1. esperamos a que el byte sea montado een el DSR, quedando el DR libre de nuevo
	while(!(pHandlerI2C->pI2Cx->SR1 & I2C_SR1_TXE)){
		__NOP();
	}
}

/* esta funcion se aproveha del hecho de que enviar la direccion de memoria que se quiere leer es lo mismo que enviar un byte cualquiera, por lo
 * tanto simplemente se envia un byte con la posicion de memoria qeu se desea leer
 */
void i2c_send_memory_address(I2C_Handler_t *pHandlerI2C, uint8_t memAddr){
	i2c_send_byte(pHandlerI2C, memAddr);
}

/* esta funcion evalua que el ultimo byte transmitido haya sido enviado completamente por el TSR, luego de esto envia una senal de stop */
void i2c_send_close_comm(I2C_Handler_t *pHandlerI2C){
//	while (!(pHandlerI2C->pI2Cx->SR1 & I2C_SR1_RXNE)){
//		__NOP();
//	}
	i2c_stop_signal(pHandlerI2C); //detenemos la senal de stop
}

uint8_t i2c_read_byte(I2C_Handler_t *pHandlerI2C){
	while(!(pHandlerI2C->pI2Cx->SR1 & I2C_SR1_RXNE)){ //9. esperamos a que el byte sea recibido
		__NOP();
	}

	pHandlerI2C->i2c_data = pHandlerI2C->pI2Cx->DR; //carga el valor recibido en el handler

	return pHandlerI2C->i2c_data; // retorna el valor cargado en el handler
}

// FUNCIONES PUBLICAS DEL DRIVER

uint8_t i2c_ReadSingleRegister(I2C_Handler_t *pHandlerI2C, uint8_t regToRead){
	uint8_t auxRead = 0; //creamos una variable auxiliar
	i2c_start_signal(pHandlerI2C); //generamos la condicion de inicio
	i2c_send_slave_address_rw(pHandlerI2C, eI2C_WRITE_DATA); //enviamos la direccion del esclavo y la indicacion de escribir para que el esclavo acceda a dicha posicion de memoria
	i2c_send_memory_address(pHandlerI2C, regToRead); //enviamos la direccion de memoria del registro que deseaemos leer
	i2c_restart_signal(pHandlerI2C); //enviamos la condicion de restart
	i2c_send_slave_address_rw(pHandlerI2C, eI2C_READ_DATA); //enviamos la direccion del esclavo y la indicacion de leer
	auxRead = i2c_read_byte(pHandlerI2C); //leemos el dato enviado por el esclavo
	i2c_send_no_ack(pHandlerI2C); //enviamos un no ack para que el esclavo no envie mas bytes
	i2c_stop_signal(pHandlerI2C); //enviamos la condicion de stop para que el esclavo se detenga despues de haber enviado un byte

	return auxRead; //retornamos la variable auxiliar cocn el dato leido

}

uint8_t readManyRegisters(I2C_Handler_t *pHandlerI2C, uint8_t regToread, uint8_t *bufferRxData, uint8_t numberOfBytes){
	i2c_start_signal(pHandlerI2C); //generamos una condicion de start
	i2c_send_slave_address_rw(pHandlerI2C, eI2C_WRITE_DATA); //enviamos la direccion del esclavo y la orden de escribir
	i2c_send_memory_address(pHandlerI2C, regToread); //enviamos la direccion desde la cual queremos leer
	i2c_restart_signal(pHandlerI2C); //enviamos una senal de restart
	i2c_send_slave_address_rw(pHandlerI2C, eI2C_READ_DATA); //enviamos la direccion del esclavo con la indicacion de que queremos leer
	i2c_send_ack(pHandlerI2C); //enviamos un ack para que el esclavo siga enviando datos
	while(numberOfBytes > 0){
		if (numberOfBytes == 1){ //si el numero de bytes llega a 1, entonces se ha recibido el ultimo byte y hay que parar
			i2c_send_no_ack(pHandlerI2C); //enviamos un no ack para que el esclavo pare de enviar datos
			i2c_send_close_comm(pHandlerI2C); //enviamos un commando de parada generando la senal de stop, para que se detenga despues de recibir el ultimo byte

		}else { //en caso contrario todavia no se ha llegado al ultimo byte, por tanto hay que guardar el valor en el buffer, el puntero incrementa y se reduce el contador en 1
			*bufferRxData = i2c_read_byte(pHandlerI2C); //guardamos el valor recibido
			bufferRxData++; //avanzamos de posicion en el buffer

		}
		numberOfBytes--; //disminuimos el contador en 1
	}
	return numberOfBytes;

}

void i2c_WriteSingleRegister(I2C_Handler_t *pHandlerI2C, uint8_t regToWrite, uint8_t newValue){
	i2c_start_signal(pHandlerI2C); //generamos la condicion de start
	i2c_send_slave_address_rw(pHandlerI2C, eI2C_WRITE_DATA); //enviamos la direccion del esclavo y la orden de escribir
	i2c_send_memory_address(pHandlerI2C, regToWrite); //enviamos la direccion de memoria que queremos escribir
	i2c_send_byte(pHandlerI2C, newValue); //enviamos el valor que queremos escribir en el registro dado
	i2c_send_close_comm(pHandlerI2C); //enviamos la condicion de stop para que el esclavo envie un solo bite y se detenga
}


void i2c_WriteManyRegisters(I2C_Handler_t *pHandlerI2C, uint8_t regToWrite, uint8_t *bufferRxData, uint8_t numberOfBytes){
	i2c_start_signal(pHandlerI2C); //enviamos la senal de start
	i2c_send_slave_address_rw(pHandlerI2C, eI2C_WRITE_DATA); //enviamos la direccion del esclavo y la condicion de escritura
	i2c_send_memory_address(pHandlerI2C, regToWrite); //enviamos la posicion de memoria desde la cual queremos escribir

	while (numberOfBytes > 0){
		i2c_send_byte(pHandlerI2C, *bufferRxData); //enviamos los datos contenidos en el buffer hasta que acabemos
		bufferRxData++; //se avanza en el buffer dedspues de cada envio
		numberOfBytes--; //se reduce el contador hasta que no deseeemos enviar mas bytes
	}

	i2c_send_close_comm(pHandlerI2C); //enviamos la condicion de parada para que ya no se escriban mas registros
}
