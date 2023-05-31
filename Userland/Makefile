include Makefile.inc

MODULE=userland.bin
SOURCES=$(wildcard [^_]*.c)
SOURCES_ASM=$(wildcard asm/*.asm)
OBJECTS_ASM=$(SOURCES_ASM:asm/%.asm=obj/%.asm.o)

all: $(MODULE)

$(MODULE): $(SOURCES) $(OBJECTS_ASM)
	$(GCC) $(GCCFLAGS) -I./include -T userland.ld _loader.c $(OBJECTS_ASM) $(SOURCES) -o $(MODULE)
	$(GCC) $(GCCFLAGS) -I./include -T userland.ld -Wl,--oformat=elf64-x86-64 _loader.c $(OBJECTS_ASM) $(SOURCES) -o userland.elf

obj/%.asm.o : asm/%.asm
	mkdir -p obj
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm -rf *.o
	rm -rf obj

.PHONY: all clean print