include ../mk/make.linux.app

all: SimpleC

SimpleC.o:
	gcc -S -o SimpleC.s SimpleC.c
	as -EL -no-mdebug -mabi=32 -mno-shared -v -KPIC -o SimpleC.o SimpleC.s

SimpleC: SimpleC.o
	$(LINUX_APP_LINK)

clean:
	rm -f SimpleC.[so] SimpleC

