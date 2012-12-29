all:
	gcc -nostdlib -nostdinc -march=loongson2e -mtune=loongson2e -mabi=32 -mips3 -G0 -mno-abicalls -fno-pic -EL  -c -o file1.o file1.c
	objdump -Dtr -s file1.o > file1.dump
	readelf -a file1.o > file1.txt

clean:
	rm -rf file1.o file1.txt file1.dump
