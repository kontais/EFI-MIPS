a.o: a.s
	gcc -c -P -x assembler-with-cpp -mips64 -o a64.o a.s
	gcc -c -P -x assembler-with-cpp -mips3 -o a3.o a.s
#	gcc -c -P -x assembler-with-cpp -mips32 -o a32.o a.s
#	gcc -c -P -x assembler-with-cpp -mips2 a.s
#	gcc -c -P -x assembler-with-cpp -mips1 a.s

clean:
	rm -rf *.o
