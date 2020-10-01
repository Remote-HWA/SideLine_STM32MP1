PROG = cortex-A7.elf
SRCS = main_cortex-A7.c

CLEANFILES = $(PROG)
CFLAGS += -Iinc
CFLAGS += -I${CURDIR}
SRCS +=  $(shell find src -type f -name '*.c')
SRCS +=  $(shell find wolfcrypt/src -type f -name '*.c')

OBJ :=  $(SRCS:.c=.o)

all: $(PROG)

$(PROG): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) 

clean:
	rm -f $(CLEANFILES) $(patsubst %.c,%.o, $(SRCS))
