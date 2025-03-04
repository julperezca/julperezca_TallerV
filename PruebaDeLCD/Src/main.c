#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stm32f4xx.h>
#include "gpio_driver_hal.h"
#include "timer_driver_hal.h"
#include "usart_driver_hal.h"
#include "i2c_driver_hal.h"
#include "systick_driver.h"
//#include "LCD_driver.h"

// Dirección del adaptador PCF8574 (cambia si es necesario)
#define LCD_I2C_ADDR  0x21  //  pantalla 18
//#define LCD_I2C_ADDR  0x20   // pantalla 39
//#define LCD_I2C_ADDR  0x24	// pantalla 11

// Definición de bits de control del PCF8574
#define RS_SELECT_ID         (0b0 << 0)
#define RS_SELECT_DR         (0b1 << 0)
#define RW_SELECT_WRITE      (0b0 << 1)
#define RW_SELECT_READ       (0b1 << 1)
#define EN_DISABLED          (0b0 << 2)
#define EN_ENABLE            (0b1 << 2)
#define LED_DISABLED         (0b0 << 3)
#define LED_ENABLE           (0b1 << 3)


#define	ENTER_MODE_4_BIT     (0b0011 << 4)
#define	RF_MODE_4_BIT        (0b0010 << 4)
#define	RF_LINE_CHARACTER    (0b00101000)
#define	EM_CURSOR_RIGHT      (0b00000110)
#define	ADD                  (0b1000 << 4)

#define BITS_H                (0b1111 << 4)
#define BITS_L                (0b1111)

#define	TURN_ON_DISPLAY      (0b00001000)
#define	DIPLAY_ENABLE        (0b00001100)
#define	CURSOR_BLINKY        (0b00001111)
#define	CLEAR_DISPLAY        (0b00000001)


#define D4                    4
#define D5                    5
#define D6                    6
#define D7                    7



// Variables para el I2C
I2C_Handler_t lcdI2C = {0};
GPIO_Handler_t pinSCL = {0};
GPIO_Handler_t pinSDA = {0};

// Prototipos de funciones
void initSystem(void);
void config_I2C(void);
void LCD_Init(void);
void LCD_SendNibble(uint8_t data);
void LCD_Command(uint8_t cmd);
void LCD_Char(char data);
void LCD_Print(char *str);
void LCD_SendByte(uint8_t data, uint8_t mode);
void LCD_ToggleEnable(uint8_t data);


/**
 * Función principal del programa.
 */
int main(void) {
	systickConfig(CLOCK_SOURCE_16MHz);

    initSystem();

    msDelay(100);

    while (1) {
    }
    return 0;
}

/**
 * Inicializa los periféricos del sistema.
 */
void initSystem(void) {
    // Configurar I2C
    config_I2C();
}

/**
 * Configuración de I2C en STM32.
 */
void config_I2C(void) {
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

    lcdI2C.pI2Cx = I2C1;
    lcdI2C.i2c_mainClock = I2C_MAIN_CLOCK_16_MHz;
    lcdI2C.i2c_mode = eI2C_MODE_SM;
    lcdI2C.slaveAddress = LCD_I2C_ADDR;
    i2c_Config(&lcdI2C);
}

/**
 * Inicializa la LCD en modo 4 bits.
 */
void LCD_Init(void) {
    msDelay(50); // Esperar que la LCD encienda

    LCD_SendNibble(0x30);
    msDelay(5);
    LCD_SendNibble(0x30);
    msDelay(1);
    LCD_SendNibble(0x30);
    msDelay(1);
    LCD_SendNibble(0x20);  // Cambio a 4 bits
    msDelay(1);

    // Configuración en 4 bits
    LCD_Command(0x28);  // 2 líneas, 5x8 caracteres
    LCD_Command(0x0C);  // Display ON, Cursor OFF
    LCD_Command(0x06);  // Modo de escritura: Incremento
    LCD_Command(0x01);  // Limpiar pantalla
    msDelay(2);
}
