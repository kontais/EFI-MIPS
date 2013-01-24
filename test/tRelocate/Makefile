include ../mk/make.linux.app

SEC_OBJECTS = SecMain.o cp0.o

#all: SecMain
all: cp0.o

SecMain: $(SEC_OBJECTS)
	$(LINUX_APP_LINK)

SecMain.o: SecMain.c
	$(CC) -Wall -c -o SecMain.o SecMain.c
cp0.o: cp0.s ArchDefs.h
	$(CC) -Wall -I. -P -xassembler-with-cpp -c -o cp0.o cp0.s
clean:
	rm -rf SecMain.o SecMain cp0.o cp0.efi cp0.elf a.out cp0.bin

ld : cp0.o
	ld -e GetWord -Ttext 0x10000 cp0.o -o cp0.elf
	 objcopy -R .reginfo -O binary --set-section-flags .bss=alloc,load,contents cp0.elf cp0.bin

efi: cp0.o
	./ElfLink 0x10000 cp0.o cp0.efi



