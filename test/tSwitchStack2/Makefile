all: a

s.o: s.s
	gcc -march=loongson2e -mtune=loongson2e -mabi=32 -mips3 -P -x assembler-with-cpp -c -o s.o s.s

a.o: a.c
	gcc -march=loongson2e -mtune=loongson2e -mabi=32 -mips3 -c -o a.o a.c

a: a.o s.o
	gcc -o a a.o s.o

clean:
	rm -rf *.o a
