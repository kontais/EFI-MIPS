s.o: s.s
	gcc -G 0 -mno-abicalls -fno-pic -mabi=32 -mips3 -P -x assembler-with-cpp -c -o s.o s.s
	objdump -d s.o > s.d
clean:
	rm -rf *.o s.d
