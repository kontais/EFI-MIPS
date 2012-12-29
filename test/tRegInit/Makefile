all: a

a: a.o ../../Sample/Platform/YeeLoong/Build/Mips32/RegInit.a
	gcc -g -o a a.o ../../Sample/Platform/YeeLoong/Build/Mips32/RegInit.a


a.o : a.c
	gcc -I../../Sample/Library/Sec/Include -g -c -o a.o a.c

clean:
	rm -rf a a.o

