all: s.so o.so

s.o:s.c
	gcc  -nostdlib -nostdinc -fPIC -c -o s.o s.c

s.so: s.o
	ld -q  -fPIC -shared -e m -Ttext=100000 -o s.so s.o
	objdump -Dtr s.so > s.so.dump
	readelf -a s.so > s.so.readelf

o.o:o.c
	gcc  -nostdlib -nostdinc -G0 -mno-abicalls -fno-pic  -c -o o.o o.c

o.so: o.o
	ld -r -e m -Ttext=100000 -o o.so o.o
	objdump -Dtr o.so > o.so.dump
	readelf -a o.so > o.so.readelf


clean:
	rm -rf s.so* *.o m o.so*

#C_STD_FLAGS     = -Wall -c -o $@ $(INC)
#C_ARCH_FLAGS    = -mabi=32 -mips3 -EL -Wa,-mno-pdr
#C_PIC_FLAGS     = -G0 -mno-abicalls -fno-pic

