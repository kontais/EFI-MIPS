LOAD_ADDR=0x00400000
TOOL_DIR=/home/loongson/EFI-MIPS/Sample/Platform/YeeLoong/Build/Tools
FWIMAGE=$(TOOL_DIR)/FwImage
ELFLINK=$(TOOL_DIR)/ElfLink
ELFTOBIN=$(TOOL_DIR)/ElfToBin

all: 
	gcc -Wa,-mno-pdr -c -G 0 -mno-abicalls -fno-pic -o m.o m.c
	ld -r -d -N -G 0 -o m.rel m.o
	objcopy -R .reginfo m.rel
	$(FWIMAGE) -t 0 SECURITY_CORE m.rel m.efi
	readelf -a m.efi > m.efi.readelf
	ld -e tm -d -N -G 0 -Ttext $(LOAD_ADDR) -o m.elf m.rel
	objcopy -R .reginfo -O binary --set-section-flags .bss=alloc,load,contents m.elf m.bin
	$(ELFTOBIN) m.elf m.bin2
	objdump -Dtr m.rel > m.rel.d
	$(ELFLINK) $(LOAD_ADDR) m.efi m.link
	hd m.bin > m.bin.hex
	hd m.link > m.link.hex


clean:
	rm -rf m.o m.elf m.efi.readelf m.efi m.rel m.hex m.bin m.link *.bin* *.hex
