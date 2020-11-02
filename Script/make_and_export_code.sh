#source /home/joseph/STM32_workspace/STM32MP15-Ecosystem-v1.1.0/Developer-Package/SDK/environment-setup-cortexa7t2hf-neon-vfpv4-openstlinux_weston-linux-gnueabi
#cd CortexA7_code/
#make clean
#make
#cd ..
cd CortexA7_code
make 
cd ..
ssh root@192.168.0.15 'rm -r SideLine; mkdir SideLine'
scp CortexA7_code/SideLine_CA7.elf CortexM4_code/Debug/SideLine_CM4.elf Script/launch.sh root@192.168.0.15:/home/root/SideLine
ssh root@192.168.0.15 'chmod -R 777 SideLine'