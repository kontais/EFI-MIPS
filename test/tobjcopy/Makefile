RMOVE_SECTIONS = -R .pdr  \
								 -R .comment \
								 -R .gnu.attributes \
								 -R .mdebug.abi32 

all:
	gcc -nostdlib -nostdinc -march=loongson2e -mtune=loongson2e -mabi=32 -mips3 -G0 -mno-abicalls -fno-pic -EL  -c -o file1.o file1.c
	objdump -Dtr -s file1.o > file1.dump
	readelf -a file1.o > file1.txt
	ld -r -d -o t.o file1.o
	cp t.o t.orig.o
	readelf -a t.o > t.all
	objcopy $(RMOVE_SECTIONS) t.o
	readelf -a t.o > t.txt
	objdump -Dtr -s t.o > t.dump



clean:
	rm -rf file1.o file1.txt file1.dump t.*
