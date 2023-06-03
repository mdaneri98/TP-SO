include Makefile.inc

DIRECTORIES= . ./programs

MODULE=userland.bin
SOURCES:=$(foreach dir,$(DIRECTORIES),$(wildcard $(dir)/[^_]*.c))

SOURCES_ASM=$(wildcard asm/*.asm)
OBJECTS_ASM=$(SOURCES_ASM:asm/%.asm=asm/%.o)

all: $(MODULE)

$(MODULE): $(SOURCES) $(OBJECTS_ASM)
	$(GCC) $(GCCFLAGS) -I./include -T userland.ld _loader.c $(OBJECTS_ASM) $(SOURCES) -o $(MODULE)
	$(GCC) $(GCCFLAGS) -I./include -T userland.ld -Wl,--oformat=elf64-x86-64 _loader.c $(OBJECTS_ASM) $(SOURCES) -o userland.elf

asm/%.o : asm/%.asm
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm -rf *.o *.bin *.elf obj/*.asm.o programs/*.o

.PHONY: all clean print