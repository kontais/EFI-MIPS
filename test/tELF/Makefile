include ../mk/make.linux.app
include make.defs


OBJECTS = Sec.o  moduleall.o

M_OBJECTS = module1.o module2.o



TARGET_LIST = Sec m.txt m.dump s.dump s.txt

all : $(TARGET_LIST)


Sec: $(OBJECTS)
	$(LINK)                         \
		$(LINK_FLAGS_EXE)               \
		$(LINUX_DYNAMIC_LINKER)         \
		-Ttext 0x413e80                 \
	 	-o Sec         \
		$(LINUX_APP_BEGAIN)         \
    $(OBJECTS)                  \
    $(LINUX_APP_LIB)                   \
		$(LINUX_APP_LIB_DIR)                \
		$(LINUX_APP_END)


Sec.o: Sec.c
	$(CC) $(C_FLAGS) Sec.c
	
module1.o: module1.c
	$(CC) $(C_FLAGS) module1.c

module2.o: module2.c
	$(CC) $(C_FLAGS) module2.c

moduleall.o: $(M_OBJECTS)
	$(LINK) $(LINK_FLAGS_DLL) $(M_OBJECTS)
	
m.txt: moduleall.o
	readelf -a moduleall.o > $@
m.dump: moduleall.o
	objdump -Dtr  moduleall.o > $@

s.dump: Sec
	objdump -Dr Sec > $@

s.txt: Sec
	readelf -a Sec > $@

	
clean:
	rm -rf $(OBJECTS) $(M_OBJECTS) $(TARGET_LIST)
