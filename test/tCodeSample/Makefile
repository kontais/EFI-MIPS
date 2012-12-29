CC   = gcc
ASM  = $(CC)

INC += -I /home/zhangtao/EFI-MIPS/Foundation/Cpu/Loongson/Include \
	-I /home/zhangtao/EFI-MIPS/Foundation/Cpu/Loongson/Include/Mips32



C_STD_FLAGS     = -Wall -c -o $@ $(INC)
C_ARCH_FLAGS    = -mabi=32 -mips3 -EL -Wa,-mno-pdr
C_PIC_FLAGS     = -G0 -mno-abicalls -fno-pic

C_FLAGS = $(C_STD_FLAGS) $(C_ARCH_FLAGS) $(C_PIC_FLAGS)
ASM_FLAGS       = -P -x assembler-with-cpp $(C_FLAGS) -D __ASSEMBLER__

all: m WEAK

a.o: a.s
	$(ASM) $(ASM_FLAGS) a.s

b.o: b.s
	$(ASM) $(ASM_FLAGS) b.s

m.o: m.c
	$(CC) $(C_FLAGS) m.c
#	$(CC) $(C_STD_FLAGS) $(C_ARCH_FLAGS) m.c

m: m.o a.o b.o
	gcc -o m m.o a.o b.o

WEAK: m.o a.o b.o
	ld -r -o m1.o m.o a.o
	readelf -a m1.o > m1.txt
	ld -r -o m2.o m.o a.o b.o
	readelf -a m2.o > m2.txt

clean:
	rm -rf *.o m *.txt
