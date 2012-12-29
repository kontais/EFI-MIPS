sw.o: sw.c
	gcc -c -o sw.o -mabi=32 -mips3 -EL -Wa,-mno-pdr -G0 -mno-abicalls -fno-pic -nostdinc -nostdlib -fno-builtin sw.c
	gcc -S -o sw.s -mabi=32 -mips3 -EL -Wa,-mno-pdr -G0 -mno-abicalls -fno-pic -nostdinc -nostdlib -fno-builtin sw.c
	objdump -D sw.o > sw.dump
	ld -e main -Ttext=420070 -o s sw.o
	objdump -D s > s.dump
#	gcc -Wall -fshort-wchar -c -o sw.o -mabi=32 -mips3 -EL -Wa,-mno-pdr -G0 -mno-abicalls -fno-pic -nostdinc -nostdlib -fno-builtin sw.c

clean:
	rm -rf sw.o sw.dump s s.dump sw.s
