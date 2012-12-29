all: a a.o a.txt ao.txt
	@echo done

a: a.c
	gcc -Wall -G0 -mno-abicalls -fno-pic -o a a.c

a.o: a.c
	gcc -Wall -G0 -mno-abicalls -fno-pic -c -o a.o a.c

ao.txt: a.o
	objdump -d a.o > ao.txt

a.txt: a
	objdump -d a > a.txt

clean:
	rm -rf a.o a.txt a ao.txt
