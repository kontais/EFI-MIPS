CC   = gcc
ASM  = $(CC)

INC =



C_STD_FLAGS     = -Wall -c -o $@ $(INC)
C_ARCH_FLAGS    = -mabi=32 -mips3 -EL -Wa,-mno-pdr
C_PIC_FLAGS     = -G0 -mno-abicalls -fno-pic

C_FLAGS = $(C_STD_FLAGS) $(C_ARCH_FLAGS) $(C_PIC_FLAGS)
ASM_FLAGS       = -P -x assembler-with-cpp $(C_FLAGS) -D __ASSEMBLER__

all: m

a.o: a.s
	cp a.s a.c
	gcc -E -P -mabi=32 -mips3 -x c a.c > a.pp
	$(ASM) $(ASM_FLAGS) a.s


m.o: m.c
	$(CC) $(C_FLAGS) m.c

m: m.o a.o
	ld -e TestMain -Ttext 0x10000 -o m m.o a.o


clean:
	rm -rf *.o m *.txt
