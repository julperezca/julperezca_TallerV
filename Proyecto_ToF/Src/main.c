#include <stdio.h>
#include <stdint.h>
#include "stm32f4xx.h"
#include "gpio_driver_hal.h"
#include "exti_driver_hal.h"
#include "timer_driver_hal.h"
#include "i2c_driver_hal.h"
#include "systick_driver.h"
#include "lidarvl53.h"
#include "pll_driver_hal.h"
#include "usart_driver_hal.h"

/* GPIO handler y TIMER para el LED de estado */
GPIO_Handler_t ledState = {0};  // PinH1
Timer_Handler_t blinkyTimer = {0};  // TIM2 Led de estado


/* GPIO handler y EXTI config para el CLK del encoder*/
GPIO_Handler_t limit_switch_CW	 = {0}; 		// PinB2
EXTI_Config_t exti_CW 			 = {0}; 		// EXTI2

/* GPIO handler y EXTI config para el SW del encoder*/
GPIO_Handler_t limit_switch_CCW  = {0}; 		// PinB15
EXTI_Config_t exti_CCW 			 = {0}; 		// EXTI15

/* GPIO handler para el DT del encoder*/
GPIO_Handler_t userData 			 = {0}; 		// PinB1





/* I2C handler y GPIO para el sensor */
I2C_Handler_t i2c_handler = {0};
GPIO_Handler_t pinSCL_ToF = {0};
GPIO_Handler_t pinSDA_ToF = {0};

GPIO_Handler_t DIR = {0};
GPIO_Handler_t STEP = {0};


/* Handler para usart6*/
USART_Handler_t hCmdTerminal 		 = {0}; 		// USART6
GPIO_Handler_t usart2Tx 			 = {0};			// Tx  pin C6
GPIO_Handler_t usart2Rx				 = {0};			// Rx  pin C7

/* Finite State Machine + subestados */
fsm_t fsm = {0};
fsm_cw_t fsm_giro = {0};
/*analisis de USART*/
uint8_t rxData = 0;
char bufferReception[100];
uint8_t counterReception;


/*buffer para parsecommands*/
char cmd[16];
char write[80];
char userMsg[100] = {0};
char endCommand[100] = {0};
char bufferData[1000] = {0};
char clearBuffer[16] = {0};

/*parametros para el parsecommands*/
unsigned int  firstParameter;
unsigned int  secondParameter;


/*cabecera de funciones*/
void config_i2c(void);
void led_state_config(void);
void reference_registers_ToF(void);
void init_config(void);
void exti_limit_switch_config(void);
void usart_init_config(void);
void stepper_config(void);
void state_machine_action(void);

/*variables */
uint8_t led_flag = 0;
uint32_t distance = 0;
uint16_t counter = 0;
uint16_t lectura = 0;
uint8_t angulo = 0;
uint16_t milimetros = 0;
float distancia = 0;
char texto[100];


#define VL53L3CX_I2C_ADDR  0x29  // Dirección I2C del sensor
#define VL53L3CX_MODEL_ID_REG 0xC0
#define VL53L3CX_MODULE_TYPE_REG 0xC1  // Registro Module_Type (16 bits)

void setHome(void){
	/* Giro en sentido horario */
	gpio_WritePin(&DIR, RESET);

	/*delay para definir giro*/
	msDelay(100);

	/*se gira hasta que se llegue al final de carrera donde está la interrupción*/
	for(uint8_t i=0;i<200;i++){
		gpio_WritePin(&STEP, SET);
		msDelay(50);
		gpio_WritePin(&STEP, RESET);
		msDelay(50);
		/*Condicion de final de carrera en sentido CW*/
		if (fsm_giro.fsmState_cw == CW_STATE){
			fsm_giro.fsmState_cw = NO_STATE;
				break;
		}
	}

	/* Giro en sentido antihorario */
	gpio_WritePin(&DIR, RESET);

	/*delay para definir giro*/
	msDelay(100);

	/*con el propósito de liberar el final de carrera*/
	for (uint8_t i = 0; i<5;i++){{
		gpio_WritePin(&STEP, SET);
		msDelay(50);
		gpio_WritePin(&STEP, RESET);
		msDelay(50);
	}

	/*se imprime en USART2*/

	usart_writeMsg(&hCmdTerminal,"Sensor is in home\n");
}
}

/*función para realizar un escaneado rápido*/
void fast_scaning(void){
	usart_writeMsg(&hCmdTerminal," Fast sampling and going home\n");
	usart_writeMsg(&hCmdTerminal,"  Distance(mm) vs Angle(°)\n");
	/*se define el sentido de giro horario*/
	gpio_WritePin(&DIR, SET);

	/*delay de sentido de giro*/
	msDelay(100);

	/*se avanza en sentido horario hasta en ifnal de carreara*/
	for(uint8_t i=0;i<200;i++){
		if (fsm_giro.fsmState_cw == CCW_STATE){
			fsm_giro.fsmState_cw = NO_STATE;
			break;
		}
		angulo = 1.8*(i+1);

		/*paso para el motor stepper*/
		gpio_WritePin(&STEP, SET);
		msDelay(100);
		gpio_WritePin(&STEP, RESET);
		msDelay(100);
		/*se toma medida en milimetros de la distancia medida por el sensor ToF*/
		milimetros=lidar_lee_mm(dir_s1);

		/*se imprime columna milimetros angulo*/
		sprintf(bufferData,"Datos : %u, %u\n", milimetros,angulo);
		usart_writeMsg(&hCmdTerminal,bufferData);

	}

	/* se elige la dirección en sentido opuesto debido al final de carrera anteior*/
	gpio_WritePin(&DIR, RESET);
	msDelay(100);

	/*giro en sentido CCW*/
	for(uint8_t i=0;i<200;i++){
		if (fsm_giro.fsmState_cw == CW_STATE){
			fsm_giro.fsmState_cw = NO_STATE;
			break;
		}
		angulo = 1.8*(i+1);

		/*paso para el motor stepper*/
		gpio_WritePin(&STEP, SET);
		msDelay(100);
		gpio_WritePin(&STEP, RESET);
		msDelay(100);
		/*se toma medida en milimetros de la distancia medida por el sensor ToF*/
		milimetros=lidar_lee_mm(dir_s1);

		/*se imprime columna milimetros angulo*/
		sprintf(bufferData,"Distancia : %u, %u\n", milimetros,angulo);
		usart_writeMsg(&hCmdTerminal,bufferData);
	}
}



int main() {
    init_config();
    lidar_init(dir_s1);
/*buffer para limpieza de terminal*/
	clearBuffer[0] = 0x1B;
	clearBuffer[1] = 0x5B;
	clearBuffer[2] = 0x32;
	clearBuffer[3] = 0x4A;

	usart_writeMsg(&hCmdTerminal,clearBuffer);
	usart_writeMsg(&hCmdTerminal,"Escriba help @ para desplegar el manual de comandos a utilizar\n");



    while (1) {
    	if(led_flag == 1){
    		gpio_TooglePin(&ledState);
    		led_flag = 0;
    	}
		if(fsm.fsmState != STANDBY_STATE){
			state_machine_action();
		}
    }
    return 0;
}



/* analisis de comando*/
void parseCommands(char *ptrBufferReception){

	sscanf(ptrBufferReception,"%s %u %u %s",cmd,&firstParameter,&secondParameter, userMsg);


	if ((strcmp(cmd,"help")) == 0){
		usart_writeMsg(&hCmdTerminal,"Help Menu CMDs. CMD_Structure: cmd # #  @\n");
		usart_writeMsg(&hCmdTerminal,"1)  sensor           -- Reconocimiento de sensor de ToF\n");
		usart_writeMsg(&hCmdTerminal,"2)  setHome           -- go to home\n");
		usart_writeMsg(&hCmdTerminal,"3) fastSampl  -- Makes a fast sample of rotation in CW and CCW.\n");
		usart_writeMsg(&hCmdTerminal,"4) slowSampl  -- Makes a slow sampling in CW rotation taking means.\n");


	}

	/* 1) Command dummy*/
	else if(strcmp(cmd,"dummy") == 0){
		usart_writeMsg(&hCmdTerminal,"CMD: dummy\n");
		sprintf(bufferData,"number A = %u \n",firstParameter);
		usart_writeMsg(&hCmdTerminal,bufferData);

		sprintf(bufferData,"number B = %u \n", secondParameter);
		usart_writeMsg(&hCmdTerminal, bufferData);
	}

	/* 1) sensor reconocimiento*/
	else if (strcmp(cmd,"sensor") == 0){
		reference_registers_ToF();
    }

	/* 2) SetHomeSensor*/
	else if (strcmp(cmd,"setHome") == 0){
		setHome();
    }


	/*sampleo rapido*/
	else if (strcmp(cmd,"fastSampl") == 0){
		fast_scaning();
	}


/*sampleo con promedio*/

	else if (strcmp(cmd,"slowSampl") == 0){

	/*va primero a home*/
	setHome();

	/*sentido de giro*/
	gpio_WritePin(&STEP, SET);
	msDelay(50);
	/*tabla de qué contra qué  distancia contra angulo*/


	usart_writeMsg(&hCmdTerminal,"Distance(mm) vs Angle(°)\n");
	for(uint8_t i=1;i < 201;i++){


		/*condicion de parada debido a los finales de carrera*/
		if (fsm_giro.fsmState_cw == CCW_STATE){
			fsm_giro.fsmState_cw = NO_STATE;
			usart_writeMsg(&hCmdTerminal,"Data sampling done\n");
			break;
		}


		/* se toman multiples medidas y se proemdia entre 10 datos*/
		for (uint8_t k = 0; k<11;k++){
			lectura=lidar_lee_mm(dir_s1);
			milimetros+=lectura;
		}
		milimetros = milimetros/10;
		angulo = 1.8*i;
		sprintf(bufferData,"%u, %u\n", milimetros,angulo);
		usart_writeMsg(&hCmdTerminal,bufferData);

	}

	/*se vuelve a la casa, inicio de final de carrera*/
	setHome();

	}


	/*The inserted msg is not in the list*/
	else{
		usart_writeMsg(&hCmdTerminal,"Wrong CMD\n");
	}
}



/*recepción de carácter*/
void ReceivedChar(void){
	if (hCmdTerminal.receivedChar != '\0'){
		bufferReception[counterReception] = hCmdTerminal.receivedChar;
		counterReception++;

		if (hCmdTerminal.receivedChar =='@'){
			bufferReception[counterReception] = '\0';
			counterReception = 0;

			fsm.fsmState = CMD_COMPLETE;
		}
	}

}





/* Función de la Finite State Machine  */
void state_machine_action(void){


	/* Switch case que evalua cada uno los estados de la FSM */
	switch (fsm.fsmState){

	case CHAR_RECEIVED_STATE:
		ReceivedChar();
		if(fsm.fsmState == CMD_COMPLETE){
			parseCommands(bufferReception);
			for (uint8_t i = 0; i < sizeof(bufferReception); i++){
				bufferReception[i] = 0;
			}
		}
		break;

	default:
		fsm.fsmState = STANDBY_STATE;			// Estado de espera
		break;
	}
	fsm.fsmState = STANDBY_STATE;
}



/*registros de referencia del sensor ToF*/
void reference_registers_ToF(void){

	/*variables para almacenar lectura de registros*/
    uint8_t modelID = 0;
    uint8_t moduleType = 0;


    /*lectura de registros de referencia del sensor ToF*/
    modelID = i2c_ReadSingleRegister(&i2c_handler, VL53L3CX_MODEL_ID_REG);
    moduleType = i2c_ReadSingleRegister(&i2c_handler, VL53L3CX_MODULE_TYPE_REG);



    if (modelID == 0xEE && moduleType == 0xAA) {
		usart_writeMsg(&hCmdTerminal,"VL53L0X detectado\n");
		sprintf(bufferData,"Model_ID: 0x%02X\n", modelID);
		usart_writeMsg(&hCmdTerminal,bufferData);
		sprintf(bufferData,"Module_Type: 0x%02X\n", moduleType);
		usart_writeMsg(&hCmdTerminal,bufferData);

    } else {
    	usart_writeMsg(&hCmdTerminal,"VL53L0X no detectado\n");
		sprintf(bufferData,"Model_ID: 0x%02X\n", modelID);
		usart_writeMsg(&hCmdTerminal,bufferData);
		sprintf(bufferData, "Module_Type: 0x%02X\n", moduleType);
		usart_writeMsg(&hCmdTerminal,bufferData);
    }
}




/*inicio de configuración*/
void init_config(void) {
	/*reloj a 100MHz*/
	pll_Config_100MHz();

	/*systick a la señal de reloj*/
    systickConfig(CLOCK_SOURCE_100MHz);

    /*led de estado*/
    led_state_config();

    /*i2c para ToF sensor config*/
    config_i2c();

    /*configuracion de pines del stepper para dar pasos*/
    stepper_config();


    /*configuración de pines y extis*/
    exti_limit_switch_config();


/* inicia girando en sentido horario*/
    gpio_WritePin(&DIR, RESET);


    /*configuración de usart2 para parse y mirar tabla de datos*/
    usart_init_config();

}


void led_state_config(void){
    /* Configuración de LED de estado y su respectivo timer */

    // GPIO config para Led de estado
    ledState.pGPIOx							= GPIOH;
    ledState.pinConfig.GPIO_PinNumber		= PIN_1;
    ledState.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
    ledState.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
    ledState.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
    ledState.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
    gpio_Config(&ledState);
    // Inicialmente led de estado está encendido
    gpio_WritePin(&ledState, RESET);

    // Config para el timer del led de estado
    blinkyTimer.pTIMx								= TIM2;
    blinkyTimer.TIMx_Config.TIMx_Prescaler  		= 10000; //100us conversion
    blinkyTimer.TIMx_Config.TIMx_Period				= 2500*4;  // 250ms
    blinkyTimer.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;
    blinkyTimer.TIMx_Config.TIMx_InterruptEnable 	= TIMER_INT_ENABLE;
    timer_Config(&blinkyTimer);
    timer_SetState(&blinkyTimer, TIMER_ON);

    /* FIN de configuración de Led de estado y su timer */
}

void config_i2c(void){

    /*  --------INICIO DE LA CONFIGURACIÓN DEL I2C PARA ToF ---------*/


	/*Pin for I2C SCL signal for ToF sensor*/
    pinSCL_ToF.pGPIOx = GPIOB;
    pinSCL_ToF.pinConfig.GPIO_PinNumber = PIN_8;
    pinSCL_ToF.pinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    pinSCL_ToF.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_OPENDRAIN;
    pinSCL_ToF.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_FAST;
    pinSCL_ToF.pinConfig.GPIO_PinAltFunMode = AF4;
    gpio_Config(&pinSCL_ToF);

    /*Pin for I2C SDA data signal for ToF sensor*/
    pinSDA_ToF.pGPIOx = GPIOB;
    pinSDA_ToF.pinConfig.GPIO_PinNumber = PIN_9;
    pinSDA_ToF.pinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    pinSDA_ToF.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_OPENDRAIN;
    pinSDA_ToF.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_FAST;
    pinSDA_ToF.pinConfig.GPIO_PinAltFunMode = AF4;
    gpio_Config(&pinSDA_ToF);

    /*I2C1 config with que ToF sensor address*/
    i2c_handler.pI2Cx = I2C1;
    i2c_handler.i2c_mainClock = I2C_MAIN_CLOCK_16_MHz;
    i2c_handler.i2c_mode = eI2C_MODE_FM;
    i2c_handler.slaveAddress = VL53L3CX_I2C_ADDR;
    i2c_Config(&i2c_handler);

    /*  --------FIN DE CONFIGURACIÓN DEL I2C PARA ToF ---------*/

}

void stepper_config(void){
    // GPIO config para Led de estado
    DIR.pGPIOx							= GPIOB;
    DIR.pinConfig.GPIO_PinNumber		= PIN_7;
    DIR.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
    DIR.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
    DIR.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
    DIR.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
    gpio_Config(&DIR);

    STEP.pGPIOx							= GPIOC;
    STEP.pinConfig.GPIO_PinNumber		= PIN_13;
    STEP.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
    STEP.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
    STEP.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
    STEP.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
    gpio_Config(&STEP);
}



void usart_init_config(void){
	/* Configuración para USART6 */
	//  GPIO Rx, Tx config
	usart2Tx.pGPIOx = GPIOA;
	usart2Tx.pinConfig.GPIO_PinNumber = PIN_2;
	usart2Tx.pinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	usart2Tx.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	usart2Tx.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_MEDIUM;
	usart2Tx.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	usart2Tx.pinConfig.GPIO_PinAltFunMode = AF7;
	gpio_Config(&usart2Tx);

	usart2Rx.pGPIOx = GPIOA;
	usart2Rx.pinConfig.GPIO_PinNumber = PIN_3;
	usart2Rx.pinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	usart2Rx.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_PUSHPULL;
	usart2Rx.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_MEDIUM;
	usart2Rx.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	usart2Rx.pinConfig.GPIO_PinAltFunMode = AF7;
	gpio_Config(&usart2Rx);

	// USART 6 CONFIG
	hCmdTerminal.ptrUSARTx = USART2;
	hCmdTerminal.USART_Config.baudrate = USART_BAUDRATE_115200;
	hCmdTerminal.USART_Config.datasize = USART_DATASIZE_8BIT;
	hCmdTerminal.USART_Config.parity = USART_PARITY_ODD;
	hCmdTerminal.USART_Config.stopbits = USART_STOPBIT_1;
	hCmdTerminal.USART_Config.mode = USART_MODE_RXTX;
	hCmdTerminal.USART_Config.enableIntRX = USART_RX_INTERRUP_ENABLE;
	hCmdTerminal.USART_Config.enableIntTX = USART_TX_INTERRUP_DISABLE;
	usart_Config(&hCmdTerminal);

		 /* Fin de la config del USART6 */
}







void exti_limit_switch_config(void){
	/* Se configura GPIO con su EXTI excepto para el userData*/

	// GPIO mode in para el final de carrera horario
	limit_switch_CCW.pGPIOx							= GPIOB;
	limit_switch_CCW.pinConfig.GPIO_PinNumber		= PIN_2;
	limit_switch_CCW.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	limit_switch_CCW.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&limit_switch_CCW);

	// Configuración EXTI para el final de carrera horario
	exti_CCW.pGPIOHandler						= &limit_switch_CCW;
	exti_CCW.edgeType							= EXTERNAL_INTERRUPT_FALLING_EDGE;
	exti_Config(&exti_CCW);

	// GPIO mode in para el final de carrera antihorario
	limit_switch_CW.pGPIOx							= GPIOB;
	limit_switch_CW.pinConfig.GPIO_PinNumber		= PIN_15;
	limit_switch_CW.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	limit_switch_CW.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&limit_switch_CW);

	// Configuración EXTI para el final de carrera horario
	exti_CW.pGPIOHandler						= &limit_switch_CW;
	exti_CW.edgeType							= EXTERNAL_INTERRUPT_FALLING_EDGE;
	exti_Config(&exti_CW);

			/* FIN de GPIO and EXTI config */
}






/*Call back timer led de estado*/
void Timer2_Callback(void) {
	led_flag = 1;
}


/* Callback de la interrupcion del pin B2 CCW que corresponde final de carrera sentido horario */
void callback_ExtInt2(void){
	fsm_giro.fsmState_cw = CCW_STATE;
}

/* Callback de la interrupción del pinB15  CW que corresponde eal final de carrera sentido antihorario */
void callback_ExtInt15(void){
	fsm_giro.fsmState_cw = CW_STATE;

}

/* Call back de recepción de char*/
void usart2_RxCallback(void){
	rxData = usart_getRxData(&hCmdTerminal);
	fsm.fsmState = CHAR_RECEIVED_STATE;
}
