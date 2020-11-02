SRCS = main_cortex-A7.c
PROG = SideLine_CA7.elf
CC = C:\SysGCC\stm32mp1\usr\bin\arm-openstlinux_weston-linux-gnueabi\arm-openstlinux_weston-linux-gnueabi-gcc.exe
SRC_FOLDER=src
SRC_FOLDER2=slope-master\slope\source\slope
CLEANFILES = $(PROG)

CFLAGS += -I..\CortexA7_code\inc
CFLAGS += -Islope-master\slope\include
CFLAGS += -Islope-master\slope\source -g


#for GUI

CFLAGS +=-IC:\SysGCC\stm32mp1\cortexa7t2hf-neon-vfpv4-openstlinux_weston-linux-gnueabi\sysroot\usr\include\gtk-3.0
CFLAGS +=-IC:\SysGCC\stm32mp1\cortexa7t2hf-neon-vfpv4-openstlinux_weston-linux-gnueabi\sysroot\usr\include\gtk-3.0
CFLAGS +=-IC:\SysGCC\stm32mp1\cortexa7t2hf-neon-vfpv4-openstlinux_weston-linux-gnueabi\sysroot\usr\include\glib-2.0
CFLAGS +=-IC:\SysGCC\stm32mp1\cortexa7t2hf-neon-vfpv4-openstlinux_weston-linux-gnueabi\sysroot\usr\lib\glib-2.0\include
CFLAGS +=-IC:\SysGCC\stm32mp1\cortexa7t2hf-neon-vfpv4-openstlinux_weston-linux-gnueabi\sysroot\usr\include\pango-1.0
CFLAGS += -IC:\SysGCC\stm32mp1\cortexa7t2hf-neon-vfpv4-openstlinux_weston-linux-gnueabi\sysroot\usr\include\cairo
CFLAGS += -IC:\SysGCC\stm32mp1\cortexa7t2hf-neon-vfpv4-openstlinux_weston-linux-gnueabi\sysroot\usr\include\gdk-pixbuf-2.0
CFLAGS += -IC:\SysGCC\stm32mp1\cortexa7t2hf-neon-vfpv4-openstlinux_weston-linux-gnueabi\sysroot\usr\include\atk-1.0
LDFLAGS += -pthread -lgtk-3 -lgdk-3 -latk-1.0 -lgio-2.0 -lpangoft2-1.0 -lgdk_pixbuf-2.0 -lpangocairo-1.0 -lcairo -lpango-1.0 -lfreetype -lfontconfig -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lrt -lglib-2.0


CFLAGS += -march=armv7ve -O0 -mthumb -Wall -lm -mfpu=neon-vfpv4 --sysroot=C:\SysGCC\stm32mp1\cortexa7t2hf-neon-vfpv4-openstlinux_weston-linux-gnueabi\sysroot -mfloat-abi=hard -mcpu=cortex-a7
SEARCHC = $(addsuffix /*.c ,$(SRC_FOLDER))
SRCS += $(wildcard $(SEARCHC))

SEARCHC2 = $(addsuffix /*.c ,$(SRC_FOLDER2))
SRCS += $(wildcard $(SEARCHC2))

OBJ :=  $(SRCS:.c=.o)
$(info SRCS is [${SRCS}])

all: $(PROG)

$(PROG): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) 

clean:
	rm -f $(CLEANFILES) $(patsubst %.c,%.o, $(SRCS))
