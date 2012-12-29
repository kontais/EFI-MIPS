CC=gcc

SEC_OBJECTS = SecMain.o cp0.o

#all: SecMain
all: cp0.o

LINUX_APP_LIB_BEGAIN = /usr/lib/gcc/mipsel-linux-gnu/4.3.1/../../../../lib/crt1.o \
					   /usr/lib/gcc/mipsel-linux-gnu/4.3.1/../../../../lib/crti.o \
					   /usr/lib/gcc/mipsel-linux-gnu/4.3.1/crtbegin.o
LINUX_LIB_INC = -L/usr/lib/gcc/mipsel-linux-gnu/4.3.1 \
				-L/usr/lib/gcc/mipsel-linux-gnu/4.3.1 \
				-L/usr/lib/gcc/mipsel-linux-gnu/4.3.1/../../../../lib \
				-L/lib/../lib \
				-L/usr/lib/../lib \
				-L/usr/lib/gcc/mipsel-linux-gnu/4.3.1/../../.. 
LINUX_APP_LIB_END = /usr/lib/gcc/mipsel-linux-gnu/4.3.1/crtend.o \
					/usr/lib/gcc/mipsel-linux-gnu/4.3.1/../../../../lib/crtn.o

LINUX_LIB = -lgcc --as-needed \
			-lgcc_s --no-as-needed \
			-lc

LINUX_DYNAMIC_LINKER = -dynamic-linker /lib/ld.so.1
SecMain: $(SEC_OBJECTS)
	ld --eh-frame-hdr -EL $(LINUX_DYNAMIC_LINKER) -melf32ltsmip \
			-o SecMain \
			$(LINUX_APP_LIB_BEGAIN) \
			$(SEC_OBJECTS) \
			$(LINUX_LIB_INC) \
			$(LINUX_LIB) \
			$(LINUX_APP_LIB_END)

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



