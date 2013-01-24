include ../mk/make.linux.app

SEC_OBJECTS = SecMain.o cp0.o

all: SecMain

SecMain: $(SEC_OBJECTS)
	$(LINUX_APP_LINK)

SecMain.o: SecMain.c
	$(CC) -Wall -c -o SecMain.o SecMain.c
cp0.o: cp0.s ArchDefs.h
	$(CC) -Wall -I. -march=loongson2e -mips3 -P -xassembler-with-cpp -c -o cp0.o cp0.s
	objdump -d cp0.o > cp0.dump
clean:
	rm -rf SecMain.o SecMain cp0.o cp0.dump

