SEC_OBJECTS = SecMain.o

all: SecMain

LINUX_APP_LIB_BEGAIN = /usr/lib/gcc/mipsel-linux-gnu/4.3.1/../../../../lib/crt1.o \
					   /usr/lib/gcc/mipsel-linux-gnu/4.3.1/../../../../lib/crti.o \
					   /usr/lib/gcc/mipsel-linux-gnu/4.3.1/crtbegin.o
LINUX_LIB_INC = -L/usr/lib/gcc/mipsel-linux-gnu/4.3.1 \
				-L/usr/lib/gcc/mipsel-linux-gnu/4.3.1 \
				-L/usr/lib/gcc/mipsel-linux-gnu/4.3.1/../../../../lib \
				-L/lib/../lib \
				-L/usr/lib/../lib \
				-L/usr/lib/gcc/mipsel-linux-gnu/4.3.1/../../.. 
LINUX_APP_LIB_END = /usr/lib/gcc/mipsel-linux-gnu/4.3.1/crtend.o \
					/usr/lib/gcc/mipsel-linux-gnu/4.3.1/../../../../lib/crtn.o

LINUX_LIB = -lgcc --as-needed \
			-lgcc_s --no-as-needed \
			-lc

LINUX_DYNAMIC_LINKER = -dynamic-linker /lib/ld.so.1
SecMain: $(SEC_OBJECTS)
	ld --eh-frame-hdr -EL $(LINUX_DYNAMIC_LINKER) -melf32ltsmip \
			-o SecMain \
			$(LINUX_APP_LIB_BEGAIN) \
			$(SEC_OBJECTS) \
			$(LINUX_LIB_INC) \
			$(LINUX_LIB) \
			$(LINUX_APP_LIB_END)

SecMain.o: SecMain.c
	$(CC) -c -o SecMain.o SecMain.c

clean:
	rm -rf SecMain.o SecMain

