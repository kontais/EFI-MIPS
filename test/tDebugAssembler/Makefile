include ../mk/make.linux.app

C_FLAGS=-Wall -mabi=32 -mips3 -EL -Wa,-mno-pdr -g



SEC_OBJECTS = SecMain.o cp0.o

all: SecMain

SecMain: $(SEC_OBJECTS)
	$(LINUX_APP_LINK)

SecMain.o: SecMain.c
	$(CC) $(C_FLAGS) -c -o SecMain.o SecMain.c
cp0.o: cp0.s ArchDefs.h
	$(CC) $(C_FLAGS) -I. -P -xassembler-with-cpp -c -o cp0.o cp0.s
clean:
	rm -rf SecMain.o SecMain cp0.o

