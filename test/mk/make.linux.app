CC=gcc

GCC_VER=/usr/lib/gcc/mipsel-linux-gnu/4.4.5
LINUX_APP_BEGAIN = /usr/lib/crt1.o /usr/lib/crti.o $(GCC_VER)/crtbegin.o
LINUX_APP_END    = $(GCC_VER)/crtend.o /usr/lib/crtn.o
LINUX_APP_LIB_DIR    = -L$(GCC_VER) -L$(GCC_VER) -L/usr/lib -L/lib 
LINUX_APP_LIB        = -lgcc --as-needed -lgcc_s --no-as-needed -lc
LINUX_DYNAMIC_LINKER = -dynamic-linker /lib/ld.so.1
LINUX_APP_LINK = ld --eh-frame-hdr -EL $(LINUX_DYNAMIC_LINKER) -melf32ltsmip -o $@ $(LINUX_APP_BEGAIN) $^ \
                 $(LINUX_APP_LIB_DIR) $(LINUX_APP_LIB) $(LINUX_APP_END)

