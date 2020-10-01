source /home/joseph/STM32_workspace/STM32MP15-Ecosystem-v1.1.0/Developer-Package/SDK/environment-setup-cortexa7t2hf-neon-vfpv4-openstlinux_weston-linux-gnueabi
cd CortexA7_code/
make clean
make
cd ..
ssh root@192.168.0.15 'rm -r SideLine; mkdir SideLine'
scp CortexA7_code/cortex-A7.elf root@192.168.0.15:/home/root/SideLine/cortex-A7.elf
scp CortexM4_code/cortex-M4.elf root@192.168.0.15:/home/root/SideLine/cortex-M4.elf
