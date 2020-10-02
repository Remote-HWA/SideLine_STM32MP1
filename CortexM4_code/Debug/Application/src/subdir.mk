################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Application/src/lock_resource.c \
../Application/src/main.c 

OBJS += \
./Application/src/lock_resource.o \
./Application/src/main.o 

C_DEPS += \
./Application/src/lock_resource.d \
./Application/src/main.d 


# Each subdirectory must supply rules for building sources it contributes
Application/src/%.o: ../Application/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DSTM32MP157Cxx -DCORE_CM4 -DUSE_HAL_DRIVER -I"/home/joseph/Documents/SideLine_STM32MP/CortexM4_code/Application/inc" -I/home/joseph/Documents/SideLine_STM32MP/STM32MP1_Cube/Drivers/BSP/STM32MP15xx_DISCO -I/home/joseph/Documents/SideLine_STM32MP/STM32MP1_Cube/Drivers/STM32MP1xx_HAL_Driver/Inc/Legacy -I/home/joseph/Documents/SideLine_STM32MP/STM32MP1_Cube/Drivers/CMSIS/Include -I/home/joseph/Documents/SideLine_STM32MP/STM32MP1_Cube/Drivers/STM32MP1xx_HAL_Driver/Inc -I/home/joseph/Documents/SideLine_STM32MP/STM32MP1_Cube/Drivers/CMSIS/Device/ST/STM32MP1xx/Include -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


