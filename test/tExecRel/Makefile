LOAD_ADDR=0x00400000
TOOL_DIR=/home/loongson/EFI-MIPS/Sample/Platform/YeeLoong/Build/Tools
FWIMAGE=$(TOOL_DIR)/FwImage
ELFLINK=$(TOOL_DIR)/ElfLink
ELFTOBIN=$(TOOL_DIR)/ElfToBin

all: 
	gcc -Wa,-mno-pdr -c -G 0 -mno-abicalls -fno-pic -o t.o t.c
	ld -r -d -N -G 0 -o t.rel t.o
#	$(FWIMAGE) -t 0 SECURITY_CORE t.rel t.efi
#	readelf -a t.efi > t.efi.readelf
	ld -e EntryPoint -q -d -N -G 0 -Ttext $(LOAD_ADDR) -o t.elf t.rel
	readelf -a t.elf > t.elf.readelf
	objdump -dtr t.elf > t.elf.dump
#	objcopy -R .reginfo -O binary --set-section-flags .bss=alloc,load,contents t.elf t.bin
#	$(ELFTOBIN) t.elf t.bin2
#	hd t.efi > t.efi.hex
#	hd t.bin > t.hex
#	$(ELFLINK) $(LOAD_ADDR) t.efi t.link


clean:
	rm -rf t.e* t.r* t.o*
