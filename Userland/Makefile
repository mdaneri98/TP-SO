include Makefile.inc

SAMPLE_DATA=0001-terminalDataModule.bin

all: terminalCodeModule terminalDataModule

terminalCodeModule:
	cd terminalCodeModule; make

terminalDataModule:
	printf "This is sample data." >> $(SAMPLE_DATA) && dd if=/dev/zero bs=1 count=1 >> $(SAMPLE_DATA)

clean:
	cd terminalCodeModule; make clean
	rm -rf *.bin
	rm -rf *.elf


.PHONY: terminalCodeModule all clean
