################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/CMSIS/Device/ST/STM32MP1xx/Source/Templates/system_stm32mp1xx.c 

OBJS += \
./Drivers/CMSIS/system_stm32mp1xx.o 

C_DEPS += \
./Drivers/CMSIS/system_stm32mp1xx.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/CMSIS/system_stm32mp1xx.o: C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/CMSIS/Device/ST/STM32MP1xx/Source/Templates/system_stm32mp1xx.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -g3 -DUSE_HAL_DRIVER -DSTM32MP157Cxx -DCORE_CM4 -DDEBUG -DSTM32 -DSTM32MP1 -DSTM32MP15xx -DSTM32MP157CACx -DSTM32MP15xx_DISCO -DSTM32MP157C_DK2 -c -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/OpenSSL/inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/Core/Inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/STM32MP1xx_HAL_Driver/Inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/STM32MP1xx_HAL_Driver/Inc/Legacy" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/BSP/STM32MP15xx_DISCO" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/CMSIS/Device/ST/STM32MP1xx/Include" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/CMSIS/Include" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/tiny-AES-c-master" -O0 -ffunction-sections -fdata-sections -Wall -fmessage-length=0 -fstack-usage -MMD -MP -MF"Drivers/CMSIS/system_stm32mp1xx.d" -MT"$@"  -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

