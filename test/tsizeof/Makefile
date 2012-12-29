CC=gcc
ASM= $(CC)

INC = -I .

C_FLAGS =  -mips32 -c -o $@ $(INC)
ASM_FLAGS = -P -x assembler-with-cpp $(C_FLAGS)


SEC_OBJECTS = SecMain.o

all: SecMain

SecMain: $(SEC_OBJECTS)
	$(CC)  -o SecMain $(SEC_OBJECTS)

SecMain.o: SecMain.c
	$(CC) $(C_FLAGS) SecMain.c

clean:
	rm -rf SecMain.o SecMain

