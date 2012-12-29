CC=gcc
ASM= $(CC)

INC = -I .

C_FLAGS =  -mips3 -c -o $@ $(INC)
ASM_FLAGS = -P -x assembler-with-cpp $(C_FLAGS)


SEC_OBJECTS = SecMain.o cp0.o

all: SecMain

SecMain: $(SEC_OBJECTS)
	$(CC)  -o SecMain $(SEC_OBJECTS)

SecMain.o: SecMain.c
	$(CC) $(C_FLAGS) SecMain.c
cp0.o: cp0.s ArchDefs.h
	$(ASM) $(ASM_FLAGS) cp0.s
clean:
	rm -rf SecMain.o SecMain cp0.o

