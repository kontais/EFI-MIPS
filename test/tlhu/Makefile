include ../mk/make.linux.app

SEC_OBJECTS = SecMain.o cp0.o

all: SecMain a

SecMain: $(SEC_OBJECTS)
	$(LINUX_APP_LINK)


SecMain.o: SecMain.c
	$(CC) -Wall -c -o SecMain.o SecMain.c

cp0.o: cp0.s ArchDefs.h
	$(CC) -Wall -I. -P -xassembler-with-cpp -c -o cp0.o cp0.s

a: a.c
	$(CC) -c -o a.o a.c
	objdump -d a.o



clean:
	rm -rf SecMain.o SecMain cp0.o a.o

