################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/lock_resource.c \
../Core/Src/main_cortex-M4.c \
../Core/Src/register.c \
../Core/Src/stm32mp1xx_hal_msp.c \
../Core/Src/stm32mp1xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c 

OBJS += \
./Core/Src/lock_resource.o \
./Core/Src/main_cortex-M4.o \
./Core/Src/register.o \
./Core/Src/stm32mp1xx_hal_msp.o \
./Core/Src/stm32mp1xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o 

C_DEPS += \
./Core/Src/lock_resource.d \
./Core/Src/main_cortex-M4.d \
./Core/Src/register.d \
./Core/Src/stm32mp1xx_hal_msp.d \
./Core/Src/stm32mp1xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/lock_resource.o: ../Core/Src/lock_resource.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -g3 -DUSE_HAL_DRIVER -DSTM32MP157Cxx -DCORE_CM4 -DDEBUG -DSTM32 -DSTM32MP1 -DSTM32MP15xx -DSTM32MP157CACx -DSTM32MP15xx_DISCO -DSTM32MP157C_DK2 -c -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/OpenSSL/inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/Core/Inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/STM32MP1xx_HAL_Driver/Inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/STM32MP1xx_HAL_Driver/Inc/Legacy" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/BSP/STM32MP15xx_DISCO" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/CMSIS/Device/ST/STM32MP1xx/Include" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/CMSIS/Include" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/tiny-AES-c-master" -O0 -ffunction-sections -fdata-sections -Wall -fmessage-length=0 -fstack-usage -MMD -MP -MF"Core/Src/lock_resource.d" -MT"$@"  -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Core/Src/main_cortex-M4.o: ../Core/Src/main_cortex-M4.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -g3 -DUSE_HAL_DRIVER -DSTM32MP157Cxx -DCORE_CM4 -DDEBUG -DSTM32 -DSTM32MP1 -DSTM32MP15xx -DSTM32MP157CACx -DSTM32MP15xx_DISCO -DSTM32MP157C_DK2 -c -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/OpenSSL/inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/Core/Inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/STM32MP1xx_HAL_Driver/Inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/STM32MP1xx_HAL_Driver/Inc/Legacy" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/BSP/STM32MP15xx_DISCO" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/CMSIS/Device/ST/STM32MP1xx/Include" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/CMSIS/Include" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/tiny-AES-c-master" -O0 -ffunction-sections -fdata-sections -Wall -fmessage-length=0 -fstack-usage -MMD -MP -MF"Core/Src/main_cortex-M4.d" -MT"$@"  -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Core/Src/register.o: ../Core/Src/register.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -g3 -DUSE_HAL_DRIVER -DSTM32MP157Cxx -DCORE_CM4 -DDEBUG -DSTM32 -DSTM32MP1 -DSTM32MP15xx -DSTM32MP157CACx -DSTM32MP15xx_DISCO -DSTM32MP157C_DK2 -c -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/OpenSSL/inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/Core/Inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/STM32MP1xx_HAL_Driver/Inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/STM32MP1xx_HAL_Driver/Inc/Legacy" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/BSP/STM32MP15xx_DISCO" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/CMSIS/Device/ST/STM32MP1xx/Include" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/CMSIS/Include" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/tiny-AES-c-master" -O0 -ffunction-sections -fdata-sections -Wall -fmessage-length=0 -fstack-usage -MMD -MP -MF"Core/Src/register.d" -MT"$@"  -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Core/Src/stm32mp1xx_hal_msp.o: ../Core/Src/stm32mp1xx_hal_msp.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -g3 -DUSE_HAL_DRIVER -DSTM32MP157Cxx -DCORE_CM4 -DDEBUG -DSTM32 -DSTM32MP1 -DSTM32MP15xx -DSTM32MP157CACx -DSTM32MP15xx_DISCO -DSTM32MP157C_DK2 -c -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/OpenSSL/inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/Core/Inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/STM32MP1xx_HAL_Driver/Inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/STM32MP1xx_HAL_Driver/Inc/Legacy" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/BSP/STM32MP15xx_DISCO" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/CMSIS/Device/ST/STM32MP1xx/Include" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/CMSIS/Include" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/tiny-AES-c-master" -O0 -ffunction-sections -fdata-sections -Wall -fmessage-length=0 -fstack-usage -MMD -MP -MF"Core/Src/stm32mp1xx_hal_msp.d" -MT"$@"  -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Core/Src/stm32mp1xx_it.o: ../Core/Src/stm32mp1xx_it.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -g3 -DUSE_HAL_DRIVER -DSTM32MP157Cxx -DCORE_CM4 -DDEBUG -DSTM32 -DSTM32MP1 -DSTM32MP15xx -DSTM32MP157CACx -DSTM32MP15xx_DISCO -DSTM32MP157C_DK2 -c -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/OpenSSL/inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/Core/Inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/STM32MP1xx_HAL_Driver/Inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/STM32MP1xx_HAL_Driver/Inc/Legacy" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/BSP/STM32MP15xx_DISCO" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/CMSIS/Device/ST/STM32MP1xx/Include" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/CMSIS/Include" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/tiny-AES-c-master" -O0 -ffunction-sections -fdata-sections -Wall -fmessage-length=0 -fstack-usage -MMD -MP -MF"Core/Src/stm32mp1xx_it.d" -MT"$@"  -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Core/Src/syscalls.o: ../Core/Src/syscalls.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -g3 -DUSE_HAL_DRIVER -DSTM32MP157Cxx -DCORE_CM4 -DDEBUG -DSTM32 -DSTM32MP1 -DSTM32MP15xx -DSTM32MP157CACx -DSTM32MP15xx_DISCO -DSTM32MP157C_DK2 -c -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/OpenSSL/inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/Core/Inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/STM32MP1xx_HAL_Driver/Inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/STM32MP1xx_HAL_Driver/Inc/Legacy" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/BSP/STM32MP15xx_DISCO" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/CMSIS/Device/ST/STM32MP1xx/Include" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/CMSIS/Include" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/tiny-AES-c-master" -O0 -ffunction-sections -fdata-sections -Wall -fmessage-length=0 -fstack-usage -MMD -MP -MF"Core/Src/syscalls.d" -MT"$@"  -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Core/Src/sysmem.o: ../Core/Src/sysmem.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -g3 -DUSE_HAL_DRIVER -DSTM32MP157Cxx -DCORE_CM4 -DDEBUG -DSTM32 -DSTM32MP1 -DSTM32MP15xx -DSTM32MP157CACx -DSTM32MP15xx_DISCO -DSTM32MP157C_DK2 -c -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/OpenSSL/inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/Core/Inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/STM32MP1xx_HAL_Driver/Inc" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/STM32MP1xx_HAL_Driver/Inc/Legacy" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/BSP/STM32MP15xx_DISCO" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/CMSIS/Device/ST/STM32MP1xx/Include" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/STM32Cube_FW_MP1_V1.2.0/Drivers/CMSIS/Include" -I"C:/Users/10055748/Documents/SideLine/SideLine_STM32MP/CortexM4_code/tiny-AES-c-master" -O0 -ffunction-sections -fdata-sections -Wall -fmessage-length=0 -fstack-usage -MMD -MP -MF"Core/Src/sysmem.d" -MT"$@"  -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

