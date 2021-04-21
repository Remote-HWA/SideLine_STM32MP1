# SideLine - How Delay-Lines (May) Leak Secrets from your SoC

## About SideLine

SideLine is a novel side-channel vector based on delay-line components widely implemented in high-end SoCs. In the associated paper, we provide a detailed method on how to access and convert delay-line data into power consumption information and we demonstrate that these entities can be used to perform remote power side-channel attacks. We report experiments carried out on two SoCs from distinct vendors and we recount several core-vs-core attack scenarios in which an adversary process located in one processor core aims at eavesdropping the activity of a victim process located in another core. For each scenario, we demonstrate the adversary ability to fully recover the secret key of an OpenSSL AES running in the victim core. Even more detrimental, we show that these attacks are still practicable if the victim or the attacker program runs over an operating system.

<p align="center">
<img src="https://user-images.githubusercontent.com/67143135/85726797-bac67600-b6f6-11ea-9162-8daf8975c3bd.png" width="700" height="250">
</p>
<p align="center"> Figure 1: The three attack scenarios presented in SideLine<p align="center">
  
## Content

A demo reproducing the SideLine attack and more!

## Requirements
- A STMicroelectronics STM32MP1 based development board. (here STM32MP157x-DK2)
- A micro SD card (e.g 8Go)

## Tutorial

- Follow [this](https://wiki.st.com/stm32mpu/wiki/Getting_started/STM32MP1_boards/STM32MP157x-DK2/Let%27s_start/Populate_the_target_and_boot_the_image) tutorial to populate the SD card with OpenSTLinux.
- Load the SideLine_CM4.elf and SideLine_CA7.elf executables in the STM32MP1 ``/home`` directory.
- From the board (using a keyboard) or through SSH type the following commands to launch the demo ``$chmod +x SideLine_CA7.elf`` then ``./SideLine_CA7.elf``.
- Under the welcome prompt type ``help`` to display the command helper.
- Type ``view 10000`` to display 10000 delay-line-based oscilloscope samples. (You can recalibrate the delay-lines by interacting with the touchscreen).
- Type ``aes 0 5000 10000`` to display 10000 traces of the tiny AES encryption computed on the CM4 processor.

## Versions used for the demo

- Our demo uses a STM32MP157C-DK2 board
- Our demo uses OpenSTlinux version 2.0.0 [here](https://www.st.com/content/st_com/en/products/embedded-software/mcu-mpu-embedded-software/stm32-embedded-software/stm32-mpu-openstlinux-distribution/stm32mp1starter.html) with FlashLayout_sdcard_stm32mp157c-dk2-trusted configuration
- SideLine_CM4.elf was compiled using STM32CubeIDE 
- SideLine_CA7.elf was compiled using the sysgcc arm-openstlinux_weston-linux-gnueabi toolchain available [here](https://gnutoolchains.com/stm32mp1/) version stm32mp1-gcc8.2.0-r2.exe









