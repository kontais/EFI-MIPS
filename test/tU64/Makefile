CC   = gcc
ASM  = $(CC)

INC += -I ~/EFI-MIPS/Foundation/Cpu/Loongson/Include \
	-I ~/EFI-MIPS/Foundation/Cpu/Loongson/Include/Mips32 \
	-I ~/EFI-MIPS/Foundation/Include/Mips32 \
	-I ~/EFI-MIPS/Foundation/Efi/Include



C_STD_FLAGS     = -Wall -c -o $@ $(INC)
C_ARCH_FLAGS    = -mabi=32 -mips3 -EL -Wa,-mno-pdr
C_PIC_FLAGS     = -G0 -mno-abicalls -fno-pic

C_FLAGS = $(C_STD_FLAGS) $(C_ARCH_FLAGS) $(C_PIC_FLAGS)
ASM_FLAGS       = -P -x assembler-with-cpp $(C_FLAGS) -D __ASSEMBLER__

all: m

a.o: a.s
	$(ASM) $(ASM_FLAGS) a.s


m.o: m.c
	$(CC) $(C_FLAGS) m.c

m: m.o a.o 
	gcc -o m m.o a.o

clean:
	rm -rf *.o m *.txt
