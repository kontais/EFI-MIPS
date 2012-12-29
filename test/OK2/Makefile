all: SimpleC

SimpleC:
	gcc -S -o SimpleC.s SimpleC.c
	as -EL -no-mdebug -mabi=32 -mno-shared -v -KPIC -o SimpleC.o SimpleC.s
	ld --eh-frame-hdr -EL -EL -dynamic-linker /lib/ld.so.1 -melf32ltsmip -o SimpleC /usr/lib/gcc/mipsel-linux-gnu/4.3.1/../../../../lib/crt1.o \
/usr/lib/gcc/mipsel-linux-gnu/4.3.1/../../../../lib/crti.o /usr/lib/gcc/mipsel-linux-gnu/4.3.1/crtbegin.o -L/usr/lib/gcc/mipsel-linux-gnu/4.3.1 \
-L/usr/lib/gcc/mipsel-linux-gnu/4.3.1 -L/usr/lib/gcc/mipsel-linux-gnu/4.3.1/../../../../lib -L/lib/../lib -L/usr/lib/../lib \
-L/usr/lib/gcc/mipsel-linux-gnu/4.3.1/../../.. SimpleC.o -lgcc --as-needed -lgcc_s --no-as-needed -lc -lgcc --as-needed -lgcc_s --no-as-needed \
/usr/lib/gcc/mipsel-linux-gnu/4.3.1/crtend.o /usr/lib/gcc/mipsel-linux-gnu/4.3.1/../../../../lib/crtn.o
clean:
	rm -f SimpleC.[so] SimpleC

