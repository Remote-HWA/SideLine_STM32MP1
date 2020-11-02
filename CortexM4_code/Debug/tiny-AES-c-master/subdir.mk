################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../tiny-AES-c-master/aes.c 

OBJS += \
./tiny-AES-c-master/aes.o 

C_DEPS += \
./tiny-AES-c-master/aes.d 


# Each subdirectory must supply rules for building sources it contributes
tiny-AES-c-master/aes.o: ../tiny-AES-c-master/aes.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -g3 -DUSE_HAL_DRIVER -DSTM32MP157Cxx -DCORE_CM4 -DDEBUG -DSTM32 -DSTM32MP1 -DSTM32MP15xx -DSTM32MP157CACx -DSTM32MP15xx_DISCO -DSTM32MP157C_DK2 -c -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/OpenSSL/inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/Core/Inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/STM32MP1xx_HAL_Driver/Inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/STM32MP1xx_HAL_Driver/Inc/Legacy" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/BSP/STM32MP15xx_DISCO" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/CMSIS/Device/ST/STM32MP1xx/Include" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/CMSIS/Include" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/tiny-AES-c-master" -O0 -ffunction-sections -fdata-sections -Wall -fmessage-length=0 -fstack-usage -MMD -MP -MF"tiny-AES-c-master/aes.d" -MT"$@"  -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

