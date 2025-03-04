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
/* GPIO handler y TIMER para el LED de estado */
GPIO_Handler_t ledState = {0};  // PinH1
Timer_Handler_t blinkyTimer = {0};  // TIM2 Led de estado


/* I2C handler y GPIO para el sensor */
I2C_Handler_t i2c_handler = {0};
GPIO_Handler_t pinSCL = {0};
GPIO_Handler_t pinSDA = {0};


void config_i2c(void);
void led_state_config(void);
void reference_registers_ToF(void);
extern void configMagic(void);
void init_config(void);

uint32_t distance = 0;
uint8_t status = 0;

#define VL53L3CX_I2C_ADDR  0x29  // Dirección I2C del sensor
#define VL53L3CX_MODEL_ID_REG 0xC0
#define VL53L3CX_MODULE_TYPE_REG 0xC1  // Registro Module_Type (16 bits)

uint16_t milimetros = 0;
float distancia = 0;
char texto[100];


int main() {
	pll_Config_100MHz();
    systickConfig(CLOCK_SOURCE_16MHz);
    configMagic();
    init_config();
    reference_registers_ToF();
    lidar_init(dir_s1);

    while (1) {
        milimetros=lidar_lee_mm(dir_s1);
        distancia=lidar_lee_cm(dir_s1);
        printf("distancia: %.2f\n",distancia);
        msDelay(500);
    }
    return 0;
}



void init_config(void) {
    led_state_config();
    config_i2c();


}


void Timer2_Callback(void) {
	gpio_TooglePin(&ledState);

}

void assert_failed(uint8_t* file, uint32_t line) {
    while (1) {}
}

void reference_registers_ToF(void){

    uint16_t modelID = 0;
    uint16_t moduleType = 0;

    modelID = i2c_Read16BitsFromRegister(&i2c_handler, VL53L3CX_MODEL_ID_REG);
    moduleType = i2c_Read16BitsFromRegister(&i2c_handler, VL53L3CX_MODULE_TYPE_REG);

    // Verificación de respuesta del sensor
    if (modelID == 0xEAAA) {
        printf("Sensor VL53L3CX detectado correctamente.\n");
        printf("Model_ID: 0x%04X\n", modelID);
        printf("Module_Type: 0x%04X\n", moduleType);
    } else {
        printf("Error: No se pudo detectar el sensor VL53L3CX.\n");
        printf("Model_ID: 0x%04X\n", modelID);
        printf("Module_Type: 0x%04X\n", moduleType);
    }

//    uint8_t modelID = 0;
//    uint8_t moduleType = 0;
//    uint8_t thirdRegister = 0;
//    uint8_t fourthRegister = 0;
//    modelID = i2c_ReadSingleRegister(&i2c_handler, VL53L3CX_MODEL_ID_REG);
//    moduleType = i2c_ReadSingleRegister(&i2c_handler, VL53L3CX_MODULE_TYPE_REG);
//    thirdRegister = i2c_ReadSingleRegister(&i2c_handler, 0xC2);
//    fourthRegister = i2c_ReadSingleRegister(&i2c_handler, 0x61);
//    if (modelID == 0xEE && moduleType == 0xAA && thirdRegister == 0x10 && fourthRegister ==0x00) {
//        printf("Sensor VL53L3CX detectado correctamente.\n");
//        printf("Model_ID: 0x%02X\n", modelID);
//        printf("Module_Type: 0x%02X\n", moduleType);
//    } else {
//        printf("Error: No se pudo detectar el sensor VL53L3CX.\n");
//        printf("Model_ID: 0x%02X\n", modelID);
//        printf("Module_Type: 0x%02X\n", moduleType);
//    }
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
    blinkyTimer.TIMx_Config.TIMx_Period				= 2500;  // 250ms
    blinkyTimer.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;
    blinkyTimer.TIMx_Config.TIMx_InterruptEnable 	= TIMER_INT_ENABLE;
    timer_Config(&blinkyTimer);
    timer_SetState(&blinkyTimer, TIMER_ON);

    /* FIN de configuración de Led de estado y su timer */
}


void config_i2c(void){
    pinSCL.pGPIOx = GPIOB;
    pinSCL.pinConfig.GPIO_PinNumber = PIN_8;
    pinSCL.pinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    pinSCL.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_OPENDRAIN;
    pinSCL.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_FAST;
    pinSCL.pinConfig.GPIO_PinAltFunMode = AF4;
    gpio_Config(&pinSCL);


    pinSDA.pGPIOx = GPIOB;
    pinSDA.pinConfig.GPIO_PinNumber = PIN_9;
    pinSDA.pinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    pinSDA.pinConfig.GPIO_PinOutputType = GPIO_OTYPE_OPENDRAIN;
    pinSDA.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_FAST;
    pinSDA.pinConfig.GPIO_PinAltFunMode = AF4;
    gpio_Config(&pinSDA);

    i2c_handler.pI2Cx = I2C1;
    i2c_handler.i2c_mainClock = I2C_MAIN_CLOCK_16_MHz;
    i2c_handler.i2c_mode = eI2C_MODE_FM;
    i2c_handler.slaveAddress = VL53L3CX_I2C_ADDR;
    i2c_Config(&i2c_handler);
}






























