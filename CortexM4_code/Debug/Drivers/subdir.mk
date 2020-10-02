################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/stm32mp15xx_disco.c \
../Drivers/stm32mp1xx_hal.c \
../Drivers/stm32mp1xx_hal_cortex.c \
../Drivers/stm32mp1xx_hal_cryp.c \
../Drivers/stm32mp1xx_hal_cryp_ex.c \
../Drivers/stm32mp1xx_hal_dma.c \
../Drivers/stm32mp1xx_hal_dma_ex.c \
../Drivers/stm32mp1xx_hal_gpio.c \
../Drivers/stm32mp1xx_hal_hsem.c \
../Drivers/stm32mp1xx_hal_pwr.c \
../Drivers/stm32mp1xx_hal_pwr_ex.c \
../Drivers/stm32mp1xx_hal_rcc.c \
../Drivers/stm32mp1xx_hal_rcc_ex.c \
../Drivers/stm32mp1xx_hal_sd.c \
../Drivers/stm32mp1xx_hal_sd_ex.c \
../Drivers/stm32mp1xx_hal_uart.c \
../Drivers/stm32mp1xx_hal_uart_ex.c \
../Drivers/stm32mp1xx_ll_sdmmc.c \
../Drivers/system_stm32mp1xx.c 

OBJS += \
./Drivers/stm32mp15xx_disco.o \
./Drivers/stm32mp1xx_hal.o \
./Drivers/stm32mp1xx_hal_cortex.o \
./Drivers/stm32mp1xx_hal_cryp.o \
./Drivers/stm32mp1xx_hal_cryp_ex.o \
./Drivers/stm32mp1xx_hal_dma.o \
./Drivers/stm32mp1xx_hal_dma_ex.o \
./Drivers/stm32mp1xx_hal_gpio.o \
./Drivers/stm32mp1xx_hal_hsem.o \
./Drivers/stm32mp1xx_hal_pwr.o \
./Drivers/stm32mp1xx_hal_pwr_ex.o \
./Drivers/stm32mp1xx_hal_rcc.o \
./Drivers/stm32mp1xx_hal_rcc_ex.o \
./Drivers/stm32mp1xx_hal_sd.o \
./Drivers/stm32mp1xx_hal_sd_ex.o \
./Drivers/stm32mp1xx_hal_uart.o \
./Drivers/stm32mp1xx_hal_uart_ex.o \
./Drivers/stm32mp1xx_ll_sdmmc.o \
./Drivers/system_stm32mp1xx.o 

C_DEPS += \
./Drivers/stm32mp15xx_disco.d \
./Drivers/stm32mp1xx_hal.d \
./Drivers/stm32mp1xx_hal_cortex.d \
./Drivers/stm32mp1xx_hal_cryp.d \
./Drivers/stm32mp1xx_hal_cryp_ex.d \
./Drivers/stm32mp1xx_hal_dma.d \
./Drivers/stm32mp1xx_hal_dma_ex.d \
./Drivers/stm32mp1xx_hal_gpio.d \
./Drivers/stm32mp1xx_hal_hsem.d \
./Drivers/stm32mp1xx_hal_pwr.d \
./Drivers/stm32mp1xx_hal_pwr_ex.d \
./Drivers/stm32mp1xx_hal_rcc.d \
./Drivers/stm32mp1xx_hal_rcc_ex.d \
./Drivers/stm32mp1xx_hal_sd.d \
./Drivers/stm32mp1xx_hal_sd_ex.d \
./Drivers/stm32mp1xx_hal_uart.d \
./Drivers/stm32mp1xx_hal_uart_ex.d \
./Drivers/stm32mp1xx_ll_sdmmc.d \
./Drivers/system_stm32mp1xx.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/%.o: ../Drivers/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DSTM32MP157Cxx -DCORE_CM4 -DUSE_HAL_DRIVER -I"/home/joseph/Documents/SideLine_STM32MP/CortexM4_code/Application/inc" -I/home/joseph/Documents/SideLine_STM32MP/STM32MP1_Cube/Drivers/BSP/STM32MP15xx_DISCO -I/home/joseph/Documents/SideLine_STM32MP/STM32MP1_Cube/Drivers/STM32MP1xx_HAL_Driver/Inc/Legacy -I/home/joseph/Documents/SideLine_STM32MP/STM32MP1_Cube/Drivers/CMSIS/Include -I/home/joseph/Documents/SideLine_STM32MP/STM32MP1_Cube/Drivers/STM32MP1xx_HAL_Driver/Inc -I/home/joseph/Documents/SideLine_STM32MP/STM32MP1_Cube/Drivers/CMSIS/Device/ST/STM32MP1xx/Include -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


