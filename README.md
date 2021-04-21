# SideLine - How Delay-Lines (May) Leak Secrets from your SoC

## About SideLine

SideLine is a novel side-channel vector based on delay-line components widely implemented in high-end SoCs. In the associated paper, we provide a detailed method on how to access and convert delay-line data into power consumption information and we demonstrate that these entities can be used to perform remote power side-channel attacks. We report experiments carried out on two SoCs from distinct vendors and we recount several core-vs-core attack scenarios in which an adversary process located in one processor core aims at eavesdropping the activity of a victim process located in another core. For each scenario, we demonstrate the adversary ability to fully recover the secret key of an OpenSSL AES running in the victim core. Even more detrimental, we show that these attacks are still practicable if the victim or the attacker program runs over an operating system.

<p align="center">
<img src="https://user-images.githubusercontent.com/67143135/115582164-f5724e00-a2c8-11eb-9094-9b57757f1361.PNG" width="600" height="200">
</p>
<p align="center"> Figure 1: The three attack scenarios presented in SideLine<p align="center">
  
## Content

A demo reproducing the SideLine attack and more!

## Requirements
- A STMicroelectronics STM32MP1 based development board. (here STM32MP157x-DK2).
- A micro SD card.

## Tutorial: Run the Demo

- Clone this github repo: ``git clone https://github.com/Remote-HWA/SideLine_STM32MP1``
- Follow [this](https://wiki.st.com/stm32mpu/wiki/Getting_started/STM32MP1_boards/STM32MP157x-DK2/Let%27s_start/Populate_the_target_and_boot_the_image) tutorial to populate the SD card with OpenSTLinux.
- Export the SideLine_CM4.elf and SideLine_CA7.elf executables into the STM32MP1 ``/home`` directory.
- From the board touchscreen console or through SSH, run the following commands to launch the demo ``chmod +x SideLine_CA7.elf`` then ``./SideLine_CA7.elf``.

<p align="center">
<img src="https://user-images.githubusercontent.com/67143135/115577648-e6899c80-a2c4-11eb-9b57-e8b00dfeec03.PNG" width="600" height="350">
</p>
<p align="center"> Figure 2: Welcome Prompt<p align="center">
  
- Under the welcome prompt, type ``view 1000`` to display 1000 delay-line-based oscilloscope **view** samples. (You can recalibrate the delay-lines by interacting with the touchscreen).
- Type ``aes 0 5000 10000 1`` to display 10000 traces of the tiny **AES** encryption computed on the CM4 processor.
- Type ``cpa 0 150 5000000 0`` to start a correlation power analysis (**CPA**) on the OpenSSL AES. This can take several hours depending on the number of traces used. The first guesses usually emerge after around 1 million traces.

<p align="center">
<img src="https://user-images.githubusercontent.com/67143135/115581814-a4faf080-a2c8-11eb-9cc4-d3b6af99fdff.png" width="800" height="200">
</p>
<p align="center"> Figure 3: SideLine Demo Modes<p align="center">

## Build your own SideLine App

This sideline demo was built on Windows. You can reproduce the following setup to recompile the executables.

- The demo uses a **STM32MP157C-DK2** board.
- The demo uses **OpenSTlinux version 2.0.0** [here](https://www.st.com/content/st_com/en/products/embedded-software/mcu-mpu-embedded-software/stm32-embedded-software/stm32-mpu-openstlinux-distribution/stm32mp1starter.html) with *FlashLayout_sdcard_stm32mp157c-dk2-trusted.tsv* configuration.
- SideLine_CM4.elf was compiled using **STM32CubeIDE** and the MCU Toolchain *(GNU Tools for STM32 (9-2020-q2-update))*.
- SideLine_CA7.elf was compiled using the **SYSGCC** *arm-openstlinux_weston-linux-gnueabi* toolchain available [here](https://gnutoolchains.com/stm32mp1/) version *stm32mp1-gcc8.2.0-r2.exe*









