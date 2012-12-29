all : a

a : a.o b.o c.o
	gcc -o $@ a.o b.o c.o

a.o : a.c
	gcc -c -o a.o a.c

b.o : b.c
	gcc -c -o b.o -mno-abicalls -fno-pic b.c

c.o : c.c
	gcc -c -o c.o c.c

clean :
	rm -rf a [abc].o
