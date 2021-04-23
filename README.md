<p align="center">
<img src="https://josephgravellier.github.io/sideline/media/sideline_logo_tr.png" width="250" height="200">
</p>

# SideLine - How Delay-Lines (May) Leak Secrets from your SoC

SideLine is a software-based power side-channel analysis vector. It uses delay-lines (located in SoC memory controllers) as power meters.

## Content

This repository provides: 
- The **source code** required to reproduce the STM32MP1 SideLine attack.
- A **tutorial** to build a SideLine demonstration on a STM32MP157C-DK2 board.

## Tutorial Requirements
- A STMicroelectronics **STM32MP1** based development board. (here *STM32MP157C-DK2*).
- A **micro SD** card.

### Prerequisite 1: Install OpenSTLinux

If OpenSTLinux is not pre-installed on your board, you'll need to download and flash it on the SD card.\
**The steps below should be followed to run the SideLine executables and the demo.**\
*You can consult [this](https://wiki.st.com/stm32mpu/wiki/Getting_started/STM32MP1_boards/STM32MP157x-DK2/Let%27s_start/Populate_the_target_and_boot_the_image) tutorial for more  informations.*

1. **Download** the STM32MP15x OpenSTLinux Starter Package v2.0.0 from [here](https://www.st.com/en/embedded-software/stm32mp1starter.html) and extract it. (You will be asked to create a ST account). **Make sure that you selected version 2.0.0**.
2. **Download** and install STM32CubeProgrammer from [here](https://www.st.com/en/development-tools/stm32cubeprog.html).
3. **Set** the boot switches 1 & 2 (located at the back of the board) to the OFF position.
4. **Connect** the USB Type A to Type C cable between PC and CN7/USB_OTG port of the STM32MP157C-DK2 board.
5. **Power** up the board and press the reset button.
6. **Launch** the STM32CubeProgrammer GUI.
7. **Select** "Open File" tab and select the "FlashLayout_sdcard_stm32mp157c-dk2-trusted.tsv" file in the previously installed OpenSTLinux Starter Package.
8. **Modify** Binary path to: /your_path/stm32mp1-openstlinux-5.4-dunfell-mp1-20-06-24/images/stm32mp1.
9. **Click** on "Download" to start the flashing process (the installation takes several minutes).
10. **Set** the boot switches (located at the back of the board) to the ON position.
11. **Power up** the board and press the reset button.

*After few seconds, the board starts and automatically goes through the ST prompt screen.*

### Prerequisite 2: Communicate with the board

If you haven't setup an ethernet connection with the board, you'll need to enable eth
**The steps below should be followed to run the SideLine executables and the demo.**\
*You can consult [this](https://wiki.st.com/stm32mpu/wiki/How_to_configure_ethernet_interface) tutorial for more  informations.*

1. **Power up** the board and press the reset button.
2. **Connect** a micro USB cable between PC and the CN11/ST-LINK port of the STM32MP157x-DK2 board.
3. **Setup** a serial connection with the board (@115200 bauds) using PuTTY or screen. *You should be able to communicate with the board at this point*.
5. **Connect** the PC to the board using an ethernet cable
4. **Type** ``ifconfig eth0 192.168.0.15``
7. **Set** your PC IPv4 IP address to ``192.168.0.16``
8. **Setup** the SSH connection using Putty or by typing ``ssh root@192.168.0.15`` on your PC

*The board is ready, you can run the demo !*

## Tutorial: Run the Demo

1. **Download** the content of this repository.
2. **Export** the SideLine_CM4.elf and SideLine_CA7.elf executables into the STM32MP1 board using WinSCP or by entering the following commands on a bash prompt:
``scp /path_to_sideline/CortexA7_code/SideLine_CA7.elf root@192.168.0.15:/home/root``\
``scp /path_to_sideline/CortexM4_code/SideLine_CM4.elf root@192.168.0.15:/home/root``\
3. **Connect** to the board using Putty or by typing ``ssh root@192.168.0.15`` on your PC.
4. **Run** the following commands to start the demo on the STM32MP1 board:\
``mkdir SideLine``
``mv -t SideLine SideLine_CA7.elf SideLine_CM4.elf``
``chmod +x -R SideLine``\
``./SideLine/SideLine_CA7.elf``\

<p align="center">
<img src="https://user-images.githubusercontent.com/67143135/115577648-e6899c80-a2c4-11eb-9b57-e8b00dfeec03.PNG" width="600" height="350">
</p>
<p align="center"> Figure 2: The welcome prompt appears! <p align="center">
 
5. **Type** ``view 1000`` to display 1000 delay-line-based oscilloscope **view** samples. (You can recalibrate the delay-lines by interacting with the touchscreen).
    - **Press** the **Calibrate** button to recalibrate the delay-lines.
    - **Press** the **Rescale** button to adjust zoom.
7. **Type** ``auto`` to calibrate the delay-lines.
8. **Type** ``aes 0 5000 10000 1`` to display 10000 traces of the tiny **AES** encryption computed on the CM4 processor.
9. **Type** ``cpa 0 150 5000000 0`` to start a correlation power analysis (**CPA**) on the OpenSSL AES. This can take several hours depending on the number of traces used. The first guesses usually emerge after around 1 million traces.
    - **Press** the **Update** button to recompute the CPA results.
    - **Press** the **Byte++** button to observe another key byte.

<p align="center">
<img src="https://user-images.githubusercontent.com/67143135/115581814-a4faf080-a2c8-11eb-9cc4-d3b6af99fdff.png" width="800" height="200">
</p>
<p align="center"> Figure 3: SideLine Demo Modes<p align="center">

## Build your own SideLine App

This SideLine demo was built on Windows. You can reproduce the following setup to recompile the executables.

- The demo uses a **STM32MP157C-DK2** board.
- The demo uses **OpenSTlinux version 2.0.0** [here](https://www.st.com/content/st_com/en/products/embedded-software/mcu-mpu-embedded-software/stm32-embedded-software/stm32-mpu-openstlinux-distribution/stm32mp1starter.html) with *FlashLayout_sdcard_stm32mp157c-dk2-trusted.tsv* configuration.
- SideLine_CM4.elf was compiled using **STM32CubeIDE** and the MCU Toolchain *(GNU Tools for STM32 (9-2020-q2-update))*.
- SideLine_CA7.elf was compiled using the **SYSGCC** *arm-openstlinux_weston-linux-gnueabi* toolchain available [here](https://gnutoolchains.com/stm32mp1/) version *stm32mp1-gcc8.2.0-r2.exe*









