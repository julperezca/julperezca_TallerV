/*
 * Con base en la API, se modificó todo el HAL _ I2C
 * adaptando el i2c_driver realizado en tallerV
 * Después de mucho trabajo(5 días seguidos) se logró
 * realizar la adaptación de las funciones de I2C+entendiemiento
 * de los registros para obtener datos del sensor
 *
 * */








#include "vl53l0x_platform.h"
#include "vl53l0x_api.h"
#include "i2c_driver_hal.h"
#include "stm32f4xx.h"
#include  "systick_driver.h"
#include <string.h>

#define I2C_TIME_OUT_BASE   10
#define I2C_TIME_OUT_BYTE   1
#define VL53L0X_OsDelay(...) HAL_Delay(2)


#ifndef HAL_I2C_MODULE_ENABLED

#endif
//extern I2C_HandleTypeDef hi2c1;
//#define VL53L0X_pI2cHandle    (&hi2c1)

/* when not customized by application define dummy one */
#ifndef VL53L0X_GetI2cBus
/** This macro can be overloaded by user to enforce i2c sharing in RTOS context
 */
#   define VL53L0X_GetI2cBus(...) (void)0
#endif

#ifndef VL53L0X_PutI2cBus
/** This macro can be overloaded by user to enforce i2c sharing in RTOS context
 */
#   define VL53L0X_PutI2cBus(...) (void)0
#endif

#ifndef VL53L0X_OsDelay
#   define  VL53L0X_OsDelay(...) (void)0
#endif


uint8_t _I2CBuffer[64];


// the ranging_sensor_comms.dll will take care of the page selection
VL53L0X_Error VL53L0X_WriteMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count) {


    if (count > sizeof(_I2CBuffer) - 1) {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }

    VL53L0X_GetI2cBus();

    i2c_WriteManyRegisters(Dev->I2cHandle, index, pdata, count);


    VL53L0X_PutI2cBus();
    return 0;
}

// the ranging_sensor_comms.dll will take care of the page selection
VL53L0X_Error VL53L0X_ReadMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count) {

    VL53L0X_GetI2cBus();

    readManyRegisters(Dev->I2cHandle, index, pdata, count+1);


    VL53L0X_PutI2cBus();
    return 0;
}

VL53L0X_Error VL53L0X_WrByte(VL53L0X_DEV Dev, uint8_t index, uint8_t data) {


    _I2CBuffer[0] = index;
    _I2CBuffer[1] = data;

    VL53L0X_GetI2cBus();
   i2c_WriteSingleRegister(Dev->I2cHandle, _I2CBuffer[0], _I2CBuffer[1]);

    VL53L0X_PutI2cBus();
    return 0;
}

VL53L0X_Error VL53L0X_WrWord(VL53L0X_DEV Dev, uint8_t index, uint16_t data) {

     uint8_t registro;
     registro = index;
    _I2CBuffer[0] = data >> 8;
    _I2CBuffer[1] = data & 0x00FF;

    VL53L0X_GetI2cBus();
    i2c_WriteManyRegisters(Dev->I2cHandle, registro, _I2CBuffer, 2);

    VL53L0X_PutI2cBus();
    return 0;
}

VL53L0X_Error VL53L0X_WrDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t data) {
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;

    uint8_t registro;
    registro = index;

    _I2CBuffer[0] = (data >> 24) & 0xFF;
    _I2CBuffer[1] = (data >> 16) & 0xFF;
    _I2CBuffer[2] = (data >> 8)  & 0xFF;
    _I2CBuffer[3] = (data >> 0 ) & 0xFF;
    VL53L0X_GetI2cBus();
    i2c_WriteManyRegisters(Dev->I2cHandle, registro, _I2CBuffer, 4);

    VL53L0X_PutI2cBus();
    return Status;
}

VL53L0X_Error VL53L0X_UpdateByte(VL53L0X_DEV Dev, uint8_t index, uint8_t AndData, uint8_t OrData) {

    uint8_t data;

    data = i2c_ReadSingleRegister(Dev->I2cHandle, index);


    data = (data & AndData) | OrData;
    i2c_WriteSingleRegister(Dev->I2cHandle, index, data);

    return 0;
}

VL53L0X_Error VL53L0X_RdByte(VL53L0X_DEV Dev, uint8_t index, uint8_t *data) {

    VL53L0X_GetI2cBus();
    *data = i2c_ReadSingleRegister(Dev->I2cHandle, index);

    VL53L0X_PutI2cBus();
    return 0;
}

VL53L0X_Error VL53L0X_RdWord(VL53L0X_DEV Dev, uint8_t index, uint16_t *data) {

	    // Bloqueamos el bus I2C antes de la comunicación
	    VL53L0X_GetI2cBus();

	    // Leer 2 bytes consecutivos desde el registro 'index'
	    readManyRegisters(Dev->I2cHandle, index, _I2CBuffer, 3);

	    // Combinar los dos bytes en un solo uint16_t
	    *data = ((uint16_t)_I2CBuffer[0] << 8) | (uint16_t)_I2CBuffer[1];
	    return 0;
}

VL53L0X_Error VL53L0X_RdDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t *data) {

	readManyRegisters(Dev->I2cHandle, index, _I2CBuffer, 5);

    *data = ((uint32_t)_I2CBuffer[0]<<24) + ((uint32_t)_I2CBuffer[1]<<16) + ((uint32_t)_I2CBuffer[2]<<8) + (uint32_t)_I2CBuffer[3];


    VL53L0X_PutI2cBus();
    return 0;
}

VL53L0X_Error VL53L0X_PollingDelay(VL53L0X_DEV Dev) {
    VL53L0X_Error status = VL53L0X_ERROR_NONE;

    // do nothing
    msDelay(2);
    return status;
}

//end of file
