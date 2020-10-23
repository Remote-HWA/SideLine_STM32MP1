SRCS = main_cortex-A7.c
PROG = SideLine_CA7.elf
CC = C:\SysGCC\stm32mp1\usr\bin\arm-openstlinux_weston-linux-gnueabi\arm-openstlinux_weston-linux-gnueabi-gcc.exe
SRC_FOLDER=src
CLEANFILES = $(PROG)

CFLAGS += -I..\CortexA7_code\inc
CFLAGS += -march=armv7ve -O0 -mthumb -Wall -lm -mfpu=neon-vfpv4 --sysroot=C:\SysGCC\stm32mp1\cortexa7t2hf-neon-vfpv4-openstlinux_weston-linux-gnueabi\sysroot -mfloat-abi=hard -mcpu=cortex-a7
SEARCHC = $(addsuffix /*.c ,$(SRC_FOLDER))
SRCS = $(wildcard $(SEARCHC))

OBJ :=  $(SRCS:.c=.o)
$(info SRCS is [${SRCS}])

all: $(PROG)

$(PROG): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) 

clean:
	rm -f $(CLEANFILES) $(patsubst %.c,%.o, $(SRCS))
