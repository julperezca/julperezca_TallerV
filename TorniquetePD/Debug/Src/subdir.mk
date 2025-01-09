################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/magic.c \
../Src/main.c 

OBJS += \
./Src/magic.o \
./Src/main.o 

C_DEPS += \
./Src/magic.d \
./Src/main.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DNUCLEO_F411RE -DSTM32 -DSTM32F4 -DSTM32F411RETx -DSTM32F411xE=1 -c -I"/home/ingfisica/STM32CubeIDE/workspace_ingfisica/CMSIS-repo/STM32Cube_FW_F4_V1.27.0/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"/home/ingfisica/STM32CubeIDE/workspace_ingfisica/CMSIS-repo/STM32Cube_FW_F4_V1.27.0/Drivers/CMSIS/Include" -I"/home/ingfisica/github/julperezca_TallerV/TorniquetePD/Inc" -I"/home/ingfisica/github/julperezca_TallerV/TorniquetePD/Drivers/Inc" -I"/home/ingfisica/github/julperezca_TallerV/PeripheralsDrivers/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/magic.d ./Src/magic.o ./Src/magic.su ./Src/main.d ./Src/main.o ./Src/main.su

.PHONY: clean-Src

