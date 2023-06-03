include Makefile.inc

TARGET:=all

KERNEL=kernel.bin
SOURCES=$(wildcard *.c)

SOURCES+=$(wildcard ./memoryManagement/*.c)
SOURCES+=$(wildcard ./bufferManagement/*.c)
SOURCES+=$(wildcard ./ipcRelated/*.c)
SOURCES+=$(wildcard ./interruptRelated/*.c)
SOURCES+=$(wildcard ./drivers/*.c)

SOURCES_ASM=$(wildcard asm/*.asm)
OBJECTS=$(SOURCES:.c=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
LOADERSRC=loader.asm

ifeq ($(TARGET), all)
	EXCLUDED := ./memoryManagement/buddyMemoryManager.c ./memoryManagement/buddyMemoryManager.o
else ifeq ($(TARGET), buddy)
	EXCLUDED := ./memoryManagement/freeListMemoryManager.c ./memoryManagement/freeListMemoryManager.o
endif

SOURCES:=$(filter-out $(EXCLUDED), $(SOURCES))
OBJECTS:=$(filter-out $(EXCLUDED), $(OBJECTS))

LOADEROBJECT=$(LOADERSRC:.asm=.o)
STATICLIBS=

all: $(KERNEL)

buddy: $(KERNEL)

$(KERNEL): $(LOADEROBJECT) $(OBJECTS) $(STATICLIBS) $(OBJECTS_ASM)
	$(LD) $(LDFLAGS) -T kernel.ld -o $(KERNEL) $(LOADEROBJECT) $(OBJECTS) $(OBJECTS_ASM) $(STATICLIBS)
	$(LD) $(LDFLAGS) -T kernel.ld --oformat=elf64-x86-64 -o kernel.elf $(LOADEROBJECT) $(OBJECTS) $(OBJECTS_ASM) $(STATICLIBS)

%.o: %.c
	$(GCC) $(GCCFLAGS) -I./include -c $< -o $@

%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

$(LOADEROBJECT):
	$(ASM) $(ASMFLAGS) $(LOADERSRC) -o $(LOADEROBJECT)

clean:
	rm -rf asm/*.o *.o *.bin *.elf

.PHONY: all clean
