include ../mk/make.linux.app

SEC_OBJECTS = SecMain.o

all: SecMain


SecMain: $(SEC_OBJECTS)
	$(LINUX_APP_LINK)

SecMain.o: SecMain.c
	$(CC) -c -o SecMain.o SecMain.c

clean:
	rm -rf SecMain.o SecMain

